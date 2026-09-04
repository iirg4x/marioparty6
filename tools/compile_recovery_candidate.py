"""Bounded scratch-only compiler call; no permits, source promotion or history."""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import ctypes
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

if __package__ in {None, ''}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import bounded_process


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(65536), b''):
            h.update(block)
    return h.hexdigest()


def safe(path: Path, root: Path) -> Path:
    path = Path(os.path.abspath(path))
    path.relative_to(root)
    for part in (path, *path.parents):
        if part.is_symlink() or (hasattr(part, 'is_junction') and part.is_junction()):
            raise ValueError(f'indirected path: {part}')
        if part == root:
            break
    return path


def tree(path: Path) -> dict[str, str]:
    if not path.is_dir():
        raise ValueError(f'missing dependency directory: {path}')
    return {p.relative_to(path).as_posix(): digest(safe(p, path))
            for p in sorted(path.rglob('*')) if p.is_file()}


@contextmanager
def compiler_lock(root: Path, name: str, seconds: float):
    if os.name == 'nt':
        kernel = ctypes.WinDLL('kernel32', use_last_error=True)
        kernel.CreateMutexW.argtypes = [ctypes.c_void_p, ctypes.c_bool, ctypes.c_wchar_p]
        kernel.CreateMutexW.restype = ctypes.c_void_p
        kernel.WaitForSingleObject.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
        kernel.ReleaseMutex.argtypes = [ctypes.c_void_p]
        kernel.CloseHandle.argtypes = [ctypes.c_void_p]
        handle = kernel.CreateMutexW(None, False, name)
        if not handle:
            raise OSError(ctypes.get_last_error(), 'CreateMutexW')
        acquired = False
        try:
            acquired = kernel.WaitForSingleObject(handle, int(seconds*1000)) in (0, 0x80)
            if not acquired:
                raise TimeoutError('compiler mutex deadline exceeded')
            yield
        finally:
            if acquired:
                kernel.ReleaseMutex(handle)
            kernel.CloseHandle(handle)
    else:
        import fcntl
        with (root / '.candidate-compile.lock').open('a+b') as stream:
            start = time.monotonic()
            while True:
                try:
                    fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)
                    break
                except BlockingIOError:
                    if time.monotonic()-start >= seconds:
                        raise TimeoutError('compiler mutex deadline exceeded')
                    time.sleep(.05)
            try:
                yield
            finally:
                fcntl.flock(stream, fcntl.LOCK_UN)


def atomic(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temp = tempfile.mkstemp(prefix=path.name+'.', suffix='.tmp', dir=path.parent)
    try:
        with os.fdopen(fd, 'wb') as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temp, path)
    finally:
        Path(temp).unlink(missing_ok=True)


