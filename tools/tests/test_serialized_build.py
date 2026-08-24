from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools.serialized_build import run


class SerializedBuildTests(unittest.TestCase):
    def test_build_is_always_serial(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "build.ninja").touch()
            with mock.patch("tools.serialized_build.subprocess.run") as invoke:
                invoke.return_value.returncode = 0
                result = run(
                    root,
                    ["build/object.o"],
                )
            self.assertEqual(result, 0)
            self.assertTrue((root / "build/.compiler-lane.lock").is_file())
            invoke.assert_called_once_with(
                ["ninja", "-j1", "build/object.o"],
                cwd=root.resolve(),
                check=False,
            )

    def test_missing_build_file_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(ValueError, "build.ninja"):
                run(
                    Path(directory),
                    ["build/object.o"],
                )

    def test_distinct_worktrees_do_not_share_the_default_lock(self) -> None:
        with tempfile.TemporaryDirectory() as first_directory, tempfile.TemporaryDirectory() as second_directory:
            first = Path(first_directory)
            second = Path(second_directory)
            (first / "build.ninja").touch()
            (second / "build.ninja").touch()
            with mock.patch("tools.serialized_build.subprocess.run") as invoke:
                invoke.return_value.returncode = 0
                self.assertEqual(run(first, ["build/first.o"]), 0)
                self.assertEqual(run(second, ["build/second.o"]), 0)
            first_lock = first / "build/.compiler-lane.lock"
            second_lock = second / "build/.compiler-lane.lock"
            self.assertTrue(first_lock.is_file())
            self.assertTrue(second_lock.is_file())
            self.assertNotEqual(first_lock.resolve(), second_lock.resolve())


if __name__ == "__main__":
    unittest.main()
