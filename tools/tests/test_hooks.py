import os
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.hooks import hook_status, install_hooks, uninstall_hooks


def run(cwd: Path, *args: str) -> None:
    subprocess.run(
        args,
        cwd=cwd,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )


class HookTests(unittest.TestCase):
    def test_install_status_uninstall(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            run(root, "git", "init", "-q")
            paths = install_hooks(root)
            self.assertEqual(set(hook_status(root).values()), {"managed"})
            if os.name != "nt":
                self.assertTrue(all(path.stat().st_mode & 0o100 for path in paths))
            pre_commit = next(path for path in paths if path.name == "pre-commit")
            script = pre_commit.read_text(encoding="utf-8")
            self.assertNotIn("origin/main", script)
            self.assertIn("MP6_AGENT_BASE", script)
            pre_push = next(path for path in paths if path.name == "pre-push")
            push_script = pre_push.read_text(encoding="utf-8")
            self.assertIn("tools/agent.py memory startup-check --no-sync", pre_commit.read_text(encoding="utf-8"))
            self.assertIn("tools/agent.py memory startup-check --no-sync", push_script)
            self.assertIn("git rev-parse --local-env-vars", push_script)
            self.assertIn('unset "$name"', push_script)
            self.assertIn('"refs/heads/main"', push_script)
            self.assertIn("tools/progress_gate.py", push_script)
            uninstall_hooks(root)
            self.assertEqual(set(hook_status(root).values()), {"missing"})


if __name__ == "__main__":
    unittest.main()
