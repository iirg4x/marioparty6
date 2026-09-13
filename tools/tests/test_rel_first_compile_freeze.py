"""Synthetic continuation acceptance; no real batch files touched."""
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from tools import rel_first_compile_freeze as freeze

class FreezeTests(unittest.TestCase):
    def test_remaining_rejects_unknown_and_duplicate(self):
        mods = [{'name': 'm', 'functions': ['a', 'b']}]
        self.assertEqual(freeze.remaining(mods, [{'module': 'm', 'function': 'a'}])[0]['functions'], ['b'])
        with self.assertRaises(ValueError): freeze.remaining(mods, [{'module': 'x', 'function': 'a'}])
        with self.assertRaises(ValueError): freeze.remaining(mods, [{'module': 'm', 'function': 'a'}] * 2)

    def test_frozen_phase(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / 'root'; proof = Path(tmp) / 'proof'
            base = root / 'build/custom-batch'; run = base / 'run'
            def put(p, text):
                p.parent.mkdir(parents=True, exist_ok=True); p.write_text(text); return str(p)
            inc = root / 'include'; gen = proof / 'build/GP6E01/include'
            put(inc / 'a.h', '#include "outside.h"\n#if 0\n#include <stdint.h>\n#endif\n'); put(gen / 'b.h', '')
            put(root / 'outside.h', 'typedef int frozen;')
            macro = Path(tmp) / 'm2c_macros.h'; put(macro, '')
            prefix = put(base / 'prefix.h', '#include "a.h"\n#include "' + macro.as_posix() + '"\n')
            ctx = put(base / 'context.i', 'typedef int frozen;')
            target = put(proof / 'target.plf', 'binary'); asm = put(base / 'asm.s', '.text')
            put(root / 'configure.py', '# original')
            m = dict(schema='rel_first_compile_batch/v1', root=str(root), proof_root=str(proof), output_root=str(run), m2c=str(Path(tmp) / 'm2c.py'), objdiff='fake', context_path=ctx, prefix_path=prefix, flags=['-i',str(inc),'-i','build/GP6E01/include','-i',str(root)], modules=[dict(name='m', functions=['a','b'], target_path=target, assembly_paths=[asm])])
            mp = Path(put(base / 'manifest.json', json.dumps(m)))
            empty = root / 'empty'; empty.mkdir()
            m['flags'] += ['-i', str(empty), '-ir', '-ipa', 'off', '-inline', 'auto']
            mp.write_text(json.dumps(m))
            put(run / 'STOP', '')
            put(run / 'census.jsonl', json.dumps({'module':'m','function':'a'}) + '\n')
            put(run / 'bindings.json', json.dumps({'manifest_sha256':freeze.sha(mp),'files':{p:freeze.sha(p) for p in (prefix,ctx,target,asm)}}))
            with patch.object(sys,'argv',['freeze',str(mp),'phase2','--drained']), patch.object(freeze.subprocess,'run',return_value=SimpleNamespace(stdout='commit\n')):
                freeze.main()
            result = json.loads((base / 'phase2/manifest.json').read_text())
            self.assertEqual(result['modules'][0]['functions'], ['b'])
            self.assertEqual(result['frozen_input_schema'], 'rel_frozen_inputs/v1')
            watched = result['modules'][0]['header_watch_paths']
            self.assertTrue(any(p.endswith('outside.h') for p in watched))
            self.assertTrue(any(p.endswith('a.h') for p in watched))
            self.assertFalse(any(p.endswith('b.h') or p.endswith('.plf') for p in watched))
            self.assertIn('-ipa', result['flags'])
            include_paths = [Path(result['flags'][i+1]) for i, arg in enumerate(result['flags']) if arg == '-i']
            self.assertTrue(all(p.is_dir() for p in include_paths))
            self.assertEqual(list(include_paths[-1].iterdir()), [])
            self.assertEqual(result['continuation']['prior_completed'], 1)
            self.assertTrue(all(str(base / 'phase2') in p for p in result['source_watch_sha256']))
            self.assertNotEqual(result['modules'][0]['target_path'], target)
            self.assertIn('phase2', Path(result['prefix_path']).read_text())
            self.assertEqual((root / 'outside.h').read_text(), 'typedef int frozen;')

if __name__ == '__main__': unittest.main()
