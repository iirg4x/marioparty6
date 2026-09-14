"""Apply the REL translation fixes to a NEW copy of the pinned local m2c.

Never changes the source installation or selects a compiler/build configuration.
The destination becomes usable only after the final file hashes are verified.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def child(root: Path, name: str) -> Path:
    rel = Path(name)
    if rel.is_absolute() or '..' in rel.parts:
        raise ValueError('invalid package path: ' + name)
    path = root / rel
    if not path.resolve().is_relative_to(root.resolve()):
        raise ValueError('package path escapes root: ' + name)
    return path


def apply_environment(destination: Path) -> dict[str, str]:
    """Apply relative to this copied tree, never an ambient parent repository."""
    env = os.environ.copy()
    for name in ('GIT_DIR', 'GIT_WORK_TREE', 'GIT_COMMON_DIR', 'GIT_INDEX_FILE',
                 'GIT_PREFIX', 'GIT_OBJECT_DIRECTORY', 'GIT_ALTERNATE_OBJECT_DIRECTORIES'):
        env.pop(name, None)
    # git apply supports a missing explicit Git directory without creating one.
    # The copied tree excludes .git. Selecting that absent directory prevents
    # parent discovery (and works with both native Windows and MSYS Git, whose
    # GIT_CEILING_DIRECTORIES path syntaxes differ).
    if (destination / '.git').exists():
        raise ValueError('copied destination unexpectedly contains .git: ' + str(destination))
    env['GIT_DIR'] = str(destination.resolve() / '.git')
    return env


def install(source: Path, destination: Path) -> dict:
    package = Path(__file__).resolve().parent
    manifest_path = package / 'manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    if manifest['schema'] != 'm2c_rel_first_pass_patch/v1':
        raise ValueError('unsupported patch manifest')
    patch = package / 'm2c.patch'
    if digest(patch) != manifest['patch_sha256']:
        raise ValueError('patch content drift')
    source, destination = source.resolve(strict=True), destination.absolute()
    if destination.exists() or destination.is_symlink():
        raise ValueError('destination must be new: ' + str(destination))
    if destination.resolve().is_relative_to(source):
        raise ValueError('destination must not be inside the source installation')
    for name, expected in manifest['required_baseline_files'].items():
        path = child(source, name)
        if not path.is_file() or digest(path) != expected:
            raise ValueError('baseline dependency mismatch: ' + name)
    for item in manifest['files']:
        path = child(source, item['path'])
        expected = item['before_sha256']
        if (expected is None and path.exists()) or (expected is not None and digest(path) != expected):
            raise ValueError('baseline patch input mismatch: ' + item['path'])
    # This is a small source/test copy, not a compiler or game artifact tree.
    shutil.copytree(source, destination, symlinks=True, ignore=shutil.ignore_patterns(
        '.git', '__pycache__', '*.m2c', '.pytest_cache', '.venv', 'venv', '.coverage'))
    # The portable patch uses LF. Normalize only its copied inputs; restore the
    # verified output spelling before checking the original working-file hashes.
    for item in manifest['files']:
        path = child(destination, item['path'])
        if item['before_sha256'] is not None:
            path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n'))
    for args in (['--check'], []):
        process = subprocess.run(['git', '-c', 'core.autocrlf=false', 'apply', *args, str(patch)], cwd=destination,
                                 env=apply_environment(destination),
                                 capture_output=True, text=True, timeout=30)
        if process.returncode:
            raise ValueError('patch application failed; unused destination preserved: '
                             + (process.stdout + process.stderr)[-2000:])
    for item in manifest['files']:
        path = child(destination, item['path'])
        if item['after_newline'] == 'crlf':
            path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))
        if digest(path) != item['after_sha256']:
            raise ValueError('installed content mismatch: ' + item['path'])
    receipt = dict(schema='m2c_rel_first_pass_install/v1', source=str(source),
                   destination=str(destination), manifest_sha256=digest(manifest_path),
                   patch_sha256=manifest['patch_sha256'], files_verified=len(manifest['files']),
                   source_unchanged=True, matching_credit=False)
    for name, expected in manifest['required_baseline_files'].items():
        if digest(child(source, name)) != expected:
            raise ValueError('source installation changed during copy: ' + name)
    (destination / 'rel-first-pass-install.json').write_text(
        json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    return receipt


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--destination', type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(install(args.source, args.destination), indent=2))


if __name__ == '__main__':
    main()
