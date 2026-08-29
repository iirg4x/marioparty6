from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from tools import hidden_arithmetic_owner_join as join


def seal(value: dict, field: str) -> dict:
    value.pop(field, None)
    value[field] = join.canonical_sha256(value)
    return value


class Evidence:
    SESSION = "session-synthetic0001"
    FUNCTION = "SyntheticArithmeticOwner"
    PROCESS = 4242

    def __init__(self, root: Path):
        self.root = root
        self.source = self._bytes("candidate.c", b"void f(void) { float speed; }\n")
        self.trace_object = self._bytes("trace.o", b"trace-object")
        self.target_object = self._bytes("target.o", b"target-object")
        self.production_object = self._bytes("candidate.o", b"production-object")
        self.compiler_sha = "1" * 64
        self.strict_report = self._json("strict.json", {"left": {"sections": []}, "right": {"sections": []}})
        self.data_report = self._json("data.json", {"left": {"sections": []}, "right": {"sections": []}})
        self.strict_sha, self.data_sha = join.file_sha256(self.strict_report), join.file_sha256(self.data_report)
        self.machine_events = self._machine_events()
        self.pcode_events = self._pcode_events()
        self.envelope = seal({"context": {"session_id": self.SESSION}, "inventory": {"arguments": [], "locals": [{"token": f"local-{self.SESSION}-speed", "name": "speed"}]}}, "envelope_sha256")
        self.graph = self._graph()
        self.hook = self._hook()
        self.partial = self._partial()
        self.focus = self._focus()
        self.receipt = self._receipt()
        self.context = self._context()

    def _bytes(self, name: str, payload: bytes) -> Path:
        path = self.root / name; path.write_bytes(payload); return path

    def _json(self, name: str, payload: dict) -> Path:
        path = self.root / name; path.write_text(json.dumps(payload, sort_keys=True), encoding="utf-8"); return path

    def _jsonl(self, name: str, rows: list[dict]) -> Path:
        path = self.root / name; path.write_text("".join(json.dumps(row, sort_keys=True) + "\n" for row in rows), encoding="utf-8"); return path

    def _event(self, **values) -> dict:
        return {"schema": join.EVENT_SCHEMA, "session_id": self.SESSION, "function": self.FUNCTION, "process_id": self.PROCESS, **values}

    def _machine_events(self) -> list[dict]:
        rows = []
        for producer, site, pprod, pcons, offset in ((19, 20, "prod1", "cons1", 0x10), (39, 40, "prod2", "cons2", 0x14)):
            word = 0xC0010000 | offset
            rows.append(self._event(event_id=f"m{producer}", sequence=producer * 10, event_kind="machine_emission", hook_id="gc27_machine_emit", lane="pcode", status="CAPTURED", instruction_index=producer, mnemonic="lfs", memory_op="load", pcode_token=pprod, ppc_bytes=f"{word:08x}", ppc_word=word, registers={"base": "r1", "data": "f0"}))
            word = 0xEC1F0032
            rows.append(self._event(event_id=f"m{site}", sequence=site * 10 + 9, event_kind="machine_emission", hook_id="gc27_machine_emit", lane="pcode", status="UNKNOWN", reason="ambiguous reaching definition", instruction_index=site, mnemonic="fmuls", arithmetic_op="multiply", pcode_token=pcons, ppc_bytes=f"{word:08x}", ppc_word=word, registers={"destination": "f0", "source_a": "f31", "source_b": "f0"}))
        return rows

    def _pc(self, event_id: str, token: str, ordinal: int, color: int, ig: str, **owner) -> dict:
        base = 100 if token.startswith("prod") else 1000
        number = int(token[-1])
        return self._event(event_id=event_id, sequence=base * number + ordinal, event_kind="pcode_capture", hook_id="pcode_color_post", stage="pcode_color_post", lane="pcode", status="CAPTURED", confirmed=True, pcode_token=token, operand_count=3, operand_ordinal=ordinal, operand_bank="FPR", final_color=color, ig_token=ig, operand_index=999 + ordinal, **owner)

    def _pcode_events(self) -> list[dict]:
        rows = []
        for n in (1, 2):
            hidden, hig = f"hidden-{self.SESSION}-{n}", f"ig-{self.SESSION}-hidden-{n}"
            rows.append(self._pc(f"p{n}p", f"prod{n}", 0, 0, hig, hidden_owner_token=hidden))
            rows.extend([self._pc(f"p{n}d", f"cons{n}", 0, 0, f"ig-{self.SESSION}-dest-{n}", hidden_owner_token=f"hidden-{self.SESSION}-dest-{n}"), self._pc(f"p{n}a", f"cons{n}", 1, 31, f"ig-{self.SESSION}-speed", object_token=f"local-{self.SESSION}-speed"), self._pc(f"p{n}b", f"cons{n}", 2, 0, hig, hidden_owner_token=hidden)])
        return rows

    def _graph(self) -> dict:
        sites = []
        for n, site in ((1, 20), (2, 40)):
            operands = []
            for event in [row for row in self.pcode_events if row.get("pcode_token") == f"cons{n}"]:
                identity = {"status": "PRESENT", "object_token": event["object_token"]} if "object_token" in event else {"status": "UNKNOWN", "hidden_owner_token": event["hidden_owner_token"]}
                operands.append({"event_id": event["event_id"], "ig_token": event["ig_token"], "final_color": event["final_color"], "operand_ordinal": event["operand_ordinal"], "owner_identity": identity})
            sites.append({"instruction_index": site, "pcode_token": f"cons{n}", "ppc_bytes": "ec1f0032", "pcode_operands": operands})
        return seal({"schema": "ownership_failure_graph/v1", "session_id": self.SESSION, "function": self.FUNCTION, "authority_advanced": False, "unresolved_machine_sites": sites}, "failure_graph_sha256")

    def _hook(self) -> dict:
        return seal({"schema": "mwcc_capsule_same_session_hook_validation/v1", "status": "AUTHENTICATED_PARTIAL_CAPTURE", "authority_advanced": False, "board_admission": False, "session_id": self.SESSION, "function": self.FUNCTION, "source": {"sha256": join.file_sha256(self.source)}, "compiler": {"sha256": self.compiler_sha}, "compiler_owned_object": {"sha256": join.file_sha256(self.trace_object)}, "hooks": [{"id": "pcode_color_post", "lane": "pcode"}, {"id": "gc27_machine_emit", "lane": "pcode"}]}, "receipt_sha256")

    @staticmethod
    def _descriptor(path: Path) -> dict:
        return {"path": str(path.resolve()), "sha256": join.file_sha256(path), "size": path.stat().st_size}

    def _partial(self) -> dict:
        envelope = self._json("envelope.json", self.envelope); graph = self._json("graph.json", self.graph); hook = self._json("hook.json", self.hook); machine = self._jsonl("machine.jsonl", self.machine_events); pcode = self._jsonl("pcode.jsonl", self.pcode_events)
        trust_fields = {"function": self.FUNCTION, "source_sha256": join.file_sha256(self.source), "compiler_sha256": self.compiler_sha}
        return seal({"schema": join.PARTIAL_SCHEMA, "status": "UNKNOWN", "diagnostic_only": True, "authority_advanced": False, "board_admission": False, "context": {"function": self.FUNCTION, "session_id": self.SESSION, "process_id": self.PROCESS, "source": {"path": str(self.source.resolve()), "sha256": join.file_sha256(self.source)}, "compiler": {"sha256": self.compiler_sha}}, "trust_root": {"fields": trust_fields, "binding_sha256": join.canonical_sha256(trust_fields)}, "compiler_owned_object": self._descriptor(self.trace_object), "artifacts": {"candidate_envelope": self._descriptor(envelope), "failure_graph": self._descriptor(graph), "hook_validation": self._descriptor(hook), "machine_events": self._descriptor(machine), "pcode_events": self._descriptor(pcode)}}, "manifest_sha256")

    @staticmethod
    def _row(index: int, kind: str | None, formatted: str | None) -> dict:
        row = {"index": index}
        if kind is not None: row["diff_kind"] = kind
        if formatted is not None: row["instruction"] = {"formatted": formatted}
        return row

    def _focus(self) -> dict:
        target, candidate = [self._row(0, None, "stwu r1, -0x20(r1)")], [self._row(0, None, "stwu r1, -0x20(r1)")]
        spec = [(18, "DIFF_ARG_MISMATCH", "lfs f0, 0x10(r1)", "DIFF_ARG_MISMATCH", "lfs f1, 0x10(r1)"), (19, "DIFF_INSERT", None, "DIFF_INSERT", "lfs f0, 0x14(r1)"), (20, "DIFF_ARG_MISMATCH", "fmuls f1, f31, f0", "DIFF_ARG_MISMATCH", "fmuls f0, f31, f0"), (21, "DIFF_DELETE", "lfs f0, 0x14(r1)", "DIFF_DELETE", None), (38, "DIFF_ARG_MISMATCH", "lfs f0, 0x18(r1)", "DIFF_ARG_MISMATCH", "lfs f1, 0x18(r1)"), (39, "DIFF_INSERT", None, "DIFF_INSERT", "lfs f0, 0x1c(r1)"), (40, "DIFF_ARG_MISMATCH", "fmuls f1, f31, f0", "DIFF_ARG_MISMATCH", "fmuls f0, f31, f0"), (41, "DIFF_DELETE", "lfs f0, 0x1c(r1)", "DIFF_DELETE", None)]
        self.residual = []
        for i, tk, tf, ck, cf in spec:
            target.append(self._row(i, tk, tf)); candidate.append(self._row(i, ck, cf)); self.residual.append({"index": i, "target_diff_kind": tk, "target_formatted": tf, "candidate_diff_kind": ck, "candidate_formatted": cf})
        metric = {"target_size": 168, "candidate_size": 168, "diff_rows": 8}
        exact_ids = ["SiblingA", "SiblingB"]
        siblings = {"all_sibling_metric_sha256": "a" * 64, "exact_identities": exact_ids, "exact_identity_sha256": join.canonical_sha256(exact_ids), "exact_sibling_count": 2}
        sections = {"target": [{"name": name, "size": "4", "match_percent": 100.0} for name in (".data", ".bss", ".sdata2")], "candidate": [{"name": name, "size": "4"} for name in (".data", ".bss", ".sdata2")]}
        report = lambda path, digest: {"path": str(path.resolve()), "sha256": digest, "size_bytes": path.stat().st_size}
        return seal({"schema": "focus_symbol_report/v1", "schema_version": 1, "function": self.FUNCTION, "authority_advanced": False, "input_binding": {"strict_report": report(self.strict_report, self.strict_sha), "data_report": report(self.data_report, self.data_sha)}, "channels": {"strict": {"metric": metric, "target": {"rows": target}, "candidate": {"rows": candidate}, "protected_siblings": siblings, "sections": sections}, "data": {"metric": metric, "target": {"rows": target[1:]}, "candidate": {"rows": candidate[1:]}, "protected_siblings": copy.deepcopy(siblings)}}}, "artifact_sha256")

    def _receipt(self) -> dict:
        row = {"offset": 4, "section_offset": 4, "type": 10, "type_name": "R_PPC_REL24", "addend": 0, "effective_target": {"mode": "undefined_symbol", "name": "callee", "kind": "STT_NOTYPE"}}
        target = {"object": {"sha256": join.file_sha256(self.target_object)}, "physical_relocation_count": 1, "physical_relocations": [row]}
        candidate = {"object": {"sha256": join.file_sha256(self.production_object)}, "physical_relocation_count": 1, "physical_relocations": [copy.deepcopy(row)]}
        return seal({"schema": "mp6_physical_relocation_receipt/v1", "authority_advanced": False, "source_patch_emitted": False, "focus": {"function": self.FUNCTION}, "focus_artifact": {"sha256": join.file_sha256(self.root / "focus.json") if (self.root / "focus.json").exists() else "0" * 64}, "strict_report": {"sha256": self.strict_sha}, "target": target, "candidate": candidate, "physical_relocations_exact": True, "physical_relocation_differences": []}, "receipt_sha256")

    def _context(self) -> dict:
        partial = self._json("partial.json", self.partial); focus = self._json("focus.json", self.focus)
        self.receipt["focus_artifact"]["sha256"] = join.file_sha256(focus); seal(self.receipt, "receipt_sha256")
        receipt = self._json("physical.json", self.receipt)
        source_bytes = self.source.read_bytes(); start = source_bytes.index(b"float speed"); end = start + len(b"float speed")
        desc = self._descriptor
        controls = [{"control_id": "direct", "source_sha256": "4" * 64, "object_sha256": "5" * 64, "source_class": "direct_live_expression_boundary", "boundary_kind": "DIRECT_EXPRESSION_ARITHMETIC_BOUNDARY", "outcome": "nonexact"}, {"control_id": "spill", "source_sha256": "6" * 64, "object_sha256": "7" * 64, "source_class": "spill_reload_boundary", "boundary_kind": "SPILL_RELOAD_BOUNDARY", "outcome": "nonexact"}]
        return seal({"schema": join.CONTEXT_SCHEMA, "partial_evidence_path": str(partial.resolve()), "partial_evidence_sha256": join.file_sha256(partial), "function": self.FUNCTION, "session_id": self.SESSION, "source_sha256": join.file_sha256(self.source), "compiler_sha256": self.compiler_sha, "trace_candidate_object_sha256": join.file_sha256(self.trace_object), "failure_graph_sha256": join.file_sha256(self.root / "graph.json"), "machine_sites": [20, 40], "production_row_groups": [{"trace_machine_site": 20, "production_fmuls_index": 20, "residual_indices": [18, 19, 20, 21]}, {"trace_machine_site": 40, "production_fmuls_index": 40, "residual_indices": [38, 39, 40, 41]}], "source_owners": [{"name": "speed", "object_token": f"local-{self.SESSION}-speed", "byte_start": start, "byte_end": end, "span_sha256": hashlib.sha256(source_bytes[start:end]).hexdigest()}], "source_class_allowlist": ["named_owner_chronology_x_reloaded_value_boundary", "named_owner_chronology_x_direct_expression_boundary"], "rejected_controls": controls, "target_object": desc(self.target_object), "production_candidate_object": desc(self.production_object), "focus_artifact": {**desc(focus), "artifact_sha256": self.focus["artifact_sha256"]}, "strict_report_sha256": self.strict_sha, "data_report_sha256": self.data_sha, "physical_relocation_receipt": {**desc(receipt), "receipt_sha256": self.receipt["receipt_sha256"]}, "physical_relocation_count": 1, "residual_rows": self.residual}, "context_sha256")

    def analyze(self): return join.analyze(self.context)
    def reseal(self): seal(self.context, "context_sha256")

    def rewrite_partial(self):
        seal(self.partial, "manifest_sha256"); path = Path(self.context["partial_evidence_path"]); path.write_text(json.dumps(self.partial, sort_keys=True), encoding="utf-8"); self.context["partial_evidence_sha256"] = join.file_sha256(path); self.reseal()

    def rewrite_events(self, kind: str):
        rows = self.machine_events if kind == "machine" else self.pcode_events; path = self._jsonl(f"{kind}.jsonl", rows); self.partial["artifacts"][f"{kind}_events"] = self._descriptor(path); self.rewrite_partial()

    def rewrite_focus(self):
        seal(self.focus, "artifact_sha256"); path = Path(self.context["focus_artifact"]["path"]); path.write_text(json.dumps(self.focus, sort_keys=True), encoding="utf-8"); self.context["focus_artifact"].update(sha256=join.file_sha256(path), size=path.stat().st_size, artifact_sha256=self.focus["artifact_sha256"]); self.receipt["focus_artifact"]["sha256"] = join.file_sha256(path); self.rewrite_receipt(); self.reseal()

    def rewrite_receipt(self):
        seal(self.receipt, "receipt_sha256"); path = Path(self.context["physical_relocation_receipt"]["path"]); path.write_text(json.dumps(self.receipt, sort_keys=True), encoding="utf-8"); self.context["physical_relocation_receipt"].update(sha256=join.file_sha256(path), size=path.stat().st_size, receipt_sha256=self.receipt["receipt_sha256"]); self.reseal()


