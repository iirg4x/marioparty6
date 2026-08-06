from __future__ import annotations

import tempfile
import subprocess
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

from tools import recovery_pass as module


def row(kind: str, formatted: str) -> dict:
    return {"diff_kind": kind, "instruction": {"formatted": formatted}}


def symbol(name: str, size: int, target: int | None, match: float | None, rows=None) -> dict:
    value = {
        "name": name,
        "size": str(size),
        "kind": "SYMBOL_FUNCTION",
        "instructions": list(rows or []),
    }
    if target is not None:
        value["target_symbol"] = target
    if match is not None:
        value["match_percent"] = match
    return value


class RecoveryPassTests(unittest.TestCase):
    def test_serialized_build_lock_releases_for_the_next_process(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            lock = Path(directory) / "retail-build.lock"
            with module.serialized_build_lock(lock, 0.1):
                self.assertTrue(lock.is_file())
            with module.serialized_build_lock(lock, 0.1):
                self.assertTrue(lock.is_file())

    def test_serialized_build_lock_reports_bounded_contention(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            lock = Path(directory) / "retail-build.lock"
            with patch.object(module, "_lock_file_nonblocking", return_value=False):
                with self.assertRaisesRegex(ValueError, "retail build lock remained busy"):
                    with module.serialized_build_lock(lock, 0.0):
                        self.fail("busy lock must not enter the build section")

    def test_serialized_build_lock_excludes_another_process(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            lock = Path(directory) / "retail-build.lock"
            script = (
                "import sys,time; from pathlib import Path; "
                "from tools.recovery_pass import serialized_build_lock; "
                "ctx=serialized_build_lock(Path(sys.argv[1]),5); ctx.__enter__(); "
                "print('locked',flush=True); time.sleep(1); ctx.__exit__(None,None,None)"
            )
            child = subprocess.Popen(
                [sys.executable, "-c", script, str(lock)],
                cwd=module.DEFAULT_ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            try:
                assert child.stdout is not None
                self.assertEqual(child.stdout.readline().strip(), "locked")
                with self.assertRaisesRegex(ValueError, "retail build lock remained busy"):
                    with module.serialized_build_lock(lock, 0.0):
                        self.fail("second process must not enter the build section")
                self.assertEqual(child.wait(timeout=5), 0, child.stderr.read() if child.stderr else "")
                with module.serialized_build_lock(lock, 0.1):
                    pass
            finally:
                if child.poll() is None:
                    child.terminate()
                    child.wait(timeout=5)
                if child.stdout is not None:
                    child.stdout.close()
                if child.stderr is not None:
                    child.stderr.close()

    def test_objdiff_uses_the_selected_repo_as_its_working_directory(self) -> None:
        root = Path("C:/selected/repo")
        executable = root / "build/tools/objdiff-cli.exe"
        output = root / "build/report.json"
        unit = {"target_path": "orig/owner.o", "base_path": "build/owner.o"}

        with patch.object(module.subprocess, "run") as run:
            run.return_value.returncode = 0
            module.objdiff(executable, root, unit, output, data_value=True)

        command = run.call_args.args[0]
        self.assertEqual(run.call_args.kwargs["cwd"], root)
        self.assertEqual(command[command.index("-1") + 1], str(root / "orig/owner.o"))
        self.assertEqual(command[command.index("-2") + 1], str(root / "build/owner.o"))
        self.assertIn("functionRelocDiffs=data_value", command)

    def test_cache_key_reuses_unchanged_source_objects_and_reports(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "src" / "board" / "owner.c"
            target = root / "orig" / "owner.o"
            base = root / "build" / "owner.o"
            strict = root / "strict.json"
            value = root / "value.json"
            for path, content in (
                (source, b"void owner(void) {}\n"),
                (target, b"target"),
                (base, b"base"),
                (strict, b"{}\n"),
                (value, b"{}\n"),
            ):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(content)
            unit = {"name": "main/board/owner", "target_path": str(target), "base_path": str(base)}
            arguments = (root, unit, source, strict, value, None, None, None, root / "objdiff", [])
            first = module.recovery_cache_key(*arguments)
            self.assertEqual(first, module.recovery_cache_key(*arguments))
            source.write_text("void owner(void) { return; }\n", encoding="utf-8")
            self.assertNotEqual(first["key"], module.recovery_cache_key(*arguments)["key"])
            source.write_bytes(b"void owner(void) {}\n")
            self.assertEqual(first, module.recovery_cache_key(*arguments))
            strict.write_text('{"changed": true}\n', encoding="utf-8")
            self.assertNotEqual(first["key"], module.recovery_cache_key(*arguments)["key"])

    def test_captrap_commutative_fingerprints(self) -> None:
        report = {
            "left": {
                "symbols": [
                    symbol(
                        "mbev_CapBoble",
                        2656,
                        0,
                        99.96988,
                        [
                            row("DIFF_ARG_MISMATCH", "fmuls f0, f0, f23"),
                            row("DIFF_ARG_MISMATCH", "fmuls f0, f0, f19"),
                        ],
                    ),
                    symbol(
                        "ev_CapMasuNumGet",
                        72,
                        1,
                        99.44444,
                        [row("DIFF_ARG_MISMATCH", "add r3, r3, r31")],
                    ),
                ]
            },
            "right": {
                "symbols": [
                    symbol(
                        "mbev_CapBoble",
                        2656,
                        0,
                        99.96988,
                        [
                            row("DIFF_ARG_MISMATCH", "fmuls f0, f23, f0"),
                            row("DIFF_ARG_MISMATCH", "fmuls f0, f19, f0"),
                        ],
                    ),
                    symbol(
                        "ev_CapMasuNumGet",
                        72,
                        1,
                        99.44444,
                        [row("DIFF_ARG_MISMATCH", "add r3, r31, r3")],
                    ),
                ]
            },
        }
        boble, masu = report["left"]["symbols"]
        self.assertEqual(
            module.commutative_swap_kind(module.paired_changed(report, boble)),
            "floating_commutative_swap",
        )
        self.assertEqual(
            module.commutative_swap_kind(module.paired_changed(report, masu)),
            "integer_commutative_swap",
        )

    def test_capmove_six_function_gain_and_order_restoration(self) -> None:
        sizes = [1356, 1356, 1356, 1320, 1368, 112]
        names = ["Kinoko", "SKinoko", "PKinoko", "MKinoko", "NKinoko", "EffCreate"]
        current_left = [symbol(name, size, index, 100.0) for index, (name, size) in enumerate(zip(names, sizes))]
        current_right = [symbol(name, size, index, 100.0) for index, (name, size) in enumerate(zip(names, sizes))]
        baseline_left = [symbol(name, size, index, 98.0, [row("DIFF_REPLACE", "nop")]) for index, (name, size) in enumerate(zip(names, sizes))]
        baseline_right_names = [names[4], *names[:4], names[5]]
        baseline_right = [symbol(name, sizes[names.index(name)], index, 98.0, [row("DIFF_REPLACE", "nop")]) for index, name in enumerate(baseline_right_names)]
        current = {"left": {"symbols": current_left}, "right": {"symbols": current_right}}
        baseline = {"left": {"symbols": baseline_left}, "right": {"symbols": baseline_right}}
        gain = module.delta(current, baseline)
        assert gain is not None
        self.assertEqual(gain["newly_exact"], sorted(names))
        self.assertEqual(gain["newly_exact_bytes"], 6868)
        self.assertEqual(gain["regressed_exact"], [])
        target_order = names
        self.assertEqual(module.order_diagnostics(target_order, names)["inversions"], 0)
        self.assertEqual(module.order_diagnostics(target_order, baseline_right_names)["inversions"], 4)

    def test_capthrow_live_shape_is_stable_without_regression(self) -> None:
        exact_names = [f"Exact{index}" for index in range(8)]
        partial_names = ["mbev_CapThrowman", "mbev_CapPakkun"]
        missing_names = [f"Missing{index}" for index in range(9)]
        missing_sizes = [1000] * 8 + [22128]
        right_names = [*exact_names, *partial_names]
        right = [symbol(name, 64, index, 100.0) for index, name in enumerate(right_names)]
        left = [symbol(name, 64, index, 100.0) for index, name in enumerate(exact_names)]
        left += [
            symbol(
                name,
                512,
                8 + index,
                91.0 - index,
                [row("DIFF_ARG_MISMATCH", f"mr r{3 + index}, r31")],
            )
            for index, name in enumerate(partial_names)
        ]
        left += [
            symbol(name, size, None, None)
            for name, size in zip(missing_names, missing_sizes)
        ]
        current = {"left": {"symbols": left}, "right": {"symbols": right}}
        state = module.report_state(current)
        self.assertEqual((len(state["exact"]), state["total"]), (8, 19))
        missing = [item for item in left if item.get("target_symbol") is None]
        self.assertEqual(len(missing), 9)
        self.assertEqual(sum(int(item["size"]) for item in missing), 30128)
        target_common = right_names
        source_common = [*reversed(right_names[:5]), *right_names[5:]]
        self.assertEqual(module.order_diagnostics(target_common, source_common)["inversions"], 10)
        stable = module.delta(current, current)
        assert stable is not None
        self.assertEqual(stable["regressed_exact"], [])
        self.assertEqual(stable["newly_exact"], [])

    def test_shared_cause_clusters_plan_dice_wipe_and_star_like_residuals(self) -> None:
        def item(
            name: str,
            size: int,
            category: str,
            diagnostics: list[str],
            diff_kinds: dict[str, int],
            relocation: str,
            calls: list[str],
            identifiers: list[str],
        ) -> dict:
            return {
                "function": name,
                "target_bytes": size,
                "category": category,
                "diagnostics": diagnostics,
                "diff_kinds": diff_kinds,
                "relocation_identity_pattern": relocation,
                "target_source_size_delta": 0,
                "target_call_skeleton": calls,
                "source_local_identifiers": {"types": identifiers, "work_identifiers": []},
                "strict_exact": False,
            }

        clusters = module.plan_shared_cause_clusters(
            [
                item("DiceRollA", 300, "local_order_cycle", ["local_declaration_or_first_use_cycle"], {"DIFF_ARG_MISMATCH": 4}, "paired_instruction_residual", [], ["DICE_WORK"]),
                item("DiceRollB", 300, "local_order_cycle", ["local_declaration_or_first_use_cycle"], {"DIFF_ARG_MISMATCH": 4}, "paired_instruction_residual", [], ["DICE_WORK"]),
                item("DiceRollC", 300, "local_order_cycle", ["local_declaration_or_first_use_cycle"], {"DIFF_ARG_MISMATCH": 4}, "paired_instruction_residual", [], ["DICE_WORK"]),
                item("WipeGrid", 700, "relocation_identity_only", [], {"DIFF_ARG_MISMATCH": 3}, "data_value_exact_only", ["WipeCheck"], []),
                item("WipePaper", 600, "relocation_identity_only", [], {"DIFF_ARG_MISMATCH": 3}, "data_value_exact_only", ["WipeCheck"], []),
                item("StarRise", 128, "branch_destination_only", ["branch_destination_only"], {"DIFF_ARG_MISMATCH": 1}, "paired_instruction_residual", ["espScaleSet"], []),
                item("StarFall", 128, "branch_destination_only", ["branch_destination_only"], {"DIFF_ARG_MISMATCH": 1}, "paired_instruction_residual", ["espScaleSet"], []),
                item("StarFlash", 128, "branch_destination_only", ["branch_destination_only"], {"DIFF_ARG_MISMATCH": 1}, "paired_instruction_residual", ["espScaleSet"], []),
            ]
        )
        by_cause = {cluster["cause"]: cluster for cluster in clusters}
        self.assertEqual(set(by_cause), {"local_declaration_or_first_use_cycle", "relocation_identity_only", "branch_destination_only"})
        self.assertTrue(by_cause["local_declaration_or_first_use_cycle"]["actionable"])
        self.assertTrue(by_cause["branch_destination_only"]["actionable"])
        self.assertFalse(by_cause["relocation_identity_only"]["actionable"])
        self.assertTrue(by_cause["relocation_identity_only"]["owner_audit_only"])
        self.assertEqual(by_cause["relocation_identity_only"]["expected_exact_bytes_per_compiler_probe"], 0)
        self.assertEqual(by_cause["local_declaration_or_first_use_cycle"]["shared_evidence"], {"source_local_identifiers": ["DICE_WORK"]})
        self.assertEqual(by_cause["branch_destination_only"]["function_count"], 3)

    def test_shared_cause_clusters_reject_heterogeneous_and_isolated_residuals(self) -> None:
        shared_shape = {
            "category": "local_order_cycle",
            "diagnostics": ["local_declaration_or_first_use_cycle"],
            "diff_kinds": {"DIFF_ARG_MISMATCH": 4},
            "relocation_identity_pattern": "paired_instruction_residual",
            "target_source_size_delta": 0,
            "strict_exact": False,
        }
        ranked = [
            {
                **shared_shape,
                "function": "DiceLike",
                "target_bytes": 900,
                "target_call_skeleton": ["mbRandMod"],
                "source_local_identifiers": {"types": ["DICE_WORK"], "work_identifiers": ["diceWork"]},
            },
            {
                **shared_shape,
                "function": "UnrelatedLike",
                "target_bytes": 900,
                "target_call_skeleton": ["WipeCheck"],
                "source_local_identifiers": {"types": ["WIPE_WORK"], "work_identifiers": ["wipeWork"]},
            },
            {
                **shared_shape,
                "function": "IsolatedLike",
                "target_bytes": 1200,
                "target_call_skeleton": ["espKill"],
                "source_local_identifiers": {"types": ["STAR_WORK"], "work_identifiers": ["starWork"]},
            },
        ]
        self.assertEqual(module.plan_shared_cause_clusters(ranked), [])

    def test_ready_missing_definition_family_requires_named_closed_dependencies(self) -> None:
        def item(name: str, size: int, calls: list[str]) -> dict:
            return {
                "function": name,
                "target_bytes": size,
                "category": "missing_definition",
                "target_call_skeleton": calls,
                "strict_exact": False,
            }

        clusters = module.plan_shared_cause_clusters(
            [
                item("MgCallVsEffNumGet", 124, []),
                item("MgCallVsEffCreate", 300, []),
                item("mbMgCallVsEffCreate", 328, ["MgCallVsEffPosSet"]),
                item("MgCallVsEffOMExec", 516, ["MgCallVsEffNumGet"]),
                item("MgCallVsEffPosSet", 612, []),
            ]
        )
        self.assertEqual(len(clusters), 1)
        family = clusters[0]
        self.assertTrue(family["implementation_ready"])
        self.assertEqual(family["implementation_target_bytes"], 1880)
        self.assertEqual(family["potential_exact_bytes"], 1880)
        self.assertNotIn("expected_exact_bytes_per_compiler_probe", family)
        self.assertEqual(family["expected_compiler_probes"], 0)
        self.assertEqual(
            family["shared_evidence"]["internal_target_call_edges"],
            [
                {"caller": "MgCallVsEffOMExec", "callee": "MgCallVsEffNumGet"},
                {"caller": "mbMgCallVsEffCreate", "callee": "MgCallVsEffPosSet"},
            ],
        )

    def test_ready_missing_definition_family_rejects_no_edge_or_open_dependency(self) -> None:
        def item(name: str, size: int, calls: list[str]) -> dict:
            return {
                "function": name,
                "target_bytes": size,
                "category": "missing_definition",
                "target_call_skeleton": calls,
                "strict_exact": False,
            }

        self.assertEqual(
            module.plan_shared_cause_clusters(
                [item("MgCallVsEffCreate", 600, []), item("MgCallVsEffPosSet", 600, [])]
            ),
            [],
        )

    def test_preferred_packet_meets_bulk_bounds_without_splitting_dependencies(self) -> None:
        def item(rank: int, name: str, size: int, calls: list[str]) -> dict:
            return {
                "function": name,
                "target_rank": rank,
                "target_bytes": size,
                "category": "missing_definition",
                "target_call_skeleton": calls,
            }

        ranked = [
            item(0, "SingleMicListenerCreate", 84, []),
            item(1, "SingleMicListener", 104, []),
            item(2, "SingleMicCreate", 116, ["SingleMicListenerCreate"]),
            item(3, "SingleMgSaveInit", 132, []),
            item(4, "SingleEffMgStop", 320, ["SingleEffOMExec"]),
            item(5, "SingleLast5", 388, []),
            item(6, "SingleMasuOrderInit", 392, []),
            item(7, "SingleEffMgMasuHook", 480, []),
            item(8, "SingleEffInit", 576, []),
            item(9, "SingleEffMgCapsuleHook", 720, []),
            item(10, "SingleEffMgExplodeHook", 732, []),
            item(11, "SingleEffMgHook", 968, []),
            item(12, "SingleEffOMExec", 1412, ["SingleEffMgStop"]),
        ]

        packet = module.preferred_missing_definition_packet(ranked)

        assert packet is not None
        self.assertTrue(packet["ready"])
        self.assertTrue(packet["dependency_closed"])
        self.assertEqual(packet["function_count"], 12)
        self.assertEqual(packet["target_bytes"], 6036)
        self.assertNotIn("SingleLast5", packet["functions"])
        self.assertLess(
            packet["functions"].index("SingleMicListenerCreate"),
            packet["functions"].index("SingleMicCreate"),
        )
        self.assertTrue(
            {"SingleEffMgStop", "SingleEffOMExec"}.issubset(packet["functions"])
        )

    def test_preferred_packet_never_splits_an_oversized_missing_component(self) -> None:
        ranked = [
            {
                "function": name,
                "target_rank": rank,
                "target_bytes": size,
                "category": "missing_definition",
                "target_call_skeleton": calls,
            }
            for rank, (name, size, calls) in enumerate(
                [
                    ("LargeA", 4000, ["LargeB"]),
                    ("LargeB", 4000, []),
                    *[(f"Small{index}", 300, []) for index in range(12)],
                ]
            )
        ]

        packet = module.preferred_missing_definition_packet(ranked)

        assert packet is not None
        self.assertTrue(packet["ready"])
        self.assertTrue({"LargeA", "LargeB"}.isdisjoint(packet["functions"]))
        self.assertEqual(packet["function_count"], 12)
        self.assertEqual(packet["target_bytes"], 3600)

    def test_worker_dispatch_blocks_new_work_on_regression_or_unbuilt_source(self) -> None:
        regression = module.worker_dispatch(
            {
                "strict_delta": {"regressed_exact": ["ExactLost"]},
                "data_value_delta": {"regressed_exact": []},
                "ranked_functions": [
                    {"function": "ExactLost", "target_bytes": 240},
                    {"function": "NewBody", "target_bytes": 900, "source_pending_build": True},
                ],
                "preferred_implementation_packet": {
                    "ready": True,
                    "functions": ["NewBody"],
                    "function_count": 1,
                    "target_bytes": 900,
                },
            }
        )
        self.assertEqual(regression["mode"], "regression_reconciliation")
        self.assertEqual(regression["functions"], ["ExactLost"])

        pending = module.worker_dispatch(
            {
                "strict_delta": {"regressed_exact": []},
                "data_value_delta": {"regressed_exact": []},
                "ranked_functions": [
                    {"function": "NewBody", "target_rank": 4, "target_bytes": 900, "source_pending_build": True},
                ],
                "preferred_implementation_packet": {
                    "ready": True,
                    "functions": ["NewBody"],
                    "function_count": 1,
                    "target_bytes": 900,
                },
            }
        )
        self.assertEqual(pending["mode"], "verification_first")
        self.assertEqual(pending["functions"], ["NewBody"])

    def test_worker_dispatch_prefers_closed_packet_and_rotates_exhausted_owner(self) -> None:
        implementation = module.worker_dispatch(
            {
                "ranked_functions": [],
                "preferred_implementation_packet": {
                    "ready": True,
                    "functions": ["HelperA", "CallerB"],
                    "function_count": 2,
                    "target_bytes": 1024,
                },
            }
        )
        self.assertEqual(implementation["mode"], "implementation")
        self.assertEqual(implementation["functions"], ["HelperA", "CallerB"])

        rotate = module.worker_dispatch(
            {
                "ranked_functions": [],
                "preferred_implementation_packet": None,
                "shared_cause_clusters": [
                    {
                        "actionable": False,
                        "owner_audit_only": True,
                        "functions": ["Blocked"],
                    }
                ],
            }
        )
        self.assertEqual(rotate["mode"], "rotate_owner")
        self.assertFalse(rotate["ready"])

    def test_probe_history_summary_filters_owner_alias_and_packet_functions(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            history = root / module.PROBE_HISTORY
            history.parent.mkdir(parents=True)
            history.write_text(
                """{
  "schema_version": 2,
  "probes": {
    "a": {"owner":"main:board/captrap","symbol":"MetalShock","probe_key":"outer-product","status":"rejected","reason":"worse"},
    "b": {"owner":"main:board/captrap","symbol":"Other","probe_key":"noise","status":"rejected","reason":"irrelevant"},
    "c": {"owner":"main:board/player","symbol":"MetalShock","probe_key":"foreign","status":"rejected","reason":"other owner"}
  }
}
""",
                encoding="utf-8",
            )
            result = module.probe_history_summary(
                root, "main/board/captrap", ["MetalShock"]
            )
        self.assertEqual(result["status"], "available")
        self.assertEqual(
            [(item["symbol"], item["probe_key"]) for item in result["records"]],
            [("MetalShock", "outer-product")],
        )

    def test_worker_packet_is_directly_executable_and_requires_data_pool_proof(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "build" / "packet"
            strict = output / "strict.json"
            value = output / "data-value.json"
            source = root / "src" / "board" / "single.c"
            target = root / "orig" / "single.o"
            candidate = root / "build" / "GP6E01" / "src" / "board" / "single.o"
            for path, content in (
                (source, b"void SingleHelper(void) {}\n"),
                (target, b"target object"),
                (candidate, b"candidate object"),
            ):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(content)
            report = {
                "unit": "main/board/single",
                "source": "src/board/single.c",
                "commit": "a" * 40,
                "summary": {"strict_exact": 30, "functions_total": 58},
                "preferred_implementation_packet": {
                    "ready": True,
                    "functions": ["SingleHelper", "SingleCaller"],
                    "function_count": 2,
                    "target_bytes": 1200,
                },
                "ranked_functions": [
                    {
                        "function": "SingleHelper",
                        "target_rank": 1,
                        "target_bytes": 400,
                        "category": "missing_definition",
                        "strict_match_percent": None,
                        "strict_diff_rows": 0,
                        "diff_kind_shape": {},
                        "target_call_skeleton": [],
                        "diagnostics": [],
                        "safe_actions": ["implement"],
                    },
                    {
                        "function": "SingleCaller",
                        "target_rank": 2,
                        "target_bytes": 800,
                        "category": "missing_definition",
                        "strict_match_percent": None,
                        "strict_diff_rows": 0,
                        "diff_kind_shape": {},
                        "target_call_skeleton": ["SingleHelper"],
                        "diagnostics": [],
                        "safe_actions": ["implement after helper"],
                    },
                ],
                "selected_knowledge_cards": [
                    {
                        "id": "card-one",
                        "freshness": "active",
                        "rule": "keep the call family closed",
                        "counterexamples": ["do not split the caller from its helper"],
                    }
                ],
                "shared_cause_clusters": [],
                "graphify": {"fresh": True},
            }
            report["ranked_functions"].reverse()
            packet = module.build_worker_packet(
                root,
                {
                    "target_path": "orig/single.o",
                    "base_path": "build/GP6E01/src/board/single.o",
                },
                report,
                output,
                strict,
                value,
            )
            duplicate = module.build_worker_packet(
                root,
                {
                    "target_path": "orig/single.o",
                    "base_path": "build/GP6E01/src/board/single.o",
                },
                report,
                output,
                strict,
                value,
            )
            prompt = module.render_worker_prompt(packet)
            source.write_text("void SingleHelper(void) { return; }\n", encoding="utf-8")
            changed = module.build_worker_packet(
                root,
                {
                    "target_path": "orig/single.o",
                    "base_path": "build/GP6E01/src/board/single.o",
                },
                report,
                output,
                strict,
                value,
            )

        self.assertEqual(packet["dispatch"]["mode"], "implementation")
        self.assertEqual(packet["owner"], "main:board/single")
        self.assertEqual(
            [item["function"] for item in packet["function_evidence"]],
            ["SingleHelper", "SingleCaller"],
        )
        self.assertEqual(packet["packet_id"], duplicate["packet_id"])
        self.assertNotEqual(packet["packet_id"], changed["packet_id"])
        self.assertEqual(
            packet["function_evidence"][1]["internal_dependencies"],
            ["SingleHelper"],
        )
        self.assertTrue(
            packet["function_evidence"][0]["probe"]["probe_key"].startswith(
                "implementation/" + packet["packet_id"][:16]
            )
        )
        self.assertEqual(packet["budgets"]["max_probes"]["compiler_reconciliation"], 3)
        self.assertIn("tools.serialized_build", packet["commands"]["build"])
        self.assertIn("tools/blind_recovery.py", packet["commands"]["organicity"])
        self.assertIn("--baseline-strict", packet["commands"]["verify"])
        self.assertIn(".sdata2/.rodata", prompt)
        self.assertIn("do not split the caller", prompt)
        self.assertIn("12-20 functions", prompt)

    def test_source_pending_build_is_not_scheduled_as_missing_implementation(self) -> None:
        def item(name: str, category: str, calls: list[str]) -> dict:
            return {
                "function": name,
                "target_bytes": 600,
                "category": category,
                "target_call_skeleton": calls,
                "strict_exact": False,
            }

        clusters = module.plan_ready_missing_definition_families(
            [
                item("mbev_MgCallVsEffCreate", "missing_definition", ["mbev_MgCallVsEffPosSet"]),
                item("mbev_MgCallVsEffPosSet", "missing_definition", ["mbev_MgCallVsEffCreate"]),
                item("mbev_MgCallVsEffOMExec", "source_pending_build", ["mbev_MgCallVsEffCreate"]),
            ]
        )
        self.assertEqual(len(clusters), 1)
        self.assertEqual(
            clusters[0]["functions"],
            ["mbev_MgCallVsEffCreate", "mbev_MgCallVsEffPosSet"],
        )
        self.assertNotIn("mbev_MgCallVsEffOMExec", clusters[0]["functions"])

    def test_analyze_uses_current_source_definition_guard_for_object_missing(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "src" / "board" / "owner.c"
            source.parent.mkdir(parents=True)
            source.write_text("void Present(void) {}\n", encoding="utf-8")
            strict = {
                "left": {
                    "symbols": [
                        symbol("Present", 120, None, None),
                        symbol("Absent", 160, None, None),
                    ]
                },
                "right": {"symbols": []},
            }
            value = {"left": {"symbols": []}, "right": {"symbols": []}}
            unit = {
                "name": "main/board/owner",
                "target_path": "orig/owner.o",
                "base_path": "build/owner.o",
            }
            with (
                patch.object(module, "paired_single_quarantine", return_value=(None, set(), "missing", None)),
                patch.object(module, "select_cards", return_value=[]),
            ):
                report = module.analyze(
                    root,
                    unit,
                    source,
                    strict,
                    value,
                    None,
                    None,
                    [],
                    [],
                    False,
                    None,
                )

        self.assertEqual(report["summary"]["object_missing"], 2)
        self.assertEqual(report["summary"]["source_pending_build"], 1)
        self.assertEqual(report["summary"]["missing_definitions"], 1)
        ranked = {item["function"]: item for item in report["ranked_functions"]}
        self.assertEqual(ranked["Present"]["category"], "source_pending_build")
        self.assertTrue(ranked["Present"]["object_missing"])
        self.assertTrue(ranked["Present"]["source_definition_present"])
        self.assertFalse(ranked["Present"]["strict_exact"])
        self.assertEqual(len(ranked["Present"]["safe_actions"]), 1)
        self.assertEqual(ranked["Absent"]["category"], "missing_definition")
        self.assertFalse(ranked["Absent"]["source_definition_present"])

    def test_mgcall_report_only_counts_current_source_pending_definitions(self) -> None:
        root = module.DEFAULT_ROOT
        report_dir = root / "build" / "board-autonomy" / "mgcall-roulette-number-family"
        strict_path = report_dir / "strict.json"
        value_path = report_dir / "data-value.json"
        source = root / "src" / "board" / "mgcall.c"
        if not strict_path.is_file() or not value_path.is_file() or not source.is_file():
            self.skipTest("mgcall report-only fixture is unavailable")
        config = module.read_json(root / "objdiff.json")
        unit = next(item for item in config["units"] if item.get("name") == "main/board/mgcall")
        report = module.analyze(
            root,
            unit,
            source,
            module.read_json(strict_path),
            module.read_json(value_path),
            None,
            None,
            [],
            [],
            False,
            None,
        )
        summary = report["summary"]
        self.assertGreater(summary["source_pending_build"], 0)
        self.assertEqual(
            summary["source_pending_build"] + summary["missing_definitions"],
            summary["object_missing"],
        )
        ranked = {item["function"]: item for item in report["ranked_functions"]}
        for item in ranked.values():
            if not item["object_missing"]:
                continue
            expected = (
                "source_pending_build"
                if item["source_definition_present"]
                else "missing_definition"
            )
            self.assertEqual(item["category"], expected)
        pending_names = {
            name for name, item in ranked.items() if item["category"] == "source_pending_build"
        }
        for cluster in report["shared_cause_clusters"]:
            if cluster.get("implementation_ready"):
                self.assertTrue(pending_names.isdisjoint(cluster["functions"]))
        markdown = module.render_markdown(report)
        self.assertIn(
            f"Source-present/object-missing (pending build): **{summary['source_pending_build']} functions",
            markdown,
        )

    def test_ready_missing_definition_family_accepts_exact_homologous_siblings(self) -> None:
        def item(name: str, size: int, calls: list[str]) -> dict:
            return {
                "function": name,
                "target_bytes": size,
                "category": "missing_definition",
                "target_call_skeleton": calls,
                "strict_exact": False,
            }

        clusters = module.plan_shared_cause_clusters(
            [
                item("mbev_MgCallDonkey", 308, ["HuPrcSleep", "ModelPosGet", "ModelPosSet"]),
                item("mbev_MgCallKoopa", 308, ["HuPrcSleep", "ModelPosGet", "ModelPosSet"]),
            ]
        )
        self.assertEqual(len(clusters), 1)
        family = clusters[0]
        self.assertEqual(family["cause"], "homologous_missing_definition_siblings")
        self.assertEqual(family["functions"], ["mbev_MgCallDonkey", "mbev_MgCallKoopa"])
        self.assertEqual(family["implementation_target_bytes"], 616)
        self.assertEqual(
            family["shared_evidence"],
            {
                "normalized_prefix": ["mg", "call"],
                "target_byte_size": 308,
                "ordered_nonruntime_target_calls": ["ModelPosGet", "ModelPosSet"],
            },
        )

    def test_ready_missing_definition_family_rejects_homologous_sibling_near_misses(self) -> None:
        def item(name: str, size: int, calls: list[str]) -> dict:
            return {
                "function": name,
                "target_bytes": size,
                "category": "missing_definition",
                "target_call_skeleton": calls,
                "strict_exact": False,
            }

        self.assertEqual(
            module.plan_shared_cause_clusters(
                [
                    item("mbev_MgCallDonkey", 308, ["ModelPosGet", "ModelPosSet"]),
                    item("mbev_MgCallKoopa", 308, ["ModelPosSet", "ModelPosGet"]),
                ]
            ),
            [],
        )
        self.assertEqual(
            module.plan_shared_cause_clusters(
                [
                    item("mbev_MgCallDonkey", 308, ["ModelPosGet", "ModelPosSet"]),
                    item("mbev_MgCallKoopa", 304, ["ModelPosGet", "ModelPosSet"]),
                ]
            ),
            [],
        )
        self.assertEqual(
            module.plan_shared_cause_clusters(
                [
                    item("MgCallVsEffCreate", 600, ["OtherMissingHelper"]),
                    item("MgCallVsEffPosSet", 600, ["MgCallVsEffCreate"]),
                    item("OtherMissingHelper", 100, []),
                ]
            ),
            [],
        )

    def test_paired_single_quarantine_is_explicit_and_excluded_from_clusters(self) -> None:
        card = {
            "id": module.PAIRED_SINGLE_QUARANTINE_CARD,
            "applicability": {"stable_ids": ["mbObjFadeCreate", "mbObjFadeTexRotSet"]},
            "safe_actions": ["reopen only with authenticated source evidence"],
        }
        with (
            patch.object(module, "load", return_value={"patterns": [card]}),
            patch.object(module, "card_freshness", return_value={"effective_status": "active", "reason": "validated"}),
        ):
            selected, stable_ids, status, reason = module.paired_single_quarantine(Path("C:/repo"))
        self.assertEqual(selected, card)
        self.assertEqual(stable_ids, {"mbObjFadeCreate", "mbObjFadeTexRotSet"})
        self.assertEqual((status, reason), ("active", "validated"))
        with (
            patch.object(module, "load", return_value={"patterns": [card]}),
            patch.object(module, "card_freshness", return_value={"effective_status": "stale", "reason": "watched input changed"}),
        ):
            _, _, status, reason = module.paired_single_quarantine(Path("C:/repo"))
        self.assertEqual((status, reason), ("stale", "watched input changed"))
        self.assertEqual(
            module.plan_shared_cause_clusters(
                [
                    {
                        "function": "mbObjFadeCreate",
                        "target_bytes": 332,
                        "category": "paired_residual",
                        "diagnostics": [],
                        "diff_kinds": {"DIFF_ARG_MISMATCH": 46},
                        "relocation_identity_pattern": "paired_instruction_residual",
                        "target_source_size_delta": 0,
                        "target_call_skeleton": ["mbObjPosSetV"] * 6,
                        "source_local_identifiers": {"types": [], "work_identifiers": []},
                        "strict_exact": False,
                        "quarantined_by_card": module.PAIRED_SINGLE_QUARANTINE_CARD,
                    },
                    {
                        "function": "mbObjFadeTexRotSet",
                        "target_bytes": 200,
                        "category": "paired_residual",
                        "diagnostics": [],
                        "diff_kinds": {"DIFF_ARG_MISMATCH": 42},
                        "relocation_identity_pattern": "paired_instruction_residual",
                        "target_source_size_delta": 0,
                        "target_call_skeleton": ["mbObjPosSetV"] * 6,
                        "source_local_identifiers": {"types": [], "work_identifiers": []},
                        "strict_exact": False,
                        "quarantined_by_card": module.PAIRED_SINGLE_QUARANTINE_CARD,
                    },
                ]
            ),
            [],
        )

    def test_relocation_clusters_require_shared_owner_not_wipe_calls_or_capevent_types(self) -> None:
        def item(
            name: str,
            size: int,
            calls: list[str],
            types: list[str],
            target_owner: str,
            source_owner: str,
        ) -> dict:
            return {
                "function": name,
                "target_bytes": size,
                "category": "relocation_identity_only",
                "diagnostics": [],
                "diff_kinds": {"DIFF_ARG_MISMATCH": 3},
                "relocation_identity_pattern": "data_value_exact_only",
                "target_source_size_delta": 0,
                "target_call_skeleton": calls,
                "source_local_identifiers": {"types": types, "work_identifiers": []},
                "relocation_owner_evidence": [
                    {
                        "target_owner": target_owner,
                        "source_owner": source_owner,
                        "type": "R_PPC_ADDR16_LO",
                    }
                ],
                "strict_exact": False,
            }

        wipe_calls = [
            "C_MTXOrtho", "GXSetProjection", "GXSetViewport", "GXSetScissor",
            "GXSetNumTevStages", "GXSetTevOrder", "GXBegin",
        ]
        wipe = module.plan_shared_cause_clusters(
            [
                item("WipeImageDraw", 1676, wipe_calls, ["WIPE_IMAGE_WORK"], "wipeImageWhite", "imageColorPool"),
                item("WipeGridDraw", 1864, wipe_calls, ["WIPE_GRID_WORK"], "wipeGridWhite", "gridColorPool"),
                item("WipePaperDraw", 2184, wipe_calls, ["WIPE_PAPER_WORK"], "wipePaperWhite", "paperColorPool"),
            ]
        )
        self.assertEqual(len(wipe), 1)
        self.assertFalse(wipe[0]["actionable"])
        self.assertTrue(wipe[0]["owner_audit_only"])
        self.assertEqual(wipe[0]["actionability_reason"], "missing_shared_relocation_owner")

        capevent = module.plan_shared_cause_clusters(
            [
                item("mbev_CapEffGlowCoinAdd", 1500, [], ["HuVecF"], "capGlowColor", "glowPool"),
                item("mbev_CapHermiteGetV", 592, [], ["HuVecF"], "capHermiteColor", "hermitePool"),
            ]
        )
        self.assertEqual(len(capevent), 1)
        self.assertFalse(capevent[0]["actionable"])
        self.assertTrue(capevent[0]["owner_audit_only"])
        self.assertEqual(capevent[0]["shared_evidence"], {})

    def test_relocation_cluster_is_actionable_with_exact_shared_owner_identity(self) -> None:
        common = {
            "category": "relocation_identity_only",
            "diagnostics": [],
            "diff_kinds": {"DIFF_ARG_MISMATCH": 3},
            "relocation_identity_pattern": "data_value_exact_only",
            "target_source_size_delta": 0,
            "target_call_skeleton": [],
            "source_local_identifiers": {"types": [], "work_identifiers": []},
            "relocation_owner_evidence": [
                {
                    "target_owner": "sharedTargetColor",
                    "source_owner": "sharedSourceColor",
                    "type": "R_PPC_ADDR16_LO",
                }
            ],
            "strict_exact": False,
        }
        clusters = module.plan_shared_cause_clusters(
            [
                {**common, "function": "OwnerA", "target_bytes": 600},
                {**common, "function": "OwnerB", "target_bytes": 600},
            ]
        )
        self.assertEqual(len(clusters), 1)
        self.assertTrue(clusters[0]["actionable"])
        self.assertFalse(clusters[0]["owner_audit_only"])
        self.assertEqual(
            clusters[0]["shared_evidence"]["relocation_owner_pairs"],
            [{"target_owner": "sharedTargetColor", "source_owner": "sharedSourceColor", "type": "R_PPC_ADDR16_LO"}],
        )

    def test_target_call_cluster_recognizes_live_coin_add_family(self) -> None:
        common_prefix = [
            "_savegpr_17", "abs", "abs", "mbPlayerCoinGet", "mbPlayerCoinGet",
            "mbPlayerCoinGet", "mbPlayerCoinAdd", "mbAudFXPlay", "HuPrcSleep", "abs",
        ]
        display_tail = [
            "mbPlayerCoinAdd", "mbAudFXPlay", "mbPlayerPosGet", "mbCoinDispCreate",
            "HuPrcVSleep", "_restgpr_17",
        ]

        def item(name: str, size: int, diff_kinds: dict[str, int], calls: list[str]) -> dict:
            return {
                "function": name,
                "target_bytes": size,
                "category": "paired_residual",
                "diagnostics": [],
                "diff_kinds": diff_kinds,
                "relocation_identity_pattern": "paired_instruction_residual",
                "target_source_size_delta": -16,
                "target_call_skeleton": calls,
                "source_local_identifiers": {"types": [], "work_identifiers": []},
                "strict_exact": False,
            }

        clusters = module.plan_shared_cause_clusters(
            [
                item(
                    "mbCoinAddProcExec", 524,
                    {"DIFF_ARG_MISMATCH": 40, "DIFF_REPLACE": 3, "DIFF_DELETE": 4},
                    [*common_prefix, *display_tail],
                ),
                item(
                    "mbCoinAddDispExec", 536,
                    {"DIFF_ARG_MISMATCH": 47, "DIFF_REPLACE": 1, "DIFF_DELETE": 4},
                    [*common_prefix, *display_tail],
                ),
                item(
                    "mbCoinAddExec", 304,
                    {"DIFF_ARG_MISMATCH": 37, "DIFF_REPLACE": 1, "DIFF_DELETE": 4},
                    [*common_prefix, "mbAudFXPlay", "_restgpr_17"],
                ),
            ]
        )
        self.assertEqual(len(clusters), 1)
        cluster = clusters[0]
        self.assertEqual(cluster["cause"], "repeated_target_call_skeleton")
        self.assertEqual(cluster["functions"], ["mbCoinAddDispExec", "mbCoinAddExec", "mbCoinAddProcExec"])
        self.assertEqual(cluster["target_bytes"], 1364)
        self.assertTrue(cluster["actionable"])
        self.assertEqual(cluster["knowledge_card_id"], module.TARGET_CALL_CLUSTER_CARD)
        self.assertEqual(
            cluster["shared_evidence"]["ordered_target_call_skeleton"],
            ["mbPlayerCoinGet", "mbPlayerCoinGet", "mbPlayerCoinGet", "mbPlayerCoinAdd", "mbAudFXPlay", "mbAudFXPlay"],
        )

    def test_target_call_cluster_rejects_empty_short_generic_and_heterogeneous_groups(self) -> None:
        def item(name: str, calls: list[str]) -> dict:
            return {
                "function": name,
                "target_bytes": 700,
                "category": "paired_residual",
                "diagnostics": [],
                "diff_kinds": {"DIFF_ARG_MISMATCH": 8},
                "relocation_identity_pattern": "paired_instruction_residual",
                "target_source_size_delta": 0,
                "target_call_skeleton": calls,
                "source_local_identifiers": {"types": [], "work_identifiers": []},
                "strict_exact": False,
            }

        ranked = [
            item("EmptyA", []),
            item("EmptyB", []),
            item("ShortA", ["mbCoinGet", "mbCoinSet", "mbCoinGet", "mbCoinSet", "mbCoinGet"]),
            item("ShortB", ["mbCoinGet", "mbCoinSet", "mbCoinGet", "mbCoinSet", "mbCoinGet"]),
            item("GenericA", ["_savegpr_17", "abs", "HuPrcSleep", "HuPrcVSleep", "memcpy", "_restgpr_17"]),
            item("GenericB", ["_savegpr_17", "abs", "HuPrcSleep", "HuPrcVSleep", "memcpy", "_restgpr_17"]),
            item("CoinFamily", ["mbCoinGet", "mbCoinSet", "mbCoinGet", "mbCoinSet", "mbCoinGet", "mbCoinSet", "mbCoinGet"]),
            item("WipeFamily", ["WipeCheck", "WipeCreate", "WipeCheck", "WipeCreate", "WipeCheck", "WipeCreate", "WipeCheck"]),
        ]
        self.assertEqual(module.plan_shared_cause_clusters(ranked), [])


if __name__ == "__main__":
    unittest.main()
