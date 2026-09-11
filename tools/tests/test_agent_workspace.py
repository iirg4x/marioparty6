import json
import subprocess
import tempfile
import unittest
from contextlib import ExitStack, redirect_stdout
from io import StringIO
from pathlib import Path

from unittest.mock import Mock, patch

from tools.agent import Check, _safe_name, _with_operational_context_owner, doctor_checks, public_check
from tools.recovery_core import load, quality_findings
from tools.tests.test_recovery_workflow import RecoveryWorkflowTests


class AgentWorkspaceTests(unittest.TestCase):
    def public_gate(self, *, overrides=None, data=None, base="HEAD"):
        stack = ExitStack()
        self.addCleanup(stack.close)
        data = data or {"root": Path("fixture"), "owners": []}
        defaults = {
            "load": Mock(return_value=data),
            "doctor_checks": Mock(return_value=[Check("doctor", "pass", "ok")]),
            "_metadata_errors": Mock(return_value=[]),
            "_git": Mock(return_value=subprocess.CompletedProcess([], 0, "", "")),
            "_changed_forbidden": Mock(return_value=[]),
            "quality_findings": Mock(return_value=[]),
            "queue_path": Mock(return_value=Path("queue")),
            "read_queue": Mock(return_value={"tasks": ["task"]}),
            "check_diff_claim": Mock(return_value={"task": "task", "errors": []}),
            "_run": Mock(return_value=subprocess.CompletedProcess([], 0)),
            "_public_context": Mock(),
        }
        defaults.update(overrides or {})
        for name, mock in defaults.items():
            stack.enter_context(patch("tools.agent." + name, mock))
        output = StringIO()
        with redirect_stdout(output):
            result = public_check(data, base=base)
        return result, defaults, output.getvalue()

    def test_public_preflight_failures_skip_expensive_commands(self):
        finding = {"path": "src/test.c", "line": 1, "rule": "guard",
                   "message": "override", "classification": "unreviewed"}
        failures = {
            "doctor_checks": [Check("doctor", "fail", "bad")],
            "_metadata_errors": ["bad metadata"],
            "_git": subprocess.CompletedProcess([], 1, "bad diff", ""),
            "_changed_forbidden": ["build.ninja"],
            "quality_findings": [finding],
            "check_diff_claim": {"task": "task", "errors": ["outside claim"]},
        }
        for name, value in failures.items():
            with self.subTest(check=name):
                result, mocks, output = self.public_gate(overrides={name: Mock(return_value=value)})
                self.assertEqual(result, 1)
                mocks["_run"].assert_not_called()
                mocks["_public_context"].assert_not_called()
                self.assertIn("test suite not run", output)
                self.assertNotIn("gate: PASS", output)

    def test_public_clean_runs_complete_suite_and_refreshes_context_once(self):
        result, mocks, output = self.public_gate()
        self.assertEqual(result, 0)
        commands = [call.args[0] for call in mocks["_run"].call_args_list]
        self.assertEqual([command[2:] for command in commands], [
            ["compileall", "-q", "tools"],
            ["unittest", "discover", "-s", "tools/tests", "-v"],
        ])
        self.assertEqual(mocks["load"].call_count, 2)
        self.assertEqual(mocks["quality_findings"].call_count, 2)
        mocks["_public_context"].assert_called_once()
        self.assertIn("gate: PASS", output)

    def test_public_suite_failure_is_not_erased_by_final_review(self):
        result, mocks, output = self.public_gate(overrides={
            "_run": Mock(side_effect=[subprocess.CompletedProcess([], 0),
                                      subprocess.CompletedProcess([], 1)])})
        self.assertEqual(result, 1)
        self.assertEqual(mocks["quality_findings"].call_count, 2)
        self.assertIn("gate: FAILED", output)

    def test_public_post_test_changed_quality_and_invalid_metadata_fail(self):
        finding = {"path": "src/test.c", "line": 1, "rule": "guard",
                   "message": "new override", "classification": "unreviewed"}
        for name, value in (("quality_findings", [finding]),
                            ("_metadata_errors", ["new invalid metadata"])):
            with self.subTest(check=name):
                result, mocks, output = self.public_gate(overrides={
                    name: Mock(side_effect=[[], value])})
                self.assertEqual(result, 1)
                self.assertEqual(mocks["_run"].call_count, 2)
                mocks["_public_context"].assert_not_called()
                self.assertIn("gate: FAILED", output)

    def test_public_reloads_resolved_exception_instead_of_stale_review(self):
        for remove_guard in (False, True):
            with self.subTest(remove_guard=remove_guard), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                self.fixture(root)
                source = root / "src/a.c"
                original = source.read_text(encoding="utf-8")
                source.write_text("#define _MATH_H\n" + original, encoding="utf-8")
                subprocess.run(["git", "add", "src/a.c"], cwd=root, check=True)
                subprocess.run(["git", "commit", "-qm", "guard"], cwd=root, check=True)
                exceptions = root / "config/recovery/exceptions.json"
                payload = {"schema_version": 1, "exceptions": [{
                    "id": "guard", "path": "src/a.c",
                    "rules": ["include_guard_override"],
                    "classification": "authenticated",
                }]}
                exceptions.write_text(json.dumps(payload), encoding="utf-8")
                initial = load(root, validate=False)

                def run_suite(command, **kwargs):
                    if "unittest" in command:
                        payload["exceptions"][0]["classification"] = "resolved"
                        exceptions.write_text(json.dumps(payload), encoding="utf-8")
                        if remove_guard:
                            source.write_text(original, encoding="utf-8")
                    return subprocess.CompletedProcess(command, 0)

                result, mocks, output = self.public_gate(data=initial, base="HEAD^", overrides={
                    "load": Mock(wraps=load),
                    "quality_findings": Mock(wraps=quality_findings),
                    "_run": Mock(side_effect=run_suite),
                })
                self.assertEqual(result, 0 if remove_guard else 1)
                self.assertEqual(mocks["_run"].call_count, 2)
                final = mocks["quality_findings"].call_args.args[0]
                self.assertEqual(final["exceptions"][0]["classification"], "resolved")
                self.assertEqual(initial["exceptions"][0]["classification"], "authenticated")
                if remove_guard:
                    mocks["_public_context"].assert_called_once_with(final)
                    self.assertNotIn("include_guard_override:", output)
                else:
                    mocks["_public_context"].assert_not_called()
                    self.assertIn("include_guard_override:", output)

    def test_public_final_review_detects_new_unstaged_guard(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.fixture(root)
            source = root / "src/a.c"
            original = source.read_text(encoding="utf-8")

            def run_suite(command, **kwargs):
                if "unittest" in command:
                    source.write_text("#define _MATH_H\n" + original, encoding="utf-8")
                return subprocess.CompletedProcess(command, 0)

            result, mocks, output = self.public_gate(data=load(root), overrides={
                "load": Mock(wraps=load),
                "quality_findings": Mock(wraps=quality_findings),
                "_run": Mock(side_effect=run_suite),
            })
            self.assertEqual(result, 1)
            self.assertEqual(mocks["_run"].call_count, 2)
            mocks["_public_context"].assert_not_called()
            self.assertIn("include_guard_override:", output)

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

    def test_context_can_use_unreviewed_operational_owner(self):
        data = {"root": Path("."), "owners": []}
        catalog = {
            "owners": [
                {
                    "id": "main:board/math",
                    "module": "main",
                    "source": "src/board/math.c",
                    "configured_status": "NonMatching",
                }
            ]
        }
        with patch("tools.agent._catalog", return_value=catalog):
            result = _with_operational_context_owner(data, "main:board/math")
        self.assertEqual(result["owners"][0]["id"], "main:board/math")
        self.assertEqual(result["owners"][0]["status"]["binary"], "partial")
        self.assertIn("no reviewed", result["owners"][0]["summary"])
        self.assertEqual(data["owners"], [])

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
