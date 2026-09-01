from __future__ import annotations

import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools import crack_cell_runner
from tools import owner_campaign


class AdjacentInlineHelperScopeTests(unittest.TestCase):
    def setUp(self) -> None:
        self.base = (
            "int before = 1;\n"
            "\n"
            "float mbev_CapBiriQVelocity(float speed, float axis) {\n"
            "    return speed * axis;\n"
            "}\n"
            "int after = 2;\n"
        )
        self.candidate = (
            "int before = 1;\n"
            "\n"
            "static inline float VelocityScale(float magnitude, float axis) {\n"
            "    return magnitude * axis;\n"
            "}\n"
            "float mbev_CapBiriQVelocity(float speed, float axis) {\n"
            "    float x = VelocityScale(speed, axis);\n"
            "    return VelocityScale(x, axis);\n"
            "}\n"
            "int after = 2;\n"
        )
        self.base_start, self.base_end, _ = owner_campaign._find_function_span(
            self.base, "mbev_CapBiriQVelocity"
        )
        self.candidate_start, self.candidate_end, _ = owner_campaign._find_function_span(
            self.candidate, "mbev_CapBiriQVelocity"
        )
        helper_lines = self.candidate.splitlines(keepends=True)
        helper_bytes = "".join(helper_lines[2:5]).encode()
        self.scope = {
            "kind": owner_campaign.ADJACENT_HELPER_SCOPE_KIND,
            "base_source_sha256": hashlib.sha256(self.base.encode()).hexdigest(),
            "candidate_source_sha256": hashlib.sha256(self.candidate.encode()).hexdigest(),
            "base_insertion": {
                "line": self.base_start,
                "sha256": hashlib.sha256(b"").hexdigest(),
            },
            "helper": {
                "name": "VelocityScale",
                "start_line": 3,
                "end_line": 5,
                "sha256": hashlib.sha256(helper_bytes).hexdigest(),
            },
            "use_sites": [
                {"line": 7, "name": "VelocityScale", "column": 14},
                {"line": 8, "name": "VelocityScale", "column": 11},
            ],
        }

    def _validate(self, base: str | None = None, candidate: str | None = None, scope=None):
        base = self.base if base is None else base
        candidate = self.candidate if candidate is None else candidate
        scope = self.scope if scope is None else scope
        bstart, bend, _ = owner_campaign._find_function_span(
            base, "mbev_CapBiriQVelocity"
        )
        cstart, cend, _ = owner_campaign._find_function_span(
            candidate, "mbev_CapBiriQVelocity"
        )
        return owner_campaign.validate_candidate_scope(
            base_text=base,
            candidate_text=candidate,
            function="mbev_CapBiriQVelocity",
            base_start_line=bstart,
            base_end_line=bend,
            candidate_start_line=cstart,
            candidate_end_line=cend,
            base_source_sha256=hashlib.sha256(base.encode()).hexdigest(),
            candidate_source_sha256=hashlib.sha256(candidate.encode()).hexdigest(),
            scope=scope,
        )

    def test_velocity_scale_shape_is_hash_bound_and_valid(self) -> None:
        validated = self._validate()
        self.assertEqual(validated["kind"], "function_plus_adjacent_static_inline")
        self.assertEqual(validated["helper"]["name"], "VelocityScale")
        self.assertEqual(len(validated["use_sites"]), 2)

    def test_non_adjacent_helper_fails_closed(self) -> None:
        candidate = self.candidate.replace(
            "}\nfloat mbev_CapBiriQVelocity", "}\nint bridge = 0;\nfloat mbev_CapBiriQVelocity", 1
        )
        with self.assertRaisesRegex(owner_campaign.CampaignError, "immediately adjacent"):
            self._validate(candidate=candidate)

    def test_non_static_inline_helper_fails_closed(self) -> None:
        candidate = self.candidate.replace("static inline float VelocityScale", "inline float VelocityScale")
        helper_lines = candidate.splitlines(keepends=True)
        helper_bytes = "".join(helper_lines[2:5]).encode()
        scope = {**self.scope}
        scope["candidate_source_sha256"] = hashlib.sha256(candidate.encode()).hexdigest()
        scope["helper"] = {
            **scope["helper"],
            "sha256": hashlib.sha256(helper_bytes).hexdigest(),
        }
        with self.assertRaisesRegex(owner_campaign.CampaignError, "static inline"):
            self._validate(candidate=candidate, scope=scope)

    def test_use_outside_target_function_fails_closed(self) -> None:
        candidate = self.candidate.replace("int after = 2;", "int after = (int)VelocityScale(1.0f, 2.0f);")
        scope = {**self.scope}
        scope["candidate_source_sha256"] = hashlib.sha256(candidate.encode()).hexdigest()
        with self.assertRaisesRegex(owner_campaign.CampaignError, "suffix|escapes the target function"):
            self._validate(candidate=candidate, scope=scope)

    def test_cell_runner_uses_scope_validator(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            base_path = root / "base.c"
            candidate_path = root / "candidate.c"
            base_path.write_bytes(self.base.encode())
            candidate_path.write_bytes(self.candidate.encode())
            artifact = {
                "function": "mbev_CapBiriQVelocity",
                "base_sha256": hashlib.sha256(self.base.encode()).hexdigest(),
                "function_span": {
                    "start_line": self.base_start,
                    "end_line": self.base_end,
                    "base_span_sha256": hashlib.sha256(
                        "".join(self.base.splitlines(keepends=True)[self.base_start - 1:self.base_end]).encode()
                    ).hexdigest(),
                },
                "candidate_scope": self.scope,
            }
            crack_cell_runner._validate_candidate_cell(base_path, candidate_path, artifact)

    def test_owner_campaign_loader_accepts_sealed_helper_scope(self) -> None:
        with tempfile.TemporaryDirectory() as raw:
            root = Path(raw)
            (root / "src").mkdir()
            (root / "build").mkdir()
            live = root / "src" / "test.c"
            base_path = root / "build" / "base.c"
            candidate_path = root / "build" / "candidate.c"
            descriptor_path = root / "build" / "candidate.json"
            live.write_bytes(self.base.encode("utf-8"))
            base_path.write_bytes(self.base.encode("utf-8"))
            candidate_path.write_bytes(self.candidate.encode("utf-8"))
            base_span = "".join(
                self.base.splitlines(keepends=True)[self.base_start - 1:self.base_end]
            ).encode("utf-8")
            candidate_span = "".join(
                self.candidate.splitlines(keepends=True)[
                    self.candidate_start - 1:self.candidate_end
                ]
            ).encode("utf-8")
            frontier_sha = "f" * 64
            body = {
                "schema": owner_campaign.CANDIDATE_SCHEMA,
                "campaign_id": "helper-scope-v1",
                "function": "mbev_CapBiriQVelocity",
                "base_frontier_sha256": frontier_sha,
                "base_source": {
                    "path": "build/base.c",
                    "sha256": hashlib.sha256(self.base.encode()).hexdigest(),
                },
                "candidate_source": {
                    "path": "build/candidate.c",
                    "sha256": hashlib.sha256(self.candidate.encode()).hexdigest(),
                },
                "function_span": {
                    "base_start_line": self.base_start,
                    "base_end_line": self.base_end,
                    "candidate_start_line": self.candidate_start,
                    "candidate_end_line": self.candidate_end,
                    "base_sha256": hashlib.sha256(base_span).hexdigest(),
                    "candidate_sha256": hashlib.sha256(candidate_span).hexdigest(),
                },
                "candidate_scope": self.scope,
                "hypothesis_family": "adjacent-inline-helper",
                "natural_c": True,
                "rebase_depth": 0,
                "created_at": "2026-01-01T00:00:00+00:00",
            }
            descriptor = {
                **body,
                "candidate_sha256": owner_campaign._digest_json(body),
            }
            descriptor_path.write_text(json.dumps(descriptor), encoding="utf-8")
            campaign = {
                "campaign_id": "helper-scope-v1",
                "_source": live,
                "allowed_build_paths": ["build"],
                "allowed_source_paths": ["src/test.c"],
                "forbidden_constructs": [],
            }
            frontier = {
                "function": "mbev_CapBiriQVelocity",
                "frontier_sha256": frontier_sha,
                "source_sha256": hashlib.sha256(self.base.encode()).hexdigest(),
            }
            loaded = owner_campaign._load_candidate(
                root, descriptor_path, campaign, frontier
            )
            self.assertEqual(loaded["candidate_scope"], self.scope)


if __name__ == "__main__":
    unittest.main()
