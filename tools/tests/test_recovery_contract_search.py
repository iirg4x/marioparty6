import json
import unittest
import tempfile
from pathlib import Path
from unittest import mock
from tools import recovery_contract_search as adapter
from tools import recovery_call_contract_repair as repair
from tools.tests import test_recovery_call_contract_repair as fixtures


class CompositionTests(unittest.TestCase):
    def test_ordered_one_cell_rebinds_unique_original_statements(self):
        source = b'int f(void) {\n int n;\n n = api();\n work->value = n;\n return n;\n}\n'
        statements = []
        for text in (b'n = api();', b'work->value = n;'):
            pos = source.index(text)
            statements.append(dict(original=text.decode(), start_byte=pos, end_byte=pos+len(text), sha256=repair._sha(text)))
        constraints = dict(source_sha256=repair._sha(source), local='n', statements=statements,
                           compatible_value_conversions=True, field_stability_reviewed=True)
        stages = [{'family': adapter.FAMILY, 'constraints': {'source_sha256': repair._sha(source)}},
                  {'family': 'sequence_result_consumer', 'constraints': constraints}]
        intermediate = source.replace(b' int n;', b' extern void other(void);\n int n;')
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            out = root/'build/composed'
            out.mkdir(parents=True)
            first = dict(candidate_source=intermediate, source_sha256=repair._sha(source),
                         candidate_sha256=repair._sha(intermediate), evidence=[])
            with mock.patch.object(adapter, 'generate_shape', return_value=[first]) as generate:
                result = adapter.generate_composed(root, root/'index', 'f', source, {'stages': stages}, out)
            generate.assert_called_once()
            self.assertEqual(len(result), 1)
            self.assertIn(b'work->value = (n = api(), n);', result[0]['candidate_source'])
            self.assertEqual(result[0]['source_sha256'], repair._sha(source))
            proof = json.loads((out/'composition.json').read_text())
            self.assertEqual(proof['intermediate_sha256'], repair._sha(intermediate))
            constraints['statements'][0]['start_byte'] += 1
            with self.assertRaises(ValueError):
                adapter.generate_composed(root, root/'index', 'f', source, {'stages': stages}, out)

    def test_reversed_or_extra_stages_rejected(self):
        for stages in ([], [{'family': 'sequence_result_consumer'}, {'family': adapter.FAMILY}], [{}]*3):
            with self.assertRaises(ValueError):
                adapter.generate_composed(Path('.'), Path('index'), 'f', b'x', {'stages': stages}, Path('build/x'))


class ContractSearchTests(unittest.TestCase):
    def setUp(self):
        fixtures.ContractRepairTests.setUp(self)

    def run_adapter(self, **changes):
        self.capture.write_text(json.dumps(self.doc))
        constraints = {"reviewed": True, "source_sha256": self.desc["sha256"],
                       "capture": {"path": str(self.capture), "sha256": repair.compiler.digest(self.capture)},
                       "contracts": self.contracts}
        constraints.update(changes)
        with mock.patch.object(repair.frontier, "verify"):
            return adapter.generate_shape(self.root, self.index, "f", constraints, self.root / "build/output")

    def test_real_generator_output_shape_and_evidence(self):
        result = self.run_adapter()
        self.assertEqual(len(result), 1)
        row = result[0]
        self.assertEqual(row["family"], adapter.FAMILY)
        expected = self.raw.replace(b"{\n", b"{\n    extern void A(int x);\n    extern void B(int x);\n", 1)
        self.assertEqual(row["candidate_source"], expected)
        self.assertEqual(row["candidate_sha256"], repair._sha(expected))
        self.assertTrue(row["evidence"])
        self.assertEqual(self.source.read_bytes(), self.raw)

    def test_correct_current_contracts_emit_nothing(self):
        for e in self.doc["events"]:
            e["actual_call_origin"]["normalized_expression_tree"]["tree"]["type"].update(native_kind=0, byte_width=0)
        with mock.patch.object(repair.compiler, "compile_candidate", create=True, side_effect=AssertionError("no compile")):
            self.assertEqual(self.run_adapter(), [])
        self.assertFalse((self.root / "build/output").exists())

    def test_stale_or_unreviewed_refused(self):
        for change in ({"source_sha256": "0"*64}, {"reviewed": False}):
            with self.assertRaises(ValueError):
                self.run_adapter(**change)

    def test_capture_mismatch_not_treated_as_no_discrepancy(self):
        self.doc["source"] = dict(self.desc, sha256="0"*64)
        with self.assertRaisesRegex(ValueError, "capture source"):
            self.run_adapter()


if __name__ == "__main__":
    unittest.main()
