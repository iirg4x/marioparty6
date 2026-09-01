from __future__ import annotations

import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest
from unittest import mock


TOOL = Path(__file__).resolve().parents[1] / "capsule_same_session_capture.py"
SPEC = importlib.util.spec_from_file_location("capsule_same_session_capture_test_target", TOOL)
assert SPEC is not None and SPEC.loader is not None
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
VARINFO_TOOL = TOOL.with_name("mwcc_win32_varinfo.py")
VARINFO_SPEC = importlib.util.spec_from_file_location("mwcc_win32_varinfo_test_target", VARINFO_TOOL)
assert VARINFO_SPEC is not None and VARINFO_SPEC.loader is not None
VARINFO = importlib.util.module_from_spec(VARINFO_SPEC)
VARINFO_SPEC.loader.exec_module(VARINFO)


def descriptor(path: Path) -> dict[str, object]:
    data = path.read_bytes()
    return {"path": str(path), "size": len(data), "sha256": hashlib.sha256(data).hexdigest()}


def minimal_ppc_elf_object() -> bytes:
    ident = b"\x7fELF\x01\x02\x01" + b"\x00" * 9
    header = struct.pack(
        ">16sHHIIIIIHHHHHH",
        ident,
        1,
        20,
        1,
        0,
        0,
        52,
        0,
        52,
        0,
        0,
        40,
        1,
        0,
    )
    return header + b"\x00" * 40


class FakeBackend:
    capabilities = {"read_image", "install_breakpoint", "remove_breakpoint", "single_step", "run", "close"}

    def __init__(self, *, conflict: bool = False, duplicate_object: bool = False, duplicate_vreg: bool = False, rewrite: Path | None = None, physical_rows: list[dict[str, object]] | None = None, machine_rows: list[dict[str, object]] | None = None, physical_before_inventory: bool = False, inventory_empty_first: bool = False, token_collision: bool = False) -> None:
        self.conflict = conflict
        self.duplicate_object = duplicate_object
        self.duplicate_vreg = duplicate_vreg
        self.rewrite = rewrite
        self.physical_rows = list(physical_rows or [])
        self.machine_rows = list(machine_rows or [])
        self.physical_before_inventory = physical_before_inventory
        self.inventory_empty_first = inventory_empty_first
        self.token_collision = token_collision
        self.inventory_calls = 0
        self.session: object | None = None
        self.active: set[int] = set()
        self.installs: list[int] = []
        self.removes: list[int] = []
        self.steps: list[tuple[int, int, bool]] = []
        self.closed = False
        self.process_id = 4321
        self.write_index = 0
        self.expected_prefixes: dict[int, bytes] = {}

    def read_image(self, address: int, size: int) -> bytes:
        prefix = self.expected_prefixes.get(address)
        if prefix is None:
            prefix = bytes.fromhex(str(MODULE.HOOK_BY_ADDRESS[address]["prefix"]))
        return prefix[:size]

    def expected_hook_prefix(self, row: dict[str, object]) -> bytes:
        """Return the selected profile row when two compilers share an address."""

        prefix = bytes.fromhex(str(row["prefix"]))
        self.expected_prefixes[int(row["address"])] = prefix
        return prefix

    def install_breakpoint(self, address: int) -> None:
        if address in self.active:
            raise MODULE.Rejected(f"duplicate fake install 0x{address:08x}")
        self.active.add(address)
        self.installs.append(address)

    def remove_breakpoint(self, address: int) -> None:
        self.active.discard(address)
        self.removes.append(address)

    def single_step(self, address: int, thread_id: int, *, rearm: bool) -> None:
        self.steps.append((address, thread_id, rearm))
        self.active.discard(address)

    def snapshot_inventory(self) -> dict[str, list[dict[str, object]]]:
        self.inventory_calls += 1
        if self.inventory_empty_first and self.inventory_calls == 1:
            return {"locals": [], "arguments": []}
        locals_rows = [
            {"pointer": 0x1010, "varinfo_pointer": 0x5010, "name": "local_gpr"},
            {"pointer": 0x1020, "varinfo_pointer": 0x5020, "name": "local_fpr"},
        ]
        arguments_rows = [
            {"pointer": 0x2010, "varinfo_pointer": 0x6010, "name": "arg_gpr"},
            {"pointer": 0x2020, "varinfo_pointer": 0x6020, "name": "arg_fpr"},
        ]
        if self.duplicate_object:
            locals_rows.append(dict(locals_rows[0]))
        return {"locals": locals_rows, "arguments": arguments_rows}

    def capture_stack_write(self, hook_id: str, thread_id: int) -> dict[str, object]:
        del thread_id
        pointer = 0x1010 + (self.write_index % 2) * 0x10
        self.write_index += 1
        return {"object": pointer, "value": 16 + self.write_index, "kind": "local"}

    def capture_pcode(self, hook_id: str, thread_id: int) -> dict[str, object]:
        del thread_id
        return {"status": "CAPTURED", "stage": hook_id}

    def capture_regalloc(self, hook_id: str, thread_id: int) -> list[dict[str, object]]:
        del hook_id, thread_id
        rows = [
            {"object": 0x1010, "kind": "local", "ig_node": 0x3010, "vreg_id": "r32", "bank": "GPR"},
            {"object": 0x1020, "kind": "local", "ig_node": 0x3020, "vreg_id": "f32", "bank": "FPR"},
            {"object": 0x2010, "kind": "argument", "ig_node": 0x4010, "vreg_id": "r33", "bank": "GPR"},
            {"object": 0x2020, "kind": "argument", "ig_node": 0x4020, "vreg_id": "f33", "bank": "FPR"},
        ]
        if self.duplicate_vreg:
            rows[-1] = dict(rows[-1], vreg_id="r32", bank="GPR")
        if self.token_collision:
            assert self.session is not None
            rows[0] = dict(
                rows[0],
                object_token=f"argument-{self.session.session_id}-000000",
            )
        return rows

    def capture_physical_regalloc(self, hook_id: str, thread_id: int) -> dict[str, object]:
        del hook_id, thread_id
        if self.physical_rows:
            return self.physical_rows.pop(0)
        return {
            "object": 0x1010,
            "varinfo_pointer": 0x5010,
            "noregister": 0,
            "flags": 2,
            "rclass": 4,
            "reg": 17,
            "reg_hi": 0,
        }

    def capture_machine_emission(self, hook_id: str, thread_id: int) -> dict[str, object]:
        del hook_id, thread_id
        if not self.machine_rows:
            return {"status": "UNKNOWN", "reason": "incomplete machine emission evidence"}
        return self.machine_rows.pop(0)

    def current_function(self) -> str:
        return "mbCapListDebug"

    def run(self, session: MODULE.CombinedCaptureSession) -> None:
        self.session = session
        hook_by_id = {str(row["id"]): row for row in session.auth["hooks"]}
        physical_hook_id = next(
            str(row["id"])
            for row in session.auth["hooks"]
            if row["role"] == "regalloc_post"
        )
        write_hook_ids = tuple(
            str(row["id"])
            for row in session.auth["hooks"]
            if row["role"] == "object_stack_write"
        )
        pcode_hook_ids = MODULE._pcode_stage_hook_ids(tuple(session.auth["hooks"]))
        session.on_process_started(self.process_id)
        session.on_breakpoint(int(hook_by_id["function_filter"]["address"]), 1)
        session.on_single_step(1)
        if self.physical_before_inventory:
            session.on_breakpoint(int(hook_by_id[physical_hook_id]["address"]), 1)
            session.on_single_step(1)
        session.on_breakpoint(int(hook_by_id["allocation_pre"]["address"]), 1)
        session.on_single_step(1)
        session.on_breakpoint(int(hook_by_id["allocation_post"]["address"]), 1)
        session.on_single_step(1)
        for hook_id in write_hook_ids:
            address = int(hook_by_id[hook_id]["address"])
            session.on_breakpoint(address, 1)
            session.on_single_step(1)
        for hook_id in pcode_hook_ids:
            session.on_breakpoint(int(hook_by_id[hook_id]["address"]), 1)
            session.on_single_step(1)
        session.on_breakpoint(int(hook_by_id["regalloc"]["address"]), 1)
        session.on_single_step(1)
        if not self.physical_before_inventory:
            physical_rows = max(1, len(self.physical_rows))
            for _ in range(physical_rows):
                session.on_breakpoint(int(hook_by_id[physical_hook_id]["address"]), 1)
                session.on_single_step(1)
        machine_count = len(self.machine_rows)
        if machine_count and int(MODULE.GC27_MACHINE_EMIT_HOOK["address"]) in self.active:
            for _ in range(machine_count):
                session.on_breakpoint(int(MODULE.GC27_MACHINE_EMIT_HOOK["address"]), 1)
                session.on_single_step(1)
        if self.rewrite is not None:
            self.rewrite.write_text("rewritten", encoding="utf-8")
        session.on_process_exit(0)

    def close(self) -> None:
        self.closed = True


class GC26FrontendBackend(FakeBackend):
    """Fake one-process backend exercising the combined frontend hooks."""

    def __init__(self, *, duplicate_frontend_object: bool = False) -> None:
        super().__init__()
        self.duplicate_frontend_object = duplicate_frontend_object

    def capture_frontend(self, hook_id: str, thread_id: int) -> dict[str, object]:
        del thread_id
        if hook_id in MODULE._frontend_chronology.GENERIC_HOOK_IDS:
            index = MODULE._frontend_chronology.GENERIC_HOOK_IDS.index(hook_id)
            pointers = [0x1010, 0x1020, 0x2010]
            if self.duplicate_frontend_object and index == 1:
                return {"pointer": pointers[0]}
            return {"pointer": pointers[index]}
        if hook_id == "bulk_object_link":
            return {"object_pointers": [0x1010, 0x1020, 0x2010]}
        raise MODULE.Rejected(f"unexpected fake frontend hook: {hook_id}")

    def run(self, session: MODULE.CombinedCaptureSession) -> None:
        self.session = session
        hook_by_id = {str(row["id"]): row for row in session.auth["hooks"]}

        def hit(hook_id: str) -> None:
            session.on_breakpoint(int(hook_by_id[hook_id]["address"]), 1)
            session.on_single_step(1)

        session.on_process_started(self.process_id)
        hit("reset")
        for hook_id in MODULE._frontend_chronology.GENERIC_HOOK_IDS:
            hit(hook_id)
        hit("bulk_object_link")
        # Frontend list construction precedes the shared function-filter
        # boundary in the authenticated chronology; the combined session must
        # retain these generations before selecting the target epoch.
        hit("function_filter")
        hit("allocation_pre")
        hit("allocation_post")
        for hook_id in (
            str(row["id"])
            for row in session.auth["hooks"]
            if row["role"] == "object_stack_write"
        ):
            hit(hook_id)
        for hook_id in MODULE._pcode_stage_hook_ids(tuple(session.auth["hooks"])):
            hit(hook_id)
        hit("regalloc")
        hit("regalloc_post")
        session.on_process_exit(0)


def auth(root: Path, *, session_id: str = "session-0000000000000001") -> dict[str, object]:
    files = {}
    for name, data in (
        ("capsule.c", b"capsule source\n"),
        ("mwcceppc.exe", b"compiler image\n"),
        ("authority", b"external authority\n"),
    ):
        path = root / f"{name}.bin"
        if name.endswith(".c") or name.endswith(".exe"):
            path = root / name
        path.write_bytes(data)
        files["source" if name.endswith(".c") else "compiler" if name.endswith(".exe") else name] = descriptor(path)
    files["wrapper"] = files["authority"]
    files["debugger"] = files["authority"]
    files["transport"] = files["authority"]
    request = {
        "function": "mbCapListDebug",
        "function_sha256": "a" * 64,
        "argv": ["mwcceppc.exe", "-c", str(root / "capsule.c")],
        "cwd": str(root),
        "source": files["source"],
        "compiler": files["compiler"],
        "authority": files["authority"],
        "session_id": session_id,
    }
    return {
        "request": request,
        "request_path": root / "request.json",
        "request_sha256": "b" * 64,
        "paths": {},
    }


def trust_root_for_request(request_path: Path, *, include_outputs: bool = False) -> MODULE.ExternalTrustRoot:
    request = MODULE.strict_json_loads(request_path.read_text(encoding="utf-8"), "request")
    values: dict[str, object] = {
        "request": descriptor(request_path),
        "function": request["function"],
        "function_sha256": request["function_sha256"],
        "cwd": request["cwd"],
        "argv": tuple(request["argv"]),
    }
    for name in ("source", "compiler", "wrapper", "debugger", "transport", "authority"):
        values[name] = request[name]
    if include_outputs:
        for key in ("event_stream_stack", "event_stream_pcode", "envelope"):
            values[key] = descriptor(Path(request["paths"][key]))
    return MODULE.ExternalTrustRoot.from_mapping(values)


def reseal_capture(envelope_path: Path, request_path: Path, envelope: dict[str, object]) -> MODULE.ExternalTrustRoot:
    """Rewrite only authenticated derived bytes for adversarial fixtures."""

    events = envelope["events"]
    assert isinstance(events, list)
    session_id = str(envelope["context"]["session_id"])
    for index, event in enumerate(events):
        event["sequence"] = index
        event["event_id"] = f"{session_id}-e{index:06d}"
    envelope["event_count"] = len(events)
    lanes = {}
    for lane, key in (("stack", "event_stream_stack"), ("pcode", "event_stream_pcode")):
        lane_events = [event for event in events if event["lane"] == lane]
        data = MODULE.canonical_lane_bytes(lane_events)
        path = Path(envelope["outputs"][key]["path"])
        path.write_bytes(data)
        digest = hashlib.sha256(data).hexdigest()
        lanes[lane] = {
            "event_count": len(lane_events),
            "event_ids": [event["event_id"] for event in lane_events],
            "size_bytes": len(data),
            "sha256": digest,
        }
        envelope["outputs"][key] = {"path": str(path), "size": len(data), "sha256": digest}
    envelope["lanes"] = lanes
    unsigned = {key: value for key, value in envelope.items() if key != "envelope_sha256"}
    envelope["envelope_sha256"] = MODULE.canonical_hash(unsigned)
    envelope_path.write_text(json.dumps(envelope, sort_keys=True, indent=2), encoding="utf-8")
    return trust_root_for_request(request_path, include_outputs=True)


def prepared_capture(root: Path, *, backend: FakeBackend | None = None, session_id: str = "session-0000000000000009", source_data: bytes = b"s") -> tuple[Path, Path, dict[str, object], MODULE.ExternalTrustRoot]:
    for name, data in (
        ("capsule.c", source_data), ("mwcceppc.exe", b"c"), ("authority.bin", b"a"),
        ("wrapper.bin", b"w"), ("debugger.bin", b"d"), ("transport.bin", b"t"),
    ):
        (root / name).write_bytes(data)
    request_path = MODULE.prepare_request(
        {
            "function": "mbCapListDebug",
            "function_sha256": "a" * 64,
            "argv": ["wrapper.bin", "mwcceppc.exe", "-c", str(root / "capsule.c")],
            "cwd": str(root),
            "source": str(root / "capsule.c"),
            "compiler": str(root / "mwcceppc.exe"),
            "wrapper": str(root / "wrapper.bin"),
            "debugger": str(root / "debugger.bin"),
            "transport": str(root / "transport.bin"),
            "authority": str(root / "authority.bin"),
            "session_id": session_id,
        },
        root / "capture",
    )
    root_anchor = trust_root_for_request(request_path)
    captured = MODULE.capture_with_backend(request_path, backend or FakeBackend(), external_trust_root=root_anchor)
    envelope_path = root / "capture" / "same-session.envelope.json"
    return request_path, envelope_path, captured, trust_root_for_request(request_path, include_outputs=True)
