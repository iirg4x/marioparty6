import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import recovery_call_contract_repair as repair


class EnvelopeAdapterTests(unittest.TestCase):
    def fixture(self):
        common = dict(session_id='session-test', process_id=1, function='f', status='CAPTURED')
        return {'context': dict(common, compiler={'sha256': repair.COMPILER_SHA256}),
                'events': [dict(common, event_kind='return_temp_allocation', expression_token='expr',
                                allocated_vreg=32, counter_after=33),
                           dict(common, event_kind='source_pcode_origin', expression_token='expr',
                                direct_callee_name='api', expression_kind=54, child_edge='CAPTURED_ACTIVE_HANDLER')]}

    def test_actual_allocation_not_exact_type(self):
        calls, differences = repair.inspect_calls(self.fixture(), 'f', {'api'})
        self.assertEqual(len(differences), 1)
        self.assertIsNone(calls[0]['compiler_seen_type']['exact_type'])

    def test_unrelated_token_and_snapshot_are_not_allocations(self):
        d = self.fixture()
        d['events'][1]['expression_token'] = 'other'
        self.assertEqual(repair.inspect_calls(d, 'f', {'api'}), ([], []))
        d = self.fixture()
        d['events'][0]['event_kind'] = 'source_pcode_origin'
        self.assertEqual(repair.inspect_calls(d, 'f', {'api'}), ([], []))

    def test_ambiguous_name_and_wrong_function_refused(self):
        d = self.fixture()
        d['events'].append(dict(d['events'][1], direct_callee_name='other'))
        with self.assertRaises(ValueError):
            repair.inspect_calls(d, 'f', {'api'})
        with self.assertRaises(ValueError):
            repair.inspect_calls(self.fixture(), 'other', {'api'})

    def test_stale_object_or_source_binding_refused(self):
        with mock.patch.object(repair.expression_join, 'bind', return_value={'comparison': {'status': 'mismatch'}}):
            with self.assertRaises(ValueError):
                repair.bind_envelope(self.fixture(), 'capture', 'object', 'source', 'f')
        with mock.patch.object(repair.expression_join, 'bind', side_effect=ValueError('stale source')):
            with self.assertRaises(ValueError):
                repair.bind_envelope(self.fixture(), 'capture', 'object', 'source', 'f')


def event(name, sequence, kind=1, width=4):
    tree = {"native_kind": 54, "callee": {"object": {"name": name}},
            "type": {"status": "CAPTURED", "native_kind": kind, "byte_width": width, "native_basic_code": 7}}
    return {"function": "f", "sequence": sequence, "counter_before": 32 + sequence,
            "counter_after": 33 + sequence,
            "actual_call_origin": {"binding": "native_return_allocation_frame", "status": "CAPTURED",
                                   "normalized_expression_tree": {"tree": tree}}}


class ContractRepairTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / "build").mkdir()
        self.raw = b"static int f(void)\n{\n    A(1);\n    { extern void A(int x); A(1); }\n    return 0;\n}\n"
        self.source = self.root / "source.c"
        self.source.write_bytes(self.raw)
        self.index = self.root / "index.json"
        self.desc = repair._read(self.root, self.source)[1]
        self.index.write_text(json.dumps({"inputs": {"source": self.desc}, "functions": [{"function": "f"}]}))
        self.capture = self.root / "capture.json"
        a, b, _ = repair._function_span(self.raw, "f")
        self.doc = {"status": "CAPTURED", "function": "f", "source": self.desc,
                    "compiler": {"sha256": repair.COMPILER_SHA256},
                    "function_sha256": repair._sha(self.raw[a:b]),
                    "events": [event("A", 1), event("A", 2, 0, 0), event("B", 3, 0, 0), event("TrueInt", 4)]}
        self.contracts = []
        for name in ("A", "B"):
            path = self.root / (name + ".h")
            raw = ("void " + name + "(int x);").encode()
            path.write_bytes(raw)
            self.contracts.append({"declaration": "extern void " + name + "(int x);",
                                   "evidence": {"path": path.name, "sha256": repair._sha(raw),
                                                "start_byte": 0, "end_byte": len(raw)}})

    def generate(self, **extra):
        self.capture.write_text(json.dumps(self.doc))
        args = dict(root=self.root, index=self.index, capture=self.capture,
                    capture_sha256=repair.compiler.digest(self.capture), function="f",
                    contracts=self.contracts, out_dir=self.root / "build/result", reviewed=True)
        args.update(extra)
        with mock.patch.object(repair.frontier, "verify"):
            return repair.generate(**args)

    def test_partial_scope_composes_ordered_set_and_preserves_bytes(self):
        result = self.generate()
        expected = self.raw.replace(b"{\n", b"{\n    extern void A(int x);\n    extern void B(int x);\n", 1)
        self.assertEqual(Path(result["candidate"]).read_bytes(), expected)
        self.assertEqual(result["discrepant_calls"], 1)
        self.assertTrue(result["runnable_manifest"])
        selection = json.loads((self.root / "build/result/selection.json").read_text())
        self.assertEqual([c["status"] for c in selection["calls"]], ["discrepant", "agrees", "agrees"])
        self.assertEqual(selection["unreviewed_call_count"], 1)
        self.assertEqual(self.source.read_bytes(), self.raw)
        again = self.generate(out_dir=self.root / "build/repeat")
        self.assertEqual(result["candidate_sha256"], again["candidate_sha256"])

    def test_enclosing_context_cannot_supply_actual_type(self):
        self.doc["events"][0]["normalized_expression_tree"] = {"tree": {"type": {"native_kind": 0, "byte_width": 0}}}
        result = self.generate()
        self.assertEqual(result["discrepant_calls"], 1)
        self.doc["events"][0]["actual_call_origin"] = {}
        with self.assertRaisesRegex(ValueError, "no captured"):
            self.generate(out_dir=self.root / "build/no-actual")

    def test_unknown_and_true_integer_abi_not_inferred_void(self):
        self.doc["events"] = [event("TrueInt", 1), event("Unknown", 2)]
        with self.assertRaisesRegex(ValueError, "no captured"):
            self.generate()
        self.contracts[0]["declaration"] = "extern int A(int x);"
        with self.assertRaisesRegex(ValueError, "void signature"):
            self.generate()

    def test_stale_capture_source_or_function(self):
        self.doc["source"] = dict(self.desc, sha256="0" * 64)
        with self.assertRaisesRegex(ValueError, "capture source"):
            self.generate()
        self.doc["source"] = self.desc
        self.doc["function_sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "function span/hash"):
            self.generate()

    def test_capture_hash_and_contract_drift(self):
        with self.assertRaisesRegex(ValueError, "hash drift"):
            self.generate(capture_sha256="0" * 64)
        (self.root / "A.h").write_text("int A(int x);")
        with self.assertRaisesRegex(ValueError, "hash drift"):
            self.generate()

    def test_replay_source_requires_hash_and_disables_manifest(self):
        frozen = self.root / "frozen.c"
        frozen.write_bytes(self.raw)
        # Capture still names the mutable live path, now advanced by retention.
        self.doc["source"] = self.desc
        self.source.write_bytes(self.raw + b"\n")
        current = repair._read(self.root, self.source)[1]
        self.index.write_text(json.dumps({"inputs": {"source": current}, "functions": [{"function": "f"}]}))
        with self.assertRaisesRegex(ValueError, "requires its SHA"):
            self.generate(source=frozen)
        result = self.generate(source=frozen, source_sha256=self.desc["sha256"])
        self.assertFalse(result["runnable_manifest"])
        manifest = json.loads(Path(result["manifest"]).read_text())
        self.assertFalse(manifest["root_reviewed"])
        self.assertEqual(manifest["live_source_sha256"], self.desc["sha256"])

    def test_unknown_actual_return_type_fails_closed(self):
        self.doc["events"][0]["actual_call_origin"]["normalized_expression_tree"]["tree"]["type"] = {"status": "UNKNOWN"}
        with self.assertRaisesRegex(ValueError, "unavailable"):
            self.generate()

    def test_different_compiler_type_format_rejected(self):
        self.doc["compiler"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "pinned GC2.6"):
            self.generate()


class DeclarationDiagnosticTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)

    def bound(self, filename, text):
        raw = text.encode()
        path = self.root / filename
        path.write_bytes(raw)
        return dict(path=filename, sha256=repair._sha(raw), start_byte=0, end_byte=len(raw))

    def diagnose(self, caller="extern void HipDrop();", provider="void HipDrop(s16 id, HuVecF *pos) {", **extra):
        return repair.diagnose_declarations(root=self.root,
            caller=self.bound("caller.c", caller), provider=self.bound("provider.c", provider),
            reviewed_type_aliases=extra.pop("reviewed_type_aliases", {"s16": "short"}), **extra)

    def test_donkey_narrow_formal_exposes_conversion_and_incompatibility(self):
        report = self.diagnose()
        self.assertEqual(report["compatibility"], "INCOMPATIBLE")
        self.assertEqual(report["parameter_conversions"][0]["provider_type"], "short")
        self.assertEqual(report["parameter_conversions"][0]["default_promoted_type"], "int")
        self.assertTrue(report["requires_source_exception"])
        self.assertFalse(report["source_patch_emitted"])
        self.assertFalse(report["authority_advanced"])
        self.assertIn("UNKNOWN", report["native_formal_join"])
        self.assertIn("not proved", report["visibility_status"])
        self.assertEqual(report["diagnostic_sha256"], repair._sha(repair.frontier.canonical(
            {k: v for k, v in report.items() if k != "diagnostic_sha256"})))

    def test_typed_original_exposes_narrowing_without_incompatible_label(self):
        report = self.diagnose(caller="void HipDrop(s16, HuVecF *);")
        self.assertEqual(report["compatibility"], "COMPATIBLE")
        self.assertEqual(report["parameter_conversions"][0]["caller_conversion"],
                         "conversion to declared parameter type")

    def test_no_argument_prototype_is_not_nonprototype(self):
        report = self.diagnose(caller="void HipDrop(void);", provider="void HipDrop(void) {")
        self.assertEqual(report["observed_declarations"][0]["kind"], "prototype")
        self.assertEqual(report["compatibility"], "COMPATIBLE")
        self.assertEqual(report["parameter_conversions"], [])

    def test_renamed_float_case_and_promotion_stable_case(self):
        report = self.diagnose(caller="void Other();", provider="void Other(float value) {")
        self.assertEqual(report["compatibility"], "INCOMPATIBLE")
        self.assertEqual(report["parameter_conversions"][0]["default_promoted_type"], "double")
        report = self.diagnose(caller="void Other();", provider="void Other(int value, void *data) {")
        self.assertEqual(report["compatibility"], "COMPATIBLE")

    def test_unknown_types_and_complex_declarators_fail_unknown(self):
        for provider in ["void HipDrop(s16 id);", "void HipDrop(void (*cb)(int));",
                         "void HipDrop(int a[2]);", "void HipDrop(int, ...);",
                         "void HipDrop();"]:
            with self.subTest(provider=provider):
                self.assertEqual(self.diagnose(provider=provider, reviewed_type_aliases={})["compatibility"], "UNKNOWN")

    def test_mismatched_name_or_span_or_hash_rejected(self):
        with self.assertRaisesRegex(ValueError, "name mismatch"):
            self.diagnose(provider="void Other(int);")
        caller = self.bound("caller.c", "void HipDrop();")
        provider = self.bound("provider.c", "void HipDrop(int);")
        for change in [dict(sha256="0"*64), dict(start_byte=True), dict(end_byte=9999)]:
            with self.subTest(change=change), self.assertRaises(ValueError):
                repair.diagnose_declarations(root=self.root, caller=dict(caller, **change), provider=provider)

    def test_drift_at_end_rejected(self):
        with mock.patch.object(repair.compiler, "digest", return_value="0"*64):
            with self.assertRaisesRegex(ValueError, "changed during"):
                self.diagnose()

    def test_nonprototype_cannot_be_generated_as_canonical_repair(self):
        desc = self.bound("api.h", "void HipDrop();")
        with self.assertRaisesRegex(ValueError, "nonprototype is not a canonical repair"):
            repair._contracts(self.root, [{"declaration": "extern void HipDrop();", "evidence": desc}])

    def test_invalid_variadic_contract_cannot_be_generated(self):
        for params in ('...', 'void, ...', 'int, ..., double', 'int,', '...x'):
            with self.subTest(params=params):
                text = 'extern void Api('+params+');'
                desc = self.bound('api.h', text)
                with self.assertRaisesRegex(ValueError, 'invalid parameter list'):
                    repair._contracts(self.root, [{'declaration': text, 'evidence': desc}])
        text = 'extern void Api(int count, ...);'
        desc = self.bound('api.h', text)
        self.assertEqual(len(repair._contracts(self.root, [{'declaration': text, 'evidence': desc}])[0]), 1)


if __name__ == "__main__":
    unittest.main()
