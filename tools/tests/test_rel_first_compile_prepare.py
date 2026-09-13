"""Synthetic preparation checks; no real compiler or link inputs touched."""
import importlib
import io
from contextlib import redirect_stderr
import json
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from tools import rel_first_compile_prepare as prepare


class PrepareTests(unittest.TestCase):
    def test_import_is_read_only(self):
        with patch.object(prepare.subprocess, 'run') as run, \
                patch.object(Path, 'mkdir') as mkdir:
            importlib.reload(prepare)
        run.assert_not_called()
        mkdir.assert_not_called()

    def test_provider_selected_macros_and_original_flags(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)/'root'
            proof, current, output = root/'proof', root/'current', root/'new-batch'
            def put(path, text):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text)
                return path
            put(root/'configure.py', '# config')
            put(current/'build/GP6E01/config.json', json.dumps({'modules': []}))
            put(current/'build.ninja', '')
            flags = ['-proc', 'gekko', '-char', 'unsigned', '-i', str(root/'include')]
            flags_json = put(root/'flags.json', json.dumps({'flags': flags}))
            m2c = put(root/'selected-tool/m2c.py', '# selected')
            macro = put(m2c.parent/'m2c_macros.h', '#define M2C_UNK int')
            calls = []
            def preprocess(argv, **kwargs):
                calls.append(argv)
                source = Path(argv[argv.index('-EP')+1])
                self.assertIn('Math/fdlibm.h', source.read_text())
                self.assertEqual(argv[2:2+len(flags)], flags)
                self.assertEqual(kwargs['cwd'], proof.resolve())
                Path(argv[argv.index('-o')+1]).write_text('extern double exp(double);')
                return SimpleNamespace(returncode=0, stdout='', stderr='')
            with patch.object(prepare.subprocess, 'run', side_effect=preprocess), \
                    patch.object(prepare.subprocess, 'check_output', return_value='commit\n'):
                result = prepare.prepare(root=root, proof_root=proof, current=current,
                    flags_json=flags_json, m2c=m2c, output=output)
            self.assertEqual(len(calls), 1)
            manifest = json.loads(Path(result['manifest']).read_text())
            self.assertEqual(manifest['flags'], flags)
            self.assertEqual(manifest['m2c'], str(m2c.resolve()))
            self.assertEqual(manifest['selection']['module_count'], 0)
            prefix = Path(manifest['prefix_path']).read_text()
            self.assertIn(macro.as_posix(), prefix)
            self.assertIn('Math/fdlibm.h', prefix)
            before = {p: p.read_bytes() for p in output.rglob('*') if p.is_file()}
            with patch.object(prepare.subprocess, 'run') as run:
                with self.assertRaisesRegex(ValueError, 'output already exists'):
                    prepare.prepare(root=root, proof_root=proof, current=current,
                        flags_json=flags_json, m2c=m2c, output=output)
            run.assert_not_called()
            self.assertEqual(before, {p: p.read_bytes() for p in output.rglob('*') if p.is_file()})

    def test_cli_requires_explicit_inputs(self):
        with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            prepare.main([])

    def test_selected_unit_filter_and_private_context_are_preserved(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            current = root/'current'
            def put(path, text):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text)
                return path
            put(root/'configure.py', '# config')
            put(root/'include/REL/example.h', 'void fn_1_A0(void);')
            put(current/'build/GP6E01/config.json', json.dumps({'modules': [
                {'name': 'example', 'units': [{'object': 'orig.o', 'code_size': 4},
                                             {'object': 'selected.o', 'code_size': 4}]}]}))
            put(current/'build.ninja', 'build build/GP6E01/example/example.plf: link orig.o | implicit\n')
            put(current/'build/GP6E01/example/example.plf', 'target')
            flags = put(root/'flags.json', '{"flags": []}')
            m2c = put(root/'translator/m2c.py', '')
            put(m2c.parent/'m2c_macros.h', '')
            symbols = [{'name': name, 'info': 2, 'section': 1, 'size': 4, 'value': 0}
                       for name in ('fn_1_A0', '_prolog', '__runtime')]
            def run(argv, **kwargs):
                if '-EP' in argv:
                    Path(argv[-1]).write_text('void fn_1_A0(void);')
                else:
                    Path(argv[-1]).write_text('.fn fn_1_A0, global\n')
                return SimpleNamespace(returncode=0, stdout='', stderr='')
            with patch.object(prepare, '_parse_elf_structure', return_value={'symbols': symbols, 'sections': [{}, {}]}) as parse, \
                    patch.object(prepare.subprocess, 'run', side_effect=run), \
                    patch.object(prepare.subprocess, 'check_output', return_value='commit\n'):
                result = prepare.prepare(root=root, proof_root=root/'proof', current=current,
                    flags_json=flags, m2c=m2c, output=root/'new')
            self.assertFalse(any(call.args[0].name == 'selected.o' for call in parse.call_args_list))
            module = json.loads(Path(result['manifest']).read_text())['modules'][0]
            self.assertEqual(module['functions'], ['fn_1_A0'])
            self.assertEqual(module['unselected_code_units'], 1)
            self.assertIn('REL/example.h', Path(module['prefix_path']).read_text())
            self.assertIn('Math/fdlibm.h', Path(module['prefix_path']).read_text())
