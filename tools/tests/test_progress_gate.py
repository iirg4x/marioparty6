import unittest

from tools.progress_gate import PROGRESS_PATHS, progress_errors


class ProgressGateTests(unittest.TestCase):
    def test_source_promotion_requires_status(self) -> None:
        self.assertEqual(
            progress_errors(
                ["src/board/capsule.c"], matching_status_changed=False
            ),
            ["recovery source or Matching status changed, but STATUS.md was not updated"],
        )
        self.assertEqual(
            progress_errors(
                ["src/board/capsule.c", "STATUS.md"],
                matching_status_changed=False,
            ),
            [],
        )

    def test_matching_change_requires_status_and_generated_snapshot(self) -> None:
        errors = progress_errors(
            ["configure.py", "STATUS.md"], matching_status_changed=True
        )
        self.assertEqual(len(errors), 1)
        self.assertIn("generated progress files", errors[0])
        self.assertEqual(
            progress_errors(
                ["configure.py", "STATUS.md", *PROGRESS_PATHS],
                matching_status_changed=True,
            ),
            [],
        )

    def test_unrelated_change_needs_no_progress_update(self) -> None:
        self.assertEqual(
            progress_errors(["docs/getting_started.md"], matching_status_changed=False),
            [],
        )


if __name__ == "__main__":
    unittest.main()
