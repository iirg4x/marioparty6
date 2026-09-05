from contextlib import redirect_stdout
import io
from pathlib import Path
import unittest
from unittest.mock import patch

from tools import agent


class ReadOnlyContextTests(unittest.TestCase):
    def run_context(self, *flags):
        output = io.StringIO()
        with (
            patch("sys.argv", ["agent.py", "context", "function", "focus", *flags]),
            patch.object(agent, "root_from", return_value=Path.cwd()),
            patch.object(agent, "load", return_value={}),
            patch.object(agent, "startup_check") as startup,
            patch.object(agent, "_write_context") as write,
            redirect_stdout(output),
        ):
            result = agent.main()
        return result, output.getvalue(), startup, write

    def test_read_only_skips_sync_and_forces_stdout(self):
        result, output, startup, write = self.run_context("--read-only")
        self.assertEqual(result, 0)
        startup.assert_not_called()
        self.assertTrue(write.call_args.args[1].stdout)
        self.assertIn("no execution or retention authority", output)

    def test_read_only_rejects_output_file(self):
        result, _, startup, write = self.run_context("--read-only", "--output", "pack.md")
        self.assertEqual(result, 2)
        startup.assert_not_called()
        write.assert_not_called()

    def test_default_still_checks_startup(self):
        result, _, startup, write = self.run_context("--stdout")
        self.assertEqual(result, 0)
        startup.assert_called_once_with(Path.cwd(), sync_reports=True, strict_reports=True)
        write.assert_called_once()


if __name__ == "__main__":
    unittest.main()
