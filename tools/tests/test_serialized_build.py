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
                    lock_path=root / "retail-build.lock",
                )
            self.assertEqual(result, 0)
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
                    lock_path=Path(directory) / "retail-build.lock",
                )


if __name__ == "__main__":
    unittest.main()
