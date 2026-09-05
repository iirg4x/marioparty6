"""Bounded scratch-only compiler call; no permits, source promotion or history."""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import ctypes
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
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


def validate_command(command: object) -> list[str]:
    if not isinstance(command, list) or not command:
        raise ValueError('compiler command must be a nonempty JSON argv list')
    for index, arg in enumerate(command):
        if not isinstance(arg, str) or not arg.strip() or any(c in arg for c in '\0\r\n'):
            raise ValueError(f'compiler argv[{index}] must be a nonempty string without NUL or newlines')
    return list(command)


def _command_descriptor(path: Path) -> tuple[list[str], dict[str, str]]:
    try:
        path = path.resolve(strict=True)
    except OSError as exc:
        raise ValueError(f'cannot read command JSON {path}: {exc.strerror}') from exc
    if not path.is_file():
        raise ValueError(f'command JSON is not a file: {path}')
    raw = path.read_bytes()
    if len(raw) > 1024 * 1024:
        raise ValueError(f'command JSON exceeds 1 MiB: {path}')
    try:
        command = validate_command(json.loads(raw.decode('utf-8')))
    except (UnicodeError, ValueError) as exc:
        raise ValueError(f'invalid command JSON {path}: {exc}') from exc
    return command, {'path': str(path), 'sha256': hashlib.sha256(raw).hexdigest()}


def load_command_json(path: Path) -> list[str]:
    return _command_descriptor(path)[0]


def _tool_path(path: Path, base: Path) -> Path:
    path = base / path
    try:
        path = path.resolve(strict=True)
    except OSError as exc:
        raise ValueError(f'cannot read compiler tool {path}: {exc.strerror}') from exc
    if not path.is_file():
        raise ValueError(f'compiler tool is not a file: {path}')
    return path


def _resolved_command(command: list[str], scratch: Path) -> list[str]:
    command = validate_command(command)
    executable = Path(command[0])
    if not executable.is_absolute() and not any(c in command[0] for c in '/\\'):
        found = shutil.which(command[0])
        if found is None:
            raise ValueError(f'compiler executable not found: {command[0]}')
        executable = Path(found)
    executable = _tool_path(executable, scratch)
    if executable.suffix.lower() in {'.bat', '.cmd'}:
        raise ValueError(f'compiler executable must not be a shell batch file: {executable}')
    if os.name == 'nt' and executable.suffix.lower() not in {'.exe', '.com'}:
        raise ValueError(f'compiler executable must be a native .exe or .com file: {executable}')
    if os.name != 'nt' and not os.access(executable, os.X_OK):
        raise ValueError(f'compiler executable is not executable: {executable}')
    command[0] = str(executable)
    return command


def preflight_context(*, root: Path, scratch: Path, command: list[str],
                      tools: list[Path], command_descriptor: Path | None = None) -> dict:
    """Read-only context used identically by CLI preflight and compilation."""
    root = Path(os.path.abspath(root))
    scratch = safe(scratch, root)
    command = _resolved_command(command, scratch)
    descriptor = None
    if command_descriptor is not None:
        described, descriptor = _command_descriptor(command_descriptor)
        if _resolved_command(described, scratch) != command:
            raise ValueError(f'command JSON changed or does not describe argv: {command_descriptor}')
    tool_paths = {Path(command[0]), *(_tool_path(p, root) for p in tools)}
    headers = tree(scratch / 'include')
    if headers != tree(root / 'include'):
        raise ValueError('scratch headers differ from live headers; refresh scratch context')
    return {'headers': headers, 'generated_headers': tree(scratch / 'build/GP6E01/include'),
            'tools': {str(p): digest(p) for p in sorted(tool_paths)},
            'command': command, 'command_descriptor': descriptor,
            'environment_sha256': hashlib.sha256(json.dumps(dict(os.environ), sort_keys=True).encode()).hexdigest()}


