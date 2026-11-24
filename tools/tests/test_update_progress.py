import json
import tempfile
import unittest
from pathlib import Path

from tools.update_progress import (
    ProgressError,
    build_snapshot,
    check_snapshot,
    output_documents,
    update_from_build,
)


class UpdateProgressTests(unittest.TestCase):
    def progress_data(self):
        return {
            "all": {
                "code": 1213,
                "code/total": 10000,
                "data": 3225,
                "data/total": 10000,
            },
            "dol": {
                "code": 4742,
                "code/total": 10000,
                "data": 7459,
                "data/total": 10000,
            },
            "modules": {
                "code": 465,
                "code/total": 10000,
                "data": 542,
                "data/total": 10000,
            },
        }

    def test_build_snapshot_calculates_percentages(self):
        snapshot = build_snapshot(self.progress_data(), "GP6E01")
        self.assertEqual(
            snapshot["categories"]["dol"]["code"]["percent"], 47.42
        )
        self.assertEqual(
            snapshot["categories"]["modules"]["data"]["percent"], 5.42
        )

    def test_output_documents_generate_badge_endpoints(self):
        snapshot = build_snapshot(self.progress_data(), "GP6E01")
        documents = output_documents(snapshot)
        dol = json.loads(documents["dol.json"])
        dlls = json.loads(documents["dlls.json"])
        self.assertEqual(dol["label"], "DOL")
        self.assertEqual(dol["message"], "47.42%")
        self.assertEqual(dlls["message"], "4.65%")
        self.assertEqual(documents["code.json"], documents["all.json"])
        self.assertEqual(
            json.loads(documents["code.json"]),
            {"schemaVersion": 1, "label": "Code", "message": "12.13%", "color": "orange"},
        )
        self.assertEqual(
            json.loads(documents["data.json"]),
            {"schemaVersion": 1, "label": "Data", "message": "32.25%", "color": "yellow"},
        )

    def test_missing_data_metric_is_rejected(self):
        snapshot = build_snapshot(self.progress_data(), "GP6E01")
        del snapshot["categories"]["all"]["data"]
        with self.assertRaisesRegex(ProgressError, "no data metric: all"):
            output_documents(snapshot)

    def test_update_and_check(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "progress.json"
            output = root / "published"
            source.write_text(json.dumps(self.progress_data()), encoding="utf-8")

            changed = update_from_build(source, output, "GP6E01")
            self.assertEqual(
                {path.name for path in changed},
                {"GP6E01.json", "all.json", "dol.json", "dlls.json", "code.json", "data.json"},
            )
            self.assertEqual(
                check_snapshot(output / "GP6E01.json", output), []
            )

            changed = update_from_build(source, output, "GP6E01")
            self.assertEqual(changed, [])

    def test_check_detects_stale_badge(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root / "published"
            source = root / "progress.json"
            source.write_text(json.dumps(self.progress_data()), encoding="utf-8")
            update_from_build(source, output, "GP6E01")
            (output / "code.json").unlink()
            (output / "data.json").write_text("{}\n", encoding="utf-8")
            errors = check_snapshot(output / "GP6E01.json", output)
            self.assertEqual(
                errors,
                [
                    f"missing generated progress file: {output / 'code.json'}",
                    f"stale generated progress file: {output / 'data.json'}",
                ],
            )

    def test_missing_category_is_rejected(self):
        progress = self.progress_data()
        del progress["modules"]
        with self.assertRaisesRegex(ProgressError, "modules"):
            build_snapshot(progress, "GP6E01")


if __name__ == "__main__":
    unittest.main()
