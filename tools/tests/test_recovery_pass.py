from __future__ import annotations

import tempfile
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


if __name__ == "__main__":
    unittest.main()
