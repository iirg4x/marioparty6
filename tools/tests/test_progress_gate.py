import unittest

from tools.progress_gate import COMMON_PROGRESS_PATHS, progress_errors


class ProgressGateTests(unittest.TestCase):
    def test_source_promotion_requires_status(self) -> None:
        self.assertEqual(
            progress_errors(
                ["src/board/capsule.c"]
            ),
            ["recovery source or Matching status changed, but STATUS.md was not updated"],
        )
        self.assertEqual(
            progress_errors(
                ["src/board/capsule.c", "STATUS.md"],
            ),
            [],
        )

    def test_canonical_cpp_source_suffixes_require_status(self) -> None:
        for prefix, suffix in (
            ("src/Runtime", ".cp"),
            ("src/Runtime", ".cpp"),
            ("src/Runtime", ".h"),
            ("src/Runtime", ".hpp"),
            ("include/Runtime", ".h"),
            ("include/Runtime", ".hpp"),
        ):
            with self.subTest(suffix=suffix):
                self.assertEqual(
                    progress_errors([f"{prefix}/example{suffix}"]),
                    [
                        "recovery source or Matching status changed, but STATUS.md was not updated"
                    ],
                )

    def test_dol_matching_change_requires_only_dol_snapshot(self) -> None:
        errors = progress_errors(
            ["configure.py", "STATUS.md"], matching_categories={"dol"}
        )
        self.assertEqual(len(errors), 1)
        self.assertIn("generated progress files", errors[0])
        self.assertEqual(
            progress_errors(
                [
                    "configure.py",
                    "STATUS.md",
                    *COMMON_PROGRESS_PATHS,
                    "progress/dol.json",
                ],
                matching_categories={"dol"},
            ),
            [],
        )

    def test_rel_matching_change_requires_only_dll_snapshot(self) -> None:
        self.assertEqual(
            progress_errors(
                [
                    "configure.py",
                    "STATUS.md",
                    *COMMON_PROGRESS_PATHS,
                    "progress/dlls.json",
                ],
                matching_categories={"dlls"},
            ),
            [],
        )

    def test_mixed_matching_change_requires_both_category_snapshots(self) -> None:
        errors = progress_errors(
            [
                "configure.py",
                "STATUS.md",
                *COMMON_PROGRESS_PATHS,
                "progress/dol.json",
            ],
            matching_categories={"dol", "dlls"},
        )
        self.assertEqual(len(errors), 1)
        self.assertIn("progress/dlls.json", errors[0])

    def test_unrelated_change_needs_no_progress_update(self) -> None:
        self.assertEqual(
            progress_errors(["docs/getting_started.md"]),
            [],
        )


if __name__ == "__main__":
    unittest.main()
