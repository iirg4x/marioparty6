from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools import dform_scalar_owner_composer as composer


FUNCTION = "ExampleGuideMove"
OWNER = "main:board/example"
FINAL_ROWS = [6, 7, 8, 9, 10]


def _sha(label: str) -> str:
    return hashlib.sha256(label.encode("utf-8")).hexdigest()


def _row(index: int, formatted: str | None, *, diff: str | None = None) -> dict[str, object]:
    row: dict[str, object] = {"index": index}
    if diff:
        row["diff_kind"] = diff
    if formatted is not None:
        row["instruction"] = {
            "address": 0x1000 + (4 * index),
            "formatted": formatted,
            "size": 4,
        }
    return row


def _target_rows(diff_indices: set[int]) -> list[dict[str, object]]:
    formatted = [
        "stwu r1, -0x40(r1)",
        "psq_l f1, 0x0(r3), 0, qr0",
        "lfs f2, 0x8(r3)",
        "psq_st f1, 0x0(r4), 0, qr0",
        "stfs f2, 0x8(r4)",
        "li r3, 1",
        "lfd f1, @pool@sda21",
        None,
        "fsub f1, f0, f1",
        "lfd f0, lbl_POOL@sda21",
        "fmul f0, f1, f0",
        "fmr f28, f1",
        "lfs f29, lbl_SCALE@sda21",
        "bl ExampleConsumer",
    ]
    result = []
    for index, value in enumerate(formatted):
        kind = None
        if index in diff_indices:
            kind = "DIFF_INSERT" if index == 7 else "DIFF_ARG_MISMATCH"
        result.append(_row(index, value, diff=kind))
    return result


def _candidate_rows(role: str, diff_indices: set[int]) -> list[dict[str, object]]:
    rows = _target_rows(set())
    if role == "structural":
        replacements = {
            1: "psq_l f3, 0x0(r3), 0, qr0",
            2: "lfs f4, 0x8(r3)",
            3: "psq_st f3, 0x0(r4), 0, qr0",
            4: "stfs f4, 0x8(r4)",
        }
        for index, value in replacements.items():
            rows[index]["instruction"]["formatted"] = value  # type: ignore[index]
    if role in {"structural", "dform"}:
        rows[11]["instruction"]["formatted"] = "fmr f27, f1"  # type: ignore[index]
        rows[12]["instruction"]["formatted"] = "lfs f30, lbl_SCALE@sda21"  # type: ignore[index]
    if role != "exact":
        rows[6]["instruction"]["formatted"] = "lfd f2, lbl_POOL@sda21"  # type: ignore[index]
        rows[7] = _row(7, "lfd f1, @pool@sda21")
        rows[8]["instruction"]["formatted"] = "fsub f0, f0, f1"  # type: ignore[index]
        rows[9] = _row(9, None)
        rows[10]["instruction"]["formatted"] = "fmul f0, f2, f0"  # type: ignore[index]
    for index in diff_indices:
        rows[index]["diff_kind"] = "DIFF_DELETE" if index == 9 else "DIFF_ARG_MISMATCH"
    return rows


def _protected() -> dict[str, object]:
    identities = ["alpha:0", "beta:4"]
    return {
        "focus_identity_excluded": f"{FUNCTION}:8",
        "function_counts": {"paired": 3},
        "sibling_count": 2,
        "exact_sibling_count": 2,
        "exact_identities": identities,
        "exact_identity_sha256": composer.canonical_sha256(identities),
        "all_sibling_metric_sha256": _sha("all-sibling-metrics"),
    }


def _relocations(role: str, side: str) -> dict[str, object]:
    entries: list[dict[str, object]] = []
    row_indices = [6, 9, 13] if side == "target" else [6, 7, 13]
    for row_index in row_indices:
        diff_kind = None
        if role != "exact" and row_index in FINAL_ROWS:
            diff_kind = "DIFF_ARG_MISMATCH"
        entries.append(
            {
                "row_index": row_index,
                "diff_kind": diff_kind,
                "instruction_address": 0x1000 + (4 * row_index),
                "instruction_formatted": "bl ExampleConsumer" if row_index == 13 else "lfd f1, lbl_POOL@sda21",
                "instruction_size": 4,
                "relocation": {"type_name": "R_PPC_REL24" if row_index == 13 else "R_PPC_EMB_SDA21"},
                "target_symbol_index": row_index,
            }
        )
    return {
        "count": 3,
        "entries": entries,
        "entries_sha256": composer.canonical_sha256(entries),
        "targets": [],
        "pool_dependencies": [],
        "pool_dependency_sha256": composer.canonical_sha256([]),
    }