def context_digest(context: dict) -> str:
    return hashlib.sha256(json.dumps(context, sort_keys=True).encode()).hexdigest()


def _diagnostics(stdout: bytes, stderr: bytes) -> str:
    return '\n'.join(f'{name}:\n{data.decode("utf-8", "replace")[-4000:]}'
                     for name, data in (('stdout', stdout), ('stderr', stderr)) if data)


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
                      mutex_name: str = 'Global\\CodexBoardCapspecialCandidateCompile',
                      command_descriptor: Path | None = None) -> dict:
    root = Path(os.path.abspath(root))
    scratch = safe(scratch, root)
    source, output = safe(source, root), safe(output, root / 'build')
    staged, obj = safe(scratch / source_relpath, scratch), safe(scratch / object_relpath, scratch)
    live = safe(root / source_relpath, root)
    if output in (source, live, staged, obj):
        raise ValueError('output aliases source or scratch object')
    receipt_path = output.with_suffix(output.suffix + '.receipt.json')
    stdout_path = output.with_suffix(output.suffix + '.stdout.log')
    stderr_path = output.with_suffix(output.suffix + '.stderr.log')
    if not math.isfinite(timeout) or timeout <= 0:
        raise ValueError('positive finite compiler timeout required')
    context = preflight_context(root=root, scratch=scratch, command=command,
                                tools=tools, command_descriptor=command_descriptor)
    command = context['command']
    inputs = {source, live, staged, obj, *(Path(p) for p in context['tools'])}
    if context['command_descriptor'] is not None:
        inputs.add(Path(context['command_descriptor']['path']))
    for product in (output, receipt_path, stdout_path, stderr_path):
        safe(product, root / 'build')
        if product in inputs:
            raise ValueError(f'compiler output aliases an input: {product}')
    with compiler_lock(root, mutex_name, timeout):
        if preflight_context(root=root, scratch=scratch, command=command,
                             tools=tools, command_descriptor=command_descriptor) != context:
            raise ValueError('compiler context changed before launch')
        candidate = source.read_bytes()
        if len(candidate) > 4*1024*1024:
            raise ValueError('candidate source exceeds 4 MiB')
        source_sha = hashlib.sha256(candidate).hexdigest()
        headers = context['headers']
        context_sha = context_digest(context)
        original = staged.read_bytes()
        # Remove old products only after preflight succeeds. Never accept one
        # after a failed launch; receipt is published last on success.
        output.unlink(missing_ok=True)
        receipt_path.unlink(missing_ok=True)
        stdout_path.unlink(missing_ok=True)
        stderr_path.unlink(missing_ok=True)
        obj.unlink(missing_ok=True)
        start = time.monotonic()
        try:
            atomic(staged, candidate)
            try:
                result = bounded_process.run(command, cwd=scratch, timeout=timeout)
            except bounded_process.ProcessLimitError as exc:
                atomic(stdout_path, exc.stdout)
                atomic(stderr_path, exc.stderr)
                raise RuntimeError(f'{exc}; diagnostics: {stdout_path}, {stderr_path}\n'
                                   f'{_diagnostics(exc.stdout, exc.stderr)}') from exc
            atomic(stdout_path, result.stdout)
            atomic(stderr_path, result.stderr)
            if result.returncode:
                raise RuntimeError(f'compiler failed ({result.returncode}); diagnostics: '
                                   f'{stdout_path}, {stderr_path}\n'
                                   f'{_diagnostics(result.stdout, result.stderr)}')
            if digest(staged) != source_sha or digest(source) != source_sha:
                raise RuntimeError('source changed during compilation')
            if tree(scratch / 'include') != headers or tree(root / 'include') != headers:
                raise RuntimeError('headers changed during compilation')
            if tree(scratch / 'build/GP6E01/include') != context['generated_headers']:
                raise RuntimeError('generated headers changed during compilation')
            if any(digest(Path(p)) != sha for p, sha in context['tools'].items()):
                raise RuntimeError('compiler context changed during compilation')
            descriptor = context['command_descriptor']
            if descriptor is not None and digest(Path(descriptor['path'])) != descriptor['sha256']:
                raise RuntimeError(f'command JSON changed during compilation: {descriptor["path"]}')
            if not obj.is_file() or not 0 < obj.stat().st_size <= 16*1024*1024:
                raise RuntimeError('compiler did not produce a bounded nonempty object')
            data = obj.read_bytes()
            receipt = {'schema': 'recovery_candidate_compile/v1', 'source_sha256': source_sha,
                       'context_sha256': context_sha, 'object_sha256': hashlib.sha256(data).hexdigest(),
                       'command': command, 'tools': context['tools'],
                       'command_descriptor': descriptor,
                       'header_set_sha256': hashlib.sha256(json.dumps(headers, sort_keys=True).encode()).hexdigest(),
                       'generated_header_set_sha256': hashlib.sha256(json.dumps(context['generated_headers'], sort_keys=True).encode()).hexdigest(),
                       'object_size': len(data), 'seconds': time.monotonic()-start,
                       'stdout_sha256': hashlib.sha256(result.stdout).hexdigest(),
                       'stderr_sha256': hashlib.sha256(result.stderr).hexdigest(),
                       'stdout_path': str(stdout_path), 'stderr_path': str(stderr_path)}
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
    for name in ('root', 'scratch', 'source-relpath', 'object-relpath'):
        parser.add_argument('--'+name, required=True)
    compiler = parser.add_mutually_exclusive_group(required=True)
    compiler.add_argument('--compiler-script', help='existing PowerShell compiler script')
    compiler.add_argument('--command-json', help='UTF-8 JSON argv list; executable paths are scratch-relative or on PATH')
    parser.add_argument('--source')
    parser.add_argument('--output')
    parser.add_argument('--preflight', action='store_true')
    parser.add_argument('--tool', action='append', default=[])
    parser.add_argument('--timeout', type=float, default=120)
    parser.add_argument('--mutex-name', default='Global\\CodexBoardCapspecialCandidateCompile')
    args = parser.parse_args()
    root = Path(args.root).absolute()
    path = lambda raw: root / raw
    try:
        if not math.isfinite(args.timeout) or args.timeout <= 0:
            raise ValueError('positive finite compiler timeout required')
        descriptor = path(args.command_json) if args.command_json else None
        tool_paths = [path(p) for p in args.tool]
        if descriptor is not None:
            command = load_command_json(descriptor)
        else:
            script = _tool_path(path(args.compiler_script), root)
            command = ['powershell.exe', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', str(script)]
            tool_paths.insert(0, script)
        if args.preflight:
            scratch = safe(path(args.scratch), root)
            staged = safe(scratch / args.source_relpath, scratch)
            safe(scratch / args.object_relpath, scratch)
            if not staged.is_file():
                raise ValueError(f'missing scratch source: {staged}')
            context = preflight_context(root=root, scratch=scratch, command=command,
                                        tools=tool_paths, command_descriptor=descriptor)
            print(json.dumps({'status': 'context preflight passed; no compiler launched or source changed',
                              'context_sha256': context_digest(context), 'context': context,
                              'command': context['command'], 'tools': context['tools'],
                              'command_descriptor': context['command_descriptor']}, sort_keys=True))
            return 0
        if not args.source or not args.output:
            parser.error('--source and --output are required unless --preflight is used')
        result = compile_candidate(root=root, scratch=path(args.scratch), source=path(args.source),
                                   output=path(args.output), source_relpath=args.source_relpath,
                                   object_relpath=args.object_relpath, command=command,
                                   tools=tool_paths, timeout=args.timeout,
                                   mutex_name=args.mutex_name, command_descriptor=descriptor)
        print(json.dumps(result, sort_keys=True))
        return 0
    except (OSError, ValueError, RuntimeError) as exc:
        print(f'candidate compile: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
