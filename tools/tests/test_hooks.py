import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from tools import agent_queue, promote_recovered_c, promote_supporting_change
from tools.hooks import HookError, hook_status, install_hooks, uninstall_hooks


def run(root, *args):
    return subprocess.run(args, cwd=root, capture_output=True, text=True, check=True).stdout.strip()


class HookTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.temp = Path(temp.name)
        self.root = self.temp / "tool worktree"
        self.root.mkdir()
        for args in (("init", "-q", "-b", "main"), ("config", "user.name", "Test"),
                     ("config", "user.email", "test@example.com")):
            run(self.root, "git", *args)
        self.write("src/game/example.c", "int Example(void) { return 0; }\n")
        self.write("configure.py", 'Object(NonMatching, "game/example.c")\n')
        self.write("STATUS.md", "Initial status\n")
        self.commit(self.root, "Initial project")
        self.base = run(self.root, "git", "rev-parse", "HEAD")
        run(self.root, "git", "checkout", "-qb", "agent/lab")
        (self.root / "tools").mkdir()
        # Exercise real installed dispatcher, promotion and queue modules;
        # instrument only expensive AI check endpoints.
        for path in Path(__file__).resolve().parents[1].glob("*.py"):
            shutil.copy2(path, self.root / "tools" / path.name)
        recorder = ("import os, pathlib, sys\n"
                    "with pathlib.Path(os.environ['HOOK_CHECK_LOG']).open('a') as f:\n"
                    "    f.write(pathlib.Path(__file__).name + ' ' + ' '.join(sys.argv[1:]) + '\\n')\n")
        for name in ("agent.py", "claim_diff.py", "recovery_index.py", "knowledge_cards.py", "blind_recovery.py"):
            self.write("tools/" + name, recorder)
        self.write("tools/tests/test_instrumented.py", "import os, pathlib, unittest\n"
                   "class Check(unittest.TestCase):\n"
                   "    def test_full_suite(self):\n"
                   "        with pathlib.Path(os.environ['HOOK_CHECK_LOG']).open('a') as f:\n"
                   "            f.write('full suite\\n')\n")
        self.write("AI_WORKSPACE.md", "Private workspace\n")
        self.write("src/game/example.c", "int Example(void) { return 1; }\n")
        self.write("configure.py", 'Object(Matching, "game/example.c")\n')
        self.commit(self.root, "Recover example privately")
        self.source = run(self.root, "git", "rev-parse", "HEAD")
        task = {"id": "test", "owner": "main:game/example", "status": "ready",
                "change_class": "build-configuration", "shared_files": ["configure.py"],
                "verification": {"verified_commit": self.source, "public_gate": "pass",
                                 "consumers": {"main:game/example": "exact"}}}
        queue = agent_queue.queue_path(self.root)
        queue.parent.mkdir(parents=True, exist_ok=True)
        queue.write_text(json.dumps({"schema_version": 2, "tasks": [task], "resources": {}}), encoding="utf-8")
        self.clean = self.temp / "clean worktree"
        self.manifest = promote_recovered_c.create_promotion(
            self.root, base_ref="main", source_ref=self.source,
            paths=["src/game/example.c"], owner="main:game/example",
            branch="recovery/example", worktree=self.clean, title="Recover example", allow_unverified=False)
        self.head = self.manifest["promotion"]["commit"]
        self.paths = install_hooks(self.root)
        self.log = self.temp / "checks.log"
        self.env = dict(os.environ, MP6_PYTHON=sys.executable, HOOK_CHECK_LOG=str(self.log))

    def write(self, path, text, root=None):
        target = (root or self.root) / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text, encoding="utf-8")

    def commit(self, root, title):
        run(root, "git", "add", ".")
        run(root, "git", "-c", "core.hooksPath=", "commit", "-qm", title)

    def hook(self, kind="pre-push", lines=None, root=None):
        if lines is None:
            lines = self.ref(self.head, "recovery/example")
        path = self.temp / "hook-input"
        path.write_text(lines, encoding="utf-8")
        return subprocess.run(("git", "hook", "run", "--to-stdin=" + str(path), kind),
                              cwd=root or self.clean, env=self.env, capture_output=True, text=True)

    def ref(self, head, branch, base=None):
        return f"x {head} refs/heads/{branch} {base or '0' * 40}\n"

    def assertPass(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def assertRejected(self, result, text):
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn(text, result.stderr)

    def test_install_status_uninstall_and_clean_push(self):
        self.assertFalse((self.clean / "tools/agent.py").exists())
        self.assertEqual(set(hook_status(self.root).values()), {"managed"})
        self.assertPass(self.hook())
        self.assertFalse(self.log.exists())
        self.assertIn(self.root.as_posix(), self.paths[0].read_text(encoding="utf-8"))
        uninstall_hooks(self.root)
        self.assertEqual(set(hook_status(self.root).values()), {"missing"})

    def test_clean_precommit_sidecars_and_source_rejection(self):
        self.write("STATUS.md", "Recovered example\n", self.clean)
        run(self.clean, "git", "add", "STATUS.md")
        self.assertPass(self.hook("pre-commit", ""))
        self.write("src/game/example.c", "int Example(void) { return 2; }\n", self.clean)
        run(self.clean, "git", "add", "src/game/example.c")
        self.assertRejected(self.hook("pre-commit", ""), "fresh verified promotion")
        # Git partial commits supply a separate index. The real installed
        # wrapper must inspect that tree, not the rejected ordinary index.
        self.env["GIT_INDEX_FILE"] = str(self.temp / "partial-commit-index")
        subprocess.run(("git", "read-tree", "HEAD"), cwd=self.clean,
                       env=self.env, capture_output=True, text=True, check=True)
        self.assertPass(self.hook("pre-commit", ""))
        del self.env["GIT_INDEX_FILE"]
        self.assertRejected(self.hook("pre-commit", ""), "fresh verified promotion")
        self.assertFalse(self.log.exists())

    def test_leakage_mixed_unrecognized_and_deleted_refs(self):
        mixed = self.ref(self.head, "recovery/example") + self.ref(self.source, "agent/lab")
        self.assertRejected(self.hook(lines=mixed), "separately")
        self.assertRejected(self.hook(lines=self.ref(self.source, "recovery/example")), "AI workspace files")
        self.assertRejected(self.hook(lines=self.ref(self.head, "topic")), "unrecognized")
        self.assertRejected(self.hook(lines=f"x {self.head} refs/tags/public {'0' * 40}\n"), "unrecognized")
        self.assertPass(self.hook(lines=self.ref('0' * 40, "recovery/old", self.head)))
        self.assertRejected(self.hook(lines=self.ref('0' * 40, "main", self.base)), "deleting main")
        self.assertFalse(self.log.exists())

    def test_main_progress_required_and_clean_merge_accepted(self):
        self.assertRejected(self.hook(lines=self.ref(self.head, "main", self.base)), "STATUS.md")
        self.write("STATUS.md", "Recovered example\n", self.clean)
        self.commit(self.clean, "Update public status")
        updated = run(self.clean, "git", "rev-parse", "HEAD")
        run(self.root, "git", "checkout", "-qb", "merge-test", self.base)
        run(self.root, "git", "-c", "core.hooksPath=", "merge", "--no-ff", "-qm", "Merge recovery", updated)
        merge = run(self.root, "git", "rev-parse", "HEAD")
        run(self.root, "git", "checkout", "-q", "agent/lab")
        self.assertPass(self.hook(lines=self.ref(merge, "main", self.base)))
        self.assertFalse(self.log.exists())

    def test_project_staged_progress_and_descendant_push(self):
        project = self.temp / "project worktree"
        manifest = promote_supporting_change.create_promotion(
            self.root, base_ref="main", source_ref=self.source, paths=["configure.py"],
            owner="main:game/example", branch="project/example", worktree=project,
            title="Mark example matching", allow_unverified=False)
        head = manifest["promotion"]["commit"]
        self.assertRejected(self.hook(root=project, lines=self.ref(head, "project/example")), "STATUS.md")
        for path in ("STATUS.md", "progress.csv", "progress/GP6E01.json", "progress/all.json", "progress/dol.json"):
            self.write(path, "Updated\n", project)
        run(project, "git", "add", ".")
        self.assertPass(self.hook("pre-commit", "", root=project))
        self.commit(project, "Refresh public progress")
        head = run(project, "git", "rev-parse", "HEAD")
        self.assertPass(self.hook(root=project, lines=self.ref(head, "project/example")))
        self.assertFalse(self.log.exists())

    def test_missing_tool_root_and_manifest(self):
        (self.root / "tools/agent.py").unlink()
        self.assertRejected(self.hook(), "untrusted installed tooling root")
        self.write("tools/agent.py", "")
        self.env["MP6_TOOL_ROOT"] = str(self.clean)
        self.assertPass(self.hook())
        Path(self.manifest["local_manifest"]).unlink()
        self.assertRejected(self.hook(), "expected one exact promotion manifest")
        with self.assertRaises(HookError):
            install_hooks(self.clean)

    def test_stale_proof_policy_and_unrelated_malformed_manifest(self):
        self.write("build/promotion/unrelated/manifest.json", "{broken")
        self.assertPass(self.hook())
        self.write("config/recovery/exceptions.json", '{"exceptions": []}')
        self.assertRejected(self.hook(), "policy is not present")
        (self.root / "config/recovery/exceptions.json").unlink()
        queue = agent_queue.queue_path(self.root)
        data = json.loads(queue.read_text(encoding="utf-8"))
        data["tasks"][0]["verification"]["verified_commit"] = self.base
        queue.write_text(json.dumps(data), encoding="utf-8")
        self.assertRejected(self.hook(), "queue verification is bound")

    def test_ai_hooks_keep_all_checks_with_actual_root(self):
        self.assertPass(self.hook(root=self.root, lines=self.ref(self.source, "agent/lab")))
        text = self.log.read_text(encoding="utf-8")
        for expected in ("agent.py", "startup-check --no-sync", "recovery_index.py", "knowledge_cards.py", "blind_recovery.py", "full suite"):
            self.assertIn(expected, text)
        self.assertIn("--root " + str(self.root), text)
        self.assertPass(self.hook("pre-commit", "", root=self.root))
        self.assertIn("claim_diff.py --root " + str(self.root), self.log.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
