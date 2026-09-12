from __future__ import annotations

import contextlib
import io
import hashlib
import os
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_learning_rules as rules
from tools import source_linked_owner_closure as closure
from tools.tests import test_crack_learning_rules as fixtures


def _seal_manifest(context: dict[str, object]) -> None:
    manifest = context["link_manifest"]
    assert isinstance(manifest, dict)
    manifest.pop("manifest_canonical_sha256", None)
    manifest["manifest_canonical_sha256"] = closure.canonical_sha256(manifest)
    retail = context["retail_output"]
    assert isinstance(retail, dict)
    retail["link_manifest_canonical_sha256"] = manifest["manifest_canonical_sha256"]


def _context(*, objdiff_sha256: str = "9" * 64) -> dict[str, object]:
    candidate_object = "2" * 64
    target_object = "3" * 64
    main_binary = "4" * 64
    context: dict[str, object] = {
        "schema": closure.CONTEXT_SCHEMA,
        "report_artifact_sha256": "f" * 64,
        "owner": "main:board/capevent",
        "focus_function": "mbev_CapCoinDisp",
        "objdiff_canonical_sha256": objdiff_sha256,
        "configure": {
            "source_path": "src/board/capevent.c",
            "configured_status": "Matching",
            "configure_sha256": "a" * 64,
            "status_receipt_sha256": "b" * 64,
        },
        "candidate": {
            "source_sha256": "1" * 64,
            "object_path": "build/GP6E01/src/board/capevent.o",
            "object_sha256": candidate_object,
            "strict_report_sha256": "5" * 64,
            "data_report_sha256": "6" * 64,
            "compile_attestation_sha256": "7" * 64,
            "candidate_record_sha256": "8" * 64,
            "functions_exact": 237,
            "functions_total": 237,
            "strict_diff_rows": 0,
            "data_diff_rows": 0,
            "physical_relocations_exact": 31,
            "physical_relocations_total": 31,
            "protected_sibling_losses": 0,
            "owner_sections_exact": True,
        },
        "target": {"object_sha256": target_object},
        "link_manifest": {
            "schema": closure.LINK_MANIFEST_SCHEMA,
            "manifest_file_sha256": "c" * 64,
            "owner": "main:board/capevent",
            "source_path": "src/board/capevent.c",
            "configured_status": "Matching",
            "selected_object_path": "build/GP6E01/src/board/capevent.o",
            "selected_object_sha256": candidate_object,
            "object_origin": "reconstructed_source",
            "clean_build": True,
            "build_receipt_sha256": "d" * 64,
        },
        "retail_output": {
            "link_manifest_canonical_sha256": "0" * 64,
            "configured_files_exact": 137,
            "configured_files_total": 137,
            "checksum_receipt_sha256": "e" * 64,
            "main_binary_sha256": main_binary,
            "retail_main_binary_sha256": main_binary,
            "main_binary_byte_identical": True,
        },
        "addressable_owner": {
            "target": {
                "symbol": "lbl_802C4890",
                "section": ".sdata2",
                "size_bytes": 4,
                "alignment": 4,
                "value_bits": "437a0000",
                "read_only": True,
                "symbol_extent_sealed": True,
                "creation_order": 91,
                "section_receipt_sha256": "1" * 64,
                "chronology_receipt_sha256": "2" * 64,
            },
            "source": {
                "name": "capCoinDispPosYOffset",
                "declaration_class": "one_element_read_only_float_array",
                "element_count": 1,
                "size_bytes": 4,
                "initializer_bits": "437a0000",
                "section": ".sdata2",
                "read_only": True,
                "use_count": 1,
                "semantic_consumer": "pos.y += capCoinDispPosYOffset[0]",
                "creation_order": 91,
                "source_order_receipt_sha256": "3" * 64,
            },
            "relocation": {
                "type": "R_PPC_EMB_SDA21",
                "count": 1,
                "consumer_function": "mbev_CapCoinDisp",
                "target_owner": "lbl_802C4890",
                "candidate_owner": "capCoinDispPosYOffset",
                "physical_identity": True,
                "receipt_sha256": "4" * 64,
            },
            "controls": {
                "direct_scalar_literal_rejected": True,
                "automatic_or_volatile_rejected": True,
                "synthetic_target_label_absent": True,
                "padding_absent": True,
                "register_shaping_absent": True,
                "control_receipt_sha256": "5" * 64,
            },
        },
        "telemetry": {
            "telemetry_complete": False,
            "excluded_from_measured_crack_per_hour": True,
            "no_imputation": True,
            "interval_log_sha256": "6" * 64,
        },
        "authority_advanced": False,
    }
    _seal_manifest(context)
    return context


