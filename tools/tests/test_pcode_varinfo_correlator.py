import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from tools import pcode_varinfo_correlator as module


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "tools" / "pcode_varinfo_correlator.py"
SOURCE_SHA = "a" * 64
COMPILER_SHA = "b" * 64
OWNERSHIP_SHA = "c" * 64
MANIFEST_SHA = "d" * 64


def allocator_trace(*, duplicate=False):
    locals_rows = [
        {
            "name": "alpha",
            "datatype": 1,
            "type_code": 2,
            "known_varinfo": {"flags": 2, "rclass": 4, "reg": 31, "usage": 4},
        },
        {
            "name": "beta",
            "datatype": 1,
            "type_code": 1,
            "known_varinfo": {"flags": 0, "rclass": 0, "reg": 0, "usage": 1},
        },
        {"name": "ghost", "datatype": 1, "type_code": 1, "known_varinfo": {}},
    ]
    if duplicate:
        locals_rows.append(
            {"name": "alpha", "datatype": 1, "type_code": 2, "known_varinfo": {}}
        )
    return {
        "schema": "mwcc_allocator_trace/v1",
        "function": "CapEffThrowMasu",
        "status": "captured",
        "target": "CapEffThrowMasu",
        "compiler": {"sha256": COMPILER_SHA},
        "assignment_events": [
            {
                "order": 0,
                "bank": "pass_4357d0",
                "after_locals": locals_rows,
                "selected_object": locals_rows[0],
            }
        ],
        "limitations": ["synthetic fixture"],
    }


def pcode_trace(*, explicit=False):
    local_rows = [
        {
            "name": "alpha",
            "vreg_ids": ["r32"] if explicit else [],
            "vreg_status": "AUTHENTICATED" if explicit else "UNOBSERVED",
        },
        {"name": "beta", "vreg_ids": [], "vreg_status": "UNOBSERVED"},
        {"name": "ghost", "vreg_ids": [], "vreg_status": "UNOBSERVED"},
    ]
    return {
        "schema": "mwcc_gc26_pcode_trace/v2",
        "status": "CAPTURED",
        "authentication": {
            "source_hash_authenticated": True,
            "source_provenance": "AUTHENTICATED_TEST_SOURCE_AAAAAAAA",
            "artifacts": {
                "source": {"sha256": SOURCE_SHA},
                "compiler": {"sha256": COMPILER_SHA},
            }
        },
        "capture": {
            "function": "CapEffThrowMasu",
            "capture_status": "CAPTURED",
            "source_inventory": {"status": "CAPTURED", "locals": local_rows},
            "limitations": ["no frontend object identity is joined to PCode virtual registers"],
            "pcode": {
                "backend-00-initial-code.txt": {
                    "instructions": [
                        {
                            "order": 0,
                            "block": "B1",
                            "mnemonic": "stfs",
                            "memory_objects": ["alpha"],
                            "operands": "f32,r1,0(alpha)",
                            "source_line": 10,
                            "virtual_registers": ["r32"],
                        },
                        {
                            "order": 1,
                            "block": "B1",
                            "mnemonic": "lfs",
                            "memory_objects": ["alpha"],
                            "operands": "f33,r1,0(alpha)",
                            "source_line": 11,
                            "virtual_registers": ["r33"],
                        },
                        {
                            "order": 2,
                            "block": "B2",
                            "mnemonic": "stw",
                            "memory_objects": ["beta"],
                            "operands": "r3,r1,0(beta)",
                            "source_line": 12,
                            "virtual_registers": [],
                        },
                        {
                            "order": 3,
                            "block": "B2",
                            "mnemonic": "li",
                            "memory_objects": [],
                            "operands": "r34,0",
                            "source_line": 13,
                            "virtual_registers": ["r34"],
                        },
                    ],
                    "virtual_registers": ["r32", "r33", "r34"],
                }
            },
            "vreg_chronology": {
                "vregs": [
                    {
                        "vreg_id": "r32",
                        "creation_order": 0,
                        "first_occurrence": 0,
                        "last_occurrence": 0,
                        "occurrence_count": 1,
                        "crossed_call_orders": [],
                        "blocks": ["B1"],
                        "source_lines": [10],
                        "interval_kind": "OCCURRENCE_SPAN_NOT_COMPILER_LIVENESS",
                        "reuse_status": "UNOBSERVED",
                    },
                    {
                        "vreg_id": "r33",
                        "creation_order": 1,
                        "first_occurrence": 1,
                        "last_occurrence": 1,
                        "occurrence_count": 1,
                        "crossed_call_orders": [],
                        "blocks": ["B1"],
                        "source_lines": [11],
                        "interval_kind": "OCCURRENCE_SPAN_NOT_COMPILER_LIVENESS",
                        "reuse_status": "UNOBSERVED",
                    },
                    {
                        "vreg_id": "r34",
                        "creation_order": 2,
                        "first_occurrence": 3,
                        "last_occurrence": 3,
                        "occurrence_count": 1,
                        "crossed_call_orders": [],
                        "blocks": ["B2"],
                        "source_lines": [13],
                        "interval_kind": "OCCURRENCE_SPAN_NOT_COMPILER_LIVENESS",
                        "reuse_status": "UNOBSERVED",
                    },
                ]
            },
        },
    }


def pcode_v3_trace():
    return {
        "schema": "mwcc_gc26_pcode_trace/v3",
        "status": "UNKNOWN",
        "function": "CapEffThrowMasu",
        "stages": {
            "backend-00-initial-code.pcode.json": {
                "instructions": [
                    {
                        "order": 0,
                        "block": 1,
                        "mnemonic": "stfs",
                        "operands": [],
                        "sourceoffset": {"status": "EXACT", "value": 10},
                    }
                ]
            }
        },
        "limitations": ["frontend join unavailable"],
    }


