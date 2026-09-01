from __future__ import annotations

import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from contextlib import nullcontext
from unittest.mock import patch

from tools import crack_current_residual as residual


FUNCTION = "FocusFunction"
OWNER = "main:test/focus"
UNIT = "main/focus"
TOOLCHAIN = "b" * 64
BASE_COMMIT = "1" * 40


def _file_descriptor(path: str, byte: str = "a") -> dict[str, object]:
    return {"path": path, "sha256": byte * 64, "size_bytes": 1}


def _hash_descriptor(byte: str = "a") -> dict[str, object]:
    return {"sha256": byte * 64, "size_bytes": 1}


def _report(*, changed: bool = True) -> dict[str, object]:
    target_row: dict[str, object] = {
        "instruction": {"address": "0x100", "formatted": "li r3,0"}
    }
    candidate_row: dict[str, object] = {
        "instruction": {"address": "0x100", "formatted": "li r4,0"}
    }
    if changed:
        target_row["diff_kind"] = "DIFF_ARG_MISMATCH"
        candidate_row["diff_kind"] = "DIFF_ARG_MISMATCH"
    return {
        "left": {
            "symbols": [
                {
                    "name": FUNCTION,
                    "kind": "SYMBOL_FUNCTION",
                    "instructions": [target_row],
                }
            ]
        },
        "right": {
            "symbols": [
                {
                    "name": FUNCTION,
                    "kind": "SYMBOL_FUNCTION",
                    "instructions": [candidate_row],
                }
            ]
        },
    }


def _sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class CurrentResidualPureTests(unittest.TestCase):
    def test_focus_payload_limit_admits_large_board_functions_but_stays_bounded(self) -> None:
        residual._validate_focus_payload_size(b"x" * 611_412)
        residual._validate_focus_payload_size(b"x" * residual.MAX_FOCUS_BYTES)
        with self.assertRaisesRegex(
            residual.ResidualEvidenceError, "focus artifact exceeds compact evidence limit"
        ):
            residual._validate_focus_payload_size(
                b"x" * (residual.MAX_FOCUS_BYTES + 1)
            )

    def test_row_ids_bind_both_proof_channels(self) -> None:
        strict = _report()
        data = _report()
        rows = residual.residual_row_ids(strict, data, FUNCTION)
        self.assertEqual(rows, [
            "strict:FocusFunction:row:0:kind=DIFF_ARG_MISMATCH:target=0x100:candidate=0x100",
            "data:FocusFunction:row:0:kind=DIFF_ARG_MISMATCH:target=0x100:candidate=0x100",
        ])

    def test_row_ids_are_empty_only_when_both_channels_are_exact(self) -> None:
        self.assertEqual(
            residual.residual_row_ids(
                _report(changed=False), _report(changed=False), FUNCTION
            ),
            [],
        )

    def test_artifact_is_closed_self_hashed_and_bounded(self) -> None:
        artifact = residual.build_residual_artifact(
            owner=OWNER,
            function=FUNCTION,
            base_sha256="a" * 64,
            source_sha256="a" * 64,
            target_sha256="c" * 64,
            base_commit=BASE_COMMIT,
            unit=UNIT,
            start_line=2,
            end_line=8,
            base_span_sha256="d" * 64,
            toolchain_key=TOOLCHAIN,
            residual_rows=["strict:FocusFunction:row:0:kind=ARG"],
            base_object=_file_descriptor("evidence/base.o"),
            target_object=_file_descriptor("evidence/target.o", "b"),
            focus_report=_file_descriptor("evidence/focus.json", "c"),
            physical_summary=_file_descriptor("evidence/physical.json", "d"),
            strict_report=_hash_descriptor("e"),
            data_report=_hash_descriptor("f"),
            physical_receipt=_hash_descriptor("1"),
            producer=_file_descriptor("tools/crack_current_residual.py", "2"),
        )
        self.assertEqual(set(artifact), {
            "schema", "owner", "function", "base_commit", "unit",
            "base_sha256", "source_sha256", "target_sha256",
            "function_span", "toolchain_key", "base_object", "target_object",
            "focus_report", "physical_summary", "strict_report", "data_report",
            "physical_receipt", "producer", "residual_rows",
            "current_source_bound", "authority_advanced", "residual_sha256",
        })
        unsigned = dict(artifact)
        digest = unsigned.pop("residual_sha256")
        self.assertEqual(digest, residual._json_sha(unsigned))
        self.assertFalse(artifact["authority_advanced"])

    def test_artifact_requires_same_current_source_for_base_and_source(self) -> None:
        with self.assertRaisesRegex(residual.ResidualEvidenceError, "same current source"):
            residual.build_residual_artifact(
                owner=OWNER,
                function=FUNCTION,
                base_sha256="a" * 64,
                source_sha256="b" * 64,
                target_sha256="c" * 64,
                base_commit=BASE_COMMIT,
                unit=UNIT,
                start_line=1,
                end_line=1,
                base_span_sha256="d" * 64,
                toolchain_key=TOOLCHAIN,
                residual_rows=["row"],
                base_object=_file_descriptor("evidence/base.o"),
                target_object=_file_descriptor("evidence/target.o"),
                focus_report=_file_descriptor("evidence/focus.json"),
                physical_summary=_file_descriptor("evidence/physical.json"),
                strict_report=_hash_descriptor(),
                data_report=_hash_descriptor(),
                physical_receipt=_hash_descriptor(),
                producer=_file_descriptor("tools/crack_current_residual.py"),
            )


class CurrentResidualMaterializationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        (self.root / ".git").mkdir()
        (self.root / "src").mkdir()
        (self.root / "build").mkdir()
        (self.root / "tools").mkdir()
        self.producer = self.root / "tools" / "crack_current_residual.py"
        self.producer.write_bytes(b"bound producer")
        self.source = self.root / "src" / "focus.c"
        self.source.write_text("int before;\nint FocusFunction(void) { return 0; }\n", encoding="utf-8")
        self.output = self.root / "build" / "residual.json"
        self.source_sha = _sha(self.source)
        self.target_sha = hashlib.sha256(b"target-object").hexdigest()
        self.authoritative_base = self.root / "build" / "base.o"
        self.authoritative_base.write_bytes(b"authoritative-sentinel")
        self.tool = self.root / "fake-tool.exe"
        self.tool.write_bytes(b"tool")
        self.toolchain = {
            "objdiff": {"path_object": self.tool},
            "binutils": {"path_object": self.root},
            "ninja": {"path_object": self.tool},
        }

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _scratch(self) -> Path:
        scratch = self.root / "scratch-worktree"
        if scratch.exists():
            import shutil
            shutil.rmtree(scratch)
        (scratch / ".git").mkdir(parents=True)
        (scratch / "src").mkdir()
        (scratch / "build").mkdir()
        (scratch / "build" / "target.o").write_bytes(b"target-object")
        (scratch / "objdiff.json").write_text(
            json.dumps({"units": [{
                "name": UNIT,
                "target_path": "build/target.o",
                "base_path": "build/base.o",
            }]}),
            encoding="utf-8",
        )
        return scratch

    def _run_materializer(
        self,
        *,
        report: dict[str, object] | None = None,
        physical_exact: bool = False,
        cleanup_error: str | None = None,
        proof_error: bool = False,
        compile_error: BaseException | None = None,
        focus_override: dict[str, object] | None = None,
    ) -> dict[str, object]:
        report = report or _report()
        scratch = self._scratch()

        def run(command: list[str], *, cwd: Path, label: str, timeout: float) -> str:
            self.assertEqual(label, "current base object build")
            self.assertEqual(command[1:3], ["-j1", "build\\base.o"])
            self.assertEqual(cwd, scratch)
            self.assertGreater(timeout, 0.0)
            if compile_error is not None:
                raise compile_error
            (scratch / "build" / "base.o").write_bytes(b"base-object")
            return "built"

        def run_objdiff(*args: object, **kwargs: object) -> None:
            if proof_error:
                raise residual.bundle.EvidenceError("proof sentinel")
            output = args[3]
            assert isinstance(output, Path)
            output.write_text(json.dumps(report), encoding="utf-8")

        physical = {
            "schema": "mp6_physical_relocation_receipt/v1",
            "target": {
                "size": 12, "instruction_count": 3,
                "physical_relocation_count": 1, "physical_relocations": [],
            },
            "candidate": {
                "size": 10, "instruction_count": 2,
                "physical_relocation_count": 2, "physical_relocations": [],
            },
            "physical_relocations_exact": physical_exact,
            "physical_relocation_differences": [] if physical_exact else [{"row": 0}],
        }

        def normalized_rows(side: str, *, all_rows: bool) -> list[dict[str, object]]:
            raw = report[side]["symbols"][0]["instructions"]
            result: list[dict[str, object]] = []
            for index, row in enumerate(raw):
                if not all_rows and not row.get("diff_kind"):
                    continue
                normalized: dict[str, object] = {"index": index}
                if "diff_kind" in row:
                    normalized["diff_kind"] = row["diff_kind"]
                instruction = row.get("instruction")
                if isinstance(instruction, dict):
                    normalized["instruction"] = dict(instruction)
                result.append(normalized)
            return result

        focus = {
            "schema": "focus_symbol_report/v1",
            "function": FUNCTION,
            "channels": {
                "strict": {
                    "target": {"rows": normalized_rows("left", all_rows=True)},
                    "candidate": {"rows": normalized_rows("right", all_rows=True)},
                },
                "data": {
                    "target": {"rows": normalized_rows("left", all_rows=False)},
                    "candidate": {"rows": normalized_rows("right", all_rows=False)},
                },
            },
            "physical_relocations": {
                "physical_relocation_differences": physical[
                    "physical_relocation_differences"
                ],
            },
        }
        with patch.object(residual, "_git", return_value="commit"), \
             patch.object(residual, "__file__", str(self.producer)), \
             patch.object(residual, "_create_disposable_worktree", return_value=scratch), \
             patch.object(residual, "_remove_disposable_worktree", return_value=cleanup_error), \
             patch.object(residual, "serialized_build_lock", return_value=nullcontext()), \
             patch.object(residual.bundle, "_load_toolchain", return_value=self.toolchain), \
             patch.object(residual, "_verify_objdiff_bounded"), \
             patch.object(residual, "_verify_readelf_bounded"), \
             patch.object(residual, "_verify_ninja_bounded"), \
             patch.object(residual, "_ensure_configured_bounded", return_value=scratch / "retail"), \
             patch.object(residual.bundle, "_remove_staged_retail"), \
             patch.object(residual, "_run_bounded", side_effect=run) as compile_run, \
             patch.object(residual, "_run_objdiff_bounded", side_effect=run_objdiff), \
             patch.object(residual, "_physical_receipt_bounded", return_value=physical), \
             patch.object(
                 residual.focus_symbol_report,
                 "build_from_paths",
                 return_value=focus if focus_override is None else focus_override,
             ):
            result = residual.materialize_current_residual(
                root=self.root,
                base_commit=BASE_COMMIT,
                owner=OWNER,
                unit=UNIT,
                function=FUNCTION,
                source=self.source,
                source_sha256=self.source_sha,
                target_sha256=self.target_sha,
                toolchain_key=TOOLCHAIN,
                start_line=1,
                end_line=2,
                output=self.output,
                manifest_path=self.root / "manifest.json",
            )
        self.assertEqual(compile_run.call_count, 1)
        self.assertEqual(self.authoritative_base.read_bytes(), b"authoritative-sentinel")
        return result

    def test_materializes_once_and_publishes_compact_artifact_and_physical_sidecar(self) -> None:
        result = self._run_materializer()
        artifact_path = self.root / result["artifact"]
        physical_path = self.root / result["physical_summary"]
        focus_path = self.root / result["focus_report"]
        target_path = self.root / result["target_object"]
        base_path = self.root / result["base_object"]
        self.assertTrue(artifact_path.is_file())
        self.assertTrue(physical_path.is_file())
        self.assertTrue(focus_path.is_file())
        self.assertEqual(target_path.read_bytes(), b"target-object")
        self.assertEqual(base_path.read_bytes(), b"base-object")
        artifact = json.loads(artifact_path.read_text(encoding="utf-8"))
        self.assertEqual(artifact["schema"], residual.SCHEMA)
        self.assertEqual(len(artifact["residual_rows"]), 3)
        self.assertTrue(artifact["residual_rows"][0].startswith("strict:"))
        self.assertTrue(artifact["residual_rows"][1].startswith("data:"))
        self.assertTrue(artifact["residual_rows"][2].startswith("physical:"))
        self.assertEqual(artifact["residual_sha256"], residual._json_sha({
            key: value for key, value in artifact.items() if key != "residual_sha256"
        }))
        physical = json.loads(physical_path.read_text(encoding="utf-8"))
        self.assertEqual(physical["physical_difference_count"], 1)
        self.assertEqual(physical["base_physical_relocation_count"], 2)
        self.assertEqual(physical["physical_summary_sha256"], residual._json_sha({
            key: value for key, value in physical.items()
            if key != "physical_summary_sha256"
        }))
        self.assertFalse((self.root / "candidate.o").exists())

    def test_base_commit_verification_avoids_typed_peel_revision_syntax(self) -> None:
        with patch.object(residual, "_git", return_value="commit") as git:
            residual._verify_base_commit(self.root, BASE_COMMIT, timeout=17.0)

        git.assert_called_once_with(
            self.root,
            ["cat-file", "-t", BASE_COMMIT],
            "base commit verification",
            timeout=17.0,
        )
        self.assertNotIn("^", "".join(git.call_args.args[1]))

    def test_focus_compaction_keeps_diff_context_and_hashes_exact_physical_rows(self) -> None:
        rows = [
            {
                "index": index,
                "diff_kind": "DIFF_ARG_MISMATCH" if index == 2500 else None,
                "instruction": {
                    "address": hex(0x1000 + 4 * index),
                    "formatted": f"addi r3,r3,{index}",
                },
            }
            for index in range(5000)
        ]
        physical_rows = [{"offset": index * 4, "type": "R_PPC_REL24"} for index in range(500)]
        focus = {
            "schema": "focus_symbol_report/v1",
            "function": FUNCTION,
            "input_binding": {},
            "channels": {
                "strict": {
                    "target": {"rows_kind": "all", "rows": rows},
                    "candidate": {"rows_kind": "all", "rows": rows},
                },
                "data": {
                    "target": {"rows_kind": "diff_only", "rows": [rows[2500]]},
                    "candidate": {"rows_kind": "diff_only", "rows": [rows[2500]]},
                },
            },
            "physical_relocations": {
                "target": {"physical_relocations": physical_rows},
                "candidate": {"physical_relocations": physical_rows},
                "physical_relocation_differences": [],
            },
            "policies": {"strict_rows": "all_normalized_rows"},
        }

        compact = residual._sanitize_focus_artifact(focus, self.root)

        strict_target = compact["channels"]["strict"]["target"]
        self.assertEqual(strict_target["rows_kind"], "diff_context")
        self.assertEqual(
            [row["index"] for row in strict_target["rows"]],
            [2498, 2499, 2500, 2501, 2502],
        )
        self.assertNotIn(
            "physical_relocations", compact["physical_relocations"]["target"]
        )
        self.assertEqual(
            compact["physical_relocations"]["target"][
                "physical_relocation_payload_sha256"
            ],
            residual._json_sha(physical_rows),
        )
        self.assertLess(len(residual._canonical(compact)), residual.MAX_FOCUS_BYTES)

    def test_publish_rollback_retries_after_replace_and_unlink_failures(self) -> None:
        first = self.root / "build" / "first.evidence"
        second = self.root / "build" / "second.evidence"
        replace_calls = 0
        unlink_calls = 0
        original_replace = residual.os.replace
        original_unlink = Path.unlink

        def replace(source: object, destination: object) -> None:
            nonlocal replace_calls
            replace_calls += 1
            if replace_calls == 2:
                raise OSError("replace sentinel")
            original_replace(source, destination)

        def unlink(path: Path, *, missing_ok: bool = False) -> None:
            nonlocal unlink_calls
            if path == first:
                unlink_calls += 1
                if unlink_calls == 1:
                    raise OSError("unlink sentinel")
            original_unlink(path, missing_ok=missing_ok)

        with patch.object(residual.os, "replace", side_effect=replace), \
             patch.object(Path, "unlink", autospec=True, side_effect=unlink):
            with self.assertRaisesRegex(
                residual.ResidualEvidenceError,
                "replace sentinel.*rollback incomplete",
            ) as raised:
                residual._publish_bundle([(first, b"first"), (second, b"second")])

        self.assertIn(
            "evidence rollback remains required for: first.evidence",
            getattr(raised.exception, "__notes__", []),
        )
        self.assertEqual(replace_calls, 2)
        self.assertEqual(unlink_calls, 2)
        self.assertFalse(first.exists())
        self.assertFalse(second.exists())
        self.assertEqual(list((self.root / "build").glob("*.tmp")), [])

    def test_bounded_runner_terminates_a_timed_out_process(self) -> None:
        with self.assertRaisesRegex(residual.ResidualEvidenceError, "hung command timed out"):
            residual._run_bounded(
                [sys.executable, "-c", "import time; time.sleep(30)"],
                cwd=self.root,
                label="hung command",
                timeout=0.05,
            )

    def test_configure_timeout_uses_the_process_deadline(self) -> None:
        configured = self.root / "configured"
        (configured / "orig" / "GP6E01").mkdir(parents=True)
        retail_source = self.root / "retail-source"
        retail_source.mkdir()
        toolchain = {
            "orig": {"path_object": retail_source},
            "binutils": {"path_object": self.root},
            "compilers": {"path_object": self.root},
            "dtk": {"path_object": self.root},
            "sjiswrap": {"path_object": self.root},
        }

        def timeout_run(
            command: list[str], *, cwd: Path, label: str, timeout: float
        ) -> str:
            self.assertEqual(label, "detached worktree configure")
            self.assertEqual(cwd, configured)
            self.assertEqual(timeout, 0.25)
            raise residual.ResidualEvidenceError(
                "detached worktree configure timed out after 0.250s"
            )

        with patch.object(residual, "_run_bounded", side_effect=timeout_run), \
             patch.object(residual.bundle, "_remove_staged_retail"):
            with self.assertRaisesRegex(
                residual.ResidualEvidenceError, "detached worktree configure timed out"
            ):
                residual._ensure_configured_bounded(
                    configured, toolchain, self.tool, 0.25
                )

    def test_objdiff_timeout_uses_the_process_deadline(self) -> None:
        target = self.root / "build" / "target.o"
        candidate = self.root / "build" / "candidate.o"
        output = self.root / "build" / "strict.json"
        target.write_bytes(b"target")
        candidate.write_bytes(b"candidate")

        def timeout_run(
            command: list[str], *, cwd: Path, label: str, timeout: float
        ) -> str:
            self.assertEqual(label, "objdiff strict")
            self.assertEqual(cwd, self.root)
            self.assertEqual(timeout, 0.125)
            raise residual.ResidualEvidenceError(
                "objdiff strict timed out after 0.125s"
            )

        with patch.object(residual, "_run_bounded", side_effect=timeout_run):
            with self.assertRaisesRegex(
                residual.ResidualEvidenceError, "objdiff strict timed out"
            ):
                residual._run_objdiff_bounded(
                    self.tool, target, candidate, output,
                    data=False, root=self.root, timeout=0.125,
                )
        self.assertFalse(output.exists())

    def test_process_timeout_fails_before_publication_and_cleans_up(self) -> None:
        timeout = residual.ResidualEvidenceError(
            "current base object build timed out after 0.050s; process tree terminated"
        )
        with self.assertRaisesRegex(
            residual.ResidualEvidenceError, "current base object build timed out"
        ):
            self._run_materializer(compile_error=timeout)
        self.assertFalse(self.output.exists())
        self.assertFalse(
            self.output.with_name(self.output.name + ".focus.json").exists()
        )
        self.assertFalse(
            self.output.with_name(self.output.name + ".physical.json").exists()
        )

    def test_focus_row_fidelity_gate_rejects_forged_focus_row(self) -> None:
        focus = {
            "schema": "focus_symbol_report/v1",
            "function": FUNCTION,
            "channels": {
                "strict": {
                    "target": {"rows": [{
                        "index": 0,
                        "diff_kind": "DIFF_ARG_MISMATCH",
                        "instruction": {"address": "0x101", "formatted": "li r3,0"},
                    }]},
                    "candidate": {"rows": [{
                        "index": 0,
                        "diff_kind": "DIFF_ARG_MISMATCH",
                        "instruction": {"address": "0x100", "formatted": "li r4,0"},
                    }]},
                },
                "data": {
                    "target": {"rows": [{
                        "index": 0,
                        "diff_kind": "DIFF_ARG_MISMATCH",
                        "instruction": {"address": "0x101", "formatted": "li r3,0"},
                    }]},
                    "candidate": {"rows": [{
                        "index": 0,
                        "diff_kind": "DIFF_ARG_MISMATCH",
                        "instruction": {"address": "0x100", "formatted": "li r4,0"},
                    }]},
                },
            },
            "physical_relocations": {
                "physical_relocation_differences": [{"row": 0}],
            },
        }
        with self.assertRaisesRegex(
            residual.ResidualEvidenceError, "focus residual rows do not match"
        ):
            self._run_materializer(focus_override=focus)
        self.assertFalse(self.output.exists())

    def test_cleanup_failure_withdraws_materialized_evidence(self) -> None:
        with self.assertRaisesRegex(
            residual.ResidualEvidenceError, "evidence is not admissible"
        ):
            self._run_materializer(cleanup_error="cleanup sentinel")
        self.assertFalse(self.output.exists())
        self.assertFalse(
            self.output.with_name(self.output.name + ".focus.json").exists()
        )

    def test_primary_failure_survives_cleanup_failure(self) -> None:
        with self.assertRaisesRegex(
            residual.ResidualEvidenceError, "proof adapter failed"
        ) as raised:
            self._run_materializer(
                proof_error=True, cleanup_error="cleanup sentinel"
            )
        self.assertIn("cleanup sentinel", " ".join(raised.exception.__notes__))

    def test_source_hash_drift_fails_before_compile_and_writes_nothing(self) -> None:
        with patch.object(residual, "_git", return_value="commit"), \
             patch.object(residual.bundle, "_run") as compile_run:
            with self.assertRaisesRegex(residual.ResidualEvidenceError, "source SHA-256 drifted"):
                residual.materialize_current_residual(
                    root=self.root,
                    base_commit=BASE_COMMIT,
                    owner=OWNER,
                    unit=UNIT,
                    function=FUNCTION,
                    source=self.source,
                    source_sha256="f" * 64,
                    target_sha256=self.target_sha,
                    toolchain_key=TOOLCHAIN,
                    start_line=1,
                    end_line=2,
                    output=self.output,
                    manifest_path=self.root / "manifest.json",
                )
            compile_run.assert_not_called()
        self.assertFalse(self.output.exists())

    def test_exact_base_is_not_published_as_residual(self) -> None:
        with self.assertRaisesRegex(residual.ResidualEvidenceError, "no residual"):
            self._run_materializer(
                report=_report(changed=False), physical_exact=True
            )
        self.assertFalse(self.output.exists())

    def test_stale_output_is_rejected_before_compile(self) -> None:
        self.output.write_text("stale", encoding="ascii")
        with patch.object(residual, "_git", return_value="commit"), \
             patch.object(residual.bundle, "_run") as compile_run:
            with self.assertRaisesRegex(residual.ResidualEvidenceError, "stale evidence"):
                residual.materialize_current_residual(
                    root=self.root,
                    base_commit=BASE_COMMIT,
                    owner=OWNER,
                    unit=UNIT,
                    function=FUNCTION,
                    source=self.source,
                    source_sha256=self.source_sha,
                    target_sha256=self.target_sha,
                    toolchain_key=TOOLCHAIN,
                    start_line=1,
                    end_line=2,
                    output=self.output,
                    manifest_path=self.root / "manifest.json",
                )
            compile_run.assert_not_called()


if __name__ == "__main__":
    unittest.main()
