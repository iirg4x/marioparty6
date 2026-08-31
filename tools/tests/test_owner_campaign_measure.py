from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

from tools import owner_campaign_measure as adapter
from tools.owner_campaign_verify import verify_measurement


def _sha(value: object) -> str:
    return hashlib.sha256(
        json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode()
    ).hexdigest()


def _focus(function: str = "fn", *, differences: int = 1) -> dict[str, object]:
    rows = [
        {"index": index, "diff_kind": "DIFF_ARG_MISMATCH", "instruction": {"address": 100 + index}}
        for index in range(differences)
    ]
    metric = {"target_size": 32, "candidate_size": 32, "diff_rows": differences}
    channel = {
        "metric": metric,
        "target": {"rows": rows, "instruction_count": 8},
        "candidate": {"rows": rows, "instruction_count": 8},
        "protected_siblings": {
            "sibling_count": 1,
            "exact_sibling_count": 1,
            "exact_identities": ["sibling"],
        },
    }
    physical = {
        "target": {
            "physical_relocation_count": 2,
            "physical_relocations": [
                {"offset": 16, "type": "SDA21", "effective_target": "@1"},
                {"offset": 20, "type": "REL24", "effective_target": "fn"},
            ],
        },
        "candidate": {
            "physical_relocation_count": 2,
            "physical_relocations": [
                {"offset": 16, "type": "SDA21", "effective_target": "@1"},
                {"offset": 20, "type": "REL24", "effective_target": "fn"},
            ],
        },
        "physical_relocation_differences": [],
    }
    return {"channels": {"strict": channel, "data": channel}, "physical_relocations": physical}