def pcode_v3_authenticated_trace(*, mutate=None):
    """Synthetic normalized v3 packet with direct local+argument ownership."""

    direct = [
        {"object_ordinal": 0, "vreg_id": "r32", "status": "AUTHENTICATED"},
        {"object_ordinal": 1, "vreg_id": "f33", "status": "AUTHENTICATED"},
    ]
    source_inventory = {
        "status": "CAPTURED",
        "reason": "direct unique same-session source-object to PCode vreg ownership",
        "locals": [
            {
                "kind": "local",
                "ordinal": 0,
                "compiler_list_order": 0,
                "name": "alpha",
                "datatype": 1,
                "size": 4,
                "vreg_ids": ["r32"],
                "vreg_status": "AUTHENTICATED",
            }
        ],
        "arguments": [
            {
                "kind": "argument",
                "ordinal": 1,
                "compiler_list_order": 0,
                "name": "arg",
                "datatype": 1,
                "size": 4,
                "vreg_ids": ["f33"],
                "vreg_status": "AUTHENTICATED",
            }
        ],
    }
    packet = {
        "schema": "mwcc_gc26_pcode_trace/v3",
        # The normalized producer keeps the diagnostic envelope UNKNOWN;
        # pcode_status is the successful top-level stage status.
        "status": "UNKNOWN",
        "pcode_status": "EXACT",
        "function": "CapEffThrowMasu",
        "provenance": {
            "manifest_sha256": MANIFEST_SHA,
            "source_sha256": SOURCE_SHA,
            "compiler_sha256": COMPILER_SHA,
            "ownership_sha256": OWNERSHIP_SHA,
            "source_provenance": "AUTHENTICATED_TEST_SOURCE_AAAAAAAA",
            "ownership_provenance": "AUTHENTICATED_TEST_OWNERSHIP_CCCCCCCC",
        },
        "authentication": {
            "status": "AUTHENTICATED",
            "reason": "manifest and immutable compiler/source/ownership artifacts are bound",
            "manifest_sha256": MANIFEST_SHA,
            "compiler_path": "C:/compiler",
            "compiler_sha256": COMPILER_SHA,
            "compiler_size": 1,
            "source_path": "C:/source",
            "source_sha256": SOURCE_SHA,
            "source_size": 1,
            "ownership_path": "C:/ownership",
            "ownership_sha256": OWNERSHIP_SHA,
            "ownership_size": 1,
            "ownership_events_path": "C:/ownership-events",
            "ownership_events_sha256": OWNERSHIP_SHA,
            "ownership_events_size": 1,
            "function": "CapEffThrowMasu",
            "cwd": "C:/",
            "argv": ["mwcc", "-c", "C:/source"],
            "session_id": "session-1",
            "process_id": 1,
            "source_hash_authenticated": True,
            "source_provenance": "AUTHENTICATED_TEST_SOURCE_AAAAAAAA",
        },
        "frontend_join": {
            "status": "AUTHENTICATED",
            "reason": "direct unique same-session source-object to PCode vreg ownership",
            "session": {
                "session_id": "session-1",
                "process_id": 1,
                "function": "CapEffThrowMasu",
                "source": "C:/source",
                "compiler": "C:/compiler",
                "argv": ["mwcc", "-c", "C:/source"],
                "cwd": "C:/",
                "snapshot_complete": True,
                "source_capture_stage": "frontend-initial-code",
                "regalloc_capture_banks": ["fpr", "gpr"],
            },
            "direct_object_vregs": json.loads(json.dumps(direct)),
        },
        "ownership": {
            "status": "AUTHENTICATED",
            "reason": "direct unique same-session source-object to PCode vreg ownership",
            "session": {
                "session_id": "session-1",
                "process_id": 1,
                "function": "CapEffThrowMasu",
                "source": "C:/source",
                "compiler": "C:/compiler",
                "argv": ["mwcc", "-c", "C:/source"],
                "cwd": "C:/",
                "snapshot_complete": True,
                "source_capture_stage": "frontend-initial-code",
                "regalloc_capture_banks": ["fpr", "gpr"],
            },
            "direct_object_vregs": json.loads(json.dumps(direct)),
        },
        "source_inventory": source_inventory,
        "stages": {
            "backend-00-initial-code.pcode.json": {
                "instructions": [
                    {
                        "order": 0,
                        "block": 1,
                        "mnemonic": {"status": "EXACT", "value": "stfs"},
                        "sourceoffset": {"status": "EXACT", "value": 10},
                        "operands": [
                            {
                                "index": 0,
                                "object_reference": {"status": "EXACT", "value": "present"},
                                "register": {"status": "EXACT", "value": 3},
                            }
                        ],
                    }
                ]
            }
        },
        "limitations": ["synthetic fixture"],
    }
    if mutate is not None:
        mutate(packet)
    return packet


