import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.promote_recovered_c import (
    PromotionError,
    _normalise_path,
    audit_promotion,
    branch_errors,
    create_promotion,
    plan_promotion,
    source_ai_markers,
    source_quality_errors,
    synthetic_rel_source,
)


def run(cwd: Path, *args: str) -> str:
    result = subprocess.run(
        args,
        cwd=cwd,
        check=True,
        text=True,
        capture_output=True,
    )
    return result.stdout.strip()


class PromotionTests(unittest.TestCase):
    def fixture(self) -> tuple[tempfile.TemporaryDirectory[str], Path, str]:
        temporary = tempfile.TemporaryDirectory()
        root = Path(temporary.name) / "repo"
        root.mkdir()
        run(root, "git", "init", "-q", "-b", "main")
        run(root, "git", "config", "user.email", "test@example.com")
        run(root, "git", "config", "user.name", "Test")
        (root / "src/game").mkdir(parents=True)
        (root / "tools").mkdir()
        (root / "src/game/example.c").write_text(
            "int Example(void)\n{\n    return 0;\n}\n",
            encoding="utf-8",
        )
        (root / "README.md").write_text("Human project\n", encoding="utf-8")
        run(root, "git", "add", ".")
        run(root, "git", "commit", "-qm", "Initial source")
        base = run(root, "git", "rev-parse", "HEAD")
        run(root, "git", "checkout", "-qb", "agent/recovery-lab")
        (root / "src/game/example.c").write_text(
            "int Example(void)\n{\n    return 1;\n}\n",
            encoding="utf-8",
        )
        (root / "tools/lab.py").write_text("print('tooling')\n", encoding="utf-8")
        run(root, "git", "add", ".")
        run(root, "git", "commit", "-qm", "Recover example with internal tooling")
        return temporary, root, base

    def test_normalise_accepts_only_canonical_recovered_source_and_header_suffixes(self):
        self.assertEqual(
            _normalise_path(r"  src\game\example.c  "),
            "src/game/example.c",
        )
        for suffix in (".cp", ".cpp"):
            with self.subTest(suffix=suffix):
                self.assertEqual(
                    _normalise_path(f"src\\game\\example{suffix}"),
                    f"src/game/example{suffix}",
                )
        for path in (
            "include/game/example.h",
            "include/game/example.hpp",
            "src/game/example.h",
            "src/game/example.hpp",
        ):
            with self.subTest(path=path):
                self.assertEqual(_normalise_path(path), path)
        for path in (
            "src/game/example.cc",
            "src/game/example.cxx",
            "src/game/example.CP",
            "src/game/example.s",
            "include/game/example.cp",
            "include/game/example.cpp",
            "src/game/example.inc",
            "tools/example.py",
        ):
            with self.subTest(path=path):
                with self.assertRaisesRegex(
                    PromotionError,
                    r"automatic promotion accepts only",
                ):
                    _normalise_path(path)

    def test_plan_selects_only_recovered_c(self):
        temporary, root, base = self.fixture()
        try:
            value = plan_promotion(
                root,
                base_ref=base,
                source_ref="HEAD",
                allow_unverified=True,
            )
            self.assertEqual(
                [item["path"] for item in value["files"]],
                ["src/game/example.c"],
            )
        finally:
            temporary.cleanup()

    def test_plan_rejects_unchanged_source_and_header_paths(self):
        temporary, root, base = self.fixture()
        try:
            (root / "include/game").mkdir(parents=True)
            (root / "include/game/unchanged.h").write_text(
                "int Unchanged(void);\n", encoding="utf-8"
            )
            run(root, "git", "add", "include/game/unchanged.h")
            run(root, "git", "commit", "-qm", "Add unchanged header")
            same = run(root, "git", "rev-parse", "HEAD")
            with self.assertRaisesRegex(PromotionError, "identical to base"):
                plan_promotion(
                    root,
                    base_ref=same,
                    source_ref=same,
                    paths=["src/game/example.c", "include/game/unchanged.h"],
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_plan_auto_selects_canonical_sources_and_headers(self):
        temporary, root, base = self.fixture()
        try:
            for suffix in (".cp", ".cpp"):
                (root / f"src/game/example{suffix}").write_text(
                    "int Example(void) { return 1; }\n",
                    encoding="utf-8",
                )
            # Auto-selection must include canonical headers while ignoring
            # unrelated support and unsupported source suffixes.
            (root / "src/game/example.cc").write_text(
                "int Example(void) { return 1; }\n", encoding="utf-8"
            )
            (root / "include/game/example.h").parent.mkdir(parents=True)
            (root / "include/game/example.h").write_text(
                "int Example(void);\n", encoding="utf-8"
            )
            (root / "include/game/example.hpp").write_text(
                "int Example(void);\n", encoding="utf-8"
            )
            for suffix in (".h", ".hpp"):
                (root / f"src/game/example{suffix}").write_text(
                    "int Example(void);\n", encoding="utf-8"
                )
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "Add canonical source suffixes")
            value = plan_promotion(
                root,
                base_ref=base,
                source_ref="HEAD",
                allow_unverified=True,
            )
            self.assertEqual(
                [item["path"] for item in value["files"]],
                [
                    "include/game/example.h",
                    "include/game/example.hpp",
                    "src/game/example.c",
                    "src/game/example.cp",
                    "src/game/example.cpp",
                    "src/game/example.h",
                    "src/game/example.hpp",
                ],
            )
            explicit_headers = plan_promotion(
                root,
                base_ref=base,
                source_ref="HEAD",
                paths=["src/game/example.h", "src/game/example.hpp"],
                allow_unverified=True,
            )
            self.assertEqual(
                [item["path"] for item in explicit_headers["files"]],
                ["src/game/example.h", "src/game/example.hpp"],
            )
            with self.assertRaisesRegex(PromotionError, "README.md"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["src/game/example.cp", "README.md"],
                    allow_unverified=True,
                )
            with self.assertRaisesRegex(PromotionError, r"automatic promotion accepts only"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["src/game/example.cc"],
                    allow_unverified=True,
                )
            for unsupported in ("configure.py", "asm/example.s"):
                with self.subTest(unsupported=unsupported):
                    with self.assertRaisesRegex(PromotionError, unsupported):
                        plan_promotion(
                            root,
                            base_ref=base,
                            source_ref="HEAD",
                            paths=["include/game/example.h", unsupported],
                            allow_unverified=True,
                        )
        finally:
            temporary.cleanup()

    def test_audit_accepts_canonical_sources_and_headers(self):
        temporary, root, base = self.fixture()
        try:
            run(root, "git", "checkout", "-qb", "recovery/canonical", base)
            for suffix in (".cp", ".cpp"):
                (root / f"src/game/example{suffix}").write_text(
                    "int Example(void) { return 1; }\n",
                    encoding="utf-8",
                )
            (root / "include/game").mkdir(parents=True)
            for suffix in (".h", ".hpp"):
                (root / f"include/game/example{suffix}").write_text(
                    "int Example(void);\n",
                    encoding="utf-8",
                )
            for suffix in (".h", ".hpp"):
                (root / f"src/game/example{suffix}").write_text(
                    "int Example(void);\n", encoding="utf-8"
                )
            run(
                root,
                "git",
                "add",
                "src/game/example.cp",
                "src/game/example.cpp",
                "src/game/example.h",
                "src/game/example.hpp",
                "include/game/example.h",
                "include/game/example.hpp",
            )
            run(root, "git", "commit", "-qm", "Recover canonical source")
            result = audit_promotion(
                root,
                base_ref=base,
                head_ref="HEAD",
                selected_paths=[
                    "src/game/example.cp",
                    "src/game/example.cpp",
                    "src/game/example.h",
                    "src/game/example.hpp",
                    "include/game/example.h",
                    "include/game/example.hpp",
                ],
            )
            self.assertTrue(result["clean_human_promotion"])
            self.assertEqual(
                [item["path"] for item in result["files"]],
                [
                    "include/game/example.h",
                    "include/game/example.hpp",
                    "src/game/example.cp",
                    "src/game/example.cpp",
                    "src/game/example.h",
                    "src/game/example.hpp",
                ],
            )
        finally:
            temporary.cleanup()

    def test_plan_selects_requested_commit_for_repeated_owner(self):
        temporary, root, base = self.fixture()
        try:
            source_commit = run(root, "git", "rev-parse", "HEAD")
            owner = "REL:example:application"
            queue_path = root / ".git/agent-coordination/queue.json"
            queue_path.parent.mkdir(parents=True, exist_ok=True)
            queue_path.write_text(
                json.dumps(
                    {
                        "schema_version": 2,
                        "tasks": [
                            {
                                "id": "older-pass",
                                "owner": owner,
                                "status": "done",
                                "verification": {
                                    "public_gate": "pass",
                                    "verified_commit": base,
                                },
                            },
                            {
                                "id": "current-pass",
                                "owner": owner,
                                "status": "done",
                                "verification": {
                                    "public_gate": "pass",
                                    "verified_commit": source_commit,
                                },
                            },
                        ],
                        "resources": {},
                    }
                ),
                encoding="utf-8",
            )
            value = plan_promotion(
                root,
                base_ref=base,
                source_ref=source_commit,
                paths=["src/game/example.c"],
                owner=owner,
            )
            self.assertEqual(
                value["queue_proof"]["verified_commit"], source_commit
            )
        finally:
            temporary.cleanup()

    def test_noncanonical_support_paths_are_not_promotable(self):
        temporary, root, base = self.fixture()
        try:
            with self.assertRaisesRegex(PromotionError, "automatic promotion accepts only"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    paths=["include/game/example.cpp"],
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_ai_attribution_in_comment_is_rejected(self):
        self.assertTrue(
            source_ai_markers("int f(void) { return 0; } // generated by Claude\n")
        )
        temporary, root, base = self.fixture()
        try:
            (root / "src/game/example.c").write_text(
                "// AI-generated recovery\nint Example(void) { return 1; }\n",
                encoding="utf-8",
            )
            run(root, "git", "add", "src/game/example.c")
            run(root, "git", "commit", "-qm", "Update source")
            with self.assertRaisesRegex(PromotionError, "AI-generated"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_full_file_quality_rejects_historical_raw_hex(self):
        temporary, root, base = self.fixture()
        try:
            (root / "src/game/example.c").write_text(
                "int Existing(void) { return 0x2A; }\n"
                "int Example(void) { return 1; }\n",
                encoding="utf-8",
            )
            run(root, "git", "add", "src/game/example.c")
            run(root, "git", "commit", "-qm", "Update another function")
            with self.assertRaisesRegex(PromotionError, "raw_hex_literal"):
                plan_promotion(
                    root,
                    base_ref=base,
                    source_ref="HEAD",
                    allow_unverified=True,
                )
        finally:
            temporary.cleanup()

    def test_quality_scan_ignores_comments_but_rejects_controls(self):
        temporary, root, _ = self.fixture()
        try:
            findings = source_quality_errors(
                root,
                "src/game/example.c",
                "// 0x2A volatile\nvolatile int value;\n",
            )
            self.assertEqual(len(findings), 1)
            self.assertIn("volatile", findings[0])
        finally:
            temporary.cleanup()

    def test_header_guards_allow_matching_leading_guard_only(self):
        temporary, root, _ = self.fixture()
        try:
            for suffix in (".h", ".hpp"):
                macro = "_EXAMPLE_H" if suffix == ".h" else "_EXAMPLE_HPP"
                text = (
                    f"#ifndef {macro}\n"
                    f"#define {macro}\n"
                    "int Example(void);\n"
                    "#endif\n"
                )
                self.assertFalse(
                    any(
                        "include_guard_override" in finding
                        for finding in source_quality_errors(
                            root, f"include/game/example{suffix}", text
                        )
                    )
                )
            self.assertTrue(
                any(
                    "include_guard_override" in finding
                    for finding in source_quality_errors(
                        root,
                        "src/game/example.c",
                        "#ifndef _EXAMPLE_H\n"
                        "#define _EXAMPLE_H\n"
                        "int Example(void);\n"
                        "#endif\n",
                    )
                )
            )
            foreign = (
                "#ifndef _EXAMPLE_H\n"
                "#define _EXAMPLE_H\n"
                "#define _FOREIGN_H\n"
                "#endif\n"
            )
            self.assertTrue(
                any(
                    "include_guard_override" in finding
                    for finding in source_quality_errors(
                        root, "include/game/example.h", foreign
                    )
                )
            )
            foreign_leading = (
                "#ifndef _OTHER_H\n"
                "#define _OTHER_H\n"
                "int Example(void);\n"
                "#endif\n"
            )
            self.assertTrue(
                any(
                    "include_guard_override" in finding
                    for finding in source_quality_errors(
                        root, "include/game/example.h", foreign_leading
                    )
                )
            )
            self.assertTrue(
                any(
                    "include_guard_override" in finding
                    for finding in source_quality_errors(
                        root,
                        "include/game/alloc.h",
                        "#ifndef _NOTALLOC_H\n"
                        "#define _NOTALLOC_H\n"
                        "int Example(void);\n"
                        "#endif\n",
                    )
                )
            )
            foreign_hpp = (
                "#ifndef _OTHER_HPP\n"
                "#define _OTHER_HPP\n"
                "int Example(void);\n"
                "#endif\n"
            )
            self.assertTrue(
                any(
                    "include_guard_override" in finding
                    for finding in source_quality_errors(
                        root, "include/game/example.hpp", foreign_hpp
                    )
                )
            )
            self.assertTrue(
                any(
                    "include_guard_override" in finding
                    for finding in source_quality_errors(
                        root, "src/game/example.c", foreign
                    )
                )
            )
        finally:
            temporary.cleanup()

    def test_synthetic_rel_shard_paths_are_rejected(self):
        self.assertTrue(synthetic_rel_source("src/REL/staffdll/application_pass5_0000.c"))
        self.assertTrue(synthetic_rel_source("src/REL/mdsingdll/application_3116c.c"))
        self.assertTrue(synthetic_rel_source("src/REL/mdsingdll/application_3116.hpp"))
        self.assertTrue(synthetic_rel_source("src/REL/mdsingdll/mdsing_tail8.c"))
        self.assertFalse(synthetic_rel_source("src/REL/staffdll/staff.c"))

    def test_create_starts_from_main_and_copies_exact_blob(self):
        temporary, root, base = self.fixture()
        promotion = Path(temporary.name) / "promotion"
        try:
            source_commit = run(root, "git", "rev-parse", "HEAD")
            hook = root / ".git/hooks/pre-commit"
            hook.write_text("#!/bin/sh\nexit 99\n", encoding="utf-8")
            hook.chmod(0o755)
            result = create_promotion(
                root,
                base_ref=base,
                source_ref=source_commit,
                branch="recovery/example",
                worktree=promotion,
                title="Recover example processing",
                paths=["src/game/example.c"],
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
            self.assertEqual(changed, ["src/game/example.c"])
            self.assertFalse((promotion / "tools/lab.py").exists())
            self.assertTrue(result["promotion"]["audit"]["clean_human_promotion"])
            source_blob = run(root, "git", "rev-parse", f"{source_commit}:src/game/example.c")
            promoted_blob = run(promotion, "git", "rev-parse", "HEAD:src/game/example.c")
            self.assertEqual(source_blob, promoted_blob)
            self.assertEqual(run(promotion, "git", "rev-parse", "HEAD^"), base)
            message = run(promotion, "git", "log", "-1", "--format=%B")
            self.assertEqual(message, "Recover example processing")
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(promotion)],
                cwd=root,
                check=False,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            temporary.cleanup()

    def test_create_copies_canonical_headers_and_exact_blobs(self):
        temporary, root, base = self.fixture()
        promotion = Path(temporary.name) / "header-promotion"
        try:
            (root / "include/game").mkdir(parents=True)
            for suffix in (".h", ".hpp"):
                (root / f"include/game/example{suffix}").write_text(
                    "int Example(void);\n",
                    encoding="utf-8",
                )
                (root / f"src/game/example{suffix}").write_text(
                    "int Example(void);\n", encoding="utf-8"
                )
            run(
                root,
                "git",
                "add",
                "include/game/example.h",
                "include/game/example.hpp",
                "src/game/example.h",
                "src/game/example.hpp",
            )
            run(root, "git", "commit", "-qm", "Add reviewed headers")
            source_commit = run(root, "git", "rev-parse", "HEAD")
            result = create_promotion(
                root,
                base_ref=base,
                source_ref=source_commit,
                branch="recovery/example-headers",
                worktree=promotion,
                title="Recover example headers",
                paths=[
                    "include/game/example.h",
                    "include/game/example.hpp",
                    "src/game/example.h",
                    "src/game/example.hpp",
                ],
                owner=None,
                allow_unverified=True,
            )
            self.assertTrue(result["promotion"]["audit"]["clean_human_promotion"])
            self.assertEqual(
                run(
                    promotion,
                    "git",
                    "diff",
                    "--name-only",
                    f"{base}...HEAD",
                ).splitlines(),
                [
                    "include/game/example.h",
                    "include/game/example.hpp",
                    "src/game/example.h",
                    "src/game/example.hpp",
                ],
            )
            for prefix in ("include/game/example", "src/game/example"):
                for suffix in (".h", ".hpp"):
                    self.assertEqual(
                        run(root, "git", "rev-parse", f"{source_commit}:{prefix}{suffix}"),
                        run(promotion, "git", "rev-parse", f"HEAD:{prefix}{suffix}"),
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

    def test_create_uses_authenticated_policy_from_ai_workspace(self):
        temporary, root, base = self.fixture()
        promotion = Path(temporary.name) / "promotion"
        try:
            (root / "src/game/example.c").write_text(
                "int Example(void)\n{\n    int value = 1;\n    value = value;\n    return value;\n}\n",
                encoding="utf-8",
            )
            exceptions = root / "config/recovery/exceptions.json"
            exceptions.parent.mkdir(parents=True)
            exceptions.write_text(
                json.dumps(
                    {
                        "exceptions": [
                            {
                                "classification": "authenticated",
                                "path": "src/game/example.c",
                                "rules": ["self_assignment"],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )
            run(root, "git", "add", "src/game/example.c", "config/recovery/exceptions.json")
            run(root, "git", "commit", "-qm", "Authenticate source shape")
            source_commit = run(root, "git", "rev-parse", "HEAD")

            result = create_promotion(
                root,
                base_ref=base,
                source_ref=source_commit,
                branch="recovery/example-policy",
                worktree=promotion,
                title="Recover example processing",
                paths=["src/game/example.c"],
                owner=None,
                allow_unverified=True,
            )

            self.assertTrue(result["promotion"]["audit"]["clean_human_promotion"])
            self.assertFalse((promotion / "config/recovery/exceptions.json").exists())
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
            "recovery/agent-fix",
            "recovery/ai-fix",
            "recovery/agent_fix",
            "recovery/fix-by-ai",
            "recovery/claude-frand",
        ):
            self.assertTrue(
                any("AI/agent" in error for error in branch_errors(branch)),
                branch,
            )
        # "ai"/"agent" embedded inside larger words are not attribution.
        self.assertEqual(branch_errors("recovery/maintain-repair"), [])
        self.assertEqual(branch_errors("recovery/example"), [])

    def test_audit_rejects_non_c_and_ai_commit_messages(self):
        temporary, root, base = self.fixture()
        try:
            run(root, "git", "checkout", "-qb", "recovery/bad", base)
            (root / "README.md").write_text("changed\n", encoding="utf-8")
            run(root, "git", "add", "README.md")
            run(root, "git", "commit", "-qm", "AI assisted cleanup")
            result = audit_promotion(
                root,
                base_ref=base,
                head_ref="HEAD",
            )
            self.assertFalse(result["clean_human_promotion"])
            self.assertTrue(any("README.md" in error for error in result["errors"]))
            self.assertTrue(any("attribution" in error for error in result["errors"]))
        finally:
            temporary.cleanup()


if __name__ == "__main__":
    unittest.main()
