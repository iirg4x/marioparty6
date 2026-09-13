from pathlib import Path
import os
import hashlib
import json
import re
import subprocess
import tempfile
import unittest
from unittest.mock import patch
from tools import decompctx


class ContextTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root/'src').mkdir()
        (self.root/'include').mkdir()

    def write(self, path, text):
        (self.root/path).write_text(text)

    def test_foreign_cwd_void_prototype_and_repeated_calls(self):
        self.write('src/a.c', '#include "api.h"\n#include "keep.s"\n')
        self.write('include/api.h', '/* license */\n#ifndef API_H\n#define API_H\nvoid A(int n);\n#endif\n')
        original = os.getcwd()
        try:
            os.chdir(self.root/'include')
            one = decompctx.import_c_file('src/a.c', [], root=self.root)
            two = decompctx.import_c_file('src/a.c', [], root=self.root)
        finally:
            os.chdir(original)
        self.assertEqual(one, two)
        self.assertIn('void A(int n);', one)
        self.assertIn('#include "keep.s"', one)
        self.assertIn('#ifndef API_H', one)

    def test_missing_leaf_does_not_publish(self):
        self.write('src/a.c', '#include "api.h"\n')
        self.write('include/api.h', '#include "absent.h"\n')
        self.assertEqual(decompctx.main(['--root', str(self.root), 'src/a.c', '-o', 'ctx.c', '-d', 'ctx.d']), 1)
        self.assertFalse((self.root/'ctx.c').exists())
        self.assertFalse((self.root/'ctx.d').exists())

    def test_explicit_owner_root_wins_over_tool_checkout(self):
        self.write('src/a.c', '#include "api.h"\n')
        self.write('include/api.h', 'void OwnerAPI(void);\n')
        with tempfile.TemporaryDirectory() as other:
            tool_root = Path(other)
            (tool_root/'include').mkdir()
            (tool_root/'include/api.h').write_text('int WrongToolAPI(void);\n')
            original = os.getcwd()
            try:
                os.chdir(tool_root)
                with patch.object(decompctx, 'root_dir', str(tool_root)), patch.object(
                        decompctx, 'include_dirs', [str(tool_root/'include')]):
                    output = decompctx.import_c_file('src/a.c', [], root=self.root)
            finally:
                os.chdir(original)
        self.assertIn('void OwnerAPI(void);', output)
        self.assertNotIn('WrongToolAPI', output)

    def test_missing_leaf_preserves_existing_output_and_depfile(self):
        self.write('src/a.c', '#include "absent.h"\n')
        self.write('ctx.c', 'previous complete context\n')
        self.write('ctx.d', 'previous complete dependencies\n')
        before = [(self.root/name).read_bytes() for name in ('ctx.c', 'ctx.d')]
        self.assertEqual(decompctx.main(['--root', str(self.root), 'src/a.c', '-d', 'ctx.d']), 1)
        self.assertEqual(before, [(self.root/name).read_bytes() for name in ('ctx.c', 'ctx.d')])

    def test_guarded_cycle_and_unguarded_cycle(self):
        self.write('src/a.c', '#include "api.h"\n')
        self.write('include/api.h', '/* prefix */\n#ifndef API_H\n#define API_H\n#include "api.h"\nvoid A(void);\n#endif\n')
        out = decompctx.import_c_file('src/a.c', [], root=self.root)
        self.assertEqual(out.count('void A(void);'), 1)
        self.write('include/api.h', '/* prefix */\n#include "api.h"\n')
        with self.assertRaisesRegex(decompctx.ContextError, 'cycle'):
            decompctx.import_c_file('src/a.c', [], root=self.root)

    def test_atomic_success_and_dependencies(self):
        self.write('src/a.c', '#include "api.h"\n')
        self.write('include/api.h', 'void A(void);\n')
        self.assertEqual(decompctx.main(['--root', str(self.root), 'src/a.c', '-d', 'ctx.d']), 0)
        self.assertIn('void A(void);', (self.root/'ctx.c').read_text())
        self.assertIn('include/api.h', (self.root/'ctx.d').read_text().replace('\\', '/'))

    def test_call_context_finds_real_headers_and_provider_without_guessing(self):
        self.write('include/actor.h', 'typedef struct Actor Actor;\nActor *ActorCreate(int n);\n')
        self.write('src/mic.c', 'short MicCreate(char *path) { return 0; }\n')
        asm = '.fn init, global\nbl ActorCreate\nbl MicCreate\nbl init\nbl Unknown\n'
        result = decompctx.discover_call_context(self.root, asm, '', ['src/mic.c'])
        self.assertEqual(result['include_hints'], ['actor.h'])
        self.assertEqual(result['unresolved'], ['Unknown'])
        self.assertEqual(result['local_calls'], ['init'])
        provider = result['missing']['MicCreate'][0]
        self.assertEqual(provider['declaration'], 'short MicCreate(char *path)')
        self.assertEqual(provider['role'], 'provider')
        self.assertTrue(provider['definition'])
        self.assertEqual(provider['sha256'], hashlib.sha256((self.root/'src/mic.c').read_bytes()).hexdigest())
        corrected = decompctx.discover_call_context(self.root, asm,
            'Actor *ActorCreate(int n);\nshort MicCreate(char *path);\n')
        self.assertEqual(corrected['covered_calls'], ['ActorCreate', 'MicCreate'])

    def test_function_scanner_ignores_calls_comments_macros_and_nonprototypes(self):
        text = '''/* void Fake(int n); */
#define BAD(x) Fake(x)
typedef void (*Callback)(int n);
extern "C" {
void Real(
    int n, void (*callback)(int));
int Old();
int Local(void) { Real(1, 0); return Missing(1); }
}
'''
        rows = decompctx.declared_functions(text)
        self.assertEqual(set(rows), {'Real', 'Old', 'Local'})
        self.assertEqual(len(rows['Real']), 1)
        self.assertEqual(rows['Real'][0]['line'], 5)
        self.assertFalse(rows['Old'][0]['prototyped'])
        result = decompctx.discover_call_context(self.root, 'bl Old\nbl Real', text)
        self.assertEqual(result['covered_calls'], ['Real'])
        self.assertEqual(result['unresolved'], [])
        self.assertEqual(result['present_without_prototype'], ['Old'])

    def test_call_context_preserves_conflicting_alternatives(self):
        self.write('include/a.h', 'void Create(int n);\n')
        self.write('include/b.h', 'int Create(float n);\n')
        rows = decompctx.discover_call_context(self.root, 'bl Create', '')['missing']['Create']
        self.assertEqual([row['declaration'] for row in rows], ['void Create(int n)', 'int Create(float n)'])

    def test_call_context_cli_and_provider_containment(self):
        self.write('src/context.i', 'void Present(void);\n')
        self.write('src/target.s', 'bl Present\nbl Missing\n')
        self.assertEqual(decompctx.main(['src/context.i', '--root', str(self.root),
            '--calls-from', 'src/target.s', '-o', 'calls.json']), 0)
        self.assertEqual(json.loads((self.root/'calls.json').read_text())['unresolved'], ['Missing'])
        with self.assertRaisesRegex(decompctx.ContextError, 'Provider'):
            decompctx.discover_call_context(self.root, '', '', ['../escape.c'])

    def test_local_void_helper_saved_result_warns_without_inventing_signature(self):
        asm = '.fn Normalize, global\nbl PSVECNormalize\nli r30, 1\nmr r3, r30\nlwz r30, 8(r1)\nblr\n.endfn Normalize\n'
        result = decompctx.discover_call_context(self.root, asm,
            'void Normalize(Vec *src, Vec *dst);\n')
        self.assertEqual(result['return_warnings'][0]['function'], 'Normalize')
        self.assertNotIn('suggested_return_type', result['return_warnings'][0])
        self.assertEqual(decompctx.discover_call_context(self.root, asm,
            'int Normalize(Vec *src, Vec *dst);\n')['return_warnings'], [])
        # A later real call overwrites r3: this does not support the cue.
        self.assertEqual(decompctx.discover_call_context(self.root,
            asm.replace('lwz r30, 8(r1)', 'bl RealCall\nlwz r30, 8(r1)'),
            'void Normalize(Vec *src, Vec *dst);\n')['return_warnings'], [])
        self.assertEqual(decompctx.discover_call_context(self.root,
            asm.replace('lwz r30, 8(r1)', 'li r3, 0\nlwz r30, 8(r1)'),
            'void Normalize(Vec *src, Vec *dst);\n')['return_warnings'], [])

    def test_header_named_directory_is_not_read_as_file(self):
        (self.root/'include/Runtime.H').mkdir()
        self.assertEqual(decompctx.discover_call_context(self.root, 'bl Missing', '')['unresolved'], ['Missing'])

    def test_compiler_save_restore_helpers_are_not_missing_application_prototypes(self):
        result = decompctx.discover_call_context(self.root, 'bl _savegpr_22\nbl _restgpr_22\nbl Unknown', '')
        self.assertEqual(result['compiler_helper_calls'], ['_restgpr_22', '_savegpr_22'])
        self.assertEqual(result['unresolved'], ['Unknown'])