def _fallback_context() -> dict[str, object]:
    context = _context()
    configure = context["configure"]
    manifest = context["link_manifest"]
    target = context["target"]
    assert isinstance(configure, dict)
    assert isinstance(manifest, dict)
    assert isinstance(target, dict)
    configure["configured_status"] = "NonMatching"
    manifest["configured_status"] = "NonMatching"
    manifest["selected_object_path"] = "orig/GP6E01/board/capevent.o"
    manifest["selected_object_sha256"] = target["object_sha256"]
    manifest["object_origin"] = "extracted_target_fallback"
    _seal_manifest(context)
    return context


class SourceLinkedOwnerClosureTests(unittest.TestCase):
    def test_matching_source_link_closure_and_addressable_owner(self) -> None:
        result = closure.evaluate(_context())

        self.assertTrue(result["matched"])
        self.assertTrue(result["closure_ready"])
        self.assertEqual(result["status"], "SOURCE_LINK_CLOSURE_VERIFIED")
        self.assertEqual(result["configured_status"], "Matching")
        self.assertEqual(
            result["addressable_owner_diagnosis"]["source_class"],
            "minimum_live_addressable_read_only_f32_owner",
        )
        self.assertFalse(result["authority_advanced"])
        self.assertTrue(closure.verify_self_hash(result))

    def test_nonmatching_checksum_is_blocked_as_fallback_linked(self) -> None:
        result = closure.evaluate(_fallback_context())

        self.assertTrue(result["matched"])
        self.assertFalse(result["closure_ready"])
        self.assertTrue(result["retail_checksum_exact"])
        self.assertEqual(result["status"], "BLOCKED_FALLBACK_LINKED")
        self.assertIn("NonMatching", " ".join(result["blocked_reasons"]))
        self.assertEqual(result["linked_object"]["origin"], "extracted_target_fallback")

    def test_matching_requires_candidate_object_hash_in_manifest(self) -> None:
        context = _context()
        manifest = context["link_manifest"]
        assert isinstance(manifest, dict)
        manifest["selected_object_sha256"] = "0" * 64
        _seal_manifest(context)

        result = closure.evaluate(context)

        self.assertFalse(result["closure_ready"])
        self.assertEqual(result["status"], "BLOCKED_SOURCE_LINK_PROVENANCE")
        self.assertIn("linked object hash", " ".join(result["blocked_reasons"]))

    def test_matching_requires_candidate_object_path_in_manifest(self) -> None:
        context = _context()
        manifest = context["link_manifest"]
        assert isinstance(manifest, dict)
        manifest["selected_object_path"] = "build/GP6E01/obj/board/capevent.o"
        _seal_manifest(context)

        result = closure.evaluate(context)

        self.assertFalse(result["closure_ready"])
        self.assertIn("linked object path", " ".join(result["blocked_reasons"]))

    def test_retail_output_must_bind_same_manifest(self) -> None:
        context = _context()
        retail = context["retail_output"]
        assert isinstance(retail, dict)
        retail["link_manifest_canonical_sha256"] = "0" * 64

        result = closure.evaluate(context)

        self.assertFalse(result["closure_ready"])
        self.assertIn("not bound", " ".join(result["blocked_reasons"]))

    def test_owner_proof_must_be_complete(self) -> None:
        context = _context()
        candidate = context["candidate"]
        assert isinstance(candidate, dict)
        candidate["functions_exact"] = 236
        candidate["physical_relocations_exact"] = 30
        candidate["owner_sections_exact"] = False

        result = closure.evaluate(context)

        self.assertFalse(result["closure_ready"])
        self.assertGreaterEqual(len(result["blocked_reasons"]), 3)

    def test_addressable_owner_requires_exact_four_byte_extent(self) -> None:
        context = _context()
        owner = context["addressable_owner"]
        assert isinstance(owner, dict)
        target = owner["target"]
        assert isinstance(target, dict)
        target["size_bytes"] = 8

        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "four-byte extent"):
            closure.parse_context(context)

    def test_addressable_owner_rejects_raw_target_source_name(self) -> None:
        context = _context()
        owner = context["addressable_owner"]
        assert isinstance(owner, dict)
        source = owner["source"]
        relocation = owner["relocation"]
        assert isinstance(source, dict)
        assert isinstance(relocation, dict)
        source["name"] = "lbl_802C4890"
        source["semantic_consumer"] = "pos.y += lbl_802C4890[0]"
        relocation["candidate_owner"] = "lbl_802C4890"

        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "semantic"):
            closure.parse_context(context)

    def test_addressable_owner_requires_one_exact_sda21_consumer(self) -> None:
        context = _context()
        owner = context["addressable_owner"]
        assert isinstance(owner, dict)
        relocation = owner["relocation"]
        assert isinstance(relocation, dict)
        relocation["count"] = 2

        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "one exact SDA21"):
            closure.parse_context(context)

    def test_incomplete_telemetry_requires_exclusion_without_imputation(self) -> None:
        context = _context()
        telemetry = context["telemetry"]
        assert isinstance(telemetry, dict)
        telemetry["excluded_from_measured_crack_per_hour"] = False

        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "excluded"):
            closure.parse_context(context)

    def test_focus_binding_fails_closed(self) -> None:
        result = closure.evaluate(
            _context(),
            focus_symbol="DifferentFunction",
            objdiff_canonical_sha256="9" * 64,
        )

        self.assertFalse(result["matched"])
        self.assertFalse(result["closure_ready"])
        self.assertEqual(result["status"], "CONTEXT_NOT_BOUND_TO_FOCUS")
        self.assertTrue(closure.verify_self_hash(result))

    def test_deterministic_under_mapping_order(self) -> None:
        context = _context()
        reordered = json.loads(json.dumps(context, sort_keys=True))

        self.assertEqual(closure.evaluate(context), closure.evaluate(reordered))

    def test_cli_require_closure_is_an_actual_gate(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            matching_path = root / "matching.json"
            blocked_path = root / "blocked.json"
            output_path = root / "result.json"
            blocked_output_path = root / "blocked-result.json"
            matching_path.write_text(json.dumps(_context()), encoding="utf-8")
            blocked_path.write_text(json.dumps(_fallback_context()), encoding="utf-8")

            self.assertEqual(
                closure.main(
                    [
                        "--context",
                        str(matching_path),
                        "--output",
                        str(output_path),
                        "--require-closure",
                    ]
                ),
                0,
            )
            self.assertTrue(closure.verify_self_hash(json.loads(output_path.read_text())))
            self.assertEqual(
                closure.main(
                    [
                        "--context",
                        str(blocked_path),
                        "--output",
                        str(blocked_output_path),
                        "--require-closure",
                    ]
                ),
                2,
            )
            self.assertFalse(json.loads(blocked_output_path.read_text())["closure_ready"])

    def test_dispatcher_cli_emits_same_closed_document(self) -> None:
        report = fixtures._report(
            "mbev_CapCoinDisp",
            [fixtures._instruction(0x100, "blr")],
            [fixtures._instruction(0x100, "blr")],
            target_size=4,
            candidate_size=4,
        )
        objdiff_sha = rules._sha256(rules._canonical(report))
        context = _context(objdiff_sha256=objdiff_sha)
        expected = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapCoinDisp",
            source_linked_owner_closure_context=context,
        )
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            report_path = root / "report.json"
            context_path = root / "context.json"
            report_path.write_text(json.dumps(report), encoding="utf-8")
            context_path.write_text(json.dumps(context), encoding="utf-8")
            with contextlib.redirect_stdout(io.StringIO()) as output:
                return_code = rules.main(
                    [
                        "--report",
                        str(report_path),
                        "--function",
                        "mbev_CapCoinDisp",
                        "--source-linked-owner-closure-context",
                        str(context_path),
                    ]
                )
            self.assertEqual(return_code, 0)
            self.assertEqual(json.loads(output.getvalue()), expected)

    def test_dispatcher_surfaces_verified_and_blocked_states(self) -> None:
        report = fixtures._report(
            "mbev_CapCoinDisp",
            [fixtures._instruction(0x100, "blr")],
            [fixtures._instruction(0x100, "blr")],
            target_size=4,
            candidate_size=4,
        )
        objdiff_sha = rules._sha256(rules._canonical(report))
        context = _context(objdiff_sha256=objdiff_sha)

        verified = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapCoinDisp",
            source_linked_owner_closure_context=context,
        )
        evaluation = fixtures._evaluation(verified, closure.RULE_ID)
        self.assertTrue(evaluation["matched"])
        self.assertTrue(evaluation["evidence"]["closure"]["closure_ready"])

        blocked_context = _fallback_context()
        blocked_context["objdiff_canonical_sha256"] = objdiff_sha
        blocked = rules.diagnose_document(
            report,
            focus_symbol="mbev_CapCoinDisp",
            source_linked_owner_closure_context=blocked_context,
        )
        blocked_evaluation = fixtures._evaluation(blocked, closure.RULE_ID)
        self.assertTrue(blocked_evaluation["matched"])
        self.assertFalse(blocked_evaluation["evidence"]["closure"]["closure_ready"])


class WorkspaceObservationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.source = "src/board/single.c"
        self.obj = "build/GP6E01/src/board/single.o"
        self.graph = (f"build {self.obj}: mwcc_sjis {self.source}\n"
                      "build build/GP6E01/other.elf: link build/GP6E01/board/single.o\n"
                      "build build/GP6E01/main.elf: link $\n"
                      f"    {self.obj}\n"
                      "build build/GP6E01/main.dol: elf2dol build/GP6E01/main.elf\n")
        for name, data in ((self.source, b"source"), (self.obj, b"object"),
                           ("build/GP6E01/main.elf", b"elf"),
                           ("build/GP6E01/main.dol", b"retail"),
                           ("retail/sys/main.dol", b"retail")):
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        (self.root / "build.ninja").write_text(self.graph, encoding="utf-8")
        self.manifest = hashlib.sha1(b"retail").hexdigest() + "  build/GP6E01/main.dol\n"
        (self.root / "build.sha1").write_text(self.manifest, encoding="utf-8")

    def verify(self, **kwargs):
        return closure.verify_workspace(root=self.root, ninja="build.ninja", manifest="build.sha1",
            retail_root=self.root / "retail", link_output="build/GP6E01/main.elf",
            selected=[(self.source, self.obj)], **kwargs)

    def test_multiline_fallback_elsewhere_and_hash_binding(self):
        result = self.verify(expected_hashes=[(self.source, hashlib.sha256(b"source").hexdigest())])
        self.assertEqual(result["containers_exact"], 1)
        self.assertEqual(result["compile_provenance"], "UNKNOWN")
        self.assertFalse(result["source_recovery_proven"])
        self.assertTrue(closure.verify_self_hash(result))

    def test_selected_fallback_rejected(self):
        (self.root / "build.ninja").write_text(self.graph.replace(
            f"    {self.obj}", "    build/GP6E01/board/single.o"), encoding="utf-8")
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "select exactly"):
            self.verify()

    def test_extra_owner_object_rejected(self):
        (self.root / "build.ninja").write_text(self.graph.replace(
            f"    {self.obj}", f"    {self.obj} build/GP6E01/board/single.o"), encoding="utf-8")
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "ambiguous"):
            self.verify()

    def test_source_cannot_be_target_fallback(self):
        self.source = "build/GP6E01/board/single.o"
        (self.root / self.source).parent.mkdir(parents=True, exist_ok=True)
        (self.root / self.source).write_bytes(b"fallback")
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "canonical"):
            self.verify()

    def test_source_and_object_hash_drift(self):
        for name in (self.source, self.obj):
            with self.subTest(name=name), self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "drift"):
                self.verify(expected_hashes=[(name, "0" * 64)])

    def test_missing_output(self):
        (self.root / "build/GP6E01/main.dol").unlink()
        with self.assertRaises(OSError):
            self.verify()

    def test_one_differing_byte(self):
        (self.root / "build/GP6E01/main.dol").write_bytes(b"retaiX")
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "mismatch"):
            self.verify()

    def test_main_dol_must_convert_selected_link_output(self):
        (self.root / "build.ninja").write_text(self.graph.replace(
            "elf2dol build/GP6E01/main.elf", "elf2dol build/GP6E01/other.elf"), encoding="utf-8")
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "does not convert selected"):
            self.verify()

    def test_built_retail_hardlink_rejected(self):
        built = self.root / "build/GP6E01/main.dol"
        built.unlink()
        os.link(self.root / "retail/sys/main.dol", built)
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "aliased/duplicate"):
            self.verify()

    def test_output_alias_and_outside_build(self):
        for output in (self.obj, "build/GP6E01/main.dol", "src/result.json"):
            with self.subTest(output=output), self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "result output"):
                self.verify(output=output)
        alias = self.root / "build/alias.json"
        os.link(self.root / self.obj, alias)
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "result output"):
            self.verify(output="build/alias.json")
        self.assertEqual((self.root / self.obj).read_bytes(), b"object")

    def test_manifest_tampering_duplicate_and_omission(self):
        for text in (self.manifest.replace(self.manifest[:40], "0" * 40), self.manifest * 2, ""):
            (self.root / "build.sha1").write_text(text, encoding="utf-8")
            with self.assertRaises(closure.SourceLinkedClosureInputError):
                self.verify()

    def test_unmanifested_graph_container(self):
        (self.root / "build.ninja").write_text(self.graph +
            "build build/GP6E01/foo/foo.rel: makerel foo.plf\n", encoding="utf-8")
        with self.assertRaisesRegex(closure.SourceLinkedClosureInputError, "sets differ"):
            self.verify()

    def test_ninja_escaping_variables_and_ambiguity(self):
        edges = closure._ninja_edges("build build/a$ b.o: cc src/a$ b.c\n"
                                    "build a$$b: link build/a$ b.o\n")
        self.assertEqual(edges["a$b"][1], ["build/a b.o"])
        for text in ("build $out: link a\n", "build out: link ${in}\n",
                     "include more.ninja\n", "build out: link a\nbuild out: link b\n"):
            with self.assertRaises(closure.SourceLinkedClosureInputError):
                closure._ninja_edges(text)

    def test_atomic_result_and_cli(self):
        result = self.verify(output="build/observation.json")
        self.assertEqual(json.loads((self.root / "build/observation.json").read_text()), result)
        with contextlib.redirect_stdout(io.StringIO()):
            code = closure.main(["verify-workspace", "--root", str(self.root),
                "--ninja", "build.ninja", "--manifest", "build.sha1", "--retail-root",
                str(self.root / "retail"), "--link-output", "build/GP6E01/main.elf",
                "--selected", self.source, self.obj])
        self.assertEqual(code, 0)


if __name__ == "__main__":
    unittest.main()
