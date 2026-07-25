import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.agent import _safe_name, doctor_checks
from tools.recovery_core import load
from tools.tests.test_recovery_workflow import RecoveryWorkflowTests


class AgentWorkspaceTests(unittest.TestCase):
    def fixture(self, root: Path) -> None:
        RecoveryWorkflowTests().fixture(root)
        project_path = root / "config/recovery/project.json"
        project = json.loads(project_path.read_text(encoding="utf-8"))
        project["agent_readiness"] = {
            "required_files": ["config/recovery/project.json"],
            "forbidden_paths": ["README.example.md"],
        }
        project_path.write_text(json.dumps(project), encoding="utf-8")
        subprocess.run(["git", "init", "-q"], cwd=root, check=True)
        subprocess.run(
            ["git", "config", "user.email", "test@example.com"],
            cwd=root,
            check=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Test"], cwd=root, check=True
        )
        subprocess.run(["git", "add", "."], cwd=root, check=True)
        subprocess.run(
            ["git", "commit", "-qm", "fixture"], cwd=root, check=True
        )

    def test_safe_context_filename(self):
        self.assertEqual(
            _safe_name("REL:mdpartydll:mdparty/fn_1_BBD8"),
            "REL_mdpartydll_mdparty_fn_1_BBD8",
        )

    def test_doctor_detects_template_leftovers(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            checks = doctor_checks(load(root, validate=False))
            cleanup = next(item for item in checks if item.name == "template cleanup")
            self.assertEqual(cleanup.status, "pass")

            (root / "README.example.md").write_text("template", encoding="utf-8")
            checks = doctor_checks(load(root, validate=False))
            cleanup = next(item for item in checks if item.name == "template cleanup")
            self.assertEqual(cleanup.status, "fail")
            self.assertIn("README.example.md", cleanup.detail)

    def test_doctor_reports_tracked_generated_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            generated = root / "build.ninja"
            generated.write_text("rule test\n", encoding="utf-8")
            subprocess.run(["git", "add", "-f", "build.ninja"], cwd=root, check=True)
            subprocess.run(
                ["git", "commit", "-qm", "track generated"],
                cwd=root,
                check=True,
            )
            checks = doctor_checks(load(root, validate=False))
            generated_check = next(
                item for item in checks if item.name == "generated files"
            )
            self.assertEqual(generated_check.status, "fail")
            self.assertIn("build.ninja", generated_check.detail)


if __name__ == "__main__":
    unittest.main()
