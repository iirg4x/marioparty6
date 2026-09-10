from pathlib import Path
import os
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


if __name__ == '__main__':
    unittest.main()
