import copy
import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import recovery_call_contract_repair as repair


class CalleeReturnTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)

    def diagnose(self, instructions, **extra):
        rows = [{"instruction": {"address": str(i*4), "formatted": text}} for i, text in enumerate(instructions)]
        path = self.root / "report.json"
        path.write_text(json.dumps({"left": {"symbols": [{"name": "api", "address": "0", "size": str(4*len(rows)), "instructions": rows}]}}), encoding="utf-8")
        return repair.diagnose_callee_return(root=self.root, report=path,
            report_sha256=extra.pop("report_sha256", repair._sha(path.read_bytes())), symbol="api", **extra)

    def test_common_entry_value_multiple_returns(self):
        result = self.diagnose(["subi r3, r3, 0x259", "cmpwi r3, 0", "bge 0x10", "blr", "blr"])
        self.assertEqual(result["status"], "COMMON_VALUE")
        self.assertEqual(result["common_normalized_entry_value"]["addend"], -601)
        self.assertEqual(result["common_normalized_entry_value"]["base"], "entry.r3")
        self.assertEqual(result["reachable_return_count"], 2)
        self.assertEqual(result["returns"][0]["r3_reaching_definition_indices"], [0])
        self.assertEqual(result["c_abi_return_type"], "UNKNOWN")
        self.assertFalse(result["source_patch_emitted"])

    def test_call_clobber_and_conflicting_exits(self):
        for code in (["addi r3, r3, -601", "bl 0x100", "blr"],
                     ["beq 0xc", "li r3, 1", "blr", "li r3, 2", "blr"],
                     ["lwz r3, 0(r4)", "blr"]):
            self.assertEqual(self.diagnose(code)["status"], "UNKNOWN")

    def test_unsupported_flow_and_multi_register_write(self):
        for code in (["bctr", "blr"], ["bc 12, 2, 0x8", "blr", "blr"],
                     ["lmw r0, 0(r1)", "blr"], ["lwzu r4, 4(r3)", "blr"],
                     ["b 0x100", "blr"], ["nop"]):
            self.assertEqual(self.diagnose(code)["status"], "UNKNOWN")

    def test_join_loop_and_unreachable_unknown(self):
        result = self.diagnose(["addi r3, r3, -601", "bdnz 0x4", "blr", "bctr"])
        self.assertEqual(result["status"], "COMMON_VALUE")
        self.assertEqual(self.diagnose(["addi r3, r3, 1", "bdnz 0x0", "blr"])["status"], "UNKNOWN")
        joined = self.diagnose(["beq 0xc", "li r3, 1", "b 0x10", "li r3, 1", "blr"])
        self.assertEqual(joined["status"], "COMMON_VALUE")
        self.assertEqual(joined["returns"][0]["r3_reaching_definition_indices"], [1, 3])

    def test_call_provenance_and_unknown_relocation(self):
        restored = self.diagnose(["mr r31, r3", "bl 0x100", "mr r3, r31", "blr"])
        self.assertEqual(restored["common_normalized_entry_value"]["base"], "entry.r3")
        self.assertEqual(self.diagnose(["addi r3, r4, symbol@l", "blr"])["status"], "UNKNOWN")

    def test_signed16_immediates_and_out_of_range(self):
        for code, addend in (("addi r3, r3, 0xffff", -1), ("li r3, 0xffff", -1),
                             ("addis r3, r3, 0xffff", -65536),
                             ("subi r3, r3, 0x8000", -32768)):
            self.assertEqual(self.diagnose([code, "blr"])["common_normalized_entry_value"]["addend"], addend)
        for code in ("addi r3, r3, 0x10000", "li r3, -32769", "subi r3, r3, 0xffff"):
            self.assertEqual(self.diagnose([code, "blr"])["status"], "UNKNOWN")

    def test_separate_exits_same_value_different_definitions(self):
        result = self.diagnose(["beq 0xc", "li r3, 1", "blr", "li r3, 1", "blr"])
        self.assertEqual(result["status"], "COMMON_VALUE")
        self.assertEqual([r["r3_reaching_definition_indices"] for r in result["returns"]], [[1], [3]])

    def test_malformed_operand_counts_cannot_prove_return(self):
        for code in ("mr r3, r3, r4", "mr r4, r4, r5", "mr r3", "mr",
                     "li r3, 1, 2", "li r3", "addi r3, r3, 1, 2",
                     "addis r3, r3", "subi r3, r3, 1, 2", "subis r3",
                     "blr r3"):
            with self.subTest(instruction=code):
                result = self.diagnose([code, "blr"])
                self.assertEqual(result["status"], "UNKNOWN")
                self.assertTrue(result["issues"])

    def test_report_drift_and_signature_binding(self):
        with self.assertRaisesRegex(ValueError, "hash drift"):
            self.diagnose(["blr"], report_sha256="0"*64)
        path = self.root / "api.h"
        path.write_text("void api(int id);", encoding="utf-8")
        desc = dict(path=str(path), sha256=repair._sha(path.read_bytes()), start_byte=0, end_byte=path.stat().st_size)
        result = self.diagnose(["blr"], source_signature=desc)
        self.assertEqual(result["source_signature"]["return_type"], "void")
        with mock.patch.object(repair.compiler, "digest", return_value="0"*64):
            with self.assertRaisesRegex(ValueError, "changed during analysis"):
                self.diagnose(["blr"])

    def test_cli_read_only(self):
        self.diagnose(["blr"])
        path = self.root / "report.json"
        args = ["repair", "callee-return", "--root", str(self.root), "--report", str(path),
                "--report-sha256", repair._sha(path.read_bytes()), "--symbol", "api"]
        with mock.patch("sys.argv", args), contextlib.redirect_stdout(io.StringIO()) as output:
            self.assertEqual(repair.main(), 0)
        self.assertEqual(json.loads(output.getvalue())["status"], "COMMON_VALUE")
        self.assertEqual(len(list(self.root.iterdir())), 1)

    def test_malformed_truncated_rows_and_null_alignment(self):
        self.diagnose(["nop", "blr"])
        path = self.root / "report.json"
        document = json.loads(path.read_text())
        for mutation in ("formatted", "address", "truncate", "gap", "size"):
            changed = copy.deepcopy(document)
            symbol = changed["left"]["symbols"][0]
            rows = symbol["instructions"]
            if mutation in {"formatted", "address"}:
                del rows[0]["instruction"][mutation]
            elif mutation == "truncate":
                rows.pop(0)
            elif mutation == "gap":
                rows[1]["instruction"]["address"] = "8"
            else:
                del symbol["size"]
            path.write_text(json.dumps(changed))
            with self.assertRaises(ValueError):
                repair.diagnose_callee_return(root=self.root, report=path,
                    report_sha256=repair._sha(path.read_bytes()), symbol="api")
        document["left"]["symbols"][0]["instructions"].insert(1, {"instruction": None})
        path.write_text(json.dumps(document))
        result = repair.diagnose_callee_return(root=self.root, report=path,
                    report_sha256=repair._sha(path.read_bytes()), symbol="api")
        self.assertEqual(result["status"], "COMMON_VALUE")


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
    def test_declarations_cli_accepts_relative_root_without_emitting_patch(self):
        request = dict(caller=self.bound("caller.c", "void Api(void);"),
                       provider=self.bound("provider.c", "int Api(void) {"))
        (self.root / "request.json").write_text(json.dumps(request))
        stdout = io.StringIO()
        with contextlib.chdir(self.root), contextlib.redirect_stdout(stdout), mock.patch.object(
            repair.sys, "argv", ["tool", "declarations", "--root", ".", "--request", "request.json"]
        ):
            self.assertEqual(repair.main(), 0)
        result = json.loads(stdout.getvalue())
        self.assertTrue(result["return_comparison"]["drift"])
        self.assertFalse(result["source_patch_emitted"])

    def test_return_drift_and_corrected_int_contract(self):
        report = self.diagnose(caller="extern void Api(int);", provider="int Api(int value) {")
        self.assertEqual(report["parameter_compatibility"], "COMPATIBLE")
        self.assertEqual(report["compatibility"], "INCOMPATIBLE")
        self.assertTrue(report["return_comparison"]["drift"])
        self.assertFalse(report["source_authority"])
        report = self.diagnose(caller="extern int Api(int);", provider="int Api(int value) {")
        self.assertEqual(report["compatibility"], "COMPATIBLE")
        self.assertFalse(report["return_comparison"]["drift"])

    def test_missing_declaration_is_bound_conditional_old_c_int(self):
        caller = dict(self.bound("caller.c", "Api(8, 30);"),
                      declaration_status="missing", symbol="Api")
        provider = self.bound("provider.c", "void Api(int type, int time) {")
        report = repair.diagnose_declarations(root=self.root, caller=caller, provider=provider)
        self.assertEqual(report["return_comparison"]["caller_type"], "int")
        self.assertTrue(report["return_comparison"]["drift"])
        self.assertIn("if caller visibility claim holds", report["observed_declarations"][0]["implicit_int_status"])
        self.assertIn("not resolved", report["missing_declaration_proof"])
        with self.assertRaisesRegex(ValueError, "bound callsite"):
            repair.diagnose_declarations(root=self.root, caller=dict(caller, symbol="Other"), provider=provider)
        with self.assertRaisesRegex(ValueError, "hash drift"):
            repair.diagnose_declarations(root=self.root, caller=dict(caller, sha256="0"*64), provider=provider)

    def test_unknown_return_typedef_and_ambiguous_visibility_stay_unknown(self):
        for spelling in ("Opaque", "Opaque *", "const int"):
            report = self.diagnose(caller=f"{spelling} Api(void);", provider="int Api(void) {")
            self.assertEqual(report["compatibility"], "UNKNOWN")
            self.assertIsNone(report["return_comparison"]["drift"])
        caller = dict(self.bound("caller.c", "void Api(void);"), visibility="ambiguous")
        provider = self.bound("provider.c", "int Api(void) {")
        report = repair.diagnose_declarations(root=self.root, caller=caller, provider=provider)
        self.assertEqual(report["compatibility"], "UNKNOWN")
        self.assertIsNone(report["return_comparison"]["drift"])

    def test_supported_return_pointer_alias_and_generator_restriction(self):
        report = self.diagnose(caller="s16 Api(void);", provider="short Api(void) {")
        self.assertEqual(report["compatibility"], "COMPATIBLE")
        report = self.diagnose(caller="void *Api(void);", provider="void *Api(void) {")
        self.assertEqual(report["compatibility"], "COMPATIBLE")
        with self.assertRaisesRegex(ValueError, "canonical void"):
            repair._signature("int Api(void);")

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
