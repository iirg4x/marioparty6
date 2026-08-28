import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools import differential_allocator_causal as dac


H = "1" * 64
SESSION = "session-differential-allocator-0001"


def seal(value, field):
    value.pop(field, None)
    value[field] = dac.canonical_sha256(value)
    return value


class Evidence:
    def __init__(self, root: Path, function="GenericAllocatorCase", mappings=None, controls=None):
        self.root = root
        self.function = function
        self.mappings = mappings or [("ownerA", "r31", "r30"), ("ownerB", "r30", "r31")]
        self.controls = controls or []
        source_lines = [f"int {owner} = input_{index};\n" for index, (owner, _, _) in enumerate(self.mappings)]
        source_text = "".join(source_lines)
        self.source_path = root / "candidate.c"
        self.source_path.write_text(source_text, encoding="ascii", newline="")
        source_bytes = self.source_path.read_bytes()
        spans = []
        offset = 0
        for index, ((owner, _, _), line) in enumerate(zip(self.mappings, source_lines)):
            encoded = line.encode("ascii")
            spans.append({
                "span_id": f"span{index}", "natural": True, "byte_start": offset,
                "byte_end": offset + len(encoded), "text_sha256": hashlib.sha256(encoded).hexdigest(),
            })
            offset += len(encoded)
        self.spans = seal({
            "schema": dac.SPAN_SCHEMA, "function": function, "authority_advanced": False,
            "session_id": SESSION, "trust_anchor_sha256": "9" * 64,
            "source": {"path": str(self.source_path.resolve()), "size": len(source_bytes), "sha256": hashlib.sha256(source_bytes).hexdigest()},
            "spans": spans,
        }, "manifest_sha256")
        target_rows = []
        candidate_rows = []
        facts = []
        for index, (owner, candidate_register, target_register) in enumerate(self.mappings, 10):
            target_rows.append({"index": index, "diff_kind": "DIFF_ARG_MISMATCH", "instruction": {"formatted": f"mr {target_register},r3"}})
            candidate_rows.append({"index": index, "diff_kind": "DIFF_ARG_MISMATCH", "instruction": {"formatted": f"mr {candidate_register},r3"}})
            facts.append({
                "owner_id": owner, "source_span_id": f"span{index - 10}", "row_indices": [index],
                "session_id": SESSION,
                "object_token": f"object{index}", "varinfo_id": f"varinfo{index}",
                "pcode_def_token": f"def{index}", "pcode_use_tokens": [f"use{index}"],
                "ig_node_id": f"ig{index}", "vreg": f"r{100 + index}",
                "candidate_physical_register": candidate_register, "target_physical_register": target_register,
                "interference_neighbors": [f"igNeighbor{index}"], "classification": "UNIQUE", "missing_edge": None,
                "lifetime": {"birth": index, "assignment": index + 1, "first_use": index + 2, "last_use": index + 3},
            })
        channel = {"target": {"rows": copy.deepcopy(target_rows)}, "candidate": {"rows": copy.deepcopy(candidate_rows)}}
        self.focus = seal({
            "schema": dac.FOCUS_SCHEMA, "function": function, "authority_advanced": False,
            "channels": {"strict": copy.deepcopy(channel), "data": copy.deepcopy(channel)},
        }, "artifact_sha256")
        self.streams = seal({
            "schema": dac.STREAM_SCHEMA, "function": function, "authority_advanced": False,
            "target_cfg_sha256": "2" * 64, "candidate_cfg_sha256": "2" * 64,
            "target_relocation_sha256": "3" * 64, "candidate_relocation_sha256": "3" * 64,
            "target": {"rows": copy.deepcopy(target_rows)}, "candidate": {"rows": copy.deepcopy(candidate_rows)},
        }, "stream_sha256")
        self.relocations = seal({
            "physical_relocations_exact": True, "physical_relocation_differences": [],
        }, "receipt_sha256")
        self.trace = seal({
            "schema": dac.TRACE_SCHEMA, "function": function, "compiler_sha256": "4" * 64,
            "tool_sha256": "5" * 64, "session_id": SESSION, "trust_anchor_sha256": "9" * 64,
            "authority_advanced": False, "owner_facts": facts,
        }, "trace_sha256")
        rejected = [
            {"control_id": control, "source_sha256": "6" * 64, "object_sha256": "7" * 64, "outcome": "measured nonexact control"}
            for control in self.controls
        ]
        owner_ids = [owner for owner, _, _ in self.mappings]
        rows = list(range(10, 10 + len(self.mappings)))
        hypothesis = {
            "source_class": "natural_lifetime_boundary", "owner_ids": owner_ids, "row_indices": rows,
            "suppresses_control_ids": list(self.controls),
            "axes": [
                {"id": "owner_boundary", "hypothesis": "preserve the semantic owner boundary", "source_action": "move the truthful owner assignment to its first consumer", "topology_token": "owner_boundary"},
                {"id": "consumer_boundary", "hypothesis": "preserve the typed consumer boundary", "source_action": "consume the live typed owner at the existing call", "topology_token": "consumer_boundary"},
            ],
        }
        def descriptor(payload_sha256):
            return {"path": str((root / "unused.json").resolve()), "file_sha256": H, "payload_sha256": payload_sha256}
        self.context = seal({
            "schema": dac.CONTEXT_SCHEMA, "function": function, "focus": descriptor(self.focus["artifact_sha256"]),
            "physical_streams": descriptor(self.streams["stream_sha256"]),
            "physical_relocation_receipt": descriptor(self.relocations["receipt_sha256"]),
            "source_spans": descriptor(self.spans["manifest_sha256"]), "trace": descriptor(self.trace["trace_sha256"]),
            "compiler": {"path": str((root / "compiler.exe").resolve()), "size": 0, "sha256": "4" * 64},
            "tool": {"path": str((root / "tool.py").resolve()), "size": 0, "sha256": "5" * 64},
            "source_class_allowlist": ["natural_lifetime_boundary"],
            "source_class_hypotheses": [hypothesis], "rejected_controls": rejected,
            "session_id": SESSION, "trust_anchor_sha256": "9" * 64,
        }, "context_sha256")

    def solve(self):
        for key, value, digest_field in (
            ("focus", self.focus, "artifact_sha256"),
            ("physical_streams", self.streams, "stream_sha256"),
            ("physical_relocation_receipt", self.relocations, "receipt_sha256"),
            ("source_spans", self.spans, "manifest_sha256"),
            ("trace", self.trace, "trace_sha256"),
        ):
            self.context[key]["payload_sha256"] = value[digest_field]
        seal(self.context, "context_sha256")
        return dac.solve(self.context, self.focus, self.streams, self.relocations, self.spans, self.trace, self.context["context_sha256"])

    def materialize(self):
        compiler = self.root / "compiler.exe"
        tool = self.root / "tool.py"
        compiler.write_bytes(b"compiler")
        tool.write_bytes(b"tool")
        compiler_sha = dac.file_sha256(compiler)
        tool_sha = dac.file_sha256(tool)
        self.trace["compiler_sha256"] = compiler_sha
        self.trace["tool_sha256"] = tool_sha
        seal(self.trace, "trace_sha256")

        def write_bound(name, value):
            path = self.root / name
            path.write_text(json.dumps(value, sort_keys=True), encoding="utf-8")
            digest_field = {
                "focus.json": "artifact_sha256", "streams.json": "stream_sha256",
                "relocations.json": "receipt_sha256", "spans.json": "manifest_sha256",
                "trace.json": "trace_sha256",
            }[name]
            return {"path": str(path.resolve()), "file_sha256": dac.file_sha256(path), "payload_sha256": value[digest_field]}

        self.context["focus"] = write_bound("focus.json", self.focus)
        self.context["physical_streams"] = write_bound("streams.json", self.streams)
        self.context["physical_relocation_receipt"] = write_bound("relocations.json", self.relocations)
        self.context["source_spans"] = write_bound("spans.json", self.spans)
        self.context["trace"] = write_bound("trace.json", self.trace)
        self.context["compiler"] = {"path": str(compiler.resolve()), "size": compiler.stat().st_size, "sha256": compiler_sha}
        self.context["tool"] = {"path": str(tool.resolve()), "size": tool.stat().st_size, "sha256": tool_sha}
        seal(self.context, "context_sha256")
        context_path = self.root / "context.json"
        context_path.write_text(json.dumps(self.context, sort_keys=True), encoding="utf-8")
        return context_path


class DifferentialAllocatorCausalTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)

    def tearDown(self):
        self.temporary.cleanup()

    def test_two_gpr_closed_interaction_ranks_one_natural_class(self):
        result = Evidence(self.root).solve()
        self.assertEqual(result["status"], "RANKED_SOURCE_CLASS")
        self.assertTrue(result["maximal_closed_permutation"]["complete"])
        self.assertEqual(result["ranked_source_classes"][0]["source_class"], "natural_lifetime_boundary")
        request = result["candidate_interaction_request"]
        self.assertIsNotNone(request)
        self.assertEqual(request["schema"], dac.REQUEST_SCHEMA)
        self.assertEqual(request["max_cells"], 1)
        self.assertFalse(request["matrix_expansion"])
        self.assertNotIn("axes", request)
        self.assertEqual(len(request["composed_axes"]), 2)
        unsigned_request = dict(request)
        unsigned_request.pop("request_sha256")
        self.assertEqual(request["request_sha256"], dac.canonical_sha256(unsigned_request))
        unsigned = dict(result)
        unsigned.pop("result_sha256")
        self.assertEqual(result["result_sha256"], dac.canonical_sha256(unsigned))
        self.assertFalse(result["authority_advanced"])

    def test_config_pad_main_equivalent_reports_precise_absent_hook(self):
        evidence = Evidence(self.root, function="ConfigPadMain")
        evidence.trace["owner_facts"][0]["varinfo_id"] = None
        evidence.trace["owner_facts"][0]["vreg"] = None
        evidence.trace["owner_facts"][0]["classification"] = "UNKNOWN"
        evidence.trace["owner_facts"][0]["missing_edge"] = "object_to_varinfo"
        seal(evidence.trace, "trace_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["first_missing_edge"], "owner_ownerA_missing_edge:object_to_varinfo")
        self.assertIsNone(result["candidate_interaction_request"])

    def test_config_pad_open_extra_data_owner_three_cycle(self):
        mappings = [("dataP", "r25", "r26"), ("pad", "r26", "r27"), ("state", "r27", "r25")]
        result = Evidence(self.root, function="ConfigPadOpen", mappings=mappings).solve()
        self.assertEqual(result["status"], "RANKED_SOURCE_CLASS")
        self.assertEqual(len(result["maximal_closed_permutation"]["mapping"]), 3)

    def test_exact_c384_equivalent_has_zero_groups_and_does_not_infer(self):
        evidence = Evidence(self.root, function="ConfigPadOpen")
        for channel in evidence.focus["channels"].values():
            channel["target"]["rows"] = []
            channel["candidate"]["rows"] = []
        seal(evidence.focus, "artifact_sha256")
        evidence.streams["target"]["rows"] = []
        evidence.streams["candidate"]["rows"] = []
        seal(evidence.streams, "stream_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertFalse(result["inference_started"])
        self.assertEqual(result["first_missing_edge"], "focus_has_zero_residual_groups")

    def test_allocation_consumer_chain_replay(self):
        mappings = [("allocation", "r28", "r30"), ("cursor", "r30", "r31"), ("object", "r31", "r28")]
        result = Evidence(self.root, function="AllocationConsumer", mappings=mappings).solve()
        self.assertEqual(result["status"], "RANKED_SOURCE_CLASS")
        self.assertEqual([fact["owner_id"] for fact in result["owner_facts"]], ["allocation", "cursor", "object"])

    def test_cfg_difference_fails_before_allocator_inference(self):
        evidence = Evidence(self.root)
        evidence.streams["candidate_cfg_sha256"] = "8" * 64
        seal(evidence.streams, "stream_sha256")
        result = evidence.solve()
        self.assertEqual(result["failure_stage"], "pre_allocator_gate")
        self.assertFalse(result["inference_started"])
        self.assertIn("cfg_fingerprint_differs", result["blockers"])

    def test_missing_physical_fingerprint_is_rejected_before_inference(self):
        evidence = Evidence(self.root)
        evidence.streams.pop("target_cfg_sha256")
        seal(evidence.streams, "stream_sha256")
        with self.assertRaisesRegex(dac.DifferentialAllocatorInputError, "missing fields: target_cfg_sha256"):
            evidence.solve()

    def test_malformed_physical_fingerprint_is_rejected_before_inference(self):
        evidence = Evidence(self.root)
        evidence.streams["candidate_relocation_sha256"] = "not-a-sha"
        seal(evidence.streams, "stream_sha256")
        with self.assertRaisesRegex(dac.DifferentialAllocatorInputError, "must be lowercase SHA-256"):
            evidence.solve()

    def test_physical_stream_unknown_field_is_rejected(self):
        evidence = Evidence(self.root)
        evidence.streams["target"]["rows"][0]["unsealed_annotation"] = True
        seal(evidence.streams, "stream_sha256")
        with self.assertRaisesRegex(dac.DifferentialAllocatorInputError, "unsupported fields: unsealed_annotation"):
            evidence.solve()

    def test_sealed_row_99_candidate_residual_cannot_hide_outside_focus(self):
        evidence = Evidence(self.root)
        evidence.streams["target"]["rows"].append({
            "index": 99, "diff_kind": None, "instruction": {"formatted": "mr r3,r3"},
        })
        evidence.streams["candidate"]["rows"].append({
            "index": 99, "diff_kind": "DIFF_ARG_MISMATCH", "instruction": {"formatted": "mr r4,r3"},
        })
        seal(evidence.streams, "stream_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertFalse(result["inference_started"])
        self.assertIn("candidate_physical_stream_focus_residual_set_mismatch", result["blockers"])
        self.assertIn("row_99_physical_stream_diff_kind_mismatch", result["blockers"])
        self.assertIn("row_99_nonfocus_physical_text_or_operand_mismatch", result["blockers"])

    def test_nonfocus_aligned_operand_discrepancy_is_rejected(self):
        evidence = Evidence(self.root)
        evidence.streams["target"]["rows"].append({
            "index": 99, "diff_kind": None, "instruction": {"formatted": "mr r3,r3"},
        })
        evidence.streams["candidate"]["rows"].append({
            "index": 99, "diff_kind": None, "instruction": {"formatted": "mr r4,r3"},
        })
        seal(evidence.streams, "stream_sha256")
        result = evidence.solve()
        self.assertFalse(result["inference_started"])
        self.assertIn("row_99_nonfocus_physical_text_or_operand_mismatch", result["blockers"])

    def test_asymmetric_target_and_candidate_physical_extras_are_rejected(self):
        for side in ("target", "candidate"):
            with self.subTest(side=side):
                subroot = self.root / side
                subroot.mkdir()
                evidence = Evidence(subroot)
                evidence.streams[side]["rows"].append({
                    "index": 99, "diff_kind": None, "instruction": {"formatted": "mr r3,r3"},
                })
                seal(evidence.streams, "stream_sha256")
                result = evidence.solve()
                self.assertFalse(result["inference_started"])
                self.assertIn("physical_stream_row_sets_differ", result["blockers"])

    def test_relocation_difference_fails_before_allocator_inference(self):
        evidence = Evidence(self.root)
        evidence.relocations["physical_relocations_exact"] = False
        evidence.relocations["physical_relocation_differences"] = ["offset_12"]
        seal(evidence.relocations, "receipt_sha256")
        result = evidence.solve()
        self.assertFalse(result["inference_started"])
        self.assertIn("physical_relocations_not_exact", result["blockers"])

    def test_generic_capspecial_cycle_binds_lifetimes_and_rejected_controls(self):
        controls = ["c937", "c940", "c941", "c942"]
        evidence = Evidence(
            self.root, function="ArbitraryCoinHandler",
            mappings=[("semanticWork", "r26", "r28"), ("resourceFlag", "r28", "r26")], controls=controls,
        )
        result = evidence.solve()
        self.assertEqual(result["status"], "RANKED_SOURCE_CLASS")
        self.assertEqual(result["ranked_source_classes"][0]["suppresses_control_ids"], controls)
        self.assertEqual(result["owner_facts"][0]["lifetime"], {"birth": 10, "assignment": 11, "first_use": 12, "last_use": 13})
        self.assertNotIn("Koopa", json.dumps(result))

    def test_cross_session_trace_is_unknown(self):
        evidence = Evidence(self.root)
        evidence.trace["session_id"] = "session-differential-allocator-foreign"
        seal(evidence.trace, "trace_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("trace_session_id_mismatch", result["blockers"])

    def test_cross_session_owner_fact_is_unknown(self):
        evidence = Evidence(self.root)
        evidence.trace["owner_facts"][0]["session_id"] = "session-differential-allocator-foreign"
        seal(evidence.trace, "trace_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("owner_ownerA_session_id_mismatch", result["blockers"])

    def test_trace_trust_anchor_mismatch_is_unknown(self):
        evidence = Evidence(self.root)
        evidence.trace["trust_anchor_sha256"] = "a" * 64
        seal(evidence.trace, "trace_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("trace_trust_anchor_mismatch", result["blockers"])

    def test_source_span_session_mismatch_is_unknown(self):
        evidence = Evidence(self.root)
        evidence.spans["session_id"] = "session-differential-allocator-foreign"
        seal(evidence.spans, "manifest_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("source_span_session_id_mismatch", result["blockers"])

    def test_target_pseudo_owner_is_derived_from_stream_and_claim_verified(self):
        evidence = Evidence(self.root)
        evidence.trace["owner_facts"][0]["target_physical_register"] = "r29"
        seal(evidence.trace, "trace_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("owner_ownerA_target_register_claim_mismatch", result["blockers"])
        fact = result["owner_facts"][0]
        self.assertEqual(fact["target_claim"], "r29")
        self.assertEqual(fact["target"], "r30")
        self.assertEqual(fact["target_residual_interval"]["anchors"][0]["operand_positions"], [0])

    def test_missing_control_suppression_returns_unknown(self):
        evidence = Evidence(self.root, controls=["c937"])
        evidence.context["source_class_hypotheses"][0]["suppresses_control_ids"] = []
        seal(evidence.context, "context_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["first_missing_edge"], "minimum_causal_frontier_not_unique")

    def test_ambiguous_minimum_frontier_lists_at_most_three_without_request(self):
        evidence = Evidence(self.root)
        base = evidence.context["source_class_hypotheses"][0]
        for ordinal in range(1, 4):
            alternate = copy.deepcopy(base)
            alternate["source_class"] = f"alternate_boundary_{ordinal}"
            evidence.context["source_class_allowlist"].append(alternate["source_class"])
            evidence.context["source_class_hypotheses"].append(alternate)
        seal(evidence.context, "context_sha256")
        result = evidence.solve()
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(len(result["ranked_source_classes"]), 3)
        self.assertTrue(result["maximal_closed_permutation"]["complete"])
        self.assertIsNone(result["candidate_interaction_request"])

    def test_prohibited_matrix_hypothesis_is_rejected(self):
        evidence = Evidence(self.root)
        evidence.context["source_class_hypotheses"][0]["axes"][0]["source_action"] = "run a declaration matrix"
        seal(evidence.context, "context_sha256")
        with self.assertRaisesRegex(dac.DifferentialAllocatorInputError, "prohibited source class"):
            evidence.solve()

    def test_output_schema_is_parseable(self):
        schema = json.loads((Path(__file__).parents[1] / "DIFFERENTIAL_ALLOCATOR_CAUSAL_V1.schema.json").read_text(encoding="utf-8"))
        self.assertEqual(schema["properties"]["schema"]["const"], dac.SCHEMA)

    def test_path_entrypoint_verifies_all_bound_files_and_binaries(self):
        evidence = Evidence(self.root)
        context_path = evidence.materialize()
        result = dac.solve_from_paths(context_path, evidence.context["context_sha256"])
        self.assertEqual(result["status"], "RANKED_SOURCE_CLASS")
        evidence.source_path.write_text("tampered", encoding="ascii")
        with self.assertRaisesRegex(dac.DifferentialAllocatorInputError, "source spans source file identity mismatch"):
            dac.solve_from_paths(context_path, evidence.context["context_sha256"])

    def test_direct_api_rejects_payload_substitution_under_same_context_anchor(self):
        evidence = Evidence(self.root)
        substituted = copy.deepcopy(evidence.focus)
        substituted["channels"]["strict"]["target"]["rows"][0]["instruction"]["formatted"] = "mr r29,r3"
        seal(substituted, "artifact_sha256")
        with self.assertRaisesRegex(dac.DifferentialAllocatorInputError, "focus payload differs from context binding"):
            dac.solve(
                evidence.context, substituted, evidence.streams, evidence.relocations,
                evidence.spans, evidence.trace, evidence.context["context_sha256"],
            )


if __name__ == "__main__":
    unittest.main()