def _channel(role: str, name: str, target_rows: list[dict[str, object]], candidate_rows: list[dict[str, object]], diff_count: int) -> dict[str, object]:
    exact = role == "exact"
    candidate_size = 92 if role == "structural" else 100
    rows_kind = "all" if name == "strict" else "diff_only"
    if rows_kind == "diff_only":
        target_rows = [row for row in target_rows if row.get("diff_kind")]
        candidate_rows = [row for row in candidate_rows if row.get("diff_kind")]
    result: dict[str, object] = {
        "metric": {
            "target_size": 100,
            "candidate_size": candidate_size,
            "diff_rows": diff_count,
            "diff_kinds": {} if exact else {"DIFF_ARG_MISMATCH": diff_count},
            "exact": exact,
            "match_percent": 100.0 if exact else 90.0,
            "paired": True,
            "paired_symbol": FUNCTION,
        },
        "target": {
            "symbol": {"name": FUNCTION},
            "instruction_count": len(target_rows),
            "raw_instruction_sha256": _sha(f"{role}-{name}-target-raw"),
            "instruction_payload_sha256": _sha("target-payload"),
            "rows_kind": rows_kind,
            "rows": target_rows,
            "diff_row_count": diff_count,
        },
        "candidate": {
            "symbol": {"name": FUNCTION},
            "instruction_count": len(candidate_rows),
            "raw_instruction_sha256": _sha(f"{role}-{name}-candidate-raw"),
            "instruction_payload_sha256": _sha(f"{role}-candidate-payload"),
            "rows_kind": rows_kind,
            "rows": candidate_rows,
            "diff_row_count": diff_count,
        },
        "sections": {"target": [], "candidate": []},
        "protected_siblings": _protected(),
    }
    if name == "strict":
        result["relocation_annotations"] = {
            "authority": "report_annotation_not_physical_proof",
            "storage": "full",
            "target": _relocations(role, "target"),
            "candidate": _relocations(role, "candidate"),
        }
    else:
        result["relocation_annotations"] = {
            "authority": "report_annotation_not_physical_proof",
            "storage": "strict_channel_only",
            "strict_channel_reference": True,
        }
    return result


def _artifact(role: str) -> dict[str, object]:
    counts = {"structural": 11, "dform": 7, "owner_pool": 5, "exact": 0}
    diff_indices = {
        "structural": {1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12},
        "dform": {6, 7, 8, 9, 10, 11, 12},
        "owner_pool": set(FINAL_ROWS),
        "exact": set(),
    }[role]
    target = _target_rows(diff_indices)
    candidate = _candidate_rows(role, diff_indices)
    value: dict[str, object] = {
        "schema": composer.FOCUS_SCHEMA,
        "schema_version": 1,
        "function": FUNCTION,
        "input_binding": {},
        "channels": {
            "strict": _channel(role, "strict", copy.deepcopy(target), copy.deepcopy(candidate), counts[role]),
            "data": _channel(role, "data", copy.deepcopy(target), copy.deepcopy(candidate), counts[role]),
        },
        "physical_relocations": {"status": "UNKNOWN"},
        "policies": {},
        "source_patch_emitted": False,
        "retention_authorized": False,
        "promotion_authorized": False,
        "authority_advanced": False,
    }
    value["artifact_sha256"] = composer.canonical_sha256(value)
    return value


