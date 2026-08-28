from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools import volatile_owner_causal_join as reducer


SESSION = "session-0000000000000042"
FUNCTION = "fn_80001234"


def _seal(value: dict[str, object], field: str) -> dict[str, object]:
    value[field] = reducer.canonical_sha256(value)
    return value


def _instruction(index: int, formatted: str) -> dict[str, object]:
    return {
        "index": index,
        "diff_kind": "DIFF_ARG_MISMATCH",
        "arg_diff": [{}, {"diff_index": 0}, {}],
        "instruction": {"address": str(index * 4), "size": 4, "formatted": formatted},
    }


def _side(rows: list[dict[str, object]]) -> dict[str, object]:
    return {
        "rows_kind": "diff_only",
        "rows": rows,
        "diff_row_count": len(rows),
        "symbol": {"name": FUNCTION, "kind": "SYMBOL_FUNCTION", "size": "16"},
    }


def focus(physical_file_sha256: str, physical_payload_sha256: str) -> dict[str, object]:
    target = [_instruction(10, "add r4, r6, r7"), _instruction(11, "add r3, r6, r7")]
    candidate = [_instruction(10, "add r3, r6, r7"), _instruction(11, "add r4, r6, r7")]
    metric = {"target_size": 16, "candidate_size": 16, "diff_rows": 2, "diff_kinds": {"DIFF_ARG_MISMATCH": 2}, "exact": False}
    value: dict[str, object] = {
        "schema": reducer.FOCUS_SCHEMA,
        "authority_advanced": False,
        "function": FUNCTION,
        "input_binding": {"strict_report": {"path": "strict.json", "sha256": "17" * 32, "size_bytes": 100}, "data_report": {"path": "data.json", "sha256": "18" * 32, "size_bytes": 100}, "retail_target_authenticated": True, "authority_advanced": False},
        "physical_relocations": {
            "status": "exact",
            "authority": "independent_physical_receipt",
            "binding": {"path": "physical.json", "sha256": physical_file_sha256, "size_bytes": 10},
            "receipt_schema": "physical/v1",
            "target": {},
            "candidate": {},
            "physical_relocation_differences": [],
            "symbol_attribution_aliases": [],
            "receipt_payload_sha256": physical_payload_sha256,
        },
        "channels": {
            "strict": {"target": _side(copy.deepcopy(target)), "candidate": _side(copy.deepcopy(candidate)), "metric": copy.deepcopy(metric)},
            "data": {"target": _side(copy.deepcopy(target)), "candidate": _side(copy.deepcopy(candidate)), "metric": copy.deepcopy(metric)},
        },
    }
    return _seal(value, "artifact_sha256")


def span() -> dict[str, object]:
    value: dict[str, object] = {
        "schema": "mwcc_source_span_bindings/v1",
        "function": FUNCTION,
        "function_sha256": "12" * 32,
        "session_id": SESSION,
        "source": {"path": r"C:\proof\owner.c", "size": 100, "sha256": "13" * 32},
        "spans": [],
        "authority_advanced": False,
    }
    return _seal(value, "manifest_sha256")


def _fact(number: int, register: str, vreg: str) -> dict[str, object]:
    return {
        "fact_id": f"owner-fact-{number:06d}",
        "row_id": f"residual-{number:06d}",
        "role": "destination",
        "pcode_id": f"pcode-{SESSION}-{number:06d}",
        "ig_node_id": f"ig-{SESSION}-{number:06d}",
        "vreg": vreg,
        "final_color": int(register[1:]),
        "physical_register": register,
        "def_id": f"def-{20 + number:06d}",
        "use_ids": [f"use-{30 + number:06d}"],
        "classification": "UNIQUE",
    }


