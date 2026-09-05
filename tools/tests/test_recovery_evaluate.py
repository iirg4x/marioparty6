from __future__ import annotations

import copy
import hashlib
import json
import subprocess
import sys
import tempfile
import threading
import unittest
from pathlib import Path
from unittest import mock

from tools import recovery_evaluate as evaluate
from tools import recovery_frontier as frontier
from tools.tests.test_focus_symbol_report import _report


class RecoveryEvaluateTests(unittest.TestCase):
    """Exercise one bounded evaluation without a compiler or retail inputs."""

    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name) / "repo"
        (self.root / "build/GP6E01/include").mkdir(parents=True)
        (self.root / "include").mkdir()

        self.source = self.root / "source.c"
        self.candidate = self.root / "candidate.c"
        self.target_object = self.root / "target.o"
        self.baseline_object = self.root / "baseline.o"
        self.new_object = self.root / "new.o"
        self.objdiff = self.root / "objdiff.exe"
        self.readelf = self.root / "readelf.exe"
        self.compiler_tool = self.root / "compiler.exe"
        self.command_json = self.root / "compile.json"

        self.source.write_text("int f(void) { return 1; }\n", encoding="utf-8")
        self.candidate.write_text("int f(void) { return 2; }\n", encoding="utf-8")
        self.target_object.write_bytes(b"target object")
        self.baseline_object.write_bytes(b"baseline object")
        self.new_object.write_bytes(b"new object")
        for path in (self.objdiff, self.readelf, self.compiler_tool):
            path.write_bytes(path.name.encode("ascii"))
        self.command_json.write_text(
            json.dumps([sys.executable, "-c", "pass", "{source}", "{object}"]),
            encoding="utf-8",
        )

        self.before = _report(focus_exact=False, sibling_exact=True)
        self.after = _report(focus_exact=True, sibling_exact=True)
        (self.root / "strict.json").write_text(json.dumps(self.before), encoding="utf-8")
        (self.root / "data.json").write_text(json.dumps(self.before), encoding="utf-8")

        # Include a source/object-bound compile receipt so duplicate-source
        # lookup can prove that the current compiler context is the same.
        context = evaluate._command_context(self.root, self.command_json, [Path("compiler.exe")])
        receipt = {
            "schema": "recovery_candidate_compile/v1",
            "source_sha256": evaluate.compiler.digest(self.source),
            "object_sha256": evaluate.compiler.digest(self.baseline_object),
            "context_sha256": hashlib.sha256(frontier.canonical(context)).hexdigest(),
            "command": context["argv_template"],
        }
        self.compile_receipt = self.root / "baseline.compile.json"
        self.compile_receipt.write_text(json.dumps(receipt), encoding="utf-8")

        base = frontier.snapshot(
            root=self.root,
            owner="main:board/snpc",
            source=Path("source.c"),
            target=Path("target.o"),
            candidate=Path("baseline.o"),
            strict=Path("strict.json"),
            data=Path("data.json"),
            toolchain_key="GC/2.6/test",
            compile_receipt=Path("baseline.compile.json"),
        )
        self.index = self.root / "build/index.json"
        frontier.publish(self.root, self.index, base)

        self._inventory_mode = "changed"
        self._function_size = {"FocusFunction": 8, "ProtectedSibling": 4}
        self.report_calls: list[tuple[bool, Path, int]] = []
        self.compile_calls: list[Path] = []
        self.readelf_calls: list[list[str]] = []
        self._report_barrier = threading.Barrier(2)

    def tearDown(self) -> None:
        self.temp.cleanup()

    def _inventory(self, path: Path) -> dict[str, object]:
        path = Path(path)
        resolved = path.resolve()
        if resolved == self.target_object.resolve():
            semantic = "target-semantic"
        elif resolved == self.baseline_object.resolve():
            semantic = "baseline-semantic"
        elif self._inventory_mode == "baseline":
            semantic = "baseline-semantic"
        else:
            semantic = "candidate-semantic"
        return {
            "schema": "recovery_object_inventory/v1",
            "path": str(path),
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "semantic_sha256": semantic,
            "size_bytes": path.stat().st_size,
            "functions": {
                name: {"size": size} for name, size in self._function_size.items()
            },
        }

    @staticmethod
    def _comparison(*, sibling_loss: bool = False, raw_slide: bool = False,
                    identity_loss: bool = False) -> dict[str, object]:
        focus_raw_before = 0 if identity_loss else 1
        focus_raw_after = 1 if identity_loss else 2 if raw_slide else 0
        focus_normalized_before = 0 if identity_loss else 1
        focus_normalized_after = 1 if identity_loss else 0
        return {
            "function_census_equal": True,
            "allocated_nontext_changed": False,
            "functions": {
                "FocusFunction": {
                    "physical_diff_before": focus_raw_before,
                    "physical_diff_after": focus_raw_after,
                    "closed_physical_row_losses": ([{"offset": 0, "type": 10, "candidate": "changed"}]
                                                     if identity_loss else []),
                    "normalized_diff_before": focus_normalized_before,
                    "normalized_diff_after": focus_normalized_after,
                    "closed_normalized_row_losses": ([{"offset": 0, "type": 10, "candidate": "changed"}]
                                                        if identity_loss else []),
                    "closed_normalized_row_loss_count": 1 if identity_loss else 0,
                    "raw_exact_target": not raw_slide and not identity_loss,
                    "candidate_physical_exact": not raw_slide and not identity_loss,
                },
                "ProtectedSibling": {
                    "physical_diff_before": 0,
                    "physical_diff_after": 1 if sibling_loss else 0,
                    "closed_physical_row_losses": ([{"offset": 0, "type": 10, "candidate": "changed"}]
                                                     if sibling_loss else []),
                    "normalized_diff_before": 0,
                    "normalized_diff_after": 1 if sibling_loss else 0,
                    "closed_normalized_row_losses": ([{"offset": 0, "type": 10, "candidate": "changed"}]
                                                        if sibling_loss else []),
                    "closed_normalized_row_loss_count": 1 if sibling_loss else 0,
                    "raw_exact_target": True,
                    "candidate_physical_exact": not sibling_loss,
                },
            },
        }

    def _fake_compile(self, root: Path, candidate: Path, output: Path,
                      command_json: Path, compiler_tools: list[Path], timeout: float) -> dict[str, object]:
        self.compile_calls.append(output)
        self.assertTrue(output.parent.name.startswith(".evaluate-"))
        output.write_bytes(b"fresh candidate object")
        return {
            "schema": "recovery_candidate_compile/v1",
            "source_sha256": evaluate.compiler.digest(candidate),
            "object_sha256": evaluate.compiler.digest(output),
            "context_sha256": "mocked-context",
            "command": [str(command_json)],
        }

    def _fake_report(self, objdiff: Path, target: Path, candidate: Path, output: Path,
                     root: Path, data: bool, timeout: float) -> None:
        self._report_barrier.wait(timeout=3)
        self.report_calls.append((data, output, threading.get_ident()))
        output.write_text(json.dumps(self.after), encoding="utf-8")

    def _fake_readelf(self, argv: list[str], **kwargs: object) -> subprocess.CompletedProcess:
        self.readelf_calls.append(list(argv))
        return subprocess.CompletedProcess(argv, 0, b"", b"")

    def _patch_measurement(self, *, sibling_loss: bool = False):
        self.after = _report(focus_exact=True, sibling_exact=not sibling_loss)
        comparison = self._comparison(sibling_loss=sibling_loss)
        return mock.patch.multiple(
            evaluate.objects,
            inventory=mock.Mock(side_effect=self._inventory),
            compare=mock.Mock(return_value=comparison),
        )

    def _evaluate(self, out_name: str, *, candidate_object: Path | None = None,
                  command_json: Path | None = None) -> dict[str, object]:
        kwargs: dict[str, object] = {
            "root": self.root,
            "index": self.index,
            "candidate": self.candidate,
            "functions": ["FocusFunction"],
            "out": self.root / "build" / out_name,
            "objdiff": self.objdiff,
            "readelf": self.readelf,
        }
        if candidate_object is not None:
            kwargs["candidate_object"] = candidate_object
        else:
            kwargs["command_json"] = command_json or self.command_json
            kwargs["compiler_tools"] = [Path("compiler.exe")]
        return evaluate.evaluate(**kwargs)  # type: ignore[arg-type]

    def test_command_context_and_mocked_compile_bind_placeholders(self) -> None:
        context = evaluate._command_context(self.root, self.command_json, [Path("compiler.exe")])
        self.assertEqual(context["argv_template"][-2:], ["{source}", "{object}"])
        self.assertIn(str(self.compiler_tool.resolve()), context["tools"])

        output = self.root / "build" / "mock-object.o"

        def fake_run(argv: list[str], **kwargs: object) -> subprocess.CompletedProcess:
            self.assertIn(str(output), argv)
            output.write_bytes(b"mocked object")
            return subprocess.CompletedProcess(argv, 0, b"compiler stdout", b"")

        with mock.patch.object(evaluate.bounded_process, "run", side_effect=fake_run) as run:
            receipt = evaluate._compile_candidate(
                self.root, self.candidate, output, self.command_json,
                [Path("compiler.exe")], 5,
            )
        self.assertEqual(run.call_count, 1)
        self.assertEqual(receipt["source_sha256"], evaluate.compiler.digest(self.candidate))
        self.assertEqual(receipt["object_sha256"], evaluate.compiler.digest(output))
        self.assertEqual(receipt["context_sha256"], hashlib.sha256(frontier.canonical(context)).hexdigest())

    def test_invalid_baseline_index_fails_before_output(self) -> None:
        value = json.loads(self.index.read_text(encoding="utf-8"))
        value["owner"] = "tampered"
        self.index.write_text(json.dumps(value), encoding="utf-8")
        output = self.root / "build" / "invalid.json"
        with self.assertRaisesRegex(ValueError, "index digest"):
            self._evaluate("invalid.json", candidate_object=self.baseline_object)
        self.assertFalse(output.exists())

    def test_duplicate_source_short_circuits_with_matching_receipt(self) -> None:
        self.candidate = self.source
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_compile_candidate", side_effect=AssertionError("compiled")), \
                mock.patch.object(evaluate, "_report", side_effect=AssertionError("reported")), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=AssertionError("readelf")):
            result = self._evaluate("duplicate-source.json")
        self.assertEqual(result["status"], "duplicate_source")
        self.assertEqual(result["compiler_runs"], 0)
        self.assertEqual(result["objdiff_runs"], 0)
        self.assertFalse(list((self.root / "build").glob(".evaluate-*")))

    def test_same_source_with_changed_context_is_not_reused(self) -> None:
        changed_command = self.root / "changed-compile.json"
        changed_command.write_text(
            json.dumps([sys.executable, "-c", "pass", "-Dchanged", "{source}", "{object}"]),
            encoding="utf-8",
        )
        self.candidate = self.source
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_compile_candidate", side_effect=self._fake_compile), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("changed-context.json", command_json=changed_command)
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["compiler_runs"], 1)
        self.assertEqual(len(self.compile_calls), 1)

    def test_duplicate_object_skips_proof_and_cleans_private_directory(self) -> None:
        self._inventory_mode = "baseline"
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_report", side_effect=AssertionError("reported")), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=AssertionError("readelf")):
            result = self._evaluate("duplicate-object.json", candidate_object=self.baseline_object)
        self.assertEqual(result["status"], "duplicate_object")
        self.assertEqual(result["compiler_runs"], 0)
        self.assertEqual(result["objdiff_runs"], 0)
        self.assertFalse(result["retention_ready"])
        self.assertFalse(list((self.root / "build").glob(".evaluate-*")))

    def test_exact_command_workflow_runs_parallel_reports_preserves_candidate(self) -> None:
        source_before = self.candidate.read_bytes()
        index_before = self.index.read_bytes()
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_compile_candidate", side_effect=self._fake_compile), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("exact.json")
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["compiler_runs"], 1)
        self.assertEqual(result["objdiff_runs"], 2)
        self.assertTrue(result["retention_ready"])
        self.assertEqual(sorted(data for data, _, _ in self.report_calls), [False, True])
        self.assertEqual(len({thread for _, _, thread in self.report_calls}), 2)
        self.assertEqual(len(self.readelf_calls), 3)
        self.assertEqual(self.candidate.read_bytes(), source_before)
        self.assertEqual(self.index.read_bytes(), index_before)
        self.assertTrue((self.root / "build/exact.candidate.c").is_file())
        self.assertTrue((self.root / "build/exact.candidate.o").is_file())
        self.assertTrue((self.root / "build/exact.compile.json").is_file())
        self.assertFalse(list((self.root / "build").glob(".evaluate-*")))
        self.assertNotIn("artifacts", result)
        self.assertNotIn("retained_private_directory", result)

    def test_same_context_cache_short_circuits_second_measurement(self) -> None:
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            first = self._evaluate("cached-first.json", candidate_object=self.new_object)
            report_count = len(self.report_calls)
            readelf_count = len(self.readelf_calls)
            second = self._evaluate("cached-second.json", candidate_object=self.new_object)
        self.assertEqual(first["status"], "exact")
        self.assertEqual(second["status"], "duplicate_source")
        self.assertEqual(second["compiler_runs"], 0)
        self.assertEqual(second["objdiff_runs"], 0)
        self.assertEqual(len(self.report_calls), report_count)
        self.assertEqual(len(self.readelf_calls), readelf_count)
        self.assertIn("reused_measurement", second)
        self.assertFalse(list((self.root / "build").glob(".evaluate-*")))

    def test_dependency_fingerprint_change_invalidates_cache(self) -> None:
        bindings = [{"parser": "first"}, {"parser": "changed"}]
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_implementation_binding", side_effect=bindings), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            first = self._evaluate("dependency-first.json", candidate_object=self.new_object)
            second = self._evaluate("dependency-changed.json", candidate_object=self.new_object)
        self.assertEqual(first["status"], "exact")
        self.assertEqual(second["status"], "exact")
        self.assertNotEqual(first["context_key"], second["context_key"])
        self.assertEqual(first["implementation"], bindings[0])
        self.assertEqual(second["implementation"], bindings[1])
        self.assertEqual(len(self.report_calls), 4)
        self.assertEqual(len(self.readelf_calls), 6)

    def test_same_source_content_at_different_paths_has_distinct_cache_keys(self) -> None:
        same_a = self.root / "same-a.c"
        same_b = self.root / "same-b.c"
        same_a.write_bytes(self.source.read_bytes())
        same_b.write_bytes(self.source.read_bytes())
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            self.candidate = same_a
            first = self._evaluate("path-a.json", candidate_object=self.new_object)
            self.candidate = same_b
            second = self._evaluate("path-b.json", candidate_object=self.new_object)
        self.assertEqual(first["status"], "exact")
        self.assertEqual(second["status"], "exact")
        self.assertEqual(first["candidate_source"]["sha256"], second["candidate_source"]["sha256"])
        self.assertNotEqual(first["context_key"], second["context_key"])
        self.assertEqual(len(self.report_calls), 4)
        self.assertEqual(len(self.readelf_calls), 6)

    def test_raw_text_slide_with_same_canonical_calls_is_not_rejected(self) -> None:
        comparison = self._comparison(raw_slide=True)
        with mock.patch.multiple(
                evaluate.objects,
                inventory=mock.Mock(side_effect=self._inventory),
                compare=mock.Mock(return_value=comparison),
        ), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("normalized-slide.json", candidate_object=self.new_object)
        self.assertEqual(result["status"], "improved")
        self.assertEqual(result["regressions"], [])
        self.assertEqual(result["object_comparison"]["functions"]["FocusFunction"]["physical_diff_after"], 2)
        self.assertEqual(result["object_comparison"]["functions"]["FocusFunction"]["normalized_diff_after"], 0)

    def test_legacy_comparison_falls_back_to_raw_relocation_checks(self) -> None:
        comparison = self._comparison(raw_slide=True)
        for row in comparison["functions"].values():
            for key in (
                "closed_normalized_row_losses",
                "closed_normalized_row_loss_count",
                "normalized_diff_before",
                "normalized_diff_after",
            ):
                row.pop(key, None)
        classified = evaluate._classify(
            frontier.summarize(self.before, "strict"),
            frontier.summarize(self.before, "data"),
            frontier.summarize(self.after, "strict"),
            frontier.summarize(self.after, "data"),
            comparison,
            ["FocusFunction"],
        )
        self.assertEqual(classified["status"], "rejected")
        self.assertTrue(any("physical" in item for item in classified["regressions"]))

    def test_changed_canonical_call_identity_is_rejected(self) -> None:
        comparison = self._comparison(identity_loss=True)
        with mock.patch.multiple(
                evaluate.objects,
                inventory=mock.Mock(side_effect=self._inventory),
                compare=mock.Mock(return_value=comparison),
        ), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("normalized-identity-loss.json", candidate_object=self.new_object)
        self.assertEqual(result["status"], "rejected")
        self.assertTrue(any("canonical relocation" in item for item in result["regressions"]))
        self.assertEqual(result["object_comparison"]["functions"]["FocusFunction"]["physical_diff_after"], 1)

    def test_cleanup_error_keeps_primary_status_but_clears_readiness(self) -> None:
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_compile_candidate", side_effect=self._fake_compile), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf), \
                mock.patch.object(evaluate, "_cleanup", return_value=["cleanup sentinel"]):
            result = self._evaluate("cleanup-error.json")
        self.assertEqual(result["status"], "exact")
        self.assertFalse(result["retention_ready"])
        self.assertEqual(result["cleanup_errors"], ["cleanup sentinel"])
        self.assertTrue((self.root / "build/cleanup-error.json").is_file())

    def test_positive_fresh_compile_is_preserved_without_baseline_receipt(self) -> None:
        base = frontier.snapshot(
            root=self.root,
            owner="main:board/snpc",
            source=Path("source.c"),
            target=Path("target.o"),
            candidate=Path("baseline.o"),
            strict=Path("strict.json"),
            data=Path("data.json"),
            toolchain_key="GC/2.6/test",
        )
        frontier.publish(self.root, self.index, base)
        with self._patch_measurement(), \
                mock.patch.object(evaluate, "_compile_candidate", side_effect=self._fake_compile), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("unbound-baseline.json")
        self.assertEqual(result["status"], "exact")
        self.assertFalse(result["retention_ready"])
        self.assertTrue((self.root / "build/unbound-baseline.candidate.c").is_file())
        self.assertTrue((self.root / "build/unbound-baseline.candidate.o").is_file())
        self.assertTrue((self.root / "build/unbound-baseline.compile.json").is_file())
        self.assertIn("measured_candidate", result)

    def test_uint64_string_function_sizes_are_accepted_from_objdiff(self) -> None:
        # Objdiff emits uint64 sizes as JSON strings. Keep the fixture's
        # instruction coverage physically valid while exercising conversion.
        sizes = {"FocusFunction": 8, "ProtectedSibling": 4}
        before = copy.deepcopy(self.before)
        after = copy.deepcopy(self.after)
        for document in (before, after):
            for side in ("left", "right"):
                for symbol in document[side]["symbols"]:
                    if symbol.get("kind") == "SYMBOL_FUNCTION":
                        symbol["size"] = str(sizes[symbol["name"]])
        self._function_size = sizes
        (self.root / "strict.json").write_text(json.dumps(before), encoding="utf-8")
        (self.root / "data.json").write_text(json.dumps(before), encoding="utf-8")
        base = frontier.snapshot(
            root=self.root,
            owner="main:board/snpc",
            source=Path("source.c"),
            target=Path("target.o"),
            candidate=Path("baseline.o"),
            strict=Path("strict.json"),
            data=Path("data.json"),
            toolchain_key="GC/2.6/test",
            compile_receipt=Path("baseline.compile.json"),
        )
        frontier.publish(self.root, self.index, base)
        self.before = before
        self.after = after
        with mock.patch.multiple(
                evaluate.objects,
                inventory=mock.Mock(side_effect=self._inventory),
                compare=mock.Mock(return_value=self._comparison()),
        ), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("uint64-size.json", candidate_object=self.new_object)
        self.assertEqual(result["status"], "exact")
        self.assertEqual(result["objdiff_runs"], 2)

    def test_malformed_instruction_coverage_is_rejected(self) -> None:
        malformed = copy.deepcopy(self.after)
        for side in ("left", "right"):
            for symbol in malformed[side]["symbols"]:
                if symbol.get("name") == "FocusFunction":
                    symbol["size"] = "12"
        self._function_size = {"FocusFunction": 12, "ProtectedSibling": 4}
        self.after = malformed
        with mock.patch.multiple(
                evaluate.objects,
                inventory=mock.Mock(side_effect=self._inventory),
                compare=mock.Mock(return_value=self._comparison()),
        ), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("malformed-coverage.json", candidate_object=self.new_object)
        self.assertEqual(result["status"], "failed")
        self.assertIn("instruction coverage", result["reason"])
        self.assertEqual(result["objdiff_runs"], 2)
        self.assertFalse(list((self.root / "build").glob(".evaluate-*")))

    def test_protected_sibling_regression_rejects_candidate(self) -> None:
        with self._patch_measurement(sibling_loss=True), \
                mock.patch.object(evaluate, "_report", side_effect=self._fake_report), \
                mock.patch.object(evaluate.bounded_process, "run", side_effect=self._fake_readelf):
            result = self._evaluate("rejected.json", candidate_object=self.new_object)
        self.assertEqual(result["status"], "rejected")
        self.assertFalse(result["retention_ready"])
        self.assertTrue(any("ProtectedSibling" in item for item in result["regressions"]))
        self.assertEqual(result["objdiff_runs"], 2)
        self.assertFalse(list((self.root / "build").glob(".evaluate-*")))

    def test_compile_failure_returns_compact_result_and_cleans_private_directory(self) -> None:
        source_before = self.candidate.read_bytes()
        with mock.patch.object(evaluate, "_compile_candidate", side_effect=ValueError("sentinel compile failure")):
            result = self._evaluate("failed.json")
        self.assertEqual(result["status"], "failed")
        self.assertIn("sentinel compile failure", result["reason"])
        self.assertEqual(result["stage"], "compile")
        self.assertEqual(result["objdiff_runs"], 0)
        self.assertEqual(result["cleanup_errors"], [])
        self.assertEqual(self.candidate.read_bytes(), source_before)
        self.assertFalse(list((self.root / "build").glob(".evaluate-*")))


if __name__ == "__main__":
    unittest.main()