class HiddenArithmeticOwnerJoinTests(unittest.TestCase):
    def setUp(self): self.temp = tempfile.TemporaryDirectory(); self.e = Evidence(Path(self.temp.name))
    def tearDown(self): self.temp.cleanup()

    def test_complete_unique_join_ranks_derived_class(self):
        result = self.e.analyze(); self.assertEqual(result["status"], "RANKED_SOURCE_CLASS"); self.assertEqual(result["ranked_source_classes"][0]["source_class"], "named_owner_chronology_x_reloaded_value_boundary"); self.assertEqual(result["ranked_source_classes"][0]["predicted_scope"]["row_count"], 8)
        for chain in result["chains"]:
            owner = chain["named_owner"]; self.assertEqual(owner["allocator_identity"]["kind"], "IG_NODE"); self.assertEqual(owner["edge_mode"], "DIRECT_PCODE_OBJECT_TO_IG_COLOR"); self.assertIsNone(owner["legacy_vreg_id"])

    def test_operand_index_mutation_is_ignored(self):
        baseline = self.e.analyze()
        for row in self.e.pcode_events: row["operand_index"] += 100000
        self.e.rewrite_events("pcode"); changed = self.e.analyze()
        self.assertEqual(changed["chains"], baseline["chains"])
        self.assertEqual(changed["ranked_source_classes"], baseline["ranked_source_classes"])

    def test_duplicate_operand_fails(self): self.e.pcode_events.append(copy.deepcopy(self.e.pcode_events[1])); self.e.rewrite_events("pcode"); self._fails("complete three-operand")
    def test_color_conflict_fails(self): self.e.pcode_events[2]["final_color"] = 30; self.e.rewrite_events("pcode"); self._fails("color conflicts")
    def test_non_fmuls_fails(self): self.e.machine_events[1]["mnemonic"] = "fadds"; self.e.rewrite_events("machine"); self._fails("not a sealed fmuls")
    def test_nonunique_producer_fails(self): self.e.pcode_events.append(copy.deepcopy(self.e.pcode_events[0])); self.e.rewrite_events("pcode"); self._fails("no unique PCode definition")
    def test_late_producer_fails(self): self.e.machine_events[0]["instruction_index"] = 21; self.e.rewrite_events("machine"); self._fails("chronology")
    def test_producer_color_conflict_fails(self): self.e.pcode_events[0]["final_color"] = 1; self.e.rewrite_events("pcode"); self._fails("producer color conflicts")

    def test_nested_fmuls_unknown_producer_output_is_sealed(self):
        for producer in (self.e.machine_events[0], self.e.machine_events[2]):
            producer.update(status="UNKNOWN", reason="ambiguous reaching definition", mnemonic="fmuls", arithmetic_op="multiply")
            producer.pop("memory_op", None)
            producer["ppc_bytes"] = "ec000032"; producer["ppc_word"] = int(producer["ppc_bytes"], 16)
            producer["registers"] = {"destination": "f0", "source_a": "f0", "source_b": "f0"}
        self.e.rewrite_events("machine")
        result = self.e.analyze()
        self.assertEqual(result["status"], "RANKED_SOURCE_CLASS")
        definition = result["chains"][0]["hidden_definition"]
        self.assertEqual(definition["machine_status"], "UNKNOWN")
        self.assertFalse(definition["producer_input_ownership_claimed"])

    def test_unknown_producer_other_reason_fails(self):
        producer = self.e.machine_events[0]
        producer.update(status="UNKNOWN", reason="debugger timeout")
        self.e.rewrite_events("machine"); self._fails("producer machine event is not authenticated")

    def test_unknown_producer_missing_reason_fails(self):
        producer = self.e.machine_events[0]
        producer["status"] = "UNKNOWN"; producer.pop("reason", None)
        self.e.rewrite_events("machine"); self._fails("producer machine event is not authenticated")
    def test_session_drift_fails(self): self.e.context["session_id"] = "session-drift"; self.e.reseal(); self._fails("session mismatch")
    def test_source_hash_drift_fails(self): self.e.context["source_sha256"] = "8" * 64; self.e.reseal(); self._fails("source mismatch")
    def test_focus_hash_drift_fails(self): self.e.context["focus_artifact"]["sha256"] = "8" * 64; self.e.reseal(); self._fails("focus_artifact identity mismatch")
    def test_focus_residual_drift_fails(self): self.e.context["residual_rows"][0]["candidate_formatted"] = "fmuls f2, f31, f0"; self.e.reseal(); self._fails("residual row descriptors drifted")

    def test_focus_local_relocation_symbol_ordinals_are_not_semantic(self):
        target_row = self.e.focus["channels"]["strict"]["target"]["rows"][0]
        candidate_row = self.e.focus["channels"]["strict"]["candidate"]["rows"][0]
        target, candidate = target_row["instruction"], candidate_row["instruction"]
        target["relocation"] = {"type": 10, "type_name": "R_PPC_REL24", "target_symbol": 111}
        candidate["relocation"] = {"type": 10, "type_name": "R_PPC_REL24", "target_symbol": 127}
        target["size"] = candidate["size"] = 4
        target_row["parts_sha256"] = candidate_row["parts_sha256"] = "b" * 64
        self.e.rewrite_focus()
        self.assertEqual(self.e.analyze()["status"], "RANKED_SOURCE_CLASS")

    def test_focus_relocation_backed_formatted_aliases_are_not_semantic(self):
        target_row = self.e.focus["channels"]["strict"]["target"]["rows"][0]
        candidate_row = self.e.focus["channels"]["strict"]["candidate"]["rows"][0]
        target, candidate = target_row["instruction"], candidate_row["instruction"]
        target.update(formatted="lfs f28, lbl_802C4100@sda21", relocation={"type": 109, "type_name": "R_PPC_EMB_SDA21", "target_symbol": 29}, size=4)
        candidate.update(formatted="lfs f28, @302@sda21", relocation={"type": 109, "type_name": "R_PPC_EMB_SDA21", "target_symbol": 29}, size=4)
        target_row["parts_sha256"] = candidate_row["parts_sha256"] = "c" * 64
        self.e.rewrite_focus()
        self.assertEqual(self.e.analyze()["status"], "RANKED_SOURCE_CLASS")

    def test_focus_relocation_type_drift_fails(self):
        target_row = self.e.focus["channels"]["strict"]["target"]["rows"][0]
        candidate_row = self.e.focus["channels"]["strict"]["candidate"]["rows"][0]
        target, candidate = target_row["instruction"], candidate_row["instruction"]
        target["relocation"] = {"type": 10, "type_name": "R_PPC_REL24", "target_symbol": 111}
        candidate["relocation"] = {"type": 11, "type_name": "R_PPC_REL14", "target_symbol": 127}
        target["size"] = candidate["size"] = 4
        target_row["parts_sha256"] = candidate_row["parts_sha256"] = "d" * 64
        self.e.rewrite_focus(); self._fails("relocation type drift")

    def test_focus_relocation_backed_opcode_parts_drift_fails(self):
        self._relocation_parts_drift("stfs f28, @302@sda21", "e" * 64)

    def test_focus_relocation_backed_register_parts_drift_fails(self):
        self._relocation_parts_drift("lfs f27, @302@sda21", "e" * 64)

    def test_focus_relocation_backed_digest_only_drift_fails(self):
        self._relocation_parts_drift("lfs f28, @302@sda21", "e" * 64)

    def _relocation_parts_drift(self, candidate_formatted: str, candidate_parts: str):
        target_row = self.e.focus["channels"]["strict"]["target"]["rows"][0]
        candidate_row = self.e.focus["channels"]["strict"]["candidate"]["rows"][0]
        target, candidate = target_row["instruction"], candidate_row["instruction"]
        target.update(formatted="lfs f28, lbl_802C4100@sda21", relocation={"type": 109, "type_name": "R_PPC_EMB_SDA21", "target_symbol": 29}, size=4)
        candidate.update(formatted=candidate_formatted, relocation={"type": 109, "type_name": "R_PPC_EMB_SDA21", "target_symbol": 29}, size=4)
        target_row["parts_sha256"] = "c" * 64
        candidate_row["parts_sha256"] = candidate_parts
        self.e.rewrite_focus(); self._fails("structural parts drift")
    def test_receipt_hash_drift_fails(self): self.e.context["physical_relocation_receipt"]["sha256"] = "9" * 64; self.e.reseal(); self._fails("physical_relocation_receipt identity mismatch")
    def test_insufficient_controls_fails(self): self.e.context["rejected_controls"] = self.e.context["rejected_controls"][:1]; self.e.reseal(); self._fails("competing boundary")
    def test_asserted_answer_field_rejected(self): self.e.context["source_class"] = "named_owner_chronology_x_reloaded_value_boundary"; self.e.reseal(); self._fails("fields are not canonical")

    def test_producer_sequence_after_consumer_fails(self): self.e.pcode_events[0]["sequence"] = 9000; self.e.rewrite_events("pcode"); self._fails("event chronology")

    def test_fmuls_mapping_must_belong_to_group(self): self.e.context["production_row_groups"][0]["production_fmuls_index"] = 40; self.e.reseal(); self._fails("fmuls mappings")

    def test_data_sibling_gate_drift_fails(self): self.e.focus["channels"]["data"]["protected_siblings"]["exact_sibling_count"] = 1; self.e.rewrite_focus(); self._fails("sibling digest/count")

    def test_derived_class_already_measured_fails(self): self.e.context["rejected_controls"][0]["source_class"] = "named_owner_chronology_x_reloaded_value_boundary"; self.e.reseal(); self._fails("already measured")

    def test_physical_offset_mutation_fails(self): self.e.receipt["candidate"]["physical_relocations"][0]["offset"] = 8; self.e.rewrite_receipt(); self._fails("normalized rows differ")

    def test_physical_type_mutation_fails(self): self.e.receipt["candidate"]["physical_relocations"][0]["type"] = 11; self.e.rewrite_receipt(); self._fails("normalized rows differ")

    def test_physical_addend_mutation_fails(self): self.e.receipt["candidate"]["physical_relocations"][0]["addend"] = 4; self.e.rewrite_receipt(); self._fails("normalized rows differ")

    def test_physical_effective_target_mutation_fails(self): self.e.receipt["candidate"]["physical_relocations"][0]["effective_target"]["name"] = "other"; self.e.rewrite_receipt(); self._fails("normalized rows differ")

    def test_physical_count_mutation_fails(self): self.e.receipt["candidate"]["physical_relocation_count"] = 2; self.e.rewrite_receipt(); self._fails("count drift")

    def test_schema_contracts_track_runtime(self):
        context_schema = json.loads(Path("tools/HIDDEN_ARITHMETIC_OWNER_JOIN_CONTEXT_V1.schema.json").read_text())
        output_schema = json.loads(Path("tools/HIDDEN_ARITHMETIC_OWNER_JOIN_V1.schema.json").read_text())
        self.assertEqual(set(context_schema["required"]), set(self.e.context))
        self.assertNotIn("production_gates", context_schema["properties"])
        self.assertIn("boundary_kind", context_schema["$defs"]["rejectedControl"]["required"])
        self.assertEqual(output_schema["properties"]["schema"]["const"], join.OUTPUT_SCHEMA)

    def test_partial_manifest_self_hash_drift_fails(self):
        self.e.partial["manifest_sha256"] = "f" * 64
        path = Path(self.e.context["partial_evidence_path"]); path.write_text(json.dumps(self.e.partial, sort_keys=True), encoding="utf-8")
        self.e.context["partial_evidence_sha256"] = join.file_sha256(path); self.e.reseal(); self._fails("self-hash mismatch")

    def test_envelope_self_hash_drift_fails(self):
        self.e.envelope["envelope_sha256"] = "f" * 64
        path = self.e.root / "envelope.json"; path.write_text(json.dumps(self.e.envelope, sort_keys=True), encoding="utf-8")
        self.e.partial["artifacts"]["candidate_envelope"] = self.e._descriptor(path); self.e.rewrite_partial(); self._fails("self-hash mismatch")

    def test_relocation_difference_fails(self):
        path = Path(self.e.context["physical_relocation_receipt"]["path"]); receipt = copy.deepcopy(self.e.receipt); receipt["physical_relocations_exact"] = False; seal(receipt, "receipt_sha256"); path.write_text(json.dumps(receipt, sort_keys=True), encoding="utf-8"); self.e.context["physical_relocation_receipt"].update(sha256=join.file_sha256(path), size=path.stat().st_size, receipt_sha256=receipt["receipt_sha256"]); self.e.reseal(); self._fails("not exact")

    def _fails(self, pattern: str):
        with self.assertRaisesRegex(join.JoinInputError, pattern): self.e.analyze()


class HiddenArithmeticOwnerJoinCliTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.e = Evidence(Path(self.temp.name))
        build_root = Path(join.__file__).resolve().parent.parent / "build"
        build_root.mkdir(exist_ok=True)
        self.output_temp = tempfile.TemporaryDirectory(dir=build_root)
        self.output_dir = Path(self.output_temp.name)

    def tearDown(self):
        self.output_temp.cleanup(); self.temp.cleanup()

    def context_path(self) -> Path:
        return self.e._json("cli-context.json", self.e.context)

    def test_ranked_cli_atomic_success_and_tool_binding(self):
        output = self.output_dir / "ranked.json"
        self.assertEqual(join.main(["--context", str(self.context_path()), "--output", str(output)]), 0)
        result = json.loads(output.read_text())
        tool = Path(join.__file__).resolve()
        self.assertEqual(result["implementation"], {"path": str(tool), "size": tool.stat().st_size, "sha256": join.file_sha256(tool)})
        self.assertFalse(list(self.output_dir.glob(".ranked.json.*.tmp")))

    def test_unknown_cli_atomic_output_and_exit_two(self):
        self.e.context["source_sha256"] = "8" * 64; self.e.reseal()
        output = self.output_dir / "unknown.json"
        self.assertEqual(join.main(["--context", str(self.context_path()), "--output", str(output)]), 2)
        result = json.loads(output.read_text())
        self.assertEqual(result["status"], "UNKNOWN")
        self.assertEqual(result["implementation"], join.implementation_binding())

    def test_outside_build_rejected_without_output(self):
        output = Path(self.temp.name) / "outside.json"
        self.assertEqual(join.main(["--context", str(self.context_path()), "--output", str(output.resolve())]), 2)
        self.assertFalse(output.exists())

    def test_existing_output_symlink_rejected_when_supported(self):
        target = self.output_dir / "target.json"; target.write_text("old")
        output = self.output_dir / "linked.json"
        try:
            output.symlink_to(target)
        except OSError:
            self.skipTest("symlink creation unavailable")
        self.assertEqual(join.main(["--context", str(self.context_path()), "--output", str(output)]), 2)
        self.assertEqual(target.read_text(), "old")

    def test_symlinked_parent_rejected_when_supported(self):
        real = self.output_dir / "real"; real.mkdir()
        linked = self.output_dir / "linked-parent"
        try:
            linked.symlink_to(real, target_is_directory=True)
        except OSError:
            self.skipTest("directory symlink creation unavailable")
        output = linked / "result.json"
        self.assertEqual(join.main(["--context", str(self.context_path()), "--output", str(output)]), 2)
        self.assertFalse((real / "result.json").exists())

    def test_replace_failure_preserves_existing_and_cleans_temp(self):
        output = self.output_dir / "existing.json"; output.write_text("old")
        with mock.patch.object(join.os, "replace", side_effect=OSError("replace failed")):
            self.assertEqual(join.main(["--context", str(self.context_path()), "--output", str(output)]), 2)
        self.assertEqual(output.read_text(), "old")
        self.assertFalse(list(self.output_dir.glob(".existing.json.*.tmp")))

    def test_write_failure_preserves_existing_and_cleans_temp(self):
        output = self.output_dir / "existing.json"; output.write_text("old")
        with mock.patch.object(join.os, "fsync", side_effect=OSError("fsync failed")):
            self.assertEqual(join.main(["--context", str(self.context_path()), "--output", str(output)]), 2)
        self.assertEqual(output.read_text(), "old")
        self.assertFalse(list(self.output_dir.glob(".existing.json.*.tmp")))


if __name__ == "__main__": unittest.main()
