from __future__ import annotations

import concurrent.futures
import contextlib
import gzip
import hashlib
import io
import json
import os
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from unittest import mock

from tools import match_workbench as module


def _sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _descriptor(path: Path) -> dict[str, object]:
    return {"path": str(path), "size_bytes": path.stat().st_size, "sha256": _sha256(path)}


def _write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, separators=(",", ":"), ensure_ascii=False) + "\n", encoding="utf-8")


def _rehash(value: dict[str, object], field: str) -> dict[str, object]:
    body = dict(value)
    body.pop(field, None)
    canonical = json.dumps(body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
    body[field] = hashlib.sha256(canonical.encode("utf-8")).hexdigest()
    return body


def _report(function: str, *, exact: bool = False, large: bool = False) -> dict[str, object]:
    rows: list[dict[str, object]] = []
    if not exact:
        rows.append({"diff_kind": "REG_SWAP", "instruction": {"formatted": "mr r3,r4"}})
    if large:
        rows.extend({"diff_kind": "NOP", "instruction": {"formatted": "nop"}} for _ in range(400))
    return {
        "left": {
            "symbols": [
                {
                    "name": function,
                    "kind": "SYMBOL_FUNCTION",
                    "size": "4",
                    "target_symbol": 0,
                    "match_percent": 100.0 if exact else 75.0,
                    "instructions": rows,
                }
            ]
        },
        "right": {"symbols": [{"name": function, "kind": "SYMBOL_FUNCTION", "size": "4"}]},
    }


def _assessment_report(
    *,
    focus_match: float = 75.0,
    focus_size: str = "4",
    focus_candidate_size: str = "4",
    sibling_match: float = 100.0,
    sibling_size: str = "8",
    sibling_candidate_size: str = "8",
    sibling_diff_kind: str | None = None,
) -> dict[str, object]:
    focus_rows = [] if focus_match == 100.0 else [
        {"diff_kind": "REG_SWAP", "instruction": {"formatted": "mr r3,r4"}}
    ]
    sibling_rows = [] if sibling_match == 100.0 and sibling_diff_kind is None else [
        {
            "diff_kind": sibling_diff_kind or "REG_SWAP",
            "instruction": {"formatted": "mr r5,r6"},
        }
    ]
    return {
        "left": {
            "symbols": [
                {
                    "name": "focus",
                    "kind": "SYMBOL_FUNCTION",
                    "size": focus_size,
                    "target_symbol": 0,
                    "match_percent": focus_match,
                    "instructions": focus_rows,
                },
                {
                    "name": "sibling",
                    "kind": "SYMBOL_FUNCTION",
                    "size": sibling_size,
                    "target_symbol": 1,
                    "match_percent": sibling_match,
                    "instructions": sibling_rows,
                },
            ]
        },
        "right": {
            "symbols": [
                {"name": "focus", "kind": "SYMBOL_FUNCTION", "size": focus_candidate_size},
                {"name": "sibling", "kind": "SYMBOL_FUNCTION", "size": sibling_candidate_size},
            ]
        },
    }


def _assessment_multi_report(
    symbols: tuple[tuple[str, float], ...],
) -> dict[str, object]:
    left_symbols: list[dict[str, object]] = []
    right_symbols: list[dict[str, object]] = []
    for index, (name, match_percent) in enumerate(symbols):
        left_symbols.append(
            {
                "name": name,
                "kind": "SYMBOL_FUNCTION",
                "size": "4",
                "target_symbol": index,
                "match_percent": match_percent,
                "instructions": (
                    []
                    if match_percent == 100.0
                    else [{"diff_kind": "REG_SWAP", "instruction": {"formatted": "mr r3,r4"}}]
                ),
            }
        )
        right_symbols.append({"name": name, "kind": "SYMBOL_FUNCTION", "size": "4"})
    return {
        "left": {"symbols": left_symbols},
        "right": {"symbols": right_symbols},
    }


def _residual_report(
    entries: tuple[tuple[str, float, str, str, tuple[str, ...]], ...],
) -> dict[str, object]:
    left_symbols: list[dict[str, object]] = []
    right_symbols: list[dict[str, object]] = []
    for index, (name, match_percent, target_size, candidate_size, diff_kinds) in enumerate(entries):
        rows = [
            {"diff_kind": kind, "instruction": {"formatted": "mr r3,r4"}}
            for kind in diff_kinds
        ]
        left_symbols.append(
            {
                "name": name,
                "kind": "SYMBOL_FUNCTION",
                "size": target_size,
                "target_symbol": index,
                "match_percent": match_percent,
                "instructions": rows,
            }
        )
        right_symbols.append(
            {
                "name": name,
                "kind": "SYMBOL_FUNCTION",
                "size": candidate_size,
            }
        )
    return {
        "left": {"symbols": left_symbols},
        "right": {"symbols": right_symbols},
    }


def _stack_instruction(
    address: int,
    mnemonic: str,
    register: str,
    offset: int,
    base: str = "r1",
) -> dict[str, object]:
    return {
        "address": address,
        "size": 4,
        "formatted": f"{mnemonic} {register},{offset}({base})",
        "parts": [
            {"opcode": {"mnemonic": mnemonic, "opcode": 0}},
            {"arg": {"opaque": register}},
            {"separator": True},
            {"arg": {"signed": offset}},
            {"basic": "("},
            {"arg": {"opaque": base}},
            {"basic": ")"},
        ],
    }


def _stack_paired_single_instruction(
    address: int,
    mnemonic: str,
    register: str,
    offset: int,
    w: object = 0,
    quantization_register: object = "qr0",
    base: str = "r1",
) -> dict[str, object]:
    """Build the canonical objdiff shape used by real CrackOM psq rows."""
    return {
        "address": address,
        "size": 4,
        "formatted": (
            f"{mnemonic} {register}, 0x{offset:x}({base}), "
            f"{w}, {quantization_register}"
        ),
        "parts": [
            {"opcode": {"mnemonic": mnemonic, "opcode": 478}},
            {"arg": {"opaque": register}},
            {"separator": True},
            {"arg": {"signed": offset}},
            {"basic": "("},
            {"arg": {"opaque": base}},
            {"basic": ")"},
            {"separator": True},
            {"arg": {"opaque": str(w)}},
            {"separator": True},
            {"arg": {"opaque": str(quantization_register)}},
        ],
    }


def _stack_register_instruction(
    address: int,
    mnemonic: str,
    register: str,
) -> dict[str, object]:
    return {
        "address": address,
        "size": 4,
        "formatted": f"{mnemonic} {register}",
        "parts": [
            {"opcode": {"mnemonic": mnemonic, "opcode": 0}},
            {"arg": {"opaque": register}},
        ],
    }


def _stack_direct_call_instruction(
    address: int,
    target: str = "OSReport",
) -> dict[str, object]:
    return {
        "address": address,
        "size": 4,
        "formatted": f"bl {target}",
        "parts": [
            {"opcode": {"mnemonic": "bl", "opcode": 267}},
            {"arg": {"reloc": True}},
        ],
    }


def _stack_residue_report(
    instructions: list[dict[str, object]],
) -> dict[str, object]:
    return {
        "left": {
            "symbols": [
                {
                    "name": "focus",
                    "kind": "SYMBOL_FUNCTION",
                    "size": "16",
                    "target_symbol": 0,
                    "match_percent": 75.0,
                    "instructions": [{"instruction": item} for item in instructions],
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": "focus",
                    "kind": "SYMBOL_FUNCTION",
                    "size": "16",
                }
            ]
        },
    }


class MatchWorkbenchTests(unittest.TestCase):
    maxDiff = None

    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.target = self.root / "target.o"
        self.source = self.root / "candidate.c"
        self.object = self.root / "candidate.o"
        self.strict = self.root / "strict.json"
        self.data = self.root / "data.json"
        self.target.write_bytes(b"target-bytes")
        self.source.write_text("int fn(void) { return 1; }\n", encoding="utf-8")
        self.object.write_bytes(b"object-bytes")
        _write_json(self.strict, _report("fn"))
        _write_json(self.data, _report("fn", exact=True))
        self.manifest = self.root / "request.json"
        _write_json(self.manifest, self._manifest())
        self.workspace = self.root / "build" / "match"

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _manifest(self, *, session_id: str = "session-1", **request_overrides: object) -> dict[str, object]:
        value: dict[str, object] = {
            "schema": module.REQUEST_SCHEMA,
            "schema_version": 1,
            "session_id": session_id,
            "owner": "REL:demo:fn",
            "unit": "demo.c",
            "function": "fn",
            "target": _descriptor(self.target),
            "context": {
                "base_commit": "abcdef1234567890",
                "toolchain_key": "GC/1.3.2",
                "compiler": None,
                "compile_argv": [],
                "compile_inputs": [],
            },
            "policy": {
                "max_workers": 4,
                "max_report_bytes": 2_000_000,
                "max_compact_bytes": 16_000,
                "allowed_job_kinds": ["artifact-fact", "cfg", "safe-probe"],
            },
        }
        value.update(request_overrides)
        return value

    def _complete_context(self, *, response_file: bool = False, dirty: bool = False) -> dict[str, object]:
        """Build a fully sealed executable context for compiler-reuse tests."""
        compiler = self.root / "complete-compiler.bin"
        wrapper = self.root / "complete-wrapper.bin"
        dependency = self.root / "complete-dependency.h"
        build_rule = self.root / "complete-build.ninja"
        runtime = self.root / "complete-runtime.dll"
        output = self.root / "complete-output.o"
        depfile = self.root / "complete-output.d"
        for path, contents in (
            (compiler, b"compiler"),
            (wrapper, b"wrapper"),
            (dependency, b"#define VALUE 1\n"),
            (build_rule, b"rule cc\n  command = compiler\n"),
            (runtime, b"runtime"),
            (output, b"output"),
            (depfile, b"complete-output.o: candidate.c complete-dependency.h\n"),
        ):
            path.write_bytes(contents)
        compile_cwd = self.root / "complete-cwd"
        include_root = self.root / "complete-include"
        source_tree_root = self.root / "complete-source-tree"
        compile_cwd.mkdir(exist_ok=True)
        include_root.mkdir(exist_ok=True)
        (include_root / "header.h").write_bytes(b"header\n")
        source_tree_root.mkdir(exist_ok=True)
        (source_tree_root / "tracked.c").write_bytes(b"tracked\n")
        dirty_patch = None
        state = "clean"
        if dirty:
            patch = self.root / "complete-dirty.patch"
            patch.write_bytes(b"diff --git a/tracked.c b/tracked.c\n")
            dirty_patch = module.descriptor(patch)
            state = "dirty"
        source_descriptor = module.descriptor(self.source)
        dependency_descriptor = module.descriptor(dependency)
        compile_inputs = [source_descriptor, dependency_descriptor]
        input_paths = sorted(item["path"] for item in compile_inputs)
        environment = module._current_environment()
        selected_environment = {
            "PATH": environment["variables"]["PATH"],
        }
        argv = [str(compiler), "-c", str(self.source)]
        argv_binding = None
        if response_file:
            response = compile_cwd / "complete.rsp"
            response.write_text("-c candidate.c\n", encoding="utf-8")
            argv = [str(compiler), "@complete.rsp"]
            argv_binding = {
                "schema": module.ARGV_BINDING_SCHEMA,
                "expanded": True,
                "expanded_argv": [str(compiler), "-c", str(self.source)],
                "response_files": [module.descriptor(response)],
            }
        context: dict[str, object] = {
            "base_commit": "abcdef1234567890",
            "toolchain_key": "GC/complete",
            "compiler": module.descriptor(compiler),
            "compile_argv": argv,
            "compile_cwd": module._directory_descriptor(
                {"path": str(compile_cwd)},
                root=self.root,
                label="test compile cwd",
                tree_names=False,
            ),
            "compile_tools": [module.descriptor(wrapper)],
            "compile_inputs": compile_inputs,
            "dependency_provenance": {
                "schema": module.DEPENDENCY_PROVENANCE_SCHEMA,
                "fresh": True,
                "depfile": module.descriptor(depfile),
                "input_paths": input_paths,
                "path_set_sha256": module._sha256_bytes(module._canonical(input_paths)),
            },
            "build_rule": module.descriptor(build_rule),
            "include_roots": [
                module._directory_descriptor(
                    {"path": str(include_root)},
                    root=self.root,
                    label="test include root",
                    tree_names=True,
                )
            ],
            "environment": {
                "schema": module.ENVIRONMENT_SCHEMA,
                "variables": selected_environment,
                "codepage": environment["codepage"],
                "locale": environment["locale"],
            },
            "runtime_dlls": [module.descriptor(runtime)],
            "compile_outputs": {
                "schema": module.OUTPUT_BINDING_SCHEMA,
                "output": module.descriptor(output),
                "depfile": module.descriptor(depfile),
            },
            "source_tree": {
                "schema": module.SOURCE_TREE_SCHEMA,
                "root": module._directory_descriptor(
                    {"path": str(source_tree_root)},
                    root=self.root,
                    label="test source tree",
                    tree_names=True,
                ),
                "state": state,
                "dirty_patch": dirty_patch,
            },
            "context_complete": True,
        }
        if argv_binding is not None:
            context["argv_binding"] = argv_binding
        return context

    def _init(self, *, workspace: Path | None = None) -> dict[str, object]:
        return module.init_workspace(self.root, self.manifest, workspace or self.workspace)

    def _record(
        self,
        candidate_id: str = "c1",
        *,
        source: Path | None = None,
        object_path: Path | None = None,
        strict_report: Path | None = None,
        data_report: Path | None = None,
        hypothesis: str = "natural candidate",
        axis: str = "register-lifetime",
        status: str = "measured",
        reason: str = "candidate measured",
        focus_symbol: str | list[str] | None = None,
        heavy_seconds: float | None = None,
        compile_attestation: Path | None = None,
    ) -> dict[str, object]:
        source_path = source or self.source
        object_file = object_path or self.object
        if compile_attestation is None:
            compile_attestation = self._attestation(
                candidate_id,
                source=source_path,
                object_path=object_file,
            )
        return module.record_candidate(
            self.root,
            self.workspace,
            candidate_id=candidate_id,
            source=source_path,
            object_path=object_file,
            compile_attestation=compile_attestation,
            strict_report=strict_report or self.strict,
            data_report=data_report,
            hypothesis=hypothesis,
            axis=axis,
            status=status,
            reason=reason,
            focus_symbol=focus_symbol,
            heavy_seconds=heavy_seconds,
        )

    def _attestation(
        self,
        label: str,
        *,
        source: Path | None = None,
        object_path: Path | None = None,
        workspace: Path | None = None,
    ) -> Path:
        workspace_path = workspace or self.workspace
        session = json.loads((workspace_path / "session.json").read_text(encoding="utf-8"))
        context = session["request"]["context"]
        compiler = context.get("compiler")
        output = self.root / f"{label}-compile-attestation.json"
        module.create_compile_attestation(
            self.root,
            workspace_path,
            source=source or self.source,
            object_path=object_path or self.object,
            output=output,
            producer_kind=(
                "external-compile-attestation" if compiler is not None else "test-fixture"
            ),
            producer_command=list(context.get("compile_argv", [])),
            notes="test fixture",
        )
        return output

    def test_function_telemetry_reports_coverage_rates_and_routes_centrally(self) -> None:
        self._init()
        self._record("c1", data_report=self.data, heavy_seconds=1.5)

        exact_source = self.root / "exact.c"
        exact_object = self.root / "exact.o"
        exact_strict = self.root / "exact-strict.json"
        exact_data = self.root / "exact-data.json"
        exact_source.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        exact_object.write_bytes(b"exact-object")
        _write_json(exact_strict, _report("fn", exact=True))
        _write_json(exact_data, _report("fn", exact=True))
        self._record(
            "c2",
            source=exact_source,
            object_path=exact_object,
            strict_report=exact_strict,
            data_report=exact_data,
            hypothesis="exact natural candidate",
            axis="cfg-and-lifetime",
            status="retained",
            reason="strict and data exact",
            heavy_seconds=2.5,
        )

        result = module.build_function_telemetry(
            self.root,
            self.workspace,
            focus_symbol="fn",
            elapsed_seconds=7200,
            active_seconds=3600,
            tracer_runs=2,
            donor_searches=1,
        )
        self.assertEqual(result["schema"], module.FUNCTION_TELEMETRY_SCHEMA)
        self.assertEqual(result["status"], "exact_with_complete_time_coverage")
        self.assertEqual(result["campaign"]["candidate_count"], 2)
        self.assertEqual(result["campaign"]["first_exact_candidate_id"], "c2")
        self.assertEqual(result["campaign"]["candidates_through_first_exact"], 2)
        self.assertEqual(result["campaign"]["nonexact_candidates_before_first_exact"], 1)
        self.assertEqual(result["campaign"]["outcome_counts"], {"measured": 1, "retained": 1})
        self.assertEqual(result["time"]["heavy_seconds"], 4.0)
        self.assertTrue(result["time"]["heavy_seconds_complete"])
        self.assertEqual(result["throughput"]["exact_functions_per_elapsed_hour"], 0.5)
        self.assertEqual(result["throughput"]["exact_functions_per_active_hour"], 1.0)
        self.assertEqual(result["throughput"]["exact_functions_per_heavy_process_hour"], 900.0)
        self.assertEqual(result["throughput"]["exact_bytes_per_elapsed_hour"], 2.0)
        self.assertEqual(result["activity"], {"tracer_runs": 2, "donor_searches": 1, "source": "caller_attested"})
        self.assertEqual(result["coverage"]["physical_relocations"], "not_authenticated_by_candidate_telemetry")
        self.assertFalse(result["authority_advanced"])

        argv = [
            "--root",
            str(self.root),
            "telemetry",
            "--workspace",
            str(self.workspace),
            "--function",
            "fn",
            "--elapsed-seconds",
            "7200",
            "--active-seconds",
            "3600",
            "--tracer-runs",
            "2",
            "--donor-searches",
            "1",
        ]
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(module.main(argv), 0)
        self.assertEqual(json.loads(output.getvalue()), result)

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "telemetry",
                "--workspace",
                str(self.workspace),
                "--function",
                "fn",
                "--elapsed-seconds",
                "7200",
                "--active-seconds",
                "3600",
                "--tracer-runs",
                "2",
                "--donor-searches",
                "1",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout), result)

    def test_function_telemetry_keeps_missing_time_unknown_and_rejects_empty_focus(self) -> None:
        self._init()
        self._record("c1", data_report=self.data)
        result = module.build_function_telemetry(
            self.root, self.workspace, focus_symbol="fn"
        )
        self.assertEqual(result["status"], "not_exact")
        self.assertFalse(result["time"]["heavy_seconds_complete"])
        self.assertEqual(result["time"]["heavy_seconds_missing_candidate_ids"], ["c1"])
        self.assertIsNone(result["throughput"]["exact_functions_per_elapsed_hour"])
        self.assertIsNone(result["throughput"]["exact_functions_per_heavy_process_hour"])
        with self.assertRaisesRegex(module.MatchError, "no candidate history"):
            module.build_function_telemetry(
                self.root, self.workspace, focus_symbol="missing"
            )
        with self.assertRaisesRegex(module.MatchError, "greater than zero"):
            module.build_function_telemetry(
                self.root, self.workspace, focus_symbol="fn", elapsed_seconds=0
            )

    def test_causal_reducer_ranks_explicit_else_return_and_routes_centrally(self) -> None:
        def instruction(
            address: int,
            formatted: str,
            *,
            diff_kind: str | None = None,
            branch_dest: int | None = None,
        ) -> dict[str, object]:
            row: dict[str, object] = {
                "instruction": {
                    "address": str(address),
                    "size": 4,
                    "formatted": formatted,
                }
            }
            if diff_kind is not None:
                row["diff_kind"] = diff_kind
            if branch_dest is not None:
                row["instruction"]["branch_dest"] = str(branch_dest)  # type: ignore[index]
                row["branch_dest"] = str(branch_dest)
            return row

        target = [
            instruction(100, "cmpwi r3, 1"),
            instruction(104, "bne 0x74", diff_kind="DIFF_ARG_MISMATCH", branch_dest=116),
            instruction(108, "bl body"),
            instruction(112, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            instruction(116, "b 0x78", diff_kind="DIFF_DELETE", branch_dest=120),
            instruction(120, "blr"),
        ]
        candidate = [
            instruction(500, "cmpwi r3, 1"),
            instruction(504, "bne 0x204", diff_kind="DIFF_ARG_MISMATCH", branch_dest=516),
            instruction(508, "bl body"),
            {"diff_kind": "DIFF_DELETE"},
            {"diff_kind": "DIFF_DELETE"},
            instruction(516, "blr"),
        ]
        report_path = self.root / "cascade-report.json"
        _write_json(
            report_path,
            {
                "left": {
                    "symbols": [
                        {
                            "name": "hook",
                            "kind": "SYMBOL_FUNCTION",
                            "address": "100",
                            "size": "24",
                            "target_symbol": 0,
                            "match_percent": 90.0,
                            "instructions": target,
                        }
                    ]
                },
                "right": {
                    "symbols": [
                        {
                            "name": "hook",
                            "kind": "SYMBOL_FUNCTION",
                            "address": "500",
                            "size": "16",
                            "match_percent": 90.0,
                            "instructions": candidate,
                        }
                    ]
                },
            },
        )
        result = module.reduce_objdiff_cascades(
            self.root, report=report_path, focus_symbol="hook"
        )
        self.assertEqual(result["schema"], module.CAUSAL_REDUCER_SCHEMA)
        self.assertFalse(result["authority_advanced"])
        self.assertEqual(
            result["audit"]["hypotheses"][0]["classification"],
            "explicit_else_return_epilogue",
        )
        self.assertIn(
            "else-return",
            result["audit"]["causal_groups"][0]["recommended_source_axis"],
        )
        self.assertRegex(result["causal_reducer_sha256"], r"^[0-9a-f]{64}$")

        argv = [
            "--root",
            str(self.root),
            "cascade",
            "--report",
            str(report_path),
            "--function",
            "hook",
        ]
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(module.main(argv), 0)
        self.assertEqual(json.loads(output.getvalue()), result)

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "cascade",
                "--report",
                str(report_path),
                "--function",
                "hook",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout), result)

        with self.assertRaisesRegex(module.MatchError, "focus_not_found"):
            module.reduce_objdiff_cascades(
                self.root, report=report_path, focus_symbol="missing"
            )

    def test_pool_decoder_types_owner_only_mismatches_and_routes_centrally(self) -> None:
        from tools.tests.test_pool_reloc_summary import _report as pool_report

        report_path = self.root / "pool-report.json"
        _write_json(report_path, pool_report())
        result = module.decode_pool_ownership(
            self.root,
            report=report_path,
            focus_symbol="PoolFocus",
        )
        self.assertEqual(result["schema"], module.POOL_DECODER_SCHEMA)
        self.assertFalse(result["authority_advanced"])
        self.assertRegex(result["pool_decoder_sha256"], r"^[0-9a-f]{64}$")
        decoded = result["decode"]
        self.assertEqual(
            decoded["summary"]["classification_counts"]["owner_identity_mismatch"],
            2,
        )
        self.assertTrue(
            any(
                group["target"]["owner"]["typed"].get("mwcc_role")
                == "signed-int-to-double-bias"
                for group in decoded["groups"]
                if group.get("target")
            )
        )

        argv = [
            "--root",
            str(self.root),
            "pools",
            "--report",
            str(report_path),
            "--function",
            "PoolFocus",
        ]
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(module.main(argv), 0)
        self.assertEqual(json.loads(output.getvalue()), result)

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "pool-decode",
                "--report",
                str(report_path),
                "--function",
                "PoolFocus",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout), result)

        with self.assertRaisesRegex(module.MatchError, "candidate function not found"):
            module.decode_pool_ownership(
                self.root,
                report=report_path,
                focus_symbol="missing",
            )

    def test_interaction_planner_builds_kamekku_factorial_and_routes_centrally(self) -> None:
        from tools.tests.test_candidate_interaction_planner import _request

        request_path = self.root / "kamekku-interactions.json"
        _write_json(request_path, _request())
        request_before = request_path.read_bytes()
        result = module.plan_candidate_interactions(
            self.root,
            request=request_path,
        )
        self.assertEqual(result["schema"], module.INTERACTION_PLANNER_SCHEMA)
        self.assertFalse(result["production_modified"])
        self.assertFalse(result["authority_advanced"])
        self.assertRegex(result["interaction_planner_sha256"], r"^[0-9a-f]{64}$")
        plan = result["plan"]
        self.assertEqual(plan["summary"]["raw_cell_count"], 4)
        self.assertEqual(plan["summary"]["unique_topology_count"], 4)
        self.assertEqual(plan["summary"]["generate_and_compile_count"], 4)
        self.assertTrue(
            any(cell["interaction_order"] == 2 for cell in plan["cells"])
        )
        self.assertEqual(request_path.read_bytes(), request_before)

        argv = [
            "--root",
            str(self.root),
            "interaction-plan",
            "--request",
            str(request_path),
        ]
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(module.main(argv), 0)
        self.assertEqual(json.loads(output.getvalue()), result)

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "factorial-plan",
                "--request",
                str(request_path),
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout), result)

    def _job_script(self, name: str = "probe.py", body: str | None = None) -> Path:
        path = self.root / name
        path.write_text(
            textwrap.dedent(
                body
                or """
                import json, os, pathlib, sys, time
                output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
                output.mkdir(parents=True, exist_ok=True)
                time.sleep(float(sys.argv[1]) if len(sys.argv) > 1 else 0.05)
                (output / "result.json").write_text(json.dumps({"readonly": os.environ.get("MATCH_WORKBENCH_READ_ONLY")}), encoding="utf-8")
                print("probe-ok", flush=True)
                """
            ),
            encoding="utf-8",
        )
        return path

    def _jobs(
        self,
        script: Path,
        *,
        resource_class: str = "read_only_subprocess",
        job_ids: tuple[str, ...] = ("j1",),
        distinct: bool = False,
        max_output_bytes: int | None = None,
        timeout_seconds: int = 10,
        env: dict[str, str] | None = None,
    ) -> Path:
        jobs: list[dict[str, object]] = []
        for job_id in job_ids:
            job: dict[str, object] = {
                "job_id": job_id,
                "kind": "safe-probe",
                "resource_class": resource_class,
                "executable": _descriptor(Path(sys.executable)),
                # Include a harmless per-job argument when requested so the
                # fingerprints differ and the bounded executor has real
                # independent work to schedule.  With ``distinct=False``
                # aliases intentionally collapse to one fingerprint.
                "argv": [str(script), "0.20", job_id] if distinct else [str(script)],
                "cwd": str(self.root),
                "inputs": [_descriptor(script)],
                "outputs": ["result.json"],
                "timeout_seconds": timeout_seconds,
            }
            if max_output_bytes is not None:
                job["max_output_bytes"] = max_output_bytes
            if env is not None:
                job["env"] = dict(env)
            jobs.append(job)
        path = self.root / f"jobs-{resource_class}.json"
        _write_json(path, {"schema": module.JOBS_SCHEMA, "schema_version": 1, "jobs": jobs})
        return path

    def test_init_self_hash_idempotency_and_immutable_conflict(self) -> None:
        first = self._init()
        self.assertEqual(first["status"], "initialized")
        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        self.assertEqual(session["session_sha256"], first["session"]["session_sha256"])
        body = dict(session)
        body["authority_advanced"] = True
        session_path.write_text(json.dumps(body), encoding="utf-8")
        with self.assertRaisesRegex(module.MatchError, "self-hash mismatch"):
            module.lookup_matches(self.root, self.workspace, self.source)

        # Restore a clean workspace and prove a repeat is a no-op.
        session_path.write_text(json.dumps(first["session"], separators=(",", ":")), encoding="utf-8")
        second = self._init()
        self.assertEqual(second["status"], "unchanged")
        conflict = self.root / "conflict.json"
        _write_json(conflict, self._manifest(session_id="session-2"))
        with self.assertRaisesRegex(module.MatchError, "different immutable session"):
            module.init_workspace(self.root, conflict, self.workspace)

    def test_target_and_session_descriptor_mutation_fail_closed(self) -> None:
        self._init()
        original_target = self.target.read_bytes()
        self.target.write_bytes(b"mutated-target")
        with self.assertRaisesRegex(module.MatchError, "session target changed"):
            module.lookup_matches(self.root, self.workspace, self.source)
        self.target.write_bytes(original_target)

        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        session["request"]["target"]["sha256"] = "0" * 64
        body = dict(session)
        body.pop("session_sha256", None)
        canonical = json.dumps(body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        session["session_sha256"] = hashlib.sha256(canonical.encode("utf-8")).hexdigest()
        session_path.write_text(json.dumps(session, separators=(",", ":")), encoding="utf-8")
        with self.assertRaisesRegex(module.MatchError, "session target changed"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_target_cas_mutation_fails_closed(self) -> None:
        initialized = self._init()
        target_cas = self.workspace / initialized["session"]["target_blob"]["cas_path"]
        target_cas.write_bytes(b"tampered-target-cas")
        with self.assertRaisesRegex(module.MatchError, "session target CAS"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_repair_requires_authenticated_target_parent_identity(self) -> None:
        self._init()
        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        session.pop("target_parent_identity")
        _write_json(session_path, _rehash(session, "session_sha256"))
        with self.assertRaisesRegex(
            module.MatchError, "session target parent identity is missing"
        ):
            module.repair_target(self.root, self.workspace)

    def test_repair_target_restores_mutation_and_second_repair_is_unchanged(self) -> None:
        self._init()
        self.target.write_bytes(b"mutated-target")
        with self.assertRaisesRegex(module.MatchError, "session target changed"):
            module.lookup_matches(self.root, self.workspace, self.source)

        repaired = module.repair_target(self.root, self.workspace)
        self.assertEqual(repaired["status"], "restored")
        self.assertFalse(repaired["authority_advanced"])
        self.assertEqual(self.target.read_bytes(), b"target-bytes")
        self.assertEqual(
            module.lookup_matches(self.root, self.workspace, self.source)["status"],
            "new",
        )

        repeated = module.repair_target(self.root, self.workspace)
        self.assertEqual(repeated["status"], "unchanged")
        self.assertFalse(repeated["authority_advanced"])
        self.assertEqual(repeated["target"], repaired["target"])

    def test_repair_target_restores_missing_target(self) -> None:
        self._init()
        self.target.unlink()
        repaired = module.repair_target(self.root, self.workspace)
        self.assertEqual(repaired["status"], "restored")
        self.assertFalse(repaired["authority_advanced"])
        self.assertEqual(self.target.read_bytes(), b"target-bytes")
        self.assertEqual(module._sha256_file(self.target), repaired["target"]["sha256"])

    def test_repair_target_parent_replacement_cannot_redirect_write(self) -> None:
        target_parent = self.root / "target-parent"
        target_parent.mkdir()
        target = target_parent / "target.o"
        target.write_bytes(b"target-bytes")
        _write_json(self.manifest, self._manifest(target=_descriptor(target)))
        workspace = self.root / "build" / "match-parent-replacement"
        module.init_workspace(self.root, self.manifest, workspace)
        target.write_bytes(b"mutated-target")

        displaced_parent = self.root / "displaced-target-parent"
        redirected_target = target_parent / "target.o"
        swap_attempted = False
        swap_blocked = False
        real_replace = module.os.replace

        def swapping_replace(src: object, dst: object, *args: object, **kwargs: object) -> None:
            nonlocal swap_attempted, swap_blocked
            if not swap_attempted:
                swap_attempted = True
                try:
                    target_parent.rename(displaced_parent)
                except OSError:
                    # Windows directory handles held without delete sharing
                    # must make the path replacement itself fail.
                    swap_blocked = True
                else:
                    target_parent.mkdir()
                    redirected_target.write_bytes(b"outside-target")
            real_replace(src, dst, *args, **kwargs)

        with mock.patch.object(module.os, "replace", side_effect=swapping_replace):
            if module.os.name == "nt":
                repaired = module.repair_target(self.root, workspace)
                self.assertEqual(repaired["status"], "restored")
                self.assertTrue(swap_blocked)
                self.assertEqual(target.read_bytes(), b"target-bytes")
            else:
                with self.assertRaises(module.MatchError):
                    module.repair_target(self.root, workspace)
                self.assertTrue(swap_attempted)
                self.assertEqual(redirected_target.read_bytes(), b"outside-target")

    def test_repair_target_parent_replacement_before_pin_fails_closed(self) -> None:
        target_parent = self.root / "target-parent-before-pin"
        target_parent.mkdir()
        target = target_parent / "target.o"
        target.write_bytes(b"target-bytes")
        _write_json(self.manifest, self._manifest(target=_descriptor(target)))
        workspace = self.root / "build" / "match-parent-before-pin"
        module.init_workspace(self.root, self.manifest, workspace)
        target.write_bytes(b"mutated-target")

        displaced_parent = self.root / "displaced-target-parent-before-pin"
        method_name = "_open_windows" if module.os.name == "nt" else "_open_posix"
        real_open = getattr(module._PinnedTargetParent, method_name)
        swap_attempted = False

        def swapping_open(parent: object) -> object:
            nonlocal swap_attempted
            swap_attempted = True
            target_parent.rename(displaced_parent)
            target_parent.mkdir()
            return real_open(parent)

        with mock.patch.object(
            module._PinnedTargetParent,
            method_name,
            autospec=True,
            side_effect=swapping_open,
        ):
            with self.assertRaisesRegex(
                module.MatchError, "session target parent changed before pin"
            ):
                module.repair_target(self.root, workspace)

        self.assertTrue(swap_attempted)
        self.assertEqual(
            (displaced_parent / "target.o").read_bytes(), b"mutated-target"
        )
        self.assertFalse((target_parent / "target.o").exists())

    def test_repair_target_parent_replacement_after_session_auth_fails_closed(self) -> None:
        target_parent = self.root / "target-parent-after-session-auth"
        target_parent.mkdir()
        target = target_parent / "target.o"
        target.write_bytes(b"target-bytes")
        _write_json(self.manifest, self._manifest(target=_descriptor(target)))
        workspace = self.root / "build" / "match-parent-after-session-auth"
        module.init_workspace(self.root, self.manifest, workspace)
        target.write_bytes(b"mutated-target")

        displaced_parent = self.root / "displaced-target-parent-after-session-auth"
        real_load_session = module._load_session
        swap_attempted = False

        def swapping_load_session(*args: object, **kwargs: object) -> object:
            nonlocal swap_attempted
            session = real_load_session(*args, **kwargs)
            swap_attempted = True
            target_parent.rename(displaced_parent)
            target_parent.mkdir()
            return session

        with mock.patch.object(
            module, "_load_session", side_effect=swapping_load_session
        ):
            with self.assertRaisesRegex(
                module.MatchError,
                "session target parent changed from its authenticated identity|"
                "session target parent changed before pin",
            ):
                module.repair_target(self.root, workspace)

        self.assertTrue(swap_attempted)
        self.assertEqual(
            (displaced_parent / "target.o").read_bytes(), b"mutated-target"
        )
        self.assertFalse((target_parent / "target.o").exists())

    def test_directory_identity_accepts_normal_posix_link_count(self) -> None:
        directory = self.root / "normal-directory"
        directory.mkdir()
        (directory / "child-directory").mkdir()
        identity = module._directory_identity(directory, "normal directory")
        self.assertEqual(identity, (directory.stat().st_dev, directory.stat().st_ino))

    def test_repair_target_rejects_corrupt_cas(self) -> None:
        initialized = self._init()
        target_cas = self.workspace / initialized["session"]["target_blob"]["cas_path"]
        target_cas.write_bytes(b"tampered-target-cas")
        self.target.write_bytes(b"mutated-target")
        with self.assertRaisesRegex(module.MatchError, "session target CAS"):
            module.repair_target(self.root, self.workspace)

    def test_repair_target_reauthenticates_compiler_and_compile_inputs(self) -> None:
        compiler = self.root / "compiler.bin"
        wrapper = self.root / "wrapper.bin"
        compile_input = self.root / "compile-input.h"
        compiler.write_bytes(b"compiler")
        wrapper.write_bytes(b"wrapper")
        compile_input.write_bytes(b"#define VALUE 1\n")
        manifest_value = self._manifest()
        manifest_value["context"] = {
            "base_commit": "abcdef1234567890",
            "toolchain_key": "GC/1.3.2",
            "compiler": _descriptor(compiler),
            "compile_argv": [str(compiler)],
            "compile_cwd": str(self.root),
            "compile_tools": [_descriptor(wrapper)],
            "compile_inputs": [_descriptor(compile_input)],
            "context_complete": True,
        }
        _write_json(self.manifest, manifest_value)
        workspace = self.root / "build" / "match-context"
        module.init_workspace(self.root, self.manifest, workspace)
        self.target.unlink()
        compile_input.write_bytes(b"#define VALUE 2\n")
        with self.assertRaisesRegex(module.MatchError, "session compile input 0"):
            module.repair_target(self.root, workspace)

    def test_complete_context_requires_cwd_and_authenticates_tool_chain(self) -> None:
        compiler = self.root / "compiler.bin"
        wrapper = self.root / "wrapper.bin"
        compile_input = self.root / "compile-input.h"
        compiler.write_bytes(b"compiler")
        wrapper.write_bytes(b"wrapper")
        compile_input.write_bytes(b"#define VALUE 1\n")
        manifest_value = self._manifest()
        manifest_value["context"] = {
            "base_commit": "abcdef1234567890",
            "toolchain_key": "GC/2.6",
            "compiler": _descriptor(wrapper),
            "compile_argv": [str(compiler), "-c", str(self.source)],
            "compile_tools": [_descriptor(compiler)],
            "compile_inputs": [_descriptor(self.source), _descriptor(compile_input)],
            "context_complete": True,
        }
        _write_json(self.manifest, manifest_value)
        with self.assertRaisesRegex(module.MatchError, "compile_cwd"):
            self._init()

        manifest_value["context"]["compile_cwd"] = str(self.root)
        _write_json(self.manifest, manifest_value)
        initialized = self._init()
        context = initialized["session"]["request"]["context"]
        self.assertFalse(module._compile_context_complete(context))
        self.assertEqual(Path(context["compile_cwd"]["path"]), self.root.resolve())
        self._record()
        self.assertFalse(
            module.lookup_matches(self.root, self.workspace, self.source)["skip_compile"]
        )

        compiler.write_bytes(b"mutated-compiler")
        with self.assertRaisesRegex(module.MatchError, "session compile tool 0"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_complete_context_seals_every_reuse_gate_and_mutations_fail_closed(self) -> None:
        context = self._complete_context()
        self.assertTrue(module._compile_context_complete(context))

        def clone() -> dict[str, object]:
            return json.loads(json.dumps(context))

        mutations = {
            "compiler identity": lambda value: value["compiler"]["identity"].update({"inode": 0}),
            "dependency freshness": lambda value: value["dependency_provenance"].update({"fresh": False}),
            "dependency path set": lambda value: value["dependency_provenance"]["input_paths"].pop(),
            "build rule": lambda value: value["build_rule"].update({"sha256": "0" * 64}),
            "include tree": lambda value: value["include_roots"][0].update({"tree_name_fingerprint": "0" * 64}),
            "environment": lambda value: value["environment"].update({"codepage": "forged"}),
            "runtime DLL": lambda value: value["runtime_dlls"].clear(),
            "output binding": lambda value: value["compile_outputs"]["output"].update({"sha256": "0" * 64}),
            "source tree state": lambda value: value["source_tree"].update({"state": "dirty", "dirty_patch": None}),
            "cwd identity": lambda value: value["compile_cwd"]["identity"].update({"inode": 0}),
        }
        for label, mutate in mutations.items():
            with self.subTest(label=label):
                mutated = clone()
                mutate(mutated)
                self.assertFalse(module._compile_context_complete(mutated))

        manifest_value = self._manifest()
        manifest_value["context"] = context
        _write_json(self.manifest, manifest_value)
        self._init()
        self._record()
        self.assertTrue(module.lookup_matches(self.root, self.workspace, self.source)["skip_compile"])

        runtime = self.root / "complete-runtime.dll"
        runtime.write_bytes(b"runtime-mutated")
        with self.assertRaisesRegex(module.MatchError, "session runtime DLL 0"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_complete_context_seals_environment_and_dirty_patch_identity(self) -> None:
        context = self._complete_context(dirty=True)
        self.assertTrue(module._compile_context_complete(context))
        patch = self.root / "complete-dirty.patch"
        patch.write_bytes(b"forged patch")
        self.assertFalse(module._compile_context_complete(context))

        context = self._complete_context()
        manifest_value = self._manifest()
        manifest_value["context"] = context
        _write_json(self.manifest, manifest_value)
        self._init()
        self._record()
        with mock.patch.dict(os.environ, {"PATH": "forged-path"}, clear=False):
            with self.assertRaisesRegex(module.MatchError, "session environment"):
                module.lookup_matches(self.root, self.workspace, self.source)

    def test_response_file_argv_requires_expanded_authenticated_binding(self) -> None:
        context = self._complete_context(response_file=True)
        context.pop("argv_binding")
        manifest_value = self._manifest()
        manifest_value["context"] = context
        _write_json(self.manifest, manifest_value)
        with self.assertRaisesRegex(module.MatchError, "argv_binding"):
            self._init(workspace=self.root / "build" / "response-missing")

        context = self._complete_context(response_file=True)
        manifest_value["context"] = context
        _write_json(self.manifest, manifest_value)
        initialized = self._init(workspace=self.root / "build" / "response-bound")
        self.assertTrue(module._compile_context_complete(initialized["session"]["request"]["context"]))

    def test_complete_context_rejects_same_path_replacement_of_compile_cwd(self) -> None:
        compiler = self.root / "compiler.bin"
        compile_input = self.root / "compile-input.h"
        compile_cwd = self.root / "compiler-cwd"
        compiler.write_bytes(b"compiler")
        compile_input.write_bytes(b"#define VALUE 1\n")
        compile_cwd.mkdir()
        manifest_value = self._manifest()
        manifest_value["context"] = {
            "base_commit": "abcdef1234567890",
            "toolchain_key": "GC/2.6",
            "compiler": _descriptor(compiler),
            "compile_argv": [str(compiler), "-c", str(self.source)],
            "compile_cwd": str(compile_cwd),
            "compile_inputs": [_descriptor(self.source), _descriptor(compile_input)],
            "context_complete": True,
        }
        _write_json(self.manifest, manifest_value)
        self._init()

        original = self.root / "original-compiler-cwd"
        compile_cwd.rename(original)
        compile_cwd.mkdir()
        with self.assertRaisesRegex(module.MatchError, "session compile cwd changed"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_legacy_context_shape_remains_readable_but_cannot_claim_executable_reuse(self) -> None:
        manifest_value = self._manifest()
        normalized = module._request(manifest_value, root=self.root)
        self.assertNotIn("compile_cwd", normalized["context"])
        self.assertNotIn("compile_tools", normalized["context"])
        self.assertFalse(module._compile_context_complete(normalized["context"]))
        # A v3 session persisted a normalized cwd object with only the
        # device/inode pair; it must remain byte-for-byte readable.
        self.assertEqual(module._request(normalized, root=self.root), normalized)

        compiler = self.root / "legacy-compiler.bin"
        compiler.write_bytes(b"compiler")
        legacy_executable = {
            "base_commit": "abcdef1234567890",
            "toolchain_key": "GC/2.6",
            "compiler": _descriptor(compiler),
            "compile_argv": [str(compiler)],
            "compile_inputs": [_descriptor(self.source)],
            "context_complete": False,
        }
        manifest_value["context"] = legacy_executable
        _write_json(self.manifest, manifest_value)
        initialized = self._init()
        context = initialized["session"]["request"]["context"]
        self.assertNotIn("compile_cwd", context)
        self.assertNotIn("compile_tools", context)
        self.assertFalse(module._compile_context_complete(context))

    def test_repair_target_rejects_target_indirection_and_hardlink(self) -> None:
        self._init()
        replacement = self.root / "replacement-target.o"
        replacement.write_bytes(b"replacement-target")
        self.target.unlink()
        try:
            self.target.symlink_to(replacement)
        except (OSError, NotImplementedError) as exc:
            self.skipTest(f"symlink creation unavailable: {exc}")
        with self.assertRaisesRegex(module.MatchError, "indirection"):
            module.repair_target(self.root, self.workspace)

        self.target.unlink()
        os.link(replacement, self.target)
        with self.assertRaisesRegex(module.MatchError, "hard link"):
            module.repair_target(self.root, self.workspace)

    def test_repair_target_rejects_self_hashed_request_target_rebinding(self) -> None:
        self._init()
        self.target.unlink()
        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        session["request"]["target"]["sha256"] = "0" * 64
        _write_json(session_path, _rehash(session, "session_sha256"))
        with self.assertRaisesRegex(module.MatchError, "session target CAS is not bound"):
            module.repair_target(self.root, self.workspace)

    def test_self_hashed_session_cannot_rebind_target_away_from_manifest(self) -> None:
        self._init()
        replacement = self.root / "replacement-target.o"
        replacement.write_bytes(b"replacement-target")
        replacement_descriptor = _descriptor(replacement)
        replacement_relative = (
            f"cas/blobs/target/{replacement_descriptor['sha256'][:2]}/"
            f"{replacement_descriptor['sha256']}.bin"
        )
        replacement_cas = self.workspace / replacement_relative
        replacement_cas.parent.mkdir(parents=True, exist_ok=True)
        replacement_cas.write_bytes(replacement.read_bytes())
        session_path = self.workspace / "session.json"
        session = json.loads(session_path.read_text(encoding="utf-8"))
        session["request"]["target"] = replacement_descriptor
        session["target_blob"] = {
            "kind": "target",
            "sha256": replacement_descriptor["sha256"],
            "size_bytes": replacement_descriptor["size_bytes"],
            "cas_path": replacement_relative,
            "dedup_hit": False,
        }
        _write_json(session_path, _rehash(session, "session_sha256"))
        with self.assertRaisesRegex(module.MatchError, "request manifest"):
            module.lookup_matches(self.root, self.workspace, self.source)

    def test_manifest_duplicate_key_unknown_field_and_descriptor_mismatch_rejected(self) -> None:
        duplicate = self.root / "duplicate.json"
        duplicate.write_text(
            '{"schema":"match_workbench_request/v1","schema":"match_workbench_request/v1"}\n',
            encoding="utf-8",
        )
        with self.assertRaisesRegex(module.MatchError, "duplicate JSON key"):
            module.init_workspace(self.root, duplicate, self.workspace)

        unknown = self._manifest()
        unknown["unexpected"] = True
        unknown_path = self.root / "unknown.json"
        _write_json(unknown_path, unknown)
        with self.assertRaisesRegex(module.MatchError, "unknown field"):
            module.init_workspace(self.root, unknown_path, self.workspace)

        mismatch = self._manifest()
        mismatch["target"] = {**_descriptor(self.target), "size_bytes": self.target.stat().st_size + 1}
        mismatch_path = self.root / "mismatch.json"
        _write_json(mismatch_path, mismatch)
        with self.assertRaisesRegex(module.MatchError, "descriptor mismatch"):
            module.init_workspace(self.root, mismatch_path, self.workspace)

        with self.assertRaisesRegex(module.MatchError, "workspace must stay beneath"):
            module.init_workspace(self.root, self.manifest, self.root.parent / "outside-workbench")

    def test_descriptor_rejects_hardlink(self) -> None:
        hardlink = self.root / "target-hardlink.o"
        os.link(self.target, hardlink)
        manifest = self._manifest()
        manifest["target"] = _descriptor(hardlink)
        path = self.root / "hardlink.json"
        _write_json(path, manifest)
        with self.assertRaisesRegex(module.MatchError, "hard link"):
            module.init_workspace(self.root, path, self.workspace)

    def test_workbench_lock_rejects_hardlink_alias(self) -> None:
        self.workspace.mkdir(parents=True)
        victim = self.root / "lock-victim"
        victim.write_bytes(b"")
        os.link(victim, self.workspace / ".workbench.lock")
        with self.assertRaisesRegex(module.MatchError, "lock must have exactly one hard link"):
            module.init_workspace(self.root, self.manifest, self.workspace)

    def test_descriptor_rejects_symlink_when_supported(self) -> None:
        symlink = self.root / "target-symlink.o"
        try:
            symlink.symlink_to(self.target)
        except (OSError, NotImplementedError) as exc:
            self.skipTest(f"symlink creation unavailable: {exc}")
        symlink_manifest = self._manifest()
        symlink_manifest["target"] = {"path": str(symlink), **{k: _descriptor(self.target)[k] for k in ("size_bytes", "sha256")}}
        symlink_path = self.root / "symlink.json"
        _write_json(symlink_path, symlink_manifest)
        with self.assertRaisesRegex(module.MatchError, "indirection"):
            module.init_workspace(self.root, symlink_path, self.root / "build" / "symlink")

    def test_record_cas_reports_deterministic_gzip_and_idempotency(self) -> None:
        self._init()
        first = self._record(data_report=self.data)
        self.assertEqual(first["status"], "recorded")
        record = first["record"]
        body = dict(record)
        claimed = body.pop("record_sha256")
        canonical = json.dumps(body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        self.assertEqual(claimed, hashlib.sha256(canonical.encode("utf-8")).hexdigest())
        for kind in ("source_blob", "object_blob"):
            blob = self.workspace / record[kind]["cas_path"]
            self.assertTrue(blob.is_file())
            self.assertEqual(_sha256(blob), record[kind]["sha256"])
        strict_info = record["reports"]["strict"]
        data_info = record["reports"]["data"]
        self.assertEqual(strict_info["codec"], "gzip")
        self.assertEqual(strict_info["raw_sha256"], _sha256(self.strict))
        self.assertEqual(data_info["raw_sha256"], _sha256(self.data))
        for info in (strict_info, data_info):
            cached = self.workspace / info["cas_path"]
            with gzip.open(cached, "rb") as stream:
                self.assertEqual(stream.read(), (self.strict if info is strict_info else self.data).read_bytes())
            self.assertEqual(info["compressed_size_bytes"], cached.stat().st_size)

        repeated = self._record(data_report=self.data)
        self.assertEqual(repeated["status"], "unchanged")
        self.assertEqual(repeated["record"], record)

        source2 = self.root / "candidate-copy.c"
        source2.write_bytes(self.source.read_bytes())
        object2 = self.root / "candidate-copy.o"
        object2.write_bytes(self.object.read_bytes())
        strict2 = self.root / "strict-copy.json"
        strict2.write_bytes(self.strict.read_bytes())
        duplicate = self._record("c2", source=source2, object_path=object2, strict_report=strict2, data_report=self.data)
        self.assertEqual(duplicate["status"], "duplicate")
        self.assertEqual(duplicate["record"]["duplicate_of"], "c1")
        self.assertNotEqual(
            duplicate["record"]["source_context_key"], record["source_context_key"]
        )
        self.assertTrue(duplicate["record"]["reports"]["strict"]["dedup_hit"])
        self.assertEqual(duplicate["record"]["reports"]["strict"]["cas_path"], strict_info["cas_path"])

        jobs = self._jobs(self._job_script(), job_ids=("reuse",))
        module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        reused = module.diagnose_candidate(self.root, self.workspace, "c2", jobs)
        self.assertEqual(reused["summary"], {"ran": 1, "cached": 0, "failed": 0})
        self.assertEqual(reused["jobs"][0]["cache_status"], "ran")

    def test_candidate_guard_rejects_extracted_target_layout(self) -> None:
        self._init()
        extracted_target = (
            self.root / "build" / "capsule-v376" / "GP6E01" / "obj" / "board" / "capsule.o"
        )
        extracted_target.parent.mkdir(parents=True)
        extracted_target.write_bytes(b"extracted-target-bytes")

        with self.assertRaisesRegex(module.MatchError, "target role.*candidate/donor role"):
            self._record("extracted-target", object_path=extracted_target)

    def test_candidate_guard_rejects_target_hash_at_candidate_path(self) -> None:
        self._init()
        candidate_root = self.root / "build" / "capsule-v376" / "GP6E01" / "src" / "board"
        candidate_root.mkdir(parents=True)
        candidate_source = candidate_root / "capsule.c"
        candidate_source.write_bytes(self.source.read_bytes())
        candidate_object = candidate_root / "capsule.o"
        candidate_object.write_bytes(self.target.read_bytes())

        with self.assertRaisesRegex(
            module.MatchError,
            "candidate/donor path role candidate.*already registered with target role",
        ):
            self._record(
                "target-hash",
                source=candidate_source,
                object_path=candidate_object,
            )

    def test_missing_or_corrupt_report_cas_fails_matrix(self) -> None:
        self._init()
        recorded = self._record(data_report=self.data)
        strict_cas = self.workspace / recorded["record"]["reports"]["strict"]["cas_path"]
        strict_cas.unlink()
        with self.assertRaisesRegex(module.MatchError, "report"):
            module.build_matrix(self.root, self.workspace)

        second_workspace = self.root / "build" / "match-corrupt"
        module.init_workspace(self.root, self.manifest, second_workspace)
        second = module.record_candidate(
            self.root,
            second_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            compile_attestation=self._attestation(
                "corrupt-report-c1", workspace=second_workspace
            ),
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        data_cas = second_workspace / second["record"]["reports"]["data"]["cas_path"]
        data_cas.write_bytes(b"not-a-gzip-report")
        with self.assertRaisesRegex(module.MatchError, "report"):
            module.build_matrix(self.root, second_workspace)

    def test_lookup_source_and_object_indexes_skip_work(self) -> None:
        self._init()
        self.assertEqual(module.lookup_matches(self.root, self.workspace, self.source)["status"], "new")
        first = self._record()
        normalized_alias = module.lookup_matches(
            self.root, self.workspace, self.source.relative_to(self.root)
        )
        self.assertEqual(normalized_alias["status"], "known_source")
        self.assertEqual(
            normalized_alias["source_context_key"], first["record"]["source_context_key"]
        )
        source_copy = self.root / "source-copy.c"
        source_copy.write_bytes(self.source.read_bytes())
        object_other = self.root / "other.o"
        object_other.write_bytes(b"different-object")
        source_hit = module.lookup_matches(self.root, self.workspace, source_copy, object_other)
        self.assertEqual(source_hit["status"], "new")
        self.assertFalse(source_hit["skip_compile"])
        self.assertFalse(source_hit["skip_diagnostics"])
        self.assertNotEqual(source_hit["source_context_key"], first["record"]["source_context_key"])

        # The same bytes are a distinct compile context when the compiler sees
        # a different source path/basename, so a different object is permitted.
        second = self._record("c2", source=source_copy, object_path=object_other)
        self.assertEqual(second["status"], "recorded")
        self.assertNotEqual(
            second["record"]["source_context_key"], first["record"]["source_context_key"]
        )

        object_third = self.root / "third.o"
        object_third.write_bytes(b"third-object")
        original_path_conflict = module.lookup_matches(
            self.root, self.workspace, self.source, object_third
        )
        self.assertEqual(original_path_conflict["status"], "known_source")
        self.assertIn("frozen compile context is incomplete", original_path_conflict["reason"])

        other_source = self.root / "other.c"
        other_source.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object_hit = module.lookup_matches(self.root, self.workspace, other_source, self.object)
        self.assertEqual(object_hit["status"], "known_object")
        self.assertFalse(object_hit["skip_compile"])
        self.assertFalse(object_hit["skip_diagnostics"])
        self.assertEqual(object_hit["diagnostic_reuse_candidate_id"], "c1")

    def test_incomplete_real_compiler_context_cannot_mint_new_evidence(self) -> None:
        compiler = self.root / "compiler.bin"
        compiler.write_bytes(b"compiler")
        manifest = self._manifest()
        manifest["context"] = {
            "base_commit": "abcdef1234567890",
            "toolchain_key": "GC/2.6",
            "compiler": _descriptor(compiler),
            "compile_argv": [str(compiler)],
            "compile_inputs": [],
        }
        _write_json(self.manifest, manifest)
        self._init()
        with self.assertRaisesRegex(
            module.MatchError, "authenticated wrapper/tool chain"
        ):
            self._attestation("incomplete-context")
        lookup = module.lookup_matches(self.root, self.workspace, self.source)
        self.assertEqual(lookup["status"], "new")
        self.assertFalse(lookup["skip_compile"])
        self.assertFalse(
            json.loads((self.workspace / "session.json").read_text(encoding="utf-8"))["request"]["context"]["context_complete"]
        )

    def test_record_rejects_cross_toolchain_attestation_before_mutation(self) -> None:
        context_a = self._complete_context()
        manifest_a = self._manifest(session_id="context-a")
        manifest_a["context"] = context_a
        _write_json(self.manifest, manifest_a)
        workspace_a = self.root / "build" / "context-a"
        module.init_workspace(self.root, self.manifest, workspace_a)

        compiler_b = self.root / "compiler-b.bin"
        compiler_b.write_bytes(b"compiler-b")
        context_b = json.loads(json.dumps(context_a))
        context_b["toolchain_key"] = "GC/context-b"
        context_b["compiler"] = _descriptor(compiler_b)
        context_b["compile_argv"][0] = str(compiler_b)
        manifest_b_path = self.root / "manifest-b.json"
        manifest_b = self._manifest(session_id="context-b")
        manifest_b["context"] = context_b
        _write_json(manifest_b_path, manifest_b)
        workspace_b = self.root / "build" / "context-b"
        module.init_workspace(self.root, manifest_b_path, workspace_b)
        attestation_b = self._attestation(
            "context-b-candidate", workspace=workspace_b
        )

        with self.assertRaisesRegex(
            module.MatchError, "compiler/wrapper/argv context does not match"
        ):
            module.record_candidate(
                self.root,
                workspace_a,
                candidate_id="cross-context",
                source=self.source,
                object_path=self.object,
                compile_attestation=attestation_b,
                strict_report=self.strict,
                data_report=self.data,
                hypothesis="cross-context evidence",
                axis="compiler-context",
            )
        self.assertEqual(list((workspace_a / "candidates").glob("*.json")), [])
        self.assertEqual(
            json.loads((workspace_a / "index.json").read_text(encoding="utf-8"))["sequence"],
            0,
        )

    def test_compile_attestation_requires_the_exact_session_argv(self) -> None:
        manifest = self._manifest(session_id="argv-binding")
        manifest["context"] = self._complete_context()
        _write_json(self.manifest, manifest)
        self._init()
        with self.assertRaisesRegex(
            module.MatchError, "exactly equal the immutable session compile_argv"
        ):
            module.create_compile_attestation(
                self.root,
                self.workspace,
                source=self.source,
                object_path=self.object,
                output=self.root / "wrong-argv-attestation.json",
                producer_kind="external-compile-attestation",
                producer_command=["different-compiler", "-c", str(self.source)],
            )
        self.assertFalse((self.root / "wrong-argv-attestation.json").exists())

    def test_provenance_audit_flags_unattested_and_cross_context_records(self) -> None:
        context_a = self._complete_context()
        manifest_a = self._manifest(session_id="audit-a")
        manifest_a["context"] = context_a
        _write_json(self.manifest, manifest_a)
        workspace_a = self.root / "build" / "audit-a"
        module.init_workspace(self.root, self.manifest, workspace_a)
        record_a = module.record_candidate(
            self.root,
            workspace_a,
            candidate_id="legacy",
            source=self.source,
            object_path=self.object,
            compile_attestation=self._attestation(
                "audit-a-legacy", workspace=workspace_a
            ),
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="legacy candidate",
            axis="compiler-context",
        )["record"]

        candidate_path = workspace_a / "candidates" / "legacy.json"
        legacy = json.loads(candidate_path.read_text(encoding="utf-8"))
        legacy.pop("compile_attestation")
        legacy = _rehash(legacy, "record_sha256")
        _write_json(candidate_path, legacy)
        index_path = workspace_a / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["last_record_sha256"] = legacy["record_sha256"]
        _write_json(index_path, _rehash(index, "index_sha256"))
        audit = module.audit_candidate_provenance(self.root, workspace_a)
        self.assertEqual(audit["status"], "requires_migration")
        self.assertEqual(audit["counts"], {
            "context_match": 0,
            "cross_context": 0,
            "unattested": 1,
        })
        self.assertEqual(audit["rows"][0]["ordinal"], record_a["ordinal"])

        compiler_b = self.root / "audit-compiler-b.bin"
        compiler_b.write_bytes(b"compiler-b")
        context_b = json.loads(json.dumps(context_a))
        context_b["toolchain_key"] = "GC/audit-b"
        context_b["compiler"] = _descriptor(compiler_b)
        context_b["compile_argv"][0] = str(compiler_b)
        manifest_b_path = self.root / "audit-manifest-b.json"
        manifest_b = self._manifest(session_id="audit-b")
        manifest_b["context"] = context_b
        _write_json(manifest_b_path, manifest_b)
        workspace_b = self.root / "build" / "audit-b"
        module.init_workspace(self.root, manifest_b_path, workspace_b)
        attestation_b = self._attestation(
            "audit-b-legacy", workspace=workspace_b
        )
        provenance_path = self.root / "audit-provenance.json"
        _write_json(
            provenance_path,
            _rehash(
                {
                    "schema": module.PROVENANCE_MANIFEST_SCHEMA,
                    "schema_version": 1,
                    "candidates": [
                        {"candidate_id": "legacy", "attestation": str(attestation_b)}
                    ],
                },
                "manifest_sha256",
            ),
        )
        cross = module.audit_candidate_provenance(
            self.root, workspace_a, manifest=provenance_path
        )
        self.assertEqual(cross["counts"]["cross_context"], 1)
        self.assertEqual(cross["rows"][0]["actual_toolchain_key"], "GC/audit-b")
        self.assertEqual(cross["rows"][0]["evidence"], "external_manifest")

    def test_provenance_migration_preserves_attempts_and_duplicate_relations(self) -> None:
        context_a = self._complete_context()
        manifest_a = self._manifest(session_id="migration-a")
        manifest_a["context"] = context_a
        _write_json(self.manifest, manifest_a)
        workspace_a = self.root / "build" / "migration-a"
        module.init_workspace(self.root, self.manifest, workspace_a)
        attestation_a = self._attestation("migration-a", workspace=workspace_a)
        for candidate_id, seconds in (("first", 1.25), ("duplicate", 2.5), ("wrong", 3.75)):
            module.record_candidate(
                self.root,
                workspace_a,
                candidate_id=candidate_id,
                source=self.source,
                object_path=self.object,
                compile_attestation=attestation_a,
                strict_report=self.strict,
                data_report=self.data,
                hypothesis=f"attempt {candidate_id}",
                axis="compiler-context",
                heavy_seconds=seconds,
            )

        compiler_b = self.root / "migration-compiler-b.bin"
        compiler_b.write_bytes(b"compiler-b")
        context_b = json.loads(json.dumps(context_a))
        context_b["toolchain_key"] = "GC/migration-b"
        context_b["compiler"] = _descriptor(compiler_b)
        context_b["compile_argv"][0] = str(compiler_b)
        manifest_b_path = self.root / "migration-manifest-b.json"
        manifest_b = self._manifest(session_id="migration-b")
        manifest_b["context"] = context_b
        _write_json(manifest_b_path, manifest_b)
        workspace_b = self.root / "build" / "migration-b"
        module.init_workspace(self.root, manifest_b_path, workspace_b)
        attestation_b = self._attestation("migration-b", workspace=workspace_b)
        provenance_path = self.root / "migration-provenance.json"
        _write_json(
            provenance_path,
            _rehash(
                {
                    "schema": module.PROVENANCE_MANIFEST_SCHEMA,
                    "schema_version": 1,
                    "candidates": [
                        {"candidate_id": "first", "attestation": str(attestation_b)},
                        {"candidate_id": "duplicate", "attestation": str(attestation_b)},
                        {"candidate_id": "wrong", "attestation": str(attestation_a)},
                    ],
                },
                "manifest_sha256",
            ),
        )
        migrated = module.migrate_candidate_provenance(
            self.root,
            workspace_a,
            workspace_b,
            manifest=provenance_path,
        )
        self.assertEqual(migrated["counts"], {
            "imported": 2,
            "skipped_cross_context": 1,
        })
        matrix = module.build_matrix(self.root, workspace_b)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(set(rows), {"first", "duplicate"})
        self.assertEqual(rows["first"]["heavy_seconds"], 1.25)
        self.assertEqual(rows["duplicate"]["heavy_seconds"], 2.5)
        self.assertEqual(rows["duplicate"]["duplicate_of"], "first")
        repeated = module.migrate_candidate_provenance(
            self.root,
            workspace_a,
            workspace_b,
            manifest=provenance_path,
        )
        self.assertEqual(repeated, migrated)

    def test_compile_provenance_commands_route_directly_and_centrally(self) -> None:
        context = self._complete_context()
        manifest = self._manifest(session_id="provenance-cli-source")
        manifest["context"] = context
        _write_json(self.manifest, manifest)
        source_workspace = self.root / "build" / "provenance-cli-source"
        module.init_workspace(self.root, self.manifest, source_workspace)
        attestation = self.root / "provenance-cli-attestation.json"
        output = io.StringIO()
        argv = [
            "--root",
            str(self.root),
            "attest-compile",
            "--workspace",
            str(source_workspace),
            "--source",
            str(self.source),
            "--object",
            str(self.object),
            "--output",
            str(attestation),
            "--producer-kind",
            "external-compile-attestation",
        ]
        for argument in context["compile_argv"]:
            argv.append(f"--producer-arg={argument}")
        argv.append("--json")
        with contextlib.redirect_stdout(output):
            self.assertEqual(module.main(argv), 0)
        self.assertEqual(json.loads(output.getvalue())["status"], "attested")
        module.record_candidate(
            self.root,
            source_workspace,
            candidate_id="cli-candidate",
            source=self.source,
            object_path=self.object,
            compile_attestation=attestation,
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="CLI provenance candidate",
            axis="compiler-context",
        )
        central = Path(__file__).resolve().parents[1] / "agent.py"
        audit_receipt = self.root / "provenance-cli-audit.json"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "provenance-audit",
                "--workspace",
                str(source_workspace),
                "--output",
                str(audit_receipt),
                "--json",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        audit_result = json.loads(process.stdout)
        self.assertEqual(audit_result["status"], "clean")
        self.assertEqual(
            json.loads(audit_receipt.read_text(encoding="utf-8")), audit_result
        )

        destination_manifest_path = self.root / "provenance-cli-destination.json"
        destination_manifest = self._manifest(session_id="provenance-cli-destination")
        destination_manifest["context"] = context
        _write_json(destination_manifest_path, destination_manifest)
        destination_workspace = self.root / "build" / "provenance-cli-destination"
        module.init_workspace(
            self.root, destination_manifest_path, destination_workspace
        )
        provenance_manifest = self.root / "provenance-cli-manifest.json"
        _write_json(
            provenance_manifest,
            _rehash(
                {
                    "schema": module.PROVENANCE_MANIFEST_SCHEMA,
                    "schema_version": 1,
                    "candidates": [
                        {
                            "candidate_id": "cli-candidate",
                            "attestation": str(attestation),
                        }
                    ],
                },
                "manifest_sha256",
            ),
        )
        migration_receipt = self.root / "provenance-cli-migration.json"
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "provenance-migrate",
                        "--source-workspace",
                        str(source_workspace),
                        "--destination-workspace",
                        str(destination_workspace),
                        "--manifest",
                        str(provenance_manifest),
                        "--output",
                        str(migration_receipt),
                        "--json",
                    ]
                ),
                0,
            )
        migration_result = json.loads(output.getvalue())
        self.assertEqual(migration_result["counts"]["imported"], 1)
        self.assertEqual(
            json.loads(migration_receipt.read_text(encoding="utf-8")),
            migration_result,
        )
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "provenance-migrate",
                        "--source-workspace",
                        str(source_workspace),
                        "--destination-workspace",
                        str(destination_workspace),
                        "--manifest",
                        str(provenance_manifest),
                        "--output",
                        str(migration_receipt),
                        "--json",
                    ]
                ),
                0,
            )
        self.assertEqual(json.loads(output.getvalue()), migration_result)
        self.assertEqual(
            json.loads(migration_receipt.read_text(encoding="utf-8")),
            migration_result,
        )

    def test_materialize_rejects_tampered_candidate_object_cas(self) -> None:
        self._init()
        recorded = self._record()
        object_cas = self.workspace / recorded["record"]["object_blob"]["cas_path"]
        object_cas.write_bytes(b"tampered-object-cas")
        with self.assertRaisesRegex(module.MatchError, "candidate object_blob CAS"):
            module.materialize_candidate_object(
                self.root,
                self.workspace,
                "c1",
                self.source,
                self.root / "tampered-materialized.o",
            )

    def test_materialize_rejects_workbench_destination_and_indirection(self) -> None:
        self._init()
        self._record()
        with self.assertRaisesRegex(module.MatchError, "differ from the source"):
            module.materialize_candidate_object(
                self.root,
                self.workspace,
                "c1",
                self.source,
                self.source,
            )
        with self.assertRaisesRegex(module.MatchError, "outside the workbench"):
            module.materialize_candidate_object(
                self.root,
                self.workspace,
                "c1",
                self.source,
                self.workspace / "should-not-write.o",
            )

        replacement = self.root / "replacement.o"
        replacement.write_bytes(b"replacement")
        destination = self.root / "linked-materialized.o"
        try:
            destination.symlink_to(replacement)
        except (OSError, NotImplementedError) as exc:
            self.skipTest(f"symlink creation unavailable: {exc}")
        with self.assertRaisesRegex(module.MatchError, "indirection"):
            module.materialize_candidate_object(
                self.root,
                self.workspace,
                "c1",
                self.source,
                destination,
            )

    def test_legacy_source_context_record_remains_readable_only_at_its_recorded_path(self) -> None:
        self._init()
        recorded = self._record()
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        candidate.pop("compile_input_identity")
        session = module._load_session(self.workspace, self.root)
        legacy_key = module._legacy_context_key(session, candidate["source"]["sha256"])
        candidate["source_context_key"] = legacy_key
        candidate = _rehash(candidate, "record_sha256")
        _write_json(candidate_path, candidate)

        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["source_context_index"] = {legacy_key: "c1"}
        index["last_record_sha256"] = candidate["record_sha256"]
        _write_json(index_path, _rehash(index, "index_sha256"))

        same_path = module.lookup_matches(self.root, self.workspace, self.source)
        self.assertEqual(same_path["status"], "known_source")
        self.assertEqual(same_path["source_context_key"], legacy_key)
        self.assertEqual(module.build_matrix(self.root, self.workspace)["aggregate"]["candidate_count"], 1)

        copied = self.root / "legacy-copy.c"
        copied.write_bytes(self.source.read_bytes())
        copied_lookup = module.lookup_matches(self.root, self.workspace, copied)
        self.assertEqual(copied_lookup["status"], "new")
        self.assertNotEqual(copied_lookup["source_context_key"], legacy_key)

    def test_lookup_rejects_same_byte_source_replacement_during_identity_check(self) -> None:
        self._init()
        self._record()
        original_loader = module._load_candidate
        replaced = False

        def replace_source(*args: object, **kwargs: object) -> object:
            nonlocal replaced
            if not replaced:
                replacement = self.root / "lookup-replacement.c"
                replacement.write_bytes(self.source.read_bytes())
                os.replace(replacement, self.source)
                replaced = True
            return original_loader(*args, **kwargs)

        with mock.patch.object(module, "_load_candidate", side_effect=replace_source):
            with self.assertRaisesRegex(module.MatchError, "identity changed"):
                module.lookup_matches(self.root, self.workspace, self.source)

    def test_record_rejects_same_byte_source_replacement_before_cas_copy(self) -> None:
        self._init()
        original_load_index = module._load_index
        replaced = False

        def replace_source(*args: object, **kwargs: object) -> object:
            nonlocal replaced
            if not replaced:
                replacement = self.root / "record-replacement.c"
                replacement.write_bytes(self.source.read_bytes())
                os.replace(replacement, self.source)
                replaced = True
            return original_load_index(*args, **kwargs)

        with mock.patch.object(module, "_load_index", side_effect=replace_source):
            with self.assertRaisesRegex(module.MatchError, "identity changed"):
                self._record()
        self.assertFalse((self.workspace / "candidates" / "c1.json").exists())
        self.assertFalse(any(path.name.endswith(".tmp") for path in self.workspace.rglob("*")))

    def test_lookup_fails_closed_for_missing_indexed_record_or_candidate_cas(self) -> None:
        self._init()
        self._record()
        index = json.loads((self.workspace / "index.json").read_text(encoding="utf-8"))
        candidate_path = self.workspace / index["candidates"]["c1"]
        candidate_path.unlink()
        with self.assertRaisesRegex(module.MatchError, "candidate record"):
            module.lookup_matches(self.root, self.workspace, self.source)

        cas_workspace = self.root / "build" / "lookup-cas"
        module.init_workspace(self.root, self.manifest, cas_workspace)
        recorded = module.record_candidate(
            self.root,
            cas_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            compile_attestation=self._attestation(
                "lookup-cas-c1", workspace=cas_workspace
            ),
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        source_cas = cas_workspace / recorded["record"]["source_blob"]["cas_path"]
        source_cas.unlink()
        with self.assertRaisesRegex(module.MatchError, "candidate source_blob CAS|path component does not exist"):
            module.lookup_matches(self.root, cas_workspace, self.source)

        corrupt_workspace = self.root / "build" / "lookup-corrupt-cas"
        module.init_workspace(self.root, self.manifest, corrupt_workspace)
        corrupt = module.record_candidate(
            self.root,
            corrupt_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            compile_attestation=self._attestation(
                "lookup-corrupt-c1", workspace=corrupt_workspace
            ),
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        object_cas = corrupt_workspace / corrupt["record"]["object_blob"]["cas_path"]
        object_cas.write_bytes(b"corrupt-object-cas")
        with self.assertRaisesRegex(module.MatchError, "candidate object_blob CAS"):
            module.lookup_matches(self.root, corrupt_workspace, self.source)

    def test_concurrent_record_calls_have_one_record_and_no_partial_state(self) -> None:
        self._init()
        compile_attestation = self._attestation("concurrent-c1")
        def record() -> dict[str, object]:
            return self._record(compile_attestation=compile_attestation)

        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
            results = list(pool.map(lambda _: record(), range(4)))
        self.assertEqual(sum(result["status"] == "recorded" for result in results), 1)
        self.assertEqual(sum(result["status"] == "unchanged" for result in results), 3)
        candidate_path = self.workspace / "candidates" / "c1.json"
        json.loads(candidate_path.read_text(encoding="utf-8"))
        self.assertFalse(any(path.name.endswith(".tmp") for path in self.workspace.rglob("*")))
        index = json.loads((self.workspace / "index.json").read_text(encoding="utf-8"))
        self.assertEqual(index["sequence"], 1)

    def test_record_rejects_indexed_missing_candidate_and_recovers_exact_final_append(self) -> None:
        self._init()
        first = self._record("c1")
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate_path.unlink()
        with self.assertRaisesRegex(module.MatchError, "immutable candidate index entry"):
            self._record("c1")

        recovery_workspace = self.root / "build" / "recover-append"
        module.init_workspace(self.root, self.manifest, recovery_workspace)
        module.record_candidate(
            self.root,
            recovery_workspace,
            candidate_id="c1",
            source=self.source,
            object_path=self.object,
            compile_attestation=self._attestation(
                "recover-c1", workspace=recovery_workspace
            ),
            strict_report=self.strict,
            data_report=self.data,
            hypothesis="natural candidate",
            axis="register-lifetime",
        )
        source2 = self.root / "recovery-source-2.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "recovery-object-2.o"
        object2.write_bytes(b"recovery-object-two")
        strict2 = self.root / "recovery-strict-2.json"
        _write_json(strict2, _report("fn"))
        module.record_candidate(
            self.root,
            recovery_workspace,
            candidate_id="c2",
            source=source2,
            object_path=object2,
            compile_attestation=self._attestation(
                "recover-c2",
                source=source2,
                object_path=object2,
                workspace=recovery_workspace,
            ),
            strict_report=strict2,
            data_report=None,
            hypothesis="natural candidate",
            axis="layout",
        )
        index_path = recovery_workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["candidates"].pop("c2")
        for mapping_name in ("source_context_index", "object_index"):
            index[mapping_name] = {
                key: value for key, value in index[mapping_name].items() if value != "c2"
            }
        index["sequence"] = 1
        c1_record = json.loads((recovery_workspace / "candidates" / "c1.json").read_text(encoding="utf-8"))
        index["last_record_sha256"] = c1_record["record_sha256"]
        index_path.write_text(json.dumps(_rehash(index, "index_sha256"), separators=(",", ":")), encoding="utf-8")

        recovered = module.record_candidate(
            self.root,
            recovery_workspace,
            candidate_id="c2",
            source=source2,
            object_path=object2,
            compile_attestation=self._attestation(
                "recover-c2",
                source=source2,
                object_path=object2,
                workspace=recovery_workspace,
            ),
            strict_report=strict2,
            data_report=None,
            hypothesis="natural candidate",
            axis="layout",
        )
        self.assertEqual(recovered["status"], "unchanged")
        final_index = json.loads(index_path.read_text(encoding="utf-8"))
        self.assertEqual(final_index["sequence"], 2)
        self.assertEqual(final_index["candidates"]["c2"], "candidates/c2.json")

    def test_diagnose_fails_closed_for_missing_indexed_result_without_running_job(self) -> None:
        self._init()
        self._record()
        marker = self.root / "missing-result-ran.marker"
        script = self._job_script(
            name="missing-result-probe.py",
            body="""
            import os, pathlib
            pathlib.Path(os.environ["MISSING_RESULT_MARKER"]).write_text("ran", encoding="utf-8")
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            """,
        )
        jobs = self._jobs(script, env={"MISSING_RESULT_MARKER": str(marker)})
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertTrue(marker.is_file())
        marker.unlink()
        result_path = self.workspace / "diagnostics" / f"{first['jobs'][0]['fingerprint']}.json"
        result_path.unlink()

        failed = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = failed["jobs"][0]
        self.assertEqual(row["status"], "failed")
        self.assertIn("no result event", row["error"])
        self.assertFalse(marker.exists(), "an indexed missing result must not launch the diagnostic")

    def test_matrix_rejects_orphan_candidate_and_diagnostic_context_records(self) -> None:
        self._init()
        self._record()
        orphan_candidate = self.workspace / "candidates" / "orphan.json"
        orphan_candidate.write_text("{}\n", encoding="utf-8")
        with self.assertRaisesRegex(module.MatchError, "candidate index"):
            module.build_matrix(self.root, self.workspace)

        orphan_candidate.unlink()
        jobs = self._jobs(self._job_script("orphan-context.py"), job_ids=("orphan-context",))
        batch = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        old_fingerprint = batch["jobs"][0]["fingerprint"]
        result_path = self.workspace / "diagnostics" / f"{old_fingerprint}.json"
        result = json.loads(result_path.read_text(encoding="utf-8"))
        result["candidate_source_sha256"] = "0" * 64
        session = module._load_session(self.workspace, self.root)
        forged_identity = {
            "source": {"sha256": result["candidate_source_sha256"]},
            "object": {"sha256": result["candidate_object_sha256"]},
            "source_context_key": result["source_context_key"],
        }
        new_fingerprint = module._job_fingerprint(session, forged_identity, result["job_spec"])
        old_output_root = self.workspace / "job-output" / old_fingerprint
        new_output_root = self.workspace / "job-output" / new_fingerprint
        old_output_root.rename(new_output_root)
        for output in result["outputs"]:
            relative = Path(output["path"]).relative_to(old_output_root)
            output["path"] = str(new_output_root / relative)
        result["fingerprint"] = new_fingerprint
        forged_result_path = self.workspace / "diagnostics" / f"{new_fingerprint}.json"
        forged_result_path.write_text(
            json.dumps(_rehash(result, "result_sha256"), separators=(",", ":")),
            encoding="utf-8",
        )
        result_path.unlink()
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["diagnostic_index"].pop(old_fingerprint)
        index["diagnostic_index"][new_fingerprint] = f"diagnostics/{new_fingerprint}.json"
        index_path.write_text(
            json.dumps(_rehash(index, "index_sha256"), separators=(",", ":")),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(module.MatchError, "producer binding"):
            module.build_matrix(self.root, self.workspace)

    def test_parallel_diagnostics_are_isolated_sorted_and_cached(self) -> None:
        self._init()
        self._record()
        script = self._job_script()
        jobs = self._jobs(script, job_ids=("j2", "j1", "j3"), distinct=True)
        real_pool = module.ThreadPoolExecutor
        observed_workers: list[int | None] = []

        class RecordingPool(real_pool):
            def __init__(self, *args: object, **kwargs: object) -> None:
                observed_workers.append(kwargs.get("max_workers", args[0] if args else None))
                super().__init__(*args, **kwargs)

        with mock.patch.object(module, "ThreadPoolExecutor", RecordingPool):
            first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs, max_workers=2)
        self.assertEqual(observed_workers, [2], "diagnostics must use the requested bounded worker count")
        self.assertEqual([row["requested_job_id"] for row in first["jobs"]], ["j1", "j2", "j3"])
        self.assertEqual(first["summary"], {"ran": 3, "cached": 0, "failed": 0})
        self.assertTrue(all(row["status"] == "passed" for row in first["jobs"]))
        self.assertTrue(all(row["cache_status"] == "ran" for row in first["jobs"]))
        output_roots = list((self.workspace / "job-output").iterdir())
        self.assertEqual(len(output_roots), 3)
        self.assertTrue(
            all(json.loads((path / "result.json").read_text(encoding="utf-8"))["readonly"] == "1" for path in output_roots)
        )

        # A repeated request is a pure CAS cache hit.  Alias jobs in one batch
        # also prove that duplicate diagnostic fingerprints are reused.
        second = module.diagnose_candidate(self.root, self.workspace, "c1", jobs, max_workers=2)
        self.assertEqual(second["summary"], {"ran": 0, "cached": 3, "failed": 0})
        self.assertTrue(all(row["cache_status"] == "cached" for row in second["jobs"]))
        aliases = self._jobs(script, job_ids=("alias-a", "alias-b"), distinct=False)
        aliased = module.diagnose_candidate(self.root, self.workspace, "c1", aliases, max_workers=2)
        self.assertEqual(aliased["summary"], {"ran": 1, "cached": 1, "failed": 0})
        self.assertEqual([row["cache_status"] for row in aliased["jobs"]], ["ran", "deduplicated_in_run"])
        renamed = self._jobs(script, job_ids=("alias-c", "alias-d"), distinct=False)
        renamed_result = module.diagnose_candidate(self.root, self.workspace, "c1", renamed, max_workers=2)
        self.assertEqual(renamed_result["summary"], {"ran": 0, "cached": 2, "failed": 0})

    def test_deleted_cached_diagnostic_output_never_returns_stale_cache(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script(), job_ids=("cache-check",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        first_output = Path(first["jobs"][0]["outputs"][0]["path"])
        self.assertTrue(first_output.is_file())
        first_output.unlink()

        second = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = second["jobs"][0]
        self.assertNotEqual(row["cache_status"], "cached")
        self.assertIn(row["status"], {"passed", "failed"})
        if row["status"] == "passed":
            self.assertTrue(first_output.is_file(), "a safe rerun must recreate the private output")

    def test_malformed_cached_output_descriptor_fails_without_raw_exception(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script(), job_ids=("cache-shape",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        fingerprint = first["jobs"][0]["fingerprint"]
        result_path = self.workspace / "diagnostics" / f"{fingerprint}.json"
        result = json.loads(result_path.read_text(encoding="utf-8"))
        result["outputs"][0].pop("sha256")
        _write_json(result_path, _rehash(result, "result_sha256"))
        rerun = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertEqual(rerun["summary"]["failed"], 1)
        with self.assertRaisesRegex(module.MatchError, "required field"):
            module.build_matrix(self.root, self.workspace)

    def test_indexed_diagnostic_rejects_forged_byte_accounting_and_job_labels(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("event-accounting.py"), job_ids=("accounting",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        fingerprint = first["jobs"][0]["fingerprint"]
        result_path = self.workspace / "diagnostics" / f"{fingerprint}.json"
        original = json.loads(result_path.read_text(encoding="utf-8"))

        forged_bytes = dict(original)
        forged_bytes["output_bytes"] = 0
        _write_json(result_path, _rehash(forged_bytes, "result_sha256"))
        with self.assertRaisesRegex(module.MatchError, "byte accounting mismatch"):
            module.build_matrix(self.root, self.workspace)

        forged_label = dict(original)
        forged_label["kind"] = "forged-label"
        _write_json(result_path, _rehash(forged_label, "result_sha256"))
        with self.assertRaisesRegex(module.MatchError, "job labels do not match"):
            module.build_matrix(self.root, self.workspace)

    def test_diagnostics_are_isolated_for_distinct_source_contexts(self) -> None:
        self._init()
        self._record("c1")
        source2 = self.root / "source-context-2.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "object-context-2.o"
        object2.write_bytes(b"object-context-two")
        self._record("c2", source=source2, object_path=object2)
        jobs = self._jobs(self._job_script("isolation.py"), job_ids=("same-job",))
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        second = module.diagnose_candidate(self.root, self.workspace, "c2", jobs)
        self.assertEqual(first["jobs"][0]["cache_status"], "ran")
        self.assertEqual(second["jobs"][0]["cache_status"], "ran")
        self.assertNotEqual(
            first["jobs"][0]["candidate_object_sha256"],
            second["jobs"][0]["candidate_object_sha256"],
        )
        output_roots = list((self.workspace / "job-output").iterdir())
        self.assertEqual(len(output_roots), 2)

    def test_matrix_does_not_attribute_same_object_diagnostics_to_other_source_context(self) -> None:
        self._init()
        self._record("c1")
        source2 = self.root / "source-context-same-object.c"
        source2.write_text("int fn(void) { return 3; }\n", encoding="utf-8")
        # The immutable object is intentionally shared, while the source
        # context differs.  A diagnostic result for c1 must not be projected
        # onto c2 merely because the object hash is equal.
        self._record("c2", source=source2, object_path=self.object)
        jobs = self._jobs(self._job_script("same-object.py"), job_ids=("same-object",))
        module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertIn(rows["c1"]["diagnostic_status"], {"available", "passed_read_only"})
        self.assertEqual(rows["c2"]["diagnostic_status"], "not_run")
        self.assertEqual(rows["c2"]["next_action"], "run_read_only_diagnostics_for_source_context")

    def test_matrix_focus_history_is_bounded_and_deterministic(self) -> None:
        self._init()

        def add_candidate(candidate_id: str, focus: str, status: str) -> None:
            source = self.root / f"{candidate_id}.c"
            source.write_text(f"int {candidate_id}(void) {{ return 1; }}\n", encoding="utf-8")
            object_path = self.root / f"{candidate_id}.o"
            object_path.write_bytes(candidate_id.encode("ascii"))
            strict = self.root / f"{candidate_id}-strict.json"
            data = self.root / f"{candidate_id}-data.json"
            _write_json(strict, _report(focus))
            _write_json(data, _report(focus, exact=True))
            self._record(
                candidate_id,
                source=source,
                object_path=object_path,
                strict_report=strict,
                data_report=data,
                hypothesis=f"{focus} source shape",
                axis=f"{focus}-axis",
                status=status,
                reason=f"{status} candidate",
                focus_symbol=focus,
            )

        add_candidate("c1", "CapSelectMasuPlayer", "measured")
        add_candidate("c2", "CapSelectMasuCom", "rejected")
        add_candidate("c3", "CapSelectMasuPlayer", "retained")

        full = module.build_matrix(self.root, self.workspace)
        self.assertNotIn("view", full)
        compact = module.build_matrix(
            self.root,
            self.workspace,
            focus_symbol="CapSelectMasuPlayer",
            limit=1,
            compact=True,
        )
        self.assertEqual(compact["view"], "compact")
        self.assertEqual(compact["query"]["total_candidate_count"], 3)
        self.assertEqual(compact["query"]["selected_candidate_count"], 1)
        row = compact["rows"][0]
        self.assertEqual(row["candidate_id"], "c1")
        self.assertEqual(row["focus_symbol"], "CapSelectMasuPlayer")
        self.assertEqual(row["strict_percent"], 75.0)
        self.assertEqual(row["data_percent"], 100.0)
        self.assertEqual(row["strict_diff_rows"], 1)
        self.assertEqual(row["data_diff_rows"], 0)
        self.assertEqual(row["target_size"], 4)
        self.assertEqual(row["candidate_size"], 4)
        self.assertEqual(row["outcome"]["status"], "measured")
        self.assertEqual(row["axis"], "CapSelectMasuPlayer-axis")
        self.assertEqual(row["name"], "CapSelectMasuPlayer source shape")
        self.assertEqual(
            [item["candidate_id"] for item in module.build_matrix(
                self.root,
                self.workspace,
                focus_symbol="CapSelectMasuPlayer",
                compact=True,
            )["rows"]],
            ["c1", "c3"],
        )
        latest = module.build_matrix(
            self.root,
            self.workspace,
            focus_symbol="CapSelectMasuPlayer",
            limit=1,
            compact=True,
            latest=True,
        )
        self.assertEqual(latest["query"]["order"], "newest")
        self.assertEqual(latest["rows"][0]["candidate_id"], "c3")
        ordered = module.build_matrix(
            self.root,
            self.workspace,
            focus_symbol="CapSelectMasuPlayer",
            compact=True,
            order="newest",
        )
        self.assertEqual(
            [item["candidate_id"] for item in ordered["rows"]],
            ["c3", "c1"],
        )

    def test_matrix_focus_history_text_cli(self) -> None:
        self._init()
        self._record()
        with mock.patch("builtins.print") as printer:
            result = module.main([
                "--root",
                str(self.root),
                "matrix",
                "--workspace",
                str(self.workspace),
                "--focus-symbol",
                "fn",
                "--limit",
                "1",
                "--compact",
            ])
        self.assertEqual(result, 0)
        rendered = "\n".join(str(call.args[0]) for call in printer.call_args_list)
        self.assertIn("matrix-focus", rendered)
        self.assertIn("c1", rendered)
        with mock.patch("builtins.print") as printer:
            result = module.main([
                "--root",
                str(self.root),
                "matrix",
                "--workspace",
                str(self.workspace),
                "--focus-symbol",
                "fn",
                "--limit",
                "1",
                "--compact-json",
            ])
        self.assertEqual(result, 0)
        payload = json.loads(str(printer.call_args.args[0]))
        self.assertEqual(payload["view"], "compact")
        self.assertEqual(payload["rows"][0]["candidate_id"], "c1")

    def test_matrix_ignores_unindexed_diagnostic_result(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("unindexed.py"), job_ids=("unindexed",))
        batch = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        fingerprint = batch["jobs"][0]["fingerprint"]
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["diagnostic_index"].pop(fingerprint, None)
        index_body = dict(index)
        index_body.pop("index_sha256", None)
        canonical = json.dumps(index_body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        index["index_sha256"] = hashlib.sha256(canonical.encode("utf-8")).hexdigest()
        index_path.write_text(json.dumps(index, separators=(",", ":")), encoding="utf-8")
        try:
            matrix = module.build_matrix(self.root, self.workspace)
        except module.MatchError as exc:
            self.assertIn("diagnostic index", str(exc))
        else:
            self.assertEqual(matrix["rows"][0]["diagnostic_status"], "not_run")

    def test_diagnostic_stdout_over_bound_is_rejected(self) -> None:
        script = self._job_script(
            name="verbose-probe.py",
            body="""
            import os, pathlib, sys
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            sys.stdout.write("x" * 4096)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script, max_output_bytes=1024)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = result["jobs"][0]
        self.assertTrue(row["stdout_truncated"])
        self.assertNotEqual(row["status"], "passed")

    def test_declared_output_file_over_bound_is_rejected(self) -> None:
        script = self._job_script(
            name="large-output-probe.py",
            body="""
            import os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_bytes(b"x" * 4096)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script, max_output_bytes=1024)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = result["jobs"][0]
        self.assertNotEqual(row["status"], "passed")
        self.assertTrue(row.get("output_limit_exceeded") or "output" in row.get("error", "").lower())

    def test_stdout_stderr_and_declared_outputs_share_one_budget(self) -> None:
        script = self._job_script(
            name="combined-output-probe.py",
            body="""
            import os, pathlib, sys
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            sys.stdout.write("s" * 400)
            sys.stderr.write("e" * 400)
            (output / "result.json").write_bytes(b"o" * 400)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script, max_output_bytes=1024)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        row = result["jobs"][0]
        self.assertEqual(row["status"], "failed")
        self.assertTrue(row["output_limit_exceeded"])
        self.assertGreater(row["output_bytes"], 1024)

    def test_undeclared_private_output_is_rejected_and_indexed_as_failure(self) -> None:
        script = self._job_script(
            name="undeclared-output-probe.py",
            body="""
            import os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            (output / "undeclared.bin").write_bytes(b"not-declared")
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertEqual(result["summary"], {"ran": 1, "cached": 0, "failed": 1})
        self.assertEqual(result["jobs"][0]["status"], "failed")
        self.assertIn("undeclared private outputs", result["jobs"][0]["error"])
        matrix = module.build_matrix(self.root, self.workspace)
        self.assertEqual(matrix["rows"][0]["diagnostic_status"], "failed")

    def test_declared_output_directory_is_cleaned_and_indexed_as_failure(self) -> None:
        script = self._job_script(
            name="directory-output-probe.py",
            body="""
            import os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            (output / "result.json").mkdir(parents=True)
            """,
        )
        self._init()
        self._record()
        jobs = self._jobs(script)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertEqual(result["summary"], {"ran": 1, "cached": 0, "failed": 1})
        self.assertIn("not a regular file", result["jobs"][0]["error"])
        self.assertFalse(Path(result["jobs"][0]["outputs"][0]["path"]).exists())
        matrix = module.build_matrix(self.root, self.workspace)
        self.assertEqual(matrix["rows"][0]["diagnostic_status"], "failed")

    def test_timeout_change_creates_a_new_diagnostic_fingerprint(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("timeout-fingerprint.py"), timeout_seconds=10)
        first = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        first_row = first["jobs"][0]
        jobs_value = json.loads(jobs.read_text(encoding="utf-8"))
        jobs_value["jobs"][0]["timeout_seconds"] = 11
        _write_json(jobs, jobs_value)
        second = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        second_row = second["jobs"][0]
        self.assertNotEqual(first_row["fingerprint"], second_row["fingerprint"])
        self.assertEqual(second_row["cache_status"], "ran")
        self.assertEqual(len(list((self.workspace / "diagnostics").glob("*.json"))), 2)

    def test_arbitrary_parent_environment_is_not_inherited(self) -> None:
        script = self._job_script(
            name="environment-probe.py",
            body="""
            import json, os, pathlib
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text(json.dumps({"secret": os.environ.get("MATCH_TEST_SECRET", "missing")}), encoding="utf-8")
            """,
        )
        self._init()
        self._record()
        old_secret = os.environ.get("MATCH_TEST_SECRET")
        os.environ["MATCH_TEST_SECRET"] = "must-not-leak"
        try:
            jobs = self._jobs(script)
            result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        finally:
            if old_secret is None:
                os.environ.pop("MATCH_TEST_SECRET", None)
            else:
                os.environ["MATCH_TEST_SECRET"] = old_secret
        output_path = Path(result["jobs"][0]["outputs"][0]["path"])
        self.assertEqual(json.loads(output_path.read_text(encoding="utf-8"))["secret"], "missing")

    def test_serial_native_proof_compiler_and_authority_resources_rejected_before_subprocess(self) -> None:
        self._init()
        self._record()
        marker = self.root / "executed.marker"
        script = self._job_script(
            body="""
            import pathlib, os
            pathlib.Path(os.environ["EXECUTED_MARKER"]).write_text("executed", encoding="utf-8")
            """
        )
        for resource in ("compiler", "native_debug", "proof", "authority", "retail_link"):
            jobs = self._jobs(script, resource_class=resource, env={"EXECUTED_MARKER": str(marker)})
            with self.assertRaisesRegex(module.MatchError, "serial resource class"):
                module.diagnose_candidate(self.root, self.workspace, "c1", jobs)
        self.assertFalse(marker.exists())

    def test_job_input_toctou_is_reported_and_output_escape_rejected(self) -> None:
        self._init()
        self._record()
        mutable = self.root / "mutable.input"
        mutable.write_text("before", encoding="utf-8")
        script = self._job_script(
            name="mutate.py",
            body="""
            import os, pathlib
            pathlib.Path(os.environ["MUTABLE_INPUT"]).write_text("after", encoding="utf-8")
            output = pathlib.Path(os.environ["MATCH_WORKBENCH_OUTPUT_ROOT"])
            output.mkdir(parents=True, exist_ok=True)
            (output / "result.json").write_text("{}", encoding="utf-8")
            """,
        )
        jobs_path = self._jobs(script, env={"MUTABLE_INPUT": str(mutable)})
        jobs = json.loads(jobs_path.read_text(encoding="utf-8"))
        jobs["jobs"][0]["inputs"].append(_descriptor(mutable))
        _write_json(jobs_path, jobs)
        result = module.diagnose_candidate(self.root, self.workspace, "c1", jobs_path)
        self.assertEqual(result["summary"]["failed"], 1)
        self.assertIn("changed from its authenticated descriptor", result["jobs"][0]["error"])

        escape = self._jobs(script, env={"MUTABLE_INPUT": str(mutable)})
        escaped = json.loads(escape.read_text(encoding="utf-8"))
        escaped["jobs"][0]["outputs"] = ["../outside.json"]
        _write_json(escape, escaped)
        with self.assertRaisesRegex(module.MatchError, "output must be a relative contained path"):
            module.diagnose_candidate(self.root, self.workspace, "c1", escape)

        embedded = self._jobs(script, env={"MUTABLE_INPUT": str(mutable)})
        embedded_value = json.loads(embedded.read_text(encoding="utf-8"))
        embedded_value["jobs"][0]["argv"] = [str(script), "--config={workspace}/index.json"]
        _write_json(embedded, embedded_value)
        with self.assertRaisesRegex(module.MatchError, "placeholders must occupy the entire"):
            module.diagnose_candidate(self.root, self.workspace, "c1", embedded)

    def test_matrix_is_self_hashed_deterministic_and_points_to_next_action(self) -> None:
        self._init()
        first = self._record(data_report=self.data)
        source2 = self.root / "source2.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "object2.o"
        object2.write_bytes(b"object-two")
        strict2 = self.root / "strict2.json"
        _write_json(strict2, _report("fn", exact=True))
        self._record("c2", source=source2, object_path=object2, strict_report=strict2, data_report=self.data, axis="layout")
        matrix = module.build_matrix(self.root, self.workspace)
        matrix_body = dict(matrix)
        matrix_hash = matrix_body.pop("matrix_sha256")
        canonical = json.dumps(matrix_body, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n"
        self.assertEqual(matrix_hash, hashlib.sha256(canonical.encode("utf-8")).hexdigest())
        self.assertEqual([row["candidate_id"] for row in matrix["rows"]], ["c1", "c2"])
        self.assertEqual(matrix["aggregate"]["candidate_count"], 2)
        self.assertEqual(matrix["rows"][0]["next_action"], "continue_one_axis_matching")
        self.assertEqual(
            matrix["rows"][1]["next_action"],
            "authenticate_report_binding_then_run_serial_proof_and_closure",
        )
        self.assertEqual(matrix, module.build_matrix(self.root, self.workspace))

    def test_matrix_rejected_is_fail_closed_but_retained_keeps_closure_action(self) -> None:
        self._init()
        rejected = self._record(
            "rejected",
            data_report=self.data,
            status="rejected",
            reason="target-shaped probe is a no-go",
        )
        source2 = self.root / "retained-source.c"
        source2.write_text("int fn(void) { return 2; }\n", encoding="utf-8")
        object2 = self.root / "retained-object.o"
        object2.write_bytes(b"retained-object")
        strict2 = self.root / "retained-strict.json"
        _write_json(strict2, _report("fn", exact=True))
        retained = self._record(
            "retained",
            source=source2,
            object_path=object2,
            strict_report=strict2,
            data_report=self.data,
            status="retained",
            reason="natural exact candidate",
        )

        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(rejected["record"]["outcome"]["status"], "rejected")
        self.assertEqual(rows["rejected"]["next_action"], "do_not_advance_rejected_candidate")
        self.assertEqual(
            rows["retained"]["next_action"],
            "authenticate_report_binding_then_run_serial_proof_and_closure",
        )
        self.assertFalse(rows["rejected"]["next_action"].startswith("authenticate"))
        self.assertEqual(retained["record"]["outcome"]["status"], "retained")

    def test_record_focus_symbol_drives_compacts_and_legacy_defaults_to_session(self) -> None:
        self._init()
        focused_strict = self.root / "focused-strict.json"
        focused_data = self.root / "focused-data.json"
        _write_json(focused_strict, _report("other", exact=False))
        _write_json(focused_data, _report("other", exact=True))
        focused = self._record(
            "focused",
            strict_report=focused_strict,
            data_report=focused_data,
            focus_symbol="other",
        )
        record = focused["record"]
        self.assertEqual(record["focus_symbol"], "other")
        self.assertEqual(record["reports"]["strict"]["compact"]["focus"]["name"], "other")
        self.assertEqual(record["reports"]["data"]["compact"]["focus"]["name"], "other")

        focused_body = dict(record)
        focused_hash = focused_body.pop("record_sha256")
        canonical = json.dumps(
            focused_body, ensure_ascii=False, sort_keys=True, separators=(",", ":")
        ) + "\n"
        self.assertEqual(focused_hash, hashlib.sha256(canonical.encode("utf-8")).hexdigest())
        self.assertEqual(
            module.record_candidate(
                self.root,
                self.workspace,
                candidate_id="focused",
                source=self.source,
                object_path=self.object,
                compile_attestation=self._attestation("focused"),
                strict_report=focused_strict,
                data_report=focused_data,
                hypothesis="natural candidate",
                axis="register-lifetime",
                focus_symbol="other",
            )["status"],
            "unchanged",
        )
        cli_output = io.StringIO()
        cli_attestation = self._attestation("cli-focused")
        with contextlib.redirect_stdout(cli_output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "record",
                        "--workspace",
                        str(self.workspace),
                        "--candidate-id",
                        "cli",
                        "--source",
                        str(self.source),
                        "--object",
                        str(self.object),
                        "--compile-attestation",
                        str(cli_attestation),
                        "--strict-report",
                        str(focused_strict),
                        "--data-report",
                        str(focused_data),
                        "--hypothesis",
                        "natural candidate",
                        "--axis",
                        "register-lifetime",
                        "--focus-symbol",
                        "other",
                        "--json",
                    ]
                ),
                0,
            )
        cli_record = json.loads(cli_output.getvalue())["record"]
        self.assertEqual(cli_record["focus_symbol"], "other")

        legacy = self._record("legacy")
        self.assertNotIn("focus_symbol", legacy["record"])
        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(rows["legacy"]["focus_symbol"], "fn")
        self.assertEqual(rows["legacy"]["strict_focus"]["name"], "fn")

    def test_focus_symbol_is_optional_but_candidate_schema_remains_closed(self) -> None:
        self._init()
        self._record("c1", focus_symbol="other")
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        candidate["unexpected"] = True
        _write_json(candidate_path, _rehash(candidate, "record_sha256"))
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["last_record_sha256"] = json.loads(candidate_path.read_text(encoding="utf-8"))["record_sha256"]
        _write_json(index_path, _rehash(index, "index_sha256"))
        with self.assertRaisesRegex(module.MatchError, "candidate record contains unknown field"):
            module.build_matrix(self.root, self.workspace)

    def test_matrix_rejects_malformed_compact_focus_without_raw_exception(self) -> None:
        self._init()
        self._record(data_report=self.data)
        candidate_path = self.workspace / "candidates" / "c1.json"
        candidate = json.loads(candidate_path.read_text(encoding="utf-8"))
        candidate["reports"]["strict"]["compact"]["focus"] = "not-an-object"
        candidate = _rehash(candidate, "record_sha256")
        _write_json(candidate_path, candidate)
        index_path = self.workspace / "index.json"
        index = json.loads(index_path.read_text(encoding="utf-8"))
        index["last_record_sha256"] = candidate["record_sha256"]
        _write_json(index_path, _rehash(index, "index_sha256"))
        with self.assertRaisesRegex(module.MatchError, "compact.focus"):
            module.build_matrix(self.root, self.workspace)

    def test_duplicate_object_with_different_report_does_not_reuse_evidence(self) -> None:
        self._init()
        self._record("c1")
        exact = self.root / "strict-exact-duplicate.json"
        _write_json(exact, _report("fn", exact=True))
        second = self._record("c2", strict_report=exact)
        self.assertEqual(second["record"]["duplicate_of"], "c1")
        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(
            rows["c2"]["next_action"],
            "run_read_only_diagnostics_for_source_context",
        )

    def test_duplicate_object_chain_prefers_same_source_context_evidence(self) -> None:
        self._init()
        self._record("c1")

        source2 = self.root / "candidate-copy.c"
        source2.write_bytes(self.source.read_bytes())
        object2 = self.root / "candidate-copy.o"
        object2.write_bytes(self.object.read_bytes())
        second = self._record("c2", source=source2, object_path=object2)
        self.assertEqual(second["record"]["duplicate_of"], "c1")

        third = self._record(
            "c3",
            source=source2,
            object_path=object2,
            hypothesis="same source context measured again",
        )
        self.assertEqual(third["status"], "duplicate")
        self.assertEqual(third["record"]["duplicate_of"], "c2")
        matrix = module.build_matrix(self.root, self.workspace)
        rows = {row["candidate_id"]: row for row in matrix["rows"]}
        self.assertEqual(rows["c2"]["next_action"], "run_read_only_diagnostics_for_source_context")
        self.assertEqual(rows["c3"]["next_action"], "reuse_existing_evidence")

    def test_matrix_allows_generation_manifest_but_not_unindexed_candidate_record(self) -> None:
        self._init()
        recorded = self._record("c1")
        candidate_directory = self.workspace / "candidates"
        _write_json(
            candidate_directory / "manifest.json",
            {
                "schema": "private-candidate-generation-manifest/v1",
                "production_modified": False,
                "candidates": [{"id": "c1"}],
            },
        )
        matrix = module.build_matrix(self.root, self.workspace)
        self.assertEqual(matrix["aggregate"]["candidate_count"], 1)

        orphan = json.loads(
            (candidate_directory / "c1.json").read_text(encoding="utf-8")
        )
        orphan["candidate_id"] = "orphan"
        orphan["ordinal"] = 2
        orphan["previous_record_sha256"] = recorded["record"]["record_sha256"]
        _write_json(candidate_directory / "orphan.json", _rehash(orphan, "record_sha256"))
        with self.assertRaisesRegex(
            module.MatchError,
            "candidate index does not cover every immutable candidate record",
        ):
            module.build_matrix(self.root, self.workspace)

        (candidate_directory / "orphan.json").unlink()
        _write_json(candidate_directory / "manifest.json", {"schema": module.CANDIDATE_SCHEMA})
        with self.assertRaisesRegex(
            module.MatchError,
            "candidate generation manifest cannot contain an immutable candidate record",
        ):
            module.build_matrix(self.root, self.workspace)

    def test_assess_reports_focus_delta_counts_changed_siblings_and_central_json(self) -> None:
        baseline_strict = self.root / "baseline-strict.json"
        candidate_strict = self.root / "candidate-strict.json"
        baseline_data = self.root / "baseline-data.json"
        candidate_data = self.root / "candidate-data.json"
        _write_json(
            baseline_strict,
            _assessment_report(
                focus_match=75.0,
                focus_size="4",
                focus_candidate_size="4",
                sibling_match=100.0,
                sibling_size="8",
                sibling_candidate_size="8",
            ),
        )
        _write_json(
            candidate_strict,
            _assessment_report(
                focus_match=100.0,
                focus_size="4",
                focus_candidate_size="6",
                sibling_match=100.0,
                sibling_size="8",
                sibling_candidate_size="9",
            ),
        )
        _write_json(baseline_data, _assessment_report(focus_match=75.0))
        _write_json(candidate_data, _assessment_report(focus_match=100.0))
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline_strict),
                        "--candidate-strict",
                        str(candidate_strict),
                        "--baseline-data",
                        str(baseline_data),
                        "--candidate-data",
                        str(candidate_data),
                        "--focus-symbol",
                        "focus",
                    ]
                ),
                0,
            )
        result = json.loads(output.getvalue())
        self.assertEqual(result["schema"], module.ASSESSMENT_SCHEMA)
        self.assertEqual(result["verdict"], "accepted")
        strict = result["reports"]["strict"]
        self.assertEqual(strict["exact_function_counts"], {"before": {"exact": 1, "total": 2}, "after": {"exact": 2, "total": 2}})
        self.assertEqual(strict["focus"]["before"]["size"], 4)
        self.assertEqual(strict["focus"]["after"]["candidate_size"], 6)
        self.assertEqual(strict["focus"]["delta"]["match_percent"], 25)
        self.assertEqual(strict["focus"]["delta"]["diff_kind_delta"], {"REG_SWAP": -1})
        changed = [row for row in result["changed_siblings"] if row["report"] == "strict"]
        self.assertEqual([row["symbol"] for row in changed], ["sibling"])
        self.assertEqual(changed[0]["delta"]["candidate_size"], 1)
        self.assertEqual(result["regressions"], [])

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "assess",
                "--baseline-strict",
                str(baseline_strict),
                "--candidate-strict",
                str(candidate_strict),
                "--focus-symbol",
                "focus",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout)["verdict"], "accepted")

    def test_residuals_rank_classify_exclude_and_route_centrally(self) -> None:
        strict_path = self.root / "residual-strict.json"
        data_path = self.root / "residual-data.json"
        entries = (
            ("both", 40.0, "24", "20", ("ARG", "ARG", "REG")),
            ("strict_only", 90.0, "16", "20", ("REG",)),
            ("data_only", 100.0, "8", "8", ()),
            ("known", 70.0, "12", "12", ("NOP",)),
            ("exact", 100.0, "4", "4", ()),
        )
        data_entries = (
            ("both", 50.0, "24", "20", ("ARG", "REG")),
            ("strict_only", 100.0, "16", "20", ()),
            ("data_only", 90.0, "8", "8", ("CALL",)),
            ("known", 95.0, "12", "12", ("NOP",)),
            ("exact", 100.0, "4", "4", ()),
        )
        _write_json(strict_path, _residual_report(entries))
        _write_json(data_path, _residual_report(data_entries))

        argv = [
            "--root",
            str(self.root),
            "residuals",
            "--strict-report",
            str(strict_path),
            "--data-report",
            str(data_path),
            "--exclude-known-exact",
            "known",
        ]
        first_output = io.StringIO()
        with contextlib.redirect_stdout(first_output):
            self.assertEqual(module.main(argv), 0)
        second_output = io.StringIO()
        with contextlib.redirect_stdout(second_output):
            self.assertEqual(module.main(argv), 0)
        self.assertEqual(first_output.getvalue(), second_output.getvalue())

        result = json.loads(first_output.getvalue())
        self.assertEqual(result["schema"], module.RESIDUALS_SCHEMA)
        self.assertEqual(result["excluded_symbols"], ["known"])
        self.assertEqual(result["function_counts"]["strict"], {"exact": 2, "total": 5, "nonexact": 3})
        self.assertEqual(result["function_counts"]["data"], {"exact": 2, "total": 5, "nonexact": 3})
        self.assertEqual(
            result["classification_counts"],
            {"both": 1, "data_only": 1, "strict_only": 1},
        )
        self.assertEqual(result["residual_count"], 3)
        self.assertEqual(result["excluded_function_count"], 1)
        self.assertEqual(result["excluded_residual_count"], 1)
        residuals = result["residuals"]
        self.assertEqual(
            [(row["rank"], row["symbol"], row["classification"]) for row in residuals],
            [
                (1, "both", "both"),
                (2, "strict_only", "strict_only"),
                (3, "data_only", "data_only"),
            ],
        )
        self.assertEqual(residuals[0]["strict"]["target_size"], 24)
        self.assertEqual(residuals[0]["strict"]["candidate_size"], 20)
        self.assertEqual(residuals[0]["strict"]["match_percent"], 40)
        self.assertEqual(residuals[0]["strict"]["diff_kinds"], {"ARG": 2, "REG": 1})
        self.assertEqual(residuals[0]["data"]["diff_kinds"], {"ARG": 1, "REG": 1})

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "residuals",
                "--strict",
                str(strict_path),
                "--data",
                str(data_path),
                "--exclude-symbol",
                "known",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout), result)

    def test_stack_residue_reports_authenticated_target_zero_read_slots(self) -> None:
        report = self.root / "stack-residue.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_instruction(0x100, "stw", "r3", -4),
                    _stack_instruction(0x104, "stwu", "r1", -8),
                    _stack_instruction(0x108, "lwz", "r4", -8),
                    _stack_instruction(0x10C, "stfd", "f2", -12),
                ]
            ),
        )

        first = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        second = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        self.assertEqual(first, second)
        self.assertEqual(first["target_stack_access_count"], 4)
        self.assertEqual(
            [
                (row["offset"], row["write_count"], row["read_count"])
                for row in first["stack_slots"]
            ],
            [(-12, 1, 0), (-8, 1, 1), (-4, 1, 0)],
        )
        self.assertEqual(first["zero_read_slot_count"], 1)
        self.assertEqual(
            [row["offset"] for row in first["zero_read_slots"]],
            [-4],
        )
        self.assertEqual(first["slots"], first["zero_read_slots"])
        self.assertEqual(first["zero_read_slots"][0]["writes"][0]["address"], 0x100)
        self.assertEqual(first["stack_slots"][0]["width"], 8)
        self.assertEqual(first["stack_slots"][0]["overlap_read_count"], 1)
        self.assertEqual(
            first["stack_slots"][0]["byte_range"], {"start": -12, "end": -4}
        )
        self.assertEqual(
            first["stack_slots"][0]["writes"][0]["overlap_evidence"][0]["offset"],
            -8,
        )
        self.assertEqual(first["authority_advanced"], False)
        self.assertEqual(first["report"]["sha256"], _sha256(report))

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "stack-residue",
                "--report",
                str(report),
                "--focus-symbol",
                "focus",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout), first)

    def test_stack_residue_rejects_unsupported_target_operand_shape(self) -> None:
        report = self.root / "stack-residue-unsupported.json"
        instruction = _stack_instruction(0x200, "stw", "r3", -4)
        instruction["parts"] = [
            {"opcode": {"mnemonic": "stw", "opcode": 0}},
            {"arg": {"opaque": "r3"}},
            {"separator": True},
            {"arg": {"opaque": "-4(r1)"}},
        ]
        _write_json(report, _stack_residue_report([instruction]))

        with self.assertRaisesRegex(module.MatchError, "unsupported.*operand shape"):
            module.inspect_stack_residue(
                self.root,
                report=report,
                focus_symbol="focus",
            )

        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "stack-residue",
                    "--report",
                    str(report),
                    "--focus-symbol",
                    "focus",
                ]
            )
        self.assertEqual(status, 2)
        self.assertIn("unsupported", output.getvalue())

    def test_stack_residue_classifies_wide_overlap_in_both_instruction_orders(self) -> None:
        for name, instructions in (
            (
                "wide-write-narrow-read",
                [
                    _stack_instruction(0x220, "stfd", "f2", 0x18),
                    _stack_instruction(0x224, "lwz", "r3", 0x1C),
                ],
            ),
            (
                "narrow-write-wide-read",
                [
                    _stack_instruction(0x230, "stw", "r3", 0x1C),
                    _stack_instruction(0x234, "lfd", "f2", 0x18),
                ],
            ),
        ):
            report = self.root / f"stack-residue-{name}.json"
            _write_json(report, _stack_residue_report(instructions))
            result = module.inspect_stack_residue(
                self.root,
                report=report,
                focus_symbol="focus",
            )

            write_slot = next(row for row in result["stack_slots"] if row["write_count"])
            self.assertEqual(result["zero_read_slot_count"], 0)
            self.assertFalse(write_slot["residue_candidate"])
            self.assertFalse(write_slot["writes"][0]["zero_read"])
            self.assertEqual(write_slot["writes"][0]["zero_read_reason"], "read_byte_range_overlap")
            self.assertEqual(len(write_slot["writes"][0]["overlap_evidence"]), 1)
            self.assertEqual(
                write_slot["writes"][0]["overlap_evidence"][0]["byte_range"],
                {"start": 0x1C, "end": 0x20}
                if name == "wide-write-narrow-read"
                else {"start": 0x18, "end": 0x20},
            )

    def test_stack_residue_respects_byte_and_halfword_boundaries(self) -> None:
        report = self.root / "stack-residue-byte-boundaries.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_instruction(0x240, "stb", "r3", 0x20),
                    _stack_instruction(0x244, "lhz", "r4", 0x1F),
                    _stack_instruction(0x248, "stb", "r5", 0x30),
                    _stack_instruction(0x24C, "lhz", "r6", 0x2E),
                ]
            ),
        )

        result = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        by_offset = {row["offset"]: row for row in result["stack_slots"]}
        self.assertEqual(by_offset[0x20]["width"], 1)
        self.assertEqual(by_offset[0x20]["read_count"], 0)
        self.assertEqual(by_offset[0x20]["overlap_read_count"], 1)
        self.assertFalse(by_offset[0x20]["residue_candidate"])
        self.assertEqual(by_offset[0x30]["width"], 1)
        self.assertEqual(by_offset[0x30]["read_count"], 0)
        self.assertTrue(by_offset[0x30]["residue_candidate"])
        self.assertEqual(result["zero_read_slot_count"], 1)

    def test_stack_residue_canonicalizes_sp_and_r1_for_overlap(self) -> None:
        report = self.root / "stack-residue-sp-r1.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_instruction(0x260, "stw", "r3", 0x40, base="sp"),
                    _stack_instruction(0x264, "lwz", "r4", 0x40, base="r1"),
                ]
            ),
        )

        result = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        self.assertEqual(len(result["stack_slots"]), 1)
        slot = result["stack_slots"][0]
        self.assertEqual(slot["base_register"], "r1")
        self.assertEqual(slot["read_count"], 1)
        self.assertFalse(slot["residue_candidate"])
        self.assertEqual(slot["writes"][0]["base_register"], "sp")
        self.assertEqual(slot["reads"][0]["base_register"], "r1")

    def test_stack_residue_supported_direct_d_form_mnemonics_have_widths(self) -> None:
        supported = {
            "lbz": ("read", 1),
            "lha": ("read", 2),
            "lhz": ("read", 2),
            "lwz": ("read", 4),
            "ld": ("read", 8),
            "lfs": ("read", 4),
            "lfd": ("read", 8),
            "stb": ("write", 1),
            "sth": ("write", 2),
            "stw": ("write", 4),
            "std": ("write", 8),
            "stfs": ("write", 4),
            "stfd": ("write", 8),
        }
        for index, (mnemonic, (kind, width)) in enumerate(supported.items()):
            arguments = [("opaque", "r3"), ("signed", index * 16), ("opaque", "r1")]
            self.assertEqual(module._stack_memory_kind(mnemonic), kind)
            self.assertEqual(
                module._stack_d_form_stack_access(mnemonic, arguments),
                (kind, "r1", index * 16),
            )
            self.assertEqual(module._STACK_DFORM_WIDTHS[mnemonic], width)

    def test_stack_residue_handles_real_crackom_paired_single_shapes(self) -> None:
        report = self.root / "stack-residue-crackom-psq.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_paired_single_instruction(
                        0x300, "psq_st", "f31", 0x28, 0, "qr0"
                    ),
                    _stack_paired_single_instruction(
                        0x304, "psq_l", "f31", 0x28, 0, "qr0"
                    ),
                    _stack_paired_single_instruction(
                        0x308, "psq_st", "f30", 0x40, 1, "qr0"
                    ),
                ]
            ),
        )

        result = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        by_offset = {row["offset"]: row for row in result["stack_slots"]}
        wide = by_offset[0x28]
        narrow = by_offset[0x40]
        self.assertEqual(wide["width"], 8)
        self.assertEqual(wide["byte_range"], {"start": 0x28, "end": 0x30})
        self.assertEqual(wide["write_count"], 1)
        self.assertEqual(wide["read_count"], 1)
        self.assertFalse(wide["residue_candidate"])
        self.assertEqual(wide["writes"][0]["formatted"], "psq_st f31, 0x28(r1), 0, qr0")
        self.assertEqual(wide["writes"][0]["operand_form"], "paired_single")
        self.assertEqual(wide["writes"][0]["paired_single_w"], 0)
        self.assertEqual(wide["writes"][0]["quantization_register"], "qr0")
        self.assertEqual(narrow["width"], 4)
        self.assertEqual(narrow["byte_range"], {"start": 0x40, "end": 0x44})
        self.assertTrue(narrow["residue_candidate"])
        self.assertEqual(narrow["writes"][0]["paired_single_w"], 1)

    def test_stack_residue_paired_single_widths_use_byte_overlap(self) -> None:
        report = self.root / "stack-residue-psq-byte-overlap.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_paired_single_instruction(
                        0x320, "psq_st", "f2", 0x50, 0, "qr0"
                    ),
                    _stack_instruction(0x324, "lwz", "r3", 0x54),
                    _stack_paired_single_instruction(
                        0x328, "psq_st", "f3", 0x60, 1, "qr0"
                    ),
                    _stack_instruction(0x32C, "lwz", "r4", 0x64),
                ]
            ),
        )

        result = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        by_offset = {row["offset"]: row for row in result["stack_slots"]}
        self.assertEqual(by_offset[0x50]["width"], 8)
        self.assertEqual(by_offset[0x50]["overlap_read_count"], 1)
        self.assertFalse(by_offset[0x50]["residue_candidate"])
        self.assertEqual(by_offset[0x60]["width"], 4)
        self.assertEqual(by_offset[0x60]["overlap_read_count"], 0)
        self.assertTrue(by_offset[0x60]["residue_candidate"])
        self.assertEqual(result["zero_read_slot_count"], 1)

    def test_stack_residue_rejects_ambiguous_paired_single_selectors(self) -> None:
        for index, (selector, value) in enumerate(
            (("W", 2), ("W", "w"), ("I", "qrx"), ("I", "qr8"))
        ):
            instruction = _stack_paired_single_instruction(
                0x340 + index * 4,
                "psq_st",
                "f2",
                -0x20,
                0 if selector == "I" else value,
                value if selector == "I" else "qr0",
            )
            report = self.root / f"stack-residue-psq-ambiguous-{index}.json"
            _write_json(report, _stack_residue_report([instruction]))
            with self.subTest(selector=selector, value=value), self.assertRaisesRegex(
                module.MatchError,
                selector,
            ):
                module.inspect_stack_residue(
                    self.root,
                    report=report,
                    focus_symbol="focus",
                )

    def test_stack_residue_rejects_ambiguous_paired_single_operand_shape(self) -> None:
        for suffix, mutate in (
            ("missing-selector", lambda parts: parts[:-2]),
            ("non-fpr-destination", lambda parts: parts[:1] + [{"arg": {"opaque": "r3"}}] + parts[2:]),
        ):
            instruction = _stack_paired_single_instruction(
                0x360, "psq_l", "f2", 0x20, 0, "qr0"
            )
            instruction["parts"] = mutate(instruction["parts"])
            report = self.root / f"stack-residue-psq-ambiguous-shape-{suffix}.json"
            _write_json(report, _stack_residue_report([instruction]))
            with self.subTest(shape=suffix), self.assertRaisesRegex(
                module.MatchError,
                "unsupported.*operand shape",
            ):
                module.inspect_stack_residue(
                    self.root,
                    report=report,
                    focus_symbol="focus",
                )

    def test_stack_residue_rejects_unsupported_memory_forms_and_updates(self) -> None:
        unsupported = {
            "lwzx": [
                ("opaque", "r3"),
                ("opaque", "r4"),
                ("opaque", "r1"),
            ],
            "lswi": [
                ("opaque", "r3"),
                ("opaque", "r1"),
                ("unsigned", 4),
            ],
            "lwarx": [
                ("opaque", "r3"),
                ("opaque", "r1"),
                ("opaque", "r4"),
            ],
            "lvx": [
                ("opaque", "r3"),
                ("opaque", "r4"),
                ("opaque", "r1"),
            ],
            "psq_l": [
                ("opaque", "f2"),
                ("signed", 0),
                ("opaque", "r1"),
            ],
            "lwzu": [
                ("opaque", "r3"),
                ("signed", -4),
                ("opaque", "r1"),
            ],
            "stwu": [
                ("opaque", "r3"),
                ("signed", 8),
                ("opaque", "r1"),
            ],
        }
        for index, (mnemonic, arguments) in enumerate(unsupported.items()):
            parts: list[dict[str, object]] = [
                {"opcode": {"mnemonic": mnemonic, "opcode": 0}}
            ]
            for kind, value in arguments:
                parts.append({"arg": {kind: value}})
            instruction = _stack_instruction(0x280 + index * 4, "r3", "r3", -4)
            instruction["formatted"] = f"{mnemonic} ..."
            instruction["parts"] = parts
            report = self.root / f"stack-residue-unsupported-{mnemonic}.json"
            _write_json(report, _stack_residue_report([instruction]))
            with self.subTest(mnemonic=mnemonic), self.assertRaisesRegex(
                module.MatchError, "unsupported"
            ):
                module.inspect_stack_residue(
                    self.root,
                    report=report,
                    focus_symbol="focus",
                )

    def test_stack_residue_keeps_true_named_crackom_listdebug_capcheck_candidates(self) -> None:
        for index, name in enumerate(
            ("CapEffCrackOMExec", "mbCapListDebug", "CapCheckComPath")
        ):
            report_value = _stack_residue_report(
                [_stack_instruction(0x2C0 + index * 0x10, "stw", "r0", -4)]
            )
            report_value["left"]["symbols"][0]["name"] = name
            report_value["right"]["symbols"][0]["name"] = name
            report = self.root / f"stack-residue-{name}.json"
            _write_json(report, report_value)
            with self.subTest(symbol=name):
                result = module.inspect_stack_residue(
                    self.root,
                    report=report,
                    focus_symbol=name,
                )
                self.assertEqual([row["offset"] for row in result["slots"]], [-4])
                self.assertEqual(result["zero_read_slot_count"], 1)

    def test_stack_residue_excludes_negative_frame_pointer_update(self) -> None:
        report = self.root / "stack-residue-frame-update.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_instruction(0x300, "stwu", "r1", -32),
                    _stack_instruction(0x304, "stw", "r3", 8),
                ]
            ),
        )

        result = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )

        self.assertEqual(
            [
                (
                    row["offset"],
                    row["write_count"],
                    row["residue_write_count"],
                    row["excluded_write_count"],
                    row["residue_candidate"],
                    row["excluded_from_residue"],
                )
                for row in result["stack_slots"]
            ],
            [(-32, 1, 0, 1, False, True), (8, 1, 1, 0, True, False)],
        )
        frame_write = result["stack_slots"][0]["writes"][0]
        self.assertEqual(frame_write["classification"], "frame_pointer_update")
        self.assertTrue(frame_write["excluded_from_residue"])
        self.assertEqual([row["offset"] for row in result["slots"]], [8])
        self.assertEqual(result["slots"], result["zero_read_slots"])
        self.assertEqual(result["excluded_stack_access_count"], 1)
        self.assertEqual(result["excluded_stack_slot_count"], 1)

    def test_stack_residue_excludes_outgoing_argument_store_only_with_call_context(self) -> None:
        report = self.root / "stack-residue-outgoing-argument.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_instruction(0x400, "stw", "r0", 8),
                    _stack_register_instruction(0x404, "addi", "r3"),
                    _stack_register_instruction(0x408, "li", "r4"),
                    _stack_direct_call_instruction(0x40C),
                ]
            ),
        )

        first = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        second = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )
        self.assertEqual(first, second)
        self.assertEqual(first["zero_read_slot_count"], 0)
        self.assertEqual(first["outgoing_call_argument_access_count"], 1)
        self.assertEqual(first["outgoing_call_argument_slot_count"], 1)
        slot = first["stack_slots"][0]
        self.assertEqual(slot["offset"], 8)
        self.assertEqual(slot["write_count"], 1)
        self.assertEqual(slot["residue_write_count"], 0)
        self.assertEqual(slot["excluded_write_count"], 1)
        self.assertFalse(slot["residue_candidate"])
        self.assertTrue(slot["excluded_from_residue"])
        write = slot["writes"][0]
        self.assertEqual(write["classification"], "outgoing_call_argument")
        self.assertTrue(write["excluded_from_residue"])
        self.assertEqual(
            write["call_context"]["argument_destinations"], ["r3", "r4"]
        )
        self.assertEqual(write["call_context"]["call_instruction_index"], 3)
        self.assertEqual(
            write["call_context"]["proof"],
            "direct_bl_reloc_after_contiguous_argument_setup",
        )

    def test_stack_residue_keeps_offset_eight_without_proven_call_context(self) -> None:
        report = self.root / "stack-residue-offset-eight-local.json"
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_instruction(0x500, "stw", "r0", 8),
                    _stack_register_instruction(0x504, "lwz", "r30"),
                    _stack_direct_call_instruction(0x508, "mbExitCheck"),
                ]
            ),
        )

        result = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )

        slot = result["stack_slots"][0]
        self.assertEqual(slot["offset"], 8)
        self.assertEqual(slot["residue_write_count"], 1)
        self.assertEqual(slot["excluded_write_count"], 0)
        self.assertTrue(slot["residue_candidate"])
        self.assertEqual(result["outgoing_call_argument_access_count"], 0)
        self.assertEqual(result["slots"], result["zero_read_slots"])

    def test_stack_residue_keeps_positive_store_without_direct_reloc_call(self) -> None:
        report = self.root / "stack-residue-no-direct-call.json"
        bad_call = _stack_direct_call_instruction(0x60C)
        bad_call["parts"] = [
            {"opcode": {"mnemonic": "bl", "opcode": 267}},
            {"arg": {"opaque": "OSReport"}},
        ]
        _write_json(
            report,
            _stack_residue_report(
                [
                    _stack_instruction(0x600, "stw", "r0", 0x10),
                    _stack_register_instruction(0x604, "li", "r3"),
                    _stack_register_instruction(0x608, "li", "r4"),
                    bad_call,
                ]
            ),
        )

        result = module.inspect_stack_residue(
            self.root,
            report=report,
            focus_symbol="focus",
        )

        slot = result["stack_slots"][0]
        self.assertEqual(slot["offset"], 0x10)
        self.assertTrue(slot["residue_candidate"])
        self.assertEqual(slot["writes"][0]["classification"], "stack_write")
        self.assertEqual(result["outgoing_call_argument_access_count"], 0)

    def test_residuals_reject_strict_data_identity_or_pairing_mismatch(self) -> None:
        strict_path = self.root / "residual-mismatch-strict.json"
        data_path = self.root / "residual-mismatch-data.json"
        _write_json(
            strict_path,
            _residual_report((("alpha", 75.0, "4", "4", ("REG",)),)),
        )
        _write_json(
            data_path,
            _residual_report((("beta", 75.0, "4", "4", ("REG",)),)),
        )
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "residuals",
                    "--strict-report",
                    str(strict_path),
                    "--data-report",
                    str(data_path),
                ]
            )
        self.assertEqual(status, 2)
        self.assertIn("strict/data function identity mismatch", output.getvalue())

        unpaired = _residual_report(
            (("alpha", 75.0, "4", "4", ("REG",)),)
        )
        unpaired["left"]["symbols"][0]["target_symbol"] = 99
        _write_json(data_path, unpaired)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "residuals",
                    "--strict-report",
                    str(strict_path),
                    "--data-report",
                    str(data_path),
                ]
            )
        self.assertEqual(status, 2)
        self.assertIn("is not paired", output.getvalue())

    def test_assess_repeatable_focus_symbols_are_sorted_and_each_focus_is_checked(self) -> None:
        baseline_strict = self.root / "multi-baseline-strict.json"
        candidate_strict = self.root / "multi-candidate-strict.json"
        baseline_data = self.root / "multi-baseline-data.json"
        candidate_data = self.root / "multi-candidate-data.json"
        baseline = _assessment_multi_report(
            (("beta", 75.0), ("alpha", 75.0), ("sibling", 100.0))
        )
        candidate = _assessment_multi_report(
            (("beta", 100.0), ("alpha", 100.0), ("sibling", 100.0))
        )
        _write_json(baseline_strict, baseline)
        _write_json(candidate_strict, candidate)
        _write_json(baseline_data, baseline)
        _write_json(candidate_data, candidate)

        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline_strict),
                    "--candidate-strict",
                    str(candidate_strict),
                    "--baseline-data",
                    str(baseline_data),
                    "--candidate-data",
                    str(candidate_data),
                    "--focus-symbol",
                    "beta",
                    "--focus-symbol",
                    "alpha",
                ]
            )
        self.assertEqual(status, 0)
        result = json.loads(output.getvalue())
        self.assertEqual(result["verdict"], "accepted")
        self.assertEqual(result["focus_symbols"], ["alpha", "beta"])
        self.assertNotIn("focus_symbol", result)
        self.assertNotIn("focus", result)
        self.assertEqual(
            [row["symbol"] for row in result["focuses"]], ["alpha", "beta"]
        )
        self.assertEqual(
            [row["symbol"] for row in result["strict"]["focuses"]], ["alpha", "beta"]
        )
        self.assertNotIn("focus", result["strict"])
        self.assertEqual(result["data"]["focuses"][0]["symbol"], "alpha")

    def test_assess_multi_focus_rejects_one_focus_regression(self) -> None:
        baseline = self.root / "multi-regression-baseline.json"
        candidate = self.root / "multi-regression-candidate.json"
        _write_json(
            baseline,
            _assessment_multi_report((("alpha", 100.0), ("beta", 75.0))),
        )
        _write_json(
            candidate,
            _assessment_multi_report((("alpha", 95.0), ("beta", 100.0))),
        )
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline),
                    "--candidate-strict",
                    str(candidate),
                    "--focus-symbol",
                    "alpha",
                    "--focus-symbol",
                    "beta",
                ]
            )
        self.assertEqual(status, 1)
        result = json.loads(output.getvalue())
        self.assertEqual(result["verdict"], "rejected")
        self.assertEqual(
            [(row["symbol"], row["reason"]) for row in result["regressions"]],
            [("alpha", "previously_exact_focus_regressed")],
        )

    def test_record_multi_focus_persists_sorted_symbols_and_matrix_compacts(self) -> None:
        self._init()
        strict = self.root / "multi-strict.json"
        data = self.root / "multi-data.json"
        _write_json(strict, _assessment_multi_report((("beta", 100.0), ("alpha", 100.0))))
        _write_json(data, _assessment_multi_report((("beta", 100.0), ("alpha", 100.0))))
        first = self._record(
            "multi",
            strict_report=strict,
            data_report=data,
            focus_symbol=["beta", "alpha"],
        )
        record = first["record"]
        self.assertEqual(record["focus_symbols"], ["alpha", "beta"])
        self.assertNotIn("focus_symbol", record)
        self.assertEqual(
            [row["name"] for row in record["reports"]["strict"]["compact"]["focuses"]],
            ["alpha", "beta"],
        )
        self.assertNotIn("focus", record["reports"]["strict"]["compact"])

        unchanged = module.record_candidate(
            self.root,
            self.workspace,
            candidate_id="multi",
            source=self.source,
            object_path=self.object,
            compile_attestation=self._attestation("multi"),
            strict_report=strict,
            data_report=data,
            hypothesis="natural candidate",
            axis="register-lifetime",
            focus_symbol=["alpha", "beta"],
        )
        self.assertEqual(unchanged["status"], "unchanged")
        matrix = module.build_matrix(self.root, self.workspace)
        row = matrix["rows"][0]
        self.assertEqual(row["focus_symbols"], ["alpha", "beta"])
        self.assertEqual(
            [focus["name"] for focus in row["strict_focuses"]], ["alpha", "beta"]
        )
        self.assertTrue(row["next_action"].startswith("authenticate"))

    def test_prepare_composes_record_request_without_mutating_workspace(self) -> None:
        baseline_strict = self.root / "prepare-baseline-strict.json"
        candidate_strict = self.root / "prepare-candidate-strict.json"
        baseline_data = self.root / "prepare-baseline-data.json"
        candidate_data = self.root / "prepare-candidate-data.json"
        _write_json(baseline_strict, _assessment_report(focus_match=75.0))
        _write_json(candidate_strict, _assessment_report(focus_match=100.0))
        _write_json(baseline_data, _assessment_report(focus_match=75.0))
        _write_json(candidate_data, _assessment_report(focus_match=100.0))
        self._init()
        compile_attestation = self._attestation("prepare-ready")

        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "prepare",
                    "--baseline-strict",
                    str(baseline_strict),
                    "--candidate-strict",
                    str(candidate_strict),
                    "--baseline-data",
                    str(baseline_data),
                    "--candidate-data",
                    str(candidate_data),
                    "--focus-symbol",
                    "focus",
                    "--workspace",
                    str(self.workspace),
                    "--candidate-id",
                    "prepared",
                    "--source",
                    str(self.source),
                    "--object",
                    str(self.object),
                    "--compile-attestation",
                    str(compile_attestation),
                    "--hypothesis",
                    "natural candidate",
                    "--axis",
                    "register-lifetime",
                    "--reason",
                    "accepted report pair",
                    "--json",
                ]
            )
        self.assertEqual(status, 0)
        result = json.loads(output.getvalue())
        self.assertEqual(result["schema"], module.PREPARATION_SCHEMA)
        self.assertEqual(result["status"], "ready")
        self.assertEqual(result["assessment"]["verdict"], "accepted")
        request = result["record_request"]
        self.assertEqual(request["candidate_id"], "prepared")
        self.assertEqual(request["strict_report"], str(candidate_strict.resolve()))
        self.assertEqual(request["data_report"], str(candidate_data.resolve()))
        self.assertEqual(request["focus_symbol"], "focus")
        self.assertEqual(request["reason"], "accepted report pair")
        self.assertEqual(result["artifacts"]["source"]["sha256"], _sha256(self.source))
        self.assertEqual(result["artifacts"]["object"]["sha256"], _sha256(self.object))
        self.assertEqual(list((self.workspace / "candidates").glob("*.json")), [])
        self.assertEqual(
            json.loads((self.workspace / "index.json").read_text(encoding="utf-8"))["sequence"],
            0,
        )

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "prepare",
                "--baseline-strict",
                str(baseline_strict),
                "--candidate-strict",
                str(candidate_strict),
                "--focus-symbol",
                "focus",
                "--workspace",
                str(self.workspace),
                "--candidate-id",
                "prepared-central",
                "--source",
                str(self.source),
                "--object",
                str(self.object),
                "--compile-attestation",
                str(compile_attestation),
                "--hypothesis",
                "natural candidate",
                "--axis",
                "register-lifetime",
                "--json",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout)["status"], "ready")

    def test_prepare_withholds_record_request_on_sibling_regression(self) -> None:
        baseline_strict = self.root / "prepare-regression-baseline.json"
        candidate_strict = self.root / "prepare-regression-candidate.json"
        _write_json(
            baseline_strict,
            _assessment_report(focus_match=75.0, sibling_match=100.0),
        )
        _write_json(
            candidate_strict,
            _assessment_report(focus_match=100.0, sibling_match=95.0),
        )
        self._init()
        compile_attestation = self._attestation("prepare-rejected")

        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "prepare",
                    "--baseline-strict",
                    str(baseline_strict),
                    "--candidate-strict",
                    str(candidate_strict),
                    "--focus-symbol",
                    "focus",
                    "--workspace",
                    str(self.workspace),
                    "--candidate-id",
                    "rejected",
                    "--source",
                    str(self.source),
                    "--object",
                    str(self.object),
                    "--compile-attestation",
                    str(compile_attestation),
                    "--hypothesis",
                    "natural candidate",
                    "--axis",
                    "register-lifetime",
                    "--json",
                ]
            )
        self.assertEqual(status, 1)
        result = json.loads(output.getvalue())
        self.assertEqual(result["status"], "rejected")
        self.assertIsNone(result["record_request"])
        self.assertEqual(result["assessment"]["verdict"], "rejected")
        self.assertEqual(
            result["assessment"]["regressions"][0]["reason"],
            "previously_exact_sibling_regressed",
        )
        self.assertEqual(list((self.workspace / "candidates").glob("*.json")), [])
        self.assertEqual(
            json.loads((self.workspace / "index.json").read_text(encoding="utf-8"))["sequence"],
            0,
        )

    def test_assess_rejects_previously_exact_sibling_regression(self) -> None:
        baseline = self.root / "regression-baseline.json"
        candidate = self.root / "regression-candidate.json"
        _write_json(baseline, _assessment_report(focus_match=75.0, sibling_match=100.0))
        _write_json(candidate, _assessment_report(focus_match=100.0, sibling_match=95.0))
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline),
                        "--candidate-strict",
                        str(candidate),
                        "--focus-symbol",
                        "focus",
                        "--json",
                    ]
                ),
                1,
            )
        result = json.loads(output.getvalue())
        self.assertEqual(result["verdict"], "rejected")
        self.assertEqual(len(result["regressions"]), 1)
        self.assertEqual(result["regressions"][0]["symbol"], "sibling")
        self.assertEqual(result["regressions"][0]["reason"], "previously_exact_sibling_regressed")

    def test_assess_rejects_focus_regression_without_sibling_loss(self) -> None:
        baseline = self.root / "focus-regression-baseline.json"
        candidate = self.root / "focus-regression-candidate.json"
        _write_json(baseline, _assessment_report(focus_match=99.95, sibling_match=100.0))
        _write_json(candidate, _assessment_report(focus_match=99.74, sibling_match=100.0))
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline),
                        "--candidate-strict",
                        str(candidate),
                        "--focus-symbol",
                        "focus",
                        "--json",
                    ]
                ),
                1,
            )
        result = json.loads(output.getvalue())
        self.assertEqual(result["verdict"], "rejected")
        self.assertEqual(len(result["regressions"]), 1)
        self.assertEqual(result["regressions"][0]["symbol"], "focus")
        self.assertEqual(result["regressions"][0]["reason"], "focus_match_percent_regressed")

    def test_assess_rejects_missing_or_unpaired_focus(self) -> None:
        baseline = self.root / "focus-baseline.json"
        candidate = self.root / "focus-candidate.json"
        _write_json(baseline, _assessment_report())
        _write_json(candidate, _assessment_report())

        missing_output = io.StringIO()
        with contextlib.redirect_stdout(missing_output):
            missing_status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline),
                    "--candidate-strict",
                    str(candidate),
                    "--focus-symbol",
                    "missing",
                ]
            )
        self.assertEqual(missing_status, 2)
        self.assertIn("lacks requested focus symbol", missing_output.getvalue())

        unpaired = _assessment_report()
        unpaired["right"]["symbols"][0]["name"] = "different"
        unpaired["left"]["symbols"][0]["target_symbol"] = 99
        _write_json(candidate, unpaired)
        unpaired_output = io.StringIO()
        with contextlib.redirect_stdout(unpaired_output):
            unpaired_status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline),
                    "--candidate-strict",
                    str(candidate),
                    "--focus-symbol",
                    "focus",
                ]
            )
        self.assertEqual(unpaired_status, 2)
        self.assertIn("is not paired", unpaired_output.getvalue())

    def test_assess_rejects_invalid_target_index_even_with_same_name(self) -> None:
        baseline = self.root / "target-index-baseline.json"
        candidate = self.root / "target-index-candidate.json"
        _write_json(baseline, _assessment_report())

        missing = object()
        for label, target_index in (
            ("absent", missing),
            ("null", None),
            ("out-of-range", 99),
        ):
            malformed = _assessment_report()
            focus = malformed["left"]["symbols"][0]
            if target_index is missing:
                del focus["target_symbol"]
            else:
                focus["target_symbol"] = target_index
            # The right-side symbol deliberately keeps the same name.  A
            # canonical objdiff report must trust only target_symbol.
            _write_json(candidate, malformed)
            output = io.StringIO()
            with self.subTest(target_symbol=label), contextlib.redirect_stdout(output):
                status = module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline),
                        "--candidate-strict",
                        str(candidate),
                        "--focus-symbol",
                        "focus",
                    ]
                )
            self.assertEqual(status, 2)
            self.assertIn("is not paired", output.getvalue())

    def test_assess_rejects_metadata_or_symbol_free_reports(self) -> None:
        baseline = self.root / "shape-baseline.json"
        candidate = self.root / "shape-candidate.json"
        _write_json(candidate, _assessment_report())
        for malformed in (
            {"metadata": {"tool": "objdiff"}},
            {"left": {"symbols": []}, "right": {"symbols": []}},
        ):
            _write_json(baseline, malformed)
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                status = module.main(
                    [
                        "--root",
                        str(self.root),
                        "assess",
                        "--baseline-strict",
                        str(baseline),
                        "--candidate-strict",
                        str(candidate),
                        "--focus-symbol",
                        "focus",
                    ]
                )
            self.assertEqual(status, 2)
            self.assertIn("error:", output.getvalue())

    def test_assess_rejects_strict_data_focus_pairing_mismatch(self) -> None:
        baseline_strict = self.root / "pair-baseline-strict.json"
        candidate_strict = self.root / "pair-candidate-strict.json"
        baseline_data = self.root / "pair-baseline-data.json"
        candidate_data = self.root / "pair-candidate-data.json"
        _write_json(baseline_strict, _assessment_report())
        _write_json(candidate_strict, _assessment_report())
        data_baseline = _assessment_report()
        data_candidate = _assessment_report()
        data_baseline["right"]["symbols"][0]["name"] = "different"
        data_candidate["right"]["symbols"][0]["name"] = "different"
        _write_json(baseline_data, data_baseline)
        _write_json(candidate_data, data_candidate)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            status = module.main(
                [
                    "--root",
                    str(self.root),
                    "assess",
                    "--baseline-strict",
                    str(baseline_strict),
                    "--candidate-strict",
                    str(candidate_strict),
                    "--baseline-data",
                    str(baseline_data),
                    "--candidate-data",
                    str(candidate_data),
                    "--focus-symbol",
                    "focus",
                ]
            )
        self.assertEqual(status, 2)
        self.assertIn("strict/data focus pairing mismatch", output.getvalue())

    def test_missing_index_with_existing_records_fails_closed(self) -> None:
        self._init()
        self._record()
        (self.workspace / "index.json").unlink()
        with self.assertRaisesRegex(module.MatchError, "index"):
            module.build_matrix(self.root, self.workspace)

    def test_direct_module_cli_and_central_agent_routing(self) -> None:
        manifest_arg = str(self.manifest)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(module.main(["--root", str(self.root), "init", manifest_arg, "--workspace", str(self.workspace), "--json"]), 0)
        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [sys.executable, str(central), "--root", str(self.root), "match", "lookup", "--workspace", str(self.workspace), "--source", str(self.source), "--json"],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout)["status"], "new")

        self.target.write_bytes(b"mutated-target")
        repair = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "repair-target",
                "--workspace",
                str(self.workspace),
                "--json",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(repair.returncode, 0, repair.stderr or repair.stdout)
        self.assertEqual(json.loads(repair.stdout)["status"], "restored")
        self.assertFalse(json.loads(repair.stdout)["authority_advanced"])

    def test_default_text_diagnose_and_matrix_output_has_operational_counts(self) -> None:
        self._init()
        self._record()
        jobs = self._jobs(self._job_script("text-output.py"), job_ids=("text-output",))
        diagnose_text = io.StringIO()
        with contextlib.redirect_stdout(diagnose_text):
            self.assertEqual(
                module.main(
                    [
                        "--root",
                        str(self.root),
                        "diagnose",
                        "--workspace",
                        str(self.workspace),
                        "--candidate-id",
                        "c1",
                        "--jobs",
                        str(jobs),
                    ]
                ),
                0,
            )
        diagnose_output = diagnose_text.getvalue().lower()
        self.assertIn("ran", diagnose_output)
        self.assertIn("failed", diagnose_output)

        matrix_text = io.StringIO()
        with contextlib.redirect_stdout(matrix_text):
            self.assertEqual(
                module.main(["--root", str(self.root), "matrix", "--workspace", str(self.workspace)]),
                0,
            )
        matrix_output = matrix_text.getvalue().lower()
        self.assertIn("candidate", matrix_output)
        self.assertIn("next", matrix_output)

    def test_donor_shapes_handles_nested_braces_comments_and_string_literals(self) -> None:
        current = self.root / "capsule.c"
        donor = self.root / "donor.c"
        current.write_text(
            'int CapShopNextGet(int value) {\n'
            '    const char *text = "}"; /* a brace in a comment: { } */\n'
            '    if (value) {\n'
            '        return value;\n'
            '    }\n'
            '    return 0;\n'
            '}\n',
            encoding="utf-8",
        )
        donor.write_text(
            'int CapShopNextGet(int value) {\n'
            '  const char* text = "}"; // comment with }\n'
            '  if (value) {\n'
            '    return value + 1;\n'
            '  }\n'
            '  return 0;\n'
            '}\n',
            encoding="utf-8",
        )

        result = module.donor_shapes(
            self.root,
            source=current,
            focus_symbol="CapShopNextGet",
            donor_files=[str(donor)],
        )

        self.assertEqual(result["schema"], module.DONOR_SHAPES_SCHEMA)
        self.assertEqual(result["focus_symbol"], "CapShopNextGet")
        self.assertEqual(result["current"]["source"]["sha256"], _sha256(current))
        self.assertEqual(result["variant_count"], 1)
        self.assertEqual(result["donor_definition_count"], 1)
        variant = result["variants"][0]
        self.assertEqual(variant["rank"], 1)
        self.assertGreater(variant["source_shape_diff_line_count"], 0)
        self.assertEqual(variant["representative"]["path"], "donor.c")
        self.assertFalse(result["target_proof"])
        self.assertFalse(result["auto_edit"])
        self.assertEqual(result["evidence_class"], "donor_source_shape_only")

    def test_donor_shapes_deduplicates_normalized_bodies_and_ranks_deterministically(self) -> None:
        current = self.root / "current.c"
        donor_a = self.root / "z-donor.c"
        donor_b = self.root / "a-donor.c"
        donor_c = self.root / "m-donor.c"
        current.write_text(
            "int CapShopNextGet(int value) { return value; }\n",
            encoding="utf-8",
        )
        duplicate_body = "int CapShopNextGet(int value) { /* same */ return value; }\n"
        donor_a.write_text(duplicate_body, encoding="utf-8")
        donor_b.write_text(duplicate_body.replace("/* same */", "// same\n"), encoding="utf-8")
        donor_c.write_text(
            "int CapShopNextGet(int value) { return value + 1; }\n",
            encoding="utf-8",
        )

        first = module.donor_shapes(
            self.root,
            source=current,
            focus_symbol="CapShopNextGet",
            donor_files=[str(donor_a), str(donor_c), str(donor_b)],
        )
        second = module.donor_shapes(
            self.root,
            source=current,
            focus_symbol="CapShopNextGet",
            donor_files=[str(donor_b), str(donor_a), str(donor_c)],
        )

        self.assertEqual(first, second)
        self.assertEqual(first["variant_count"], 2)
        self.assertEqual(first["donor_definition_count"], 3)
        self.assertEqual(first["variants"][0]["donor_count"], 2)
        self.assertEqual(
            first["variants"][0]["representative"]["path"], "a-donor.c"
        )
        self.assertEqual(first["variants"][0]["rank"], 1)
        self.assertEqual(first["variants"][1]["rank"], 2)
        self.assertEqual(
            [item["source"]["path"] for item in first["variants"][0]["donors"]],
            ["a-donor.c", "z-donor.c"],
        )

        central = Path(__file__).resolve().parents[1] / "agent.py"
        process = subprocess.run(
            [
                sys.executable,
                str(central),
                "--root",
                str(self.root),
                "match",
                "donor-shapes",
                "--source",
                str(current),
                "--focus-symbol",
                "CapShopNextGet",
                "--donor-file",
                str(donor_a),
                "--donor-file",
                str(donor_b),
                "--donor-file",
                str(donor_c),
                "--json",
            ],
            cwd=central.parent.parent,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(process.returncode, 0, process.stderr or process.stdout)
        self.assertEqual(json.loads(process.stdout), first)

    def test_donor_shapes_search_root_is_explicit_and_does_not_escape_scope(self) -> None:
        current = self.root / "current.c"
        scope = self.root / "scope"
        outside = self.root / "outside"
        scope.mkdir()
        outside.mkdir()
        in_scope = scope / "capsule.c"
        out_scope = outside / "capsule.c"
        current.write_text(
            "int CapShopNextGet(void) { return 0; }\n", encoding="utf-8"
        )
        in_scope.write_text(
            "int CapShopNextGet(void) { return 1; }\n", encoding="utf-8"
        )
        out_scope.write_text(
            "int CapShopNextGet(void) { return 2; }\n", encoding="utf-8"
        )
        (scope / "ignored.txt").write_text(
            "int CapShopNextGet(void) { return 3; }\n", encoding="utf-8"
        )

        result = module.donor_shapes(
            self.root,
            source=current,
            focus_symbol="CapShopNextGet",
            search_roots=[str(scope)],
        )

        self.assertEqual(result["scope"]["search_roots"], ["scope"])
        self.assertEqual(result["scope"]["scanned_file_count"], 1)
        self.assertEqual(result["scope"]["matched_file_count"], 1)
        self.assertEqual(result["variants"][0]["representative"]["path"], "scope/capsule.c")
        serialized = json.dumps(result, sort_keys=True)
        self.assertNotIn("outside/capsule.c", serialized)

    def test_donor_shapes_fails_closed_for_missing_and_ambiguous_focus(self) -> None:
        missing = self.root / "missing.c"
        current = self.root / "current.c"
        ambiguous = self.root / "ambiguous.c"
        current.write_text("int CapShopNextGet(void) { return 0; }\n", encoding="utf-8")
        missing.write_text("int Other(void) { return 0; }\n", encoding="utf-8")
        with self.assertRaisesRegex(module.MatchError, "not found"):
            module.donor_shapes(
                self.root,
                source=current,
                focus_symbol="CapShopNextGet",
                donor_files=[str(missing)],
            )

        ambiguous.write_text(
            "int CapShopNextGet(void) { return 0; }\n"
            "int CapShopNextGet(void) { return 1; }\n",
            encoding="utf-8",
        )
        with self.assertRaisesRegex(module.MatchError, "ambiguous"):
            module.donor_shapes(
                self.root,
                source=current,
                focus_symbol="CapShopNextGet",
                donor_files=[str(ambiguous)],
            )
        with self.assertRaisesRegex(module.MatchError, "ambiguous"):
            module.donor_shapes(
                self.root,
                source=ambiguous,
                focus_symbol="CapShopNextGet",
                donor_files=[str(current)],
            )

    def test_donor_shapes_rejects_extracted_target_path(self) -> None:
        current = self.root / "current.c"
        extracted_target = (
            self.root / "build" / "capsule-v376" / "GP6E01" / "obj" / "board" / "donor.c"
        )
        extracted_target.parent.mkdir(parents=True)
        current.write_text("int CapShopNextGet(void) { return 0; }\n", encoding="utf-8")
        extracted_target.write_text(
            "int CapShopNextGet(void) { return 1; }\n", encoding="utf-8"
        )

        with self.assertRaisesRegex(module.MatchError, "target role.*candidate/donor role"):
            module.donor_shapes(
                self.root,
                source=current,
                focus_symbol="CapShopNextGet",
                donor_files=[str(extracted_target)],
            )

    def test_donor_registry_aliases_converge_by_authenticated_shape(self) -> None:
        source = self.root / "capsule.c"
        source.write_text("int CapShopNextGet(void) { return 0; }\n", encoding="utf-8")
        registry = self.root / "build" / "donors.json"

        first = module.register_donor_shape(
            self.root,
            registry,
            source=source,
            focus_symbol="CapShopNextGet",
            source_kind="same-TU",
            donor_id="shop-list-donor",
            aliases=["shop-list-natural"],
            used_by_candidate_ids=["candidate-a"],
        )
        second = module.register_donor_shape(
            self.root,
            registry,
            source=source,
            focus_symbol="CapShopNextGet",
            source_kind="same_tu",
            donor_id="shop-list-alias",
            aliases=["shop-list-retry"],
            queried_by_candidate_ids=["candidate-a"],
        )

        self.assertEqual(first["status"], "registered")
        self.assertEqual(second["status"], "duplicate")
        self.assertEqual(first["canonical_id"], second["canonical_id"])
        listed = module.list_donor_shapes(self.root, registry)
        self.assertEqual(listed["record_count"], 1)
        record = listed["records"][0]
        self.assertEqual(
            set(record["aliases"]),
            {"shop-list-donor", "shop-list-natural", "shop-list-alias", "shop-list-retry"},
        )
        self.assertEqual(record["used_by_candidate_ids"], ["candidate-a"])
        self.assertEqual(record["queried_by_candidate_ids"], ["candidate-a"])
        looked_up = module.lookup_donor_shapes(
            self.root,
            registry,
            donor_id="SHOP-LIST-RETRY",
            candidate_id="candidate-b",
        )
        self.assertEqual(looked_up["record_count"], 1)
        self.assertIn("candidate-b", looked_up["records"][0]["queried_by_candidate_ids"])

    def test_donor_registry_target_objects_are_rejected_before_persistence(self) -> None:
        source = self.root / "capsule.c"
        source.write_text("int CapShopNextGet(void) { return 0; }\n", encoding="utf-8")
        registry = self.root / "build" / "donors.json"
        target_object = (
            self.root / "build" / "GP6E01" / "obj" / "board" / "capsule.o"
        )
        target_object.parent.mkdir(parents=True)
        target_object.write_bytes(b"extracted-target")

        with self.assertRaisesRegex(module.MatchError, "target-derived source"):
            module.register_donor_shape(
                self.root,
                registry,
                source=source,
                focus_symbol="CapShopNextGet",
                source_kind="target-derived",
                donor_id="bad-target",
            )
        with self.assertRaisesRegex(module.MatchError, "target role"):
            module.register_donor_shape(
                self.root,
                registry,
                source=target_object,
                focus_symbol="CapShopNextGet",
                source_kind="same-tu",
                donor_id="bad-object",
            )
        rejected = module.reject_donor_shape(
            self.root,
            registry,
            source=target_object,
            reason="extracted target object is never a source donor",
        )
        self.assertEqual(rejected["status"], "target-rejected")
        listed = module.list_donor_shapes(self.root, registry, include_rejections=True)
        self.assertEqual(listed["record_count"], 0)
        self.assertEqual(len(listed["rejections"]), 1)
        self.assertEqual(listed["rejections"][0]["source_kind"], "target-derived")

    def test_donor_registry_preserves_explicit_duplicate_record_link(self) -> None:
        canonical_source = self.root / "canonical.c"
        alternate_source = self.root / "alternate.c"
        canonical_source.write_text(
            "int CapShopNextGet(void) { return 0; }\n", encoding="utf-8"
        )
        alternate_source.write_text(
            "int CapShopNextGet(void) { return 1; }\n", encoding="utf-8"
        )
        registry = self.root / "build" / "donors.json"
        canonical = module.register_donor_shape(
            self.root,
            registry,
            source=canonical_source,
            focus_symbol="CapShopNextGet",
            source_kind="diagnostic-only",
            donor_id="canonical-candidate",
            status="rejected",
            admissibility="inadmissible",
        )
        alternate = module.register_donor_shape(
            self.root,
            registry,
            source=alternate_source,
            focus_symbol="CapShopNextGet",
            source_kind="diagnostic-only",
            donor_id="alternate-candidate",
            duplicate_of=canonical["canonical_id"],
            status="rejected",
            admissibility="inadmissible",
        )

        self.assertEqual(alternate["record"]["duplicate_of"], canonical["canonical_id"])
        listed = module.list_donor_shapes(self.root, registry)
        self.assertEqual(listed["record_count"], 2)
        alternate_record = next(
            row for row in listed["records"] if row["canonical_id"] == alternate["canonical_id"]
        )
        self.assertEqual(alternate_record["duplicate_of"], canonical["canonical_id"])

    def test_donor_registry_rejects_conflicting_alias_without_mutation(self) -> None:
        source_a = self.root / "a.c"
        source_b = self.root / "b.c"
        source_a.write_text("int CapShopNextGet(void) { return 0; }\n", encoding="utf-8")
        source_b.write_text("int CapShopNextGet(void) { return 1; }\n", encoding="utf-8")
        registry = self.root / "build" / "donors.json"
        module.register_donor_shape(
            self.root,
            registry,
            source=source_a,
            focus_symbol="CapShopNextGet",
            source_kind="same-tu",
            donor_id="shared-alias",
        )
        with self.assertRaisesRegex(module.MatchError, "already belongs"):
            module.register_donor_shape(
                self.root,
                registry,
                source=source_b,
                focus_symbol="CapShopNextGet",
                source_kind="same-tu",
                donor_id="shared-alias",
            )
        self.assertEqual(module.list_donor_shapes(self.root, registry)["record_count"], 1)


if __name__ == "__main__":
    unittest.main()