def compile_candidate(*, root: Path, scratch: Path, source: Path, output: Path,
                      source_relpath: str, object_relpath: str, command: list[str],
                      tools: list[Path], timeout: float = 120,
                      mutex_name: str = 'Global\\CodexBoardCapspecialCandidateCompile') -> dict:
    root = Path(os.path.abspath(root))
    scratch = safe(scratch, root)
    source, output = safe(source, root), safe(output, root / 'build')
    staged, obj = safe(scratch / source_relpath, scratch), safe(scratch / object_relpath, scratch)
    live = safe(root / source_relpath, root)
    if output in (source, live, staged, obj):
        raise ValueError('output aliases source or scratch object')
    receipt_path = output.with_suffix(output.suffix + '.receipt.json')
    safe(receipt_path, root / 'build')
    with compiler_lock(root, mutex_name, timeout):
        candidate = source.read_bytes()
        if len(candidate) > 4*1024*1024:
            raise ValueError('candidate source exceeds 4 MiB')
        source_sha = hashlib.sha256(candidate).hexdigest()
        headers = tree(scratch / 'include')
        if headers != tree(root / 'include'):
            raise ValueError('scratch headers differ from live headers; refresh scratch context')
        context = {'headers': headers, 'generated_headers': tree(scratch / 'build/GP6E01/include'),
                   'tools': {str(p.resolve()): digest(p) for p in tools},
                   'command': command,
                   'environment_sha256': hashlib.sha256(json.dumps(dict(os.environ), sort_keys=True).encode()).hexdigest()}
        context_sha = hashlib.sha256(json.dumps(context, sort_keys=True).encode()).hexdigest()
        original = staged.read_bytes()
        # Remove old products only after preflight succeeds. Never accept one
        # after a failed launch; receipt is published last on success.
        output.unlink(missing_ok=True)
        receipt_path.unlink(missing_ok=True)
        obj.unlink(missing_ok=True)
        start = time.monotonic()
        try:
            atomic(staged, candidate)
            result = bounded_process.run(command, cwd=scratch, timeout=timeout)
            if result.returncode:
                detail = result.stderr.decode('utf-8', 'replace') or result.stdout.decode('utf-8', 'replace')
                raise RuntimeError(f'compiler failed ({result.returncode}): {detail[-8000:]}')
            if digest(staged) != source_sha or digest(source) != source_sha:
                raise RuntimeError('source changed during compilation')
            if tree(scratch / 'include') != headers or tree(root / 'include') != headers:
                raise RuntimeError('headers changed during compilation')
            if tree(scratch / 'build/GP6E01/include') != context['generated_headers']:
                raise RuntimeError('generated headers changed during compilation')
            if any(digest(p) != context['tools'][str(p.resolve())] for p in tools):
                raise RuntimeError('compiler context changed during compilation')
            if not obj.is_file() or not 0 < obj.stat().st_size <= 16*1024*1024:
                raise RuntimeError('compiler did not produce a bounded nonempty object')
            data = obj.read_bytes()
            receipt = {'schema': 'recovery_candidate_compile/v1', 'source_sha256': source_sha,
                       'context_sha256': context_sha, 'object_sha256': hashlib.sha256(data).hexdigest(),
                       'command': command, 'tools': context['tools'],
                       'header_set_sha256': hashlib.sha256(json.dumps(headers, sort_keys=True).encode()).hexdigest(),
                       'generated_header_set_sha256': hashlib.sha256(json.dumps(context['generated_headers'], sort_keys=True).encode()).hexdigest(),
                       'object_size': len(data), 'seconds': time.monotonic()-start,
                       'stdout_sha256': hashlib.sha256(result.stdout).hexdigest(),
                       'stderr_sha256': hashlib.sha256(result.stderr).hexdigest()}
        finally:
            atomic(staged, original)
        atomic(output, data)
        try:
            atomic(receipt_path, json.dumps(receipt, sort_keys=True).encode())
        except BaseException:
            output.unlink(missing_ok=True)
            raise
        return receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('root', 'scratch', 'source-relpath', 'object-relpath', 'compiler-script'):
        parser.add_argument('--'+name, required=True)
    parser.add_argument('--source')
    parser.add_argument('--output')
    parser.add_argument('--preflight', action='store_true')
    parser.add_argument('--tool', action='append', default=[])
    parser.add_argument('--timeout', type=float, default=120)
    parser.add_argument('--mutex-name', default='Global\\CodexBoardCapspecialCandidateCompile')
    args = parser.parse_args()
    root = Path(args.root).absolute()
    path = lambda raw: root / raw
    script = path(args.compiler_script)
    command = ['powershell.exe', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', str(script)]
    try:
        if args.preflight:
            scratch = safe(path(args.scratch), root)
            if tree(scratch/'include') != tree(root/'include'):
                raise ValueError('scratch headers differ from live headers')
            tree(scratch/'build/GP6E01/include')
            for tool in [script, *(Path(p) for p in args.tool)]:
                digest(tool)
            print('context preflight passed; no compiler launched or source changed')
            return 0
        if not args.source or not args.output:
            parser.error('--source and --output are required unless --preflight is used')
        result = compile_candidate(root=root, scratch=path(args.scratch), source=path(args.source),
                                   output=path(args.output), source_relpath=args.source_relpath,
                                   object_relpath=args.object_relpath, command=command,
                                   tools=[script, *(Path(p) for p in args.tool)], timeout=args.timeout,
                                   mutex_name=args.mutex_name)
        print(json.dumps(result, sort_keys=True))
        return 0
    except (OSError, ValueError, RuntimeError) as exc:
        print(f'candidate compile: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