def graph() -> dict[str, object]:
    rows = [
        {"row_id": f"residual-{number:06d}", "ordinal": 0, "kind": 2, "owner_fact_id": f"owner-fact-{number:06d}", "required_operand_roles": ["destination"]}
        for number in range(2)
    ]
    section: dict[str, object] = {
        "schema": f"{reducer.GRAPH_SCHEMA}/volatile-owner-facts/v1",
        "status": "DIAGNOSTIC_ONLY",
        "authority_advanced": False,
        "session_id": SESSION,
        "function": FUNCTION,
        "source": {"path": r"C:\proof\owner.c", "size": 100, "sha256": "13" * 32},
        "compiler": {"path": r"C:\proof\mwcceppc.exe", "size": 200, "sha256": "14" * 32},
        "candidate_object": {"path": r"C:\proof\owner.o", "size": 16, "sha256": "15" * 32},
        "raw_event_hashes": [{"event_id": f"{SESSION}-e00000{number}", "sha256": f"{20 + number:02x}" * 32} for number in range(2)],
        "object_identity_bindings": [
            {"fact_id": f"owner-fact-{number:06d}", "status": "PRESENT", "object_token": f"local-{SESSION}-{number:06d}"}
            for number in range(2)
        ],
        "closed_residual_rows": rows,
        "owner_facts": [_fact(0, "r3", "r32"), _fact(1, "r4", "r33")],
    }
    _seal(section, "volatile_owner_facts_sha256")
    outer: dict[str, object] = {
        "schema": reducer.GRAPH_SCHEMA,
        "status": "UNKNOWN",
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
        "session_id": SESSION,
        "function": FUNCTION,
        "validation_failure": "machine owner join lacks an exact physical-register edge",
        "first_absent_edge": {},
        "join_attempts": [],
        "unresolved_machine_sites": [],
        "volatile_owner_facts": section,
        "machine_event_ids": [],
    }
    return _seal(outer, "failure_graph_sha256")


def context(focus_value: dict[str, object], span_value: dict[str, object], graph_value: dict[str, object], root: Path, physical_file_sha256: str, physical_payload_sha256: str) -> dict[str, object]:
    value: dict[str, object] = {
        "schema": reducer.CONTEXT_SCHEMA,
        "session_id": SESSION,
        "function": FUNCTION,
        "strict_report_sha256": "17" * 32,
        "data_report_sha256": "18" * 32,
        "focus": {"file_sha256": "aa" * 32, "artifact_sha256": focus_value["artifact_sha256"]},
        "source_span_manifest": {"file_sha256": "bb" * 32, "manifest_sha256": span_value["manifest_sha256"]},
        "target_object": {"path": str(root / "target.o"), "size": 16, "sha256": hashlib.sha256(b"T" * 16).hexdigest()},
        "candidate_object": {"path": str(root / "candidate.o"), "size": 16, "sha256": hashlib.sha256(b"C" * 16).hexdigest()},
        "physical_relocation_receipt": {"path": str(root / "physical.json"), "file_sha256": physical_file_sha256, "receipt_payload_sha256": physical_payload_sha256},
        "ownership_failure_graph": {"file_sha256": "cc" * 32, "failure_graph_sha256": graph_value["failure_graph_sha256"]},
        "source_class_allowlist": ["volatile_index_owner", "aggregate_base_owner"],
        "residual_row_bindings": [
            {"row_id": "residual-000000", "focus_row_index": 10, "candidate_operand_index": 0, "captured_role": "destination", "semantic_role": "result"},
            {"row_id": "residual-000001", "focus_row_index": 11, "candidate_operand_index": 0, "captured_role": "destination", "semantic_role": "result"},
        ],
        "source_class_hypotheses": [{"source_class": "volatile_index_owner", "row_ids": ["residual-000000", "residual-000001"]}],
    }
    return _seal(value, "context_sha256")


class VolatileOwnerCausalJoinTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        (self.root / "target.o").write_bytes(b"T" * 16)
        (self.root / "candidate.o").write_bytes(b"C" * 16)
        self.physical_receipt = {
            "schema": "physical/v1",
            "report": {"path": "strict.json", "sha256": "17" * 32},
            "physical_relocations_exact": True,
            "physical_relocation_differences": [],
        }
        physical_text = json.dumps(self.physical_receipt)
        (self.root / "physical.json").write_text(physical_text, encoding="utf-8")
        self.physical_file_sha256 = hashlib.sha256(physical_text.encode("utf-8")).hexdigest()
        self.physical_payload_sha256 = reducer.canonical_sha256(self.physical_receipt)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def fixture(self) -> tuple[dict[str, object], dict[str, object], dict[str, object], dict[str, object]]:
        focus_value, span_value, graph_value = focus(self.physical_file_sha256, self.physical_payload_sha256), span(), graph()
        graph_value["volatile_owner_facts"]["candidate_object"]["sha256"] = hashlib.sha256(b"C" * 16).hexdigest()
        self._reseal_graph(graph_value)
        return focus_value, span_value, graph_value, context(focus_value, span_value, graph_value, self.root, self.physical_file_sha256, self.physical_payload_sha256)

    def enriched_fixture(self) -> tuple[dict[str, object], dict[str, object], dict[str, object], dict[str, object]]:
        focus_value, _, graph_value, _ = self.fixture()
        source_bytes = b"int owner0;\nint owner1;\n"
        source_path = self.root / "owner.c"
        source_path.write_bytes(source_bytes)
        source_descriptor = {
            "path": str(source_path),
            "size": len(source_bytes),
            "sha256": hashlib.sha256(source_bytes).hexdigest(),
        }
        span_value: dict[str, object] = {
            "schema": "mwcc_source_span_bindings/v2",
            "function": FUNCTION,
            "function_sha256": "12" * 32,
            "session_id": SESSION,
            "source": copy.deepcopy(source_descriptor),
            "objects": [
                {
                    "byte_size": 4,
                    "identity": f"owner{number}",
                    "object_token": f"local-{SESSION}-{number:06d}",
                    "object_type": "int",
                    "ownership_mode": "source_object",
                }
                for number in range(2)
            ],
            "spans": [
                {
                    "byte_start": 0 if number == 0 else 12,
                    "byte_end": 11 if number == 0 else 23,
                    "dependency_id": None,
                    "identity": f"owner{number}",
                    "line_start": number + 1,
                    "line_end": number + 1,
                    "machine_instruction_indices": [10 + number],
                    "object_token": f"local-{SESSION}-{number:06d}",
                    "role": "declaration",
                    "text_sha256": hashlib.sha256(source_bytes[(0 if number == 0 else 12):(11 if number == 0 else 23)]).hexdigest(),
                }
                for number in range(2)
            ],
            "authority_advanced": False,
        }
        _seal(span_value, "manifest_sha256")
        section = graph_value["volatile_owner_facts"]
        section["source"] = copy.deepcopy(source_descriptor)
        for number, fact in enumerate(section["owner_facts"]):
            fact["interference_neighbors"] = [
                {
                    "ig_node_id": f"ig-{SESSION}-{100 + number:06d}",
                    "vreg": f"r{40 + number}",
                    "final_color": 6 + number,
                    "physical_register": f"r{6 + number}",
                }
            ]
            fact["missing_edge"] = None
        self._reseal_graph(graph_value)
        context_value = context(
            focus_value,
            span_value,
            graph_value,
            self.root,
            self.physical_file_sha256,
            self.physical_payload_sha256,
        )
        return focus_value, span_value, graph_value, context_value

    @staticmethod
    def build_join(focus_value: dict[str, object], span_value: dict[str, object], graph_value: dict[str, object], context_value: dict[str, object]) -> dict[str, object]:
        return reducer.build_join(
            focus_value,
            span_value,
            graph_value,
            context_value,
            context_value["context_sha256"],
        )

    def test_unique_join_proves_closed_permutation_and_one_class(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "PROVEN")
        self.assertEqual(result["register_permutation"]["changed_mapping"], [{"candidate": "r3", "target": "r4"}, {"candidate": "r4", "target": "r3"}])
        self.assertEqual(result["ranked_source_classes"], [{"rank": 1, "source_class": "volatile_index_owner"}])
        self.assertNotIn("path", json.dumps(result["evidence_binding"], sort_keys=True))

    def test_enriched_join_exposes_def_use_interference_color_and_source_span(self) -> None:
        focus_value, span_value, graph_value, context_value = self.enriched_fixture()
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["schema"], reducer.ENRICHED_SCHEMA)
        self.assertEqual(result["status"], "PROVEN")
        self.assertEqual(len(result["row_diagnostics"]), 2)
        first = result["row_diagnostics"][0]
        self.assertEqual(first["pcode_id"], f"pcode-{SESSION}-000000")
        self.assertEqual(first["def_id"], "def-000020")
        self.assertEqual(first["use_ids"], ["use-000030"])
        self.assertEqual(first["candidate_physical_register"], "r3")
        self.assertEqual(first["target_physical_register"], "r4")
        self.assertEqual(first["interference_neighbors"][0]["vreg"], "r40")
        self.assertEqual(first["source_span"]["identity"], "owner0")
        self.assertEqual(first["source_span"]["text_sha256"], hashlib.sha256(b"int owner0;").hexdigest())
        self.assertIsNone(first["missing_edge"])

    def test_enriched_join_missing_interference_is_unknown_with_explicit_edge(self) -> None:
        focus_value, span_value, graph_value, context_value = self.enriched_fixture()
        del graph_value["volatile_owner_facts"]["owner_facts"][0]["interference_neighbors"]
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["row_diagnostics"][0]["missing_edge"], "ig_interference_neighbors")
        self.assertIn("owner_fact_interference_evidence_missing:owner-fact-000000", result["blockers"])

    def test_enriched_join_capture_missing_edge_never_ranks_source(self) -> None:
        focus_value, span_value, graph_value, context_value = self.enriched_fixture()
        graph_value["volatile_owner_facts"]["owner_facts"][0]["missing_edge"] = "object_to_vreg"
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["ranked_source_classes"], [])
        self.assertEqual(result["row_diagnostics"][0]["missing_edge"], "object_to_vreg")

    def test_enriched_source_span_bytes_are_hash_verified(self) -> None:
        focus_value, span_value, graph_value, context_value = self.enriched_fixture()
        (self.root / "owner.c").write_bytes(b"int changed;\nint owner1;\n")
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("source_span_source_file_identity_mismatch", result["blockers"])

    def test_schema_loader_preserves_v1_and_selects_v2_fail_closed(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        legacy = self.build_join(focus_value, span_value, graph_value, context_value)
        legacy_schema = reducer.schema_path_for_document(legacy)
        self.assertEqual(legacy_schema.name, "VOLATILE_OWNER_CAUSAL_JOIN_V1.schema.json")
        self.assertEqual(json.loads(legacy_schema.read_text(encoding="utf-8"))["$id"], legacy_schema.name)

        focus_value, span_value, graph_value, context_value = self.enriched_fixture()
        enriched = self.build_join(focus_value, span_value, graph_value, context_value)
        enriched_schema = reducer.schema_path_for_document(enriched)
        schema = json.loads(enriched_schema.read_text(encoding="utf-8"))
        self.assertEqual(enriched_schema.name, "VOLATILE_OWNER_CAUSAL_JOIN_V2.schema.json")
        self.assertEqual(schema["properties"]["schema"]["const"], reducer.ENRICHED_SCHEMA)
        self.assertIn("row_diagnostics", schema["required"])
        self.assertIn("source_span", schema["$defs"]["binding"]["required"])
        with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "unsupported"):
            reducer.schema_path_for_document({"schema": "volatile_owner_causal_join/v3"})

    def test_output_schema_rejects_evidence_free_proven_documents(self) -> None:
        def enforce_proven_gate(schema: dict[str, object], document: dict[str, object]) -> None:
            proven_rules = schema["allOf"][0]["then"]["properties"]
            for field in ("bindings", "ranked_source_classes", "closed_residual_rows"):
                minimum = proven_rules[field]["minItems"]
                if len(document[field]) < minimum:
                    raise ValueError(field)
            permutation_rules = proven_rules["register_permutation"]["properties"]
            for field in ("mapping", "changed_mapping"):
                if len(document["register_permutation"][field]) < permutation_rules[field]["minItems"]:
                    raise ValueError(field)
            evidence_rules = proven_rules["evidence_binding"]["properties"]
            for field in ("context_sha256", "data_report_sha256", "physical_relocation_receipt"):
                if "$ref" not in evidence_rules[field] or document["evidence_binding"][field] is None:
                    raise ValueError(field)

        mutations = (
            ("bindings", lambda row: row.update(bindings=[])),
            ("mapping", lambda row: row["register_permutation"].update(mapping=[])),
            ("changed_mapping", lambda row: row["register_permutation"].update(changed_mapping=[])),
            ("closed_residual_rows", lambda row: row.update(closed_residual_rows=[])),
            ("context_sha256", lambda row: row["evidence_binding"].update(context_sha256=None)),
            ("data_report_sha256", lambda row: row["evidence_binding"].update(data_report_sha256=None)),
            ("physical_relocation_receipt", lambda row: row["evidence_binding"].update(physical_relocation_receipt=None)),
        )
        fixture_builders = (
            ("v1", self.fixture, "VOLATILE_OWNER_CAUSAL_JOIN_V1.schema.json"),
            ("v2", self.enriched_fixture, "VOLATILE_OWNER_CAUSAL_JOIN_V2.schema.json"),
        )
        for version, fixture_builder, schema_name in fixture_builders:
            focus_value, span_value, graph_value, context_value = fixture_builder()
            valid_gate = self.build_join(focus_value, span_value, graph_value, context_value)
            self.assertEqual(valid_gate["status"], "PROVEN")
            schema = json.loads(Path(reducer.__file__).with_name(schema_name).read_text(encoding="utf-8"))
            enforce_proven_gate(schema, valid_gate)
            for label, mutate in mutations:
                with self.subTest(version=version, label=label):
                    adversarial = copy.deepcopy(valid_gate)
                    mutate(adversarial)
                    with self.assertRaisesRegex(ValueError, label):
                        enforce_proven_gate(schema, adversarial)

    def test_two_owner_bindings_may_close_distinct_positions_of_one_fmuls(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        self._replace_focus_rows(
            focus_value,
            [_instruction(10, "fmuls f3, f5, f4")],
            [_instruction(10, "fmuls f3, f4, f5")],
        )
        facts = graph_value["volatile_owner_facts"]["owner_facts"]
        facts[0].update(vreg="f32", final_color=4, physical_register="f4")
        facts[1].update(vreg="f33", final_color=5, physical_register="f5")
        self._reseal_graph(graph_value)
        context_value["focus"]["artifact_sha256"] = focus_value["artifact_sha256"]
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        context_value["residual_row_bindings"] = [
            {"row_id": "residual-000000", "focus_row_index": 10, "candidate_operand_index": 1, "captured_role": "destination", "semantic_role": "lhs"},
            {"row_id": "residual-000001", "focus_row_index": 10, "candidate_operand_index": 2, "captured_role": "destination", "semantic_role": "rhs"},
        ]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "PROVEN")
        self.assertEqual([row["candidate_operand_index"] for row in result["bindings"]], [1, 2])

    def test_duplicate_register_occurrence_uses_explicit_lhax_position(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        self._replace_focus_rows(
            focus_value,
            [_instruction(10, "lhax r3, r5, r4"), _instruction(11, "mr r4, r6")],
            [_instruction(10, "lhax r3, r4, r4"), _instruction(11, "mr r5, r6")],
        )
        facts = graph_value["volatile_owner_facts"]["owner_facts"]
        facts[0].update(vreg="r32", final_color=4, physical_register="r4")
        facts[1].update(vreg="r33", final_color=5, physical_register="r5")
        self._reseal_graph(graph_value)
        context_value["focus"]["artifact_sha256"] = focus_value["artifact_sha256"]
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        context_value["residual_row_bindings"][0]["candidate_operand_index"] = 1
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "PROVEN")
        self.assertIn({"candidate": "r4", "target": "r5"}, result["register_permutation"]["changed_mapping"])

    def test_ambiguous_join_is_unknown(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        section = graph_value["volatile_owner_facts"]
        section["owner_facts"][0]["classification"] = "UNKNOWN_AMBIGUOUS_MACHINE_EDGE"
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertTrue(any("chain_not_unique" in blocker for blocker in result["blockers"]))

    def test_missing_def_use_is_unknown(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        section = graph_value["volatile_owner_facts"]
        section["owner_facts"][0].update(vreg=None, def_id=None, use_ids=[], classification="UNKNOWN_MISSING_VREG_EDGE")
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        self.assertEqual(self.build_join(focus_value, span_value, graph_value, context_value)["status"], "UNKNOWN")

    def test_extra_graph_row_is_unknown(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        section = graph_value["volatile_owner_facts"]
        section["closed_residual_rows"].append({"row_id": "residual-000002", "ordinal": 0, "kind": 2, "owner_fact_id": "owner-fact-000002", "required_operand_roles": ["destination"]})
        section["owner_facts"].append(_fact(2, "r5", "r34"))
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        self.assertEqual(self.build_join(focus_value, span_value, graph_value, context_value)["status"], "UNKNOWN")

    def test_hash_mismatch_is_rejected(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            paths = {"focus": root / "focus.json", "span": root / "span.json", "graph": root / "graph.json", "context": root / "context.json"}
            for key, value in (("focus", focus_value), ("span", span_value), ("graph", graph_value)):
                paths[key].write_text(json.dumps(value), encoding="utf-8")
            context_value["focus"]["file_sha256"] = "00" * 32
            context_value["source_span_manifest"]["file_sha256"] = hashlib.sha256(paths["span"].read_bytes()).hexdigest()
            context_value["ownership_failure_graph"]["file_sha256"] = hashlib.sha256(paths["graph"].read_bytes()).hexdigest()
            self._reseal_context(context_value)
            paths["context"].write_text(json.dumps(context_value), encoding="utf-8")
            with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "SHA-256 mismatch"):
                reducer.build_from_paths(paths["context"], paths["focus"], paths["span"], paths["graph"], context_value["context_sha256"])

    def test_path_builder_verifies_all_bound_files_and_proves(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        paths = {"focus": self.root / "focus.json", "span": self.root / "span.json", "graph": self.root / "graph.json", "context": self.root / "context.json"}
        for key, value in (("focus", focus_value), ("span", span_value), ("graph", graph_value)):
            paths[key].write_text(json.dumps(value), encoding="utf-8")
        context_value["focus"]["file_sha256"] = hashlib.sha256(paths["focus"].read_bytes()).hexdigest()
        context_value["source_span_manifest"]["file_sha256"] = hashlib.sha256(paths["span"].read_bytes()).hexdigest()
        context_value["ownership_failure_graph"]["file_sha256"] = hashlib.sha256(paths["graph"].read_bytes()).hexdigest()
        self._reseal_context(context_value)
        paths["context"].write_text(json.dumps(context_value), encoding="utf-8")
        result = reducer.build_from_paths(paths["context"], paths["focus"], paths["span"], paths["graph"], context_value["context_sha256"])
        self.assertEqual(result["status"], "PROVEN")

    def test_external_context_anchor_is_required_and_must_match(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        paths = {"focus": self.root / "anchor-focus.json", "span": self.root / "anchor-span.json", "graph": self.root / "anchor-graph.json", "context": self.root / "anchor-context.json"}
        for key, value in (("focus", focus_value), ("span", span_value), ("graph", graph_value)):
            paths[key].write_text(json.dumps(value), encoding="utf-8")
        context_value["focus"]["file_sha256"] = hashlib.sha256(paths["focus"].read_bytes()).hexdigest()
        context_value["source_span_manifest"]["file_sha256"] = hashlib.sha256(paths["span"].read_bytes()).hexdigest()
        context_value["ownership_failure_graph"]["file_sha256"] = hashlib.sha256(paths["graph"].read_bytes()).hexdigest()
        self._reseal_context(context_value)
        paths["context"].write_text(json.dumps(context_value), encoding="utf-8")
        with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "caller-supplied trust anchor"):
            reducer.build_from_paths(paths["context"], paths["focus"], paths["span"], paths["graph"], "99" * 32)
        context_sha_action = next(action for action in reducer._parser()._actions if action.dest == "context_sha256")
        self.assertTrue(context_sha_action.required)

    def test_report_and_candidate_object_cross_mismatches_fail_closed(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        bad_report = copy.deepcopy(context_value)
        bad_report["strict_report_sha256"] = "99" * 32
        self._reseal_context(bad_report)
        with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "strict-report identity"):
            self.build_join(focus_value, span_value, graph_value, bad_report)
        bad_object = copy.deepcopy(context_value)
        bad_object["candidate_object"]["sha256"] = "98" * 32
        self._reseal_context(bad_object)
        result = self.build_join(focus_value, span_value, graph_value, bad_object)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("graph_candidate_object_identity_mismatch", result["blockers"])

    def test_context_self_digest_tamper_is_rejected(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        context_value["function"] = "fn_80005678"
        with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "context self-digest mismatch"):
            self.build_join(focus_value, span_value, graph_value, context_value)

    def test_direct_api_rejects_context_substitution_without_matching_external_anchor(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        substituted = copy.deepcopy(context_value)
        substituted["source_class_allowlist"] = ["substituted_owner_class"]
        substituted["source_class_hypotheses"] = [
            {"source_class": "substituted_owner_class", "row_ids": ["residual-000000", "residual-000001"]}
        ]
        self._reseal_context(substituted)
        with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "caller-supplied trust anchor"):
            reducer.build_join(
                focus_value,
                span_value,
                graph_value,
                substituted,
                context_value["context_sha256"],
            )

    def test_object_file_bytes_must_still_match_context(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        (self.root / "target.o").write_bytes(b"X" * 16)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("target_object_file_identity_mismatch", result["blockers"])

    def test_missing_data_report_binding_and_channel_are_unknown(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        del focus_value["channels"]["data"]
        focus_value.pop("artifact_sha256")
        _seal(focus_value, "artifact_sha256")
        context_value["focus"]["artifact_sha256"] = focus_value["artifact_sha256"]
        del context_value["data_report_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("data_channel_missing", result["blockers"])
        self.assertIn("data_report_binding_missing", result["blockers"])

    def test_physical_receipt_must_be_exact_and_file_hash_bound(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        focus_value["physical_relocations"]["status"] = "mismatch"
        focus_value.pop("artifact_sha256")
        _seal(focus_value, "artifact_sha256")
        context_value["focus"]["artifact_sha256"] = focus_value["artifact_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("physical_relocation_receipt_not_exact_or_hash_bound", result["blockers"])
        (self.root / "physical.json").write_text("{}", encoding="utf-8")
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertIn("physical_relocation_receipt_file_hash_mismatch", result["blockers"])

    def test_register_plus_immediate_residual_is_unknown(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        self._replace_focus_rows(
            focus_value,
            [_instruction(10, "addi r4, r6, 2"), _instruction(11, "add r3, r6, r7")],
            [_instruction(10, "addi r3, r6, 1"), _instruction(11, "add r4, r6, r7")],
        )
        context_value["focus"]["artifact_sha256"] = focus_value["artifact_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("row_10_instruction_skeleton_differs_beyond_registers", result["blockers"])

    def test_pcode_and_ig_tokens_must_match_capture_session(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        fact = graph_value["volatile_owner_facts"]["owner_facts"][0]
        fact["pcode_id"] = "pcode-session-0000000000000043-000000"
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "not session-bound"):
            self.build_join(focus_value, span_value, graph_value, context_value)

    def test_fact_definition_and_use_tokens_must_be_canonical(self) -> None:
        mutations = (("fact_id", "owner-fact-1"), ("def_id", "def-session-000001"), ("use_ids", ["use-session-000001"]))
        for field, value in mutations:
            with self.subTest(field=field):
                focus_value, span_value, graph_value, context_value = self.fixture()
                fact = graph_value["volatile_owner_facts"]["owner_facts"][0]
                fact[field] = value
                if field == "fact_id":
                    graph_value["volatile_owner_facts"]["closed_residual_rows"][0]["owner_fact_id"] = value
                    graph_value["volatile_owner_facts"]["object_identity_bindings"][0]["fact_id"] = value
                self._reseal_graph(graph_value)
                context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
                self._reseal_context(context_value)
                with self.assertRaises(reducer.VolatileOwnerJoinInputError):
                    self.build_join(focus_value, span_value, graph_value, context_value)

    def test_hidden_unknown_object_binding_cannot_prove(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        binding = graph_value["volatile_owner_facts"]["object_identity_bindings"][0]
        binding.clear()
        binding.update(fact_id="owner-fact-000000", status="UNKNOWN", hidden_owner_token=f"hidden-ig-{SESSION}-000000")
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertTrue(any("object_identity_not_present" in blocker for blocker in result["blockers"]))

    def test_event_and_object_tokens_must_be_canonical_and_same_session(self) -> None:
        for case in ("event", "present", "hidden"):
            with self.subTest(case=case):
                focus_value, span_value, graph_value, context_value = self.fixture()
                section = graph_value["volatile_owner_facts"]
                if case == "event":
                    section["raw_event_hashes"][0]["event_id"] = "session-0000000000000043-e000000"
                elif case == "present":
                    section["object_identity_bindings"][0]["object_token"] = "local-session-0000000000000043-000000"
                else:
                    binding = section["object_identity_bindings"][0]
                    binding.clear()
                    binding.update(fact_id="owner-fact-000000", status="UNKNOWN", hidden_owner_token="hidden-ig-session-0000000000000043-000000")
                self._reseal_graph(graph_value)
                context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
                self._reseal_context(context_value)
                with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "session-bound|same-session"):
                    self.build_join(focus_value, span_value, graph_value, context_value)

    def test_pointer_like_fact_token_is_rejected(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        graph_value["volatile_owner_facts"]["owner_facts"][0]["pcode_id"] = "pcode-0xdeadbeef"
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        with self.assertRaisesRegex(reducer.VolatileOwnerJoinInputError, "pointer-free|pointer-like"):
            self.build_join(focus_value, span_value, graph_value, context_value)

    def test_missing_row_breaks_exact_closure(self) -> None:
        focus_value, span_value, graph_value, context_value = self.fixture()
        section = graph_value["volatile_owner_facts"]
        section["closed_residual_rows"].pop()
        section["owner_facts"].pop()
        self._reseal_graph(graph_value)
        context_value["ownership_failure_graph"]["failure_graph_sha256"] = graph_value["failure_graph_sha256"]
        self._reseal_context(context_value)
        result = self.build_join(focus_value, span_value, graph_value, context_value)
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertIn("graph_rows_do_not_exactly_close_focus_residual", result["blockers"])

    @staticmethod
    def _reseal_graph(graph_value: dict[str, object]) -> None:
        section = graph_value["volatile_owner_facts"]
        section.pop("volatile_owner_facts_sha256", None)
        _seal(section, "volatile_owner_facts_sha256")
        graph_value.pop("failure_graph_sha256", None)
        _seal(graph_value, "failure_graph_sha256")

    @staticmethod
    def _reseal_context(context_value: dict[str, object]) -> None:
        context_value.pop("context_sha256", None)
        _seal(context_value, "context_sha256")

    @staticmethod
    def _replace_focus_rows(focus_value: dict[str, object], target_rows: list[dict[str, object]], candidate_rows: list[dict[str, object]]) -> None:
        for channel in ("strict", "data"):
            focus_value["channels"][channel]["target"] = _side(copy.deepcopy(target_rows))
            focus_value["channels"][channel]["candidate"] = _side(copy.deepcopy(candidate_rows))
            focus_value["channels"][channel]["metric"]["diff_rows"] = len(target_rows)
            focus_value["channels"][channel]["metric"]["diff_kinds"] = {"DIFF_ARG_MISMATCH": len(target_rows)}
        focus_value.pop("artifact_sha256", None)
        _seal(focus_value, "artifact_sha256")


if __name__ == "__main__":
    unittest.main()
