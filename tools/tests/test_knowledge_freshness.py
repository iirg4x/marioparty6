import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.knowledge_freshness import card_freshness, validate_freshness


def run(cwd: Path, *args: str) -> str:
    process = subprocess.run(
        args,
        cwd=cwd,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return process.stdout.strip()


class KnowledgeFreshnessTests(unittest.TestCase):
    def test_unrelated_card_change_does_not_stale_shared_catalog_watch(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            patterns = root / "patterns.json"
            patterns.write_text(
                json.dumps(
                    {
                        "schema_version": 2,
                        "patterns": [
                            {"id": "card", "rule": "keep"},
                            {"id": "other", "rule": "first"},
                        ],
                    }
                ),
                encoding="utf-8",
            )
            run(root, "git", "add", "patterns.json")
            run(root, "git", "commit", "-qm", "base")
            commit = run(root, "git", "rev-parse", "HEAD")
            freshness = root / "freshness.json"
            freshness.write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "cards": {
                            "card": {
                                "status": "active",
                                "validated_commit": commit,
                                "validated_at": "2026-08-03",
                                "watch_paths": ["patterns.json"],
                                "supersedes": [],
                                "superseded_by": None,
                            }
                        },
                    }
                ),
                encoding="utf-8",
            )
            data = {
                "root": root,
                "project": {
                    "files": {
                        "compiler_patterns": "patterns.json",
                        "knowledge_freshness": "freshness.json",
                    }
                },
                "patterns": [{"id": "card"}],
            }

            value = json.loads(patterns.read_text(encoding="utf-8"))
            value["patterns"][1]["rule"] = "second"
            patterns.write_text(json.dumps(value), encoding="utf-8")
            run(root, "git", "add", "patterns.json")
            run(root, "git", "commit", "-qm", "change other card")
            self.assertEqual(
                card_freshness(data, "card")["effective_status"],
                "active",
            )

            value["patterns"][0]["rule"] = "changed"
            patterns.write_text(json.dumps(value), encoding="utf-8")
            run(root, "git", "add", "patterns.json")
            run(root, "git", "commit", "-qm", "change watched card")
            result = card_freshness(data, "card")
            self.assertEqual(result["effective_status"], "stale")
            self.assertIn("patterns.json", result["changed_paths"])

    def test_watched_change_marks_card_stale(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            (root / "docs").mkdir()
            evidence = root / "docs/evidence.md"
            evidence.write_text("first\n", encoding="utf-8")
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "base")
            commit = run(root, "git", "rev-parse", "HEAD")
            freshness = root / "freshness.json"
            freshness.write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "cards": {
                            "card": {
                                "status": "active",
                                "validated_commit": commit,
                                "validated_at": "2026-07-25",
                                "watch_paths": ["docs/evidence.md"],
                                "supersedes": [],
                                "superseded_by": None,
                            }
                        },
                    }
                ),
                encoding="utf-8",
            )
            data = {
                "root": root,
                "project": {"files": {"knowledge_freshness": "freshness.json"}},
                "patterns": [{"id": "card"}],
            }
            self.assertEqual(validate_freshness(data), [])
            self.assertEqual(card_freshness(data, "card")["effective_status"], "active")

            evidence.write_text("second\n", encoding="utf-8")
            dirty_result = card_freshness(data, "card")
            self.assertEqual(dirty_result["effective_status"], "stale")
            self.assertIn("docs/evidence.md", dirty_result["changed_paths"])
            run(root, "git", "add", "docs/evidence.md")
            run(root, "git", "commit", "-qm", "change")
            result = card_freshness(data, "card")
            self.assertEqual(result["effective_status"], "stale")
            self.assertIn("docs/evidence.md", result["changed_paths"])


if __name__ == "__main__":
    unittest.main()
