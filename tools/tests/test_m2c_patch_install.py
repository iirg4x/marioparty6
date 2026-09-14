"""Repository-independent portable patch installs; synthetic public-safe inputs."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch


INSTALLER = Path(__file__).resolve().parents[1] / 'patches/m2c-rel-first-pass/install.py'


@unittest.skipUnless(shutil.which('git'), 'git is required for patch installation')
class PatchInstallTests(unittest.TestCase):
    def test_external_and_nested_install_ignore_ambient_repository(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            package = root / 'package'
            package.mkdir()
            shutil.copy2(INSTALLER, package / 'install.py')
            spec = importlib.util.spec_from_file_location('portable_install_test', package / 'install.py')
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            source = root / 'source'
            source.mkdir()
            before, after = b'old\n', b'new\n'
            (source / 'example.txt').write_bytes(before)
            diff = b'diff --git a/example.txt b/example.txt\n--- a/example.txt\n+++ b/example.txt\n@@ -1 +1 @@\n-old\n+new\n'
            (package / 'm2c.patch').write_bytes(diff)
            digest = lambda value: hashlib.sha256(value).hexdigest()
            manifest = dict(schema='m2c_rel_first_pass_patch/v1', patch_sha256=digest(diff),
                            required_baseline_files={'example.txt': digest(before)},
                            files=[dict(path='example.txt', before_sha256=digest(before),
                                        after_sha256=digest(after), after_newline='lf')])
            (package / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
            repo = root / 'repository'
            subprocess.run(['git', 'init', str(repo)], check=True, capture_output=True)
            # Deliberately misleading explicit repo variables must also be ignored.
            with patch.dict(os.environ, {'GIT_DIR': str(repo / '.git'),
                                        'GIT_WORK_TREE': str(repo), 'GIT_PREFIX': 'wrong/',
                                        'GIT_CONFIG_COUNT': '1',
                                        'GIT_CONFIG_KEY_0': 'core.autocrlf',
                                        'GIT_CONFIG_VALUE_0': 'true'}):
                for destination in (root / 'external', repo / 'nested' / 'installed'):
                    receipt = module.install(source, destination)
                    self.assertEqual((destination / 'example.txt').read_bytes(), after)
                    self.assertEqual(receipt['files_verified'], 1)
                    self.assertFalse((destination / '.git').exists())
            self.assertEqual((source / 'example.txt').read_bytes(), before)
            self.assertFalse((repo / 'example.txt').exists())


if __name__ == '__main__':
    unittest.main()
