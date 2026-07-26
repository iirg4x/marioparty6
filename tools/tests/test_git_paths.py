import unittest

from tools.git_paths import windows_git_path_text


class GitPathTests(unittest.TestCase):
    def test_msys_drive_path_becomes_native_drive_path(self) -> None:
        self.assertEqual(
            windows_git_path_text("/d/work/repo/.git"),
            "D:/work/repo/.git",
        )

    def test_non_drive_path_is_unchanged(self) -> None:
        self.assertEqual(
            windows_git_path_text(".git/worktrees/worker"),
            ".git/worktrees/worker",
        )

    def test_msys_home_path_uses_native_home(self) -> None:
        self.assertEqual(
            windows_git_path_text(
                "/home/Anony/AppData/Local/Temp/repo",
                home="C:/Users/Anony",
            ).replace("\\", "/"),
            "C:/Users/Anony/AppData/Local/Temp/repo",
        )


if __name__ == "__main__":
    unittest.main()
