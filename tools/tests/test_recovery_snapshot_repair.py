import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tools import recovery_snapshot_repair as repair


class SnapshotRepairTests(unittest.TestCase):
    def setUp(self):
        self.sites = [{"source_local": f"s{i}", "result_local": f"r{i}", "player_local": f"p{i}"}
                      for i in range(3)]
        self.source = '''void f(void) {
    s16 s0; s16 r0; s16 s1; s16 r1; s16 s2; s16 r2;
    int p0, p1, p2;
    p0 = player; s0 = field; r0 = s0;
    if (r0 <= 0) { use(r0); }
    p1 = player; s1 = field; r1 = s1;
    if (r1 <= 0) {
        /* ignored brace } and s2 identifier */
        p2 = player; s2 = field; r2 = s2;
        if (r2 <= 0) { log("}"); use(r2); }
        use(r1);
    }
}
'''

    def test_three_sites_including_nested_body_and_comments(self):
        candidate, edits = repair.repair(self.source, "f", self.sites)
        self.assertEqual(len(edits), 15)
        for i in range(3):
            self.assertIn(f"s16 s{i} = field; s16 r{i} = s{i};", candidate)
        self.assertIn('log("}"); use(r2);', candidate)
        self.assertIn("/* ignored brace } and s2 identifier */", candidate)
        self.assertEqual(candidate.count("use("), self.source.count("use("))
        self.assertEqual(self.source.count("player"), candidate.count("player"))

    def test_post_scope_consumer_refused(self):
        text = self.source.replace("use(r1);", "use(r0);")
        with self.assertRaisesRegex(ValueError, "outside proposed scope"):
            repair.repair(text, "f", self.sites)

    def test_duplicate_or_colliding_names_refused(self):
        sites = copy.deepcopy(self.sites)
        sites[1]["source_local"] = "s0"
        with self.assertRaisesRegex(ValueError, "colliding"):
            repair.repair(self.source, "f", sites)

    def test_mixed_types_or_wrong_graph_refused(self):
        for source in (self.source.replace("s16 r0;", "u16 r0;"),
                       self.source.replace("r0 = s0;", "r0 = field;")):
            with self.assertRaises(ValueError):
                repair.repair(source, "f", self.sites)

    def test_if_else_refused(self):
        source = self.source.replace("{ use(r0); }", "{ use(r0); } else { use(r0); }")
        with self.assertRaisesRegex(ValueError, "if/else"):
            repair.repair(source, "f", self.sites)

    def test_generate_binding_immutable_and_review_flag(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "build").mkdir()
            source = root / "source.c"
            source.write_text(self.source, encoding="utf-8")
            desc = repair.frontier.read_bound(root, source, repair.LIMIT)[1]
            index = root / "index.json"
            index.write_text(json.dumps({"inputs": {"source": desc}, "functions": [{"function": "f"}]}))
            args = dict(root=root, index=index, function="f", sites=self.sites, out_dir=Path("build/output"))
            with mock.patch.object(repair.frontier, "verify"):
                result = repair.generate(**args)
                self.assertFalse(result["root_reviewed"])
                self.assertEqual(source.read_text(), self.source)
                with self.assertRaisesRegex(ValueError, "already exists"):
                    repair.generate(**args)
                source.write_text(self.source + "\n")
                args["out_dir"] = Path("build/stale")
                with self.assertRaisesRegex(ValueError, "stale"):
                    repair.generate(**args)


if __name__ == "__main__":
    unittest.main()