class PCodeVarInfoCorrelatorTests(unittest.TestCase):
    @staticmethod
    def _sha256(path):
        return hashlib.sha256(path.read_bytes()).hexdigest()

    def _trusted_v3_bundle(
        self,
        root,
        allocator_path,
        pcode_path,
        packet,
        *,
        primary_v3,
        normalized_path=None,
        allocator_trace=None,
    ):
        """Write a distinct, canonical trust bundle and bind the packet to it."""

        source_path = root / "source.c"
        compiler_path = root / "mwcc.exe"
        manifest_path = root / "manifest.json"
        ownership_path = root / "ownership.json"
        ownership_events_path = root / "ownership-events.json"
        source_path.write_bytes(b"trusted source bytes\n")
        compiler_path.write_bytes(b"trusted compiler bytes\n")
        manifest_path.write_bytes(b"trusted manifest bytes\n")
        ownership_path.write_bytes(b"trusted ownership bytes\n")
        ownership_events_path.write_bytes(b"trusted ownership event bytes\n")

        def descriptor(path):
            return path, self._sha256(path), path.stat().st_size

        normalized_path = normalized_path or pcode_path
        source, source_sha, source_size = descriptor(source_path)
        compiler, compiler_sha, compiler_size = descriptor(compiler_path)
        manifest, manifest_sha, manifest_size = descriptor(manifest_path)
        ownership, ownership_sha, ownership_size = descriptor(ownership_path)
        ownership_events, ownership_events_sha, ownership_events_size = descriptor(ownership_events_path)
        # The allocator compiler identity is part of the same anchored
        # session.  Update the synthetic mapping before hashing its bytes so
        # a valid direct-v3 fixture exercises the raw-byte identity gate.
        if allocator_trace is not None and allocator_trace.get("compiler", {}).get("sha256") == COMPILER_SHA:
            allocator_trace["compiler"]["sha256"] = compiler_sha
            allocator_path.write_text(json.dumps(allocator_trace), encoding="utf-8")
        allocator, allocator_sha, allocator_size = descriptor(allocator_path)

        auth = packet["authentication"]
        auth.update(
            {
                "manifest_path": str(manifest),
                "manifest_sha256": manifest_sha,
                "compiler_path": str(compiler),
                "compiler_size": compiler_size,
                "source_path": str(source),
                "source_size": source_size,
                "ownership_path": str(ownership),
                "ownership_size": ownership_size,
                "ownership_events_path": str(ownership_events),
                "ownership_events_size": ownership_events_size,
                "cwd": str(root),
                "argv": ["mwcc", "-c", str(source)],
                "session_id": "session-1",
                "process_id": 1,
                "function": "CapEffThrowMasu",
            }
        )
        for key, original, replacement in (
            ("compiler_sha256", COMPILER_SHA, compiler_sha),
            ("source_sha256", SOURCE_SHA, source_sha),
            ("ownership_sha256", OWNERSHIP_SHA, ownership_sha),
            ("ownership_events_sha256", OWNERSHIP_SHA, ownership_events_sha),
        ):
            if key in auth and auth[key] == original:
                auth[key] = replacement
        if "source_provenance" in auth and auth["source_provenance"] == "AUTHENTICATED_TEST_SOURCE_AAAAAAAA":
            auth["source_provenance"] = f"AUTHENTICATED_TEST_SOURCE_{source_sha[:8].upper()}"
        provenance = packet.get("provenance")
        if isinstance(provenance, dict):
            for key, original, replacement in (
                ("manifest_sha256", MANIFEST_SHA, manifest_sha),
                ("source_sha256", SOURCE_SHA, source_sha),
                ("compiler_sha256", COMPILER_SHA, compiler_sha),
                ("ownership_sha256", OWNERSHIP_SHA, ownership_sha),
                ("ownership_events_sha256", OWNERSHIP_SHA, ownership_events_sha),
            ):
                if key in provenance and provenance[key] == original:
                    provenance[key] = replacement
        for label in ("frontend_join", "ownership"):
            session = packet[label]["session"]
            defaults = {
                "source": "C:/source",
                "compiler": "C:/compiler",
                "argv": ["mwcc", "-c", "C:/source"],
                "cwd": "C:/",
                "session_id": "session-1",
                "process_id": 1,
                "function": "CapEffThrowMasu",
            }
            replacements = {
                "source": str(source),
                "compiler": str(compiler),
                "argv": ["mwcc", "-c", str(source)],
                "cwd": str(root),
                "session_id": "session-1",
                "process_id": 1,
                "function": "CapEffThrowMasu",
            }
            for key, replacement in replacements.items():
                if key in session and session[key] == defaults[key]:
                    session[key] = replacement

        if primary_v3:
            pcode_path.write_text(json.dumps(packet), encoding="utf-8")
        else:
            normalized_path.write_text(json.dumps(packet), encoding="utf-8")

        pcode, pcode_sha, pcode_size = descriptor(pcode_path)
        normalized, normalized_sha, normalized_size = descriptor(normalized_path)

        return module.ExternalTrustRoot(
            manifest_path=manifest,
            manifest_sha256=manifest_sha,
            manifest_size=manifest_size,
            source_path=source,
            source_sha256=source_sha,
            source_size=source_size,
            compiler_path=compiler,
            compiler_sha256=compiler_sha,
            compiler_size=compiler_size,
            ownership_path=ownership,
            ownership_sha256=ownership_sha,
            ownership_size=ownership_size,
            ownership_events_path=ownership_events,
            ownership_events_sha256=ownership_events_sha,
            ownership_events_size=ownership_events_size,
            allocator_path=allocator,
            allocator_sha256=allocator_sha,
            allocator_size=allocator_size,
            pcode_path=pcode,
            pcode_sha256=pcode_sha,
            pcode_size=pcode_size,
            pcode_v3_path=None if primary_v3 else normalized,
            pcode_v3_sha256=None if primary_v3 else normalized_sha,
            pcode_v3_size=None if primary_v3 else normalized_size,
            function="CapEffThrowMasu",
            cwd=root,
            argv=("mwcc", "-c", str(source)),
            session_id="session-1",
            process_id=1,
        )

    def correlate_authenticated(self, allocator, pcode, *, pcode_v3=None, **kwargs):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            allocator_path = root / "allocator.json"
            pcode_path = root / "pcode.json"
            pcode_v3_path = root / "pcode-v3.json"
            primary_v3 = pcode.get("schema") == "mwcc_gc26_pcode_trace/v3"
            if pcode_v3 is None and not primary_v3:
                pcode_v3 = pcode_v3_trace()
            allocator_path.write_text(json.dumps(allocator), encoding="utf-8")
            pcode_path.write_text(json.dumps(pcode), encoding="utf-8")
            if pcode_v3 is not None:
                pcode_v3_path.write_text(json.dumps(pcode_v3), encoding="utf-8")
            expected_source_sha256 = kwargs.pop("expected_source_sha256", SOURCE_SHA)
            expected_compiler_sha256 = kwargs.pop("expected_compiler_sha256", COMPILER_SHA)
            return module.correlate(
                allocator,
                pcode,
                pcode_v3_trace=pcode_v3,
                allocator_path=allocator_path,
                pcode_path=pcode_path,
                pcode_v3_path=pcode_v3_path if pcode_v3 is not None else None,
                expected_source_sha256=expected_source_sha256,
                expected_compiler_sha256=expected_compiler_sha256,
                expected_allocator_trace_sha256=self._sha256(allocator_path),
                expected_pcode_trace_sha256=self._sha256(pcode_path),
                expected_pcode_v3_trace_sha256=(
                    self._sha256(pcode_v3_path) if pcode_v3 is not None else None
                ),
                **kwargs,
            )

    def correlate_v3_primary(
        self,
        allocator=None,
        *,
        pcode_v3=None,
        temporary_parent=None,
        **kwargs,
    ):
        allocator = allocator or allocator_trace()
        pcode_v3 = pcode_v3 or pcode_v3_authenticated_trace()
        with tempfile.TemporaryDirectory(dir=temporary_parent) as directory:
            root = Path(directory)
            allocator_path = root / "allocator.json"
            pcode_path = root / "pcode-v3.json"
            allocator_path.write_text(json.dumps(allocator), encoding="utf-8")
            trust = self._trusted_v3_bundle(
                root,
                allocator_path,
                pcode_path,
                pcode_v3,
                primary_v3=True,
                allocator_trace=allocator,
            )
            return module.correlate(
                allocator,
                pcode_v3,
                trust_root=trust,
                allocator_path=allocator_path,
                pcode_path=pcode_path,
                ownership_path=trust.ownership_path,
                expected_source_sha256=kwargs.pop("expected_source_sha256", trust.source_sha256),
                expected_compiler_sha256=kwargs.pop("expected_compiler_sha256", trust.compiler_sha256),
                expected_allocator_trace_sha256=self._sha256(allocator_path),
                expected_pcode_v3_trace_sha256=self._sha256(pcode_path),
                expected_ownership_sha256=kwargs.pop("expected_ownership_sha256", trust.ownership_sha256),
                **kwargs,
            )

    def test_name_overlap_is_evidence_only_and_unmatched_objects_are_preserved(self):
        report = module.correlate(allocator_trace(), pcode_trace())
        self.assertTrue(report["fail_closed"])
        self.assertFalse(report["authority_advanced"])
        self.assertEqual(report["report_sha256"], module._report_hash(report))
        self.assertEqual(
            report["report_sha256"],
            module.correlate(allocator_trace(), pcode_trace())["report_sha256"],
        )
        self.assertEqual(report["summary"]["allocator_object_count"], 3)
        self.assertEqual(report["summary"]["pcode_vreg_count"], 3)
        rows = {row["allocator"]["name"]: row for row in report["mappings"]}
        self.assertEqual(rows["alpha"]["status"], "UNRESOLVED_EVIDENCE")
        self.assertEqual(rows["alpha"]["pcode_fingerprint"]["vreg_ids"], ["r32", "r33"])
        self.assertEqual(rows["beta"]["status"], "UNRESOLVED_NO_VREG")
        self.assertEqual(rows["ghost"]["status"], "UNMATCHED_ALLOCATOR_OBJECT")
        self.assertEqual([row["vreg_id"] for row in report["unmatched_pcode_vregs"]], ["r34"])

    def test_authenticated_inventory_binding_is_the_only_resolved_join(self):
        report = self.correlate_authenticated(allocator_trace(), pcode_trace(explicit=True))
        rows = {row["allocator"]["name"]: row for row in report["mappings"]}
        self.assertEqual(rows["alpha"]["status"], "MATCHED_AUTHENTICATED")
        self.assertEqual(rows["alpha"]["confidence"]["score"], 1.0)
        # The explicit declaration binds r32, even though the shared
        # instruction fingerprint also observes r33.  The latter remains an
        # unbound candidate rather than being guessed into the object.
        self.assertEqual(rows["alpha"]["unbound_candidate_vregs"], ["r33"])
        self.assertEqual([row["vreg_id"] for row in report["unmatched_pcode_vregs"]], ["r34"])

    def test_v3_primary_direct_join_preserves_local_and_argument_rows(self):
        report = self.correlate_v3_primary()
        rows = {row["allocator"]["name"]: row for row in report["mappings"]}
        self.assertEqual(rows["alpha"]["status"], "MATCHED_AUTHENTICATED")
        self.assertEqual(rows["alpha"]["owned_vreg_ids"], ["r32"])
        self.assertEqual(report["source_inventory"]["arguments"][0]["name"], "arg")
        self.assertEqual(report["source_inventory"]["arguments"][0]["vreg_ids"], ["f33"])
        argument = next(
            row for row in report["source_object_mappings"] if row["kind"] == "argument"
        )
        self.assertEqual(argument["status"], "MATCHED_AUTHENTICATED")
        # The v3 stage's object_reference='present' sentinel is not a name
        # fingerprint and cannot manufacture a profile for allocator objects.
        self.assertNotIn("pcode_fingerprint", rows["alpha"])

    def test_v3_optional_direct_join_is_supported_alongside_v2(self):
        report = self.correlate_authenticated(
            allocator_trace(),
            pcode_trace(explicit=True),
            pcode_v3=pcode_v3_authenticated_trace(),
            expected_ownership_sha256=OWNERSHIP_SHA,
        )
        rows = {row["allocator"]["name"]: row for row in report["mappings"]}
        self.assertEqual(rows["alpha"]["status"], "UNKNOWN")
        self.assertFalse(report["authentication_gate"]["valid"])

    def test_v3_normalized_rows_use_kind_from_inventory_and_auth_artifacts(self):
        def normalize_shape(packet):
            packet.pop("provenance")
            for container in ("locals", "arguments"):
                for row in packet["source_inventory"][container]:
                    row.pop("kind")

        report = self.correlate_v3_primary(
            pcode_v3=pcode_v3_authenticated_trace(mutate=normalize_shape)
        )
        alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
        self.assertEqual(alpha["status"], "MATCHED_AUTHENTICATED")
        self.assertRegex(report["provenance"]["source_sha256"], r"^[0-9a-f]{64}$")

    def test_v3_kind_plus_ordinal_disambiguates_local_and_argument_objects(self):
        def reuse_ordinal_with_kind(packet):
            packet["source_inventory"]["arguments"][0]["ordinal"] = 0
            for label in ("frontend_join", "ownership"):
                packet[label]["direct_object_vregs"][0]["kind"] = "local"
                packet[label]["direct_object_vregs"][1]["kind"] = "argument"
                packet[label]["direct_object_vregs"][1]["object_ordinal"] = 0

        report = self.correlate_v3_primary(
            pcode_v3=pcode_v3_authenticated_trace(mutate=reuse_ordinal_with_kind)
        )
        rows = {row["allocator"]["name"]: row for row in report["mappings"]}
        self.assertEqual(rows["alpha"]["status"], "MATCHED_AUTHENTICATED")
        self.assertEqual(report["source_object_mappings"][1]["status"], "MATCHED_AUTHENTICATED")

    def test_v3_missing_or_tampered_manifest_provenance_is_unknown(self):
        for label, mutate in (
            (
                "source",
                lambda packet: (
                    packet["provenance"].pop("source_sha256"),
                    packet["authentication"].pop("source_sha256"),
                ),
            ),
            (
                "compiler",
                lambda packet: (
                    packet["provenance"].__setitem__("compiler_sha256", "d" * 64),
                    packet["authentication"].__setitem__("compiler_sha256", "d" * 64),
                ),
            ),
            (
                "ownership",
                lambda packet: (
                    packet["provenance"].pop("ownership_sha256"),
                    packet["authentication"].pop("ownership_sha256"),
                ),
            ),
            ("top-level status", lambda packet: packet.__setitem__("pcode_status", "UNKNOWN")),
            ("frontend status", lambda packet: packet["frontend_join"].__setitem__("status", "UNKNOWN")),
            ("ownership status", lambda packet: packet["ownership"].__setitem__("status", "UNKNOWN")),
            ("authentication status", lambda packet: packet["authentication"].__setitem__("status", "UNKNOWN")),
        ):
            with self.subTest(label):
                report = self.correlate_v3_primary(pcode_v3=pcode_v3_authenticated_trace(mutate=mutate))
                alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
                self.assertEqual(alpha["status"], "UNKNOWN")
                self.assertFalse(report["authentication_gate"]["valid"])

    def test_v3_wrong_external_hashes_or_paths_never_match(self):
        for key in ("expected_source_sha256", "expected_compiler_sha256", "expected_ownership_sha256"):
            with self.subTest(key):
                report = self.correlate_v3_primary(**{key: "d" * 64})
                if report["status"] == "ERROR":
                    self.assertTrue(report["fail_closed"])
                else:
                    alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
                    self.assertEqual(alpha["status"], "UNKNOWN")

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            allocator = allocator_trace()
            packet = pcode_v3_authenticated_trace()
            allocator_path = root / "allocator.json"
            pcode_path = root / "pcode.json"
            other_path = root / "other.json"
            allocator_path.write_text(json.dumps(allocator), encoding="utf-8")
            pcode_path.write_text(json.dumps(packet), encoding="utf-8")
            other_path.write_text(json.dumps({"forged": True}), encoding="utf-8")
            with self.assertRaises(module.CorrelatorError):
                module.correlate(
                    allocator,
                    packet,
                    allocator_path=allocator_path,
                    pcode_path=other_path,
                    expected_source_sha256=SOURCE_SHA,
                    expected_compiler_sha256=COMPILER_SHA,
                    expected_allocator_trace_sha256=self._sha256(allocator_path),
                    expected_pcode_v3_trace_sha256=self._sha256(pcode_path),
                )

        report = self.correlate_v3_primary(expected_ownership_sha256=None)
        alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
        self.assertEqual(alpha["status"], "MATCHED_AUTHENTICATED")

    def test_v3_direct_sessions_must_bind_to_manifest_and_each_other(self):
        def mutate(packet):
            packet["ownership"]["session"]["session_id"] = "other-session"

        report = self.correlate_v3_primary(pcode_v3=pcode_v3_authenticated_trace(mutate=mutate))
        alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
        self.assertEqual(alpha["status"], "UNKNOWN")
        self.assertFalse(report["authentication_gate"]["valid"])

    def test_v3_anchored_argv_path_with_hex_component_is_order_independent(self):
        with tempfile.TemporaryDirectory() as directory:
            parent = Path(directory) / "fixture-0x1234"
            parent.mkdir()
            report = self.correlate_v3_primary(temporary_parent=parent)
        alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
        self.assertEqual(alpha["status"], "MATCHED_AUTHENTICATED")

    def test_v3_duplicate_missing_reused_and_one_to_many_rows_fail_closed(self):
        mutations = {
            "duplicate": lambda packet: packet["frontend_join"]["direct_object_vregs"].append(
                {"object_ordinal": 0, "vreg_id": "r34", "status": "AUTHENTICATED"}
            ),
            "missing": lambda packet: packet["frontend_join"]["direct_object_vregs"].pop(),
            "reused": lambda packet: packet["frontend_join"]["direct_object_vregs"][1].__setitem__(
                "vreg_id", "r32"
            ),
            "one-to-many": lambda packet: packet["source_inventory"]["locals"][0]["vreg_ids"].append(
                "r34"
            ),
        }
        for label, mutate in mutations.items():
            with self.subTest(label):
                report = self.correlate_v3_primary(pcode_v3=pcode_v3_authenticated_trace(mutate=mutate))
                alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
                self.assertEqual(alpha["status"], "UNKNOWN")
                self.assertTrue(report["fail_closed"])

    def test_v3_forged_name_or_unique_vreg_cannot_replace_direct_ownership(self):
        def forge_name(packet):
            packet["source_inventory"]["locals"][0]["name"] = "unique_forged_name"

        def forge_vreg(packet):
            packet["source_inventory"]["locals"][0]["vreg_ids"] = ["r99"]

        for label, mutate in (("name", forge_name), ("vreg", forge_vreg)):
            with self.subTest(label):
                report = self.correlate_v3_primary(
                    pcode_v3=pcode_v3_authenticated_trace(mutate=mutate)
                )
                alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
                self.assertEqual(alpha["status"], "UNKNOWN")

    def test_authentication_gate_rejects_empty_or_mismatched_provenance(self):
        for label, mutate in (
            ("empty authentication", lambda trace: trace.__setitem__("authentication", {})),
            (
                "function mismatch",
                lambda trace: trace["capture"].__setitem__("function", "OtherFunction"),
            ),
            (
                "compiler mismatch",
                lambda trace: trace["authentication"]["artifacts"]["compiler"].__setitem__(
                    "sha256", "c" * 64
                ),
            ),
            (
                "source provenance mismatch",
                lambda trace: trace["authentication"].__setitem__(
                    "source_provenance", "AUTHENTICATED_TEST_SOURCE_DEADBEEF"
                ),
            ),
        ):
            with self.subTest(label):
                trace = pcode_trace(explicit=True)
                mutate(trace)
                with self.assertRaises(module.CorrelatorError):
                    self.correlate_authenticated(allocator_trace(), trace)

    def test_wrong_but_valid_external_source_or_compiler_digest_rejects(self):
        for key, value in (
            ("expected_source_sha256", "c" * 64),
            ("expected_compiler_sha256", "d" * 64),
        ):
            with self.subTest(key=key):
                with self.assertRaises(module.CorrelatorError):
                    self.correlate_authenticated(
                        allocator_trace(),
                        pcode_trace(explicit=True),
                        **{key: value},
                    )

    def test_duplicate_pcode_metadata_boundaries_fail_closed(self):
        trace = pcode_trace(explicit=True)
        trace["function"] = "OtherFunction"
        with self.assertRaises(module.CorrelatorError):
            self.correlate_authenticated(allocator_trace(), trace)

        trace = pcode_trace(explicit=True)
        trace["status"] = "PARTIAL"
        with self.assertRaises(module.CorrelatorError):
            self.correlate_authenticated(allocator_trace(), trace)

    def test_allocator_status_must_be_capture_complete(self):
        for status in ("dumped", "partial", "UNKNOWN"):
            with self.subTest(status=status):
                trace = allocator_trace()
                trace["status"] = status
                with self.assertRaises(module.CorrelatorError):
                    self.correlate_authenticated(trace, pcode_trace(explicit=True))

    def test_raw_trace_anchors_require_all_exact_trusted_bytes(self):
        allocator = allocator_trace()
        pcode = pcode_trace(explicit=True)
        pcode_v3 = pcode_v3_trace()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            allocator_path = root / "allocator.json"
            pcode_path = root / "pcode.json"
            pcode_v3_path = root / "pcode-v3.json"
            allocator_path.write_text(json.dumps(allocator), encoding="utf-8")
            pcode_path.write_text(json.dumps(pcode), encoding="utf-8")
            pcode_v3_path.write_text(json.dumps(pcode_v3), encoding="utf-8")
            anchors = {
                "expected_allocator_trace_sha256": self._sha256(allocator_path),
                "expected_pcode_trace_sha256": self._sha256(pcode_path),
                "expected_pcode_v3_trace_sha256": self._sha256(pcode_v3_path),
            }

            report = module.correlate(
                allocator,
                pcode,
                pcode_v3_trace=pcode_v3,
                allocator_path=allocator_path,
                pcode_path=pcode_path,
                pcode_v3_path=pcode_v3_path,
                expected_source_sha256=SOURCE_SHA,
                expected_compiler_sha256=COMPILER_SHA,
                **anchors,
            )
            alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
            self.assertEqual(alpha["status"], "MATCHED_AUTHENTICATED")

            with self.assertRaises(module.CorrelatorError):
                module.correlate(
                    allocator,
                    pcode,
                    allocator_path=allocator_path,
                    pcode_path=pcode_path,
                    pcode_v3_path=pcode_v3_path,
                    expected_source_sha256=SOURCE_SHA,
                    expected_compiler_sha256=COMPILER_SHA,
                    **anchors,
                )

            forged_pcode = json.loads(json.dumps(pcode))
            forged_pcode["capture"]["source_inventory"]["locals"][0]["vreg_ids"] = ["r33"]
            with self.assertRaises(module.CorrelatorError):
                module.correlate(
                    allocator,
                    forged_pcode,
                    pcode_v3_trace=pcode_v3,
                    allocator_path=allocator_path,
                    pcode_path=pcode_path,
                    pcode_v3_path=pcode_v3_path,
                    expected_source_sha256=SOURCE_SHA,
                    expected_compiler_sha256=COMPILER_SHA,
                    **anchors,
                )

            for key in anchors:
                wrong = dict(anchors)
                wrong[key] = "c" * 64
                with self.subTest(anchor=key):
                    with self.assertRaises(module.CorrelatorError):
                        module.correlate(
                            allocator,
                            pcode,
                            pcode_v3_trace=pcode_v3,
                            allocator_path=allocator_path,
                            pcode_path=pcode_path,
                            pcode_v3_path=pcode_v3_path,
                            expected_source_sha256=SOURCE_SHA,
                            expected_compiler_sha256=COMPILER_SHA,
                            **wrong,
                        )

            pcode_path.write_bytes(b" \n" + pcode_path.read_bytes())
            with self.assertRaises(module.CorrelatorError):
                module.correlate(
                    allocator,
                    pcode,
                    pcode_v3_trace=pcode_v3,
                    allocator_path=allocator_path,
                    pcode_path=pcode_path,
                    pcode_v3_path=pcode_v3_path,
                    expected_source_sha256=SOURCE_SHA,
                    expected_compiler_sha256=COMPILER_SHA,
                    **anchors,
                )

    def test_empty_and_duplicate_vreg_ids_fail_closed(self):
        trace = pcode_trace(explicit=True)
        trace["capture"]["pcode"]["backend-00-initial-code.txt"]["instructions"][0][
            "virtual_registers"
        ] = [""]
        with self.assertRaises(module.CorrelatorError):
            module.correlate(allocator_trace(), trace)

        for malformed in ("rX", "x1", "f-1"):
            with self.subTest(malformed=malformed):
                trace = pcode_trace(explicit=True)
                trace["capture"]["source_inventory"]["locals"][0]["vreg_ids"] = [malformed]
                with self.assertRaises(module.CorrelatorError):
                    module.correlate(allocator_trace(), trace)

        trace = pcode_trace(explicit=True)
        trace["capture"]["vreg_chronology"]["vregs"].append(
            dict(trace["capture"]["vreg_chronology"]["vregs"][0])
        )
        with self.assertRaises(module.CorrelatorError):
            module.correlate(allocator_trace(), trace)

    def test_duplicate_inventory_names_and_claims_fail_closed(self):
        trace = pcode_trace(explicit=True)
        trace["capture"]["source_inventory"]["locals"].append(
            {"name": "alpha", "vreg_ids": ["r32"], "vreg_status": "AUTHENTICATED"}
        )
        with self.assertRaises(module.CorrelatorError):
            self.correlate_authenticated(allocator_trace(), trace)

        trace = pcode_trace(explicit=True)
        trace["capture"]["source_inventory"]["locals"][1].update(
            {"vreg_ids": ["r32"], "vreg_status": "AUTHENTICATED"}
        )
        with self.assertRaises(module.CorrelatorError):
            self.correlate_authenticated(allocator_trace(), trace)

    def test_chronology_only_declared_vreg_is_unknown(self):
        trace = pcode_trace(explicit=True)
        chronology_only = dict(trace["capture"]["vreg_chronology"]["vregs"][0])
        chronology_only["vreg_id"] = "r99"
        trace["capture"]["vreg_chronology"]["vregs"].append(chronology_only)
        trace["capture"]["source_inventory"]["locals"][0]["vreg_ids"] = ["r99"]
        report = self.correlate_authenticated(allocator_trace(), trace)
        alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
        self.assertEqual(alpha["status"], "UNKNOWN")

    def test_authenticated_claim_requires_external_digest_anchors(self):
        with self.assertRaises(module.CorrelatorError):
            module.correlate(allocator_trace(), pcode_trace(explicit=True))

    def test_forged_v3_claim_cannot_authenticate(self):
        trace = {
            "schema": "mwcc_gc26_pcode_trace/v3",
            "status": "CAPTURED",
            "function": "CapEffThrowMasu",
            "capture_status": "CAPTURED",
            "authentication": {
                "source_hash_authenticated": True,
                "source_provenance": "AUTHENTICATED_TEST_SOURCE_AAAAAAAA",
                "artifacts": {
                    "source": {"sha256": SOURCE_SHA},
                    "compiler": {"sha256": COMPILER_SHA},
                },
            },
            "source_inventory": {
                "status": "CAPTURED",
                "locals": [
                    {"name": "alpha", "vreg_ids": ["r32"], "vreg_status": "AUTHENTICATED"}
                ],
            },
            "stages": {
                "backend-00-initial-code.pcode.json": {
                    "instructions": [
                        {
                            "order": 0,
                            "block": 1,
                            "mnemonic": "stw",
                            "memory_objects": ["alpha"],
                            "virtual_registers": ["r32"],
                            "operands": [],
                        }
                    ]
                }
            },
        }
        report = self.correlate_authenticated(allocator_trace(), trace)
        alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
        self.assertEqual(alpha["status"], "UNKNOWN")
        self.assertTrue(report["fail_closed"])

    def test_cli_invalid_authenticated_claim_is_error_exit_2(self):
        trace = pcode_trace(explicit=True)
        trace["authentication"] = {}
        with tempfile.TemporaryDirectory() as directory:
            allocator_path = Path(directory) / "allocator.json"
            pcode_path = Path(directory) / "pcode.json"
            allocator_path.write_text(json.dumps(allocator_trace()), encoding="utf-8")
            pcode_path.write_text(json.dumps(trace), encoding="utf-8")
            result = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT),
                    "--allocator",
                    str(allocator_path),
                    "--pcode",
                    str(pcode_path),
                    "--expected-source-sha256",
                    SOURCE_SHA,
                    "--expected-compiler-sha256",
                    COMPILER_SHA,
                ],
                check=False,
                capture_output=True,
                text=True,
            )
        self.assertEqual(result.returncode, 2)
        body = json.loads(result.stdout)
        self.assertEqual(body["status"], "ERROR")
        self.assertTrue(body["fail_closed"])

    def test_cli_accepts_v3_as_primary_and_optional_input(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            allocator = allocator_trace()
            pcode_v2 = pcode_trace(explicit=True)
            pcode_v3 = pcode_v3_authenticated_trace()
            allocator_path = root / "allocator.json"
            pcode_v2_path = root / "pcode-v2.json"
            pcode_v3_path = root / "pcode-v3.json"
            allocator_path.write_text(json.dumps(allocator), encoding="utf-8")
            pcode_v2_path.write_text(json.dumps(pcode_v2), encoding="utf-8")
            pcode_v3_path.write_text(json.dumps(pcode_v3), encoding="utf-8")

            common = [
                sys.executable,
                str(SCRIPT),
                "--allocator",
                str(allocator_path),
                "--expected-source-sha256",
                SOURCE_SHA,
                "--expected-compiler-sha256",
                COMPILER_SHA,
                "--expected-allocator-trace-sha256",
                self._sha256(allocator_path),
                "--expected-ownership-sha256",
                OWNERSHIP_SHA,
            ]
            primary = subprocess.run(
                common
                + [
                    "--pcode",
                    str(pcode_v3_path),
                    "--expected-pcode-v3-trace-sha256",
                    self._sha256(pcode_v3_path),
                ],
                check=False,
                capture_output=True,
                text=True,
            )
            optional = subprocess.run(
                common
                + [
                    "--pcode",
                    str(pcode_v2_path),
                    "--pcode-v3",
                    str(pcode_v3_path),
                    "--expected-pcode-trace-sha256",
                    self._sha256(pcode_v2_path),
                    "--expected-pcode-v3-trace-sha256",
                    self._sha256(pcode_v3_path),
                ],
                check=False,
                capture_output=True,
                text=True,
            )
            for result in (primary, optional):
                with self.subTest(returncode=result.returncode):
                    self.assertEqual(result.returncode, 0, result.stderr)
                    body = json.loads(result.stdout)
                    alpha = next(
                        row for row in body["mappings"] if row["allocator"]["name"] == "alpha"
                    )
                    self.assertEqual(alpha["status"], "UNKNOWN")

    def test_duplicate_allocator_names_are_ambiguous(self):
        report = module.correlate(allocator_trace(duplicate=True), pcode_trace())
        rows = [row for row in report["mappings"] if row["allocator"]["name"] == "alpha"]
        self.assertEqual(len(rows), 2)
        self.assertTrue(all(row["status"] == "AMBIGUOUS" for row in rows))

    def test_v3_without_vregs_cannot_upgrade_v2_evidence(self):
        v3 = {
            "schema": "mwcc_gc26_pcode_trace/v3",
            "status": "UNKNOWN",
            "stages": {
                "backend-00-initial-code.pcode.json": {
                    "instructions": [
                        {
                            "order": 0,
                            "block": 1,
                            "mnemonic": "stfs",
                            "operands": [],
                            "sourceoffset": {"status": "EXACT", "value": 10},
                        }
                    ]
                }
            },
            "limitations": ["frontend join unavailable"],
        }
        report = module.correlate(allocator_trace(), pcode_trace(), pcode_v3_trace=v3)
        self.assertTrue(any("v3" in item for item in report["limitations"]))
        self.assertEqual(report["mappings"][0]["status"], "UNRESOLVED_EVIDENCE")

    def test_optional_malformed_v3_sidecar_cannot_downgrade_to_v2_success(self):
        sidecar = pcode_v3_trace()
        sidecar["unexpected_sidecar_field"] = "must be rejected"
        report = self.correlate_authenticated(
            allocator_trace(),
            pcode_trace(explicit=True),
            pcode_v3=sidecar,
        )
        alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
        self.assertEqual(alpha["status"], "UNKNOWN")
        self.assertFalse(report["authentication_gate"]["valid"])

    def test_pointer_free_boundary_rejects_ig_node_variants_and_free_text(self):
        for label, mutate in (
            (
                "ig-node key",
                lambda packet: packet.__setitem__("igNodeRef", 7),
            ),
            (
                "reason text",
                lambda packet: packet["source_inventory"].__setitem__("reason", "raw 0x123"),
            ),
            (
                "limitations text",
                lambda packet: packet.__setitem__("limitations", ["raw 0x0 pointer"]),
            ),
            (
                "non-string reason",
                lambda packet: packet["ownership"].__setitem__("reason", {"raw": 1}),
            ),
        ):
            with self.subTest(label=label):
                with self.assertRaises(module.CorrelatorError):
                    self.correlate_v3_primary(
                        pcode_v3=pcode_v3_authenticated_trace(mutate=mutate)
                    )
        with self.assertRaises(module.CorrelatorError):
            module._reject_pointer_material({"argv": ["mwcc", "-c", "0x123"]})

    def test_conflicting_primary_v3_aliases_return_structured_fail_closed_error(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            allocator_path = root / "allocator.json"
            pcode_path = root / "pcode-v3.json"
            allocator_path.write_text(json.dumps(allocator_trace()), encoding="utf-8")
            pcode_path.write_text(json.dumps(pcode_v3_authenticated_trace()), encoding="utf-8")
            trust = module.ExternalTrustRoot(
                pcode_path=pcode_path,
                pcode_sha256="a" * 64,
                pcode_size=pcode_path.stat().st_size,
                pcode_v3_path=pcode_path,
                pcode_v3_sha256="b" * 64,
                pcode_v3_size=pcode_path.stat().st_size,
            )
            report = module.correlate(
                allocator_trace(),
                pcode_v3_authenticated_trace(),
                trust_root=trust,
                allocator_path=allocator_path,
                pcode_path=pcode_path,
                ownership_path=root / "ownership.json",
            )
        self.assertEqual(report["status"], "ERROR")
        self.assertTrue(report["fail_closed"])
        self.assertFalse(report["authority_advanced"])
        self.assertEqual(report["report_sha256"], module._report_hash(report))
        self.assertNotIn("mappings", report)

    def test_dot_path_alias_stress_rejects_all_twelve_hundred_spellings(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            target = root / "capture.json"
            target.write_text("{}", encoding="utf-8")
            rejected = 0
            for index in range(1200):
                alias = f"{root}\\..\\{root.name}\\capture.json"
                try:
                    module._canonical_external_path(alias, f"capture[{index}]")
                except module.CorrelatorError:
                    rejected += 1
            self.assertEqual(rejected, 1200)

    def test_caller_ownership_copy_cannot_replace_anchored_ownership_path(self):
        allocator = allocator_trace()
        packet = pcode_v3_authenticated_trace()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            allocator_path = root / "allocator.json"
            pcode_path = root / "pcode-v3.json"
            allocator_path.write_text(json.dumps(allocator), encoding="utf-8")
            trust = self._trusted_v3_bundle(
                root,
                allocator_path,
                pcode_path,
                packet,
                primary_v3=True,
                allocator_trace=allocator,
            )
            ownership_copy = root / "ownership-copy.json"
            ownership_copy.write_bytes(trust.ownership_path.read_bytes())
            report = module.correlate(
                allocator,
                packet,
                trust_root=trust,
                allocator_path=allocator_path,
                pcode_path=pcode_path,
                ownership_path=ownership_copy,
                expected_source_sha256=trust.source_sha256,
                expected_compiler_sha256=trust.compiler_sha256,
                expected_allocator_trace_sha256=trust.allocator_sha256,
                expected_pcode_v3_trace_sha256=trust.pcode_sha256,
                expected_ownership_sha256=trust.ownership_sha256,
            )
        self.assertEqual(report["status"], "ERROR")
        self.assertTrue(report["fail_closed"])

    def test_in_memory_allocator_and_pcode_forgery_cannot_override_anchored_bytes(self):
        allocator = allocator_trace()
        packet = pcode_v3_authenticated_trace()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            allocator_path = root / "allocator.json"
            pcode_path = root / "pcode-v3.json"
            allocator_path.write_text(json.dumps(allocator), encoding="utf-8")
            trust = self._trusted_v3_bundle(
                root,
                allocator_path,
                pcode_path,
                packet,
                primary_v3=True,
                allocator_trace=allocator,
            )
            forged_allocator = json.loads(json.dumps(allocator))
            forged_allocator["assignment_events"][0]["after_locals"][0]["name"] = "forged"
            forged_packet = json.loads(json.dumps(packet))
            forged_packet["source_inventory"]["locals"][0]["name"] = "forged"
            for forged_allocator_value, forged_packet_value in (
                (forged_allocator, packet),
                (allocator, forged_packet),
            ):
                with self.subTest(forged="allocator" if forged_allocator_value is forged_allocator else "pcode"):
                    with self.assertRaises(module.CorrelatorError):
                        module.correlate(
                            forged_allocator_value,
                            forged_packet_value,
                            trust_root=trust,
                            allocator_path=allocator_path,
                            pcode_path=pcode_path,
                            ownership_path=trust.ownership_path,
                            expected_source_sha256=trust.source_sha256,
                            expected_compiler_sha256=trust.compiler_sha256,
                            expected_allocator_trace_sha256=trust.allocator_sha256,
                            expected_pcode_v3_trace_sha256=trust.pcode_sha256,
                            expected_ownership_sha256=trust.ownership_sha256,
                        )

    def test_wrong_payload_pcode_and_normalized_aliases_cannot_authenticate(self):
        for field in ("pcode_path", "normalized_v3_path", "pcode_v3_path"):
            with self.subTest(field=field):
                def mutate(packet, field=field):
                    packet["authentication"][field] = "C:/wrong/anchored-payload"

                report = self.correlate_v3_primary(
                    pcode_v3=pcode_v3_authenticated_trace(mutate=mutate)
                )
                alpha = next(row for row in report["mappings"] if row["allocator"]["name"] == "alpha")
                self.assertEqual(alpha["status"], "UNKNOWN")
                self.assertFalse(report["authentication_gate"]["valid"])

    def test_report_is_deterministic(self):
        first = module.correlate(allocator_trace(), pcode_trace())
        second = module.correlate(allocator_trace(), pcode_trace())
        self.assertEqual(
            json.dumps(first, sort_keys=True, separators=(",", ":")),
            json.dumps(second, sort_keys=True, separators=(",", ":")),
        )

    def test_cli_help_and_fail_closed_json(self):
        help_result = subprocess.run(
            [sys.executable, str(SCRIPT), "--help"],
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(help_result.returncode, 0)
        self.assertIn("VarInfo", help_result.stdout)
        with self.subTest("invalid input"):
            bad = subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT),
                    "--allocator",
                    str(ROOT / "does-not-exist.json"),
                    "--pcode",
                    str(ROOT / "does-not-exist-pcode.json"),
                ],
                check=False,
                capture_output=True,
                text=True,
            )
            self.assertEqual(bad.returncode, 2)
            body = json.loads(bad.stdout)
            self.assertTrue(body["fail_closed"])
            self.assertEqual(body["status"], "ERROR")


if __name__ == "__main__":
    unittest.main()
