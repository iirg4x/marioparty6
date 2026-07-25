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
            self.assertTrue(all(path.stat().st_mode & 0o100 for path in paths))
            uninstall_hooks(root)
            self.assertEqual(set(hook_status(root).values()), {"missing"})


if __name__ == "__main__":
    unittest.main()
