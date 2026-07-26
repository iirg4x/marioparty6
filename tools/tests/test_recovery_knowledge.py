import json
import sqlite3
import tempfile
import unittest
from contextlib import closing
from pathlib import Path

from tools.recovery_core import load, query_index
from tools.recovery_knowledge import (
    build_recovery_index,
    context_pack,
    knowledge_audit,
    resolve_context_target,
    select_knowledge_cards,
    validate_knowledge,
)
from tools.tests.test_recovery_workflow import RecoveryWorkflowTests


class RecoveryKnowledgeTests(unittest.TestCase):
    def fixture(self, root: Path) -> None:
        RecoveryWorkflowTests().fixture(root)
        owner_path = root / "config/recovery/owners/a.json"
        owner = json.loads(owner_path.read_text(encoding="utf-8"))
        owner["compiler"] = "GC/1.3.2"
        owner_path.write_text(json.dumps(owner), encoding="utf-8")
        (root / "docs/native_matching_wave1.md").write_text(
            "# Wave 1\n", encoding="utf-8"
        )
        (root / "docs/native_matching_wave2.md").write_text(
            "# Wave 2\n", encoding="utf-8"
        )
        card = {
            "id": "gc132-signed-contract",
            "title": "Recover narrow signed contracts before widening",
            "classification": "confirmed_rule",
            "category": "type_contracts",
            "compiler": "GC/1.3.2",
            "confidence": "confirmed",
            "summary": "Widening an s16 contract can change emitted extension and lifetime behavior.",
            "conditions": "Use when target and consumer widths show a signed 16-bit contract.",
            "source_condition": {
                "change": "Widen an s16 argument or return to int.",
                "requires": ["target width evidence", "consumer review"],
            },
            "emitted_effect": {
                "possible_changes": ["sign extension", "temporary lifetime"],
                "known_signatures": ["caller size changes after widening"],
            },
            "rule": "Choose narrow signed types from target and consumer evidence.",
            "safe_actions": [
                "inspect all callers",
                "compare extension instructions",
            ],
            "applicability": {
                "compiler_wide": True,
                "project_wide": False,
                "owners": [],
                "stable_ids": ["test:0x20"],
                "modules": [],
                "owner_tags": [],
            },
            "examples": ["test:0x20"],
            "counterexamples": [],
            "related_exceptions": [],
            "evidence": ["docs/native_matching_wave1.md"],
        }
        patterns_path = root / "config/recovery/compiler_patterns.json"
        patterns_path.write_text(
            json.dumps({"schema_version": 2, "patterns": [card]}),
            encoding="utf-8",
        )

    def test_exact_target_card_is_injected_before_source(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            data = load(root, validate=False)
            self.assertEqual(validate_knowledge(data), [])
            packet = context_pack(
                data,
                "function",
                "fn_1_20",
                owner_id="REL:test:a",
                budget=3000,
            )
            self.assertIn("Relevant recovered knowledge", packet)
            self.assertIn("Recover narrow signed contracts", packet)
            self.assertIn("Choose narrow signed types", packet)
            self.assertLess(
                packet.index("Relevant recovered knowledge"),
                packet.index("Target function"),
            )

    def test_zero_limit_disables_knowledge_section(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            data = load(root, validate=False)
            packet = context_pack(
                data,
                "function",
                "fn_1_20",
                owner_id="REL:test:a",
                budget=3000,
                knowledge_limit=0,
            )
            self.assertNotIn("Relevant recovered knowledge", packet)

    def test_counterexample_is_selected_as_warning(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            patterns_path = root / "config/recovery/compiler_patterns.json"
            document = json.loads(patterns_path.read_text(encoding="utf-8"))
            document["patterns"][0]["applicability"]["stable_ids"] = []
            document["patterns"][0]["examples"] = []
            document["patterns"][0]["counterexamples"] = ["REL:test:a"]
            patterns_path.write_text(json.dumps(document), encoding="utf-8")
            data = load(root, validate=False)
            owner, stable_identity = resolve_context_target(
                data,
                "function",
                "fn_1_20",
                "REL:test:a",
            )
            matches = select_knowledge_cards(
                data,
                owner,
                stable_identity=stable_identity,
            )
            self.assertEqual(matches[0].relevance, "counterexample")
            self.assertTrue(matches[0].counterexample)

    def test_owner_constraint_requires_explicit_scope(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            patterns_path = root / "config/recovery/compiler_patterns.json"
            document = json.loads(patterns_path.read_text(encoding="utf-8"))
            card = document["patterns"][0]
            card["classification"] = "owner_constraint"
            card["applicability"]["stable_ids"] = []
            card["applicability"]["owners"] = []
            card["applicability"]["compiler_wide"] = False
            patterns_path.write_text(json.dumps(document), encoding="utf-8")
            errors = validate_knowledge(load(root, validate=False))
            self.assertTrue(
                any("owner_constraint needs owners or stable_ids" in error for error in errors)
            )

    def test_index_search_contains_rule_and_safe_actions(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            data = load(root, validate=False)
            database = root / "build/context/recovery.sqlite"
            build_recovery_index(data, database)
            rows = query_index(database, "inspect all callers")
            self.assertEqual(rows[0]["kind"], "pattern")
            with closing(sqlite3.connect(database)) as connection:
                text = connection.execute(
                    "SELECT text FROM search WHERE kind='pattern'"
                ).fetchone()[0]
            self.assertIn("Choose narrow signed types", text)
            self.assertIn("inspect all callers", text)

    def test_audit_identifies_undistilled_wave_reports(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            audit = knowledge_audit(load(root, validate=False))
            self.assertEqual(audit["waves_referenced_by_cards"], 1)
            self.assertEqual(
                audit["waves_without_knowledge_card"],
                ["docs/native_matching_wave2.md"],
            )


if __name__ == "__main__":
    unittest.main()
