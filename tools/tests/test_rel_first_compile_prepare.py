"""Synthetic preparation checks; no real compiler or link inputs touched."""
import importlib
import io
from contextlib import redirect_stderr
import json
from pathlib import Path
import tempfile
import sys
import subprocess
from types import SimpleNamespace
import unittest
from unittest.mock import patch

if __package__ in {None, ''}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools import rel_first_compile_prepare as prepare


class PrepareTests(unittest.TestCase):
    def test_unique_external_providers_only(self):
        hit = lambda path: {'role': 'header', 'path': 'include/'+path}
        selected, unresolved = prepare.provider_headers({
            'missing': {'espEntry': [hit('game/esprite.h')],
                        'unknown': [], 'ambiguous': [hit('a.h'), hit('b.h')]},
            'local_prototype_review': {'missing': {'fn_1_A0': [hit('REL/other.h')]}}})
        self.assertEqual(selected, {'game/esprite.h': ['espEntry']})
        self.assertEqual(unresolved['ambiguous']['headers'], ['a.h', 'b.h'])
        self.assertNotIn('fn_1_A0', selected)

    def test_conflicts_and_narrow_exact_declaration_copy(self):
        old = 'inline double sqrt(double x) { return x; }\nint api(int);'
        expanded = old+'\ninline double sqrt(double x) { return 0; }\nfloat api(float);\nint abs(int);'
        self.assertEqual(prepare.provider_conflicts(old, expanded), ['api', 'sqrt'])
        self.assertEqual(prepare.builtin_declaration_supplement(expanded, ['abs']), 'int abs(int);\n')
        self.assertIsNone(prepare.builtin_declaration_supplement(expanded, ['api']))
        self.assertIsNone(prepare.builtin_declaration_supplement('Thing *api(Thing *);', ['api']))

    def test_context_only_supplement_keeps_conflicting_header_out_of_prefix(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'include').mkdir()
            (root/'include/provider.h').write_text('int abs(int);\ninline double sqrt(double x) { return x; }')
            context = root/'context.i'
            prefix = root/'prefix.h'
            context.write_text('inline double sqrt(double x) { return x; }')
            prefix.write_text('#include "base.h"\n')
            def preprocess(source, output):
                output.write_text(context.read_text()+'\n'+(root/'include/provider.h').read_text())
            ctx, pre, evidence = prepare.extend_call_context(root=root,
                assembly='.fn local, global\nbl abs\nbl fn_1_A0\n.endfn\n.fn fn_1_A0, global\n.endfn\n',
                context_path=context, prefix_path=prefix, output_base=root/'test', preprocess=preprocess, compiler_flags=[])
            self.assertEqual(Path(pre).read_text(), prefix.read_text())
            self.assertTrue(Path(ctx).read_text().endswith('int abs(int);\n'))
            record = evidence['providers'][0]
            self.assertEqual(record['mode'], 'm2c-context-only-declarations')
            self.assertEqual(record['provider_sha256'], prepare.sha(root/'include/provider.h'))
            self.assertEqual(record['preprocessed_sha256'], prepare.sha(Path(record['preprocessed_path'])))
            self.assertIn('sqrt', record['omitted_provider_bodies'])
            self.assertIn('fn_1_A0', evidence['unresolved_local_calls'])

    def test_preprocessor_conflict_uses_exact_standalone_declaration(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'include').mkdir()
            (root/'include/provider.h').write_text('int abs(int);')
            context, prefix = root/'context.i', root/'prefix.h'
            context.write_text('typedef int s32;')
            prefix.write_text('#define M_PI 3.14\n')
            calls = []
            def preprocess(source, output):
                calls.append(source.read_text())
                if '#define M_PI' in calls[-1]:
                    raise RuntimeError('M_PI redefined')
                output.write_text('int abs(int);')
            ctx, pre, evidence = prepare.extend_call_context(root=root, assembly='bl abs\n',
                context_path=context, prefix_path=prefix, output_base=root/'test',
                preprocess=preprocess, compiler_flags=['-fp', 'hard'])
            self.assertEqual(len(calls), 2)
            self.assertEqual(Path(pre).read_text(), prefix.read_text())
            self.assertEqual(Path(ctx).read_text(), 'typedef int s32;\nint abs(int);\n')
            record = evidence['providers'][0]
            self.assertEqual(record['preprocessing_environment'], 'standalone-provider-same-flags')
            self.assertEqual(record['mode'], 'm2c-context-only-declarations')
            self.assertIn('M_PI redefined', record['failed_include']['diagnostic'])

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


