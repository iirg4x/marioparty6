import json
import subprocess
import tempfile
import unittest
from pathlib import Path

from tools.blind_recovery import (
    LEGACY,
    archive_run,
    audit_repository,
    compare_assembly,
    compare_source,
    freeze_candidate,
    prepare_run,
    replay_case,
    score_organicity,
    score_run,
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


class BlindRecoveryTests(unittest.TestCase):
    def test_organicity_distinguishes_organic_and_match_shaped_code(self):
        organic = """
static void copy_values(int *out, const int *in, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        out[i] = in[i];
    }
}
"""
        match_shaped = """
#pragma options align=power
static void fn_80001234(int *out, volatile int *in)
{
#if 0
    out[0] = 2;
#endif
    register int unk_10 = (int)(short)*in;
    out[0] = unk_10;
}
"""
        self.assertEqual(score_organicity(organic).score, 100)
        result = score_organicity(match_shaped)
        self.assertLess(result.score, 60)
        self.assertTrue(result.human_review_required)

    def test_redundant_post_loop_condition_is_flagged(self):
        source = """
static void flush(Block *block)
{
    while (block->queued != 0) {
        emit(block);
        block->queued--;
    }
    if (block->queued == 0) {
        block->active = 0;
    }
}
"""
        result = score_organicity(source)
        self.assertIn(
            "guaranteed-post-loop-condition",
            {item.id for item in result.findings},
        )

    def test_source_and_assembly_are_separate_dimensions(self):
        reference = "int f(int x) { return x + 1; }\n"
        candidate = "int f(int x)\n{\n    return x + 1;\n}\n"
        source = compare_source(reference, candidate)
        self.assertTrue(source.exact_tokens)
        assembly = compare_assembly(
            "addi 3,3,1\nblr\n",
            "addi 3,3,1\nblr\n",
        )
        self.assertTrue(assembly.exact)

    def test_prepare_freeze_score_archive_and_replay(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            (root / "src").mkdir()
            source = root / "src/test.c"
            source.write_text(
                "static int add_one(int value)\n"
                "{\n"
                "    return value + 1;\n"
                "}\n",
                encoding="utf-8",
            )
            evidence = root / "evidence.md"
            evidence.write_text(
                "Signature and target assembly only.\n",
                encoding="utf-8",
            )
            target = root / "target.s"
            target.write_text("addi 3,3,1\nblr\n", encoding="utf-8")
            run(root, "git", "add", ".")
            run(root, "git", "commit", "-qm", "fixture")

            run_dir = root / "build/blind-recovery/test"
            prepare_run(
                root,
                trial_id="add-one",
                source_path="src/test.c",
                symbol="add_one",
                evidence_path=evidence,
                target_assembly_path=target,
                run_dir=run_dir,
            )
            candidate = root / "candidate.c"
            candidate.write_text(
                "static int add_one(int value) { return value + 1; }\n",
                encoding="utf-8",
            )
            freeze_candidate(run_dir, candidate)
            candidate_assembly = root / "candidate.s"
            candidate_assembly.write_text(
                "addi 3,3,1\nblr\n",
                encoding="utf-8",
            )
            (run_dir / "candidate.s").write_text(
                candidate_assembly.read_text(encoding="utf-8"),
                encoding="utf-8",
            )
            result = score_run(
                run_dir,
                candidate_assembly_path=candidate_assembly,
            )
            self.assertTrue(result["source"]["exact_tokens"])
            self.assertTrue(result["assembly"]["exact"])
            self.assertEqual(
                result["organicity"]["candidate"]["score"],
                100,
            )

            case_dir = root / "benchmarks/blind_recovery/cases/add-one"
            case = archive_run(root, run_dir, case_dir)
            manifest = root / "benchmarks/blind_recovery/manifest.json"
            manifest.parent.mkdir(parents=True, exist_ok=True)
            manifest.write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "cases": ["cases/add-one/case.json"],
                    }
                ),
                encoding="utf-8",
            )
            self.assertEqual(replay_case(root, case), [])
            audit = audit_repository(
                root,
                Path("benchmarks/blind_recovery/manifest.json"),
                replay=True,
            )
            self.assertEqual(audit["errors"], [])
            self.assertEqual(audit["warnings"], [])

    def test_legacy_cases_are_visible_warnings(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            run(root, "git", "init", "-q", "-b", "main")
            run(root, "git", "config", "user.email", "test@example.com")
            run(root, "git", "config", "user.name", "Test")
            case_dir = root / "benchmarks/blind_recovery/cases/legacy"
            case_dir.mkdir(parents=True)
            (root / "src.c").write_text(
                "int f(void) { return 1; }\n",
                encoding="utf-8",
            )
            run(root, "git", "add", "src.c")
            run(root, "git", "commit", "-qm", "fixture")
            commit = run(root, "git", "rev-parse", "HEAD")
            case = {
                "schema_version": 1,
                "id": "legacy",
                "status": LEGACY,
                "source": "src.c",
                "function": "f",
                "source_commit": commit,
                "reference_sha256": "unavailable-legacy",
                "artifacts": {},
            }
            (case_dir / "case.json").write_text(
                json.dumps(case),
                encoding="utf-8",
            )
            manifest = root / "benchmarks/blind_recovery/manifest.json"
            manifest.write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "cases": ["cases/legacy/case.json"],
                    }
                ),
                encoding="utf-8",
            )
            audit = audit_repository(root)
            self.assertEqual(audit["errors"], [])
            self.assertEqual(len(audit["warnings"]), 1)


if __name__ == "__main__":
    unittest.main()
