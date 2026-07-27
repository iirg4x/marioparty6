import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.promote_recovered_c import PromotionError
from tools.promote_supporting_change import (
    audit_promotion,
    contamination_findings,
    create_promotion,
    plan_promotion,
    supporting_branch_errors,
    supporting_message_errors,
)

CONFIGURE = """\
Matching = True
NonMatching = False


def Object(status, path):
    return (status, path)


OBJECTS = [
    Object(Matching, "game/example.c"),
    Object(Matching, "game/wrapped.c"),
    Object(NonMatching, "game/pending.c"),
]
"""


def run(cwd: Path, *args: str) -> str:
    result = subprocess.run(
        args,
        cwd=cwd,
        check=True,
        text=True,
        capture_output=True,
    )
    return result.stdout.strip()


class SupportingPromotionTests(unittest.TestCase):
    def fixture(self) -> tuple[tempfile.TemporaryDirectory[str], Path, str]:
        temporary = tempfile.TemporaryDirectory()
        root = Path(temporary.name) / "repo"
        root.mkdir()
        run(root, "git", "init", "-q", "-b", "main")
        run(root, "git", "config", "user.email", "test@example.com")
        run(root, "git", "config", "user.name", "Test")
        (root / "include/game").mkdir(parents=True)
        (root / "src/game").mkdir(parents=True)
        (root / "config/GP6E01").mkdir(parents=True)
        (root / "tools").mkdir()
        (root / "configure.py").write_text(CONFIGURE, encoding="utf-8")
        (root / "include/game/example.h").write_text(
            "#ifndef _EXAMPLE_H\n#define _EXAMPLE_H\n\nu32 Example(u32 seed);\n\n#endif\n",
            encoding="utf-8",
        )
        (root / "include/game/wrapper.h").write_text(
            '#include "game/example.h"\n',
            encoding="utf-8",
        )
        (root / "src/game/example.c").write_text(
            '#include "game/example.h"\n\nu32 Example(u32 seed)\n{\n    return seed;\n}\n',
            encoding="utf-8",
        )
        (root / "src/game/wrapped.c").write_text(
            '#include "game/wrapper.h"\n\nvoid Wrapped(void)\n{\n}\n',
            encoding="utf-8",
        )
        (root / "src/game/pending.c").write_text(
            '#include "game/example.h"\n\nvoid Pending(void)\n{\n}\n',
            encoding="utf-8",
        )
        (root / "config/GP6E01/symbols.txt").write_text(
            "Example = .text:0x80000000; // type:function\n",
            encoding="utf-8",
        )
        run(root, "git", "add", ".")
        run(root, "git", "commit", "-qm", "Initial project")
        base = run(root, "git", "rev-parse", "HEAD")
        run(root, "git", "checkout", "-qb", "agent/recovery-lab")
        (root / "include/game/example.h").write_text(
            "#ifndef _EXAMPLE_H\n#define _EXAMPLE_H\n\ns32 Example(s32 seed);\n\n#endif\n",
            encoding="utf-8",
        )
        (root / "config/GP6E01/symbols.txt").write_text(
            "Example = .text:0x80000000; // type:function scope:global\n",
            encoding="utf-8",
        )
        (root / "tools/lab.py").write_text("print('tooling')\n", encoding="utf-8")
        run(root, "git", "add", ".")
        run(root, "git", "commit", "-qm", "Adjust shared interface in the workspace")
        return temporary, root, base

    def queue(self, root: Path, verified_commit: str, **overrides) -> None:
        task = {
            "id": "task-1",
            "owner": "main:game/example-interface",
            "status": "ready",
            "change_class": "shared-interface",
            "shared_files": [
                "include/game/example.h",
                "config/GP6E01/symbols.txt",
            ],
            "verification": {
                "public_gate": "pass",
                "verified_commit": verified_commit,
                "object_report": "build/GP6E01/report.json",
                "functions_exact": "1/1",
                "relocations": "exact",
                "consumers": {"main:game/example": "exact"},
            },
        }
        task.update(overrides)
        path = root / ".git/agent-coordination/queue.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(
            json.dumps(
                {
                    "schema_version": 2,
                    "updated_at": "2026-01-01T00:00:00+00:00",
                    "tasks": [task],
                    "resources": {},
                }
            ),
            encoding="utf-8",
        )

    def test_plan_lists_headers_and_matching_fanout(self):
        temporary, root, base = self.fixture()
        try:
            value = plan_promotion(
                root,
                base_ref=base,
                source_ref="HEAD",
                paths=[
                    "include/game/example.h",
                    "config/GP6E01/symbols.txt",
                ],
                allow_unverified=True,
            )
            self.assertEqual(
                [item["path"] for item in value["files"]],
                ["config/GP6E01/symbols.txt", "include/game/example.h"],
            )
            fanout = value["affected_consumers"]["include/game/example.h"]
            self.assertEqual(fanout["include_closure"], ["include/game/wrapper.h"])
            self.assertEqual(
                fanout["matching"],
                ["main:game/example", "main:game/wrapped"],
            )
            self.assertEqual(fanout["other"], ["main:game/pending"])
            self.assertEqual(
                value["affected_matching_consumers"],
                ["main:game/example", "main:game/wrapped"],
            )
        finally:
            temporary.cleanup()

    def test_plan_requires_explicit_paths(self):
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "explicit --path"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=[],
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_recovered_c_is_refused_with_pointer(self):
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(
                PromotionError, "promote_recovered_c"
            ):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["src/game/example.c"],
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_workspace_metadata_and_other_paths_are_refused(self):
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "config/recovery"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["config/recovery/names.json"],
                    allow_unverified=True,
                )
            with self.assertRaisesRegex(PromotionError, "include/"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["tools/lab.py"],
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_contamination_is_refused_not_stripped(self):
        self.assertTrue(
            contamination_findings(
                "include/game/example.h",
                "// recovered by Claude\ns32 Example(s32 seed);\n",
            )
        )
        self.assertTrue(
            contamination_findings(
                "include/game/example.h",
                "/* TODO(ai): confirm width */\ns32 Example(s32 seed);\n",
            )
        )
        self.assertTrue(
            contamination_findings(
                "include/game/example.h",
                "// see main:game/example in queue.json\n",
            )
        )
        temporary, root, base = self.fixture()
        try:
            (root / "include/game/example.h").write_text(
                "// verified via agent-coordination evidence\ns32 Example(s32 seed);\n",
                encoding="utf-8",
            )
            run(root, "git", "add", "include/game/example.h")
            run(root, "git", "commit", "-qm", "Update header")
            with self.assertRaisesRegex(PromotionError, "contamination"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["include/game/example.h"],
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_unchanged_path_is_refused(self):
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "identical to base"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["include/game/wrapper.h"],
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_queue_verification_is_required_and_bound(self):
        temporary, root, base = self.fixture()
        try:
            source_commit = run(root, "git", "rev-parse", "HEAD")
            with self.assertRaisesRegex(PromotionError, "--owner"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["include/game/example.h"],
                )
            self.queue(root, "0" * 40)
            with self.assertRaisesRegex(PromotionError, "bound to"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["include/game/example.h"],
                    owner="main:game/example-interface",
                )
            self.queue(root, source_commit, change_class="private-source")
            with self.assertRaisesRegex(PromotionError, "change class"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["include/game/example.h"],
                    owner="main:game/example-interface",
                )
            self.queue(root, source_commit, shared_files=[])
            with self.assertRaisesRegex(PromotionError, "not declared"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["include/game/example.h"],
                    owner="main:game/example-interface",
                )
            self.queue(root, source_commit)
            value = plan_promotion(
                root,
                base_ref=base,
                source_ref="HEAD",
                paths=["include/game/example.h"],
                owner="main:game/example-interface",
            )
            self.assertEqual(
                value["queue_proof"]["change_class"], "shared-interface"
            )
        finally:
            temporary.cleanup()

    def test_create_starts_from_main_and_copies_exact_blobs(self):
        temporary, root, base = self.fixture()
        promotion = Path(temporary.name) / "promotion"
        try:
            source_commit = run(root, "git", "rev-parse", "HEAD")
            result = create_promotion(
                root,
                base_ref=base,
                source_ref=source_commit,
                branch="project/example-interface",
                worktree=promotion,
                title="Correct Example seed signedness",
                paths=[
                    "include/game/example.h",
                    "config/GP6E01/symbols.txt",
                ],
                owner=None,
                allow_unverified=True,
            )
            changed = run(
                promotion,
                "git",
                "diff",
                "--name-only",
                f"{base}...HEAD",
            ).splitlines()
            self.assertEqual(
                changed,
                ["config/GP6E01/symbols.txt", "include/game/example.h"],
            )
            self.assertFalse((promotion / "tools/lab.py").exists())
            self.assertTrue(result["promotion"]["audit"]["clean_human_promotion"])
            for path in changed:
                self.assertEqual(
                    run(root, "git", "rev-parse", f"{source_commit}:{path}"),
                    run(promotion, "git", "rev-parse", f"HEAD:{path}"),
                )
            self.assertEqual(run(promotion, "git", "rev-parse", "HEAD^"), base)
            self.assertIn("ninja -j1", result["next_steps"])
            self.assertTrue(
                any("dtk shasum" in step for step in result["next_steps"])
            )
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(promotion)],
                cwd=root,
                check=False,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            temporary.cleanup()

    def test_branch_names_with_ai_words_are_refused(self):
        for branch in (
            "project/agent-fix",
            "project/ai-fix",
            "project/agent_fix",
            "project/claude-frand",
            "project/codex-x",
        ):
            self.assertTrue(
                any(
                    "AI/agent marker" in error
                    for error in supporting_branch_errors(branch)
                ),
                branch,
            )
        self.assertEqual(supporting_branch_errors("project/example-interface"), [])
        # "ai"/"agent" embedded inside larger words are not attribution.
        self.assertEqual(supporting_branch_errors("project/maintain-repair"), [])
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "AI/agent marker"):
                create_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    branch="project/agent-fix",
                    worktree=Path(temporary.name) / "promotion",
                    title="Correct Example seed signedness",
                    paths=["include/game/example.h"],
                    owner=None,
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_commit_message_contamination_is_refused(self):
        self.assertTrue(
            any(
                "contamination" in error
                for error in supporting_message_errors(
                    "Correct main:game/example per queue.json task"
                )
            )
        )
        self.assertTrue(
            any(
                "contamination" in error
                for error in supporting_message_errors(
                    "Update per agent-coordination evidence"
                )
            )
        )
        self.assertEqual(
            supporting_message_errors("Correct Example seed signedness"), []
        )
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "contamination"):
                create_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    branch="project/example-interface",
                    worktree=Path(temporary.name) / "promotion",
                    title="Correct main:game/example per queue.json task",
                    paths=["include/game/example.h"],
                    owner=None,
                    allow_unverified=True,
                )
            # The audit re-checks messages, so a contaminated title cannot be
            # smuggled onto a project/* branch created outside the tool either.
            run(root, "git", "checkout", "-qb", "project/smuggled", base)
            (root / "include/game/example.h").write_text(
                "#ifndef _EXAMPLE_H\n#define _EXAMPLE_H\n\ns32 Example(s32 seed);\n\n#endif\n",
                encoding="utf-8",
            )
            run(root, "git", "add", "include/game/example.h")
            run(root, "git", "commit", "-qm", "Track queue.json owner main:game/example")
            result = audit_promotion(
                root,
                base_ref=base,
                head_ref="HEAD",
            )
            self.assertFalse(result["clean_human_promotion"])
            self.assertTrue(
                any("contamination" in error for error in result["errors"])
            )
        finally:
            temporary.cleanup()

    def test_create_rejects_non_project_branch(self):
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "project/"):
                create_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    branch="recovery/example",
                    worktree=Path(temporary.name) / "promotion",
                    title="Correct Example seed signedness",
                    paths=["include/game/example.h"],
                    owner=None,
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_audit_requires_build_checksum_and_consumer_evidence(self):
        temporary, root, base = self.fixture()
        promotion = Path(temporary.name) / "promotion"
        try:
            source_commit = run(root, "git", "rev-parse", "HEAD")
            create_promotion(
                root,
                base_ref=base,
                source_ref=source_commit,
                branch="project/example-interface",
                worktree=promotion,
                title="Correct Example seed signedness",
                paths=["include/game/example.h"],
                owner=None,
                allow_unverified=True,
            )
            unproven = audit_promotion(
                promotion,
                base_ref=base,
                head_ref="HEAD",
                source_ref=source_commit,
                selected_paths=["include/game/example.h"],
                require_evidence=True,
            )
            self.assertFalse(unproven["clean_human_promotion"])
            self.assertTrue(
                any("--build-gate" in error for error in unproven["errors"])
            )
            self.assertTrue(
                any("--checksum" in error for error in unproven["errors"])
            )
            self.assertTrue(
                any(
                    "main:game/wrapped" in error
                    for error in unproven["errors"]
                )
            )
            proven = audit_promotion(
                promotion,
                base_ref=base,
                head_ref="HEAD",
                source_ref=source_commit,
                selected_paths=["include/game/example.h"],
                evidence={
                    "build_gate": "pass",
                    "checksum": "pass",
                    "consumers": {
                        "main:game/example": "exact",
                        "main:game/wrapped": "exact",
                    },
                },
                require_evidence=True,
            )
            self.assertTrue(proven["clean_human_promotion"])
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(promotion)],
                cwd=root,
                check=False,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            temporary.cleanup()

    def test_audit_rejects_undeclared_and_out_of_scope_paths(self):
        temporary, root, base = self.fixture()
        try:
            run(root, "git", "checkout", "-qb", "project/bad", base)
            (root / "src/game/example.c").write_text(
                "u32 Example(u32 seed)\n{\n    return seed + 1;\n}\n",
                encoding="utf-8",
            )
            run(root, "git", "add", "src/game/example.c")
            run(root, "git", "commit", "-qm", "Adjust source")
            result = audit_promotion(
                root,
                base_ref=base,
                head_ref="HEAD",
            )
            self.assertFalse(result["clean_human_promotion"])
            self.assertTrue(
                any("promote_recovered_c" in error for error in result["errors"])
            )
        finally:
            temporary.cleanup()


if __name__ == "__main__":
    unittest.main()