def live_m657_acceptance(output):
    """Opt-in actual preprocessing/decompilation only; never compiles a source object."""
    root = Path(__file__).resolve().parents[2]
    output = Path(output).resolve()
    output.relative_to(root/'build')
    output.mkdir(parents=True, exist_ok=False)
    old = root/'build/minigame-recovery-20260913/m657'
    proof = root.parent/'mp6-project-board-star-proof-20260911'
    flags = json.loads((root/'build/minigame-recovery-20260913/m651/latest-compile.json').read_text())['flags']
    prefix = output/'original-prefix.h'
    prefix.write_text((old/'context.c').read_text())
    commands = []
    def preprocess(source, dest):
        command = [str(proof/'build/tools/sjiswrap.exe'),
                   str(proof/'build/compilers/GC/1.3.2/mwcceppc.exe'),
                   *flags, '-P', '-EP', str(source), '-o', str(dest)]
        commands.append(command)
        result = subprocess.run(command, cwd=proof, capture_output=True, text=True, timeout=90)
        if result.returncode:
            raise RuntimeError(result.stdout+result.stderr)
    ctx, pre, evidence = prepare.extend_call_context(root=root,
        assembly=(old/'target.s').read_text(), context_path=old/'context.i',
        prefix_path=prefix, output_base=output/'m657', preprocess=preprocess, compiler_flags=flags)
    translator = root/'build/rel-gap-fixes/m2c/m2c.py'
    asm = proof/'build/GP6E01/m657Dll/asm'
    results = {}
    for label, context in [('old', old/'context.i'), ('new', Path(ctx))]:
        command = [sys.executable, str(translator), '-t', 'ppc-mwcc-c', '--knr', '--force-decimal',
                   '--context', str(context), '-f', 'fn_1_D28', '-f', 'fn_1_23F4',
                   '-f', 'fn_1_42F4', '-f', 'fn_1_5094', '-f', 'fn_1_4EE4',
                   str(old/'target.s'), str(asm/'auto_03_00000000_rodata.s'),
                   str(old/'data.s'), str(asm/'auto_05_00000000_bss.s')]
        result = subprocess.run(command, cwd=proof, capture_output=True, text=True, timeout=90)
        if result.returncode:
            raise RuntimeError(result.stdout+result.stderr)
        draft = output/(label+'.c')
        draft.write_text(result.stdout)
        results[label] = {'source_sha256': prepare.sha(draft),
                          'api_lines': [line.strip() for line in result.stdout.splitlines()
                                        if any(name+'(' in line for name in ('abs', 'espEntry', 'espPosSet'))]}
    assert 's32 abs(s32, s32)' in (output/'old.c').read_text()
    new_text = (output/'new.c').read_text()
    assert 'abs(temp_r30_2->unk14 % 60);' in new_text
    assert 'espPosSet(s16,' not in new_text
    assert 'espPosSet(*(temp_r30 + (var_r31 * 2)), lbl_1_data_2F0[var_r31].unk0, lbl_1_data_2F0[var_r31].unk4);' in new_text
    assert 'fn_1_23F4' not in prepare.decompctx.declared_functions(Path(ctx).read_text())
    (output/'evidence.json').write_text(json.dumps({'providers': evidence, 'commands': commands,
                                                 'results': results}, indent=2))
    print(json.dumps({'provider_modes': [(row['header'],row['mode'],row['calls']) for row in evidence['providers']],
                      'results': results}))


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--live-m657':
        live_m657_acceptance(sys.argv[2])
    else:
        unittest.main()