def _context(artifacts: dict[str, dict[str, object]]) -> dict[str, object]:
    report_sha = _sha("report")
    counts = {"structural": 11, "dform": 7, "owner_pool": 5, "exact": 0}
    stages: dict[str, object] = {}
    for role in composer.STAGE_ROLES:
        stages[role] = {
            "file_sha256": _sha(f"{role}-file"),
            "artifact_sha256": artifacts[role]["artifact_sha256"],
            "source_sha256": _sha(f"{role}-source"),
            "object_sha256": _sha(f"{role}-object"),
            "expected": {
                "target_size": 100,
                "candidate_size": 92 if role == "structural" else 100,
                "strict_diff_rows": counts[role],
                "data_diff_rows": counts[role],
                "strict_exact": role == "exact",
                "data_exact": role == "exact",
            },
        }
    return {
        "schema": composer.CONTEXT_SCHEMA,
        "owner": OWNER,
        "function": FUNCTION,
        "report": {"sha256": report_sha},
        "stages": stages,
        "physical_receipt": {"sha256": _sha("physical"), "expected_count": 3},
        "semantic_axes": {
            "stdlib_abs_threshold": {
                "header": "stdlib.h",
                "callee": "abs",
                "source_expression": "abs((int)(target.x - model.x)) < lbl_THRESHOLD",
                "evidence_sha256": report_sha,
            },
            "dform_copy": {
                "aggregate_type": "HuVecF",
                "helper": "ConfigHuVecCopy",
                "target_row_start": 1,
                "opcodes": ["psq_l", "lfs", "psq_st", "stfs"],
                "mwcc_guard": "__MWERKS__",
                "portable_fallback": True,
                "evidence_sha256": report_sha,
            },
            "scalar_owners": {
                "reused_parameter": "time",
                "live_owner": "weight",
                "scaled_role": "scaled_duration",
                "evidence_sha256": report_sha,
            },
            "typed_pool": {
                "required_owner": "lbl_POOL",
                "decoder": "typed_pool_owner_manifest",
                "all_nonfinal_rows_closed": True,
                "evidence_sha256": report_sha,
            },
            "final_operand_order": {
                "target_expression": "time * lbl_POOL",
                "control_expression": "lbl_POOL * time",
                "row_indices": FINAL_ROWS,
                "target_opcodes": ["lfd", "<gap>", "fsub", "lfd", "fmul"],
                "candidate_opcodes": ["lfd", "lfd", "fsub", "<gap>", "fmul"],
                "required_opcodes": ["lfd", "fsub", "fmul"],
                "evidence_sha256": report_sha,
            },
        },
        "invariants": {
            "cfg_exact_after_owner_pool": True,
            "calls_exact_after_owner_pool": True,
            "frame_exact_after_dform": True,
            "natural_c_only": True,
            "zero_protected_losses": True,
            "tracer_required": False,
            "source_patch_authorized": False,
            "retention_authorized": False,
            "promotion_authorized": False,
        },
    }


def _physical() -> dict[str, object]:
    return {
        "candidate_count": 3,
        "difference_count": 0,
        "differences": [],
        "focus": FUNCTION,
        "target_count": 3,
    }


def _resign(artifact: dict[str, object], context: dict[str, object], role: str) -> None:
    artifact.pop("artifact_sha256", None)
    artifact["artifact_sha256"] = composer.canonical_sha256(artifact)
    context["stages"][role]["artifact_sha256"] = artifact["artifact_sha256"]  # type: ignore[index]


class DformScalarOwnerComposerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.artifacts = {role: _artifact(role) for role in composer.STAGE_ROLES}
        self.context = _context(self.artifacts)
        self.physical = _physical()

    def evaluate(self) -> dict[str, object]:
        return composer.evaluate(
            self.context,
            self.artifacts,
            self.physical,
            bindings={"fixture": "synthetic"},
        )

    def test_ready_plan_is_two_cells_and_authority_free(self) -> None:
        result = self.evaluate()
        self.assertEqual(result["status"], "READY")
        self.assertEqual(result["compile_budget"], 2)
        self.assertEqual(
            [cell["id"] for cell in result["ordered_cells"]],
            ["compose_abs_dform_scalar_pool", "commute_final_multiply_operands"],
        )
        self.assertFalse(result["tracer_required"])
        self.assertFalse(result["source_patch_emitted"])
        self.assertFalse(result["retention_authorized"])
        self.assertFalse(result["promotion_authorized"])
        self.assertFalse(result["authority_advanced"])

    def test_output_is_deterministic(self) -> None:
        first = self.evaluate()
        second = self.evaluate()
        self.assertEqual(first, second)
        unsigned = dict(first)
        expected = unsigned.pop("diagnosis_sha256")
        self.assertEqual(expected, composer.canonical_sha256(unsigned))

    def test_missing_context_field_fails_closed(self) -> None:
        del self.context["semantic_axes"]["typed_pool"]  # type: ignore[index]
        with self.assertRaisesRegex(composer.DformScalarOwnerInputError, "missing fields"):
            self.evaluate()

    def test_artifact_internal_hash_tamper_fails_closed(self) -> None:
        strict = self.artifacts["dform"]["channels"]["strict"]  # type: ignore[index]
        strict["metric"]["diff_rows"] = 6  # type: ignore[index]
        with self.assertRaisesRegex(composer.DformScalarOwnerInputError, "internal SHA-256"):
            self.evaluate()

    def test_missing_dform_sequence_fails_closed(self) -> None:
        strict = self.artifacts["dform"]["channels"]["strict"]  # type: ignore[index]
        strict["candidate"]["rows"][1]["instruction"]["formatted"] = "lfs f1, 0x0(r3)"  # type: ignore[index]
        _resign(self.artifacts["dform"], self.context, "dform")
        with self.assertRaisesRegex(composer.DformScalarOwnerInputError, "D-form copy"):
            self.evaluate()

    def test_final_row_signature_drift_fails_closed(self) -> None:
        axis = self.context["semantic_axes"]["final_operand_order"]  # type: ignore[index]
        axis["row_indices"] = [5, 7, 8, 9, 10]  # type: ignore[index]
        with self.assertRaisesRegex(composer.DformScalarOwnerInputError, "row indices"):
            self.evaluate()

    def test_protected_sibling_regression_fails_closed(self) -> None:
        for name in ("strict", "data"):
            protected = self.artifacts["exact"]["channels"][name]["protected_siblings"]  # type: ignore[index]
            protected["exact_sibling_count"] = 1  # type: ignore[index]
        _resign(self.artifacts["exact"], self.context, "exact")
        with self.assertRaisesRegex(composer.DformScalarOwnerInputError, "protected exact sibling"):
            self.evaluate()

    def test_physical_relocation_difference_fails_closed(self) -> None:
        self.physical["difference_count"] = 1
        self.physical["differences"] = [{"row": 1}]
        with self.assertRaisesRegex(composer.DformScalarOwnerInputError, "not exact"):
            self.evaluate()

    def test_cli_verifies_all_file_hashes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            stage_paths: dict[str, Path] = {}
            for role in composer.STAGE_ROLES:
                path = root / f"{role}.json"
                path.write_text(json.dumps(self.artifacts[role], sort_keys=True), encoding="utf-8")
                stage_paths[role] = path
                self.context["stages"][role]["file_sha256"] = composer.file_sha256(path)  # type: ignore[index]

            report = root / "report.md"
            report.write_text("bound report\n", encoding="utf-8")
            report_sha = composer.file_sha256(report)
            self.context["report"]["sha256"] = report_sha  # type: ignore[index]
            for axis in self.context["semantic_axes"].values():  # type: ignore[union-attr]
                axis["evidence_sha256"] = report_sha  # type: ignore[index]

            physical = root / "physical.json"
            physical.write_text(json.dumps(self.physical, sort_keys=True), encoding="utf-8")
            self.context["physical_receipt"]["sha256"] = composer.file_sha256(physical)  # type: ignore[index]

            context = root / "context.json"
            context.write_text(json.dumps(self.context, sort_keys=True), encoding="utf-8")
            context_sha = composer.file_sha256(context)
            output = root / "diagnosis.json"
            exit_code = composer.main(
                [
                    str(context),
                    str(stage_paths["structural"]),
                    str(stage_paths["dform"]),
                    str(stage_paths["owner_pool"]),
                    str(stage_paths["exact"]),
                    str(physical),
                    str(report),
                    "--expect-context-sha256",
                    context_sha,
                    "--output",
                    str(output),
                    "--require-ready",
                ]
            )
            self.assertEqual(exit_code, 0)
            result = json.loads(output.read_text(encoding="utf-8"))
            self.assertEqual(result["status"], "READY")
            self.assertEqual(result["input_binding"]["report"]["sha256"], report_sha)

    def test_cli_rejects_wrong_context_hash(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            context = root / "context.json"
            context.write_text(json.dumps(self.context), encoding="utf-8")
            args = [
                str(context),
                str(context),
                str(context),
                str(context),
                str(context),
                str(context),
                str(context),
                "--expect-context-sha256",
                "0" * 64,
            ]
            self.assertEqual(composer.main(args), 2)


if __name__ == "__main__":
    unittest.main()
