import json
import tempfile
import unittest
from pathlib import Path

from tools.local_evidence import render_summary, summarize_report


class LocalEvidenceTests(unittest.TestCase):
    def test_summary_extracts_exact_function_count(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.json"
            path.write_text(
                json.dumps(
                    {
                        "functions": [
                            {"name": "a", "matchPercent": 100},
                            {"name": "b", "matchPercent": 80},
                        ],
                        "relocations": [],
                    }
                ),
                encoding="utf-8",
            )
            value = summarize_report(path)
            self.assertEqual(
                value["function_counts"], {"exact": 1, "total": 2}
            )
            self.assertIn("Local object-diff evidence", render_summary([value]))


if __name__ == "__main__":
    unittest.main()
