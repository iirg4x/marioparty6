import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from contextlib import ExitStack
from pathlib import Path
from unittest import mock

from tools import board_causal_map as causal
from tools import match_workbench
from tools import mwcc_fe_chronology


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "tools" / "board_causal_map.py"


def _sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _descriptor(path):
    return {
        "path": str(path),
        "size_bytes": path.stat().st_size,
        "sha256": _sha(path),
    }


class BoardCausalMapTests(unittest.TestCase):
    def _fixture(self, directory):
        root = Path(directory)
        source = root / "owner.c"
        target = root / "target.o"
        obj = root / "candidate.o"
        report = root / "objdiff.json"
        interaction = root / "interactions.json"
        graph = root / "graph.json"
        workspace = root / "workbench"
        workspace.mkdir()
        source.write_text("void residual_fn(void) {}\n", encoding="utf-8")
        target.write_bytes(b"target-object")
        obj.write_bytes(b"candidate-object")
        report.write_text(
            json.dumps(
                {
                    "left": {
                        "symbols": [
                            {
                                "name": "residual_fn",
                                "kind": "SYMBOL_FUNCTION",
                                "address": "100",
                                "size": "12",
                                "match_percent": 75.0,
                                "target_symbol": 0,
                                "instructions": [
                                    {
                                        "diff_kind": "DIFF_RELOC_MISMATCH",
                                        "instruction": {
                                            "address": "100",
                                            "size": 4,
                                            "formatted": "lwz r3, 0(r4)",
                                        },
                                    }
                                ],
                            }
                        ],
                        "sections": [],
                    },
                    "right": {
                        "symbols": [
                            {
                                "name": "residual_fn",
                                "kind": "SYMBOL_FUNCTION",
                                "address": "500",
                                "size": "8",
                                "match_percent": 75.0,
                                "instructions": [],
                            }
                        ],
                        "sections": [],
                    },
                }
            ),
            encoding="utf-8",
        )
        interaction.write_text("{}\n", encoding="utf-8")
        graph.write_text(
            json.dumps(
                {
                    "nodes": [
                        {
                            "id": "game_src_owner_residual_fn",
                            "label": "residual_fn()",
                            "source_file": "game/src/board/owner.c",
                            "source_location": "L40",
                        }
                    ],
                    "links": [],
                }
            ),
            encoding="utf-8",
        )

        compiler_sha = "c" * 64
        context_sha = "d" * 64
        session_sha = "e" * 64
        source_desc = _descriptor(source)
        target_desc = _descriptor(target)
        object_desc = _descriptor(obj)
        report_desc = _descriptor(report)
        request = {
            "schema": causal.REQUEST_SCHEMA,
            "schema_version": 1,
            "owner": "REL:board:owner",
            "source": {**source_desc, "candidate_id": "candidate-current"},
            "target": target_desc,
            "compiler": {
                "toolchain_key": "GC/2.6",
                "compiler_sha256": compiler_sha,
                "context_sha256": context_sha,
            },
            "report": {**report_desc, "kind": "strict"},
            "workbench": {
                "path": str(workspace),
                "session_id": "owner-session",
                "session_sha256": session_sha,
            },
            "interaction_request": _descriptor(interaction),
            "graph": {
                **_descriptor(graph),
                "source_locations": [
                    {
                        "function": "residual_fn",
                        "node_id": "game_src_owner_residual_fn",
                        "node_label": "residual_fn()",
                        "source_file": "game/src/board/owner.c",
                        "source_location": "L40",
                    }
                ],
            },
        }
        request_path = root / "request.json"
        request_path.write_text(json.dumps(request, indent=2), encoding="utf-8")

        context = {
            "context_complete": True,
            "toolchain_key": "GC/2.6",
            "compiler": {
                "path": str(root / "mwcc.exe"),
                "size_bytes": 10,
                "sha256": compiler_sha,
            },
        }
        session = {
            "session_id": "owner-session",
            "session_sha256": session_sha,
            "request": {
                "owner": "REL:board:owner",
                "target": target_desc,
                "context": context,
            },
        }
        candidate = {
            "candidate_id": "candidate-current",
            "source": source_desc,
            "object": object_desc,
            "reports": {
                "strict": {
                    "raw_sha256": report_desc["sha256"],
                    "raw_size_bytes": report_desc["size_bytes"],
                }
            },
            "record_sha256": "f" * 64,
        }
        matrix = {
            "schema": "match_workbench_matrix/v1",
            "session_id": "owner-session",
            "matrix_sha256": "1" * 64,
            "rows": [
                {
                    "ordinal": 1,
                    "candidate_id": "candidate-current",
                    "source_sha256": source_desc["sha256"],
                    "object_sha256": object_desc["sha256"],
                    "focus_symbol": "residual_fn",
                    "strict_focus": {
                        "exact": False,
                        "match_percent": 75.0,
                        "diff_rows": 1,
                        "target_size": 12,
                        "candidate_size": 8,
                    },
                    "data_focus": None,
                    "hypothesis_axis": "prototype-width",
                    "axis_fingerprint": "2" * 64,
                    "outcome": {"status": "rejected", "reason": "no-go"},
                }
            ],
            "authority_advanced": False,
        }
        plan = {
            "schema": "match_workbench_interaction_plan/v1",
            "focus_symbols": ["residual_fn"],
            "axes": [
                {
                    "id": "declaration",
                    "hypothesis": "declaration chronology",
                    "control_level": "current",
                    "levels": [
                        {
                            "id": "current",
                            "source_action": "retain current declaration",
                            "evidence": ["control"],
                            "admissibility": "admissible",
                        },
                        {
                            "id": "earlier",
                            "source_action": "move declaration earlier",
                            "evidence": ["causal reducer"],
                            "admissibility": "admissible",
                        },
                    ],
                },
                {
                    "id": "prototype",
                    "hypothesis": "prototype width",
                    "control_level": "current",
                    "levels": [
                        {
                            "id": "current",
                            "source_action": "retain current prototype",
                            "evidence": ["control"],
                            "admissibility": "admissible",
                        },
                        {
                            "id": "narrow",
                            "source_action": "test evidenced narrow type",
                            "evidence": ["sign extension"],
                            "admissibility": "conditional",
                        },
                    ],
                },
            ],
            "recommended_execution_order": ["cell-next"],
            "cells": [
                {
                    "cell_id": "cell-next",
                    "selection": {"declaration": "earlier", "prototype": "narrow"},
                    "interaction_order": 2,
                    "action": "generate_and_compile",
                    "observation": None,
                }
            ],
            "interaction_plan_sha256": "3" * 64,
            "authority_advanced": False,
        }
        cascade = {
            "audit": {
                "functions": [
                    {
                        "function": "residual_fn",
                        "clusters": [
                            {
                                "classification": "relocation_or_data_mismatch",
                                "confidence": 0.88,
                                "index_start": 0,
                                "index_end": 0,
                                "target_address_start": 100,
                                "target_address_end": 100,
                                "candidate_address_start": 500,
                                "candidate_address_end": 500,
                                "diff_pair_count": 1,
                                "evidence": {
                                    "diff_kinds": {"DIFF_RELOC_MISMATCH": 1},
                                    "relocation_signal": True,
                                },
                            }
                        ],
                        "patterns": [],
                    }
                ]
            },
            "causal_reducer_sha256": "4" * 64,
            "authority_advanced": False,
        }
        pool = {
            "decode": {
                "target": {"pool_consumer_count": 1},
                "candidate": {"pool_consumer_count": 1},
                "summary": {
                    "classification_counts": {"relocation_type_mismatch": 1}
                },
                "groups": [
                    {
                        "classification": "relocation_type_mismatch",
                        "recommended_source_axis": "test literal contract",
                    }
                ],
                "groups_omitted": 0,
            },
            "pool_decoder_sha256": "5" * 64,
            "authority_advanced": False,
        }
        stack = {
            "target_instruction_count": 3,
            "target_stack_access_count": 1,
            "stack_slot_count": 1,
            "zero_read_slot_count": 1,
            "excluded_stack_access_count": 0,
            "outgoing_call_argument_access_count": 0,
            "zero_read_slots": [{"offset": 32, "width": 4}],
            "authority_advanced": False,
        }
        telemetry = {
            "schema": "match_workbench_function_telemetry/v1",
            "telemetry_sha256": "6" * 64,
            "coverage": {"candidate_history": "complete_indexed_workbench_history"},
            "authority_advanced": False,
        }
        return {
            "root": root,
            "request": request,
            "request_path": request_path,
            "workspace": workspace,
            "session": session,
            "candidate": candidate,
            "matrix": matrix,
            "plan": plan,
            "cascade": cascade,
            "pool": pool,
            "stack": stack,
            "telemetry": telemetry,
            "context": context,
            "context_sha": context_sha,
        }

    def _patch_components(self, fixture, *, plan=None):
        stack = ExitStack()
        stack.enter_context(
            mock.patch.object(match_workbench, "_workspace", return_value=fixture["workspace"])
        )
        stack.enter_context(
            mock.patch.object(match_workbench, "_load_session", return_value=fixture["session"])
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "_load_index",
                return_value={"candidates": {"candidate-current": "candidate.json"}},
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench, "_load_candidate", return_value=fixture["candidate"]
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "_require_candidate_compile_attestation",
                return_value={"status": "valid"},
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "_compile_context_projection",
                return_value={
                    "toolchain_key": "GC/2.6",
                    "compiler": fixture["context"]["compiler"],
                },
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "_compile_context_sha256",
                return_value=fixture["context_sha"],
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench, "build_matrix", return_value=fixture["matrix"]
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "build_function_telemetry",
                return_value=fixture["telemetry"],
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "reduce_objdiff_cascades",
                return_value=fixture["cascade"],
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "decode_pool_ownership",
                return_value=fixture["pool"],
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "inspect_stack_residue",
                return_value=fixture["stack"],
            )
        )
        stack.enter_context(
            mock.patch.object(
                match_workbench,
                "plan_candidate_interactions",
                return_value=plan or fixture["plan"],
            )
        )
        return stack

    def test_composes_full_residual_map_deterministically(self):
        with tempfile.TemporaryDirectory() as directory:
            fixture = self._fixture(directory)
            with self._patch_components(fixture):
                first = causal.build_causal_map(
                    fixture["root"], fixture["request_path"]
                )
                second = causal.build_causal_map(
                    fixture["root"], fixture["request_path"]
                )

        self.assertEqual(first, second)
        body = {key: value for key, value in first.items() if key != "causal_map_sha256"}
        self.assertEqual(first["causal_map_sha256"], causal._digest(body))
        self.assertFalse(first["authority_advanced"])
        self.assertFalse(first["production_modified"])
        function = first["inventory"]["functions"][0]
        self.assertEqual(function["symbol"], "residual_fn")
        self.assertEqual(function["metrics"]["target_size"], 12)
        self.assertEqual(function["metrics"]["candidate_size"], 8)
        self.assertEqual(function["metrics"]["diff_kinds"], {"DIFF_RELOC_MISMATCH": 1})
        self.assertEqual(
            function["earliest_structural_cause"]["classification"],
            "relocation_or_data_mismatch",
        )
        self.assertTrue(function["relocations"]["causal_signal"])
        self.assertEqual(function["stack"]["zero_read_slot_count"], 1)
        self.assertEqual(function["rejected_axes"][0]["axis"], "prototype-width")
        self.assertEqual(function["graph_source_locations"][0]["source_location"], "L40")
        self.assertEqual(first["coverage"]["tracer"]["status"], "UNKNOWN")
        self.assertEqual(first["coverage"]["physical_relocations"]["status"], "UNKNOWN")
        self.assertEqual(first["next_axes"][0]["rank"], 1)
        self.assertEqual(
            [row["axis"] for row in first["next_axes"][0]["dependency_closure"]],
            ["declaration", "prototype"],
        )

    def test_rejects_compiler_context_mismatch_before_composition(self):
        with tempfile.TemporaryDirectory() as directory:
            fixture = self._fixture(directory)
            fixture["request"]["compiler"]["context_sha256"] = "9" * 64
            fixture["request_path"].write_text(
                json.dumps(fixture["request"], indent=2), encoding="utf-8"
            )
            with self._patch_components(fixture):
                with self.assertRaisesRegex(
                    causal.CausalMapError, "compiler context hash"
                ):
                    causal.build_causal_map(fixture["root"], fixture["request_path"])

    def test_rejects_planner_observation_from_another_workbench_context(self):
        with tempfile.TemporaryDirectory() as directory:
            fixture = self._fixture(directory)
            plan = dict(fixture["plan"])
            plan["cells"] = [
                {
                    "cell_id": "cell-observed",
                    "selection": {"declaration": "current", "prototype": "current"},
                    "interaction_order": 0,
                    "action": "reuse_measured_candidate",
                    "observation": {
                        "candidate_id": "candidate-current",
                        "source_sha256": "0" * 64,
                        "object_sha256": fixture["matrix"]["rows"][0][
                            "object_sha256"
                        ],
                    },
                }
            ]
            plan["recommended_execution_order"] = []
            with self._patch_components(fixture, plan=plan):
                with self.assertRaisesRegex(
                    causal.CausalMapError, "observation context mismatch"
                ):
                    causal.build_causal_map(fixture["root"], fixture["request_path"])

    def test_rejects_graph_location_not_present_in_bound_graph(self):
        with tempfile.TemporaryDirectory() as directory:
            fixture = self._fixture(directory)
            fixture["request"]["graph"]["source_locations"][0][
                "source_location"
            ] = "L41"
            fixture["request_path"].write_text(
                json.dumps(fixture["request"], indent=2), encoding="utf-8"
            )
            with self._patch_components(fixture):
                with self.assertRaisesRegex(
                    causal.CausalMapError, "source_location does not match"
                ):
                    causal.build_causal_map(fixture["root"], fixture["request_path"])

    def test_duplicate_symbol_occurrences_keep_all_structural_lanes_unknown(self):
        with tempfile.TemporaryDirectory() as directory:
            fixture = self._fixture(directory)
            report_path = Path(fixture["request"]["report"]["path"])
            report = json.loads(report_path.read_text(encoding="utf-8"))
            second_left = dict(report["left"]["symbols"][0])
            second_left["address"] = "200"
            second_left["target_symbol"] = 1
            second_right = dict(report["right"]["symbols"][0])
            second_right["address"] = "600"
            report["left"]["symbols"].append(second_left)
            report["right"]["symbols"].append(second_right)
            report_path.write_text(json.dumps(report), encoding="utf-8")
            bound_report = {**_descriptor(report_path), "kind": "strict"}
            fixture["request"]["report"] = bound_report
            fixture["candidate"]["reports"]["strict"] = {
                "raw_sha256": bound_report["sha256"],
                "raw_size_bytes": bound_report["size_bytes"],
            }
            fixture["request_path"].write_text(
                json.dumps(fixture["request"], indent=2), encoding="utf-8"
            )
            with self._patch_components(fixture):
                result = causal.build_causal_map(
                    fixture["root"], fixture["request_path"]
                )

        functions = result["inventory"]["functions"]
        self.assertEqual(len(functions), 2)
        self.assertEqual(
            [row["earliest_structural_cause"]["status"] for row in functions],
            ["UNKNOWN", "UNKNOWN"],
        )
        self.assertEqual(
            [row["telemetry"]["status"] for row in functions],
            ["UNKNOWN", "UNKNOWN"],
        )
        self.assertEqual(result["coverage"]["structural_cause"]["status"], "UNKNOWN")

    def test_validates_available_tracer_receipt_provenance_without_advancing_authority(
        self,
    ):
        with tempfile.TemporaryDirectory() as directory:
            fixture = self._fixture(directory)
            receipt = fixture["root"] / "tracer-receipt.json"
            receipt.write_text("{}\n", encoding="utf-8")
            fixture["request"]["tracer_receipts"] = [
                {**_descriptor(receipt), "focus_symbols": ["residual_fn"]}
            ]
            fixture["request_path"].write_text(
                json.dumps(fixture["request"], indent=2), encoding="utf-8"
            )
            tracer_report = {
                "producer": {
                    "status": "blocked",
                    "producer_id": "native-mwcc-fe-tracer",
                },
                "provenance": {
                    "source_sha256": fixture["request"]["source"]["sha256"],
                    "compiler_sha256": fixture["request"]["compiler"][
                        "compiler_sha256"
                    ],
                    "trace_sha256": "7" * 64,
                },
                "objects": [
                    {
                        "uid": "object-1",
                        "home_join": {"candidates": [{"kind": "stack"}]},
                    }
                ],
            }
            with self._patch_components(fixture), mock.patch.object(
                mwcc_fe_chronology, "load_report", return_value=tracer_report
            ):
                result = causal.build_causal_map(
                    fixture["root"], fixture["request_path"]
                )

        self.assertEqual(result["coverage"]["tracer"]["status"], "BLOCKED")
        tracer = result["inventory"]["functions"][0]["tracer_receipts"][0]
        self.assertEqual(tracer["object_uids"], ["object-1"])
        self.assertEqual(tracer["authenticated_home_candidate_count"], 1)
        self.assertFalse(result["authority_advanced"])

    def test_help_is_available_for_direct_execution(self):
        completed = subprocess.run(
            [sys.executable, str(SCRIPT), "--help"],
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(completed.returncode, 0)
        self.assertIn("causal-map request", completed.stdout)


if __name__ == "__main__":
    unittest.main()
