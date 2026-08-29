from __future__ import annotations

import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
from pathlib import Path

from tools import crack_evidence_bundle as bundle
from tools import focus_symbol_report


FUNCTION = "FocusFunction"
OBJDFF = Path(r"C:\Users\Anony\.codex\tools\objdiff\v3.8.0\objdiff-cli.exe")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class RealEvidenceFixtureTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.assembler = shutil.which("powerpc-eabi-as") or r"C:\devkitPro\devkitPPC\bin\powerpc-eabi-as.exe"
        self.readelf = shutil.which("powerpc-eabi-readelf") or r"C:\devkitPro\devkitPPC\bin\powerpc-eabi-readelf.exe"
        if not OBJDFF.is_file() or not Path(self.assembler).is_file() or not Path(self.readelf).is_file():
            self.skipTest("real pinned objdiff/PowerPC fixture tools are unavailable")
        source = self.root / "focus.s"
        source.write_text(
            ".section .text\n"
            ".globl FocusFunction\n"
            ".type FocusFunction,@function\n"
            "FocusFunction:\n"
            "  bl External\n"
            "  blr\n"
            ".size FocusFunction,.-FocusFunction\n",
            encoding="ascii",
        )
        self.target = self.root / "target.o"
        self.candidate = self.root / "candidate.o"
        subprocess.run([self.assembler, "-mgekko", "-o", self.target, source], check=True)
        shutil.copyfile(self.target, self.candidate)

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_real_objects_reports_and_physical_receipt_form_closed_focus_artifact(self) -> None:
        strict = self.root / "strict.json"
        data = self.root / "data.json"
        physical = self.root / "physical.json"
        bundle._run_objdiff(OBJDFF, self.target, self.candidate, strict, data=False, root=self.root)
        bundle._run_objdiff(OBJDFF, self.target, self.candidate, data, data=True, root=self.root)
        receipt = bundle._physical_receipt(
            self.target, self.candidate, FUNCTION, strict, Path(self.readelf)
        )
        bundle._atomic_json(physical, receipt)
        artifact = focus_symbol_report.build_from_paths(
            strict_report_path=strict,
            data_report_path=data,
            function=FUNCTION,
            expected_strict_report_sha256=digest(strict),
            expected_data_report_sha256=digest(data),
            physical_receipt_path=physical,
            expected_physical_receipt_sha256=digest(physical),
            require_physical=True,
        )
        self.assertEqual(artifact["function"], FUNCTION)
        self.assertEqual(artifact["physical_relocations"]["status"], "exact")
        self.assertEqual(
            artifact["physical_relocations"]["target"]["physical_relocation_count"], 1
        )

    def test_real_nonexact_alignment_placeholders_do_not_count_as_instructions(self) -> None:
        source = self.root / "candidate-extra.s"
        source.write_text(
            ".section .text\n"
            ".globl FocusFunction\n"
            ".type FocusFunction,@function\n"
            "FocusFunction:\n"
            "  bl External\n"
            "  nop\n"
            "  blr\n"
            ".size FocusFunction,.-FocusFunction\n",
            encoding="ascii",
        )
        longer = self.root / "candidate-extra.o"
        subprocess.run([self.assembler, "-mgekko", "-o", longer, source], check=True)
        strict = self.root / "nonexact-strict.json"
        data = self.root / "nonexact-data.json"
        physical = self.root / "nonexact-physical.json"
        bundle._run_objdiff(OBJDFF, self.target, longer, strict, data=False, root=self.root)
        bundle._run_objdiff(OBJDFF, self.target, longer, data, data=True, root=self.root)
        report = json.loads(strict.read_text(encoding="utf-8"))
        left_focus = next(row for row in report["left"]["symbols"] if row.get("name") == FUNCTION)
        right_focus = next(row for row in report["right"]["symbols"] if row.get("name") == FUNCTION)
        raw_counts = (len(left_focus["instructions"]), len(right_focus["instructions"]))
        actual_counts = tuple(
            sum(isinstance(row.get("instruction"), dict) for row in focus["instructions"])
            for focus in (left_focus, right_focus)
        )
        self.assertEqual(actual_counts, (2, 3))
        self.assertNotEqual(raw_counts, actual_counts)
        receipt = bundle._physical_receipt(
            self.target, longer, FUNCTION, strict, Path(self.readelf)
        )
        bundle._atomic_json(physical, receipt)
        artifact = focus_symbol_report.build_from_paths(
            strict_report_path=strict,
            data_report_path=data,
            function=FUNCTION,
            expected_strict_report_sha256=digest(strict),
            expected_data_report_sha256=digest(data),
            physical_receipt_path=physical,
            expected_physical_receipt_sha256=digest(physical),
            require_physical=True,
        )
        self.assertEqual(artifact["channels"]["strict"]["target"]["instruction_count"], 2)
        self.assertEqual(artifact["channels"]["strict"]["candidate"]["instruction_count"], 3)

    def test_fabricated_json_cannot_replace_real_objdiff_report(self) -> None:
        strict = self.root / "strict.json"
        data = self.root / "data.json"
        bundle._run_objdiff(OBJDFF, self.target, self.candidate, data, data=True, root=self.root)
        strict.write_text(json.dumps({"left": {}, "right": {}, "claimed": "100%"}), encoding="utf-8")
        with self.assertRaises(focus_symbol_report.FocusReportError):
            focus_symbol_report.build_from_paths(
                strict_report_path=strict,
                data_report_path=data,
                function=FUNCTION,
                expected_strict_report_sha256=digest(strict),
                expected_data_report_sha256=digest(data),
            )

    def test_elf_parser_rejects_wrong_function_and_tracks_real_relocation(self) -> None:
        evidence = bundle._parse_elf_relocations(self.target, FUNCTION)
        self.assertEqual(evidence["size"], 8)
        self.assertEqual(evidence["instruction_count"], 2)
        self.assertEqual(evidence["physical_relocation_count"], 1)
        self.assertEqual(evidence["physical_relocations"][0]["effective_target"]["name"], "External")
        with self.assertRaisesRegex(bundle.EvidenceError, "exactly one function"):
            bundle._parse_elf_relocations(self.target, "WrongFunction")

    def test_objdiff_pin_rejects_fake_executable(self) -> None:
        fake = self.root / "objdiff-cli.exe"
        fake.write_bytes(b"not objdiff")
        with self.assertRaisesRegex(bundle.EvidenceError, "SHA-256 drifted"):
            bundle._verify_objdiff(fake)

    def test_ninja_pin_rejects_fake_executable(self) -> None:
        fake = self.root / "ninja.exe"
        fake.write_bytes(b"not ninja")
        with self.assertRaisesRegex(bundle.EvidenceError, "SHA-256 drifted"):
            bundle._verify_ninja(fake)

    def test_configure_failure_unconditionally_removes_staged_retail(self) -> None:
        tracked_orig = self.root / "orig" / "GP6E01"
        tracked_orig.mkdir(parents=True)
        (tracked_orig / ".gitkeep").write_bytes(b"")
        central_orig = self.root / "central-retail"
        (central_orig / "files").mkdir(parents=True)
        (central_orig / "sys").mkdir()
        (central_orig / "files" / "payload.bin").write_bytes(b"retail")
        (central_orig / "sys" / "main.dol").write_bytes(b"dol")
        toolchain = {
            "orig": {"path_object": central_orig},
            "binutils": {"path_object": self.root / "binutils"},
            "compilers": {"path_object": self.root / "compilers"},
            "dtk": {"path_object": self.root / "dtk.exe"},
            "sjiswrap": {"path_object": self.root / "sjiswrap.exe"},
        }
        with patch.object(bundle, "_run", side_effect=bundle.EvidenceError("configure failed")):
            with self.assertRaisesRegex(bundle.EvidenceError, "configure failed"):
                bundle._ensure_configured(self.root, toolchain, bundle.DEFAULT_NINJA)
        self.assertEqual(list(tracked_orig.iterdir()), [tracked_orig / ".gitkeep"])

    def test_staged_retail_survives_real_configure_and_selected_ninja_build(self) -> None:
        tracked_orig = self.root / "orig" / "GP6E01"
        tracked_orig.mkdir(parents=True)
        (tracked_orig / ".gitkeep").write_bytes(b"")
        central_orig = self.root / "central-retail"
        required_retail = central_orig / "files" / "dll" / "m699Dll.rel"
        required_retail.parent.mkdir(parents=True)
        required_retail.write_bytes(b"retail dependency")
        source = self.root / "src" / "focus.s"
        source.parent.mkdir()
        shutil.copyfile(self.root / "focus.s", source)
        assembler = Path(self.assembler).as_posix()
        helper = self.root / "build_helper.py"
        helper.write_text(
            "import pathlib,subprocess,sys\n"
            "required=pathlib.Path('orig/GP6E01/files/dll/m699Dll.rel')\n"
            "assert required.is_file(), 'staged retail was removed before Ninja'\n"
            "out=pathlib.Path(sys.argv[1]); out.parent.mkdir(parents=True,exist_ok=True)\n"
            f"subprocess.run([{assembler!r},'-mgekko','-o',str(out),'src/focus.s'],check=True) "
            "if out.suffix == '.o' else out.write_text('configured')\n",
            encoding="utf-8",
        )
        ninja_text = (
            "rule fixture\n"
            f"  command = \"{Path(sys.executable).as_posix()}\" build_helper.py $out\n"
            "build build/GP6E01/config.json: fixture orig/GP6E01/files/dll/m699Dll.rel\n"
            "build build/candidate.o: fixture src/focus.s | orig/GP6E01/files/dll/m699Dll.rel\n"
        )
        configure = self.root / "configure.py"
        configure.write_text(
            "import json,pathlib\n"
            f"pathlib.Path('build.ninja').write_text({ninja_text!r},encoding='utf-8')\n"
            "pathlib.Path('objdiff.json').write_text(json.dumps({'units':[]}),encoding='utf-8')\n",
            encoding="utf-8",
        )
        toolchain = {
            "orig": {"path_object": central_orig},
            "binutils": {"path_object": self.root / "binutils"},
            "compilers": {"path_object": self.root / "compilers"},
            "dtk": {"path_object": self.root / "dtk.exe"},
            "sjiswrap": {"path_object": self.root / "sjiswrap.exe"},
        }
        retail_copy = bundle._ensure_configured(self.root, toolchain, bundle.DEFAULT_NINJA)
        try:
            self.assertTrue((retail_copy / "files" / "dll" / "m699Dll.rel").is_file())
            bundle._run(
                [str(bundle.DEFAULT_NINJA), "-j1", "build/candidate.o"],
                cwd=self.root,
                label="fixture selected object build",
            )
            self.assertTrue((self.root / "build" / "candidate.o").is_file())
        finally:
            bundle._remove_staged_retail(retail_copy)
        self.assertEqual(list(tracked_orig.iterdir()), [tracked_orig / ".gitkeep"])

    def test_public_phase_finally_removes_retail_after_post_config_failure(self) -> None:
        retail_copy = self.root / "orig" / "GP6E01"
        (retail_copy / "files").mkdir(parents=True)
        (retail_copy / ".gitkeep").write_bytes(b"")
        (retail_copy / "files" / "payload.bin").write_bytes(b"retail")

        def fail_after_staging(**kwargs: object) -> dict[str, object]:
            staged = kwargs["staged_retail"]
            assert isinstance(staged, list)
            staged.append(retail_copy)
            raise bundle.EvidenceError("selected build failed")

        with patch.object(bundle, "_run_phase_impl", side_effect=fail_after_staging):
            with self.assertRaisesRegex(bundle.EvidenceError, "selected build failed"):
                bundle.run_phase(
                    root=self.root, context_path=self.root / "context.json",
                    out_root=self.root / "out", objdiff=OBJDFF,
                    readelf=Path(self.readelf),
                )
        self.assertEqual(list(retail_copy.iterdir()), [retail_copy / ".gitkeep"])

    def test_two_phase_adapter_uses_real_build_objects_and_seals_baseline(self) -> None:
        subprocess.run(["git", "init", "-q", self.root], check=True)
        source = self.root / "src" / "focus.s"
        source.parent.mkdir()
        source.write_text((self.root / "focus.s").read_text(encoding="ascii"), encoding="ascii")
        target = self.root / "build" / "target.o"
        target.parent.mkdir()
        shutil.copyfile(self.target, target)
        candidate = self.root / "build" / "candidate.o"
        assembler = Path(self.assembler).as_posix()
        (self.root / "build.ninja").write_text(
            f"rule as\n  command = {assembler} -mgekko -o $out $in\n"
            "build build/candidate.o: as src/focus.s\n",
            encoding="utf-8",
        )
        (self.root / "objdiff.json").write_text(
            json.dumps({
                "units": [{
                    "name": "main/focus", "target_path": "build/target.o",
                    "base_path": "build/candidate.o",
                }]
            }),
            encoding="utf-8",
        )
        out = self.root / "out"
        out.mkdir()
        context_body = {
            "schema": "crack_evidence_context/v1", "owner": "main:test/focus",
            "function": FUNCTION, "unit": "main/focus", "source_relpath": "src/focus.s",
            "target_sha256": digest(target), "base_source_sha256": digest(source),
            "candidate_source_sha256": digest(source), "base_commit": "1" * 40,
            "approval_sha256": "2" * 64,
            "toolchain_key": "b6764a1e5883ea1a096bfe4f8b888b93f1740f0f4046eb6149e0fe1d64cc6d90",
            "issued_at": "2026-08-29T00:00:00+00:00",
            "objdiff": {
                "path": str(OBJDFF), "sha256": bundle.OBJDFF_SHA256,
                "version": bundle.OBJDFF_VERSION, "size_bytes": OBJDFF.stat().st_size,
            },
        }
        context = {**context_body, "context_sha256": bundle._json_sha(context_body)}
        context_path = out / "approval-context.json"
        bundle._atomic_json(context_path, context)
        common = {
            "CRACK_HARNESS_OUT_ROOT": str(out), "CRACK_HARNESS_OWNER": "main:test/focus",
            "CRACK_HARNESS_FUNCTION": FUNCTION, "CRACK_HARNESS_UNIT": "main/focus",
            "CRACK_HARNESS_SOURCE_PATH": "src/focus.s",
            "CRACK_HARNESS_TARGET_SHA256": digest(target),
            "CRACK_HARNESS_BASE_COMMIT": "1" * 40,
            "CRACK_HARNESS_APPROVAL_SHA256": "2" * 64,
            "CRACK_HARNESS_CONTEXT_SHA256": context["context_sha256"],
            "CRACK_HARNESS_ISSUED_AT": context_body["issued_at"],
        }
        readelf = Path(r"C:\Users\Anony\.codex\tools\mp6\binutils-2.42-1\powerpc-eabi-readelf.exe")
        with patch.dict(
            os.environ,
            {**common, "CRACK_HARNESS_PHASE": "baseline", "CRACK_HARNESS_PHASE_NONCE": hashlib.sha256((context["context_sha256"] + ":baseline").encode()).hexdigest()},
            clear=False,
        ):
            baseline = bundle.run_phase(
                root=self.root, context_path=context_path, out_root=out,
                objdiff=OBJDFF, readelf=readelf,
            )
        sealed = {name: digest(out / name) for name in ("target.o", "baseline-strict.json", "baseline-data.json")}
        with patch.dict(
            os.environ,
            {**common, "CRACK_HARNESS_PHASE": "candidate", "CRACK_HARNESS_PHASE_NONCE": hashlib.sha256((context["context_sha256"] + ":candidate").encode()).hexdigest()},
            clear=False,
        ):
            candidate_receipt = bundle.run_phase(
                root=self.root, context_path=context_path, out_root=out,
                objdiff=OBJDFF, readelf=readelf,
            )
        self.assertEqual(baseline["phase"], "baseline")
        self.assertEqual(candidate_receipt["phase"], "candidate")
        self.assertEqual(sealed, {name: digest(out / name) for name in sealed})
        self.assertTrue((out / "physical.json").is_file())


if __name__ == "__main__":
    unittest.main()