class OwnerCampaignMeasureTests(unittest.TestCase):
    def test_physical_identity_ignores_symbol_aliases_but_not_effective_target(self) -> None:
        focus = _focus(differences=0)
        target_rows = focus["physical_relocations"]["target"]["physical_relocations"]
        candidate_rows = focus["physical_relocations"]["candidate"]["physical_relocations"]
        target_rows[0].update({"symbol": "lbl_802C4A6C", "symbol_value": 44, "addend": 0})
        candidate_rows[0].update({"symbol": "@380", "symbol_value": 0, "addend": 0})

        difference_ids, target_identity, candidate_identity = adapter._physical_identity(
            focus, "fn"
        )
        self.assertEqual(difference_ids, [])
        self.assertEqual(target_identity, candidate_identity)

        candidate_rows[0]["effective_target"] = "@different"
        _difference_ids, target_identity, candidate_identity = adapter._physical_identity(
            focus, "fn"
        )
        self.assertNotEqual(target_identity, candidate_identity)

    def test_unit_object_resolution_creates_fresh_candidate_parent(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            target = root / "build" / "GP6E01" / "obj" / "board" / "owner.o"
            candidate = root / "build" / "GP6E01" / "src" / "board" / "owner.o"
            target.parent.mkdir(parents=True)
            target.write_bytes(b"target")
            identity = adapter.Identity(
                phase="candidate", campaign_id="campaign", manifest_sha256="a" * 64,
                owner="main:board/owner", unit="main/board/owner", function="fn",
                source_sha256="b" * 64,
                target_object_sha256=hashlib.sha256(b"target").hexdigest(),
                toolchain_sha256="d" * 64, base_commit="e" * 40,
                source_path="src/board/owner.c",
            )
            with mock.patch.object(
                adapter.bundle, "_unit_paths", return_value=(target, candidate)
            ):
                resolved_target, resolved_candidate = adapter._unit_objects(root, identity)
            self.assertEqual(resolved_target, target)
            self.assertEqual(resolved_candidate, candidate)
            self.assertTrue(candidate.parent.is_dir())
            self.assertFalse(candidate.exists())

    def test_runtime_environment_names_bind_measurement_identity(self) -> None:
        args = argparse.Namespace(
            campaign_id=None,
            manifest_sha256=None,
            owner=None,
            unit=None,
            function=None,
            source_sha256=None,
            target_object_sha256=None,
            toolchain_sha256=None,
            base_commit=None,
        )
        environment = {
            "OWNER_CAMPAIGN_ID": "campaign",
            "OWNER_CAMPAIGN_MANIFEST_SHA256": "a" * 64,
            "OWNER_CAMPAIGN_OWNER": "main:board/test",
            "OWNER_CAMPAIGN_UNIT": "main/board/test",
            "OWNER_CAMPAIGN_FUNCTION": "focus",
            "OWNER_CAMPAIGN_SOURCE_SHA256": "b" * 64,
            "OWNER_CAMPAIGN_TARGET_SHA256": "c" * 64,
            "OWNER_CAMPAIGN_TOOLCHAIN_SHA256": "d" * 64,
            "OWNER_CAMPAIGN_BASE_COMMIT": "e" * 40,
        }
        with mock.patch.dict(adapter.os.environ, environment, clear=True):
            identity = adapter._identity(args, "snapshot")
        self.assertEqual(identity.campaign_id, "campaign")
        self.assertEqual(identity.target_object_sha256, "c" * 64)

    def test_measurement_is_closed_and_compact(self) -> None:
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
            identity = adapter.Identity(
                phase="candidate", campaign_id="campaign", manifest_sha256="a" * 64,
                owner="main:board/test", unit="main/board/test", function="fn",
                source_sha256="b" * 64, target_object_sha256="c" * 64,
                toolchain_sha256="d" * 64, base_commit="e" * 40,
                source_path="src/test.c",
            )
            args = adapter._parser().parse_args(["--protected-total", "1"])
            value = adapter._measurement(
                identity, _focus(), args, strict_path=strict, data_path=data,
                physical_path=physical, candidate=candidate,
            )
            self.assertEqual(value["schema"], adapter.MEASUREMENT_SCHEMA)
            unsigned = dict(value)
            digest = unsigned.pop("measurement_sha256")
            self.assertEqual(digest, _sha(unsigned))
            self.assertFalse(value["metrics"]["source_link_exact"])
            self.assertLessEqual(
                len(json.dumps(value, separators=(",", ":")).encode("utf-8")),
                adapter.MAX_MEASUREMENT_COMPACT,
            )
            self.assertEqual(set(value), {
                "schema", "phase", "campaign_id", "manifest_sha256", "owner", "unit",
                "function", "source_path", "base_commit", "source_sha256",
                "target_object_sha256", "toolchain_sha256",
                "candidate_object_sha256", "metrics", "report_receipts", "focus_evidence",
                "proofs", "exact_report", "measurement_producer_sha256", "measurement_sha256",
            })

    def test_exact_measurement_passes_independent_verifier(self) -> None:
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
            identity = adapter.Identity(
                phase="candidate", campaign_id="campaign", manifest_sha256="a" * 64,
                owner="main:board/test", unit="main/board/test", function="fn",
                source_sha256="b" * 64, target_object_sha256="c" * 64,
                toolchain_sha256="d" * 64, base_commit="e" * 40,
                source_path="src/test.c",
            )
            args = adapter._parser().parse_args(["--protected-total", "1"])
            proof = {
                "schema": "owner_campaign_source_compile_proof/v1",
                "source_path": "src/test.c",
                "source_sha256": "b" * 64,
                "candidate_object_path": "build/test.o",
                "candidate_object_sha256": hashlib.sha256(b"candidate").hexdigest(),
                "compiler_commands": ["mwcc -o build/test.o src/test.c"],
                "paired_compile_command_sha256": adapter._sha_bytes(
                    b"mwcc -o build/test.o src/test.c"
                ),
                "object_origin": "reconstructed_source",
                "fallback_asm_used": False,
                "nonmatching_fallback_linked": False,
                "authority_advanced": False,
            }
            proof = adapter._seal(proof, "proof_sha256")
            value = adapter._measurement(
                identity, _focus(differences=0), args, strict_path=strict,
                data_path=data, physical_path=physical, candidate=candidate,
                source_link_exact=True, source_link_proof=proof,
            )
            verified = verify_measurement(value)
            self.assertTrue(verified["verified"])

    def test_source_link_proof_rejects_missing_paired_command(self) -> None:
        command = "mwcc -o build/test.o src/test.c"
        proof = adapter._seal({
            "schema": "owner_campaign_source_compile_proof/v1",
            "source_path": "src/test.c",
            "source_sha256": "b" * 64,
            "candidate_object_path": "build/test.o",
            "candidate_object_sha256": "c" * 64,
            "compiler_commands": [command],
            "paired_compile_command_sha256": adapter._sha_bytes(b"different"),
            "object_origin": "reconstructed_source",
            "fallback_asm_used": False,
            "nonmatching_fallback_linked": False,
            "authority_advanced": False,
        }, "proof_sha256")
        with self.assertRaisesRegex(adapter.MeasurementError, "paired compiler command"):
            adapter._compact_source_link_proof(proof)

    def test_source_compile_provenance_accepts_mwcc_directory_output(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            source = root / "src" / "board" / "mgcall.c"
            candidate = root / "build" / "GP6E01" / "src" / "board" / "mgcall.o"
            source.parent.mkdir(parents=True)
            candidate.parent.mkdir(parents=True)
            source.write_text("int x;", encoding="utf-8")
            command = (
                "sjiswrap.exe mwcceppc.exe -c src\\board\\mgcall.c "
                "-o build\\GP6E01\\src\\board"
            )
            proof = adapter._assert_source_compile_provenance(
                command, root=root, source=source, candidate=candidate,
                owner="main:board/mgcall",
            )
            self.assertEqual(proof["candidate_object_path"], "build/GP6E01/src/board/mgcall.o")

            wrong = candidate.with_name("different.o")
            with self.assertRaisesRegex(adapter.MeasurementError, "does not bind"):
                adapter._assert_source_compile_provenance(
                    command, root=root, source=source, candidate=wrong,
                    owner="main:board/mgcall",
                )

    def test_focus_evidence_hash_is_deterministic(self) -> None:
        identity = adapter.Identity(
            phase="snapshot", campaign_id="campaign", manifest_sha256="a" * 64,
            owner="owner", unit="main/board/test", function="fn", source_sha256="b" * 64,
            target_object_sha256="c" * 64, toolchain_sha256="d" * 64,
            base_commit="e" * 40, source_path="src/test.c",
        )
        args = adapter._parser().parse_args(["--protected-total", "1"])
        first = adapter._focus_evidence(identity, _focus(), args)
        second = adapter._focus_evidence(identity, _focus(), args)
        self.assertEqual(first, second)
        unsigned = dict(first)
        digest = unsigned.pop("focus_evidence_sha256")
        self.assertEqual(digest, _sha(unsigned))
        self.assertEqual(len(first["strict_rows"]), 2)
        self.assertIn("addr=", first["strict_rows"][0])
        self.assertIn("row_sha256=", first["strict_rows"][0])

    def test_protected_census_uses_named_runtime_set(self) -> None:
        focus = _focus()
        focus["channels"]["strict"]["protected_siblings"]["sibling_count"] = 4
        focus["channels"]["strict"]["protected_siblings"]["exact_identities"] = [
            "sibling", "newly_exact"
        ]
        focus["channels"]["strict"]["protected_siblings"]["exact_sibling_count"] = 2
        identity = adapter.Identity(
            phase="candidate", campaign_id="campaign", manifest_sha256="a" * 64,
            owner="owner", unit="main/board/test", function="fn", source_sha256="b" * 64,
            target_object_sha256="c" * 64, toolchain_sha256="d" * 64,
            base_commit="e" * 40, source_path="src/test.c",
        )
        args = adapter._parser().parse_args([])
        with mock.patch.dict(
            "os.environ",
            {"OWNER_CAMPAIGN_PROTECTED_FUNCTIONS": "sibling"},
            clear=False,
        ):
            total, losses, identities, _digest = adapter._protected(
                focus, expected_total=1,
                expected_names=adapter._expected_protected_names(args),
            )
        self.assertEqual((total, losses), (1, 0))
        self.assertEqual(identities, ["newly_exact", "sibling"])

    def test_protected_census_excludes_selected_focus(self) -> None:
        focus = _focus()
        args = adapter._parser().parse_args([])
        with mock.patch.dict(
            "os.environ",
            {"OWNER_CAMPAIGN_PROTECTED_FUNCTIONS": "fn,sibling"},
            clear=False,
        ):
            total, losses, identities, _digest = adapter._protected(
                focus,
                expected_total=2,
                expected_names=adapter._expected_protected_names(args),
                focus_function="fn",
            )
        self.assertEqual((total, losses), (1, 0))
        self.assertEqual(identities, ["sibling"])

    def test_protected_census_counts_missing_named_identity(self) -> None:
        focus = _focus()
        identity = adapter.Identity(
            phase="candidate", campaign_id="campaign", manifest_sha256="a" * 64,
            owner="owner", unit="main/board/test", function="fn", source_sha256="b" * 64,
            target_object_sha256="c" * 64, toolchain_sha256="d" * 64,
            base_commit="e" * 40, source_path="src/test.c",
        )
        args = adapter._parser().parse_args([])
        with mock.patch.dict(
            "os.environ",
            {"OWNER_CAMPAIGN_PROTECTED_FUNCTIONS": "sibling,missing"},
            clear=False,
        ):
            total, losses, _identities, _digest = adapter._protected(
                focus, expected_total=2,
                expected_names=adapter._expected_protected_names(args),
            )
        self.assertEqual((total, losses), (2, 1))

    def test_large_focus_is_bounded_with_omission_digest(self) -> None:
        identity = adapter.Identity(
            phase="snapshot", campaign_id="campaign", manifest_sha256="a" * 64,
            owner="owner", unit="main/board/test", function="fn", source_sha256="b" * 64,
            target_object_sha256="c" * 64, toolchain_sha256="d" * 64,
            base_commit="e" * 40, source_path="src/test.c",
        )
        rows = [
            {"index": index, "diff_kind": "DIFF_REPLACE", "instruction": {
                "address": index * 4, "formatted": "very-long-op " + ("x" * 300),
            }}
            for index in range(512)
        ]
        focus = _focus(differences=0)
        focus["channels"]["strict"]["target"]["rows"] = rows
        focus["channels"]["strict"]["candidate"]["rows"] = rows
        focus["channels"]["strict"]["metric"]["diff_rows"] = len(rows)
        focus["channels"]["data"]["target"]["rows"] = rows
        focus["channels"]["data"]["candidate"]["rows"] = rows
        focus["channels"]["data"]["metric"]["diff_rows"] = len(rows)
        args = adapter._parser().parse_args(["--protected-total", "1"])
        value = adapter._focus_evidence(identity, focus, args)
        encoded = json.dumps(value, separators=(",", ":"))
        self.assertLessEqual(len(encoded.encode()), adapter.MAX_FOCUS_COMPACT)
        self.assertTrue(any("omitted=" in item for item in value["strict_rows"]))

    def test_snapshot_reports_retain_nonexact_physical_frontier(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            target = root / "target.o"
            candidate = root / "candidate.o"
            target.write_bytes(b"target")
            candidate.write_bytes(b"candidate")
            (root / "strict.json").write_text("{}", encoding="utf-8")
            (root / "data.json").write_text("{}", encoding="utf-8")
            identity = adapter.Identity(
                phase="snapshot", campaign_id="campaign", manifest_sha256="a" * 64,
                owner="owner", unit="main/board/test", function="fn",
                source_sha256="b" * 64,
                target_object_sha256=hashlib.sha256(b"target").hexdigest(),
                toolchain_sha256="d" * 64, base_commit="e" * 40,
                source_path="src/test.c",
            )
            with (
                mock.patch.object(adapter.bundle, "_run_objdiff"),
                mock.patch.object(
                    adapter.bundle, "_physical_receipt",
                    return_value={"status": "mismatch", "differences": [{"offset": 4}]},
                ),
                mock.patch.object(
                    adapter.focus_symbol_report, "build_from_paths", return_value=_focus()
                ) as build,
            ):
                adapter._run_reports(
                    root=root, identity=identity, target=target, candidate=candidate,
                    objdiff=root / "objdiff.exe", readelf=root / "readelf.exe",
                    temp=root, deadline=adapter.Deadline(5),
                )
            self.assertFalse(build.call_args.kwargs["require_physical"])

    def test_path_escape_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            with self.assertRaises(adapter.MeasurementError):
                adapter._absolute("../outside.json", root=root, label="output",
                                  allow_external=False, exists=False)

    def test_bound_snapshot_rejects_source_target_and_candidate_drift(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            source = root / "source.c"
            target = root / "target.o"
            candidate = root / "candidate.o"
            source.write_bytes(b"source")
            target.write_bytes(b"target")
            candidate.write_bytes(b"candidate")
            expected = {
                "source": hashlib.sha256(source.read_bytes()).hexdigest(),
                "target": hashlib.sha256(target.read_bytes()).hexdigest(),
                "candidate": hashlib.sha256(candidate.read_bytes()).hexdigest(),
            }
            for label, path, message in (
                ("source", source, "source changed"),
                ("target", target, "target object changed"),
                ("candidate", candidate, "candidate object changed"),
            ):
                original = path.read_bytes()
                path.write_bytes(original + b"-drift")
                with self.assertRaisesRegex(adapter.MeasurementError, message):
                    adapter._assert_bound_snapshot(
                        source=source, target=target, candidate=candidate,
                        expected_source_sha256=expected["source"],
                        expected_target_sha256=expected["target"],
                        expected_candidate_sha256=expected["candidate"],
                        label="test boundary",
                    )
                path.write_bytes(original)

    def test_bounded_runner_terminates(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            with self.assertRaisesRegex(adapter.MeasurementError, "deadline"):
                adapter._run_bounded(
                    [sys.executable, "-c", "import time; time.sleep(2)"],
                    cwd=root, deadline=adapter.Deadline(0.1), label="slow fixture",
                )

    def test_bounded_runner_accepts_pathlike_executable(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            output = adapter._run_bounded(
                [Path(sys.executable), "-c", "print('ready')"],
                cwd=Path(raw), deadline=adapter.Deadline(5), label="path fixture",
            )
        self.assertEqual(output.strip(), "ready")

    def test_toolchain_descriptor_and_internal_hash_domains_are_distinct(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            manifest = root / "toolchain.json"
            objdiff = root / "objdiff.exe"
            ninja = root / "ninja.exe"
            dtk = root / "dtk.exe"
            readelf_dir = root / "binutils"
            readelf_dir.mkdir()
            readelf = readelf_dir / "powerpc-eabi-readelf.exe"
            for path in (objdiff, ninja, dtk, readelf):
                path.write_bytes(path.name.encode("utf-8"))
            unsigned = {"schema": "mp6_crack_toolchain/v1", "test": True}
            manifest.write_text(
                json.dumps({**unsigned, "manifest_sha256": _sha(unsigned)},
                           sort_keys=True, separators=(",", ":")),
                encoding="utf-8",
            )
            identity = adapter.Identity(
                phase="candidate", campaign_id="campaign", manifest_sha256="a" * 64,
                owner="owner", unit="main/board/test", function="fn", source_sha256="b" * 64,
                target_object_sha256="c" * 64,
                toolchain_sha256=hashlib.sha256(manifest.read_bytes()).hexdigest(),
                base_commit="e" * 40, source_path="src/test.c",
            )
            args = adapter._parser().parse_args(["--toolchain", str(manifest)])
            loaded = {
                "objdiff": {"path_object": objdiff},
                "binutils": {"path_object": readelf_dir},
                "ninja": {"path_object": ninja},
                "dtk": {"path_object": dtk},
            }
            previous = Path.cwd()
            try:
                import os

                os.chdir(root)
                with mock.patch.object(adapter.bundle, "_load_toolchain", return_value=loaded) as loader:
                    adapter._toolchain(args, identity)
                loader.assert_called_once_with(manifest, _sha(unsigned))
            finally:
                os.chdir(previous)

    def test_final_owner_requires_actual_link_inputs(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            source = root / "source.c"
            source.write_text("int x;", encoding="utf-8")
            args = adapter._parser().parse_args([
                "--phase", "final_owner", "--root", str(root),
                "--output", "out.json", "--source", str(source),
                "--campaign-id", "campaign", "--manifest-sha256", "a" * 64,
                "--owner", "owner", "--unit", "main/board/test", "--function", "fn",
                "--source-sha256", hashlib.sha256(source.read_bytes()).hexdigest(),
                "--target-object-sha256", "c" * 64, "--toolchain-sha256", "d" * 64,
                "--base-commit", "e" * 40,
            ])
            previous = Path.cwd()
            try:
                import os

                os.chdir(root)
                with self.assertRaisesRegex(adapter.MeasurementError, "link_target"):
                    adapter.final_owner(root=root, args=args)
            finally:
                os.chdir(previous)

    def test_fallback_asm_is_never_matching_source(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            source = root / "src" / "board" / "owner.c"
            candidate = root / "build" / "owner.o"
            source.parent.mkdir(parents=True)
            candidate.parent.mkdir(parents=True)
            source.write_text("int x;", encoding="utf-8")
            candidate.write_bytes(b"object")
            with self.assertRaisesRegex(adapter.MeasurementError, "fallback"):
                adapter._assert_linker_provenance(
                    "ld build/owner.o asm/board/owner.s", root=root,
                    candidate=candidate, source=source, owner="owner",
                )

    def test_source_link_proof_must_be_sealed_before_compaction(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            candidate = root / "candidate.o"
            candidate.write_bytes(b"candidate")
            strict = root / "strict.json"
            data = root / "data.json"
            physical = root / "physical.json"
            for path in (strict, data, physical):
                path.write_text("evidence", encoding="utf-8")
            identity = adapter.Identity(
                phase="candidate", campaign_id="campaign", manifest_sha256="a" * 64,
                owner="main:board/test", unit="main/board/test", function="fn",
                source_sha256="b" * 64, target_object_sha256="c" * 64,
                toolchain_sha256="d" * 64, base_commit="e" * 40,
                source_path="src/test.c",
            )
            bad = {
                "schema": "owner_campaign_source_compile_proof/v1",
                "source_sha256": "b" * 64,
                "candidate_object_sha256": hashlib.sha256(b"candidate").hexdigest(),
                "compiler_commands": [],
                "paired_compile_command_sha256": "0" * 64,
                "proof_sha256": "f" * 64,
            }
            with self.assertRaisesRegex(adapter.MeasurementError, "proof digest"):
                adapter._measurement(
                    identity, _focus(differences=0), adapter._parser().parse_args([]),
                    strict_path=strict, data_path=data, physical_path=physical,
                    candidate=candidate, source_link_exact=True,
                    source_link_proof=bad,
                )

    def test_linker_provenance_requires_source_and_object(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            source = root / "src" / "owner.c"
            candidate = root / "build" / "owner.o"
            source.parent.mkdir(parents=True)
            candidate.parent.mkdir(parents=True)
            source.write_text("int x;", encoding="utf-8")
            candidate.write_bytes(b"object")
            with self.assertRaisesRegex(adapter.MeasurementError, "source-built object"):
                adapter._assert_linker_provenance(
                    "ld src/owner.c", root=root, candidate=candidate,
                    source=source, owner="owner",
                )

    def test_linker_provenance_binds_compile_and_link_inputs(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            source = root / "src" / "owner.c"
            candidate = root / "build" / "owner.o"
            source.parent.mkdir(parents=True)
            candidate.parent.mkdir(parents=True)
            source.write_text("int x;", encoding="utf-8")
            candidate.write_bytes(b"object")
            manifest = adapter._assert_linker_provenance(
                "mwcc -o build/owner.o src/owner.c\n"
                "mwld -o build/main.dol build/owner.o",
                root=root, candidate=candidate, source=source, owner="owner",
                link_target=root / "build" / "main.dol",
                linked_binary=root / "build" / "main.dol",
            )
            self.assertEqual(manifest["object_origin"], "reconstructed_source")
            unsigned = dict(manifest)
            digest = unsigned.pop("manifest_sha256")
            self.assertEqual(digest, _sha(unsigned))

    def test_unrelated_fallback_link_input_is_allowed(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            source = root / "src" / "owner.c"
            candidate = root / "build" / "owner.o"
            source.parent.mkdir(parents=True)
            candidate.parent.mkdir(parents=True)
            source.write_text("int x;", encoding="utf-8")
            candidate.write_bytes(b"object")
            manifest = adapter._assert_linker_provenance(
                "mwcc -o build/owner.o src/owner.c\n"
                "mwld -o build/main.dol build/owner.o asm/board/other.o",
                root=root, candidate=candidate, source=source, owner="owner",
                link_target=root / "build" / "main.dol",
                linked_binary=root / "build" / "main.dol",
            )
            self.assertFalse(manifest["fallback_asm_used"])


if __name__ == "__main__":
    unittest.main()
