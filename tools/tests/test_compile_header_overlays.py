"""Explicit header overlays cannot silently weaken reference checking."""
from pathlib import Path
import tempfile
import unittest
import subprocess
import sys
from unittest import mock

from tools import compile_recovery_candidate as cc
from tools.compile_recovery_candidate import digest, include_context


class HeaderOverlayTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.candidate = self.root / 'candidate/gssdk/mqueue.h'
        self.reference = self.root / 'reference/gssdk/mqueue.h'
        for path, contents in ((self.candidate, 'candidate'), (self.reference, 'reference')):
            path.parent.mkdir(parents=True)
            path.write_text(contents, encoding='utf-8')
        self.command = ['mwcc.exe', '-I', str(self.root / 'candidate')]
        self.overlay = {'name': 'gssdk/mqueue.h', 'candidate': str(self.candidate),
                        'candidate_sha256': digest(self.candidate),
                        'reference': str(self.reference),
                        'reference_sha256': digest(self.reference)}

    def context(self, overlays=None):
        return include_context(self.command, self.root,
                               reference_include_roots=[self.root / 'reference'],
                               header_overlays=overlays)

    def test_unapproved_mismatch(self):
        with self.assertRaisesRegex(ValueError, 'actual header differs'):
            self.context()

    def test_exact_overlay_and_binding(self):
        context = self.context([self.overlay])
        self.assertEqual(context, self.context([self.overlay]))
        self.assertEqual(context['header_overlays']['gssdk/mqueue.h'], self.overlay)
        self.assertEqual(context['references']['gssdk/mqueue.h']['actual_sha256'],
                         digest(self.candidate))
        self.assertEqual(context['references']['gssdk/mqueue.h']['sha256'],
                         digest(self.reference))

    def test_candidate_and_reference_drift(self):
        for role in ('candidate', 'reference'):
            with self.subTest(role=role):
                path = Path(self.overlay[role])
                original = path.read_bytes()
                path.write_bytes(b'drift')
                with self.assertRaisesRegex(ValueError, 'stale header overlay'):
                    self.context([self.overlay])
                path.write_bytes(original)

    def test_wrong_paths(self):
        for role in ('candidate', 'reference'):
            with self.subTest(role=role):
                other = self.root / (role + '.h')
                other.write_bytes(Path(self.overlay[role]).read_bytes())
                overlay = dict(self.overlay, **{role: str(other)})
                with self.assertRaisesRegex(ValueError, 'path mismatch'):
                    self.context([overlay])

    def test_wrong_header_unused_and_duplicate(self):
        for name in ('other.h', 'gssdk/other.h', '../mqueue.h', './gssdk/mqueue.h',
                     'gssdk\\mqueue.h'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.context([dict(self.overlay, name=name)])
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            self.context([self.overlay, self.overlay])

    def test_other_headers_remain_strict(self):
        (self.root / 'candidate/other.h').write_text('changed', encoding='utf-8')
        (self.root / 'reference/other.h').write_text('original', encoding='utf-8')
        with self.assertRaisesRegex(ValueError, 'actual header differs.*other.h'):
            self.context([self.overlay])

    def test_normal_context_unchanged(self):
        self.candidate.write_bytes(self.reference.read_bytes())
        normal = self.context()
        self.assertEqual(normal, self.context([]))
        self.assertNotIn('header_overlays', normal)
        self.assertEqual(normal['references']['gssdk/mqueue.h'], {
            'actual': str(self.candidate), 'reference': str(self.reference),
            'sha256': digest(self.reference)})

    def test_compile_binds_overlay_and_rejects_post_compile_drift(self):
        scratch = self.root / 'build/scratch'
        for base in (self.root, scratch):
            for directory in ('src', 'include', 'build/GP6E01/include'):
                (base / directory).mkdir(parents=True, exist_ok=True)
            (base / 'src/test.c').write_bytes(b'original')
        source = self.root / 'build/candidate.c'
        source.write_bytes(b'candidate')
        output = self.root / 'build/result.o'
        options = dict(root=self.root, scratch=scratch, source=source, output=output,
                       source_relpath='src/test.c', object_relpath='build/test.o',
                       command=[sys.executable, '-I', str(self.root / 'candidate')],
                       tools=[], reference_include_roots=[self.root / 'reference'],
                       header_overlays=[self.overlay],
                       mutex_name='Local\\OverlayFixture' + self.root.name)

        def compile_mock(command, *, cwd, timeout):
            (cwd / 'build/test.o').write_bytes(b'object')
            return subprocess.CompletedProcess(command, 0, stdout=b'', stderr=b'')

        with mock.patch.object(cc.bounded_process, 'run', side_effect=compile_mock):
            receipt = cc.compile_candidate(**options)
        self.assertEqual(receipt['actual_includes']['header_overlays']['gssdk/mqueue.h'],
                         self.overlay)

        def mutate(command, **kwargs):
            result = compile_mock(command, **kwargs)
            self.candidate.write_bytes(b'changed during compile')
            return result

        with mock.patch.object(cc.bounded_process, 'run', side_effect=mutate):
            with self.assertRaisesRegex(RuntimeError, 'actual compiler include context changed'):
                cc.compile_candidate(**options)
        self.assertFalse(output.exists())
        self.assertFalse(output.with_suffix('.o.receipt.json').exists())
        self.assertEqual((scratch / 'src/test.c').read_bytes(), b'original')


if __name__ == '__main__':
    unittest.main()
