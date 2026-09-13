"""Physical indexed-memory dependencies, without memory-value inference."""
import copy
import json
import os
from pathlib import Path
import unittest

from tools import recovery_causal_groups as groups
from tools.tests.test_recovery_causal_groups import report, row


def nodes(instructions, sites, **kwargs):
    rows = [row(text, i * 4) for i, text in enumerate(instructions)]
    return {node["row"]: node for node in groups.producer_slice(rows, sites, **kwargs)["nodes"]}


class IndexedProducerTests(unittest.TestCase):
    def test_load_families_preserve_base_and_define_only_destination(self):
        for op in ("lwzx", "lhzx", "lhax", "lbzx", "lfsx", "lfdx"):
            dest, move, consumer = ("f5", "fmr", "f6") if op.startswith("lf") else ("r5", "mr", "r6")
            with self.subTest(op=op):
                found = nodes(["li r3, 8", "li r4, 4", f"{op} {dest}, r3, r4",
                               "addi r7, r3, 1", f"{move} {consumer}, {dest}"], [2, 3, 4])
                load = found[2]
                self.assertEqual(load["defines"], dest)
                self.assertEqual(load["uses"], [{"register": "r3", "definition_row": 0},
                                                {"register": "r4", "definition_row": 1}])
                self.assertEqual(load["memory_root"]["base"], "r3")
                self.assertEqual(load["memory_root"]["index"], "r4")
                self.assertFalse(load["memory_root"]["zero_base"])
                self.assertEqual(load["memory_root"]["alias_identity"], "UNKNOWN")
                self.assertNotIn("base_update", load)
                self.assertEqual(found[3]["uses"][0]["definition_row"], 0)
                self.assertEqual(found[4]["uses"][0]["definition_row"], 2)

    def test_store_families_use_value_and_preserve_definitions(self):
        for op in ("stwx", "sthx", "stbx", "stfsx", "stfdx"):
            value, producer = ("f5", "lfs f5, 0(r3)") if op.startswith("stf") else ("r5", "li r5, 9")
            with self.subTest(op=op):
                found = nodes(["li r3, 8", "li r4, 4", producer, f"{op} {value}, r3, r4",
                               "add r7, r3, r4"], [3, 4])
                self.assertNotIn("defines", found[3])
                self.assertNotIn("base_update", found[3])
                self.assertEqual(found[3]["uses"], [{"register": value, "definition_row": 2},
                    {"register": "r3", "definition_row": 0}, {"register": "r4", "definition_row": 1}])
                self.assertEqual(found[3]["memory_root"]["alias_identity"], "UNKNOWN")
                self.assertEqual(found[4]["uses"], [{"register": "r3", "definition_row": 0},
                                                    {"register": "r4", "definition_row": 1}])

    def test_zero_base_is_literal_but_zero_index_is_register(self):
        found = nodes(["li r0, 4", "lwzx r5, r0, r0", "stwx r5, r0, r0"], [1, 2])
        self.assertEqual(found[1]["uses"], [{"register": "r0", "definition_row": 0}])
        self.assertTrue(found[1]["memory_root"]["zero_base"])
        self.assertEqual(found[2]["uses"], [{"register": "r5", "definition_row": 1},
                                            {"register": "r0", "definition_row": 0}])

    def test_overlapping_destination_uses_preload_definitions(self):
        for dest in ("r3", "r4"):
            found = nodes(["li r3, 8", "li r4, 4", f"lhax {dest}, r3, r4",
                           "add r7, r3, r4"], [2, 3])
            self.assertEqual(found[2]["uses"], [{"register": "r3", "definition_row": 0},
                                                {"register": "r4", "definition_row": 1}])
            self.assertEqual(found[3]["uses"], [
                {"register": "r3", "definition_row": 2 if dest == "r3" else 0},
                {"register": "r4", "definition_row": 2 if dest == "r4" else 1}])

    def test_malformed_and_update_forms_stay_unknown(self):
        malformed = ["lwzx r5, r3", "lwzx r5, r3, r4, r6", "lwzx f5, r3, r4",
                     "lfsx r5, r3, r4", "stfsx r5, r3, r4", "stwx f5, r3, r4",
                     "lhax r5, f3, r4", "lhzx r5, r3, f4", "lbzx r32, r3, r4",
                     "lfdx f32, r3, r4", "lwzx r5, r33, r4", "stwx r5, r3, 4",
                     "lwzx r5, r3, r4junk", "lwzx. r5, r3, r4"]
        malformed += [f"{op} {'f5' if op.startswith(('lf', 'stf')) else 'r5'}, r3, r4"
                      for op in ("lwzux", "lhzux", "lhaux", "lbzux", "lfsux", "lfdux",
                                 "stwux", "sthux", "stbux", "stfsux", "stfdux")]
        for instruction in malformed:
            with self.subTest(instruction=instruction):
                found = nodes(["li r3, 8", instruction, "mr r7, r3"], [1, 2])
                self.assertEqual(found[1]["status"], "UNKNOWN")
                self.assertNotIn("defines", found[1])
                self.assertNotIn("base_update", found[1])
                self.assertNotIn("definition_row", found[2]["uses"][0])

    def test_cfg_eabi_context_and_packet_sealing(self):
        instructions = ["li r14, 8", "li r15, 4", "bl helper", "lhzx r5, r14, r15",
                        "sthx r5, r14, r15", "blr"]
        rows = [row(text, i * 4) for i, text in enumerate(instructions)]
        default = nodes(instructions, [3])[3]
        self.assertTrue(all(use["status"] == "UNKNOWN" for use in default["uses"]))
        found = nodes(instructions, [3, 4], call_model="ppc-eabi", flow_model="unique-cfg")
        self.assertEqual(found[3]["uses"], [{"register": "r14", "definition_row": 0},
                                            {"register": "r15", "definition_row": 1}])
        self.assertNotIn("defines", found[4])
        packet = groups.decision_packet(report(rows, rows), "f", "void f(void) {}", 1, 1,
            "Where are the indexed operands produced?", 3, 4, producer_sites=[3, 4],
            producer_call_model="ppc-eabi", producer_flow_model="unique-cfg")
        groups.validate_decision_packet(packet)
        tampered = copy.deepcopy(packet)
        tampered["producer_context"]["target"]["nodes"][-1]["memory_root"]["index"] = "r16"
        with self.assertRaises(ValueError):
            groups.validate_decision_packet(tampered)

    def test_mtlr_preserves_gprs_but_not_call_or_malformed_boundaries(self):
        found = nodes(["li r3, 8", "li r12, 4", "mtlr r12", "lwzx r5, r3, r12"], [2, 3])
        self.assertNotIn("defines", found[2])
        self.assertEqual(found[2]["uses"], [{"register": "r12", "definition_row": 1}])
        self.assertEqual(found[3]["uses"], [{"register": "r3", "definition_row": 0},
                                            {"register": "r12", "definition_row": 1}])
        for boundary in ("mtlr f1", "mtlr r12, r3", "mtlr r32", "mtlr", "bl helper", "blrl"):
            with self.subTest(boundary=boundary):
                found = nodes(["li r3, 8", boundary, "mr r5, r3"], [1, 2])
                self.assertEqual(found[1]["status"], "UNKNOWN")
                self.assertNotIn("defines", found[1])
                self.assertNotIn("definition_row", found[2]["uses"][0])

    def test_optional_exev_read_only_acceptance(self):
        selected = os.environ.get("MP6_INDEXED_PRODUCER_FIXTURE")
        if not selected:
            self.skipTest("explicit local Exev report not selected")
        path = Path(selected)
        raw = path.read_bytes()
        document = json.loads(raw)
        for side in ("left", "right"):
            symbol = next(s for s in document[side]["symbols"] if s["name"] == "DynProgExtraEventsProcess")
            rows = symbol["instructions"]
            found = {n["row"]: n for n in groups.producer_slice(rows, [21, 22, 73, 82])["nodes"]}
            for site, op in ((21, "lhzx"), (22, "lhzx"), (73, "lhax"), (82, "sthx")):
                self.assertTrue(rows[site]["instruction"]["formatted"].startswith(op + " "))
                self.assertNotEqual(found[site].get("status"), "UNKNOWN")
                self.assertEqual(found[site]["memory_root"]["alias_identity"], "UNKNOWN")
            cfg = {n["row"]: n for n in groups.producer_slice(rows, [73, 82],
                call_model="ppc-eabi", flow_model="unique-cfg")["nodes"]}
            for site in (73, 82):
                base = cfg[site]["memory_root"]["base"]
                use = next(use for use in cfg[site]["uses"] if use["register"] == base)
                self.assertEqual(use.get("definition_row"), 13)
        self.assertEqual(raw, path.read_bytes())


if __name__ == "__main__":
    unittest.main()
