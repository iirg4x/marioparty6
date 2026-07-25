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
            run(root, "git", "add", "docs/evidence.md")
            run(root, "git", "commit", "-qm", "change")
            result = card_freshness(data, "card")
            self.assertEqual(result["effective_status"], "stale")
            self.assertIn("docs/evidence.md", result["changed_paths"])


if __name__ == "__main__":
    unittest.main()
