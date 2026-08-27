import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.context_engine import build_context
from tools.recovery_core import load
from tools.recovery_memory import RecoveryMemory
from tools.tests import test_recovery_workflow as recovery_fixture


def run(cwd: Path, *args: str) -> None:
    subprocess.run(
        args,
        cwd=cwd,
        text=True,
        capture_output=True,
        check=True,
    )


class ContextEngineMemoryTests(unittest.TestCase):
    def test_completed_report_is_rendered_from_central_memory(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            recovery_fixture.RecoveryWorkflowTests().fixture(root)
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "base")
            report = root / "Fn_CRACK_REPORT_v1.json"
            report.write_text(
                json.dumps(
                    {
                        "schema": "CRACK_REPORT/v1",
                        "owner": "REL:test:a",
                        "function": "fn_1_20",
                        "result": {
                            "strict_percent": 100.0,
                            "data_percent": 100.0,
                        },
                        "causal_explanation": (
                            "The direct typed consumer removed a redundant owner."
                        ),
                        "generalized_improvement_request": {
                            "requested_behavior": (
                                "Rank direct typed consumers before declaration permutations."
                            )
                        },
                        "chronological_attempt_ledger": [
                            {
                                "id": "c001",
                                "decision": "rejected; object-neutral",
                            }
                        ],
                    }
                ),
                encoding="utf-8",
            )
            RecoveryMemory.for_root(root).ingest_report(report)
            packet = build_context(
                load(root, validate=False),
                "function",
                "fn_1_20",
                owner_id="REL:test:a",
                budget=8000,
            )
            self.assertIn("Central recovery memory", packet)
            self.assertIn("direct typed consumer removed", packet)
            self.assertIn("Negative controls: 1", packet)


if __name__ == "__main__":
    unittest.main()