class CapsuleSameSessionCaptureTests(unittest.TestCase):
    def _gc26_auth(self, root: Path) -> dict[str, object]:
        """Return a programmatic auth fixture selecting the GC/2.6 profile."""

        value = auth(root)
        request = dict(value["request"])
        # The fake image need not exist: _path_descriptor intentionally permits
        # sealed non-live descriptors for programmatic fake sessions.  Using a
        # distinct path also prevents the fixture bytes from being mistaken for
        # the authenticated MWCC/2.6 image.
        request["compiler"] = {
            "path": str(root / "gc26-mwcceppc.exe"),
            "size": 0,
            "sha256": MODULE.GC26_COMPILER_SHA256,
        }
        value["request"] = request
        return value

    def test_gc26_frontend_chronology_is_embedded_in_one_session(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            backend = GC26FrontendBackend()
            session = MODULE.CombinedCaptureSession(self._gc26_auth(root), backend)
            envelope = session.run()

            frontend = envelope.get("frontend_chronology")
            self.assertIsInstance(frontend, dict)
            assert isinstance(frontend, dict)
            self.assertEqual(frontend.get("status"), "CAPTURED")
            packet = frontend.get("packet")
            self.assertIsInstance(packet, dict)
            assert isinstance(packet, dict)
            validated = MODULE._frontend_chronology.validate_packet(packet)
            self.assertEqual(validated, packet)
            self.assertEqual(
                packet["provenance"]["session_id"],
                envelope["context"]["session_id"],
            )
            self.assertEqual(
                packet["provenance"]["compiler_sha256"],
                MODULE.GC26_COMPILER_SHA256,
            )
            self.assertEqual(
                set(backend.installs),
                {int(row["address"]) for row in MODULE.GC26_HOOKS},
            )
            self.assertEqual(
                {event["lane"] for event in envelope["events"]},
                {"stack", "pcode"},
            )

    def test_gc26_frontend_unknown_retains_combined_stack_evidence(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            backend = GC26FrontendBackend(duplicate_frontend_object=True)
            session = MODULE.CombinedCaptureSession(self._gc26_auth(root), backend)
            envelope = session.run()

            frontend = envelope.get("frontend_chronology")
            self.assertEqual(frontend, {
                "status": "UNKNOWN",
                "reason": "incomplete frontend chronology",
            })
            self.assertIn("incomplete frontend chronology", envelope["unknown"])
            self.assertTrue(any(event["lane"] == "stack" for event in envelope["events"]))
            self.assertTrue(any(event["lane"] == "pcode" for event in envelope["events"]))

    def test_gc27_profile_does_not_install_frontend_hooks(self) -> None:
        hooks = MODULE._hooks_for_compiler(MODULE.GC27_COMPILER_SHA256)
        self.assertEqual(len(hooks), len(MODULE.GC27_HOOKS))
        self.assertFalse(any(row.get("lane") == "frontend" for row in hooks))
        self.assertTrue(
            {str(row["id"]) for row in hooks}.isdisjoint(MODULE.GC26_FRONTEND_HOOK_IDS)
        )

    def test_session_breakpoint_cleanup_does_not_mask_primary_failure(self) -> None:
        class PrimaryAndBreakpointCleanupFailure(FakeBackend):
            def run(self, session: MODULE.CombinedCaptureSession) -> None:
                del session
                raise MODULE.Rejected("primary compiler failure")

            def remove_breakpoint(self, address: int) -> None:
                del address
                raise MODULE.Rejected("restore failed")

        with TemporaryDirectory() as directory:
            session = MODULE.CombinedCaptureSession(
                auth(Path(directory)), PrimaryAndBreakpointCleanupFailure()
            )
            with self.assertRaisesRegex(
                MODULE.Rejected,
                "primary compiler failure; secondary failure: breakpoint cleanup failure",
            ) as caught:
                session.run()
            self.assertEqual(caught.exception.primary_failure, "primary compiler failure")

    def test_provisional_invalid_inventory_does_not_poison_complete_refresh(self) -> None:
        class ProvisionalInvalidInventory(FakeBackend):
            def snapshot_inventory(self) -> dict[str, list[dict[str, object]]]:
                self.inventory_calls += 1
                if self.inventory_calls == 1:
                    return {"locals": [{"pointer": 0}], "arguments": []}
                self.inventory_calls -= 1
                return super().snapshot_inventory()

        with TemporaryDirectory() as directory:
            session = MODULE.CombinedCaptureSession(
                auth(Path(directory)),
                ProvisionalInvalidInventory(),
            )
            session._capture_inventory(force=True)
            self.assertFalse(session.inventory_complete)
            self.assertEqual(session.inventory_snapshot_reasons, {"null object identity"})
            self.assertNotIn("null object identity", session.unknown)

            session._capture_inventory(force=True)
            self.assertTrue(session.inventory_complete)
            self.assertEqual(session.inventory_snapshot_reasons, set())
            self.assertNotIn("null object identity", session.unknown)

    def test_one_executed_alternate_stack_write_site_validates_without_unknown(self) -> None:
        class OneWriteBackend(FakeBackend):
            def run(self, session: MODULE.CombinedCaptureSession) -> None:
                self.session = session
                hooks = {str(row["id"]): row for row in session.auth["hooks"]}
                session.on_process_started(self.process_id)
                for hook_id in ("function_filter", "allocation_pre"):
                    session.on_breakpoint(int(hooks[hook_id]["address"]), 1)
                    session.on_single_step(1)
                session.on_breakpoint(int(hooks["object_write_0"]["address"]), 1)
                session.on_single_step(1)
                session.on_breakpoint(int(hooks["allocation_post"]["address"]), 1)
                session.on_single_step(1)
                session.on_breakpoint(int(hooks["regalloc"]["address"]), 1)
                session.on_single_step(1)
                session.on_breakpoint(int(hooks["regalloc_post"]["address"]), 1)
                session.on_single_step(1)
                session.on_process_exit(0)

        with TemporaryDirectory() as directory:
            request, envelope_path, envelope, trust = prepared_capture(
                Path(directory),
                backend=OneWriteBackend(),
            )
            del request
            self.assertFalse(any(event["event_kind"] == "lane_unknown" for event in envelope["events"]))
            self.assertNotIn("incomplete stack evidence", envelope["unknown"])
            MODULE.validate_envelope(envelope_path, trust_root=trust)

    def test_exact_path_import_loads_dependencies_from_tool_directory(self) -> None:
        script = r"""
import importlib.util
import json
from pathlib import Path
import sys

tool = Path(sys.argv[1]).resolve()
spec = importlib.util.spec_from_file_location("isolated_capsule_capture", tool)
assert spec is not None and spec.loader is not None
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
print(json.dumps({
    "stack": str(Path(module._stack_home.__file__).resolve()),
    "donor": str(Path(module._donor_cfg.__file__).resolve()),
    "frontend": str(Path(module._frontend_chronology.__file__).resolve()),
    "correlator": str(Path(module._correlator.__file__).resolve()),
    "shared_stack": module._frontend_chronology._stack_home is module._stack_home,
}, sort_keys=True))
"""
        with TemporaryDirectory() as directory:
            completed = subprocess.run(
                [sys.executable, "-I", "-c", script, str(TOOL)],
                cwd=directory,
                check=False,
                capture_output=True,
                text=True,
            )
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(
            json.loads(completed.stdout),
            {
                "stack": str(TOOL.with_name("capsule_stack_home_native.py").resolve()),
                "donor": str(TOOL.with_name("donor_cfg_align.py").resolve()),
                "frontend": str(TOOL.with_name("mwcc_fe_chronology_native.py").resolve()),
                "correlator": str(TOOL.with_name("pcode_varinfo_correlator.py").resolve()),
                "shared_stack": True,
            },
        )

    def test_exact_path_import_rejects_private_package_alias_collision(self) -> None:
        script = r"""
import importlib.util
from pathlib import Path
import sys
import types
import uuid

class FixedUuid:
    hex = "fixedcollision"

uuid.uuid4 = lambda: FixedUuid()
key = "_capsule_same_session_capture_bundle_fixedcollision"
sys.modules[key] = types.ModuleType(key)
tool = Path(sys.argv[1]).resolve()
spec = importlib.util.spec_from_file_location("isolated_capsule_capture", tool)
assert spec is not None and spec.loader is not None
module = importlib.util.module_from_spec(spec)
try:
    spec.loader.exec_module(module)
except ImportError as exc:
    print(str(exc))
else:
    raise SystemExit("alias collision was accepted")
"""
        with TemporaryDirectory() as directory:
            completed = subprocess.run(
                [sys.executable, "-I", "-c", script, str(TOOL)],
                cwd=directory,
                check=False,
                capture_output=True,
                text=True,
            )
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(
            completed.stdout.strip(),
            "private same-directory tool package alias collision",
        )

    def test_execution_transport_is_sealed_and_tamper_rejected(self) -> None:
        class ExecutionBackend(FakeBackend):
            def transport_provenance(self, request_argv: list[str]) -> dict[str, object]:
                return {
                    "mode": "wrapper_memexec",
                    "argv": list(request_argv),
                    "request_argv_sha256": MODULE.canonical_hash({"argv": list(request_argv)}),
                    "wrapper_bypassed": False,
                }

        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, trust = prepared_capture(
                root,
                backend=ExecutionBackend(),
            )
            execution = envelope["context"]["execution"]
            self.assertEqual(execution["mode"], "wrapper_memexec")
            self.assertFalse(execution["wrapper_bypassed"])
            MODULE.validate_envelope(envelope_path, trust_root=trust)

            execution["argv"] = [*execution["argv"], "-changed"]
            tampered_trust = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "wrapper execution provenance"):
                MODULE.validate_envelope(envelope_path, trust_root=tampered_trust)

    def test_envelope_authority_advance_claim_fails_closed(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, _ = prepared_capture(root)
            envelope["authority_advanced"] = True
            envelope["envelope_sha256"] = MODULE.canonical_hash(
                {key: value for key, value in envelope.items() if key != "envelope_sha256"}
            )
            envelope_path.write_text(json.dumps(envelope), encoding="utf-8")
            trust = trust_root_for_request(request_path, include_outputs=True)
            with self.assertRaisesRegex(MODULE.Rejected, "policy mismatch"):
                MODULE.validate_envelope(envelope_path, trust_root=trust)

    def test_explicitly_hashed_board_function_is_admitted_without_name_allowlist(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "src" / "board" / "capthrow.c"
            source.parent.mkdir(parents=True)
            source.write_text("void mbev_CapTogezo(void) {}\n", encoding="utf-8")
            files = {}
            for name in ("mwcceppc.exe", "wrapper.bin", "debugger.bin", "transport.bin", "authority.bin"):
                path = root / name
                path.write_bytes(name.encode("ascii"))
                files[name] = path
            request = MODULE.prepare_request({
                "function": "mbev_CapTogezo",
                "function_sha256": "1" * 64,
                "argv": [str(files["wrapper.bin"]), str(files["mwcceppc.exe"]), "-c", str(source)],
                "cwd": str(root),
                "source": str(source),
                "compiler": str(files["mwcceppc.exe"]),
                "wrapper": str(files["wrapper.bin"]),
                "debugger": str(files["debugger.bin"]),
                "transport": str(files["transport.bin"]),
                "authority": str(files["authority.bin"]),
                "session_id": "session-1111111111111111",
            }, root / "capture")
            parsed = MODULE.strict_json_loads(request.read_text(encoding="utf-8"), "request")
        self.assertEqual(parsed["function"], "mbev_CapTogezo")
        self.assertEqual(parsed["function_sha256"], "1" * 64)

    def test_hook_union_uses_authenticated_stack_and_regalloc_sites(self) -> None:
        self.assertEqual(
            [row["id"] for row in MODULE.HOOKS],
            [
                "function_filter",
                "allocation_pre",
                "allocation_post",
                "object_write_0",
                "object_write_1",
                "object_write_2",
                "regalloc",
                "regalloc_post",
            ],
        )
        self.assertEqual(
            MODULE.HOOK_BY_ID["regalloc"],
            {
                "id": "regalloc",
                "address": 0x0043598B,
                "prefix": "ff74240ce89ca809",
                "lane": "pcode",
                "role": "regalloc",
            },
        )
        self.assertEqual(MODULE.PCODE_HOOK_IDS, ())
        self.assertFalse({"pcode_create", "pcode_emit", "pcode_link"} & set(MODULE.HOOK_BY_ID))
        self.assertEqual(
            [row["id"] for row in MODULE.GC27_HOOKS],
            [
                "function_filter",
                "allocation_pre",
                "allocation_post",
                "object_write_0",
                "object_write_1",
                "object_write_2",
                "regalloc",
                "physical_pair_commit",
                "physical_single_commit",
                "precolored_commit",
                "pcode_color_pre",
                "pcode_color_post",
                "gc27_machine_emit",
            ],
        )
        self.assertNotIn(0x004D03E8, {row["address"] for row in MODULE.GC27_HOOKS})
        gc27_by_id = {str(row["id"]): row for row in MODULE.GC27_HOOKS}
        self.assertEqual(
            {
                hook_id: {
                    "address": gc27_by_id[hook_id]["address"],
                    "prefix": gc27_by_id[hook_id]["prefix"],
                }
                for hook_id in (
                    "allocation_pre",
                    "object_write_0",
                    "object_write_1",
                    "object_write_2",
                )
            },
            {
                "allocation_pre": {
                    "address": 0x0043367E,
                    "prefix": "e87d650c00598a44240450",
                },
                "object_write_0": {
                    "address": 0x004F9D74,
                    "prefix": "89432e8b530e8b420201e84821f00105e40c",
                },
                "object_write_1": {
                    "address": 0x004F9E11,
                    "prefix": "89432e8b4b0e8b410201e84821f00105dc0c",
                },
                "object_write_2": {
                    "address": 0x004F9E98,
                    "prefix": "89432e8b4b0e8b410201e84821f00105d80c",
                },
            },
        )
        legacy_by_id = {str(row["id"]): row for row in MODULE.LEGACY_HOOKS}
        self.assertEqual(legacy_by_id["object_write_0"]["address"], 0x004F9E54)
        self.assertEqual(legacy_by_id["allocation_pre"]["prefix"], "e85d660c00598a44240450")

        self.assertEqual(
            MODULE.HOOK_BY_ID["gc27_machine_emit"],
            {
                "id": "gc27_machine_emit",
                "address": 0x004EB21F,
                "prefix": "8b178b0a030dd00b5e0001e989018b43",
                "lane": "pcode",
                "role": "machine_emit",
            },
        )
        self.assertEqual(MODULE._hooks_for_compiler(MODULE.GC27_COMPILER_SHA256), MODULE.GC27_HOOKS)
        self.assertEqual(MODULE._hooks_for_compiler("0" * 64), MODULE.LEGACY_HOOKS)

        self.assertEqual(
            MODULE.HOOK_BY_ID["regalloc_post"],
            {
                "id": "regalloc_post",
                "address": 0x004D03E8,
                "prefix": "83c4085d5e5bc300",
                "lane": "pcode",
                "role": "regalloc_post",
            },
        )

    def test_gc27_v523_style_patch_prepares_and_authenticates_closed_profile(self) -> None:
        patch_source = '''
PHYSICAL = (
    ("physical_pair_commit", 0x004D0E65, "5d5f5e5bc3"),
    ("physical_single_commit", 0x004D0F6E, "5d5f5e5bc3"),
    ("precolored_commit", 0x004D0A7B, "eb768d4000"),
)
COLORS = (
    ("pcode_color_pre", 0x005086C4, "6689420483c20c83"),
    ("pcode_color_post", 0x005086C8, "83c20c83ed0173d3"),
)
def hook_union(central):
    base = [dict(row) for row in central.GC27_BASE_HOOKS]
    physical = [
        {"id": name, "address": address, "prefix": prefix,
         "lane": "pcode", "role": "regalloc_post"}
        for name, address, prefix in PHYSICAL
    ]
    colors = [
        {"id": name, "address": address, "prefix": prefix,
         "lane": "pcode", "role": "pcode_color_diagnostic"}
        for name, address, prefix in COLORS
    ]
    return tuple(base + physical + colors + [dict(central.GC27_MACHINE_EMIT_HOOK)])
'''
        with TemporaryDirectory() as directory:
            root = Path(directory)
            patch_path = root / "v523_patch.py"
            patch_path.write_text(patch_source, encoding="utf-8")
            spec = importlib.util.spec_from_file_location("v523_style_hook_patch", patch_path)
            assert spec is not None and spec.loader is not None
            patch_module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(patch_module)
            patched_hooks = patch_module.hook_union(MODULE)
            self.assertEqual(patched_hooks, MODULE.GC27_HOOKS)

            source = root / "player.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "wrapper.exe"
            debugger = root / "debugger.exe"
            transport = root / "transport.exe"
            authority = root / "authority.bin"
            source.write_bytes(b"int player;\n")
            for path in (compiler, wrapper, debugger, transport, authority):
                path.write_bytes(path.name.encode("ascii"))

            actual_hash = MODULE.sha256

            def gc27_hash(path: Path) -> str:
                selected = Path(path).resolve()
                if selected == compiler.resolve():
                    return MODULE.GC27_COMPILER_SHA256
                return actual_hash(selected)

            original_hooks = MODULE.HOOKS
            try:
                MODULE.HOOKS = patched_hooks
                with (
                    mock.patch.object(MODULE, "sha256", side_effect=gc27_hash),
                    mock.patch.object(MODULE, "_validate_authenticated_compiler_hook_image"),
                ):
                    request_path = MODULE.prepare_request(
                        {
                            "function": "MoveNumOMExec",
                            "function_sha256": "a" * 64,
                            "argv": [str(wrapper), str(compiler), "-c", str(source)],
                            "cwd": str(root),
                            "source": str(source),
                            "compiler": str(compiler),
                            "wrapper": str(wrapper),
                            "debugger": str(debugger),
                            "transport": str(transport),
                            "authority": str(authority),
                            "session_id": "session-2727272727272727",
                        },
                        root / "capture",
                    )
                    request = MODULE.strict_json_loads(request_path.read_text(encoding="utf-8"), "request")
                    self.assertEqual(tuple(request["hooks"]), patched_hooks)
                    trust = trust_root_for_request(request_path)
                    authenticated = MODULE.authenticate_request(request_path, external_trust_root=trust)
                    self.assertEqual(tuple(authenticated["hooks"]), patched_hooks)

                for runtime_mutation in (
                    tuple(dict(row) for row in MODULE.LEGACY_HOOKS),
                    patched_hooks[:-1],
                    patched_hooks + (dict(patched_hooks[-1]),),
                    patched_hooks[:-1] + ({**patched_hooks[-1], "prefix": "00"},),
                ):
                    MODULE.HOOKS = runtime_mutation
                    with self.assertRaisesRegex(MODULE.Rejected, "private backend hook patch"):
                        MODULE._validate_runtime_hook_patch(MODULE.GC27_COMPILER_SHA256)
            finally:
                MODULE.HOOKS = original_hooks

    def test_gc27_profile_matches_authenticated_compiler_file_before_launch(self) -> None:
        compiler = Path(
            r"D:\Games\Emulation\GameCube-Wii\_mp6_rebuild"
            r"\external_refs\Compilers\GC\2.7\mwcceppc.exe"
        )
        if not compiler.is_file():
            self.skipTest("authenticated GC/2.7 compiler is unavailable")
        compiler_descriptor = descriptor(compiler)
        self.assertEqual(compiler_descriptor["sha256"], MODULE.GC27_COMPILER_SHA256)
        MODULE._validate_authenticated_compiler_hook_image(
            compiler_descriptor,
            MODULE.GC27_HOOKS,
        )

        legacy_by_id = {str(row["id"]): row for row in MODULE.LEGACY_HOOKS}
        stale_ids = {
            "allocation_pre",
            "object_write_0",
            "object_write_1",
            "object_write_2",
        }
        stale = tuple(
            dict(legacy_by_id[str(row["id"])])
            if str(row["id"]) in stale_ids
            else dict(row)
            for row in MODULE.GC27_HOOKS
        )
        with self.assertRaisesRegex(
            MODULE.Rejected,
            r"0043367e.*004f9e54.*004f9ef1.*004f9f78",
        ):
            MODULE._validate_authenticated_compiler_hook_image(
                compiler_descriptor,
                stale,
            )

        profile_hooks = MODULE.GC27_HOOKS
        for mutation in (
            [dict(row) for row in profile_hooks[:-1]],
            [dict(row) for row in profile_hooks] + [dict(profile_hooks[-1])],
            [
                *[dict(row) for row in profile_hooks[:-1]],
                {**profile_hooks[-1], "prefix": "00"},
            ],
            [
                *[dict(row) for row in profile_hooks[:-2]],
                dict(profile_hooks[-1]),
                dict(profile_hooks[-2]),
            ],
        ):
            with self.assertRaisesRegex(MODULE.Rejected, "complete pinned hook union|does not match"):
                MODULE._validate_hook_rows(
                    mutation,
                    compiler_sha256=MODULE.GC27_COMPILER_SHA256,
                )

    def test_regalloc_hook_matches_authenticated_varinfo_compiler_site(self) -> None:
        hook = MODULE.HOOK_BY_ID["regalloc"]
        self.assertEqual(hook["address"], VARINFO.ASSIGN_LOCAL_FPR)
        self.assertEqual(
            bytes.fromhex(str(hook["prefix"])),
            VARINFO.EXPECTED_HOOK_BYTES[VARINFO.ASSIGN_LOCAL_FPR],
        )

    def test_causal_map_joins_sealed_source_span_to_physical_stack_and_call_return(self) -> None:
        source = (
            b"void mbCapListDebug(void) {\n"
            b"    int local_gpr;\n"
            b"    float local_fpr;\n"
            b"    local_gpr = helper();\n"
            b"    local_fpr = (float)local_gpr;\n"
            b"}\n"
        )
        with TemporaryDirectory() as directory:
            # Canonical source-span paths may contain a long hex-looking
            # directory component; that is not serialized pointer material.
            root = Path(directory) / "f67b0aa584c94b3b98b5ac13b4433886"
            root.mkdir()
            request_path, envelope_path, envelope, trust = prepared_capture(root, source_data=source)
            context = envelope["context"]
            start = source.index(b"local_gpr", source.index(b"int "))
            end = start + len(b"local_gpr")
            manifest = MODULE.seal_source_span_manifest({
                "schema": MODULE.SOURCE_SPAN_SCHEMA,
                "function": context["function"],
                "function_sha256": context["function_sha256"],
                "session_id": context["session_id"],
                "source": context["source"],
                "spans": [{
                    "object_token": envelope["inventory"]["locals"][0]["token"],
                    "identity": "local_gpr",
                    "role": "declaration",
                    "byte_start": start,
                    "byte_end": end,
                    "line_start": 2,
                    "line_end": 2,
                    "text_sha256": hashlib.sha256(source[start:end]).hexdigest(),
                }],
                "authority_advanced": False,
            })
            manifest_path = root / "source-spans.json"
            manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")

            report = MODULE.build_source_aware_causal_map(
                envelope_path,
                manifest_path,
                trust_root=trust,
            )

            joined = {row["identity"]: row for row in report["joined_objects"]}
            self.assertEqual(report["schema"], MODULE.CAUSAL_MAP_SCHEMA)
            self.assertEqual(report["status"], "CAPTURED")
            self.assertFalse(report["authority_advanced"])
            self.assertEqual(joined["local_gpr"]["status"], "MATCHED_AUTHENTICATED")
            self.assertEqual(joined["local_gpr"]["physical_register"]["bank"], "GPR")
            self.assertTrue(joined["local_gpr"]["stack_chronology"])
            self.assertEqual(joined["local_gpr"]["call_return_chronology"][0]["callee"], "helper")
            self.assertEqual(joined["local_fpr"]["status"], "UNKNOWN")
            self.assertEqual(report["frontend_chronology"]["status"], "UNKNOWN")
            self.assertIn("frontend chronology packet was not supplied", report["unknown"])
            self.assertEqual(report["context"]["source"]["sha256"], hashlib.sha256(source).hexdigest())
            self.assertFalse(envelope["authority_advanced"])

    def test_causal_map_rejects_cross_function_and_cross_session_frontend_packets(self) -> None:
        source = b"void mbCapListDebug(void) { int local_gpr; }\n"
        with TemporaryDirectory() as directory:
            # Keep the historical nondeterministic failure deterministic: a
            # canonical artifact path may contain a long hex-looking directory
            # component without becoming serialized pointer material.
            root = Path(directory) / "6b12463009464b819542b94c840e686a"
            root.mkdir()
            _, envelope_path, envelope, trust = prepared_capture(root, source_data=source)
            context = envelope["context"]
            start = source.index(b"local_gpr")
            manifest = MODULE.seal_source_span_manifest({
                "schema": MODULE.SOURCE_SPAN_SCHEMA,
                "function": context["function"],
                "function_sha256": context["function_sha256"],
                "session_id": context["session_id"],
                "source": context["source"],
                "spans": [{
                    "object_token": envelope["inventory"]["locals"][0]["token"],
                    "identity": "local_gpr",
                    "role": "declaration",
                    "byte_start": start,
                    "byte_end": start + len(b"local_gpr"),
                    "line_start": 1,
                    "line_end": 1,
                    "text_sha256": hashlib.sha256(b"local_gpr").hexdigest(),
                }],
                "authority_advanced": False,
            })
            manifest_path = root / "source-spans.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

            def frontend_packet(function: str, session_id: str) -> dict[str, object]:
                chronology = MODULE._frontend_chronology
                producer = chronology.FrontendChronologySession(
                    {
                        "source_sha256": context["source"]["sha256"],
                        "compiler_sha256": context["compiler"]["sha256"],
                        "trace_sha256": "f" * 64,
                        "session_id": session_id,
                    },
                    function=function,
                )
                producer.on_process_started(
                    hook_bytes={str(row["id"]): str(row["prefix"]) for row in chronology.HOOKS}
                )
                producer.on_hook("reset")
                producer.on_hook("target_boundary", phase="entry", function=function)
                producer.on_hook("generic_insert_0", pointer=0x1000)
                producer.on_hook("bulk_object_link", object_pointers=[0x1000])
                producer.on_post_allocation_snapshot(
                    [{"pointer": 0x1000, "varinfo_pointer": 0x2000, "home_value": -32}]
                )
                return producer.on_process_exit()

            packet_path = root / "frontend.json"
            packet_path.write_text(
                json.dumps(frontend_packet("CapSelectMasuPlayer", context["session_id"])),
                encoding="utf-8",
            )
            with self.assertRaisesRegex(MODULE.Rejected, "function does not match"):
                MODULE.build_source_aware_causal_map(
                    envelope_path,
                    manifest_path,
                    trust_root=trust,
                    frontend_chronology=packet_path,
                )

            packet_path.write_text(
                json.dumps(frontend_packet(context["function"], "session-ffffffffffffffff")),
                encoding="utf-8",
            )
            with self.assertRaisesRegex(MODULE.Rejected, "session does not match"):
                MODULE.build_source_aware_causal_map(
                    envelope_path,
                    manifest_path,
                    trust_root=trust,
                    frontend_chronology=packet_path,
                )

            packet_path.write_text(
                json.dumps(frontend_packet(context["function"], context["session_id"])),
                encoding="utf-8",
            )
            report = MODULE.build_source_aware_causal_map(
                envelope_path,
                manifest_path,
                trust_root=trust,
                frontend_chronology=packet_path,
            )
            self.assertEqual(report["frontend_chronology"]["status"], "CAPTURED_UNKNOWN_OWNERSHIP")

    def test_causal_map_rejects_cross_token_span_and_tampered_source_digest(self) -> None:
        source = b"void mbCapListDebug(void) { int local_gpr; float local_fpr; }\n"
        with TemporaryDirectory() as directory:
            root = Path(directory)
            _, envelope_path, envelope, trust = prepared_capture(root, source_data=source)
            context = envelope["context"]
            first = source.index(b"local_gpr")
            second = source.index(b"local_fpr")
            spans = []
            for index, (identity, start) in enumerate((("local_gpr", first), ("local_fpr", second))):
                token = envelope["inventory"]["locals"][index]["token"]
                spans.append({
                    "object_token": token,
                    "identity": identity,
                    "role": "declaration",
                    "byte_start": first,
                    "byte_end": first + len(b"local_gpr"),
                    "line_start": 1,
                    "line_end": 1,
                    "text_sha256": hashlib.sha256(source[first:first + len(b"local_gpr")]).hexdigest(),
                })
            manifest = MODULE.seal_source_span_manifest({
                "schema": MODULE.SOURCE_SPAN_SCHEMA,
                "function": context["function"],
                "function_sha256": context["function_sha256"],
                "session_id": context["session_id"],
                "source": context["source"],
                "spans": spans,
                "authority_advanced": False,
            })
            manifest_path = root / "source-spans.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "identity does not match|multiple Object tokens|does not contain"):
                MODULE.build_source_aware_causal_map(envelope_path, manifest_path, trust_root=trust)

            spans[1] = dict(spans[1], byte_start=second, byte_end=second + len(b"local_fpr"), text_sha256="0" * 64)
            manifest["spans"] = spans
            manifest = MODULE.seal_source_span_manifest(manifest)
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "text digest mismatch"):
                MODULE.build_source_aware_causal_map(envelope_path, manifest_path, trust_root=trust)

    def test_one_bus_interleaves_lanes_and_rearms_each_write_once(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            backend = FakeBackend()
            envelope = MODULE.CombinedCaptureSession(auth(root), backend).run()
            events = envelope["events"]
            self.assertEqual([event["sequence"] for event in events], list(range(len(events))))
            self.assertEqual(envelope["context"]["process_id"], 4321)
            self.assertEqual([step[2] for step in backend.steps], [False] * len(MODULE.HOOKS))
            self.assertEqual(len([event for event in events if event["event_kind"] == "object_stack_write_pre"]), 3)
            self.assertEqual(len([event for event in events if event["event_kind"] == "object_stack_write_post"]), 3)
            self.assertTrue(any(event["lane"] == "pcode" for event in events))
            physical = [event for event in events if event["event_kind"] == "physical_reg_assignment"]
            self.assertEqual(physical, [{
                "schema": MODULE.EVENT_SCHEMA,
                "event_id": physical[0]["event_id"],
                "sequence": physical[0]["sequence"],
                "lane": "pcode",
                "event_kind": "physical_reg_assignment",
                "session_id": physical[0]["session_id"],
                "process_id": 4321,
                "function": "mbCapListDebug",
                "object_token": "local-session-0000000000000001-000000",
                "status": "EXACT",
                "physical_reg": 17,
                "bank": "GPR",
            }])
            self.assertNotIn("vreg_id", physical[0])
            self.assertEqual(envelope["lanes"]["stack"]["event_count"] + envelope["lanes"]["pcode"]["event_count"], len(events))

    def test_tokens_and_no_raw_pointer_leakage(self) -> None:
        with TemporaryDirectory() as directory:
            envelope = MODULE.CombinedCaptureSession(auth(Path(directory)), FakeBackend()).run()
            serialized = json.dumps(envelope, sort_keys=True)
            for forbidden in ("object_pointer", "raw_pointer", "ig_node", "thread_id", "raw_address"):
                self.assertNotIn(forbidden, serialized)
            inventory = envelope["inventory"]
            self.assertEqual(len(inventory["locals"]), 2)
            self.assertEqual(len(inventory["arguments"]), 2)
            ownership = [row["ownership"] for key in ("locals", "arguments") for row in inventory[key]]
            self.assertEqual({row["bank"] for row in ownership}, {"GPR", "FPR"})
            self.assertEqual({row["status"] for row in ownership}, {"EXACT"})

    def test_duplicate_object_and_duplicate_vreg_are_unknown(self) -> None:
        with TemporaryDirectory() as directory:
            duplicate_object = MODULE.CombinedCaptureSession(auth(Path(directory)), FakeBackend(duplicate_object=True)).run()
            self.assertEqual(duplicate_object["inventory"]["status"], "UNKNOWN")
            self.assertIn("duplicate object identity", duplicate_object["unknown"])
        with TemporaryDirectory() as directory:
            duplicate_vreg = MODULE.CombinedCaptureSession(auth(Path(directory)), FakeBackend(duplicate_vreg=True)).run()
            rows = duplicate_vreg["inventory"]["arguments"]
            self.assertTrue(any(row["ownership"]["status"] == "UNKNOWN" for row in rows))
            self.assertIn("one-to-many vreg-to-object claim", duplicate_vreg["unknown"])

    def test_regalloc_token_is_canonicalized_from_object_pointer(self) -> None:
        with TemporaryDirectory() as directory:
            envelope = MODULE.CombinedCaptureSession(
                auth(Path(directory)), FakeBackend(token_collision=True)
            ).run()
        rows = [
            event
            for event in envelope["events"]
            if event["event_kind"] == "regalloc_assignment" and event.get("status") == "EXACT"
        ]
        local = next(event for event in rows if event["vreg_id"] == "r32")
        self.assertEqual(local["object_token"], "local-session-0000000000000001-000000")
        inventory_row = next(row for row in envelope["inventory"]["locals"] if row["name"] == "local_gpr")
        self.assertEqual(inventory_row["token"], local["object_token"])
        self.assertEqual(inventory_row["ownership"]["status"], "EXACT")

    def test_dispatcher_conflicting_union_fails_before_mutation(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            backend = FakeBackend()
            duplicate = [dict(row) for row in MODULE.HOOKS]
            duplicate[-1]["address"] = duplicate[0]["address"]
            with self.assertRaisesRegex(MODULE.Rejected, "pinned hook union|duplicate"):
                MODULE.SharedBreakpointDispatcher(backend, object(), duplicate)  # type: ignore[arg-type]
            self.assertEqual(backend.installs, [])

    def test_prefix_mismatch_has_no_breakpoint_mutation(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            backend = FakeBackend()
            original = backend.read_image

            def wrong(address: int, size: int) -> bytes:
                if address == int(MODULE.HOOK_BY_ID["regalloc"]["address"]):
                    return b"\x00" * size
                return original(address, size)

            backend.read_image = wrong  # type: ignore[method-assign]
            with self.assertRaisesRegex(MODULE.Rejected, "prefix mismatch"):
                MODULE.CombinedCaptureSession(auth(root), backend).run()
            self.assertEqual(backend.installs, [])

    def test_cleanup_and_request_rewrite_remove_partial_outputs(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            for name, data in (
                ("capsule.c", b"s"), ("mwcceppc.exe", b"c"), ("authority.bin", b"a"),
                ("wrapper.bin", b"w"), ("debugger.bin", b"d"), ("transport.bin", b"t"),
            ):
                (root / name).write_bytes(data)
            request_path = MODULE.prepare_request(
                {
                    "function": "mbCapListDebug",
                    "function_sha256": "a" * 64,
                    "argv": ["wrapper.bin", "mwcceppc.exe", "-c", str(root / "capsule.c")],
                    "cwd": str(root),
                    "source": str(root / "capsule.c"),
                    "compiler": str(root / "mwcceppc.exe"),
                    "wrapper": str(root / "wrapper.bin"),
                    "debugger": str(root / "debugger.bin"),
                    "transport": str(root / "transport.bin"),
                    "authority": str(root / "authority.bin"),
                    "session_id": "session-0000000000000002",
                },
                root / "capture",
            )
            self.assertTrue(request_path.exists())
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(request_path, require_empty=True, external_trust_root=trust_root)
            backend = FakeBackend(rewrite=request_path)
            with self.assertRaisesRegex(MODULE.Rejected, "request changed"):
                MODULE.capture_with_backend(request_path, backend, external_trust_root=trust_root)
            self.assertTrue(backend.closed)
            self.assertFalse((root / "capture" / "same-session.envelope.json").exists())

    def test_envelope_is_duplicate_key_safe_and_deterministic(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            first = MODULE.CombinedCaptureSession(auth(root), FakeBackend()).run()
            second = MODULE.CombinedCaptureSession(auth(root), FakeBackend()).run()
            self.assertEqual(first, second)
            path = root / "envelope.json"
            path.write_text(json.dumps(first, sort_keys=True), encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "external trust root"):
                MODULE.validate_envelope(path)
            with self.assertRaisesRegex(MODULE.Rejected, "duplicate JSON key"):
                MODULE.strict_json_loads('{"schema":"a","schema":"b"}', "fixture")

    def test_capture_with_backend_publishes_both_lanes_atomically(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            for name, data in (
                ("capsule.c", b"s"), ("mwcceppc.exe", b"c"), ("authority.bin", b"a"),
                ("wrapper.bin", b"w"), ("debugger.bin", b"d"), ("transport.bin", b"t"),
            ):
                (root / name).write_bytes(data)
            request_path = MODULE.prepare_request(
                {
                    "function": "mbCapListDebug",
                    "function_sha256": "a" * 64,
                    "argv": ["wrapper.bin", "mwcceppc.exe", "-c", str(root / "capsule.c")],
                    "cwd": str(root),
                    "source": str(root / "capsule.c"),
                    "compiler": str(root / "mwcceppc.exe"),
                    "wrapper": str(root / "wrapper.bin"),
                    "debugger": str(root / "debugger.bin"),
                    "transport": str(root / "transport.bin"),
                    "authority": str(root / "authority.bin"),
                    "session_id": "session-0000000000000003",
                },
                root / "capture",
            )
            trust_root = trust_root_for_request(request_path)
            envelope = MODULE.capture_with_backend(request_path, FakeBackend(), external_trust_root=trust_root)
            self.assertTrue((root / "capture" / "stack.events.jsonl").exists())
            self.assertTrue((root / "capture" / "pcode.events.jsonl").exists())
            self.assertEqual(
                MODULE.validate_envelope(
                    root / "capture" / "same-session.envelope.json",
                    external_trust_root=trust_root_for_request(request_path, include_outputs=True),
                ),
                envelope,
            )

    def test_constructor_session_id_cannot_override_authenticated_request(self) -> None:
        with TemporaryDirectory() as directory:
            with self.assertRaisesRegex(MODULE.Rejected, "session ID mismatch"):
                MODULE.CombinedCaptureSession(
                    auth(Path(directory)),
                    FakeBackend(),
                    session_id="session-00000000000000ff",
                )

    def test_resealed_empty_and_reversed_chronology_are_rejected(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, _ = prepared_capture(root)
            envelope["events"] = []
            root_anchor = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "chronology is empty"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, _ = prepared_capture(root)
            envelope["events"] = list(reversed(envelope["events"]))
            root_anchor = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "chronology"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

    def test_orphan_and_duplicate_token_claims_are_rejected_after_resealing(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, _ = prepared_capture(root)
            writes = [event for event in envelope["events"] if event["event_kind"] == "object_stack_write_pre"]
            target = writes[0]
            target["object_token"] = "local-session-ffffffffffffffff-000000"
            post = next(event for event in envelope["events"] if event["event_kind"] == "object_stack_write_post" and event["hook_id"] == target["hook_id"])
            post["object_token"] = target["object_token"]
            root_anchor = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "orphan token|provenance"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, _ = prepared_capture(root)
            assignments = [event for event in envelope["events"] if event["event_kind"] == "regalloc_assignment" and event["status"] == "EXACT"]
            assignments[1]["object_token"] = assignments[0]["object_token"]
            root_anchor = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "duplicate token"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

    def test_tampered_stream_and_extra_partial_file_are_rejected(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, root_anchor = prepared_capture(root)
            stream = root / "capture" / "stack.events.jsonl"
            stream.write_bytes(stream.read_bytes() + b"tampered\n")
            with self.assertRaisesRegex(MODULE.Rejected, "output event_stream_stack bytes"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, root_anchor = prepared_capture(root)
            (root / "capture" / "pcode.events.jsonl.partial").write_text("partial", encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "extra or partial"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

    def test_empty_pcode_requires_explicit_unknown_lane_edge(self) -> None:
        class NoPCode(FakeBackend):
            def run(self, session: MODULE.CombinedCaptureSession) -> None:
                hook_by_id = {str(row["id"]): row for row in session.auth["hooks"]}
                session.on_process_started(self.process_id)
                for hook_id in ("function_filter", "allocation_pre", "allocation_post"):
                    session.on_breakpoint(int(hook_by_id[hook_id]["address"]), 1)
                    session.on_single_step(1)
                for hook_id in (
                    str(row["id"])
                    for row in session.auth["hooks"]
                    if row["role"] == "object_stack_write"
                ):
                    session.on_breakpoint(int(hook_by_id[hook_id]["address"]), 1)
                    session.on_single_step(1)
                session.on_process_exit(0)

        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, root_anchor = prepared_capture(root, backend=NoPCode())
            self.assertTrue(any(event["lane"] == "pcode" and event["event_kind"] == "lane_unknown" for event in envelope["events"]))
            envelope["events"] = [event for event in envelope["events"] if event["event_kind"] != "lane_unknown"]
            root_anchor = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "PCode chronology"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

    def test_cross_session_token_provenance_is_rejected(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, _ = prepared_capture(root, session_id="session-000000000000000a")
            other_root = root / "other"
            other_root.mkdir()
            _, other_envelope_path, other_envelope, _ = prepared_capture(other_root, session_id="session-000000000000000b")
            foreign_token = next(row["token"] for row in other_envelope["inventory"]["locals"])
            for event in envelope["events"]:
                for key in ("object_token",):
                    if event.get(key, "").startswith("local-"):
                        event[key] = foreign_token
            for row in envelope["inventory"]["locals"]:
                row["token"] = foreign_token
            root_anchor = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "provenance"):
                MODULE.validate_envelope(envelope_path, external_trust_root=root_anchor)

    def test_request_requires_explicit_tools_and_anchored_source_operand(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ("capsule.c", "mwcceppc.exe", "authority.bin"):
                (root / name).write_bytes(name.encode("ascii"))
            with self.assertRaisesRegex(MODULE.Rejected, "wrapper identity"):
                MODULE.prepare_request(
                    {
                        "function": "mbCapListDebug",
                        "function_sha256": "a" * 64,
                        "argv": ["mwcceppc.exe", "-c", str(root / "capsule.c")],
                        "cwd": str(root),
                        "source": str(root / "capsule.c"),
                        "compiler": str(root / "mwcceppc.exe"),
                        "authority": str(root / "authority.bin"),
                    },
                    root / "missing-tools",
                )
            for name in ("wrapper.bin", "debugger.bin", "transport.bin"):
                (root / name).write_bytes(name.encode("ascii"))
            with self.assertRaisesRegex(MODULE.Rejected, "-c operand"):
                MODULE.prepare_request(
                    {
                        "function": "mbCapListDebug",
                        "function_sha256": "a" * 64,
                        "argv": ["wrapper.bin", "mwcceppc.exe", "-c", "other.c"],
                        "cwd": str(root),
                        "source": str(root / "capsule.c"),
                        "compiler": str(root / "mwcceppc.exe"),
                        "wrapper": str(root / "wrapper.bin"),
                        "debugger": str(root / "debugger.bin"),
                        "transport": str(root / "transport.bin"),
                        "authority": str(root / "authority.bin"),
                    },
                    root / "bad-source",
                )

    def test_nextget_functions_are_supported_with_authenticated_wrapper_shape(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "capsule.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            authority = root / "authority.bin"
            for path, data in (
                (source, b"capsule source\n"),
                (compiler, b"compiler image\n"),
                (wrapper, b"sjis wrapper\n"),
                (authority, b"authority\n"),
            ):
                path.write_bytes(data)
            manifest = {
                "function_sha256": "a" * 64,
                "argv": [str(wrapper), str(compiler), "-c", str(source)],
                "cwd": str(root),
                "source": str(source),
                "compiler": str(compiler),
                "wrapper": str(wrapper),
                "debugger": str(authority),
                "transport": str(authority),
                "authority": str(authority),
            }
            for function in ("mbCapMasuNextGet", "CapShopNextGet", "CapEffThrowMasu"):
                capture_dir = root / function
                request_path = MODULE.prepare_request(
                    {**manifest, "function": function},
                    capture_dir,
                )
                request = MODULE.strict_json_loads(request_path.read_text(encoding="utf-8"), "request")
                self.assertEqual(request["function"], function)
                self.assertEqual(request["argv"], manifest["argv"])
                authenticated = MODULE.authenticate_request(
                    request_path,
                    require_empty=True,
                    external_trust_root=trust_root_for_request(request_path),
                )
                self.assertEqual(authenticated["request"]["function"], function)

            with self.assertRaisesRegex(MODULE.Rejected, "unsupported target function"):
                MODULE.prepare_request(
                    {**manifest, "function": "CapEffThrowMasuUnsupported"},
                    root / "unsupported",
                )

    def test_player_functions_are_supported_only_with_their_authenticated_hash(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "player.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            authority = root / "authority.bin"
            for path, data in (
                (source, b"player source\n"),
                (compiler, b"compiler image\n"),
                (wrapper, b"sjis wrapper\n"),
                (authority, b"authority\n"),
            ):
                path.write_bytes(data)
            manifest = {
                "function_sha256": "c" * 64,
                "argv": [str(wrapper), str(compiler), "-c", str(source)],
                "cwd": str(root),
                "source": str(source),
                "compiler": str(compiler),
                "wrapper": str(wrapper),
                "debugger": str(authority),
                "transport": str(authority),
                "authority": str(authority),
            }
            player_functions = (
                "mbPlayerMoveMain",
                "MoveNumOMExec",
                "mbev_PlayerColBall",
                "GetBiriQEffectRadius",
                "MetalEffectCreate",
            )
            for function in player_functions:
                request_path = MODULE.prepare_request(
                    {**manifest, "function": function},
                    root / function,
                )
                request = MODULE.strict_json_loads(request_path.read_text(encoding="utf-8"), "request")
                trust_root = trust_root_for_request(request_path)
                authenticated = MODULE.authenticate_request(
                    request_path,
                    require_empty=True,
                    external_trust_root=trust_root,
                )
                self.assertEqual(authenticated["request"]["function"], function)
                self.assertEqual(authenticated["request"]["function_sha256"], "c" * 64)
                self.assertEqual(request["source"]["sha256"], hashlib.sha256(source.read_bytes()).hexdigest())

            with self.assertRaisesRegex(MODULE.Rejected, "unsupported target function"):
                MODULE.prepare_request(
                    {**manifest, "function": "PlayerUnreviewedFunction"},
                    root / "unsupported-player",
                )

    def test_player_request_keeps_function_hash_binding_in_external_root(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "player.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            authority = root / "authority.bin"
            for path in (source, compiler, wrapper, authority):
                path.write_bytes(path.name.encode("ascii"))
            request_path = MODULE.prepare_request(
                {
                    "function": "MetalEffectCreate",
                    "function_sha256": "d" * 64,
                    "argv": [str(wrapper), str(compiler), "-c", str(source)],
                    "cwd": str(root),
                    "source": str(source),
                    "compiler": str(compiler),
                    "wrapper": str(wrapper),
                    "debugger": str(authority),
                    "transport": str(authority),
                    "authority": str(authority),
                },
                root / "capture",
            )
            trust_values = trust_root_for_request(request_path)
            mismatched_root_values = {
                field: getattr(trust_values, field)
                for field in MODULE.ExternalTrustRoot.FIELDS
            }
            mismatched_root_values["function"] = "mbPlayerMoveMain"
            with self.assertRaisesRegex(MODULE.Rejected, "function does not match request"):
                MODULE.authenticate_request(
                    request_path,
                    external_trust_root=MODULE.ExternalTrustRoot(**mismatched_root_values),
                )

    def test_native_launch_preserves_wrapper_and_compiler_order(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "capsule.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            for path in (source, compiler, wrapper):
                path.write_bytes(path.name.encode("ascii"))
            request = {
                "argv": [str(wrapper), str(compiler), "-c", str(source)],
                "cwd": str(root),
                "source": descriptor(source),
                "compiler": descriptor(compiler),
                "wrapper": descriptor(wrapper),
            }
            self.assertEqual(MODULE._native_launch_argv(request), request["argv"])
            direct = {
                "argv": [str(compiler), "-c", str(source)],
                "cwd": str(root),
                "source": descriptor(source),
                "compiler": descriptor(compiler),
            }
            self.assertEqual(
                MODULE._validate_compile_argv(
                    direct["argv"],
                    cwd=direct["cwd"],
                    source=direct["source"],
                    compiler=direct["compiler"],
                    wrapper=None,
                ),
                direct["argv"],
            )
            bad = dict(request, argv=[str(compiler), str(compiler), "-c", str(source)])
            with self.assertRaisesRegex(MODULE.Rejected, "authenticated wrapper"):
                MODULE._native_launch_argv(bad)

    def test_live_compile_preflight_rejects_missing_include_root_and_seals_present_tree(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "capsule.c"
            compiler = root / "mwcceppc.exe"
            include = root / "generated-include"
            source.write_bytes(b"int capsule;\n")
            compiler.write_bytes(b"compiler")
            include.mkdir()
            (include / "version.h").write_bytes(b"#define VERSION 0\n")
            argv = [str(compiler), "-i", str(include), "-c", str(source)]
            self.assertEqual(
                MODULE._validate_compile_argv(
                    argv,
                    cwd=str(root),
                    source=descriptor(source),
                    compiler=descriptor(compiler),
                    require_include_paths=True,
                ),
                argv,
            )
            tree = MODULE._directory_tree_descriptor(include)
            self.assertEqual(tree["file_count"], 1)
            self.assertEqual(tree["path"], str(include))
            missing_argv = [str(compiler), "-i", str(root / "missing"), "-c", str(source)]
            with self.assertRaisesRegex(MODULE.Rejected, "include path is missing"):
                MODULE._validate_compile_argv(
                    missing_argv,
                    cwd=str(root),
                    source=descriptor(source),
                    compiler=descriptor(compiler),
                    require_include_paths=True,
                )

    def test_gc27_direct_transport_is_exact_ascii_argv_derivation(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "player.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            source.write_bytes(b"int player;\n")
            compiler.write_bytes(b"compiler")
            wrapper.write_bytes(b"wrapper")
            request = {
                "argv": [str(wrapper), str(compiler), "-O0,p", "-c", str(source)],
                "cwd": str(root),
                "source": descriptor(source),
                "compiler": {
                    **descriptor(compiler),
                    "sha256": MODULE.GC27_COMPILER_SHA256,
                },
                "wrapper": {
                    **descriptor(wrapper),
                    "sha256": MODULE.SJISWRAP_V111_SHA256,
                },
            }

            actual_hash = MODULE.sha256

            def authenticated_hash(path: Path) -> str:
                selected = Path(path)
                if selected == wrapper:
                    return MODULE.SJISWRAP_V111_SHA256
                if selected == compiler:
                    return MODULE.GC27_COMPILER_SHA256
                return actual_hash(selected)

            with (
                mock.patch.object(MODULE, "sha256", side_effect=authenticated_hash),
                mock.patch.object(MODULE, "_validate_authenticated_compiler_hook_image"),
            ):
                self.assertEqual(
                    MODULE._authenticated_direct_compiler_argv(request),
                    request["argv"][1:],
                )
                self.assertEqual(
                    MODULE._native_transport_plan(request),
                    ("wrapper_memexec", request["argv"]),
                )

    def test_gc27_direct_transport_rejects_unowned_or_changed_context(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "player.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            source.write_bytes(b"int player;\n")
            compiler.write_bytes(b"compiler")
            wrapper.write_bytes(b"wrapper")
            request = {
                "argv": [str(wrapper), str(compiler), "-c", str(source)],
                "cwd": str(root),
                "source": descriptor(source),
                "compiler": {**descriptor(compiler), "sha256": MODULE.GC27_COMPILER_SHA256},
                "wrapper": {**descriptor(wrapper), "sha256": MODULE.SJISWRAP_V111_SHA256},
            }
            actual_hash = MODULE.sha256

            def authenticated_hash(path: Path) -> str:
                selected = Path(path)
                if selected == wrapper:
                    return MODULE.SJISWRAP_V111_SHA256
                if selected == compiler:
                    return MODULE.GC27_COMPILER_SHA256
                return actual_hash(selected)

            wrong_pair = {
                **request,
                "compiler": {**request["compiler"], "sha256": "a" * 64},
            }
            with self.assertRaisesRegex(MODULE.Rejected, "not authorized"):
                MODULE._authenticated_direct_compiler_argv(wrong_pair)
            self.assertEqual(
                MODULE._native_transport_plan(wrong_pair),
                ("wrapper_memexec", wrong_pair["argv"]),
            )
            wrong_wrapper = {
                **request,
                "wrapper": {**request["wrapper"], "sha256": "b" * 64},
            }
            with self.assertRaisesRegex(MODULE.Rejected, "not authorized"):
                MODULE._authenticated_direct_compiler_argv(wrong_wrapper)
            altered_argv = {
                **request,
                "argv": [str(wrapper), str(root / "other.exe"), "-c", str(source)],
            }
            with self.assertRaisesRegex(MODULE.Rejected, "compiler identity"):
                MODULE._authenticated_direct_compiler_argv(altered_argv)
            non_ascii_argv = {
                **request,
                "argv": [*request["argv"], "-DNAME=\u00e9"],
            }
            with mock.patch.object(MODULE, "sha256", side_effect=authenticated_hash):
                with self.assertRaisesRegex(MODULE.Rejected, "ASCII-equivalent cwd and argv"):
                    MODULE._authenticated_direct_compiler_argv(non_ascii_argv)
                non_ascii_cwd = root / "\u00e9"
                non_ascii_cwd.mkdir()
                with self.assertRaisesRegex(MODULE.Rejected, "ASCII-equivalent cwd and argv"):
                    MODULE._authenticated_direct_compiler_argv(
                        {**request, "cwd": str(non_ascii_cwd)}
                    )
                with self.assertRaisesRegex(MODULE.Rejected, "observed wrapper map"):
                    MODULE._authenticated_direct_compiler_argv(
                        request,
                        observed_wrapper_map=True,
                    )

    def test_gc27_direct_transport_rejects_non_ascii_source_bytes(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "player.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            source.write_bytes(b"/* \x80 */\n")
            compiler.write_bytes(b"compiler")
            wrapper.write_bytes(b"wrapper")
            request = {
                "argv": [str(wrapper), str(compiler), "-c", str(source)],
                "cwd": str(root),
                "source": descriptor(source),
                "compiler": {**descriptor(compiler), "sha256": MODULE.GC27_COMPILER_SHA256},
                "wrapper": {**descriptor(wrapper), "sha256": MODULE.SJISWRAP_V111_SHA256},
            }
            actual_hash = MODULE.sha256

            def authenticated_hash(path: Path) -> str:
                selected = Path(path)
                if selected == wrapper:
                    return MODULE.SJISWRAP_V111_SHA256
                if selected == compiler:
                    return MODULE.GC27_COMPILER_SHA256
                return actual_hash(selected)

            with mock.patch.object(MODULE, "sha256", side_effect=authenticated_hash):
                with self.assertRaisesRegex(MODULE.Rejected, "ASCII-equivalent source bytes"):
                    MODULE._authenticated_direct_compiler_argv(request)

    def test_direct_backend_authenticates_same_pid_compiler_and_seals_execution(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            compiler.write_bytes(b"compiler")
            wrapper.write_bytes(b"wrapper")
            original = [str(wrapper), str(compiler), "-c", "player.c"]
            executed = original[1:]

            class Native:
                kernel32 = object()

            backend = MODULE.NativeWow64Backend(
                Native(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=MODULE.GC27_COMPILER_SHA256,
                wrapper_path=str(wrapper),
                wrapper_sha256=MODULE.SJISWRAP_V111_SHA256,
                transport_mode="authenticated_direct_compiler",
                executed_argv=executed,
            )
            with mock.patch.object(MODULE, "sha256", return_value=MODULE.GC27_COMPILER_SHA256):
                self.assertEqual(backend.classify_debug_image(100, str(compiler)), "compiler")
            self.assertEqual(
                backend.transport_provenance(original),
                {
                    "mode": "authenticated_direct_compiler",
                    "argv": executed,
                    "request_argv_sha256": MODULE.canonical_hash({"argv": original}),
                    "wrapper_bypassed": True,
                },
            )
            with self.assertRaisesRegex(MODULE.Rejected, "diverged"):
                backend.transport_provenance([*original, "-changed"])

    def test_direct_backend_selects_only_same_pid_compiler_create_event(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            compiler = root / "mwcceppc.exe"
            wrapper = root / "sjiswrap.exe"
            compiler.write_bytes(b"compiler")
            wrapper.write_bytes(b"wrapper")
            original = [str(wrapper), str(compiler), "-c", "player.c"]

            class Kernel32:
                @staticmethod
                def CloseHandle(handle: object) -> bool:
                    del handle
                    return True

            class Native:
                kernel32 = Kernel32()

            class Info:
                hProcess = 21
                hThread = 22
                lpBaseOfImage = 0x00400000
                hFile = 0

            class Session:
                def __init__(self) -> None:
                    self.process_ids: list[int] = []

                def on_process_started(self, process_id: int) -> None:
                    self.process_ids.append(process_id)

            def backend() -> MODULE.NativeWow64Backend:
                return MODULE.NativeWow64Backend(
                    Native(),
                    11,
                    12,
                    100,
                    compiler_path=str(compiler),
                    compiler_sha256=MODULE.GC27_COMPILER_SHA256,
                    wrapper_path=str(wrapper),
                    wrapper_sha256=MODULE.SJISWRAP_V111_SHA256,
                    transport_mode="authenticated_direct_compiler",
                    executed_argv=original[1:],
                )

            selected = backend()
            session = Session()
            with mock.patch.object(
                MODULE,
                "sha256",
                return_value=MODULE.GC27_COMPILER_SHA256,
            ):
                selected._select_compiler_process(
                    100,
                    7,
                    Info(),
                    str(compiler),
                    session,
                )
            self.assertTrue(selected.compiler_selected)
            self.assertEqual(selected.compiler_process_id, 100)
            self.assertEqual(selected._selection_mode, "authenticated_direct_compiler")
            self.assertEqual(selected.loader_breakpoints_remaining, 2)
            self.assertEqual(session.process_ids, [100])

            changed_pid = backend()
            with self.assertRaisesRegex(MODULE.Rejected, "changed process identity"):
                changed_pid._select_compiler_process(
                    101,
                    7,
                    Info(),
                    str(compiler),
                    Session(),
                )

    def test_direct_backend_consumes_only_two_out_of_image_init_breakpoints(self) -> None:
        backend = MODULE.NativeWow64Backend(
            object(),
            11,
            12,
            100,
            transport_mode="authenticated_direct_compiler",
            executed_argv=["mwcceppc.exe", "-c", "player.c"],
        )
        backend.base = MODULE.KNOWN_IMAGE_BASE
        backend._compiler_image_size = 0x00200000
        backend.loader_breakpoints_remaining = 2
        backend.loader_breakpoint_pending = True

        self.assertTrue(backend._consume_initial_system_breakpoint(0x77AF87F8))
        self.assertEqual(backend.loader_breakpoints_remaining, 1)
        self.assertTrue(backend.loader_breakpoint_pending)

    def test_owned_hook_closes_initial_breakpoint_budget_and_later_noise_rejects(self) -> None:
        backend = MODULE.NativeWow64Backend(
            object(),
            11,
            12,
            100,
            transport_mode="authenticated_direct_compiler",
            executed_argv=["mwcceppc.exe", "-c", "player.c"],
        )
        backend.base = MODULE.KNOWN_IMAGE_BASE
        backend._compiler_image_size = 0x00200000
        backend.loader_breakpoints_remaining = 2
        backend.loader_breakpoint_pending = True
        hook = int(MODULE.HOOK_BY_ID["function_filter"]["address"])
        session = mock.Mock()
        session.dispatcher.by_address = {hook: object()}

        backend._handle_breakpoint_exception(session, 0x77AF87F8, 7, 100)
        self.assertEqual(backend.loader_breakpoints_remaining, 1)
        session.on_breakpoint.assert_not_called()
        backend._handle_breakpoint_exception(session, hook, 7, 100)
        self.assertEqual(backend.loader_breakpoints_remaining, 0)
        session.on_breakpoint.assert_called_once_with(hook, 7, 100)
        with self.assertRaisesRegex(MODULE.Rejected, "unexpected non-loader breakpoint"):
            backend._handle_breakpoint_exception(session, 0x76FE1234, 7, 100)

        backend.loader_breakpoints_remaining = 1
        backend.loader_breakpoint_pending = True
        with self.assertRaisesRegex(MODULE.Rejected, "unexpected non-loader breakpoint"):
            backend._handle_breakpoint_exception(
                session,
                MODULE.KNOWN_IMAGE_BASE + 0x1000,
                7,
                100,
            )
        self.assertTrue(backend._consume_initial_system_breakpoint(0x76FE1234))
        self.assertEqual(backend.loader_breakpoints_remaining, 0)
        self.assertFalse(backend.loader_breakpoint_pending)
        self.assertFalse(backend._consume_initial_system_breakpoint(0x77AF87F8))

        backend.loader_breakpoints_remaining = 1
        backend.loader_breakpoint_pending = True
        self.assertFalse(
            backend._consume_initial_system_breakpoint(MODULE.KNOWN_IMAGE_BASE + 0x1000)
        )
        self.assertEqual(backend.loader_breakpoints_remaining, 1)
        self.assertTrue(backend.loader_breakpoint_pending)

    def test_later_pid_change_is_rejected(self) -> None:
        class WrongPID(FakeBackend):
            def run(self, session: MODULE.CombinedCaptureSession) -> None:
                session.on_process_started(self.process_id)
                session.on_breakpoint(int(MODULE.HOOK_BY_ID["function_filter"]["address"]), 1, self.process_id + 1)

        with TemporaryDirectory() as directory:
            with self.assertRaisesRegex(MODULE.Rejected, "debug loop ended|process id"):
                MODULE.CombinedCaptureSession(auth(Path(directory)), WrongPID()).run()


class Gc27PcodeColorCrosswalkTests(unittest.TestCase):
    SESSION_ID = "session-0000000000000001"

    @staticmethod
    def _raw(
        *,
        object_pointer: int = 0x1010,
        operand_index: int = 17,
        pcode_pointer: int = 0x12345678,
        ig_pointer: int = 0x87654321,
        operand_ordinal: int = 1,
        operand_count: int = 3,
        register_class: int = 4,
        final_color: int = 17,
    ) -> dict[str, object]:
        return {
            "pcode_pointer": pcode_pointer,
            "ig_pointer": ig_pointer,
            "operand_ordinal": operand_ordinal,
            "operand_count": operand_count,
            "operand_kind": 2,
            "register_class": register_class,
            "operand_index": operand_index,
            "final_color": final_color,
            "ig_flags": 0,
            "object_pointer": object_pointer,
        }

    def _session(self, root: Path, backend: FakeBackend) -> MODULE.CombinedCaptureSession:
        authority = auth(root)
        session = MODULE.CombinedCaptureSession(authority, backend)
        session.auth["request"]["compiler"]["sha256"] = MODULE.GC27_COMPILER_SHA256
        session.auth["hooks"] = [dict(row) for row in MODULE.GC27_HOOKS]
        session.ledger.register("local", 0x1010)
        return session

    def test_same_pcode_pointer_uses_one_token_across_color_and_machine_without_raw_addresses(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            raw = self._raw()
            backend.capture_pcode = mock.Mock(side_effect=[
                {"status": "PENDING", **raw},
                {"status": "CAPTURED", **raw},
            ])
            word = MachineEmissionDecoderTests._addi(17, 1, 8)
            backend.capture_machine_emission = mock.Mock(return_value={
                "pcode_pointer": raw["pcode_pointer"],
                "emitted_offset": 12,
                "opcode_enum": 1,
                "encoded_value": MachineEmissionDecoderTests._encoded(word),
                "descriptor_base": word & 0xFC000000,
            })
            session = self._session(Path(directory), backend)
            self.assertIsNone(session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[0], 7))
            color = session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[1], 7)
            machine = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
        assert color is not None
        self.assertEqual(color["pcode_token"], machine["pcode_token"])
        self.assertEqual(color["object_token"], f"local-{self.SESSION_ID}-000000")
        self.assertEqual(color["operand_index"], 17)
        self.assertNotIn("vreg_id", color)
        self.assertEqual(machine["owner_joins"], [])
        self.assertEqual(machine["physical_owner_joins"], [{
            "physical_register": "r17",
            "object_token": f"local-{self.SESSION_ID}-000000",
        }])
        serialized = json.dumps(color, sort_keys=True)
        self.assertNotIn("0x12345678", serialized)
        self.assertNotIn("305419896", serialized)
        self.assertNotIn("2271560481", serialized)

    def test_arithmetic_machine_event_joins_same_pcode_fpr_objects(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            multiply_pointer = 0x12345678
            left = self._raw(
                object_pointer=0x1010,
                operand_index=30,
                pcode_pointer=multiply_pointer,
                ig_pointer=0x87654321,
                operand_ordinal=0,
                operand_count=2,
                register_class=3,
                final_color=30,
            )
            right = self._raw(
                object_pointer=0x1020,
                operand_index=31,
                pcode_pointer=multiply_pointer,
                ig_pointer=0x87654322,
                operand_ordinal=1,
                operand_count=2,
                register_class=3,
                final_color=31,
            )
            backend.capture_pcode = mock.Mock(side_effect=[
                {"status": "PENDING", **left},
                {"status": "CAPTURED", **left},
                {"status": "PENDING", **right},
                {"status": "CAPTURED", **right},
            ])
            lfs_left = MachineEmissionDecoderTests._d(48, 30, 1, 0)
            lfs_right = MachineEmissionDecoderTests._d(48, 31, 1, 4)
            fmuls = (59 << 26) | (28 << 21) | (30 << 16) | (31 << 6) | (25 << 1)
            backend.capture_machine_emission = mock.Mock(side_effect=[
                {
                    "pcode_pointer": 0x11111111,
                    "emitted_offset": 0,
                    "opcode_enum": 1,
                    "encoded_value": MachineEmissionDecoderTests._encoded(lfs_left),
                    "descriptor_base": lfs_left & 0xFC000000,
                },
                {
                    "pcode_pointer": 0x22222222,
                    "emitted_offset": 4,
                    "opcode_enum": 1,
                    "encoded_value": MachineEmissionDecoderTests._encoded(lfs_right),
                    "descriptor_base": lfs_right & 0xFC000000,
                },
                {
                    "pcode_pointer": multiply_pointer,
                    "emitted_offset": 8,
                    "opcode_enum": 1,
                    "encoded_value": MachineEmissionDecoderTests._encoded(fmuls),
                    "descriptor_base": fmuls & 0xFC000000,
                },
            ])
            session = self._session(Path(directory), backend)
            session.ledger.register("local", 0x1020)
            for _row in (left, right):
                self.assertIsNone(session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[0], 7))
                self.assertIsNotNone(session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[1], 7))
            session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
            session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
            arithmetic = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)

        self.assertEqual(arithmetic["mnemonic"], "fmuls")
        self.assertEqual(arithmetic["arithmetic_type"], "f32")
        self.assertEqual(arithmetic["owner_joins"], [])
        self.assertEqual(arithmetic["physical_owner_joins"], [
            {
                "physical_register": "f30",
                "object_token": f"local-{self.SESSION_ID}-000000",
            },
            {
                "physical_register": "f31",
                "object_token": f"local-{self.SESSION_ID}-000001",
            },
        ])
        serialized = json.dumps(arithmetic, sort_keys=True)
        self.assertNotIn("0x12345678", serialized)
        self.assertNotIn(str(multiply_pointer), serialized)

    def test_hidden_owner_and_nonregister_post_noop_are_explicit_and_nonpoisoning(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            hidden = self._raw(object_pointer=0)
            backend.capture_pcode = mock.Mock(side_effect=[
                {"status": "PENDING", **hidden},
                {"status": "CAPTURED", **hidden},
                {"status": "NOOP"},
            ])
            session = self._session(Path(directory), backend)
            session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[0], 9)
            row = session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[1], 9)
            noop = session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[1], 10)
        assert row is not None
        self.assertRegex(row["hidden_owner_token"], MODULE.HIDDEN_IG_TOKEN_RE)
        self.assertNotIn("object_token", row)
        self.assertIsNone(noop)
        self.assertEqual(session.unknown, [])

    def test_conflicts_duplicates_stale_tokens_and_operand_index_as_vreg_fail_closed(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            raw = self._raw()
            backend.capture_pcode = mock.Mock(side_effect=[
                {"status": "PENDING", **raw}, {"status": "CAPTURED", **raw},
                {"status": "PENDING", **raw}, {"status": "CAPTURED", **raw},
            ])
            session = self._session(Path(directory), backend)
            session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[0], 7)
            row = session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[1], 7)
            session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[0], 7)
            with self.assertRaisesRegex(MODULE.Rejected, "duplicate PCode color evidence"):
                session._capture_pcode_color(MODULE.GC27_PCODE_COLOR_HOOKS[1], 7)
            token = str(row["pcode_token"])
            self.assertEqual(session._bind_pcode_offset(token, 4), token)
            self.assertIsNone(session._bind_pcode_offset(token, 8))
            self.assertIsNone(session._bind_pcode_offset(f"pcode-session-ffffffffffffffff-000000", 4))

            session.bus.bind_process(1)
            event = session.bus.emit("pcode", "pcode_capture", row)
            context = {
                "session_id": self.SESSION_ID,
                "process_id": 1,
                "function": "mbCapListDebug",
                "compiler": {"sha256": MODULE.GC27_COMPILER_SHA256},
            }
            MODULE._validate_event(event, 0, context)
            stale = dict(event, ig_token="ig-session-ffffffffffffffff-000000")
            with self.assertRaisesRegex(MODULE.Rejected, "token provenance is invalid"):
                MODULE._validate_event(stale, 0, context)
            event["vreg_id"] = "r17"
            with self.assertRaisesRegex(MODULE.Rejected, "payload is not closed"):
                MODULE._validate_event(event, 0, context)

    def test_gc27_empty_regalloc_is_chronology_only_but_legacy_remains_unknown(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            backend.capture_regalloc = mock.Mock(return_value=[])
            session = self._session(Path(directory), backend)
            session.bus.bind_process(1)
            session.function_entered = True
            session.on_hook(MODULE.HOOK_BY_ID["regalloc"], 7)
            self.assertFalse(any(event["event_kind"] == "regalloc_assignment" for event in session.bus.events))
            self.assertNotIn("incomplete regalloc", session.unknown)

            legacy_backend = FakeBackend()
            legacy_backend.capture_regalloc = mock.Mock(return_value=[])
            legacy = MODULE.CombinedCaptureSession(auth(Path(directory)), legacy_backend)
            legacy.bus.bind_process(2)
            legacy.function_entered = True
            legacy.on_hook(MODULE.HOOK_BY_ID["regalloc"], 7)
            self.assertIn("incomplete regalloc", legacy.unknown)
            self.assertTrue(any(event["event_kind"] == "regalloc_assignment" for event in legacy.bus.events))

    def test_gc27_completion_does_not_relabel_chronology_only_regalloc_as_missing(self) -> None:
        with TemporaryDirectory() as directory:
            session = self._session(Path(directory), FakeBackend())
            session.bus.bind_process(1)
            for hook_id in ("function_filter", "allocation_pre", "allocation_post", *MODULE.WRITE_HOOK_IDS):
                session.bus.emit(
                    "stack",
                    "function_entry" if hook_id == "function_filter" else (
                        "numeric_stack_alloc_pre" if hook_id == "allocation_pre" else (
                            "numeric_stack_alloc_post" if hook_id == "allocation_post" else "object_stack_write_post"
                        )
                    ),
                    {"hook_id": hook_id, **(
                        {"locals": [], "arguments": []} if hook_id in {"allocation_pre", "allocation_post"} else (
                            {"object_token": "UNKNOWN", "target_slot": 0, "write_observed": True}
                            if hook_id in MODULE.WRITE_HOOK_IDS else {}
                        )
                    )},
                )
            session.bus.emit(
                "pcode",
                "physical_reg_assignment",
                {"status": "UNKNOWN", "reason": "incomplete physical register evidence"},
            )
            session.bus.emit(
                "pcode",
                "machine_emission",
                {
                    "hook_id": "gc27_machine_emit",
                    "status": "UNKNOWN",
                    "reason": "unsupported machine opcode",
                },
            )
            session._ensure_lane_completion()
            self.assertFalse(any(event["event_kind"] == "lane_unknown" for event in session.bus.events))
            self.assertNotIn("incomplete PCode evidence", session.unknown)

    def test_unexecuted_alternate_stack_write_sites_are_not_missing_evidence(self) -> None:
        with TemporaryDirectory() as directory:
            session = self._session(Path(directory), FakeBackend())
            session.bus.bind_process(1)
            session.bus.emit("stack", "function_entry", {"hook_id": "function_filter"})
            session.bus.emit(
                "stack",
                "numeric_stack_alloc_pre",
                {"hook_id": "allocation_pre", "locals": [], "arguments": []},
            )
            session.bus.emit(
                "stack",
                "numeric_stack_alloc_post",
                {"hook_id": "allocation_post", "locals": [], "arguments": []},
            )
            session.bus.emit(
                "stack",
                "object_stack_write_pre",
                {"hook_id": "object_write_0", "object_token": "UNKNOWN", "target_slot": 0},
            )
            session.bus.emit(
                "stack",
                "object_stack_write_post",
                {
                    "hook_id": "object_write_0",
                    "object_token": "UNKNOWN",
                    "target_slot": 0,
                    "write_observed": True,
                },
            )
            session.bus.emit(
                "pcode",
                "physical_reg_assignment",
                {"status": "UNKNOWN", "reason": "incomplete physical register evidence"},
            )
            session.bus.emit(
                "pcode",
                "machine_emission",
                {
                    "hook_id": "gc27_machine_emit",
                    "status": "UNKNOWN",
                    "reason": "unsupported machine opcode",
                },
            )
            session._ensure_lane_completion()
            self.assertFalse(any(event["event_kind"] == "lane_unknown" for event in session.bus.events))
            self.assertNotIn("incomplete stack evidence", session.unknown)


class MachineEmissionDecoderTests(unittest.TestCase):
    SESSION_ID = "session-0000000000000001"

    @staticmethod
    def _encoded(word: int) -> int:
        return int.from_bytes(word.to_bytes(4, "big"), "little")

    @staticmethod
    def _addi(destination: int, base: int, immediate: int) -> int:
        return (14 << 26) | (destination << 21) | (base << 16) | (immediate & 0xFFFF)

    @staticmethod
    def _d(primary: int, data: int, base: int, displacement: int) -> int:
        return (primary << 26) | (data << 21) | (base << 16) | (displacement & 0xFFFF)

    @staticmethod
    def _psq(primary: int, data: int, base: int, displacement: int, *, single: int = 0, quantization: int = 0) -> int:
        return (
            (primary << 26)
            | (data << 21)
            | (base << 16)
            | ((single & 1) << 15)
            | ((quantization & 7) << 12)
            | (displacement & 0xFFF)
        )

    @staticmethod
    def _psqx(*, load: bool, data: int, base: int, index: int, single: int = 0, quantization: int = 0) -> int:
        return (
            (4 << 26)
            | (data << 21)
            | (base << 16)
            | (index << 11)
            | ((single & 1) << 10)
            | ((quantization & 7) << 7)
            | (0x0C if load else 0x0E)
        )

    def _decode(self, decoder: MODULE.MachineEmissionDecoder, index: int, word: int, *, ordinal: int | None = None) -> dict[str, object]:
        return decoder.decode(
            pcode_token=f"pcode-{self.SESSION_ID}-{(index if ordinal is None else ordinal):06d}",
            emitted_offset=index * 4,
            opcode_enum=0x100 + (index % 32),
            encoded_value=self._encoded(word),
            descriptor_base=word & 0xFC000000,
        )

    def test_legacy_envelope_rejects_resealed_gc27_machine_event(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path, envelope_path, envelope, _ = prepared_capture(root)
            exit_event = envelope["events"][-1]
            envelope["events"].insert(-1, {
                "schema": MODULE.EVENT_SCHEMA,
                "event_id": "placeholder",
                "session_id": exit_event["session_id"],
                "process_id": exit_event["process_id"],
                "function": exit_event["function"],
                "lane": "pcode",
                "sequence": 0,
                "event_kind": "machine_emission",
                "hook_id": "gc27_machine_emit",
                "status": "UNKNOWN",
                "reason": "incomplete machine emission evidence",
            })
            envelope["unknown"] = sorted({
                *envelope["unknown"],
                "incomplete machine emission evidence",
            })
            trust = reseal_capture(envelope_path, request_path, envelope)
            with self.assertRaisesRegex(MODULE.Rejected, "not owned by the compiler profile"):
                MODULE.validate_envelope(envelope_path, trust_root=trust)

    def test_move_num_indices_117_122_bind_stack_effects_and_lifetime_edges(self) -> None:
        decoder = MODULE.MachineEmissionDecoder()
        words = (
            self._addi(30, 1, 0x14),
            self._addi(31, 1, 0x08),
            self._psq(56, 30, 31, 0),
            self._d(48, 31, 31, 8),
            self._psq(60, 30, 30, 0),
            self._d(52, 31, 30, 8),
        )
        rows = [self._decode(decoder, index, word) for index, word in enumerate(words, 117)]
        self.assertTrue(all(row["status"] == "CAPTURED" for row in rows))
        self.assertEqual(
            [row.get("address_definition") for row in rows[:2]],
            [
                {"register": "r30", "stack_offset": 0x14},
                {"register": "r31", "stack_offset": 0x08},
            ],
        )
        self.assertEqual(
            [(row.get("memory_op"), row.get("effective_stack_offset"), row.get("memory_width")) for row in rows[2:]],
            [("load", 0x08, 8), ("load", 0x10, 4), ("store", 0x14, 8), ("store", 0x1C, 4)],
        )
        self.assertEqual(
            [row["reaching_definitions"] for row in rows],
            [[], [], [118], [118], [117, 119], [117, 120]],
        )

    def test_machine_decoder_fails_closed_on_ambiguous_or_unsupported_evidence(self) -> None:
        token = f"pcode-{self.SESSION_ID}-000000"
        word = self._addi(30, 1, 0x14)
        missing = MODULE.MachineEmissionDecoder().decode(
            pcode_token="UNKNOWN",
            emitted_offset=0,
            opcode_enum=1,
            encoded_value=self._encoded(word),
            descriptor_base=word & 0xFC000000,
        )
        self.assertEqual(missing, {"status": "UNKNOWN", "reason": "missing PCode token"})

        duplicate = MODULE.MachineEmissionDecoder()
        self.assertEqual(self._decode(duplicate, 1, word)["status"], "CAPTURED")
        self.assertEqual(self._decode(duplicate, 1, word)["reason"], "ambiguous PCode token")

        cases = (
            (self._d(48, 31, 30, 0), "ambiguous reaching definition"),
            (self._psq(56, 30, 1, 0, quantization=1), "quantized PSQ is unsupported"),
            (self._psqx(load=True, data=30, base=1, index=31), "indexed base is nonzero"),
            (18 << 26, "unsupported machine opcode"),
        )
        for case_index, (case_word, reason) in enumerate(cases):
            decoder = MODULE.MachineEmissionDecoder()
            row = decoder.decode(
                pcode_token=token,
                emitted_offset=case_index * 4,
                opcode_enum=case_index,
                encoded_value=self._encoded(case_word),
                descriptor_base=case_word & 0xFC000000,
            )
            self.assertEqual(row["status"], "UNKNOWN", reason)
            self.assertEqual(row["reason"], reason, reason)
            self.assertEqual(
                {key: row[key] for key in (
                    "pcode_token", "emitted_offset", "instruction_index",
                    "opcode_enum", "ppc_word", "ppc_bytes",
                )},
                {
                    "pcode_token": token,
                    "emitted_offset": case_index * 4,
                    "instruction_index": case_index,
                    "opcode_enum": case_index,
                    "ppc_word": case_word,
                    "ppc_bytes": self._encoded(case_word).to_bytes(4, "little").hex(),
                },
                reason,
            )

        mismatch = MODULE.MachineEmissionDecoder().decode(
            pcode_token=token,
            emitted_offset=0,
            opcode_enum=1,
            encoded_value=self._encoded(word),
            descriptor_base=0,
        )
        self.assertEqual(mismatch["status"], "UNKNOWN")
        self.assertEqual(mismatch["reason"], "descriptor opcode mismatch")
        self.assertEqual(mismatch["pcode_token"], token)
        self.assertEqual(mismatch["instruction_index"], 0)

    def test_machine_decoder_bl_clears_only_caller_volatile_definitions(self) -> None:
        decoder = MODULE.MachineEmissionDecoder()
        address = self._addi(30, 1, 0x14)
        volatile_address = self._addi(3, 1, 0x08)
        branch = (18 << 26) | 1
        surviving_load = self._d(48, 31, 30, 0)
        clobbered_load = self._d(48, 31, 3, 0)

        self.assertEqual(self._decode(decoder, 0, address)["status"], "CAPTURED")
        self.assertEqual(self._decode(decoder, 1, volatile_address)["status"], "CAPTURED")
        call = self._decode(decoder, 2, branch)
        self.assertEqual(call["mnemonic"], "bl")
        self.assertEqual(call["registers"], {})
        self.assertEqual(call["reaching_definitions"], [])

        surviving = self._decode(decoder, 3, surviving_load)
        self.assertEqual(surviving["status"], "CAPTURED")
        self.assertEqual(surviving["effective_stack_offset"], 0x14)
        self.assertEqual(surviving["reaching_definitions"], [0])

        clobbered = self._decode(decoder, 4, clobbered_load)
        self.assertEqual(clobbered, {
            "status": "UNKNOWN",
            "reason": "ambiguous reaching definition",
            "pcode_token": f"pcode-{self.SESSION_ID}-000004",
            "emitted_offset": 16,
            "instruction_index": 4,
            "opcode_enum": 0x104,
            "ppc_word": clobbered_load,
            "ppc_bytes": self._encoded(clobbered_load).to_bytes(4, "little").hex(),
        })

    def test_machine_decoder_fneg_requires_and_tracks_fpr_reaching_definition(self) -> None:
        decoder = MODULE.MachineEmissionDecoder()
        source_load = self._d(48, 30, 1, 0)
        fneg = (63 << 26) | (31 << 21) | (30 << 11) | (40 << 1)
        source = self._decode(decoder, 0, source_load)
        self.assertEqual(source["status"], "CAPTURED")
        row = self._decode(decoder, 1, fneg)
        self.assertEqual(row["status"], "CAPTURED")
        self.assertEqual(row["mnemonic"], "fneg")
        self.assertEqual(row["registers"], {"destination": "f31", "source": "f30"})
        self.assertEqual(row["reaching_definitions"], [0])

        missing = MODULE.MachineEmissionDecoder().decode(
            pcode_token=f"pcode-{self.SESSION_ID}-000000",
            emitted_offset=0,
            opcode_enum=1,
            encoded_value=self._encoded(fneg),
            descriptor_base=fneg & 0xFC000000,
        )
        self.assertEqual(missing["status"], "UNKNOWN")
        self.assertEqual(missing["reason"], "ambiguous reaching definition")

    def test_machine_decoder_tracks_fmuls_operands_result_and_type(self) -> None:
        decoder = MODULE.MachineEmissionDecoder()
        self.assertEqual(self._decode(decoder, 0, self._d(48, 30, 1, 0))["status"], "CAPTURED")
        self.assertEqual(self._decode(decoder, 1, self._d(48, 31, 1, 4))["status"], "CAPTURED")
        fmuls = (59 << 26) | (28 << 21) | (30 << 16) | (31 << 6) | (25 << 1)
        row = self._decode(decoder, 2, fmuls)
        self.assertEqual(row["status"], "CAPTURED")
        self.assertEqual(row["mnemonic"], "fmuls")
        self.assertEqual(
            row["registers"],
            {"destination": "f28", "source_a": "f30", "source_b": "f31"},
        )
        self.assertEqual(row["arithmetic_op"], "multiply")
        self.assertEqual(row["arithmetic_type"], "f32")
        self.assertEqual(row["reaching_definitions"], [0, 1])

        missing = MODULE.MachineEmissionDecoder().decode(
            pcode_token=f"pcode-{self.SESSION_ID}-000000",
            emitted_offset=0,
            opcode_enum=1,
            encoded_value=self._encoded(fmuls),
            descriptor_base=fmuls & 0xFC000000,
        )
        self.assertEqual(missing["status"], "UNKNOWN")
        self.assertEqual(missing["reason"], "ambiguous reaching definition")
        self.assertEqual(missing["mnemonic"], "fmuls")
        self.assertEqual(missing["known_reaching_definitions"], [])
        self.assertEqual(missing["missing_reaching_registers"], ["f30", "f31"])

        partial = MODULE.MachineEmissionDecoder()
        self.assertEqual(
            self._decode(partial, 0, self._d(48, 0, 1, 0))["status"],
            "CAPTURED",
        )
        residual = (59 << 26) | (0 << 21) | (31 << 16) | (0 << 6) | (25 << 1)
        unresolved = self._decode(partial, 1, residual)
        self.assertEqual(unresolved["status"], "UNKNOWN")
        self.assertEqual(
            unresolved["registers"],
            {"destination": "f0", "source_a": "f31", "source_b": "f0"},
        )
        self.assertEqual(
            unresolved["known_reaching_definitions"],
            [{"physical_register": "f0", "instruction_index": 0}],
        )
        self.assertEqual(unresolved["missing_reaching_registers"], ["f31"])

    def test_machine_decoder_tracks_ps_mul_operands_result_and_type(self) -> None:
        decoder = MODULE.MachineEmissionDecoder()
        self.assertEqual(self._decode(decoder, 0, self._d(48, 28, 1, 0))["status"], "CAPTURED")
        self.assertEqual(self._decode(decoder, 1, self._d(48, 29, 1, 4))["status"], "CAPTURED")
        ps_mul = 0x139C0772  # ps_mul f28,f28,f29
        row = self._decode(decoder, 2, ps_mul)
        self.assertEqual(row["status"], "CAPTURED")
        self.assertEqual(row["mnemonic"], "ps_mul")
        self.assertEqual(
            row["registers"],
            {"destination": "f28", "source_a": "f28", "source_b": "f29"},
        )
        self.assertEqual(row["arithmetic_op"], "multiply")
        self.assertEqual(row["arithmetic_type"], "paired-single")
        self.assertEqual(row["reaching_definitions"], [0, 1])

        second = 0x13DE07F2  # ps_mul f30,f30,f31
        decoded = MODULE.MachineEmissionDecoder()
        self.assertEqual(self._decode(decoded, 0, self._d(48, 30, 1, 0))["status"], "CAPTURED")
        self.assertEqual(self._decode(decoded, 1, self._d(48, 31, 1, 4))["status"], "CAPTURED")
        self.assertEqual(self._decode(decoded, 2, second)["registers"]["destination"], "f30")

    def test_machine_arithmetic_event_schema_accepts_complete_effect_only(self) -> None:
        decoder = MODULE.MachineEmissionDecoder()
        self.assertEqual(self._decode(decoder, 0, self._d(48, 30, 1, 0))["status"], "CAPTURED")
        self.assertEqual(self._decode(decoder, 1, self._d(48, 31, 1, 4))["status"], "CAPTURED")
        fmuls = (59 << 26) | (28 << 21) | (30 << 16) | (31 << 6) | (25 << 1)
        row = {
            "hook_id": "gc27_machine_emit",
            **self._decode(decoder, 2, fmuls),
            "owner_joins": [],
        }
        with TemporaryDirectory() as directory:
            session = MODULE.CombinedCaptureSession(auth(Path(directory)), FakeBackend())
            session.bus.bind_process(1)
            event = session.bus.emit("pcode", "machine_emission", row)
        context = {
            "session_id": self.SESSION_ID,
            "process_id": 1,
            "function": "mbCapListDebug",
            "compiler": {"sha256": MODULE.GC27_COMPILER_SHA256},
        }
        MODULE._validate_event(event, 0, context)
        self.assertEqual(
            event["operand_role_order"],
            ["destination", "source_a", "source_b"],
        )

        incomplete = dict(event)
        del incomplete["arithmetic_type"]
        with self.assertRaisesRegex(MODULE.Rejected, "arithmetic effect is incomplete"):
            MODULE._validate_event(incomplete, 0, context)
        invalid = dict(event, arithmetic_type="f64")
        with self.assertRaisesRegex(MODULE.Rejected, "arithmetic effect is invalid"):
            MODULE._validate_event(invalid, 0, context)
        reordered = dict(
            event,
            operand_role_order=["destination", "source_b", "source_a"],
        )
        with self.assertRaisesRegex(MODULE.Rejected, "operand role order is invalid"):
            MODULE._validate_event(reordered, 0, context)

    def test_machine_event_joins_existing_physical_owner_without_serializing_pointer(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            word = self._psq(56, 30, 1, 8)
            backend.capture_machine_emission = mock.Mock(return_value={
                "pcode_pointer": 0x12345678,
                "emitted_offset": 117 * 4,
                "opcode_enum": 0x193,
                "encoded_value": self._encoded(word),
                "descriptor_base": word & 0xFC000000,
            })
            session = MODULE.CombinedCaptureSession(auth(Path(directory)), backend)
            token = f"local-{self.SESSION_ID}-000000"
            session.mappings[token] = {"status": "EXACT", "vreg_id": "f32", "bank": "FPR"}
            session.physical_mappings[token] = {
                "status": "EXACT", "physical_reg": 30, "bank": "FPR",
            }
            session.physical_reg_owners[("FPR", 30)] = token
            row = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
            backend.capture_machine_emission.return_value = {
                **backend.capture_machine_emission.return_value,
                "pcode_pointer": 0x87654321,
            }
            ambiguous = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
        self.assertEqual(row["status"], "CAPTURED")
        self.assertEqual(row["owner_joins"], [{
            "physical_register": "f30",
            "object_token": token,
            "vreg_id": "f32",
        }])
        self.assertEqual(row["physical_owner_joins"], [{
            "physical_register": "f30",
            "object_token": token,
        }])
        self.assertEqual(row["effective_stack_offset"], 8)
        self.assertNotIn("pcode_pointer", row)
        self.assertNotIn("0x12345678", json.dumps(row, sort_keys=True))
        self.assertEqual(ambiguous, {
            "hook_id": "gc27_machine_emit",
            "status": "UNKNOWN",
            "reason": "ambiguous PCode token",
        })

    def test_incomplete_machine_event_invalidates_all_reaching_definitions(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            addi = self._addi(30, 1, 0x14)
            dependent_lfs = self._d(48, 31, 30, 0)
            backend.capture_machine_emission = mock.Mock(side_effect=[
                {
                    "pcode_pointer": 0x1000,
                    "emitted_offset": 117 * 4,
                    "opcode_enum": 1,
                    "encoded_value": self._encoded(addi),
                    "descriptor_base": addi & 0xFC000000,
                },
                {},
                {
                    "pcode_pointer": 0x3000,
                    "emitted_offset": 119 * 4,
                    "opcode_enum": 3,
                    "encoded_value": self._encoded(dependent_lfs),
                    "descriptor_base": dependent_lfs & 0xFC000000,
                },
            ])
            session = MODULE.CombinedCaptureSession(auth(Path(directory)), backend)
            valid = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
            incomplete = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
            dependent = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
        self.assertEqual(valid["status"], "CAPTURED")
        self.assertEqual(incomplete["reason"], "incomplete machine emission evidence")
        self.assertEqual(dependent["hook_id"], "gc27_machine_emit")
        self.assertEqual(dependent["status"], "UNKNOWN")
        self.assertEqual(dependent["reason"], "ambiguous reaching definition")
        self.assertEqual(dependent["emitted_offset"], 119 * 4)
        self.assertEqual(dependent["instruction_index"], 119)
        self.assertEqual(dependent["ppc_word"], dependent_lfs)
        self.assertEqual(dependent["ppc_bytes"], self._encoded(dependent_lfs).to_bytes(4, "little").hex())

    def test_machine_owner_join_rejects_cross_bank_mapping(self) -> None:
        with TemporaryDirectory() as directory:
            backend = FakeBackend()
            word = self._psq(56, 30, 1, 8)
            backend.capture_machine_emission = mock.Mock(return_value={
                "pcode_pointer": 0x12345678,
                "emitted_offset": 117 * 4,
                "opcode_enum": 0x193,
                "encoded_value": self._encoded(word),
                "descriptor_base": word & 0xFC000000,
            })
            session = MODULE.CombinedCaptureSession(auth(Path(directory)), backend)
            token = f"local-{self.SESSION_ID}-000000"
            session.mappings[token] = {"status": "EXACT", "vreg_id": "r32", "bank": "GPR"}
            session.physical_mappings[token] = {
                "status": "EXACT", "physical_reg": 30, "bank": "FPR",
            }
            session.physical_reg_owners[("FPR", 30)] = token
            row = session._capture_machine_emission(MODULE.GC27_MACHINE_EMIT_HOOK, 7)
        self.assertEqual(row, {
            "hook_id": "gc27_machine_emit",
            "status": "UNKNOWN",
            "reason": "machine owner register-bank mismatch",
        })

    def test_native_machine_capture_reads_authenticated_ebx_ebp_eax_contract(self) -> None:
        backend = MODULE.NativeWow64Backend(object(), 1, MODULE.KNOWN_IMAGE_BASE, 4321)
        backend.compiler_sha256 = MODULE.GC27_COMPILER_SHA256
        word = self._psq(56, 30, 31, 0)
        encoded = self._encoded(word)
        backend.read_register = mock.Mock(side_effect=lambda _thread, name: {
            "ebx": 0x12340000,
            "ebp": 117 * 4,
            "eax": encoded,
        }[name])
        descriptor_address = (
            MODULE.GC27_OPCODE_DESCRIPTOR_TABLE
            + 0x193 * MODULE.GC27_OPCODE_DESCRIPTOR_STRIDE
            + MODULE.GC27_OPCODE_DESCRIPTOR_BASE_OFFSET
        )

        def read(address: int, size: int) -> bytes:
            if (address, size) == (0x12340020, 2):
                return (0x193).to_bytes(2, "little")
            if (address, size) == (descriptor_address, 4):
                return (word & 0xFC000000).to_bytes(4, "little")
            self.fail(f"unexpected native read 0x{address:x}/{size}")

        backend._read = mock.Mock(side_effect=read)
        self.assertEqual(
            backend.capture_machine_emission("gc27_machine_emit", 7),
            {
                "pcode_pointer": 0x12340000,
                "emitted_offset": 117 * 4,
                "opcode_enum": 0x193,
                "encoded_value": encoded,
                "descriptor_base": word & 0xFC000000,
            },
        )

    def test_gc27_end_to_end_machine_events_join_causal_map_and_fail_closed(self) -> None:
        source_data = (
            b"void mbCapListDebug(void) {\n"
            b"    float local_fpr;\n"
            b"    float arg_fpr;\n"
            b"}\n"
        )
        words = (
            self._addi(30, 1, 0x14),
            self._addi(31, 1, 0x08),
            self._psq(56, 30, 31, 0),
            self._d(48, 31, 31, 8),
            self._psq(60, 30, 30, 0),
            self._d(52, 31, 30, 8),
        )
        machine_rows = [
            {
                "pcode_pointer": 0x700000 + index * 0x10,
                "emitted_offset": index * 4,
                "opcode_enum": 0x100 + (index % 32),
                "encoded_value": self._encoded(word),
                "descriptor_base": word & 0xFC000000,
            }
            for index, word in enumerate(words, 117)
        ]
        physical_rows = [
            {
                "object": 0x1020,
                "varinfo_pointer": 0x5020,
                "noregister": 0,
                "flags": 2,
                "rclass": 3,
                "reg": 30,
                "reg_hi": 0,
            },
            {
                "object": 0x2020,
                "varinfo_pointer": 0x6020,
                "noregister": 0,
                "flags": 2,
                "rclass": 3,
                "reg": 31,
                "reg_hi": 0,
            },
        ]
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "capsule.c"
            compiler = root / "mwcceppc.exe"
            wrapper = root / "wrapper.bin"
            debugger = root / "debugger.bin"
            transport = root / "transport.bin"
            authority = root / "authority.bin"
            source.write_bytes(source_data)
            compiler.write_bytes(b"gc27 fixture")
            for path in (wrapper, debugger, transport, authority):
                path.write_bytes(path.name.encode("ascii"))
            actual_hash = MODULE.sha256

            def authenticated_hash(path: Path) -> str:
                return MODULE.GC27_COMPILER_SHA256 if Path(path).resolve() == compiler.resolve() else actual_hash(path)

            backend = FakeBackend(physical_rows=physical_rows, machine_rows=machine_rows)
            with (
                mock.patch.object(MODULE, "sha256", side_effect=authenticated_hash),
                mock.patch.object(MODULE, "_validate_authenticated_compiler_hook_image"),
            ):
                request_path = MODULE.prepare_request(
                    {
                        "function": "mbCapListDebug",
                        "function_sha256": "a" * 64,
                        "argv": [str(wrapper), str(compiler), "-c", str(source)],
                        "cwd": str(root),
                        "source": str(source),
                        "compiler": str(compiler),
                        "wrapper": str(wrapper),
                        "debugger": str(debugger),
                        "transport": str(transport),
                        "authority": str(authority),
                        "session_id": self.SESSION_ID,
                    },
                    root / "capture",
                )
                envelope = MODULE.capture_with_backend(
                    request_path,
                    backend,
                    external_trust_root=trust_root_for_request(request_path),
                )
                envelope_path = root / "capture" / "same-session.envelope.json"
                trust = trust_root_for_request(request_path, include_outputs=True)
                context = envelope["context"]
                spans = []
                for identity, token in (
                    ("local_fpr", envelope["inventory"]["locals"][1]["token"]),
                    ("arg_fpr", envelope["inventory"]["arguments"][1]["token"]),
                ):
                    start = source_data.index(identity.encode("ascii"))
                    spans.append({
                        "object_token": token,
                        "identity": identity,
                        "role": "declaration",
                        "byte_start": start,
                        "byte_end": start + len(identity),
                        "line_start": 2 if identity == "local_fpr" else 3,
                        "line_end": 2 if identity == "local_fpr" else 3,
                        "text_sha256": hashlib.sha256(identity.encode("ascii")).hexdigest(),
                    })
                manifest = MODULE.seal_source_span_manifest({
                    "schema": MODULE.SOURCE_SPAN_SCHEMA,
                    "function": context["function"],
                    "function_sha256": context["function_sha256"],
                    "session_id": context["session_id"],
                    "source": context["source"],
                    "spans": spans,
                    "authority_advanced": False,
                })
                manifest_path = root / "source-spans.json"
                manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")
                report = MODULE.build_source_aware_causal_map(
                    envelope_path,
                    manifest_path,
                    trust_root=trust,
                )
                joined = {row["identity"]: row for row in report["joined_objects"]}
                self.assertEqual(
                    [event["instruction_index"] for event in joined["local_fpr"]["machine_emission_chronology"]],
                    [119, 121],
                )
                self.assertEqual(
                    [event["instruction_index"] for event in joined["arg_fpr"]["machine_emission_chronology"]],
                    [120, 122],
                )
                self.assertEqual(joined["local_fpr"]["status"], "MATCHED_AUTHENTICATED")
                self.assertEqual(joined["arg_fpr"]["status"], "MATCHED_AUTHENTICATED")

                missing_machine_envelope = json.loads(json.dumps(envelope))
                missing_machine_envelope["events"] = [
                    event for event in missing_machine_envelope["events"]
                    if event["event_kind"] != "machine_emission"
                ]
                missing_trust = reseal_capture(
                    envelope_path,
                    request_path,
                    missing_machine_envelope,
                )
                with self.assertRaisesRegex(MODULE.Rejected, "required by the compiler profile"):
                    MODULE.validate_envelope(envelope_path, trust_root=missing_trust)
                trust = reseal_capture(envelope_path, request_path, envelope)

                for event in envelope["events"]:
                    if event["event_kind"] == "machine_emission" and event.get("status") == "CAPTURED":
                        event["owner_joins"] = [
                            join for join in event["owner_joins"]
                            if join["object_token"] != spans[0]["object_token"]
                        ]
                        event["physical_owner_joins"] = [
                            join for join in event.get("physical_owner_joins", [])
                            if join["object_token"] != spans[0]["object_token"]
                        ]
                trust = reseal_capture(envelope_path, request_path, envelope)
                missing = MODULE.build_source_aware_causal_map(
                    envelope_path,
                    manifest_path,
                    trust_root=trust,
                )
                missing_joined = {row["identity"]: row for row in missing["joined_objects"]}
                self.assertEqual(missing_joined["local_fpr"]["status"], "UNKNOWN")
                self.assertIn("authenticated machine-emission owner join is absent", missing_joined["local_fpr"]["evidence"])

                target_event = next(
                    event for event in envelope["events"]
                    if event["event_kind"] == "machine_emission"
                    and event.get("status") == "CAPTURED"
                    and event.get("owner_joins")
                )
                target_event["owner_joins"].append(dict(target_event["owner_joins"][0]))
                trust = reseal_capture(envelope_path, request_path, envelope)
                with self.assertRaisesRegex(MODULE.Rejected, "owner join is ambiguous"):
                    MODULE.build_source_aware_causal_map(
                        envelope_path,
                        manifest_path,
                        trust_root=trust,
                    )


class SourceSpanV2StackIntervalTests(unittest.TestCase):
    SESSION_ID = "session-0000000000000010"

    @staticmethod
    def _encoded(word: int) -> int:
        return int.from_bytes(word.to_bytes(4, "big"), "little")

    @staticmethod
    def _addi(destination: int, base: int, immediate: int) -> int:
        return (14 << 26) | (destination << 21) | (base << 16) | (immediate & 0xFFFF)

    @staticmethod
    def _d(primary: int, data: int, base: int, displacement: int) -> int:
        return (primary << 26) | (data << 21) | (base << 16) | (displacement & 0xFFFF)

    def _make_fixture(self, root: Path) -> dict[str, object]:
        source_data = (
            b"void mbCapListDebug(void) {\n"
            b"    HuVecF posNorm;\n"
            b"    HuVecF pos;\n"
            b"    pos = posNorm;\n"
            b"}\n"
        )
        words = (
            self._addi(28, 1, 8),
            self._d(32, 30, 28, 0),
            self._d(32, 31, 28, 4),
            self._d(32, 27, 28, 8),
            self._addi(29, 1, 20),
            self._d(36, 30, 29, 0),
            self._d(36, 31, 29, 4),
            self._d(36, 27, 29, 8),
        )
        machine_rows = [
            {
                "pcode_pointer": 0x700000 + index * 0x10,
                "emitted_offset": index * 4,
                "opcode_enum": 0x100 + index,
                "encoded_value": self._encoded(word),
                "descriptor_base": word & 0xFC000000,
            }
            for index, word in enumerate(words)
        ]
        compiler_path = root / "mwcceppc.exe"
        actual_sha256 = MODULE.sha256

        def authenticated_sha256(path: Path | str) -> str:
            if Path(path).resolve() == compiler_path.resolve():
                return MODULE.GC27_COMPILER_SHA256
            return actual_sha256(path)

        with (
            mock.patch.object(MODULE, "sha256", side_effect=authenticated_sha256),
            mock.patch.object(MODULE, "_validate_authenticated_compiler_hook_image"),
        ):
            request_path, envelope_path, envelope, _ = prepared_capture(
                root,
                backend=FakeBackend(
                    physical_rows=[{
                        "object": 0x1010,
                        "varinfo_pointer": 0x5010,
                        "noregister": 0,
                        "flags": 2,
                        "rclass": 4,
                        "reg": 17,
                        "reg_hi": 0,
                    }],
                    machine_rows=machine_rows,
                ),
                session_id=self.SESSION_ID,
                source_data=source_data,
            )

        inventory = envelope["inventory"]
        local_rows = inventory["locals"]
        token_norm = str(local_rows[0]["token"])
        token_pos = str(local_rows[1]["token"])
        argument_token = str(inventory["arguments"][0]["token"])
        local_rows[0]["name"] = "posNorm"
        local_rows[1]["name"] = "pos"

        events = envelope["events"]
        filtered_events = []
        for event in events:
            if event["event_kind"] == "regalloc_assignment" and event.get("object_token") in {token_norm, token_pos}:
                continue
            if event["event_kind"] == "physical_reg_assignment":
                event["object_token"] = argument_token
            if event["event_kind"] in {"object_stack_write_pre", "object_stack_write_post"}:
                if event["hook_id"] == "object_write_0":
                    event["object_token"] = token_norm
                    event["target_slot"] = 0
                elif event["hook_id"] == "object_write_1":
                    event["object_token"] = token_pos
                    event["target_slot"] = 12
                elif event["hook_id"] == "object_write_2":
                    event["object_token"] = argument_token
                    event["target_slot"] = 99
            filtered_events.append(event)
        events[:] = filtered_events

        for index, event in enumerate(events):
            event["sequence"] = index
            event["event_id"] = f"{self.SESSION_ID}-e{index:06d}"
        writes = {
            token_norm: [event["event_id"] for event in events if event["event_kind"] == "object_stack_write_pre" and event.get("object_token") == token_norm]
            + [event["event_id"] for event in events if event["event_kind"] == "object_stack_write_post" and event.get("object_token") == token_norm],
            token_pos: [event["event_id"] for event in events if event["event_kind"] == "object_stack_write_pre" and event.get("object_token") == token_pos]
            + [event["event_id"] for event in events if event["event_kind"] == "object_stack_write_post" and event.get("object_token") == token_pos],
        }
        local_rows[0]["ownership"] = {
            "status": "EXACT", "mode": "stack_home", "evidence_event_ids": writes[token_norm],
            "stack_home": {"base": "r1", "offset": 0},
        }
        local_rows[1]["ownership"] = {
            "status": "EXACT", "mode": "stack_home", "evidence_event_ids": writes[token_pos],
            "stack_home": {"base": "r1", "offset": 12},
        }
        reseal_capture(envelope_path, request_path, envelope)
        context = envelope["context"]
        source = source_data

        def span(token: str, identity: str, role: str, start: int, end: int, dependency_id: str | None, indices: list[int]) -> dict[str, object]:
            prefix = source[:start]
            return {
                "object_token": token,
                "identity": identity,
                "role": role,
                "byte_start": start,
                "byte_end": end,
                "line_start": prefix.count(b"\n") + 1,
                "line_end": prefix.count(b"\n") + 1,
                "text_sha256": hashlib.sha256(source[start:end]).hexdigest(),
                "dependency_id": dependency_id,
                "machine_instruction_indices": indices,
            }

        decl_norm = source.index(b"HuVecF posNorm")
        decl_pos = source.index(b"HuVecF pos;")
        read_norm = source.rindex(b"posNorm")
        write_pos = source.index(b"pos =")
        dep = "move_copy_1386"
        manifest = MODULE.seal_source_span_manifest({
            "schema": MODULE.SOURCE_SPAN_SCHEMA_V2,
            "function": context["function"],
            "function_sha256": context["function_sha256"],
            "session_id": context["session_id"],
            "source": context["source"],
            "objects": [
                {"object_token": token_norm, "identity": "posNorm", "ownership_mode": "stack_interval", "object_type": "HuVecF", "byte_size": 12},
                {"object_token": token_pos, "identity": "pos", "ownership_mode": "stack_interval", "object_type": "HuVecF", "byte_size": 12},
            ],
            "spans": [
                span(token_norm, "posNorm", "declaration", decl_norm, decl_norm + len(b"HuVecF posNorm"), None, []),
                span(token_norm, "posNorm", "read", read_norm, read_norm + len(b"posNorm"), dep, [0, 1, 2, 3]),
                span(token_pos, "pos", "declaration", decl_pos, decl_pos + len(b"HuVecF pos"), None, []),
                span(token_pos, "pos", "write", write_pos, write_pos + len(b"pos"), dep, [4, 5, 6, 7]),
            ],
            "authority_advanced": False,
        })
        manifest_path = root / "source-spans-v2.json"
        manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")
        return {
            "request_path": request_path,
            "envelope_path": envelope_path,
            "envelope": envelope,
            "trust_root": trust_root_for_request(request_path, include_outputs=True),
            "manifest_path": manifest_path,
            "manifest": manifest,
            "token_norm": token_norm,
            "token_pos": token_pos,
        }

    def _build(
        self,
        fixture: dict[str, object],
        *,
        post_capture_analysis: bool = False,
    ) -> dict[str, object]:
        compiler_path = Path(fixture["request_path"]).parent.parent / "mwcceppc.exe"
        actual_sha256 = MODULE.sha256

        def authenticated_sha256(path: Path | str) -> str:
            if Path(path).resolve() == compiler_path.resolve():
                return MODULE.GC27_COMPILER_SHA256
            return actual_sha256(path)

        with (
            mock.patch.object(MODULE, "sha256", side_effect=authenticated_sha256),
            mock.patch.object(MODULE, "_validate_authenticated_compiler_hook_image"),
        ):
            return MODULE.build_source_aware_causal_map(
                fixture["envelope_path"],
                fixture["manifest_path"],
                trust_root=fixture["trust_root"],
                post_capture_analysis=post_capture_analysis,
            )

    @staticmethod
    def _reseal_fixture(fixture: dict[str, object]) -> None:
        fixture["trust_root"] = reseal_capture(
            fixture["envelope_path"],
            fixture["request_path"],
            fixture["envelope"],
        )

    def _make_paired_fixture(self, fixture: dict[str, object]) -> None:
        """Turn the scalar fixture into a real two-load/two-store paired seam."""

        envelope = fixture["envelope"]
        paired = {
            1: (self._d(56, 30, 28, 0), "psq_l", 8),
            2: (self._d(48, 31, 28, 8), "lfs", 4),
            5: (self._d(60, 30, 29, 0), "psq_st", 8),
            6: (self._d(52, 31, 29, 8), "stfs", 4),
        }
        for event in envelope["events"]:
            index = event.get("instruction_index")
            if index not in paired:
                continue
            word, mnemonic, width = paired[index]
            event["mnemonic"] = mnemonic
            event["memory_width"] = width
            if index == 2:
                event["effective_stack_offset"] = 16
            elif index == 6:
                event["effective_stack_offset"] = 28
            event["ppc_word"] = word
            event["ppc_bytes"] = self._encoded(word).to_bytes(4, "little").hex()

        manifest = json.loads(json.dumps(fixture["manifest"]))
        for span in manifest["spans"]:
            if span["object_token"] == fixture["token_norm"] and span["role"] == "read":
                span["machine_instruction_indices"] = [0, 1, 2]
            elif span["object_token"] == fixture["token_pos"] and span["role"] == "write":
                span["machine_instruction_indices"] = [4, 5, 6]
        manifest = MODULE.seal_source_span_manifest(manifest)
        fixture["manifest"] = manifest
        fixture["manifest_path"].write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")
        self._reseal_fixture(fixture)

    def test_v2_stack_interval_scalar_copy_matches_without_register_ownership(self) -> None:
        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            report = self._build(fixture)
        joined = {row["object_token"]: row for row in report["joined_objects"]}
        self.assertEqual(joined[fixture["token_norm"]]["status"], "MATCHED_AUTHENTICATED")
        self.assertEqual(joined[fixture["token_pos"]]["status"], "MATCHED_AUTHENTICATED")
        self.assertIsNone(joined[fixture["token_norm"]]["virtual_register"])
        self.assertIsNone(joined[fixture["token_norm"]]["physical_register"])
        self.assertEqual(joined[fixture["token_norm"]]["stack_interval_dependencies"][0]["effective_interval"], {"start": 8, "end": 20, "size": 12})
        self.assertEqual(joined[fixture["token_pos"]]["stack_interval_dependencies"][0]["effective_interval"], {"start": 20, "end": 32, "size": 12})
        for token in (fixture["token_norm"], fixture["token_pos"]):
            crosswalk = joined[token]["stack_interval_dependencies"][0]["pcode_crosswalk"]
            self.assertEqual(crosswalk["status"], "MATCHED_AUTHENTICATED")
            self.assertEqual(crosswalk["ownership_edge"], "final_stack_home_to_machine_pcode")
            self.assertIsNone(crosswalk["reason"])
        self.assertEqual(len(report["stack_copy_dependencies"]), 1)
        self.assertEqual(report["stack_copy_dependencies"][0]["status"], "MATCHED_AUTHENTICATED")
        self.assertFalse(report["stack_copy_dependencies"][0]["paired_codegen_proof"])
        self.assertFalse(report["authority_advanced"])

    def test_v2_post_capture_analysis_authenticates_drifted_capture_tool(self) -> None:
        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            context = fixture["envelope"]["context"]
            Path(context["debugger"]["path"]).write_bytes(b"new analyzer bytes")
            Path(context["transport"]["path"]).write_bytes(b"new transport bytes")

            with self.assertRaisesRegex(MODULE.Rejected, "identity mismatch"):
                self._build(fixture)

            report = self._build(fixture, post_capture_analysis=True)
            self.assertEqual(
                report["capture_tool_validation"]["mode"],
                "sealed_post_capture_descriptor",
            )
            self.assertEqual(
                report["capture_tool_validation"]["debugger"],
                context["debugger"],
            )
            self.assertEqual(
                report["tools"]["same_session"]["sha256"],
                MODULE.sha256(Path(MODULE.__file__)),
            )

            no_outputs = dict(fixture)
            no_outputs["trust_root"] = trust_root_for_request(
                fixture["request_path"],
                include_outputs=False,
            )
            with self.assertRaisesRegex(
                MODULE.Rejected,
                "event_stream_stack anchor is missing",
            ):
                self._build(no_outputs, post_capture_analysis=True)

            root = fixture["trust_root"]
            bad_values = {
                field: getattr(root, field)
                for field in MODULE.ExternalTrustRoot.FIELDS
                if getattr(root, field) is not None
            }
            bad_values["debugger_sha256"] = "0" * 64
            mismatched = dict(fixture)
            mismatched["trust_root"] = MODULE.ExternalTrustRoot(**bad_values)
            with self.assertRaisesRegex(
                MODULE.Rejected,
                "external trust root.debugger does not match request",
            ):
                self._build(mismatched, post_capture_analysis=True)

    def test_v2_stack_interval_rejects_overlapping_object_claims(self) -> None:
        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            envelope = fixture["envelope"]
            machine = {
                int(event["instruction_index"]): event
                for event in envelope["events"]
                if event.get("event_kind") == "machine_emission"
            }
            addi_word = self._addi(29, 1, 8)
            machine[4]["immediate"] = 8
            machine[4]["address_definition"]["stack_offset"] = 8
            machine[4]["ppc_word"] = addi_word
            machine[4]["ppc_bytes"] = self._encoded(addi_word).to_bytes(4, "little").hex()
            for index in (5, 6, 7):
                machine[index]["effective_stack_offset"] -= 12
            pos_row = next(
                row for row in envelope["inventory"]["locals"]
                if row["token"] == fixture["token_pos"]
            )
            pos_row["ownership"]["stack_home"]["offset"] = 0
            for event in envelope["events"]:
                if (
                    event.get("event_kind")
                    in {"object_stack_write_pre", "object_stack_write_post"}
                    and event.get("object_token") == fixture["token_pos"]
                ):
                    event["target_slot"] = 0
            self._reseal_fixture(fixture)
            report = self._build(fixture)

        joined = {row["object_token"]: row for row in report["joined_objects"]}
        for token in (fixture["token_norm"], fixture["token_pos"]):
            self.assertEqual(joined[token]["status"], "UNKNOWN")
            self.assertIn("overlapping effective stack intervals", " ".join(joined[token]["evidence"]))
        self.assertEqual(report["stack_copy_dependencies"], [])

    def test_v2_paired_codegen_proof_requires_producers_bases_and_reaching_edges(self) -> None:
        mutations = ("missing addi", "wrong addi", "wrong memory base", "missing reaching edge")
        for mutation in mutations:
            with self.subTest(mutation=mutation), TemporaryDirectory() as directory:
                fixture = self._make_fixture(Path(directory))
                self._make_paired_fixture(fixture)
                events = {
                    int(event["instruction_index"]): event
                    for event in fixture["envelope"]["events"]
                    if event.get("event_kind") == "machine_emission"
                }
                if mutation == "missing addi":
                    events[0].pop("address_definition")
                elif mutation == "wrong addi":
                    events[0]["address_definition"]["stack_offset"] = 12
                elif mutation == "wrong memory base":
                    events[1]["registers"]["base"] = "r27"
                    events[2]["registers"]["base"] = "r27"
                else:
                    events[1]["reaching_definitions"] = []
                    events[2]["reaching_definitions"] = []
                self._reseal_fixture(fixture)
                report = self._build(fixture)
                dependency = report["stack_copy_dependencies"][0]
                self.assertEqual(dependency["status"], "MATCHED_AUTHENTICATED")
                self.assertFalse(dependency["paired_codegen_proof"])
                self.assertIn("paired proof requires", dependency["paired_codegen_reason"])

    def test_v2_scalar_register_physical_only_without_object_vreg_is_unknown(self) -> None:
        with TemporaryDirectory() as directory:
            # Windows temporary roots may contain a digit-leading, long
            # hexadecimal component.  It is an authenticated source-span path,
            # not serialized runtime pointer text.
            root = Path(directory) / "8ac03784e5db4fc790d31b81f0a61c29"
            root.mkdir()
            fixture = self._make_fixture(root)
            envelope = fixture["envelope"]
            token = fixture["token_norm"]
            physical = next(
                event
                for event in envelope["events"]
                if event.get("event_kind") == "physical_reg_assignment"
            )
            physical["object_token"] = token
            physical["physical_reg"] = 30
            machine = next(
                event
                for event in envelope["events"]
                if event.get("event_kind") == "machine_emission"
                and event.get("instruction_index") == 1
            )
            machine["physical_owner_joins"] = [{
                "physical_register": "r30",
                "object_token": token,
            }]
            local = next(row for row in envelope["inventory"]["locals"] if row["token"] == token)
            local["ownership"] = {
                "status": "EXACT",
                "mode": "physical_register",
                "evidence_event_ids": [physical["event_id"]],
                "physical_reg": 30,
                "bank": "GPR",
            }
            manifest = json.loads(json.dumps(fixture["manifest"]))
            manifest["objects"][0].update({
                "ownership_mode": "scalar_register",
                "object_type": None,
                "byte_size": None,
            })
            manifest = MODULE.seal_source_span_manifest(manifest)
            fixture["manifest"] = manifest
            fixture["manifest_path"].write_text(json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8")
            self._reseal_fixture(fixture)
            report = self._build(fixture)
        row = next(item for item in report["joined_objects"] if item["object_token"] == token)
        self.assertEqual(row["status"], "UNKNOWN")
        self.assertIsNone(row["virtual_register"])
        self.assertEqual(row["physical_register"]["physical_reg"], 30)
        self.assertIn("Object-to-vreg", " ".join(row["evidence"]))

    def test_v2_stack_interval_rejects_wrong_size_missing_type_and_missing_dependency(self) -> None:
        for wrong_size in (8, 16):
            with TemporaryDirectory() as directory:
                fixture = self._make_fixture(Path(directory))
                bad = json.loads(json.dumps(fixture["manifest"]))
                bad["objects"][0]["byte_size"] = wrong_size
                fixture["manifest_path"].write_text(json.dumps(MODULE.seal_source_span_manifest(bad)), encoding="utf-8")
                with self.assertRaisesRegex(MODULE.Rejected, "exact 12-byte HuVecF"):
                    self._build(fixture)

        for field, expected in (("declaration", "typed declaration"), ("dependency", "machine dependency")):
            with TemporaryDirectory() as directory:
                fixture = self._make_fixture(Path(directory))
                bad = json.loads(json.dumps(fixture["manifest"]))
                if field == "declaration":
                    bad["spans"] = [row for row in bad["spans"] if not (row["object_token"] == fixture["token_norm"] and row["role"] == "declaration")]
                else:
                    for row in bad["spans"]:
                        if row["object_token"] == fixture["token_norm"] and row["role"] == "read":
                            row["dependency_id"] = None
                            row["machine_instruction_indices"] = []
                fixture["manifest_path"].write_text(json.dumps(MODULE.seal_source_span_manifest(bad)), encoding="utf-8")
                with self.assertRaisesRegex(MODULE.Rejected, expected):
                    self._build(fixture)

    def test_v2_stack_interval_rejects_partial_and_conflicting_dependencies(self) -> None:
        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            bad = json.loads(json.dumps(fixture["manifest"]))
            for row in bad["spans"]:
                if row["object_token"] == fixture["token_norm"] and row["role"] == "read":
                    row["machine_instruction_indices"] = [0, 1]
            fixture["manifest_path"].write_text(json.dumps(MODULE.seal_source_span_manifest(bad)), encoding="utf-8")
            report = self._build(fixture)
            joined = {row["object_token"]: row for row in report["joined_objects"]}
            self.assertEqual(joined[fixture["token_norm"]]["status"], "UNKNOWN")
            self.assertEqual(joined[fixture["token_pos"]]["status"], "MATCHED_AUTHENTICATED")
            self.assertIn("does not wholly cover", " ".join(joined[fixture["token_norm"]]["evidence"]))

        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            bad = json.loads(json.dumps(fixture["manifest"]))
            declaration = next(row for row in bad["spans"] if row["object_token"] == fixture["token_norm"] and row["role"] == "declaration")
            conflicting = dict(declaration)
            conflicting["role"] = "read"
            conflicting["dependency_id"] = "move_copy_1386"
            conflicting["machine_instruction_indices"] = [0, 1]
            bad["spans"].append(conflicting)
            fixture["manifest_path"].write_text(json.dumps(MODULE.seal_source_span_manifest(bad)), encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "conflicting machine indices"):
                self._build(fixture)

    def test_v2_stack_interval_unknown_machine_is_dependency_scoped(self) -> None:
        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            envelope = fixture["envelope"]
            unrelated = next(event for event in envelope["events"] if event["event_kind"] == "function_exit")
            unknown = {
                "schema": MODULE.EVENT_SCHEMA,
                "event_id": "placeholder",
                "sequence": 0,
                "lane": "pcode",
                "event_kind": "machine_emission",
                "function": envelope["context"]["function"],
                "session_id": envelope["context"]["session_id"],
                "process_id": envelope["context"]["process_id"],
                "hook_id": "gc27_machine_emit",
                "status": "UNKNOWN",
                "reason": "unsupported machine opcode",
                "pcode_token": f"pcode-{self.SESSION_ID}-000099",
                "emitted_offset": 99 * 4,
                "instruction_index": 99,
                "opcode_enum": 0x100,
                "ppc_word": 0,
                "ppc_bytes": "00000000",
            }
            envelope["events"].insert(envelope["events"].index(unrelated), unknown)
            envelope["unknown"] = sorted({*envelope.get("unknown", []), "unsupported machine opcode"})
            reseal_capture(fixture["envelope_path"], fixture["request_path"], envelope)
            fixture["trust_root"] = trust_root_for_request(fixture["request_path"], include_outputs=True)
            report = self._build(fixture)
            joined = {row["object_token"]: row for row in report["joined_objects"]}
            self.assertEqual(joined[fixture["token_norm"]]["status"], "MATCHED_AUTHENTICATED")
            self.assertEqual(joined[fixture["token_pos"]]["status"], "MATCHED_AUTHENTICATED")

            target = next(event for event in envelope["events"] if event["event_kind"] == "machine_emission" and event.get("instruction_index") == 1)
            target.update({"status": "UNKNOWN", "reason": "unsupported machine opcode"})
            for key in ("mnemonic", "registers", "reaching_definitions", "memory_op", "memory_width", "effective_stack_offset", "immediate", "address_definition", "owner_joins", "physical_owner_joins"):
                target.pop(key, None)
            for event in envelope["events"]:
                if isinstance(event.get("reaching_definitions"), list):
                    event["reaching_definitions"] = [
                        value for value in event["reaching_definitions"] if value != 1
                    ]
            reseal_capture(fixture["envelope_path"], fixture["request_path"], envelope)
            fixture["trust_root"] = trust_root_for_request(fixture["request_path"], include_outputs=True)
            report = self._build(fixture)
            joined = {row["object_token"]: row for row in report["joined_objects"]}
            self.assertEqual(joined[fixture["token_norm"]]["status"], "UNKNOWN")
            self.assertEqual(joined[fixture["token_pos"]]["status"], "MATCHED_AUTHENTICATED")

    def test_v2_located_fneg_before_valid_seam_is_nonpoisoning(self) -> None:
        fneg_word = (63 << 26) | (31 << 21) | (30 << 11) | (40 << 1)
        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            envelope = fixture["envelope"]
            exit_event = next(event for event in envelope["events"] if event["event_kind"] == "function_exit")
            unrelated = {
                "schema": MODULE.EVENT_SCHEMA,
                "event_id": "placeholder",
                "sequence": 0,
                "lane": "pcode",
                "event_kind": "machine_emission",
                "function": envelope["context"]["function"],
                "session_id": envelope["context"]["session_id"],
                "process_id": envelope["context"]["process_id"],
                "hook_id": "gc27_machine_emit",
                "status": "UNKNOWN",
                "reason": "unsupported machine opcode",
                "pcode_token": f"pcode-{self.SESSION_ID}-000090",
                "emitted_offset": 90 * 4,
                "instruction_index": 90,
                "opcode_enum": 0x150,
                "ppc_word": fneg_word,
                "ppc_bytes": self._encoded(fneg_word).to_bytes(4, "little").hex(),
            }
            envelope["events"].insert(envelope["events"].index(exit_event), unrelated)
            envelope["unknown"] = sorted({
                *envelope.get("unknown", []),
                "unsupported machine opcode",
            })
            self._reseal_fixture(fixture)
            report = self._build(fixture)
        joined = {row["object_token"]: row for row in report["joined_objects"]}
        self.assertEqual(joined[fixture["token_norm"]]["status"], "MATCHED_AUTHENTICATED")
        self.assertEqual(joined[fixture["token_pos"]]["status"], "MATCHED_AUTHENTICATED")

        with TemporaryDirectory() as directory:
            fixture = self._make_fixture(Path(directory))
            envelope = fixture["envelope"]
            seam = next(
                event
                for event in envelope["events"]
                if event.get("event_kind") == "machine_emission"
                and event.get("instruction_index") == 1
            )
            seam.update({
                "status": "UNKNOWN",
                "reason": "unsupported machine opcode",
                "ppc_word": fneg_word,
                "ppc_bytes": self._encoded(fneg_word).to_bytes(4, "little").hex(),
            })
            for key in (
                "mnemonic", "registers", "reaching_definitions", "memory_op", "memory_width",
                "effective_stack_offset", "immediate", "address_definition", "owner_joins",
                "physical_owner_joins",
            ):
                seam.pop(key, None)
            for event in envelope["events"]:
                if isinstance(event.get("reaching_definitions"), list):
                    event["reaching_definitions"] = [
                        value for value in event["reaching_definitions"] if value != 1
                    ]
            envelope["unknown"] = sorted({
                *envelope.get("unknown", []),
                "unsupported machine opcode",
            })
            self._reseal_fixture(fixture)
            report = self._build(fixture)
        joined = {row["object_token"]: row for row in report["joined_objects"]}
        self.assertEqual(joined[fixture["token_norm"]]["status"], "UNKNOWN")
        self.assertEqual(joined[fixture["token_pos"]]["status"], "MATCHED_AUTHENTICATED")

    @staticmethod
    def _prepare_handoff_request(root: Path, output: Path, *, session_id: str) -> Path:
        for name, data in (
            ("capsule.c", b"s"), ("mwcceppc.exe", b"c"), ("authority.bin", b"a"),
            ("wrapper.bin", b"w"), ("debugger.bin", b"d"), ("transport.bin", b"t"),
        ):
            (root / name).write_bytes(data)
        return MODULE.prepare_request(
            {
                "function": "mbCapListDebug",
                "function_sha256": "a" * 64,
                "argv": ["wrapper.bin", "mwcceppc.exe", "-c", str(root / "capsule.c"), "-o", str(output)],
                "cwd": str(root),
                "source": str(root / "capsule.c"),
                "compiler": str(root / "mwcceppc.exe"),
                "wrapper": str(root / "wrapper.bin"),
                "debugger": str(root / "debugger.bin"),
                "transport": str(root / "transport.bin"),
                "authority": str(root / "authority.bin"),
                "session_id": session_id,
            },
            root / "capture",
        )

    def test_capture_with_backend_accepts_preauthenticated_exact_compiler_output_only(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            for name, data in (
                ("capsule.c", b"s"), ("mwcceppc.exe", b"c"), ("authority.bin", b"a"),
                ("wrapper.bin", b"w"), ("debugger.bin", b"d"), ("transport.bin", b"t"),
            ):
                (root / name).write_bytes(data)
            output = root / "compiler-output.o"
            request_path = MODULE.prepare_request(
                {
                    "function": "mbCapListDebug",
                    "function_sha256": "a" * 64,
                    "argv": ["wrapper.bin", "mwcceppc.exe", "-c", str(root / "capsule.c"), "-o", str(output)],
                    "cwd": str(root),
                    "source": str(root / "capsule.c"),
                    "compiler": str(root / "mwcceppc.exe"),
                    "wrapper": str(root / "wrapper.bin"),
                    "debugger": str(root / "debugger.bin"),
                    "transport": str(root / "transport.bin"),
                    "authority": str(root / "authority.bin"),
                    "session_id": "session-000000000000000b",
                },
                root / "capture",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(request_path, require_empty=True, external_trust_root=trust_root)
            self.assertEqual(auth_context["compiler_output_paths"], (output,))
            self.assertIsNotNone(auth_context["prelaunch_empty_output_proof"])
            output.write_bytes(b"compiler output")
            envelope = MODULE.capture_with_backend(
                request_path,
                FakeBackend(),
                external_trust_root=trust_root,
                preauthenticated_auth=auth_context,
            )
            self.assertTrue(envelope["status"].startswith("CAPTURED"))

        with TemporaryDirectory() as directory:
            root = Path(directory)
            for name, data in (
                ("capsule.c", b"s"), ("mwcceppc.exe", b"c"), ("authority.bin", b"a"),
                ("wrapper.bin", b"w"), ("debugger.bin", b"d"), ("transport.bin", b"t"),
            ):
                (root / name).write_bytes(data)
            request_path = MODULE.prepare_request(
                {
                    "function": "mbCapListDebug",
                    "function_sha256": "a" * 64,
                    "argv": ["wrapper.bin", "mwcceppc.exe", "-c", str(root / "capsule.c")],
                    "cwd": str(root),
                    "source": str(root / "capsule.c"),
                    "compiler": str(root / "mwcceppc.exe"),
                    "wrapper": str(root / "wrapper.bin"),
                    "debugger": str(root / "debugger.bin"),
                    "transport": str(root / "transport.bin"),
                    "authority": str(root / "authority.bin"),
                    "session_id": "session-000000000000000c",
                },
                root / "capture",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(request_path, require_empty=True, external_trust_root=trust_root)
            (root / "capture" / "unowned.o").write_bytes(b"unowned")
            with self.assertRaisesRegex(MODULE.Rejected, "stale or partial files"):
                MODULE.capture_with_backend(
                    request_path,
                    FakeBackend(),
                    external_trust_root=trust_root,
                    preauthenticated_auth=auth_context,
                )

    def test_compiler_transport_paths_are_isolated_and_single_use(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-0000000000000012",
            )
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root_for_request(request_path),
            )
            paths = MODULE._compiler_transport_paths(auth_context)
            self.assertEqual(
                paths["directory"],
                root / "capture.compiler-transport-session-0000000000000012",
            )
            self.assertNotEqual(paths["directory"], auth_context["output_dir"])
            paths["directory"].mkdir()
            with self.assertRaisesRegex(MODULE.Rejected, "already exists"):
                MODULE._compiler_transport_paths(auth_context)

    def test_compiler_execution_receipt_seals_exit2_without_object(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-0000000000000013",
            )
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root_for_request(request_path),
            )
            paths = MODULE._compiler_transport_paths(auth_context)
            paths["directory"].mkdir()
            paths["stdout"].write_bytes(b"")
            paths["stderr"].write_bytes(b"fatal diagnostic\r\n")
            receipt = MODULE._compiler_execution_receipt(
                auth_context,
                transport_mode="authenticated_direct_compiler",
                executed_argv=auth_context["request"]["argv"][1:],
                include_search_paths=[],
                stdout_path=paths["stdout"],
                stderr_path=paths["stderr"],
                process_created=True,
                process_quiesced=True,
                terminated_by_capture=False,
                exit_code=2,
                failure="compiler process exited with code 2",
            )
            self.assertEqual(receipt["status"], "UNKNOWN")
            self.assertFalse(receipt["authority_advanced"])
            self.assertEqual(receipt["exit_code"], 2)
            self.assertFalse(receipt["compiler_output"]["exists"])
            self.assertEqual(receipt["stderr"]["sha256"], hashlib.sha256(b"fatal diagnostic\r\n").hexdigest())
            unsigned = {key: value for key, value in receipt.items() if key != "receipt_sha256"}
            self.assertEqual(receipt["receipt_sha256"], MODULE.canonical_hash(unsigned))
            MODULE._publish_compiler_execution_receipt(paths["receipt"], receipt)
            self.assertEqual(json.loads(paths["receipt"].read_text())["receipt_sha256"], receipt["receipt_sha256"])
            with self.assertRaises(FileExistsError):
                MODULE._publish_compiler_execution_receipt(paths["receipt"], receipt)

    def test_compiler_execution_receipt_binds_successful_object_hash(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-0000000000000014",
            )
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root_for_request(request_path),
            )
            paths = MODULE._compiler_transport_paths(auth_context)
            paths["directory"].mkdir()
            paths["stdout"].write_bytes(b"compiler stdout")
            paths["stderr"].write_bytes(b"")
            object_bytes = minimal_ppc_elf_object()
            output.write_bytes(object_bytes)
            receipt = MODULE._compiler_execution_receipt(
                auth_context,
                transport_mode="authenticated_direct_compiler",
                executed_argv=auth_context["request"]["argv"][1:],
                include_search_paths=[],
                stdout_path=paths["stdout"],
                stderr_path=paths["stderr"],
                process_created=True,
                process_quiesced=True,
                terminated_by_capture=False,
                exit_code=0,
                failure=None,
            )
            self.assertEqual(receipt["status"], "COMPILER_TO_OBJECT_OBSERVED")
            self.assertTrue(receipt["compiler_output"]["exists"])
            self.assertEqual(receipt["compiler_output"]["sha256"], hashlib.sha256(object_bytes).hexdigest())
            self.assertTrue(receipt["compiler_output"]["complete"])
            self.assertFalse(receipt["authority_advanced"])

    def test_compiler_terminal_receipt_admits_only_complete_capture(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root, output, session_id="session-0000000000000015"
            )
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root_for_request(request_path),
            )
            capture_result = MODULE.capture_with_backend(
                request_path,
                FakeBackend(),
                external_trust_root=trust_root_for_request(request_path),
                preauthenticated_auth=auth_context,
            )
            paths = MODULE._compiler_transport_paths(auth_context)
            paths["directory"].mkdir()
            paths["stdout"].write_bytes(b"ok\n")
            paths["stderr"].write_bytes(b"")
            output.write_bytes(minimal_ppc_elf_object())
            environment = MODULE._compiler_environment_binding({"Path": "one", "TEMP": "two"})

            receipt = MODULE._compiler_execution_receipt(
                auth_context,
                transport_mode="authenticated_direct_compiler",
                executed_argv=auth_context["request"]["argv"][1:],
                include_search_paths=[],
                stdout_path=paths["stdout"],
                stderr_path=paths["stderr"],
                process_created=True,
                process_quiesced=True,
                terminated_by_capture=False,
                exit_code=0,
                failure=None,
                environment_binding=environment,
                capture_result=capture_result,
                process_summary={
                    "observed_process_ids": [7],
                    "exited_process_ids": [7],
                    "open_process_ids": [],
                    "open_thread_handle_count": 0,
                    "active_debug_event": False,
                },
            )

            self.assertEqual(receipt["terminal_state"], "SUCCESS", receipt)
            self.assertEqual(receipt["evidence_admission"], "COMPLETE")
            self.assertEqual(receipt["object_observation"], "ADMITTED_COMPLETE")
            self.assertEqual(receipt["environment"], environment)
            self.assertTrue(all(row["exists"] for row in receipt["capture_outputs"].values()))

    def test_compiler_terminal_receipt_rejects_unsealed_cross_bound_capture(self) -> None:
        for mutation in ("malformed_envelope", "cross_session", "tampered_stream"):
            with self.subTest(mutation=mutation), TemporaryDirectory() as directory:
                root = Path(directory)
                output = root / "compiler-output.o"
                request_path = self._prepare_handoff_request(
                    root,
                    output,
                    session_id=f"session-00000000000002{len(mutation):02x}",
                )
                trust_root = trust_root_for_request(request_path)
                auth_context = MODULE.authenticate_request(
                    request_path,
                    require_empty=True,
                    external_trust_root=trust_root,
                )
                capture_result = MODULE.capture_with_backend(
                    request_path,
                    FakeBackend(),
                    external_trust_root=trust_root,
                    preauthenticated_auth=auth_context,
                )
                envelope_path = auth_context["paths"]["envelope"]
                if mutation == "malformed_envelope":
                    envelope_path.write_bytes(b"{}\n")
                elif mutation == "tampered_stream":
                    auth_context["paths"]["event_stream_stack"].write_bytes(b"tampered\n")
                else:
                    envelope = json.loads(envelope_path.read_text(encoding="utf-8"))
                    envelope["context"]["session_id"] = "session-ffffffffffffffff"
                    unsigned = {key: value for key, value in envelope.items() if key != "envelope_sha256"}
                    envelope["envelope_sha256"] = MODULE.canonical_hash(unsigned)
                    envelope_path.write_text(
                        json.dumps(envelope, ensure_ascii=True, sort_keys=True, indent=2) + "\n",
                        encoding="utf-8",
                    )
                    capture_result = dict(capture_result)
                    capture_result["envelope_sha256"] = envelope["envelope_sha256"]

                paths = MODULE._compiler_transport_paths(auth_context)
                paths["directory"].mkdir()
                paths["stdout"].write_bytes(b"")
                paths["stderr"].write_bytes(b"")
                output.write_bytes(minimal_ppc_elf_object())
                receipt = MODULE._compiler_execution_receipt(
                    auth_context,
                    transport_mode="wrapper_memexec",
                    executed_argv=auth_context["request"]["argv"],
                    include_search_paths=[],
                    stdout_path=paths["stdout"],
                    stderr_path=paths["stderr"],
                    process_created=True,
                    process_quiesced=True,
                    terminated_by_capture=False,
                    exit_code=0,
                    failure=None,
                    capture_result=capture_result,
                )
                self.assertEqual(receipt["terminal_state"], "UNKNOWN")
                self.assertEqual(receipt["evidence_admission"], "NONE")
                self.assertIsNotNone(receipt["capture_validation_failure"])

    def test_compiler_terminal_receipt_quarantines_partial_object(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root, output, session_id="session-0000000000000016"
            )
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root_for_request(request_path),
            )
            paths = MODULE._compiler_transport_paths(auth_context)
            paths["directory"].mkdir()
            paths["stdout"].write_bytes(b"")
            paths["stderr"].write_bytes(b"fatal\n")
            output.write_bytes(b"partial")

            receipt = MODULE._compiler_execution_receipt(
                auth_context,
                transport_mode="wrapper_memexec",
                executed_argv=auth_context["request"]["argv"],
                include_search_paths=[],
                stdout_path=paths["stdout"],
                stderr_path=paths["stderr"],
                process_created=True,
                process_quiesced=True,
                terminated_by_capture=False,
                exit_code=2,
                failure="compiler failed",
                capture_result=None,
            )

            self.assertEqual(receipt["terminal_state"], "UNKNOWN")
            self.assertEqual(receipt["object_observation"], "PRESENT_UNADMITTED")
            self.assertEqual(receipt["evidence_admission"], "NONE")
            self.assertEqual(receipt["primary_failure"], "compiler failed")
            self.assertFalse(receipt["compiler_output"]["complete"])

    def test_compiler_success_rejects_empty_and_truncated_object(self) -> None:
        for index, payload in enumerate((b"", minimal_ppc_elf_object()[:-1])):
            with self.subTest(payload_size=len(payload)), TemporaryDirectory() as directory:
                root = Path(directory)
                output = root / "compiler-output.o"
                request_path = self._prepare_handoff_request(
                    root,
                    output,
                    session_id=f"session-00000000000003{index:02x}",
                )
                trust_root = trust_root_for_request(request_path)
                auth_context = MODULE.authenticate_request(
                    request_path,
                    require_empty=True,
                    external_trust_root=trust_root,
                )
                capture_result = MODULE.capture_with_backend(
                    request_path,
                    FakeBackend(),
                    external_trust_root=trust_root,
                    preauthenticated_auth=auth_context,
                )
                paths = MODULE._compiler_transport_paths(auth_context)
                paths["directory"].mkdir()
                paths["stdout"].write_bytes(b"")
                paths["stderr"].write_bytes(b"")
                output.write_bytes(payload)
                receipt = MODULE._compiler_execution_receipt(
                    auth_context,
                    transport_mode="wrapper_memexec",
                    executed_argv=auth_context["request"]["argv"],
                    include_search_paths=[],
                    stdout_path=paths["stdout"],
                    stderr_path=paths["stderr"],
                    process_created=True,
                    process_quiesced=True,
                    terminated_by_capture=False,
                    exit_code=0,
                    failure=None,
                    capture_result=capture_result,
                )
                self.assertEqual(receipt["terminal_state"], "UNKNOWN")
                self.assertFalse(receipt["compiler_output"]["complete"])
                self.assertEqual(receipt["object_observation"], "PRESENT_UNADMITTED")
                self.assertEqual(receipt["evidence_admission"], "NONE")

    def test_compiler_receipt_publication_failure_leaves_no_partial_file(self) -> None:
        with TemporaryDirectory() as directory:
            path = Path(directory) / "compiler-to-object.json"
            unsigned = {"schema": MODULE.COMPILER_EXECUTION_RECEIPT_SCHEMA, "status": "UNKNOWN"}
            receipt = {**unsigned, "receipt_sha256": MODULE.canonical_hash(unsigned)}
            with mock.patch.object(MODULE.os, "fsync", side_effect=OSError("disk failure")):
                with self.assertRaisesRegex(OSError, "disk failure"):
                    MODULE._publish_compiler_execution_receipt(path, receipt)
            self.assertFalse(path.exists())

    def test_compiler_environment_block_is_immutable_and_hash_bound(self) -> None:
        environment = {"B": "2", "a": "1"}
        binding, block = MODULE._sealed_compiler_environment(environment)
        environment["a"] = "changed"
        environment["C"] = "3"
        self.assertEqual("".join(block), "a=1\0B=2\0\0\0")
        self.assertEqual(binding, MODULE._compiler_environment_binding({"B": "2", "a": "1"}))

    def test_diagnostic_stream_close_failure_preserves_primary(self) -> None:
        class BadStream:
            def close(self) -> None:
                raise OSError("flush failed")

        primary = MODULE.Rejected("compiler failed first")
        failure = MODULE._close_capture_streams([BadStream()], primary)
        self.assertIsInstance(failure, MODULE.Rejected)
        self.assertEqual(failure.primary_failure, "compiler failed first")
        self.assertEqual(len(failure.secondary_failures), 1)
        self.assertIn("stream close failure", str(failure))

    def test_compiler_terminal_receipt_classifies_nonquiescent_timeout_unknown(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root, output, session_id="session-0000000000000018"
            )
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root_for_request(request_path),
            )
            paths = MODULE._compiler_transport_paths(auth_context)
            paths["directory"].mkdir()
            paths["stdout"].write_bytes(b"")
            paths["stderr"].write_bytes(b"timeout\n")
            receipt = MODULE._compiler_execution_receipt(
                auth_context,
                transport_mode="wrapper_memexec",
                executed_argv=auth_context["request"]["argv"],
                include_search_paths=[],
                stdout_path=paths["stdout"],
                stderr_path=paths["stderr"],
                process_created=True,
                process_quiesced=False,
                terminated_by_capture=True,
                exit_code=None,
                failure="terminal timeout",
                process_summary={
                    "observed_process_ids": [7],
                    "exited_process_ids": [],
                    "open_process_ids": [7],
                    "open_thread_handle_count": 1,
                    "active_debug_event": True,
                },
            )
            self.assertEqual(receipt["terminal_state"], "UNKNOWN")
            self.assertFalse(receipt["diagnostics_sealed"])
            self.assertEqual(receipt["evidence_admission"], "NONE")

    def test_capture_cleanup_failure_preserves_primary_failure(self) -> None:
        class PrimaryAndCleanupFailure(FakeBackend):
            def run(self, session: MODULE.CombinedCaptureSession) -> None:
                del session
                raise MODULE.Rejected("primary compiler failure")

            def close(self) -> None:
                raise MODULE.Rejected("cleanup drain failure")

        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path = self._prepare_handoff_request(
                root,
                root / "compiler-output.o",
                session_id="session-0000000000000017",
            )
            trust_root = trust_root_for_request(request_path)
            with self.assertRaisesRegex(
                MODULE.Rejected,
                "primary compiler failure; secondary failure: native cleanup failure",
            ) as caught:
                MODULE.capture_with_backend(
                    request_path,
                    PrimaryAndCleanupFailure(),
                    external_trust_root=trust_root,
                )
            self.assertEqual(caught.exception.primary_failure, "primary compiler failure")
            self.assertEqual(len(caught.exception.secondary_failures), 1)

    def test_capture_triple_failure_preserves_primary_and_cleanup_order(self) -> None:
        class CompilerDispatcherAndBackendFailure(FakeBackend):
            def run(self, session: MODULE.CombinedCaptureSession) -> None:
                del session
                raise MODULE.Rejected("compiler primary failure")

            def remove_breakpoint(self, address: int) -> None:
                del address
                raise MODULE.Rejected("dispatcher restore failure")

            def close(self) -> None:
                raise MODULE.Rejected("backend close failure")

        with TemporaryDirectory() as directory:
            root = Path(directory)
            request_path = self._prepare_handoff_request(
                root,
                root / "compiler-output.o",
                session_id="session-0000000000000026",
            )
            trust_root = trust_root_for_request(request_path)
            with self.assertRaises(MODULE.Rejected) as caught:
                MODULE.capture_with_backend(
                    request_path,
                    CompilerDispatcherAndBackendFailure(),
                    external_trust_root=trust_root,
                )
            backend_failure = "native cleanup failure: Rejected: backend close failure"
            self.assertEqual(caught.exception.primary_failure, "compiler primary failure")
            self.assertEqual(len(caught.exception.secondary_failures), 2)
            dispatcher_failure = caught.exception.secondary_failures[0]
            self.assertTrue(
                dispatcher_failure.startswith(
                    "breakpoint cleanup failure: Rejected: breakpoint cleanup failed:"
                )
            )
            self.assertIn("dispatcher restore failure", dispatcher_failure)
            self.assertEqual(caught.exception.secondary_failures[1], backend_failure)
            self.assertEqual(
                str(caught.exception),
                "; ".join(
                    (
                        "compiler primary failure",
                        f"secondary failure: {dispatcher_failure}",
                        f"secondary failure: {backend_failure}",
                    )
                ),
            )

    def test_terminal_failure_combiner_deduplicates_repeated_finalization(self) -> None:
        failure = MODULE._combine_terminal_failure(
            MODULE.Rejected("compiler primary failure"), "backend close failure"
        )
        repeated = MODULE._combine_terminal_failure(failure, "backend close failure")
        self.assertIs(repeated, failure)
        self.assertEqual(repeated.primary_failure, "compiler primary failure")
        self.assertEqual(repeated.secondary_failures, ["backend close failure"])
        self.assertEqual(
            str(repeated),
            "compiler primary failure; secondary failure: backend close failure",
        )

    def test_final_owner_join_unknown_publishes_immutable_partial_evidence(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-0000000000000012",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root,
            )
            output.write_bytes(b"compiler-owned object")
            partial_dir = root / "partial-evidence"
            original_run = MODULE.CombinedCaptureSession.run

            def run_with_repeated_fmuls(session: MODULE.CombinedCaptureSession) -> dict[str, object]:
                envelope = original_run(session)
                events = envelope["events"]
                token = envelope["inventory"]["locals"][1]["token"]
                for ordinal, instruction_index in enumerate((128, 196)):
                    sequence = len(events)
                    events.append({
                        "schema": MODULE.EVENT_SCHEMA,
                        "event_id": f"{session.session_id}-e{sequence:06d}",
                        "session_id": session.session_id,
                        "process_id": envelope["context"]["process_id"],
                        "function": session.function,
                        "lane": "pcode",
                        "sequence": sequence,
                        "event_kind": "machine_emission",
                        "hook_id": "gc27_machine_emit",
                        "status": "CAPTURED",
                        "pcode_token": f"pcode-{session.session_id}-{ordinal:06d}",
                        "emitted_offset": instruction_index * 4,
                        "instruction_index": instruction_index,
                        "opcode_enum": 1,
                        "ppc_word": 0,
                        "ppc_bytes": "00000000",
                        "mnemonic": "fmuls",
                        "registers": {
                            "destination": "f1",
                            "source_a": "f31",
                            "source_b": "f0",
                        },
                        "reaching_definitions": [instruction_index - 2, instruction_index - 1],
                        "owner_joins": [],
                        "physical_owner_joins": [{
                            "object_token": token,
                            "physical_register": "f31",
                        }],
                    })
                unresolved_token = f"pcode-{session.session_id}-009999"
                for operand_ordinal, (color, owner) in enumerate((
                    (0, None),
                    (31, token),
                    (0, None),
                )):
                    sequence = len(events)
                    operand = {
                        "schema": MODULE.EVENT_SCHEMA,
                        "event_id": f"{session.session_id}-e{sequence:06d}",
                        "session_id": session.session_id,
                        "process_id": envelope["context"]["process_id"],
                        "function": session.function,
                        "lane": "pcode",
                        "sequence": sequence,
                        "event_kind": "pcode_capture",
                        "hook_id": "pcode_color_post",
                        "status": "CAPTURED",
                        "stage": "pcode_color_post",
                        "pcode_token": unresolved_token,
                        "ig_token": f"ig-{session.session_id}-{900 + operand_ordinal:06d}",
                        "operand_ordinal": operand_ordinal,
                        "operand_count": 3,
                        "operand_kind": 0,
                        "operand_class": 3,
                        "operand_bank": "FPR",
                        "operand_index": 900 + operand_ordinal,
                        "final_color": color,
                        "ig_flags": 2 if owner is None else 0,
                        "confirmed": True,
                    }
                    if owner is None:
                        operand["hidden_owner_token"] = (
                            f"hidden-ig-{session.session_id}-{900 + operand_ordinal:06d}"
                        )
                    else:
                        operand["object_token"] = owner
                    events.append(operand)
                sequence = len(events)
                events.append({
                    "schema": MODULE.EVENT_SCHEMA,
                    "event_id": f"{session.session_id}-e{sequence:06d}",
                    "session_id": session.session_id,
                    "process_id": envelope["context"]["process_id"],
                    "function": session.function,
                    "lane": "pcode",
                    "sequence": sequence,
                    "event_kind": "machine_emission",
                    "hook_id": "gc27_machine_emit",
                    "status": "UNKNOWN",
                    "reason": "ambiguous reaching definition",
                    "pcode_token": unresolved_token,
                    "emitted_offset": 256 * 4,
                    "instruction_index": 256,
                    "opcode_enum": 167,
                    "ppc_word": 0xEC1F0032,
                    "ppc_bytes": "ec1f0032",
                    "mnemonic": "fmuls",
                    "registers": {
                        "destination": "f0",
                        "source_a": "f31",
                        "source_b": "f0",
                    },
                    "arithmetic_op": "multiply",
                    "arithmetic_type": "f32",
                    "known_reaching_definitions": [{
                        "physical_register": "f0",
                        "instruction_index": 255,
                    }],
                    "missing_reaching_registers": ["f31"],
                })
                envelope["event_count"] = len(events)
                lanes = {}
                for lane, key in (("stack", "event_stream_stack"), ("pcode", "event_stream_pcode")):
                    lane_events = [event for event in events if event["lane"] == lane]
                    data = MODULE.canonical_lane_bytes(lane_events)
                    lanes[lane] = {
                        "event_count": len(lane_events),
                        "event_ids": [event["event_id"] for event in lane_events],
                        "size_bytes": len(data),
                        "sha256": hashlib.sha256(data).hexdigest(),
                    }
                    envelope["outputs"][key].update({
                        "size": len(data),
                        "sha256": hashlib.sha256(data).hexdigest(),
                    })
                envelope["lanes"] = lanes
                envelope["envelope_sha256"] = MODULE.canonical_hash({
                    key: value
                    for key, value in envelope.items()
                    if key != "envelope_sha256"
                })
                return envelope

            with mock.patch.object(MODULE.CombinedCaptureSession, "run", run_with_repeated_fmuls):
                with mock.patch.object(
                    MODULE,
                    "validate_envelope",
                    side_effect=MODULE.Rejected(
                        "machine physical owner join lacks an exact same-session assignment"
                    ),
                ):
                    result = MODULE.capture_with_backend(
                        request_path,
                        FakeBackend(),
                        external_trust_root=trust_root,
                        preauthenticated_auth=auth_context,
                        partial_evidence_dir=partial_dir,
                    )

            self.assertEqual(result["status"], "UNKNOWN")
            self.assertFalse(result["authority_advanced"])
            self.assertTrue(output.exists())
            self.assertEqual(
                sorted(path.name for path in (root / "capture").iterdir()),
                ["request.json"],
            )
            self.assertEqual(
                sorted(path.name for path in partial_dir.iterdir()),
                sorted(MODULE._PARTIAL_EVIDENCE_FILENAMES.values()),
            )
            machine_events = [
                json.loads(line)
                for line in (partial_dir / "machine.events.jsonl").read_text(encoding="utf-8").splitlines()
            ]
            captured_machine = [event for event in machine_events if event["status"] == "CAPTURED"]
            self.assertEqual([event["mnemonic"] for event in captured_machine], ["fmuls", "fmuls"])
            self.assertEqual(
                [event["reaching_definitions"] for event in captured_machine],
                [[126, 127], [194, 195]],
            )
            graph = json.loads((partial_dir / "ownership-failure-graph.json").read_text(encoding="utf-8"))
            self.assertEqual(graph["status"], "UNKNOWN")
            self.assertFalse(graph["authority_advanced"])
            self.assertEqual(graph["first_absent_edge"]["edge"], "vreg_to_physical")
            self.assertEqual(len(graph["unresolved_machine_sites"]), 1)
            unresolved = graph["unresolved_machine_sites"][0]
            self.assertEqual(unresolved["instruction_index"], 256)
            self.assertEqual(
                unresolved["known_reaching_definitions"],
                [{"physical_register": "f0", "instruction_index": 255}],
            )
            self.assertEqual(unresolved["missing_reaching_registers"], ["f31"])
            self.assertEqual(
                [row["owner_identity"]["status"] for row in unresolved["pcode_operands"]],
                ["UNKNOWN", "PRESENT", "UNKNOWN"],
            )
            manifest = json.loads((partial_dir / "partial-evidence.json").read_text(encoding="utf-8"))
            self.assertEqual(manifest["status"], "UNKNOWN")
            self.assertFalse(manifest["authority_advanced"])
            for name, artifact in manifest["artifacts"].items():
                path = Path(artifact["path"])
                self.assertEqual(path.stat().st_size, artifact["size"], name)
                self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), artifact["sha256"], name)

            volatile = graph["volatile_owner_facts"]
            self.assertEqual(volatile["session_id"], "session-0000000000000012")
            self.assertEqual(volatile["candidate_object"], manifest["compiler_owned_object"])
            self.assertEqual(
                volatile["volatile_owner_facts_sha256"],
                MODULE.canonical_hash({
                    key: value
                    for key, value in volatile.items()
                    if key != "volatile_owner_facts_sha256"
                }),
            )
            self.assertNotIn("pointer", json.dumps(volatile, sort_keys=True).lower())
            hidden_bindings = [
                row for row in volatile["object_identity_bindings"]
                if "hidden_owner_token" in row
            ]
            self.assertEqual(hidden_bindings, [])

    def _volatile_owner_fixture(
        self,
        root: Path,
        *,
        machine_registers: dict[str, str] | None,
    ) -> tuple[dict[str, object], list[dict[str, object]]]:
        request = auth(root)["request"]
        assert isinstance(request, dict)
        session_id = str(request["session_id"])
        object_token = f"local-{session_id}-000000"
        pcode_id = f"pcode-{session_id}-000000"
        events: list[dict[str, object]] = [
            {
                "event_id": f"{session_id}-e000000",
                "sequence": 0,
                "event_kind": "regalloc_assignment",
                "status": "EXACT",
                "object_token": object_token,
                "vreg_id": "f32",
                "bank": "FPR",
            },
            {
                "event_id": f"{session_id}-e000001",
                "sequence": 1,
                "event_kind": "physical_reg_assignment",
                "status": "EXACT",
                "object_token": object_token,
                "physical_reg": 17,
                "bank": "FPR",
            },
            {
                "event_id": f"{session_id}-e000002",
                "sequence": 2,
                "event_kind": "pcode_capture",
                "status": "CAPTURED",
                "pcode_token": pcode_id,
                "ig_token": f"ig-{session_id}-000000",
                "operand_ordinal": 0,
                "operand_count": 1,
                "operand_kind": 2,
                "operand_bank": "FPR",
                "operand_index": 32,
                "final_color": 17,
                "ig_flags": 0,
                "object_token": object_token,
            },
        ]
        if machine_registers is not None:
            events.append({
                "event_id": f"{session_id}-e000003",
                "sequence": 3,
                "event_kind": "machine_emission",
                "status": "CAPTURED",
                "pcode_token": pcode_id,
                "instruction_index": 9,
                "mnemonic": "fmuls",
                "registers": machine_registers,
                "operand_role_order": list(machine_registers),
                "reaching_definitions": [8],
            })
        return request, events

    def test_volatile_owner_facts_preserve_unique_def_use_join(self) -> None:
        with TemporaryDirectory() as directory:
            context, events = self._volatile_owner_fixture(
                Path(directory), machine_registers={"source_a": "f17"}
            )
            session_id = str(context["session_id"])
            events.extend([
                {
                    "event_id": f"{session_id}-e000004",
                    "sequence": 4,
                    "event_kind": "machine_emission",
                    "status": "CAPTURED",
                    "pcode_token": f"pcode-{session_id}-000004",
                    "instruction_index": 8,
                    "mnemonic": "fmuls",
                    "registers": {"destination": "f17"},
                    "operand_role_order": ["destination"],
                    "reaching_definitions": [],
                },
                {
                    "event_id": f"{session_id}-e000005",
                    "sequence": 5,
                    "event_kind": "machine_emission",
                    "status": "CAPTURED",
                    "pcode_token": f"pcode-{session_id}-000005",
                    "instruction_index": 11,
                    "mnemonic": "fmuls",
                    "registers": {"source_a": "f17"},
                    "operand_role_order": ["source_a"],
                    "reaching_definitions": [8],
                },
            ])
            facts = MODULE._build_volatile_owner_facts(events, context)
        self.assertEqual(len(facts["closed_residual_rows"]), 1)
        self.assertEqual(facts["closed_residual_rows"][0]["required_operand_roles"], ["source_a"])
        owner = facts["owner_facts"][0]
        self.assertEqual(owner["classification"], "UNIQUE")
        self.assertEqual(owner["vreg"], "f32")
        self.assertEqual(owner["physical_register"], "f17")
        self.assertEqual(owner["def_id"], "def-000008")
        self.assertEqual(owner["use_ids"], ["use-000009", "use-000011"])
        hashed_events = {
            row["event_id"]: row["sha256"] for row in facts["raw_event_hashes"]
        }
        cited_instruction_indexes = {
            int(owner["def_id"].rsplit("-", 1)[1]),
            *(int(use_id.rsplit("-", 1)[1]) for use_id in owner["use_ids"]),
        }
        cited_machine_event_ids = {
            str(event["event_id"])
            for event in events
            if event.get("event_kind") == "machine_emission"
            and event.get("instruction_index") in cited_instruction_indexes
        }
        self.assertEqual(
            cited_machine_event_ids,
            {
                f"{context['session_id']}-e000003",
                f"{context['session_id']}-e000004",
                f"{context['session_id']}-e000005",
            },
        )
        self.assertTrue(cited_machine_event_ids.issubset(hashed_events))
        events_by_id = {str(event["event_id"]): event for event in events}
        for event_id in cited_machine_event_ids:
            self.assertEqual(hashed_events[event_id], MODULE.canonical_hash(events_by_id[event_id]))

    def test_volatile_owner_facts_preserve_competing_ambiguous_joins(self) -> None:
        with TemporaryDirectory() as directory:
            context, events = self._volatile_owner_fixture(
                Path(directory),
                machine_registers={"source_a": "f17", "source_b": "f17"},
            )
            events[2]["operand_count"] = 2
            events.append({
                **events[-1],
                "event_id": f"{context['session_id']}-e000004",
                "sequence": 4,
                "operand_role_order": ["source_b", "source_a"],
            })
            facts = MODULE._build_volatile_owner_facts(events, context)
        self.assertEqual(len(facts["owner_facts"]), 2)
        self.assertEqual(
            {row["role"] for row in facts["owner_facts"]},
            {"source_a", "source_b"},
        )
        self.assertEqual(
            {row["classification"] for row in facts["owner_facts"]},
            {"UNKNOWN_AMBIGUOUS_MACHINE_EDGE"},
        )
        self.assertEqual(
            facts["closed_residual_rows"][0]["required_operand_roles"],
            ["source_a", "source_b"],
        )

    def test_volatile_owner_facts_preserve_missing_join_as_unknown(self) -> None:
        with TemporaryDirectory() as directory:
            context, events = self._volatile_owner_fixture(
                Path(directory), machine_registers=None
            )
            facts = MODULE._build_volatile_owner_facts(events, context)
        self.assertEqual(len(facts["owner_facts"]), 1)
        owner = facts["owner_facts"][0]
        self.assertEqual(owner["classification"], "UNKNOWN_MISSING_MACHINE_EDGE")
        self.assertEqual(owner["role"], "UNKNOWN")
        self.assertIsNone(owner["def_id"])
        self.assertEqual(owner["use_ids"], [])
        self.assertEqual(facts["closed_residual_rows"][0]["required_operand_roles"], [])

    def test_volatile_owner_facts_use_ordinal_to_split_duplicate_colors(self) -> None:
        with TemporaryDirectory() as directory:
            context, events = self._volatile_owner_fixture(
                Path(directory), machine_registers=None
            )
            session_id = str(context["session_id"])
            object_token = f"local-{session_id}-000000"
            pcode_id = f"pcode-{session_id}-000000"
            events = [
                {
                    **events[2],
                    "event_id": f"{session_id}-e{ordinal:06d}",
                    "sequence": ordinal,
                    "ig_token": f"ig-{session_id}-{ordinal:06d}",
                    "operand_ordinal": ordinal,
                    "operand_count": 3,
                    "final_color": color,
                    "object_token": object_token,
                }
                for ordinal, color in enumerate((0, 31, 0))
            ]
            events.append({
                "event_id": f"{session_id}-e000003",
                "sequence": 3,
                "event_kind": "machine_emission",
                "status": "CAPTURED",
                "pcode_token": pcode_id,
                "instruction_index": 608,
                "mnemonic": "fmuls",
                "registers": {
                    "destination": "f0",
                    "source_a": "f31",
                    "source_b": "f0",
                },
                "operand_role_order": ["destination", "source_a", "source_b"],
                "reaching_definitions": [606, 607],
            })
            facts = MODULE._build_volatile_owner_facts(events, context)
        self.assertEqual(
            [fact["role"] for fact in facts["owner_facts"]],
            ["destination", "source_a", "source_b"],
        )
        self.assertEqual(
            [fact["physical_register"] for fact in facts["owner_facts"]],
            ["f0", "f31", "f0"],
        )
        self.assertEqual(
            {fact["classification"] for fact in facts["owner_facts"]},
            {"UNKNOWN_MISSING_VREG_EDGE"},
        )

    def test_volatile_owner_facts_drop_unjoinable_hidden_bulk(self) -> None:
        with TemporaryDirectory() as directory:
            context, events = self._volatile_owner_fixture(
                Path(directory), machine_registers=None
            )
            hidden = dict(events[2])
            hidden.pop("object_token")
            hidden["hidden_owner_token"] = (
                f"hidden-ig-{context['session_id']}-000000"
            )
            facts = MODULE._build_volatile_owner_facts([hidden], context)
        self.assertEqual(facts["closed_residual_rows"], [])
        self.assertEqual(facts["owner_facts"], [])
        self.assertEqual(facts["object_identity_bindings"], [])
        self.assertEqual(facts["raw_event_hashes"], [])

    def test_partial_evidence_requires_one_compiler_owned_output_and_new_destination(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            for name, data in (
                ("capsule.c", b"s"), ("mwcceppc.exe", b"c"), ("authority.bin", b"a"),
                ("wrapper.bin", b"w"), ("debugger.bin", b"d"), ("transport.bin", b"t"),
            ):
                (root / name).write_bytes(data)
            request_path = MODULE.prepare_request(
                {
                    "function": "mbCapListDebug",
                    "function_sha256": "a" * 64,
                    "argv": ["wrapper.bin", "mwcceppc.exe", "-c", str(root / "capsule.c")],
                    "cwd": str(root),
                    "source": str(root / "capsule.c"),
                    "compiler": str(root / "mwcceppc.exe"),
                    "wrapper": str(root / "wrapper.bin"),
                    "debugger": str(root / "debugger.bin"),
                    "transport": str(root / "transport.bin"),
                    "authority": str(root / "authority.bin"),
                    "session_id": "session-0000000000000013",
                },
                root / "capture",
            )
            trust_root = trust_root_for_request(request_path)
            partial_dir = root / "partial-evidence"
            with mock.patch.object(
                MODULE,
                "validate_envelope",
                side_effect=MODULE.Rejected(
                    "machine physical owner join lacks an exact same-session assignment"
                ),
            ):
                with self.assertRaisesRegex(MODULE.Rejected, "exactly one compiler-owned -o output"):
                    MODULE.capture_with_backend(
                        request_path,
                        FakeBackend(),
                        external_trust_root=trust_root,
                        partial_evidence_dir=partial_dir,
                    )
            self.assertFalse(partial_dir.exists())
            self.assertEqual(
                sorted(path.name for path in (root / "capture").iterdir()),
                ["request.json"],
            )

        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-0000000000000014",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root,
            )
            output.write_bytes(b"compiler-owned object")
            partial_dir = root / "partial-evidence"
            partial_dir.mkdir()
            with mock.patch.object(
                MODULE,
                "validate_envelope",
                side_effect=MODULE.Rejected(
                    "machine physical owner join lacks an exact same-session assignment"
                ),
            ):
                with self.assertRaisesRegex(MODULE.Rejected, "already exists"):
                    MODULE.capture_with_backend(
                        request_path,
                        FakeBackend(),
                        external_trust_root=trust_root,
                        preauthenticated_auth=auth_context,
                        partial_evidence_dir=partial_dir,
                    )
            self.assertEqual(list(partial_dir.iterdir()), [])

    def test_preauthenticated_handoff_requires_empty_proof_and_rejects_output_aliases(self) -> None:
        for label, output, expected in (
            ("request collision", "capture/request.json", "collides with request or capture evidence"),
            ("capture collision", "capture/stack.events.jsonl", "collides with request or capture evidence"),
        ):
            with self.subTest(label=label), TemporaryDirectory() as directory:
                root = Path(directory)
                output_path = root / output
                request_path = self._prepare_handoff_request(
                    root,
                    output_path,
                    session_id="session-000000000000000d",
                )
                trust_root = trust_root_for_request(request_path)
                with self.assertRaisesRegex(MODULE.Rejected, expected):
                    MODULE.authenticate_request(
                        request_path,
                        require_empty=True,
                        external_trust_root=trust_root,
                    )

        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "outside.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-0000000000000010",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(
                request_path,
                require_empty=True,
                external_trust_root=trust_root,
            )
            output.write_bytes(b"compiler output")
            envelope = MODULE.capture_with_backend(
                request_path,
                FakeBackend(),
                external_trust_root=trust_root,
                preauthenticated_auth=auth_context,
            )
            self.assertTrue(envelope["status"].startswith("CAPTURED"))

        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "capture" / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-000000000000000e",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(request_path, external_trust_root=trust_root)
            output.write_bytes(b"compiler output")
            with self.assertRaisesRegex(MODULE.Rejected, "prelaunch empty-output proof"):
                MODULE.capture_with_backend(
                    request_path,
                    FakeBackend(),
                    external_trust_root=trust_root,
                    preauthenticated_auth=auth_context,
                )

        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "capture" / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-000000000000000f",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(request_path, require_empty=True, external_trust_root=trust_root)
            stale = root / "capture" / "stale.o"
            stale.write_bytes(b"stale")
            with self.assertRaisesRegex(MODULE.Rejected, "stale or partial files"):
                MODULE.capture_with_backend(
                    request_path,
                    FakeBackend(),
                    external_trust_root=trust_root,
                    preauthenticated_auth=auth_context,
                )

        with TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "capture" / "compiler-output.o"
            request_path = self._prepare_handoff_request(
                root,
                output,
                session_id="session-0000000000000011",
            )
            trust_root = trust_root_for_request(request_path)
            auth_context = MODULE.authenticate_request(request_path, require_empty=True, external_trust_root=trust_root)
            target = root / "outside.o"
            target.write_bytes(b"target")
            alternate = root / "capture" / "alternate.o"
            try:
                alternate.symlink_to(target)
                symlink_patch = mock.patch.object(Path, "is_symlink", wraps=Path.is_symlink)
            except OSError:
                alternate.write_bytes(b"alternate")
                original_is_symlink = Path.is_symlink
                symlink_patch = mock.patch.object(
                    Path,
                    "is_symlink",
                    autospec=True,
                    side_effect=lambda candidate: candidate.name == "alternate.o" or original_is_symlink(candidate),
                )
            with symlink_patch:
                with self.assertRaisesRegex(MODULE.Rejected, "not a regular file"):
                    MODULE.capture_with_backend(
                        request_path,
                        FakeBackend(),
                        external_trust_root=trust_root,
                        preauthenticated_auth=auth_context,
                    )


class NativeWow64BackendCapabilityTests(unittest.TestCase):
    def test_native_backend_exposes_real_capture_surface(self) -> None:
        backend = MODULE.NativeWow64Backend(object(), 1, 0, 4321)
        for name in (
            "current_function",
            "snapshot_inventory",
            "snapshot_objects",
            "snapshot_varinfo",
            "read_register",
            "capture_stack_write",
            "capture_pcode",
            "capture_regalloc",
            "capture_machine_emission",
            "prepare_capture",
        ):
            self.assertTrue(callable(getattr(backend, name, None)), name)
            self.assertIn(name, backend.capabilities)

    def test_native_backend_quiesces_failed_process_before_receipt_sealing(self) -> None:
        class Kernel32:
            def __init__(self) -> None:
                self.signaled = False
                self.terminated: list[tuple[int, int]] = []
                self.closed: list[int] = []

            def GetExitCodeProcess(self, _handle: object, code: object) -> bool:
                code._obj.value = 1
                return True

            def TerminateProcess(self, handle: object, exit_code: int) -> bool:
                self.terminated.append((int(handle.value), exit_code))
                self.signaled = True
                return True

            def WaitForSingleObject(self, _handle: object, _milliseconds: int) -> int:
                return 0 if self.signaled else 258

            def CloseHandle(self, handle: int) -> bool:
                self.closed.append(handle)
                return True

        kernel32 = Kernel32()
        native = type("Native", (), {"kernel32": kernel32})()
        backend = MODULE.NativeWow64Backend(native, 11, 12, 4321)
        backend.close()
        self.assertTrue(backend.exited)
        self.assertTrue(backend.process_quiesced)
        self.assertTrue(backend.compiler_terminated_by_capture)
        self.assertEqual(backend.compiler_exit_code, 1)
        self.assertEqual(kernel32.terminated, [(11, 1)])
        self.assertEqual(sorted(kernel32.closed), [11, 12])

    def test_native_backend_accepts_signaled_process_exit_code_259(self) -> None:
        class Kernel32:
            def __init__(self) -> None:
                self.terminated = False

            def WaitForSingleObject(self, _handle: object, _milliseconds: int) -> int:
                return 0

            def GetExitCodeProcess(self, _handle: object, code: object) -> bool:
                code._obj.value = 259
                return True

            def TerminateProcess(self, _handle: object, _exit_code: int) -> bool:
                self.terminated = True
                return True

            def CloseHandle(self, _handle: int) -> bool:
                return True

        kernel32 = Kernel32()
        native = type("Native", (), {"kernel32": kernel32})()
        backend = MODULE.NativeWow64Backend(native, 11, 12, 4321)
        backend.close()

        self.assertTrue(backend.exited)
        self.assertTrue(backend.process_quiesced)
        self.assertEqual(backend.compiler_exit_code, 259)
        self.assertFalse(kernel32.terminated)

    def test_native_backend_continues_active_event_and_drains_compiler_exit(self) -> None:
        import ctypes

        class ExitProcessInfo(ctypes.Structure):
            _fields_ = [("dwExitCode", ctypes.c_uint32)]

        class EventUnion(ctypes.Union):
            _fields_ = [("ExitProcess", ExitProcessInfo)]

        class DebugEvent(ctypes.Structure):
            _fields_ = [
                ("dwDebugEventCode", ctypes.c_uint32),
                ("dwProcessId", ctypes.c_uint32),
                ("dwThreadId", ctypes.c_uint32),
                ("u", EventUnion),
            ]

        class Kernel32:
            def __init__(self) -> None:
                self.signaled = False
                self.exit_event_pending = True
                self.continues: list[tuple[int, int, int]] = []
                self.terminated: list[tuple[int, int]] = []
                self.closed: list[int] = []

            def GetExitCodeProcess(self, _handle: object, code: object) -> bool:
                code._obj.value = 1 if self.signaled else 259
                return True

            def TerminateProcess(self, handle: object, exit_code: int) -> bool:
                self.terminated.append((int(handle.value), exit_code))
                return True

            def WaitForSingleObject(self, _handle: object, _milliseconds: int) -> int:
                return 0 if self.signaled else 258

            def WaitForDebugEvent(self, event_pointer: object, _timeout: int) -> bool:
                self.assert_exit_pending()
                event = ctypes.cast(event_pointer, ctypes.POINTER(DebugEvent)).contents
                event.dwDebugEventCode = 5
                event.dwProcessId = 4321
                event.dwThreadId = 8
                event.u.ExitProcess.dwExitCode = 1
                self.exit_event_pending = False
                return True

            def assert_exit_pending(self) -> None:
                if not self.exit_event_pending:
                    raise AssertionError("unexpected extra WaitForDebugEvent")

            def ContinueDebugEvent(self, process_id: int, thread_id: int, status: int) -> bool:
                row = (int(process_id), int(thread_id), int(status))
                self.continues.append(row)
                if row[:2] == (4321, 8):
                    self.signaled = True
                return True

            def CloseHandle(self, handle: int) -> bool:
                self.closed.append(handle)
                return True

        kernel32 = Kernel32()
        native = type(
            "Native",
            (),
            {
                "kernel32": kernel32,
                "DEBUG_EVENT": DebugEvent,
                "DBG_CONTINUE": 0x00010002,
                "EXIT_PROCESS_DEBUG_EVENT": 5,
                "CREATE_PROCESS_DEBUG_EVENT": 3,
            },
        )()
        backend = MODULE.NativeWow64Backend(native, 11, 12, 4321)
        backend.compiler_process_id = 4321
        backend._active_debug_event = (4321, 7)
        backend.close()

        self.assertTrue(backend.exited)
        self.assertTrue(backend.process_quiesced)
        self.assertTrue(backend.compiler_terminated_by_capture)
        self.assertEqual(backend.compiler_exit_code, 1)
        self.assertEqual(kernel32.terminated, [(11, 1)])
        self.assertEqual(
            kernel32.continues,
            [(4321, 7, 0x00010002), (4321, 8, 0x00010002)],
        )

    def test_native_backend_releases_delivered_exit_event_before_sealing(self) -> None:
        class Kernel32:
            def __init__(self) -> None:
                self.signaled = False
                self.continues: list[tuple[int, int, int]] = []
                self.terminated = False

            def GetExitCodeProcess(self, _handle: object, code: object) -> bool:
                code._obj.value = 7
                return True

            def TerminateProcess(self, _handle: object, _exit_code: int) -> bool:
                self.terminated = True
                return True

            def WaitForSingleObject(self, _handle: object, _milliseconds: int) -> int:
                return 0 if self.signaled else 258

            def ContinueDebugEvent(self, process_id: int, thread_id: int, status: int) -> bool:
                self.continues.append((int(process_id), int(thread_id), int(status)))
                self.signaled = True
                return True

            def CloseHandle(self, _handle: int) -> bool:
                return True

        kernel32 = Kernel32()
        native = type("Native", (), {"kernel32": kernel32, "DBG_CONTINUE": 0x00010002})()
        backend = MODULE.NativeWow64Backend(native, 11, 12, 4321)
        backend.compiler_process_id = 4321
        backend.exited = True
        backend.compiler_exit_code = 7
        backend._active_debug_event = (4321, 9)
        backend.close()

        self.assertTrue(backend.process_quiesced)
        self.assertFalse(kernel32.terminated)
        self.assertEqual(kernel32.continues, [(4321, 9, 0x00010002)])

    def test_native_backend_quiesces_descendants_and_finalizes_once(self) -> None:
        class Kernel32:
            def __init__(self) -> None:
                self.signaled: set[int] = set()
                self.terminated: list[tuple[int, int]] = []
                self.closed: list[int] = []

            def WaitForSingleObject(self, handle: object, _milliseconds: int) -> int:
                return 0 if int(handle.value) in self.signaled else 258

            def TerminateProcess(self, handle: object, exit_code: int) -> bool:
                raw = int(handle.value)
                self.terminated.append((raw, exit_code))
                self.signaled.add(raw)
                return True

            def GetExitCodeProcess(self, _handle: object, code: object) -> bool:
                code._obj.value = 1
                return True

            def CloseHandle(self, handle: int) -> bool:
                self.closed.append(int(handle))
                return True

        kernel32 = Kernel32()
        native = type("Native", (), {"kernel32": kernel32})()
        backend = MODULE.NativeWow64Backend(native, 11, 12, 4321)
        backend.compiler_process_id = 4321
        backend._observed_process_ids.add(5000)
        backend._process_handles[5000] = 21
        backend._owned_handles.add(21)

        backend.close()
        first_terminated = list(kernel32.terminated)
        first_closed = list(kernel32.closed)
        backend.close()

        self.assertEqual(first_terminated, [(21, 1), (11, 1)])
        self.assertEqual(kernel32.terminated, first_terminated)
        self.assertEqual(kernel32.closed, first_closed)
        self.assertTrue(backend.process_quiesced)
        self.assertEqual(backend.terminal_process_summary()["open_process_ids"], [])
        self.assertEqual(backend.terminal_process_summary()["open_thread_handle_count"], 0)

    def test_native_backend_closehandle_false_is_terminal_failure(self) -> None:
        class Kernel32:
            def WaitForSingleObject(self, _handle: object, _milliseconds: int) -> int:
                return 0

            def GetExitCodeProcess(self, _handle: object, code: object) -> bool:
                code._obj.value = 0
                return True

            def CloseHandle(self, handle: int) -> bool:
                return int(handle) != 12

        native = type("Native", (), {"kernel32": Kernel32()})()
        backend = MODULE.NativeWow64Backend(native, 11, 12, 4321)
        with self.assertRaisesRegex(MODULE.Rejected, "CloseHandle returned FALSE"):
            backend.close()
        self.assertEqual(backend.terminal_process_summary()["unclosed_handle_count"], 1)

    def test_native_backend_quiescence_timeout_is_bounded_unknown(self) -> None:
        class Kernel32:
            def WaitForSingleObject(self, _handle: object, _milliseconds: int) -> int:
                return 258

        native = type("Native", (), {"kernel32": Kernel32()})()
        backend = MODULE.NativeWow64Backend(native, 11, 0, 4321)
        with mock.patch.object(MODULE.time, "monotonic", side_effect=[0.0, 6.0]):
            with self.assertRaisesRegex(MODULE.Rejected, "did not quiesce"):
                backend._seal_process_quiescence()
        self.assertFalse(backend.process_quiesced)

    def test_native_backend_uses_owned_list_heads_and_function_filter(self) -> None:
        backend = MODULE.NativeWow64Backend(object(), 1, 0, 4321)
        calls: list[tuple[int, str]] = []

        def rows(head: int, kind: str) -> list[dict[str, object]]:
            calls.append((head, kind))
            return [{"pointer": 0x1000 if kind == "local" else 0x2000, "varinfo_pointer": 0x3000 if kind == "local" else 0x4000}]

        with mock.patch.object(backend, "_snapshot_list", side_effect=rows):
            inventory = backend.snapshot_inventory()
        self.assertEqual(calls, [
            (MODULE.LOCALS_LIST_HEAD, "local"),
            (MODULE.ARGUMENTS_LIST_HEAD, "argument"),
        ])
        self.assertEqual(set(inventory), {"locals", "arguments"})

        with mock.patch.object(backend, "_u32_optional", return_value=0x5000) as read_global:
            with mock.patch.object(backend, "_read_name", return_value="mbCapListDebug"):
                self.assertEqual(backend.current_function(), "mbCapListDebug")
        read_global.assert_called_once_with(backend._runtime(MODULE.FUNCTION_OBJECT))

    def test_native_backend_uses_authenticated_write_register_layout(self) -> None:
        backend = MODULE.NativeWow64Backend(object(), 1, 0, 4321)
        backend._object_kind[0x1000] = "local"
        with mock.patch.object(backend, "read_register", side_effect=lambda _thread, name: {"ebx": 0x1000, "eax": 0x2E}[name]) as read:
            row = backend.capture_stack_write("object_write_1", 7)
        self.assertEqual(row, {"object": 0x1000, "value": 0x2E, "kind": "local"})
        self.assertEqual(read.call_args_list, [mock.call(7, "ebx"), mock.call(7, "eax")])
        with self.assertRaisesRegex(MODULE.Rejected, "unowned stack-write hook"):
            backend.capture_stack_write("regalloc", 7)

    def test_native_backend_captures_and_confirms_gc27_pcode_color_without_serializing_pointers(self) -> None:
        backend = MODULE.NativeWow64Backend(
            object(), 1, 0, 4321, compiler_sha256=MODULE.GC27_COMPILER_SHA256
        )
        registers = {"edx": 0x1030, "esi": 0x1000, "ecx": 0x2000, "ebp": 1, "eax": 17}
        operand = bytes((2, 4, 0, 0, 17, 0))
        node = bytearray(0x18)
        node[4:8] = (0x1010).to_bytes(4, "little")
        node[0x14:0x16] = (17).to_bytes(2, "little", signed=True)

        def read(address: int, size: int) -> bytes:
            rows = {
                (0x1022, 2): (3).to_bytes(2, "little", signed=True),
                (0x1030, 6): operand,
                (0x2000, 0x18): bytes(node),
                (0x1034, 2): (17).to_bytes(2, "little", signed=True),
            }
            return rows[(address, size)]

        with (
            mock.patch.object(backend, "read_register", side_effect=lambda _thread, name: registers[name]),
            mock.patch.object(backend, "_read", side_effect=read),
        ):
            pre = backend.capture_pcode("pcode_color_pre", 7)
            post = backend.capture_pcode("pcode_color_post", 7)
            noop = backend.capture_pcode("pcode_color_post", 8)
        self.assertEqual(pre["status"], "PENDING")
        self.assertEqual(post["status"], "CAPTURED")
        self.assertEqual(post["operand_index"], 17)
        self.assertEqual(post["object_pointer"], 0x1010)
        self.assertEqual(noop, {"status": "NOOP"})

    def test_native_backend_selects_authenticated_compiler_child_not_wrapper_parent(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            wrapper = root / "sjiswrap.exe"
            compiler = root / "mwcceppc.exe"
            wrapper.write_bytes(b"wrapper")
            compiler.write_bytes(b"compiler")
            backend = MODULE.NativeWow64Backend(
                object(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                wrapper_path=str(wrapper),
                wrapper_sha256=hashlib.sha256(wrapper.read_bytes()).hexdigest(),
            )
            self.assertEqual(backend.classify_debug_image(100, str(wrapper)), "wrapper")
            self.assertEqual(backend.classify_debug_image(101, str(compiler)), "compiler")
            with self.assertRaisesRegex(MODULE.Rejected, "unrecognized debugged image"):
                backend.classify_debug_image(102, str(root / "other.exe"))

            class Info:
                hProcess = 21
                hThread = 22
                lpBaseOfImage = 0x00400000
                hFile = 0

            backend._transport_image_seen = True
            started: list[int] = []

            class Session:
                def on_process_started(self, process_id: int) -> None:
                    started.append(process_id)

            backend._select_compiler_process(101, 55, Info(), str(compiler), Session())
            self.assertTrue(backend.compiler_selected)
            self.assertEqual(backend.compiler_process_id, 101)
            self.assertEqual(backend.process_id, 101)
            self.assertEqual(backend.process, 21)
            self.assertEqual(backend.loader_breakpoints_remaining, 1)
            self.assertEqual(started, [101])

    def test_native_backend_binds_sjiswrap_manual_map_in_same_pid(self) -> None:
        """sjiswrap v1.1.1 maps mwcceppc instead of creating a child PID."""

        with TemporaryDirectory() as directory:
            root = Path(directory)
            wrapper = root / "sjiswrap.exe"
            compiler = root / "mwcceppc.exe"
            wrapper.write_bytes(b"wrapper")
            compiler.write_bytes(b"compiler")
            backend = MODULE.NativeWow64Backend(
                object(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                wrapper_path=str(wrapper),
                wrapper_sha256=hashlib.sha256(wrapper.read_bytes()).hexdigest(),
            )
            backend._transport_image_seen = True
            backend._pause_same_process_target = mock.Mock()
            started: list[int] = []

            class Session:
                def on_process_started(self, process_id: int) -> None:
                    started.append(process_id)

            backend._select_memexec_process(0x12000000, Session(), object())
            self.assertTrue(backend.compiler_selected)
            self.assertEqual(backend._selection_mode, "same_process_memexec")
            self.assertEqual(backend.compiler_process_id, 100)
            self.assertEqual(backend.process_id, 100)
            self.assertEqual(backend.process, 11)
            self.assertEqual(backend.base, 0x12000000)
            self.assertEqual(started, [100])
            backend._pause_same_process_target.assert_called_once()

    def test_native_backend_memexec_discovery_requires_all_pinned_hooks(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            compiler = root / "mwcceppc.exe"
            compiler.write_bytes(b"compiler")
            backend = MODULE.NativeWow64Backend(
                object(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
            )
            backend._compiler_pe_shape = mock.Mock(return_value=(MODULE.KNOWN_IMAGE_BASE, 0x200000))
            backend._compiler_relocation_rvas = mock.Mock(return_value=())
            backend._memory_regions = mock.Mock(return_value=[(0x00400000, 0x1000), (0x12000000, 0x200000)])
            backend._memexec_candidate_matches = mock.Mock(side_effect=[False, True])
            self.assertEqual(backend._discover_memexec_image_base(), 0x12000000)
            self.assertEqual(backend._memexec_candidate_matches.call_count, 2)
            backend._memexec_candidate_matches.reset_mock()
            self.assertEqual(backend._discover_memexec_image_base(), 0x12000000)
            backend._memexec_candidate_matches.assert_not_called()

    def test_native_backend_accepts_section_only_memexec_map(self) -> None:
        """memexec copies sections, not the PE headers, into its map."""

        backend = MODULE.NativeWow64Backend(object(), 11, 0, 100)
        base = 0x12000000
        image_size = 0x200000

        def read(_handle: int, address: int, size: int) -> bytes:
            for row in MODULE.HOOKS:
                hook_address = base + int(row["address"]) - MODULE.KNOWN_IMAGE_BASE
                if address == hook_address:
                    return bytes.fromhex(str(row["prefix"]))
            return b"\0" * size

        backend._read_from_handle = mock.Mock(side_effect=read)
        self.assertTrue(backend._memexec_candidate_matches(base, image_size))
        self.assertEqual(backend._read_from_handle.call_count, len(MODULE.HOOKS))

        # The mapped image has no PE header; a complete hook union is the
        # authenticated in-memory identity for this loader.
        first_hook_address = base + int(MODULE.HOOKS[0]["address"]) - MODULE.KNOWN_IMAGE_BASE
        backend._read_from_handle.side_effect = lambda _handle, address, size: (
            b"\0" * size if address == first_hook_address else read(_handle, address, size)
        )
        self.assertFalse(backend._memexec_candidate_matches(base, image_size))

    def test_native_backend_accepts_rebased_highlow_hook_prefix(self) -> None:
        """A relocated absolute hook operand must match its mapped bytes."""

        backend = MODULE.NativeWow64Backend(object(), 11, 0, 100)
        preferred = MODULE.KNOWN_IMAGE_BASE
        base = preferred + 0x123000
        image_size = 0x200000
        backend._compiler_preferred_base = preferred
        row = next(item for item in MODULE.HOOKS if item["id"] == "allocation_post")
        hook_rva = int(row["address"]) - preferred
        relocation_rva = hook_rva + 6
        backend._compiler_relocation_rvas_cache = (relocation_rva,)
        expected = bytearray(bytes.fromhex(str(row["prefix"])))
        original = int.from_bytes(expected[6:10], "little")
        expected[6:10] = ((original + (base - preferred)) & 0xFFFFFFFF).to_bytes(4, "little")

        def read(_handle: int, address: int, size: int) -> bytes:
            for hook in MODULE.HOOKS:
                hook_address = base + int(hook["address"]) - preferred
                if address == hook_address:
                    if hook["id"] == row["id"]:
                        return bytes(expected)
                    return bytes.fromhex(str(hook["prefix"]))
            return b"\0" * size

        backend._read_from_handle = mock.Mock(side_effect=read)
        self.assertTrue(backend._memexec_candidate_matches(base, image_size))
        expected[6] ^= 0x01
        self.assertFalse(backend._memexec_candidate_matches(base, image_size))

    def test_native_backend_accepts_partial_rebased_highlow_hook_prefix(self) -> None:
        """A pinned prefix may end inside an authenticated relocated dword."""

        backend = MODULE.NativeWow64Backend(object(), 11, 0, 100)
        preferred = MODULE.KNOWN_IMAGE_BASE
        base = preferred + 0x123000
        backend._compiler_preferred_base = preferred
        row = {"address": preferred + 0x1000, "prefix": "aabbcc"}
        relocation_rva = 0x1001
        raw_value = 0x11223344
        backend._compiler_relocation_rvas_cache = (relocation_rva,)
        backend._compiler_relocation_values_cache = {relocation_rva: raw_value}
        delta = base - preferred
        patched = ((raw_value + delta) & 0xFFFFFFFF).to_bytes(4, "little")
        self.assertEqual(
            backend._mapped_hook_prefix(row, base),
            bytes([0xAA, patched[0], patched[1]]),
        )

    def test_dispatcher_preflight_uses_rebased_memexec_hook_prefix(self) -> None:
        """Preflight must compare relocated live bytes, not stale preferred-base bytes."""

        backend = MODULE.NativeWow64Backend(object(), 11, 0, 100)
        preferred = MODULE.KNOWN_IMAGE_BASE
        base = preferred + 0x123000
        backend.base = base
        backend._compiler_preferred_base = preferred
        row = next(item for item in MODULE.HOOKS if item["id"] == "allocation_post")
        hook_rva = int(row["address"]) - preferred
        relocation_rva = hook_rva + 7
        static = bytes.fromhex(str(row["prefix"]))
        raw_value = int.from_bytes(static[7:11], "little")
        backend._compiler_relocation_rvas_cache = (relocation_rva,)
        backend._compiler_relocation_values_cache = {relocation_rva: raw_value}
        mapped = backend.expected_hook_prefix(row)
        self.assertNotEqual(mapped, static)

        by_address = {int(item["address"]): item for item in MODULE.HOOKS}
        backend.read_image = mock.Mock(
            side_effect=lambda address, size: backend.expected_hook_prefix(by_address[int(address)])[:size]
        )
        dispatcher = MODULE.SharedBreakpointDispatcher(backend, mock.Mock())
        dispatcher.preflight()
        self.assertTrue(dispatcher.prefixes_validated)
        self.assertEqual(backend.read_image.call_count, len(MODULE.HOOKS))

    def test_native_backend_probes_silent_wrapper_before_exit_race(self) -> None:
        """A quiet manual-map interval must get an authenticated observation."""

        import ctypes

        with TemporaryDirectory() as directory:
            root = Path(directory)
            wrapper = root / "sjiswrap.exe"
            compiler = root / "mwcceppc.exe"
            wrapper.write_bytes(b"wrapper")
            compiler.write_bytes(b"compiler")

            class CreateProcessInfo(ctypes.Structure):
                _fields_ = [
                    ("hProcess", ctypes.c_void_p),
                    ("hThread", ctypes.c_void_p),
                    ("lpBaseOfImage", ctypes.c_void_p),
                    ("hFile", ctypes.c_void_p),
                ]

            class EventUnion(ctypes.Union):
                _fields_ = [("CreateProcessInfo", CreateProcessInfo)]

            class DebugEvent(ctypes.Structure):
                _fields_ = [
                    ("dwDebugEventCode", ctypes.c_uint32),
                    ("dwProcessId", ctypes.c_uint32),
                    ("dwThreadId", ctypes.c_uint32),
                    ("u", EventUnion),
                ]

            class Kernel:
                def __init__(self) -> None:
                    self.events = ["create", "timeout", "exception"]
                    self.continues: list[tuple[int, int, int]] = []
                    self.debug_breaks: list[int] = []

                def WaitForDebugEvent(self, event_pointer: object, _timeout: int) -> bool:
                    kind = self.events.pop(0)
                    if kind == "timeout":
                        return False
                    event = ctypes.cast(event_pointer, ctypes.POINTER(DebugEvent)).contents
                    event.dwProcessId = 100
                    event.dwThreadId = 7
                    event.dwDebugEventCode = 3 if kind == "create" else 1
                    if kind == "create":
                        event.u.CreateProcessInfo.hProcess = 21
                        event.u.CreateProcessInfo.hThread = 22
                        event.u.CreateProcessInfo.hFile = 0
                    return True

                def ContinueDebugEvent(self, process_id: int, thread_id: int, status: int) -> bool:
                    self.continues.append((int(process_id), int(thread_id), int(status)))
                    return True

                def DebugBreakProcess(self, process_handle: object) -> bool:
                    self.debug_breaks.append(int(getattr(process_handle, "value", process_handle)))
                    return True

                def CloseHandle(self, _handle: object) -> bool:
                    return True

            kernel = Kernel()

            class Native:
                DEBUG_EVENT = DebugEvent
                ERROR_SEM_TIMEOUT = 121
                DBG_CONTINUE = 0x00010002
                kernel32 = kernel

            backend = MODULE.NativeWow64Backend(
                Native(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                wrapper_path=str(wrapper),
                wrapper_sha256=hashlib.sha256(wrapper.read_bytes()).hexdigest(),
            )
            backend._query_process_image_path = mock.Mock(return_value=str(wrapper))
            backend._discover_memexec_image_base = mock.Mock(side_effect=[None, None, 0x12000000])
            selected: list[int] = []

            def select(base: int, _session: object, _event_type: object) -> None:
                selected.append(base)
                backend.compiler_selected = True
                backend.compiler_process_id = backend.transport_process_id

            backend._select_memexec_process = mock.Mock(side_effect=select)
            session = mock.Mock()
            with mock.patch.object(MODULE.ctypes, "get_last_error", return_value=121):
                backend.prepare_capture(session)

            self.assertEqual(selected, [0x12000000])
            self.assertEqual(kernel.debug_breaks, [11])
            self.assertEqual(
                [entry[2] for entry in kernel.continues],
                [Native.DBG_CONTINUE, Native.DBG_CONTINUE],
            )
            backend._select_memexec_process.assert_called_once()

    def test_native_backend_probes_wrapper_before_fast_exit_race(self) -> None:
        """The probe must run immediately after wrapper CREATE_PROCESS."""

        import ctypes

        with TemporaryDirectory() as directory:
            root = Path(directory)
            wrapper = root / "sjiswrap.exe"
            compiler = root / "mwcceppc.exe"
            wrapper.write_bytes(b"wrapper")
            compiler.write_bytes(b"compiler")

            class CreateProcessInfo(ctypes.Structure):
                _fields_ = [
                    ("hProcess", ctypes.c_void_p),
                    ("hThread", ctypes.c_void_p),
                    ("lpBaseOfImage", ctypes.c_void_p),
                    ("hFile", ctypes.c_void_p),
                ]

            class EventUnion(ctypes.Union):
                _fields_ = [("CreateProcessInfo", CreateProcessInfo)]

            class DebugEvent(ctypes.Structure):
                _fields_ = [
                    ("dwDebugEventCode", ctypes.c_uint32),
                    ("dwProcessId", ctypes.c_uint32),
                    ("dwThreadId", ctypes.c_uint32),
                    ("u", EventUnion),
                ]

            class Kernel:
                def __init__(self) -> None:
                    self.events = ["create"]
                    self.continues: list[tuple[int, int, int]] = []
                    self.debug_breaks: list[int] = []

                def WaitForDebugEvent(self, event_pointer: object, _timeout: int) -> bool:
                    kind = self.events.pop(0)
                    event = ctypes.cast(event_pointer, ctypes.POINTER(DebugEvent)).contents
                    event.dwProcessId = 100
                    event.dwThreadId = 7
                    event.dwDebugEventCode = 3 if kind == "create" else 1
                    if kind == "create":
                        event.u.CreateProcessInfo.hProcess = 21
                        event.u.CreateProcessInfo.hThread = 22
                        event.u.CreateProcessInfo.hFile = 0
                    return True

                def ContinueDebugEvent(self, process_id: int, thread_id: int, status: int) -> bool:
                    self.continues.append((int(process_id), int(thread_id), int(status)))
                    return True

                def DebugBreakProcess(self, process_handle: object) -> bool:
                    self.debug_breaks.append(int(getattr(process_handle, "value", process_handle)))
                    self.events.insert(0, "exception")
                    return True

                def CloseHandle(self, _handle: object) -> bool:
                    return True

            kernel = Kernel()

            class Native:
                DEBUG_EVENT = DebugEvent
                ERROR_SEM_TIMEOUT = 121
                DBG_CONTINUE = 0x00010002
                kernel32 = kernel

            backend = MODULE.NativeWow64Backend(
                Native(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                wrapper_path=str(wrapper),
                wrapper_sha256=hashlib.sha256(wrapper.read_bytes()).hexdigest(),
            )
            backend._query_process_image_path = mock.Mock(return_value=str(wrapper))
            backend._discover_memexec_image_base = mock.Mock(side_effect=[None, 0x12000000])
            selected: list[int] = []

            def select(base: int, _session: object, _event_type: object) -> None:
                selected.append(base)
                backend.compiler_selected = True
                backend.compiler_process_id = backend.transport_process_id

            backend._select_memexec_process = mock.Mock(side_effect=select)
            backend.prepare_capture(mock.Mock())

            self.assertEqual(selected, [0x12000000])
            self.assertEqual(kernel.debug_breaks, [11])
            self.assertEqual(
                [entry[2] for entry in kernel.continues],
                [Native.DBG_CONTINUE, Native.DBG_CONTINUE],
            )
            backend._select_memexec_process.assert_called_once()

    def test_native_backend_retains_same_pid_thread_handle_before_pause(self) -> None:
        """A worker CREATE_THREAD event must back the first native single-step."""

        import ctypes

        with TemporaryDirectory() as directory:
            root = Path(directory)
            wrapper = root / "sjiswrap.exe"
            compiler = root / "mwcceppc.exe"
            wrapper.write_bytes(b"wrapper")
            compiler.write_bytes(b"compiler")

            class CreateProcessInfo(ctypes.Structure):
                _fields_ = [
                    ("hProcess", ctypes.c_void_p),
                    ("hThread", ctypes.c_void_p),
                    ("lpBaseOfImage", ctypes.c_void_p),
                    ("hFile", ctypes.c_void_p),
                ]

            class CreateThreadInfo(ctypes.Structure):
                _fields_ = [("hThread", ctypes.c_void_p)]

            class EventUnion(ctypes.Union):
                _fields_ = [
                    ("CreateProcessInfo", CreateProcessInfo),
                    ("CreateThread", CreateThreadInfo),
                ]

            class DebugEvent(ctypes.Structure):
                _fields_ = [
                    ("dwDebugEventCode", ctypes.c_uint32),
                    ("dwProcessId", ctypes.c_uint32),
                    ("dwThreadId", ctypes.c_uint32),
                    ("u", EventUnion),
                ]

            class Kernel:
                def __init__(self) -> None:
                    self.events = ["create", "create_thread", "exception"]
                    self.continues: list[tuple[int, int, int]] = []
                    self.debug_breaks: list[int] = []

                def WaitForDebugEvent(self, event_pointer: object, _timeout: int) -> bool:
                    kind = self.events.pop(0)
                    event = ctypes.cast(event_pointer, ctypes.POINTER(DebugEvent)).contents
                    event.dwProcessId = 100
                    event.dwThreadId = 7 if kind == "create" else 8
                    if kind == "create":
                        event.dwDebugEventCode = 3
                        event.u.CreateProcessInfo.hProcess = 21
                        event.u.CreateProcessInfo.hThread = 22
                        event.u.CreateProcessInfo.hFile = 0
                    elif kind == "create_thread":
                        event.dwDebugEventCode = 2
                        event.u.CreateThread.hThread = 33
                    else:
                        event.dwDebugEventCode = 1
                    return True

                def ContinueDebugEvent(self, process_id: int, thread_id: int, status: int) -> bool:
                    self.continues.append((int(process_id), int(thread_id), int(status)))
                    return True

                def DebugBreakProcess(self, process_handle: object) -> bool:
                    self.debug_breaks.append(int(getattr(process_handle, "value", process_handle)))
                    return True

                def CloseHandle(self, _handle: object) -> bool:
                    return True

            kernel = Kernel()

            class Native:
                DEBUG_EVENT = DebugEvent
                ERROR_SEM_TIMEOUT = 121
                DBG_CONTINUE = 0x00010002
                kernel32 = kernel

            backend = MODULE.NativeWow64Backend(
                Native(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                wrapper_path=str(wrapper),
                wrapper_sha256=hashlib.sha256(wrapper.read_bytes()).hexdigest(),
            )
            backend._query_process_image_path = mock.Mock(return_value=str(wrapper))
            backend._discover_memexec_image_base = mock.Mock(side_effect=[None, 0x12000000])

            backend.prepare_capture(mock.Mock())

            self.assertEqual(backend.transport_threads[8], 33)
            self.assertEqual(backend.threads[8], 33)
            self.assertEqual(backend._pending_debug_event, (100, 8))
            self.assertEqual(kernel.debug_breaks, [11, 11])

    def test_native_backend_reprobes_after_wrapper_worker_event_gap(self) -> None:
        """A worker can outrun the first probe before its manual map is visible."""

        import ctypes

        with TemporaryDirectory() as directory:
            root = Path(directory)
            wrapper = root / "sjiswrap.exe"
            compiler = root / "mwcceppc.exe"
            wrapper.write_bytes(b"wrapper")
            compiler.write_bytes(b"compiler")

            class CreateProcessInfo(ctypes.Structure):
                _fields_ = [
                    ("hProcess", ctypes.c_void_p),
                    ("hThread", ctypes.c_void_p),
                    ("lpBaseOfImage", ctypes.c_void_p),
                    ("hFile", ctypes.c_void_p),
                ]

            class CreateThreadInfo(ctypes.Structure):
                _fields_ = [("hThread", ctypes.c_void_p)]

            class EventUnion(ctypes.Union):
                _fields_ = [
                    ("CreateProcessInfo", CreateProcessInfo),
                    ("CreateThread", CreateThreadInfo),
                ]

            class DebugEvent(ctypes.Structure):
                _fields_ = [
                    ("dwDebugEventCode", ctypes.c_uint32),
                    ("dwProcessId", ctypes.c_uint32),
                    ("dwThreadId", ctypes.c_uint32),
                    ("u", EventUnion),
                ]

            class Kernel:
                def __init__(self) -> None:
                    # The first probe's exception is queued after the worker
                    # event.  The second exception is produced only when the
                    # bounded follow-up DebugBreakProcess is requested.
                    self.events = ["create", "create_thread", "exception"]
                    self.observed: list[str] = []
                    self.continues: list[tuple[int, int, int]] = []
                    self.debug_breaks: list[int] = []

                def WaitForDebugEvent(self, event_pointer: object, _timeout: int) -> bool:
                    if not self.events:
                        raise AssertionError("preflight entered an unbounded debug-event gap")
                    kind = self.events.pop(0)
                    self.observed.append(kind)
                    event = ctypes.cast(event_pointer, ctypes.POINTER(DebugEvent)).contents
                    event.dwProcessId = 100
                    event.dwThreadId = 7 if kind == "create" else 8
                    if kind == "create":
                        event.dwDebugEventCode = 3
                        event.u.CreateProcessInfo.hProcess = 21
                        event.u.CreateProcessInfo.hThread = 22
                        event.u.CreateProcessInfo.hFile = 0
                    elif kind == "create_thread":
                        event.dwDebugEventCode = 2
                        event.u.CreateThread.hThread = 33
                    else:
                        event.dwDebugEventCode = 1
                    return True

                def ContinueDebugEvent(self, process_id: int, thread_id: int, status: int) -> bool:
                    self.continues.append((int(process_id), int(thread_id), int(status)))
                    return True

                def DebugBreakProcess(self, process_handle: object) -> bool:
                    self.debug_breaks.append(int(getattr(process_handle, "value", process_handle)))
                    if len(self.debug_breaks) == 2:
                        self.events.append("exception")
                    return True

                def CloseHandle(self, _handle: object) -> bool:
                    return True

            kernel = Kernel()

            class Native:
                DEBUG_EVENT = DebugEvent
                ERROR_SEM_TIMEOUT = 121
                DBG_CONTINUE = 0x00010002
                kernel32 = kernel

            backend = MODULE.NativeWow64Backend(
                Native(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                wrapper_path=str(wrapper),
                wrapper_sha256=hashlib.sha256(wrapper.read_bytes()).hexdigest(),
            )
            backend._query_process_image_path = mock.Mock(return_value=str(wrapper))
            backend._discover_memexec_image_base = mock.Mock(
                side_effect=[None, None, None, 0x12000000]
            )
            selected: list[int] = []

            def select(base: int, _session: object, _event_type: object) -> None:
                selected.append(base)
                backend.compiler_selected = True
                backend.compiler_process_id = backend.transport_process_id

            backend._select_memexec_process = mock.Mock(side_effect=select)
            backend.prepare_capture(mock.Mock())

            self.assertEqual(kernel.observed, ["create", "create_thread", "exception", "exception"])
            self.assertEqual(selected, [0x12000000])
            self.assertEqual(kernel.debug_breaks, [11, 11])
            self.assertEqual(backend.transport_threads[8], 33)
            backend._select_memexec_process.assert_called_once()

    def test_native_backend_promotes_worker_handle_before_first_hook_step(self) -> None:
        """A delayed same-PID selection must retain a worker's first-hook handle."""

        import ctypes

        with TemporaryDirectory() as directory:
            root = Path(directory)
            wrapper = root / "sjiswrap.exe"
            compiler = root / "mwcceppc.exe"
            wrapper.write_bytes(b"wrapper")
            compiler.write_bytes(b"compiler")

            class CreateProcessInfo(ctypes.Structure):
                _fields_ = [
                    ("hProcess", ctypes.c_void_p),
                    ("hThread", ctypes.c_void_p),
                    ("lpBaseOfImage", ctypes.c_void_p),
                    ("hFile", ctypes.c_void_p),
                ]

            class CreateThreadInfo(ctypes.Structure):
                _fields_ = [("hThread", ctypes.c_void_p)]

            class EventUnion(ctypes.Union):
                _fields_ = [
                    ("CreateProcessInfo", CreateProcessInfo),
                    ("CreateThread", CreateThreadInfo),
                ]

            class DebugEvent(ctypes.Structure):
                _fields_ = [
                    ("dwDebugEventCode", ctypes.c_uint32),
                    ("dwProcessId", ctypes.c_uint32),
                    ("dwThreadId", ctypes.c_uint32),
                    ("u", EventUnion),
                ]

            class Context(ctypes.Structure):
                _fields_ = [
                    ("ContextFlags", ctypes.c_uint32),
                    ("Eip", ctypes.c_uint32),
                    ("EFlags", ctypes.c_uint32),
                ]

            class Kernel:
                def __init__(self) -> None:
                    # The two probe breaks are delivered after the wrapper
                    # CREATE_PROCESS and worker CREATE_THREAD events.  The
                    # selection pause is a third, distinct observation.
                    self.events = ["create", "create_thread"]
                    self.observed: list[str] = []
                    self.debug_breaks: list[int] = []
                    self.context_reads: list[int] = []
                    self.context_writes: list[int] = []

                def WaitForDebugEvent(self, event_pointer: object, _timeout: int) -> bool:
                    if not self.events:
                        raise AssertionError("preflight entered an unbounded debug-event gap")
                    kind = self.events.pop(0)
                    self.observed.append(kind)
                    event = ctypes.cast(event_pointer, ctypes.POINTER(DebugEvent)).contents
                    event.dwProcessId = 100
                    event.dwThreadId = 8 if kind == "create_thread" else 7
                    if kind == "create":
                        event.dwDebugEventCode = 3
                        event.u.CreateProcessInfo.hProcess = 21
                        event.u.CreateProcessInfo.hThread = 22
                        event.u.CreateProcessInfo.hFile = 0
                    elif kind == "create_thread":
                        event.dwDebugEventCode = 2
                        event.u.CreateThread.hThread = 33
                    else:
                        event.dwDebugEventCode = 1
                    return True

                def ContinueDebugEvent(self, process_id: int, thread_id: int, status: int) -> bool:
                    return True

                def DebugBreakProcess(self, process_handle: object) -> bool:
                    self.debug_breaks.append(int(getattr(process_handle, "value", process_handle)))
                    if len(self.debug_breaks) < 3:
                        self.events.append("probe_exception")
                    else:
                        self.events.append("pause_exception")
                    return True

                def CloseHandle(self, _handle: object) -> bool:
                    return True

                def Wow64GetThreadContext(self, handle: int, context_pointer: object) -> bool:
                    self.context_reads.append(int(handle))
                    context = ctypes.cast(context_pointer, ctypes.POINTER(Context)).contents
                    context.EFlags = 0
                    return True

                def Wow64SetThreadContext(self, handle: int, _context_pointer: object) -> bool:
                    self.context_writes.append(int(handle))
                    return True

            kernel = Kernel()

            class Native:
                DEBUG_EVENT = DebugEvent
                WOW64_CONTEXT = Context
                ERROR_SEM_TIMEOUT = 121
                DBG_CONTINUE = 0x00010002
                WOW64_CONTEXT_FULL = 0x00010007
                WOW64_CONTEXT_TF = 0x100
                kernel32 = kernel

            backend = MODULE.NativeWow64Backend(
                Native(),
                11,
                12,
                100,
                compiler_path=str(compiler),
                compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                wrapper_path=str(wrapper),
                wrapper_sha256=hashlib.sha256(wrapper.read_bytes()).hexdigest(),
            )
            backend._query_process_image_path = mock.Mock(return_value=str(wrapper))
            backend._discover_memexec_image_base = mock.Mock(
                side_effect=[None, None, None, 0x12000000]
            )
            backend.prepare_capture(mock.Mock())

            self.assertEqual(
                kernel.observed,
                ["create", "create_thread", "probe_exception", "probe_exception", "pause_exception"],
            )
            self.assertEqual(kernel.debug_breaks, [11, 11, 11])
            self.assertEqual(backend._pending_debug_event, (100, 7))
            self.assertEqual(backend.transport_threads[7], 22)
            self.assertEqual(backend.transport_threads[8], 33)
            self.assertEqual(backend.threads[7], 22)
            self.assertEqual(backend.threads[8], 33)

            backend.remove_breakpoint = mock.Mock()
            backend.single_step(MODULE.KNOWN_IMAGE_BASE, 8, rearm=False)
            self.assertEqual(kernel.context_reads, [33])
            self.assertEqual(kernel.context_writes, [33])

    def test_direct_object_to_vreg_evidence_is_one_to_one_and_pointer_local(self) -> None:
        class NativeWithEvidence:
            def read_direct_vreg_evidence(self, hook_id: str, thread_id: int) -> list[dict[str, object]]:
                self.last = (hook_id, thread_id)
                return [{"object": 0x1000, "ig_node": 0x2000, "vreg_id": "r32", "bank": "GPR"}]

        native = NativeWithEvidence()
        backend = MODULE.NativeWow64Backend(native, 1, 0, 4321)
        backend._object_kind[0x1000] = "local"
        rows = backend.capture_regalloc("regalloc", 9)
        self.assertEqual(rows, [{"object": 0x1000, "ig_node": 0x2000, "vreg_id": "r32", "bank": "GPR"}])
        self.assertEqual(native.last, ("regalloc", 9))
        with self.assertRaisesRegex(MODULE.Rejected, "unowned register-allocation hook"):
            backend.capture_regalloc("object_write_1", 9)

        class NativeWithPhysicalEvidence:
            def read_direct_vreg_evidence(self, _hook_id: str, _thread_id: int) -> list[dict[str, object]]:
                return [{"object": 0x1000, "ig_node": 0x2000, "vreg_id": "r3", "bank": "GPR"}]

        physical_backend = MODULE.NativeWow64Backend(NativeWithPhysicalEvidence(), 1, 0, 4321)
        physical_backend._object_kind[0x1000] = "local"
        with self.assertRaisesRegex(MODULE.Rejected, "physical register"):
            physical_backend.capture_regalloc("regalloc", 9)

    def test_physical_regalloc_contract_validates_varinfo_fields_and_conflicts(self) -> None:
        def session_for(root: Path) -> MODULE.CombinedCaptureSession:
            session = MODULE.CombinedCaptureSession(auth(root), FakeBackend())
            session._capture_inventory()
            return session

        with TemporaryDirectory() as directory:
            root = Path(directory)
            base = {
                "object": 0x1010,
                "varinfo_pointer": 0x5010,
                "noregister": 0,
                "flags": 2,
                "rclass": 4,
                "reg": 17,
                "reg_hi": 0,
            }
            session = session_for(root)
            self.assertEqual(
                session._normalize_physical_mapping(base),
                {
                    "object_token": "local-session-0000000000000001-000000",
                    "status": "EXACT",
                    "physical_reg": 17,
                    "bank": "GPR",
                },
            )
            self.assertEqual(
                session._normalize_physical_mapping(
                    dict(base, object=0x1020, varinfo_pointer=0x5020, reg=18)
                ),
                {
                    "object_token": "local-session-0000000000000001-000001",
                    "status": "EXACT",
                    "physical_reg": 18,
                    "bank": "GPR",
                },
            )
            session = session_for(root)
            self.assertEqual(session._normalize_physical_mapping(base)["status"], "EXACT")
            self.assertIsNone(session._normalize_physical_mapping(dict(base, reg=18)))
            self.assertIn("duplicate physical-register assignment", session.unknown)
            self.assertIsNone(
                session._normalize_physical_mapping(
                    dict(
                        base,
                        object_token="local-session-ffffffffffffffff-000000",
                        object=None,
                    )
                )
            )
            self.assertIn("null object identity", session.unknown)
            self.assertIsNone(
                session._normalize_physical_mapping(
                    dict(base, object=0x1020, varinfo_pointer=0x5020)
                )
            )
            self.assertIn("one-to-many physical-register-to-object claim", session.unknown)

            cases = (
                ("varinfo_pointer", 0x9999, "missing or invalid Object VarInfo pointer"),
                ("noregister", 1, "physical register marked no-register"),
                ("flags", 0, "physical register flags missing assignment bit"),
                ("rclass", 4 + 1, "unsupported physical register class"),
                ("reg", 32, "physical register index out of range"),
                ("reg_hi", -1, "physical register index out of range"),
            )
            for field, value, reason in cases:
                isolated = session_for(root)
                self.assertIsNone(
                    isolated._normalize_physical_mapping(dict(base, **{field: value})),
                    field,
                )
                self.assertIn(reason, isolated.unknown, field)

    def test_repeated_post_hook_rows_remain_in_same_target_window(self) -> None:
        rows = [
            {
                "object": 0x1010,
                "varinfo_pointer": 0x5010,
                "noregister": 0,
                "flags": 2,
                "rclass": 4,
                "reg": 17,
                "reg_hi": 0,
            },
            {
                "object": 0x1020,
                "varinfo_pointer": 0x5020,
                "noregister": 0,
                "flags": 2,
                "rclass": 3,
                "reg": 9,
                "reg_hi": 0,
            },
        ]
        with TemporaryDirectory() as directory:
            envelope = MODULE.CombinedCaptureSession(
                auth(Path(directory)), FakeBackend(physical_rows=rows)
            ).run()
        physical = [
            event for event in envelope["events"]
            if event["event_kind"] == "physical_reg_assignment"
        ]
        self.assertEqual(
            [(event["object_token"], event["physical_reg"], event["bank"]) for event in physical],
            [
                ("local-session-0000000000000001-000000", 17, "GPR"),
                ("local-session-0000000000000001-000001", 9, "FPR"),
            ],
        )

    def test_first_post_hook_defers_until_object_inventory_edge(self) -> None:
        with TemporaryDirectory() as directory:
            envelope = MODULE.CombinedCaptureSession(
                auth(Path(directory)), FakeBackend(physical_before_inventory=True)
            ).run()
        events = envelope["events"]
        physical = [event for event in events if event["event_kind"] == "physical_reg_assignment"]
        self.assertEqual(len(physical), 1)
        self.assertEqual(physical[0]["status"], "EXACT")
        self.assertEqual(physical[0]["object_token"], "local-session-0000000000000001-000000")
        self.assertLess(
            next(index for index, event in enumerate(events) if event["event_kind"] == "compiler_list"),
            next(index for index, event in enumerate(events) if event["event_kind"] == "physical_reg_assignment"),
        )

    def test_first_post_refreshes_initially_empty_object_inventory(self) -> None:
        with TemporaryDirectory() as directory:
            envelope = MODULE.CombinedCaptureSession(
                auth(Path(directory)),
                FakeBackend(physical_before_inventory=True, inventory_empty_first=True),
            ).run()
        events = envelope["events"]
        compiler_list = next(event for event in events if event["event_kind"] == "compiler_list")
        physical = [event for event in events if event["event_kind"] == "physical_reg_assignment"]
        self.assertEqual(len(compiler_list["locals"]), 2)
        self.assertEqual(len(compiler_list["arguments"]), 2)
        self.assertEqual(len(physical), 1)
        self.assertEqual(physical[0]["status"], "EXACT")
        self.assertEqual(physical[0]["object_token"], "local-session-0000000000000001-000000")

    def test_native_physical_regalloc_reads_verified_ebx_ebp_contract(self) -> None:
        class Native:
            pass

        backend = MODULE.NativeWow64Backend(Native(), 1, 0, 4321)
        backend._object_kind[0x1000] = "local"
        backend._object_varinfo[0x1000] = 0x2000
        backend._varinfo_object[0x2000] = 0x1000
        backend.read_register = mock.Mock(side_effect=lambda _thread, name: {"ebx": 0x1000, "ebp": 0x2000}[name])
        data = bytearray(0x2A)
        data[0x22] = 0
        data[0x24] = 2
        data[0x25] = 3
        struct.pack_into("<h", data, 0x26, 9)
        struct.pack_into("<h", data, 0x28, 0)
        backend._read = mock.Mock(return_value=bytes(data))
        self.assertEqual(
            backend.capture_physical_regalloc("regalloc_post", 7),
            {
                "hook_id": "regalloc_post",
                "object": 0x1000,
                "varinfo_pointer": 0x2000,
                "noregister": 0,
                "flags": 2,
                "rclass": 3,
                "reg": 9,
                "reg_hi": 0,
            },
        )
        backend.read_register = mock.Mock(side_effect=lambda _thread, name: {"ebx": 0x1000, "ebp": 0x3000}[name])
        with self.assertRaisesRegex(MODULE.Rejected, "VarInfo pointer"):
            backend.capture_physical_regalloc("regalloc_post", 7)

    def test_gc27_pair_physical_hook_cross_checks_and_stays_unknown(self) -> None:
        data = bytearray(0x2A)
        data[0x24] = 6
        data[0x25] = 4
        struct.pack_into("<h", data, 0x26, 17)
        struct.pack_into("<h", data, 0x28, 18)
        backend = MODULE.NativeWow64Backend(
            object(), 1, 0, 4321, compiler_sha256=MODULE.GC27_COMPILER_SHA256
        )
        backend._object_kind[0x1000] = "local"
        backend._object_varinfo[0x1000] = 0x2000
        backend._varinfo_object[0x2000] = 0x1000
        backend.read_register = mock.Mock(
            side_effect=lambda _thread, name: {
                "esi": 0x1000, "eax": 0x2000, "ebx": 18, "ebp": 17,
            }[name]
        )
        backend._read = mock.Mock(return_value=bytes(data))
        row = backend.capture_physical_regalloc("physical_pair_commit", 7)
        self.assertEqual(row["hook_id"], "physical_pair_commit")
        self.assertEqual(row["commit"], {"register_class": 4, "reg": 17, "reg_hi": 18})
        self.assertEqual(row["status"], "UNKNOWN")
        self.assertEqual(row["reason"], "paired physical register assignment unsupported")

    def test_gc27_single_physical_hook_ignores_stale_high_half(self) -> None:
        data = bytearray(0x2A)
        data[0x24] = 2
        data[0x25] = 3
        struct.pack_into("<h", data, 0x26, 17)
        struct.pack_into("<h", data, 0x28, 29)
        backend = MODULE.NativeWow64Backend(
            object(), 1, 0, 4321, compiler_sha256=MODULE.GC27_COMPILER_SHA256
        )
        backend._object_kind[0x1000] = "local"
        backend._object_varinfo[0x1000] = 0x2000
        backend._varinfo_object[0x2000] = 0x1000
        backend.read_register = mock.Mock(
            side_effect=lambda _thread, name: {
                "esi": 0x1000, "eax": 0x2000, "ebx": 3, "ebp": 17,
            }[name]
        )
        backend._read = mock.Mock(return_value=bytes(data))
        row = backend.capture_physical_regalloc("physical_single_commit", 7)
        self.assertNotIn("status", row)
        self.assertEqual(row["commit"], {"register_class": 3, "reg": 17, "reg_hi_ignored": True})
        self.assertEqual((row["rclass"], row["reg"], row["reg_hi"]), (3, 17, 0))

    def test_gc27_precolored_hook_reads_object_from_owned_list_node(self) -> None:
        backend = MODULE.NativeWow64Backend(
            object(), 1, 0, 4321, compiler_sha256=MODULE.GC27_COMPILER_SHA256
        )
        backend._object_kind[0x1000] = "local"
        backend._object_varinfo[0x1000] = 0x2000
        backend._varinfo_object[0x2000] = 0x1000
        backend.read_register = mock.Mock(
            side_effect=lambda _thread, name: {"ebx": 0x3000, "esi": 0x2000}[name]
        )
        data = bytearray(0x2A)
        data[0x24] = 2
        data[0x25] = 4
        struct.pack_into("<h", data, 0x26, 12)
        struct.pack_into("<h", data, 0x28, 0)

        def read(address: int, size: int) -> bytes:
            if (address, size) == (0x3004, 4):
                return (0x1000).to_bytes(4, "little")
            if (address, size) == (0x300C, 2):
                return bytes((4, 12))
            if (address, size) == (0x2000, 0x2A):
                return bytes(data)
            self.fail(f"unexpected GC/2.7 physical read {address:#x}/{size}")

        backend._read = mock.Mock(side_effect=read)
        row = backend.capture_physical_regalloc("precolored_commit", 7)
        self.assertEqual(row["object"], 0x1000)
        self.assertEqual(row["varinfo_pointer"], 0x2000)
        self.assertEqual((row["rclass"], row["reg"], row["reg_hi"]), (4, 12, 0))
        self.assertEqual(row["commit"], {"register_class": 4, "reg": 12, "reg_hi": 0})

    def test_gc27_null_pair_and_single_objects_are_unknown_not_pointer_reinterpretation(self) -> None:
        for hook_id in ("physical_pair_commit", "physical_single_commit"):
            with self.subTest(hook_id=hook_id):
                backend = MODULE.NativeWow64Backend(
                    object(), 1, 0, 4321, compiler_sha256=MODULE.GC27_COMPILER_SHA256
                )
                backend.read_register = mock.Mock(
                    side_effect=lambda _thread, name: {
                        "esi": 0, "eax": 0x2000, "ebx": 18, "ebp": 17,
                    }[name]
                )
                row = backend.capture_physical_regalloc(hook_id, 7)
                self.assertEqual(row["hook_id"], hook_id)
                self.assertEqual(row["status"], "UNKNOWN")
                self.assertEqual(row["reason"], "null object identity")
                self.assertEqual(row["object"], 0)

    def test_gc27_live_single_commit_mismatch_is_unknown(self) -> None:
        data = bytearray(0x2A)
        data[0x24] = 2
        data[0x25] = 3
        struct.pack_into("<h", data, 0x26, 16)
        struct.pack_into("<h", data, 0x28, 0)
        backend = MODULE.NativeWow64Backend(
            object(), 1, 0, 4321, compiler_sha256=MODULE.GC27_COMPILER_SHA256
        )
        backend._object_kind[0x1000] = "local"
        backend._object_varinfo[0x1000] = 0x2000
        backend._varinfo_object[0x2000] = 0x1000
        backend.read_register = mock.Mock(
            side_effect=lambda _thread, name: {
                "esi": 0x1000, "eax": 0x2000, "ebx": 3, "ebp": 17,
            }[name]
        )
        backend._read = mock.Mock(return_value=bytes(data))
        row = backend.capture_physical_regalloc("physical_single_commit", 7)
        self.assertEqual(row["commit"], {"register_class": 3, "reg": 17, "reg_hi_ignored": True})
        self.assertEqual(row["status"], "UNKNOWN")
        self.assertEqual(row["reason"], "incomplete physical register evidence")


class SourceSpanTemplateNormalizerTests(unittest.TestCase):
    """Adversarial coverage for capture-local v2 source-span normalization."""

    @staticmethod
    def _fixture(root: Path) -> dict[str, object]:
        source_data = (
            b"void mbCapListDebug(void) {\n"
            b"    HuVecF posNorm;\n"
            b"    HuVecF pos;\n"
            b"    pos = posNorm;\n"
            b"}\n"
        )

        class MoveNumLikeBackend(FakeBackend):
            def snapshot_inventory(self) -> dict[str, list[dict[str, object]]]:
                inventory = super().snapshot_inventory()
                inventory["locals"][0]["name"] = "posNorm"
                inventory["locals"][1]["name"] = "pos"
                return inventory

        # Build the capture with the corrected split identity and source text
        # so the test covers the reviewed pos/write versus posNorm/read
        # template directly.
        request_path, envelope_path, envelope, trust = prepared_capture(
            root,
            backend=MoveNumLikeBackend(),
            source_data=source_data,
            session_id="session-0000000000000011",
        )
        # The v2 stack helper has the authenticated post-allocation rewrite
        # needed for stack_interval objects; use its exact event preparation
        # with this fresh source/session and name map.
        token_norm = str(envelope["inventory"]["locals"][0]["token"])
        token_pos = str(envelope["inventory"]["locals"][1]["token"])
        argument_token = str(envelope["inventory"]["arguments"][0]["token"])
        events = envelope["events"]
        filtered_events = []
        for event in events:
            if event["event_kind"] == "regalloc_assignment" and event.get("object_token") in {token_norm, token_pos}:
                continue
            if event["event_kind"] == "physical_reg_assignment":
                event["object_token"] = argument_token
            if event["event_kind"] in {"object_stack_write_pre", "object_stack_write_post"}:
                if event["hook_id"] == "object_write_0":
                    event["object_token"] = token_norm
                    event["target_slot"] = 0
                elif event["hook_id"] == "object_write_1":
                    event["object_token"] = token_pos
                    event["target_slot"] = 12
                elif event["hook_id"] == "object_write_2":
                    event["object_token"] = argument_token
                    event["target_slot"] = 99
            filtered_events.append(event)
        envelope["events"] = filtered_events
        for index, event in enumerate(filtered_events):
            event["sequence"] = index
            event["event_id"] = f"{envelope['context']['session_id']}-e{index:06d}"
        writes = {
            token_norm: [
                event["event_id"] for event in filtered_events
                if event["event_kind"] == "object_stack_write_pre"
                and event.get("object_token") == token_norm
            ] + [
                event["event_id"] for event in filtered_events
                if event["event_kind"] == "object_stack_write_post"
                and event.get("object_token") == token_norm
            ],
            token_pos: [
                event["event_id"] for event in filtered_events
                if event["event_kind"] == "object_stack_write_pre"
                and event.get("object_token") == token_pos
            ] + [
                event["event_id"] for event in filtered_events
                if event["event_kind"] == "object_stack_write_post"
                and event.get("object_token") == token_pos
            ],
        }
        for row, token, offset in (
            (envelope["inventory"]["locals"][0], token_norm, 0),
            (envelope["inventory"]["locals"][1], token_pos, 12),
        ):
            row["ownership"] = {
                "status": "EXACT",
                "mode": "stack_home",
                "evidence_event_ids": writes[token],
                "stack_home": {"base": "r1", "offset": offset},
            }
        reseal_capture(envelope_path, request_path, envelope)
        trust = trust_root_for_request(request_path, include_outputs=True)
        context = envelope["context"]
        source = source_data

        def location(fragment: bytes, *, last: bool = False) -> tuple[int, int]:
            start = source.rindex(fragment) if last else source.index(fragment)
            return start, start + len(fragment)

        def span(identity: str, role: str, fragment: bytes, *, last: bool = False, dependency: str | None = None, indices: list[int] | None = None) -> dict[str, object]:
            start, end = location(fragment, last=last)
            prefix = source[:start]
            return {
                "object_token": f"<CAPTURE_TOKEN:{identity}>",
                "identity": identity,
                "role": role,
                "byte_start": start,
                "byte_end": end,
                "line_start": prefix.count(b"\n") + 1,
                "line_end": prefix.count(b"\n") + 1,
                "text_sha256": hashlib.sha256(source[start:end]).hexdigest(),
                "ownership_mode": "stack_interval",
                "stack_interval": {"base": "r1", "offset": 0 if identity == "posNorm" else 12},
            }

        decl_norm = span("posNorm", "declaration", b"HuVecF posNorm")
        read_norm = span("posNorm", "read", b"posNorm", last=True, dependency="move_num_copy", indices=[117, 118, 119, 120, 121, 122])
        decl_pos = span("pos", "declaration", b"HuVecF pos;")
        write_pos = span("pos", "write", b"pos", dependency="move_num_copy", indices=[117, 118, 119, 120, 121, 122])
        write_start = source.index(b"    pos = posNorm;") + len(b"    ")
        write_pos.update({
            "byte_start": write_start,
            "byte_end": write_start + len(b"pos"),
            "line_start": source[:write_start].count(b"\n") + 1,
            "line_end": source[:write_start].count(b"\n") + 1,
            "text_sha256": hashlib.sha256(b"pos").hexdigest(),
        })
        template = {
            "schema": "mwcc_source_span_template/v1",
            "template_schema": "mwcc_source_span_template/v1",
            "function": context["function"],
            "function_sha256": context["function_sha256"],
            "session_id": "<CAPTURE_SESSION_ID>",
            "source": dict(context["source"]),
            "spans": [decl_norm, read_norm, decl_pos, write_pos],
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "authority_advanced": False,
            "unsealed": True,
            "capture_placeholders": {"session_id": "<CAPTURE_SESSION_ID>", "object_tokens": True},
            "notes": "reviewed MoveNum pos/write and posNorm/read split",
        }
        template_path = root / "movenum-template.unsealed.json"
        template_path.write_text(json.dumps(template, indent=2, sort_keys=True), encoding="utf-8")

        def plan_binding(raw: dict[str, object], dependency_id: str | None, indices: list[int]) -> dict[str, object]:
            return {
                "identity": raw["identity"],
                "role": raw["role"],
                "byte_start": raw["byte_start"],
                "byte_end": raw["byte_end"],
                "dependency_id": dependency_id,
                "machine_instruction_indices": list(indices),
            }

        bindings = [
            plan_binding(read_norm, "move_num_copy", [117, 118, 119, 120, 121, 122]),
            plan_binding(write_pos, "move_num_copy", [117, 118, 119, 120, 121, 122]),
        ]
        for raw in (decl_norm, decl_pos):
            bindings.append(plan_binding(raw, None, []))
        plan = {
            "schema": MODULE.SOURCE_SPAN_PLAN_SCHEMA,
            "function": context["function"],
            "function_sha256": context["function_sha256"],
            "session_id": context["session_id"],
            "source": dict(context["source"]),
            "envelope": descriptor(envelope_path),
            "template": descriptor(template_path),
            "objects": [
                {"identity": "posNorm", "ownership_mode": "stack_interval", "object_type": "HuVecF", "byte_size": 12},
                {"identity": "pos", "ownership_mode": "stack_interval", "object_type": "HuVecF", "byte_size": 12},
            ],
            "bindings": bindings,
            "authority_advanced": False,
        }
        plan_path = root / "movenum-binding-plan.json"
        plan_path.write_text(json.dumps(plan, indent=2, sort_keys=True), encoding="utf-8")
        return {
            "request_path": request_path,
            "envelope_path": envelope_path,
            "envelope": envelope,
            "trust_root": trust,
            "template": template,
            "template_path": template_path,
            "plan": plan,
            "plan_path": plan_path,
            "source": source,
            "token_norm": token_norm,
            "token_pos": token_pos,
        }

    def _normalize(self, fixture: dict[str, object], output: Path) -> dict[str, object]:
        return MODULE.normalize_source_span_template(
            fixture["envelope_path"],
            fixture["template_path"],
            fixture["plan_path"],
            output,
            trust_root=fixture["trust_root"],
        )

    def test_normalize_accepts_corrected_movenum_split_and_strips_template_only_fields(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = self._fixture(root)
            output = root / "normalized.json"
            result = self._normalize(fixture, output)
            manifest = json.loads(output.read_text(encoding="utf-8"))
            self.assertEqual(result["status"], "READY")
            self.assertEqual(result["schema"], f"{MODULE.SOURCE_SPAN_SCHEMA_V2}/normalize")
            self.assertFalse(result["authority_advanced"])
            self.assertEqual(manifest["schema"], MODULE.SOURCE_SPAN_SCHEMA_V2)
            self.assertFalse(manifest["authority_advanced"])
            self.assertEqual(
                {row["identity"] for row in manifest["objects"]},
                {"posNorm", "pos"},
            )
            self.assertEqual(
                {row["object_token"] for row in manifest["objects"]},
                {fixture["token_norm"], fixture["token_pos"]},
            )
            self.assertEqual(
                {row["identity"] for row in manifest["spans"]},
                {"posNorm", "pos"},
            )
            self.assertTrue(all(set(row) == MODULE._SOURCE_SPAN_V2_FIELDS for row in manifest["spans"]))
            self.assertTrue(all("<CAPTURE" not in str(row) for row in manifest["spans"]))
            self.assertNotIn("capture_placeholders", manifest)
            self.assertNotIn("notes", manifest)
            self.assertTrue(all("ownership_mode" not in row and "stack_interval" not in row for row in manifest["spans"]))
            MODULE._validate_source_span_manifest(output, fixture["envelope"])

    def test_normalize_cli_accepts_same_capture_and_emits_closed_manifest(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = self._fixture(root)
            output = root / "normalized-cli.json"
            trust_path = root / "trust-root.json"
            trust_values = {
                field: getattr(fixture["trust_root"], field)
                for field in MODULE.ExternalTrustRoot.FIELDS
                if getattr(fixture["trust_root"], field) is not None
            }
            trust_path.write_text(json.dumps(trust_values, indent=2, sort_keys=True), encoding="utf-8")
            completed = subprocess.run(
                [
                    sys.executable, str(TOOL), "normalize-source-spans",
                    "--envelope", str(fixture["envelope_path"]),
                    "--trust-root", str(trust_path),
                    "--template", str(fixture["template_path"]),
                    "--binding-plan", str(fixture["plan_path"]),
                    "--output", str(output),
                ],
                check=False,
                capture_output=True,
                text=True,
            )
            self.assertEqual(completed.returncode, 0, completed.stderr or completed.stdout)
            result = json.loads(completed.stdout)
            self.assertEqual(result["status"], "READY")
            self.assertEqual(json.loads(output.read_text(encoding="utf-8"))["schema"], MODULE.SOURCE_SPAN_SCHEMA_V2)

    def test_normalize_rejects_stale_capture_bindings_and_placeholders(self) -> None:
        cases = (
            ("template source digest", lambda f: f["template"]["source"].update(sha256="0" * 64), "source span template source identity mismatch"),
            ("template function", lambda f: f["template"].update(function="OtherFunction"), "template function is not capture-bound"),
            ("template session", lambda f: f["template"].update(session_id="session-stale"), "template session_id is not capture-bound"),
            ("plan function", lambda f: f["plan"].update(function="OtherFunction"), "source span binding plan/template function is not capture-bound"),
            ("plan session", lambda f: f["plan"].update(session_id="<CAPTURE_SESSION_ID>"), "source span binding plan/template session_id is not capture-bound"),
            ("plan envelope", lambda f: f["plan"]["envelope"].update(sha256="0" * 64), "source span plan envelope identity mismatch"),
            ("plan placeholder identity", lambda f: f["plan"]["objects"][0].update(identity="<CAPTURE_TOKEN:posNorm>"), "does not bind one unique compiler inventory row"),
        )
        for label, mutate, pattern in cases:
            with self.subTest(label=label), TemporaryDirectory() as directory:
                root = Path(directory)
                fixture = self._fixture(root)
                mutate(fixture)
                fixture["template_path"].write_text(json.dumps(fixture["template"], indent=2, sort_keys=True), encoding="utf-8")
                fixture["plan_path"].write_text(json.dumps(fixture["plan"], indent=2, sort_keys=True), encoding="utf-8")
                with self.assertRaisesRegex(MODULE.Rejected, pattern):
                    self._normalize(fixture, root / "rejected.json")

    def test_normalize_rejects_duplicate_and_mixed_object_bindings(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = self._fixture(root)
            fixture["plan"]["bindings"].append(dict(fixture["plan"]["bindings"][0]))
            fixture["plan_path"].write_text(json.dumps(fixture["plan"], indent=2, sort_keys=True), encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "duplicate span bindings"):
                self._normalize(fixture, root / "duplicate.json")

        for field, value in (("object_type", "float[3]"), ("byte_size", 8), ("ownership_mode", "scalar_register")):
            with self.subTest(field=field), TemporaryDirectory() as directory:
                root = Path(directory)
                fixture = self._fixture(root)
                fixture["plan"]["objects"][0][field] = value
                fixture["plan_path"].write_text(json.dumps(fixture["plan"], indent=2, sort_keys=True), encoding="utf-8")
                with self.assertRaisesRegex(MODULE.Rejected, "exact 12-byte HuVecF|scalar source span object"):
                    self._normalize(fixture, root / f"bad-{field}.json")

    def test_normalize_rejects_overlapping_source_bindings(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = self._fixture(root)
            template_spans = fixture["template"]["spans"]
            plan_bindings = fixture["plan"]["bindings"]
            target = next(row for row in template_spans if row["identity"] == "posNorm" and row["role"] == "read")
            target_binding = next(row for row in plan_bindings if row["identity"] == "posNorm" and row["role"] == "read")
            write_span = next(row for row in template_spans if row["identity"] == "pos" and row["role"] == "write")
            start = int(write_span["byte_start"]) + 1
            target["byte_start"] = start
            target["text_sha256"] = hashlib.sha256(fixture["source"][start:int(target["byte_end"])]).hexdigest()
            target_binding["byte_start"] = start
            fixture["template_path"].write_text(json.dumps(fixture["template"], indent=2, sort_keys=True), encoding="utf-8")
            fixture["plan"]["template"] = descriptor(fixture["template_path"])
            fixture["plan_path"].write_text(json.dumps(fixture["plan"], indent=2, sort_keys=True), encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "overlap|claimed by multiple|binding"):
                self._normalize(fixture, root / "overlap.json")

    def test_normalize_allows_nested_roles_for_one_source_object(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = self._fixture(root)
            template_spans = fixture["template"]["spans"]
            plan_bindings = fixture["plan"]["bindings"]
            declaration = next(
                row
                for row in template_spans
                if row["identity"] == "posNorm" and row["role"] == "declaration"
            )
            nested = dict(declaration)
            nested["role"] = "call_return"
            template_spans.append(nested)
            binding = next(
                row
                for row in plan_bindings
                if row["identity"] == "posNorm" and row["role"] == "declaration"
            )
            nested_binding = dict(binding)
            nested_binding["role"] = "call_return"
            plan_bindings.append(nested_binding)
            fixture["template_path"].write_text(
                json.dumps(fixture["template"], indent=2, sort_keys=True),
                encoding="utf-8",
            )
            fixture["plan"]["template"] = descriptor(fixture["template_path"])
            fixture["plan_path"].write_text(
                json.dumps(fixture["plan"], indent=2, sort_keys=True),
                encoding="utf-8",
            )
            output = root / "nested.json"
            self._normalize(fixture, output)
            normalized = json.loads(output.read_text(encoding="utf-8"))
            roles = {
                row["role"]
                for row in normalized["spans"]
                if row["identity"] == "posNorm"
                and row["byte_start"] == declaration["byte_start"]
                and row["byte_end"] == declaration["byte_end"]
            }
            self.assertEqual(roles, {"declaration", "call_return"})

    def test_normalize_rejects_existing_output_collision(self) -> None:
        with TemporaryDirectory() as directory:
            root = Path(directory)
            fixture = self._fixture(root)
            output = root / "existing.json"
            output.write_text("sentinel", encoding="utf-8")
            with self.assertRaisesRegex(MODULE.Rejected, "output already exists"):
                self._normalize(fixture, output)


if __name__ == "__main__":
    unittest.main()
