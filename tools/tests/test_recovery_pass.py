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
        self.assertTrue(all(cluster["actionable"] for cluster in clusters))
        self.assertEqual(by_cause["relocation_identity_only"]["expected_exact_bytes_per_compiler_probe"], 1300)
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