META = '''  0 .text 00000014 00000000 00000000 00000034 2**2
00000000 g     F .text 00000014 f
'''
RAW = '''Disassembly of section .text:
00000000 <f>:
 0: 48 00 00 01 bl 0 <f>
 0: R_PPC_REL24 helper
 4: 3c 60 00 00 lis r3,0
 6: R_PPC_ADDR16_HA global
 8: 80 63 00 00 lwz r3,0(r3)
 a: R_PPC_ADDR16_LO global
 c: 48 00 00 04 b 10 <f+0x10>
 10: 4e 80 00 20 blr
'''
RELOC = '''RELOCATION RECORDS FOR [.text]:
00000000 R_PPC_REL24 helper
00000006 R_PPC_ADDR16_HA global
0000000a R_PPC_ADDR16_LO global
'''
CODE = bytes.fromhex('480000013c60000080630000480000044e800020')

ELF_HEADER = bytearray(52)
ELF_HEADER[:7] = b'\x7fELF\x01\x02\x01'
ELF_HEADER[16:24] = bytes.fromhex('0001001400000001')
ELF_HEADER[40:42] = bytes.fromhex('0034')
DATA = bytes(ELF_HEADER) + CODE
adapt = decompctx.adapt_target_function
source = Path(__file__)

class AdapterTests(unittest.TestCase):
    def test_target_identity_fail_closed(self):
        for offset, value in ((0, 0), (4, 2), (5, 1), (6, 0), (17, 2),
                              (19, 21), (23, 0), (41, 0)):
            malformed = bytearray(DATA)
            malformed[offset] = value
            with self.assertRaisesRegex(ValueError, 'ELF32'):
                adapt(bytes(malformed), META, RAW, RELOC, 'f')
        for malformed in (b'', DATA[:51], bytearray(DATA)):
            with self.assertRaisesRegex(ValueError, 'ELF32'):
                adapt(malformed, META, RAW, RELOC, 'f')

    def test_relocated_opcode_lk_and_sda21_forms_fail_closed(self):
        with self.assertRaisesRegex(ValueError, 'REL24'):
            adapt(DATA, META, RAW.replace('bl 0 <f>', 'b 0 <f>'), RELOC, 'f')
        changed_data = DATA[:52] + bytes.fromhex('48000003') + DATA[56:]
        with self.assertRaisesRegex(ValueError, 'REL24'):
            adapt(changed_data, META, RAW.replace('48 00 00 01', '48 00 00 03'), RELOC, 'f')
        for changed in (RAW.replace('lwz r3,0(r3)', 'stw r3,0(r3)'),
                        RAW.replace('lwz r3,0(r3)', 'lwz r4,0(r3)'),
                        RAW.replace('lis r3,0', 'lis r4,0')):
            with self.assertRaisesRegex(ValueError, 'target encoding'):
                adapt(DATA, META, changed, RELOC, 'f')
        with self.assertRaisesRegex(ValueError, 'SDA21 instruction form'):
            adapt(DATA, META, RAW.replace('6: R_PPC_ADDR16_HA', '4: R_PPC_EMB_SDA21'),
                  RELOC.replace('00000006 R_PPC_ADDR16_HA', '00000004 R_PPC_EMB_SDA21'), 'f')

    def test_full_bytes_calls_memory_relocations_and_labels(self):
        assembly, receipt = adapt(DATA, META, RAW, RELOC, 'f')
        self.assertEqual(receipt['instructions'], 5)
        self.assertIn('bl helper', assembly)
        self.assertIn('lwz r3,global@l(r3)', assembly)
        self.assertIn('b .L_f_10', assembly)
        self.assertEqual(receipt['instruction_bytes_sha256'], hashlib.sha256(CODE).hexdigest())
        self.assertEqual(receipt['target_sha256'], hashlib.sha256(DATA).hexdigest())
        self.assertIn('inferred', receipt['inference_caveat'])

    def test_missing_function_and_unsupported_relocation(self):
        with self.assertRaisesRegex(ValueError, 'missing/ambiguous'):
            adapt(DATA, META, RAW, RELOC, 'absent')
        with self.assertRaisesRegex(ValueError, 'unsupported relocation'):
            adapt(DATA, META, RAW.replace('R_PPC_REL24', 'R_PPC_ADDR32'),
                  RELOC.replace('R_PPC_REL24', 'R_PPC_ADDR32'), 'f')

    def test_bytes_counts_branch_and_relocation_census_fail_closed(self):
        for changed in (RAW.replace('4e 80 00 20', '4e 80 00 21'),
                        RAW.replace(' 10: 4e 80 00 20 blr\n', ''),
                        RAW.replace('b 10 <', 'b 8 <'),
                        RAW.replace(' 0: R_PPC_REL24 helper\n', '')):
            with self.assertRaises(ValueError):
                adapt(DATA, META, changed, RELOC, 'f')

    def test_sda21_preserves_symbol_and_encoded_base(self):
        raw = RAW.replace('a: R_PPC_ADDR16_LO global', '8: R_PPC_EMB_SDA21 global')
        reloc = RELOC.replace('0000000a R_PPC_ADDR16_LO', '00000008 R_PPC_EMB_SDA21')
        assembly, _ = adapt(DATA, META, raw, reloc, 'f')
        self.assertIn('lwz r3,global@sda21(r3)', assembly)
        zero_data = DATA[:60] + bytes.fromhex('80600000') + DATA[64:]
        raw = raw.replace('80 63 00 00 lwz r3,0(r3)', '80 60 00 00 lwz r3,0(0)')
        assembly, _ = adapt(zero_data, META, raw, reloc, 'f')
        self.assertIn('lwz r3,global@sda21(r0)', assembly)

    def test_bound_local_objects_when_requested(self):
        if os.environ.get('DECOMPCTX_TARGET_REPLAY') != '1':
            self.skipTest('explicit local objects not selected')
        proof = source.resolve().parents[3] / 'mp6-project-board-star-proof-20260911/build/GP6E01/obj'
        objdump = 'C:/Users/Anony/.codex/tools/mp6/binutils-2.42-1/powerpc-eabi-objdump.exe'
        for rel in ('gssdk_lib/gsapi/ctxfuncs.o', 'gssdk_lib/gsapi/callbacks.o', 'musyx/runtime/hardware.o'):
            path = proof / rel
            before = path.read_bytes()
            meta = subprocess.check_output([objdump, '-h', '-t', str(path)], text=True)
            raw = subprocess.check_output([objdump, '-dr', str(path)], text=True)
            reloc = subprocess.check_output([objdump, '-r', str(path)], text=True)
            names = re.findall(r'^[0-9a-f]+\s+\w\s+F\s+\S+\s+[0-9a-f]+\s+(\S+)\s*$', meta, re.M)
            if 'callbacks' in rel:
                names = ['asrspi_cbResult']
            elif 'hardware' in rel:
                names = names[:1]  # The existing target's first function has real SDA21.
            for name in names:
                assembly, receipt = adapt(before, meta, raw, reloc, name)
                if 'callbacks' in rel:
                    self.assertEqual(receipt['instructions'], 352)
                if 'hardware' in rel:
                    self.assertIn('@sda21', assembly)
            self.assertEqual(before, path.read_bytes())


if __name__ == '__main__':
    unittest.main()
