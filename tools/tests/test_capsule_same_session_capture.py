from __future__ import annotations

import hashlib
import importlib.util
import json
from pathlib import Path
import struct
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


class FakeBackend:
    capabilities = {"read_image", "install_breakpoint", "remove_breakpoint", "single_step", "run", "close"}

    def __init__(self, *, conflict: bool = False, duplicate_object: bool = False, duplicate_vreg: bool = False, rewrite: Path | None = None, physical_rows: list[dict[str, object]] | None = None, physical_before_inventory: bool = False, inventory_empty_first: bool = False, token_collision: bool = False) -> None:
        self.conflict = conflict
        self.duplicate_object = duplicate_object
        self.duplicate_vreg = duplicate_vreg
        self.rewrite = rewrite
        self.physical_rows = list(physical_rows or [])
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

    def read_image(self, address: int, size: int) -> bytes:
        prefix = next(row["prefix"] for row in MODULE.HOOKS if int(row["address"]) == address)
        return bytes.fromhex(str(prefix))[:size]

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

    def current_function(self) -> str:
        return "mbCapListDebug"

    def run(self, session: MODULE.CombinedCaptureSession) -> None:
        self.session = session
        session.on_process_started(self.process_id)
        session.on_breakpoint(int(MODULE.HOOK_BY_ID["function_filter"]["address"]), 1)
        session.on_single_step(1)
        if self.physical_before_inventory:
            session.on_breakpoint(int(MODULE.HOOK_BY_ID["regalloc_post"]["address"]), 1)
            session.on_single_step(1)
        session.on_breakpoint(int(MODULE.HOOK_BY_ID["allocation_pre"]["address"]), 1)
        session.on_single_step(1)
        session.on_breakpoint(int(MODULE.HOOK_BY_ID["allocation_post"]["address"]), 1)
        session.on_single_step(1)
        for hook_id in MODULE.WRITE_HOOK_IDS:
            address = int(MODULE.HOOK_BY_ID[hook_id]["address"])
            session.on_breakpoint(address, 1)
            session.on_single_step(1)
        for hook_id in MODULE.PCODE_HOOK_IDS:
            session.on_breakpoint(int(MODULE.HOOK_BY_ID[hook_id]["address"]), 1)
            session.on_single_step(1)
        session.on_breakpoint(int(MODULE.HOOK_BY_ID["regalloc"]["address"]), 1)
        session.on_single_step(1)
        if not self.physical_before_inventory:
            physical_rows = max(1, len(self.physical_rows))
            for _ in range(physical_rows):
                session.on_breakpoint(int(MODULE.HOOK_BY_ID["regalloc_post"]["address"]), 1)
                session.on_single_step(1)
        if self.rewrite is not None:
            self.rewrite.write_text("rewritten", encoding="utf-8")
        session.on_process_exit(0)

    def close(self) -> None:
        self.closed = True


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
            MODULE.HOOK_BY_ID["regalloc_post"],
            {
                "id": "regalloc_post",
                "address": 0x004D03E8,
                "prefix": "83c4085d5e5bc300",
                "lane": "pcode",
                "role": "regalloc_post",
            },
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
            root = Path(directory)
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
            root = Path(directory)
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
                session.on_process_started(self.process_id)
                for hook_id in ("function_filter", "allocation_pre", "allocation_post"):
                    session.on_breakpoint(int(MODULE.HOOK_BY_ID[hook_id]["address"]), 1)
                    session.on_single_step(1)
                for hook_id in MODULE.WRITE_HOOK_IDS:
                    session.on_breakpoint(int(MODULE.HOOK_BY_ID[hook_id]["address"]), 1)
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

    def test_later_pid_change_is_rejected(self) -> None:
        class WrongPID(FakeBackend):
            def run(self, session: MODULE.CombinedCaptureSession) -> None:
                session.on_process_started(self.process_id)
                session.on_breakpoint(int(MODULE.HOOK_BY_ID["function_filter"]["address"]), 1, self.process_id + 1)

        with TemporaryDirectory() as directory:
            with self.assertRaisesRegex(MODULE.Rejected, "debug loop ended|process id"):
                MODULE.CombinedCaptureSession(auth(Path(directory)), WrongPID()).run()


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
            "prepare_capture",
        ):
            self.assertTrue(callable(getattr(backend, name, None)), name)
            self.assertIn(name, backend.capabilities)

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


if __name__ == "__main__":
    unittest.main()
