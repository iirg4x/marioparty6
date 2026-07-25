import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from tools.context_engine import build_context, select_context_knowledge
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


if __name__ == "__main__":
    unittest.main()
