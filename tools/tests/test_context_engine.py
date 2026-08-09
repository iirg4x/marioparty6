import tempfile
import unittest
import json
from pathlib import Path
from unittest.mock import patch

from tools.agent_queue import QueueError
from tools.context_engine import (
    build_context,
    collect_rejected_probe_history,
    render_rejected_probe_history,
    select_context_knowledge,
)
from tools.recovery_knowledge import KnowledgeMatch


def card(identifier: str, title: str, category: str, change: str) -> dict:
    return {
        "id": identifier,
        "title": title,
        "classification": "confirmed_rule",
        "category": category,
        "compiler": "GC/1.3.2",
        "confidence": "confirmed",
        "summary": title,
        "conditions": "test locally",
        "rule": f"probe {category}",
        "source_condition": {"change": change, "requires": []},
        "emitted_effect": {
            "possible_changes": [category],
            "known_signatures": [],
        },
        "safe_actions": ["run a bounded probe"],
        "counterexamples": [],
        "evidence": [],
    }


class ContextEngineTests(unittest.TestCase):
    def test_symptom_filter_and_budget(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            owner = {"id": "owner", "source": "src/a.c", "compiler": "GC/1.3.2"}
            data = {
                "root": root,
                "project": {"knowledge_card_limit": 5},
                "owners": [owner],
                "patterns": [],
            }
            matches = [
                KnowledgeMatch(
                    card("header", "Header visibility", "header_visibility", "add header"),
                    35,
                    "compiler-wide",
                    ("owner uses compiler",),
                ),
                KnowledgeMatch(
                    card("loop", "Loop shape", "loop", "change loop"),
                    35,
                    "compiler-wide",
                    ("owner uses compiler",),
                ),
            ]
            with patch(
                "tools.context_engine.select_knowledge_cards",
                return_value=matches,
            ):
                selected = select_context_knowledge(
                    data,
                    owner,
                    stable_identity="x:0x1",
                    symptoms=["header visibility"],
                    limit=5,
                )
            self.assertEqual([item.card["id"] for item in selected], ["header"])

            base = """# Recovery context pack

## Recovery contract

- contract

## Owner state

- binary: exact

## Target function

- Stable identity: `x:0x1`

```c
int fn(void) { return 1; }
```

## Accepted evidence

- evidence

## Authenticated constraints

- none

## Acceptance criteria

- no regression
"""
            with (
                patch(
                    "tools.context_engine.resolve_context_target",
                    return_value=(owner, "x:0x1"),
                ),
                patch(
                    "tools.context_engine.select_knowledge_cards",
                    return_value=matches,
                ),
                patch(
                    "tools.context_engine.base_context_pack",
                    return_value=base,
                ),
                patch(
                    "tools.context_engine.card_freshness",
                    return_value={
                        "effective_status": "active",
                        "reason": "inputs unchanged",
                    },
                ),
            ):
                text = build_context(
                    data,
                    "function",
                    "fn",
                    owner_id="owner",
                    budget=1200,
                    symptoms=["header"],
                )
            self.assertIn("Relevant recovered knowledge", text)
            self.assertIn("Header visibility", text)
            self.assertNotIn("Loop shape", text)
            self.assertIn("Acceptance criteria", text)
            self.assertLessEqual(len(text), 4800)

    def test_rejected_history_normalizes_aliases_and_filters_target(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            history_dir = root / "build" / "board-autonomy"
            history_dir.mkdir(parents=True)
            (history_dir / "blocked.json").write_text(
                json.dumps(
                    {
                        "schema_version": 2,
                        "exhausted_recent_probes": [
                            {
                                "owner": "main:board/math#old-task",
                                "function": "MathFn",
                                "probe": "blocked-probe",
                                "result": "target-only `scheduling` row\nwith control text",
                            },
                            {
                                "owner": "main:board/math",
                                "function": "OtherFn",
                                "result": "must not leak",
                            },
                            {
                                "owner": "main:board/player",
                                "function": "MathFn",
                                "result": "foreign owner",
                            },
                        ],
                        "owner_checkpoint": {
                            "math": {
                                "status": "blocked",
                                "reason": "owner-level checkpoint",
                            }
                        },
                    }
                ),
                encoding="utf-8",
            )
            (history_dir / "batch-history.json").write_text(
                json.dumps(
                    {
                        "schema_version": 2,
                        "probes": {
                            "src/board/math.c|MathFn|input-a": {
                                "owner": "src/board/math.c",
                                "symbol": "MathFn",
                                "status": "rejected",
                                "probe_key": "input-a",
                                "input_key": "input-a",
                                "reason": "first source",
                            },
                            "main/board/math|MathFn|input-b": {
                                "owner": "main/board/math",
                                "symbol": "MathFn",
                                "status": "rejected",
                                "probe_key": "input-b",
                                "input_key": "input-b",
                                "reason": "second source",
                            },
                            "main:board/math|OtherFn|foreign": {
                                "owner": "main:board/math",
                                "symbol": "OtherFn",
                                "status": "rejected",
                                "reason": "wrong symbol",
                            },
                        },
                        "batches": [
                            {
                                "id": "legacy-batch",
                                "rejected": [
                                    {
                                        "owner": "board/math",
                                        "function": "MathFn",
                                        "input_key": "legacy-input",
                                        "reason": "legacy rejected record",
                                    }
                                ],
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )
            queue_path = root / "queue.json"
            queue = {
                "tasks": [
                    {
                        "owner": "main:board/math",
                        "target": "MathFn",
                        "status": "released",
                        "note": "terminal queue note; source reverted",
                    },
                    {
                        "owner": "main:board/math",
                        "target": "OtherFn",
                        "status": "done",
                        "note": "wrong target note",
                    },
                    {
                        "owner": "main:board/math",
                        "target": "MathFn",
                        "status": "done",
                        "note": "successful completion",
                    },
                    {
                        "owner": "main:board/math",
                        "target": "MathFn",
                        "status": "done",
                        "note": "done after reverted probe; must remain excluded",
                    },
                ]
            }
            with (
                patch("tools.context_engine.queue_path", return_value=queue_path),
                patch("tools.context_engine.read_queue", return_value=queue),
            ):
                records = collect_rejected_probe_history(
                    root,
                    {"id": "main:board/math", "source": "src/board/math.c"},
                    target_symbols=["MathFn"],
                )
                rendered = render_rejected_probe_history(
                    root,
                    {"id": "main:board/math", "source": "src/board/math.c"},
                    target_symbols=["MathFn"],
                )

            self.assertGreaterEqual(len(records), 5)
            self.assertLessEqual(len(records), 12)
            self.assertIn("input-a", rendered)
            self.assertIn("input-b", rendered)
            self.assertIn("legacy-input", rendered)
            self.assertIn("terminal queue note", rendered)
            self.assertNotIn("must not leak", rendered)
            self.assertNotIn("foreign owner", rendered)
            self.assertNotIn("wrong symbol", rendered)
            self.assertNotIn("wrong target note", rendered)
            self.assertNotIn("successful completion", rendered)
            self.assertNotIn("done after reverted probe", rendered)
            self.assertIn("target-only 'scheduling' row with control text", rendered)
            self.assertNotIn("`scheduling`", rendered)
            self.assertLess(rendered.index("DO NOT REPEAT"), rendered.index("MathFn"))
            self.assertIn("rtk python tools/agent.py probe lookup", rendered)
            self.assertIn("input_key", rendered)

    def test_rejected_history_malformed_ledgers_and_queue_are_best_effort(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            history_dir = root / "build" / "board-autonomy"
            history_dir.mkdir(parents=True)
            (history_dir / "blocked.json").write_text("{not json", encoding="utf-8")
            (history_dir / "batch-history.json").write_text("[]", encoding="utf-8")
            with (
                patch("tools.context_engine.queue_path", side_effect=QueueError("bad queue")),
                patch("tools.context_engine.read_queue", side_effect=QueueError("bad queue")),
            ):
                rendered = render_rejected_probe_history(
                    root,
                    {"id": "main:board/math", "source": "src/board/math.c"},
                    target_symbols=["MathFn"],
                )
            self.assertIn("DO NOT REPEAT", rendered)
            self.assertNotIn("bad queue", rendered)
            self.assertNotIn("Traceback", rendered)

    def test_rejected_history_reads_canonical_blocked_and_batch_statuses(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            history_dir = root / "build" / "board-autonomy"
            history_dir.mkdir(parents=True)
            (history_dir / "blocked.json").write_text(
                json.dumps(
                    {
                        "schema_version": 2,
                        "blocked": [
                            {
                                "owner": "main:board/mgcall",
                                "function": "MgRouletteExec",
                                "reason": "canonical top-level blocker",
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )
            (history_dir / "batch-history.json").write_text(
                json.dumps(
                    {
                        "schema_version": 2,
                        "probes": [
                            {
                                "owner": "main:board/mgcall",
                                "symbol": "MgRouletteExec",
                                "status": "rejected-neutral",
                                "input_key": "mg-neutral",
                                "reason": "canonical neutral rejection",
                            },
                            {
                                "owner": "main:board/mgcall",
                                "symbol": "MgRouletteExec",
                                "status": "rejected-regression",
                                "input_key": "mg-regression",
                                "reason": "canonical regression rejection",
                            },
                        ],
                    }
                ),
                encoding="utf-8",
            )
            queue_path = root / "queue.json"
            queue = {"tasks": []}
            with (
                patch("tools.context_engine.queue_path", return_value=queue_path),
                patch("tools.context_engine.read_queue", return_value=queue),
            ):
                records = collect_rejected_probe_history(
                    root,
                    {"id": "main:board/mgcall", "source": "src/board/mgcall.c"},
                    target_symbols=["MgRouletteExec"],
                )
                rendered = render_rejected_probe_history(
                    root,
                    {"id": "main:board/mgcall", "source": "src/board/mgcall.c"},
                    target_symbols=["MgRouletteExec"],
                )

            self.assertEqual(len(records), 3)
            self.assertEqual(
                [record["status"] for record in records],
                ["", "rejected-neutral", "rejected-regression"],
            )
            self.assertIn("canonical top-level blocker", rendered)
            self.assertIn("canonical neutral rejection", rendered)
            self.assertIn("canonical regression rejection", rendered)

    def test_rejected_history_limit_zero_returns_empty(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            history_dir = root / "build" / "board-autonomy"
            history_dir.mkdir(parents=True)
            (history_dir / "blocked.json").write_text(
                json.dumps(
                    {
                        "blocked": [
                            {
                                "owner": "main:board/math",
                                "function": "MathFn",
                                "reason": "must be omitted at zero limit",
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )
            with patch(
                "tools.context_engine.queue_path",
                return_value=root / "queue.json",
            ):
                records = collect_rejected_probe_history(
                    root,
                    {"id": "main:board/math", "source": "src/board/math.c"},
                    target_symbols=["MathFn"],
                    limit=0,
                )
            self.assertEqual(records, [])

    def test_rejected_history_skips_malformed_queue_task_status(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            queue = {
                "tasks": [
                    {
                        "owner": "main:board/math",
                        "status": [],
                        "note": "malformed status must not break context",
                    }
                ]
            }
            with (
                patch("tools.context_engine.queue_path", return_value=root / "queue.json"),
                patch("tools.context_engine.read_queue", return_value=queue),
            ):
                rendered = render_rejected_probe_history(
                    root,
                    {"id": "main:board/math", "source": "src/board/math.c"},
                    target_symbols=["MathFn"],
                )
            self.assertIn("DO NOT REPEAT", rendered)
            self.assertNotIn("malformed status", rendered)

    def test_queue_task_label_and_owner_target_are_normalized_for_function_context(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            queue = {
                "tasks": [
                    {
                        "owner": "main:board/mgcall:roulette-declaration-order-v13",
                        "source": "src/board/mgcall.c",
                        "target": "main/board/mgcall",
                        "status": "released",
                        "note": "Rejected declaration-order probe; regressed MgRouletteExec.",
                    },
                    {
                        "owner": "main:board/mgcall:roulette-declaration-order-v13",
                        "status": "released",
                        "note": "Reclaiming ownership only; worktree unchanged.",
                    },
                    {
                        "owner": "main:board/mgcall#roulette-exec-v24",
                        "source": "src/board/mgcall.c",
                        "target": "MgRouletteExec",
                        "status": "cancelled",
                        "note": "Neutral pointer-form probe; source reverted.",
                    },
                ]
            }
            with (
                patch("tools.context_engine.queue_path", return_value=root / "queue.json"),
                patch("tools.context_engine.read_queue", return_value=queue),
            ):
                rendered = render_rejected_probe_history(
                    root,
                    {"id": "main:board/mgcall", "source": "src/board/mgcall.c"},
                    target_symbols=["MgRouletteExec"],
                )
            self.assertIn("Rejected declaration-order probe", rendered)
            self.assertIn("Neutral pointer-form probe", rendered)
            self.assertNotIn("Reclaiming ownership only", rendered)
            self.assertNotIn("main/board/mgcall", rendered)

    def test_queue_function_target_lists_filter_by_symbol(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            queue = {
                "tasks": [
                    {
                        "owner": "main:board/snpc",
                        "source": "src/board/snpc.c",
                        "target": "OtherSnpcHook+OtherSnpcExec",
                        "status": "released",
                        "note": "Rejected other SNpc functions; regressed.",
                    },
                    {
                        "owner": "main:board/snpc",
                        "source": "src/board/snpc.c",
                        "target": "SNpcMoveExec,SNpcKoopaFireHook",
                        "status": "released",
                        "note": "Rejected SNpcMoveExec probe; reverted.",
                    },
                ]
            }
            with (
                patch("tools.context_engine.queue_path", return_value=root / "queue.json"),
                patch("tools.context_engine.read_queue", return_value=queue),
            ):
                rendered = render_rejected_probe_history(
                    root,
                    {"id": "main:board/snpc", "source": "src/board/snpc.c"},
                    target_symbols=["SNpcMoveExec"],
                )
            self.assertIn("SNpcMoveExec probe", rendered)
            self.assertNotIn("other SNpc functions", rendered)

    def test_rejected_history_is_mandatory_after_owner_state_under_budget(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            owner = {"id": "owner", "source": "src/a.c", "compiler": "GC/1.3.2"}
            data = {
                "root": root,
                "project": {"knowledge_card_limit": 1},
                "owners": [owner],
                "patterns": [],
            }
            base = """# Recovery context pack

## Recovery contract

- contract

## Owner state

- binary: exact

## Target function

- Stable identity: `x:0x1`

```c
int fn(void) { return 1; }
```

## Acceptance criteria

- no regression
"""
            with (
                patch("tools.context_engine.resolve_context_target", return_value=(owner, "x:0x1")),
                patch("tools.context_engine.select_knowledge_cards", return_value=[]),
                patch("tools.context_engine.base_context_pack", return_value=base),
                patch("tools.context_engine.build_catalog", side_effect=OSError("not available")),
                patch("tools.context_engine.queue_path", side_effect=QueueError("no queue")),
            ):
                text = build_context(
                    data,
                    "function",
                    "fn",
                    owner_id="owner",
                    budget=300,
                )
            owner_index = text.index("## Owner state")
            history_index = text.index("## Durable rejected-probe/blocker history")
            knowledge_index = text.index("## Relevant recovered knowledge")
            self.assertLess(owner_index, history_index)
            self.assertLess(history_index, knowledge_index)
            guard_index = text.index("DO NOT REPEAT")
            self.assertGreater(guard_index, history_index)
            self.assertLess(guard_index, text.index("## Relevant recovered knowledge"))
            self.assertIn("rtk python tools/agent.py probe lookup", text)


if __name__ == "__main__":
    unittest.main()
