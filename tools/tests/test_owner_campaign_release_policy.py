from __future__ import annotations

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]


class OwnerCampaignReleasePolicyTests(unittest.TestCase):
    def test_candidate_release_never_pauses_existing_lanes(self) -> None:
        workflow = (ROOT / "MP6_CRACKING_WORKFLOW_V2.md").read_text(
            encoding="utf-8"
        )
        owner_campaign = (ROOT / "docs" / "owner_campaign.md").read_text(
            encoding="utf-8"
        )
        combined = workflow + "\n" + owner_campaign
        workflow_words = " ".join(workflow.split())
        owner_campaign_words = " ".join(owner_campaign.split())

        self.assertNotIn("Existing lanes stay paused", combined)
        self.assertIn(
            "Existing owner lanes continue on the last verified release",
            workflow_words,
        )
        self.assertIn(
            "each lane may adopt the release independently", workflow_words
        )
        self.assertIn("does not pause unrelated lanes", workflow_words)
        self.assertIn(
            "Existing lanes keep cracking on the last verified release",
            owner_campaign_words,
        )
        self.assertIn("does not pause unrelated lanes", owner_campaign_words)


if __name__ == "__main__":
    unittest.main()
