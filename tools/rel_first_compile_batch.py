"""Bounded scratch-only first compiles. Raw objdiff scores are not recovery proof."""
from __future__ import annotations
import argparse
import concurrent.futures as futures
import hashlib
import json
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

LIMIT = 128 * 1024 * 1024
TRANSLATOR_FILE_LIMIT = 256
TRANSLATOR_BYTE_LIMIT = 8 * 1024 * 1024

def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()

def atomic(path, data):
    temp = path.with_suffix(path.suffix + '.tmp')
    temp.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    temp.replace(path)

def placeholders(text):
    return sorted(set(re.findall(r'\b(?:M2C_\w+|GLUE_F64|MULT_HI|MULTU_HI|CLZ)\b', text)))

def parse_report(data, name):
    left = next((s for s in data.get('left', {}).get('symbols', []) if s.get('name') == name), None)
    if left is None:
        raise ValueError('target symbol absent: ' + name)
    right = data.get('right', {}).get('symbols', [])
    idx = left.get('target_symbol')
    candidate = right[idx] if isinstance(idx, int) and 0 <= idx < len(right) else {}
    return {'score': left.get('match_percent'), 'target_size': left.get('size'),
            'size': candidate.get('size'), 'diffcount': sum(i.get('diff_kind') not in (None, 'DIFF_NONE') for i in left.get('instructions', []))}

def eligible(row):
    return (row.get('score') == 100 and row.get('compile_exitcode') == 0
            and row.get('objdiff_exitcode') == 0 and row.get('m2c_exitcode') == 0
            and not row.get('syntax_placeholders') and not row.get('decompile_errors'))

def translator_dependencies(launcher: str | Path) -> list[str]:
    """Bind only the selected launcher, its two Python packages and macro header.

    Standalone scripts without adjacent packages remain supported. Package
    membership is included so adding a module cannot evade final validation.
    """
    launcher = Path(launcher)
    paths, total = [], 0
    def add(path: Path) -> None:
        nonlocal total
        total += path.stat().st_size
        if len(paths) >= TRANSLATOR_FILE_LIMIT or total > TRANSLATOR_BYTE_LIMIT:
            raise ValueError('translator dependency limit exceeded: ' + str(path))
        paths.append(str(path))
    add(launcher)
    for name in ('m2c', 'm2c_pycparser'):
        package = launcher.parent / name
        if not package.exists():
            continue
        if not package.is_dir():
            raise ValueError('translator package is not a directory: ' + str(package))
        for path in package.rglob('*.py'):
            if path.is_file():
                add(path)
    macro = launcher.parent / 'm2c_macros.h'
    if macro.is_file():
        add(macro)
    return sorted(paths)

def bindings(manifest, manifest_path):
    paths = [manifest['context_path'], manifest['prefix_path']]
    proof = Path(manifest['proof_root'])
    # Snapshot once per invocation, including resumes; not a whole tool-tree or
    # per-job scan. Final validation also rejects package/content drift.
    translator = translator_dependencies(manifest['m2c'])
    paths += [*translator, manifest['objdiff'], sys.executable,
              str(proof / 'build/tools/sjiswrap.exe'),
              str(proof / 'build/compilers/GC/1.3.2/mwcceppc.exe')]
    paths += manifest.get('watched_source_paths', [])
    for module in manifest['modules']:
        paths += [module['target_path'], *module['assembly_paths']]
        paths += [module[k] for k in ('context_path', 'prefix_path') if k in module]
    for p, expected in manifest.get('source_watch_sha256', {}).items():
        if sha(p) != expected:
            raise ValueError('source watch drift: ' + p)
        paths.append(p)
    return {'manifest_sha256': sha(manifest_path),
            'translator_dependencies': {'launcher': manifest['m2c'], 'paths': translator},
            'files': {str(p): sha(p) for p in sorted(set(paths))}}

def unchanged(binding):
    translator = binding.get('translator_dependencies')
    if translator:
        try:
            if translator_dependencies(translator['launcher']) != translator['paths']:
                return False
        except (OSError, ValueError):
            return False
    return all(Path(p).is_file() and sha(p) == h for p, h in binding['files'].items())

def job_watch_paths(manifest, module):
    paths = [module['target_path'], *module['assembly_paths'],
             module.get('context_path', manifest['context_path']),
             module.get('prefix_path', manifest['prefix_path'])]
    if manifest.get('frozen_input_schema') == 'rel_frozen_inputs/v1':
        headers = module['header_watch_paths']
        if not set(headers) <= set(manifest['source_watch_sha256']):
            raise ValueError('unbound frozen header dependency')
        paths += headers
    else:
        paths += list(manifest.get('source_watch_sha256', {}))
        paths += manifest.get('watched_source_paths', [])
    return sorted(set(paths))

def clean_scratch(path):
    # Only caller-created worker directories within output_root are passed here.
    path.mkdir(parents=True, exist_ok=True)
    for item in path.iterdir():
        if item.is_dir():
            raise ValueError('unexpected scratch directory: ' + str(item))
        item.unlink()

def run_process(argv, cwd, scratch, timeout=90, report=None):
    stdout, stderr = scratch / 'stdout.log', scratch / 'stderr.log'
    began = time.monotonic()
    reason = ''
    with stdout.open('wb') as out, stderr.open('wb') as err:
        proc = subprocess.Popen(list(map(str, argv)), cwd=cwd, stdout=out, stderr=err)
        while proc.poll() is None:
            if time.monotonic() - began > timeout:
                reason = 'timeout'
            elif stdout.stat().st_size > 4 * 1024 * 1024 or stderr.stat().st_size > 65536:
                reason = 'output_limit'
            elif report and report.exists() and report.stat().st_size > 64 * 1024 * 1024:
                reason = 'report_limit'
            if reason:
                proc.kill()
                proc.wait()
                break
            time.sleep(.05)
    if stdout.stat().st_size > 4 * 1024 * 1024 or stderr.stat().st_size > 65536:
        reason = reason or 'output_limit'
    if report and report.exists() and report.stat().st_size > 64 * 1024 * 1024:
        reason = reason or 'report_limit'
    def tail(p):
        with p.open('rb') as f:
            f.seek(max(0, p.stat().st_size - 1800))
            return f.read(1800).decode('utf-8', errors='replace')
    return proc.returncode, reason, (tail(stdout) + tail(stderr))[-2800:]

def attempt(manifest, module, name, worker, binding):
    rows, src, obj = attempt_functions(manifest, module, [name], None, worker, binding)
    return rows[0], src, obj

def worker_count(value):
    try:
        count = int(value)
    except ValueError:
        raise argparse.ArgumentTypeError('workers must be an integer from 1 to 8')
    if not 1 <= count <= 8:
        raise argparse.ArgumentTypeError('workers must be from 1 to 8')
    return count

def abi_arguments(manifest):
    abi = manifest.get('ppc_abi', 'legacy')
    if abi not in ('legacy', 'gekko-eabi'):
        raise ValueError('unsupported PPC argument ABI: ' + str(abi))
    return ['--ppc-abi', abi] if abi != 'legacy' else []

def group_name_valid(name):
    return isinstance(name, str) and bool(re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', name))

def validate_translation_groups(module):
    raw = module.get('translation_groups')
    if raw is None:
        return []
    if not isinstance(raw, list):
        raise ValueError('translation_groups must be a list')
    valid_functions = set(module.get('functions') or [])
    names = set()
    grouped = set()
    groups = []
    for group in raw:
        if not isinstance(group, dict):
            raise ValueError('translation group must be an object')
        name = group.get('name')
        if not group_name_valid(name):
            raise ValueError('invalid translation group name: %r' % (name,))
        if name in names:
            raise ValueError('duplicate translation group name: ' + name)
        names.add(name)
        functions = group.get('functions')
        if not isinstance(functions, list) or not functions:
            raise ValueError('translation group functions must be a non-empty list: ' + name)
        seen = set()
        normalized = []
        for function in functions:
            if not isinstance(function, str):
                raise ValueError('translation group function must be a string')
            if function not in valid_functions:
                raise ValueError('translation group unknown function: ' + function)
            if function in seen:
                raise ValueError('translation group duplicate function: ' + function)
            if function in grouped:
                raise ValueError('translation function assigned to multiple groups: ' + function)
            seen.add(function)
            grouped.add(function)
            normalized.append(function)
        groups.append({'name': name, 'functions': normalized})
    return groups

def translation_argv(manifest, module, functions):
    function_args = []
    for function in functions:
        function_args.extend(('-f', function))
    return [sys.executable, manifest['m2c'], '-t', 'ppc-mwcc-c', *abi_arguments(manifest),
            '--knr', '--valid-syntax', '--force-decimal', '--stacktrace',
            '--context', manifest['context_path'], *function_args, *module['assembly_paths']]

def attempt_functions(manifest, module, functions, group, worker, binding):
    manifest = {**manifest, **{k: module[k] for k in ('context_path', 'prefix_path') if k in module}}
    scratch = Path(manifest['output_root']) / ('worker-' + str(worker))
    clean_scratch(scratch)
    start = time.monotonic()
    rows = []
    for name in functions:
        rows.append({
            'module': module['name'],
            'function': name,
            'translation_group': group,
            'source_sha256': None,
            'object_sha256': None,
            'target_sha256': binding['files'][module['target_path']],
            'syntax_placeholders': [],
            'm2c_exitcode': None,
            'compile_exitcode': None,
            'objdiff_exitcode': None,
            'score': None,
            'size': None,
            'diffcount': None,
            'diagnostic': ''
        })
    src, obj, report = scratch / 'candidate.c', scratch / 'candidate.o', scratch / 'report.json'
    try:
        watched = job_watch_paths(manifest, module)
        if any(sha(p) != binding['files'][p] for p in watched):
            raise ValueError('input drift before dispatch')
        argv = translation_argv(manifest, module, functions)
        rc, reason, diagnostic = run_process(argv, manifest['proof_root'], scratch)
        row_diagnostic = reason or diagnostic
        for row in rows:
            row.update(m2c_exitcode=rc, diagnostic=row_diagnostic)
        with (scratch / 'stdout.log').open('rb') as draft_file:
            draft = draft_file.read(4 * 1024 * 1024).decode('utf-8', errors='replace')
        syntax_placeholders = placeholders(draft)
        decompile_errors = bool(re.search(r'Error occurred|decompil(?:ation|e) error', draft, re.I))
        for row in rows:
            row['syntax_placeholders'] = syntax_placeholders
            row['decompile_errors'] = decompile_errors
        src.write_text(Path(manifest['prefix_path']).read_text(encoding='utf-8') + '\n' + draft, encoding='utf-8')
        source_hash = sha(src)
        for row in rows:
            row['source_sha256'] = source_hash
        if rc or reason:
            return rows, src, obj
        proof = Path(manifest['proof_root'])
        argv = [proof / 'build/tools/sjiswrap.exe', proof / 'build/compilers/GC/1.3.2/mwcceppc.exe',
                *manifest.get('flags', []), '-c', src, '-o', obj]
        rc, reason, diagnostic = run_process(argv, proof, scratch)
        row_diagnostic = reason or diagnostic
        for row in rows:
            row.update(compile_exitcode=rc, diagnostic=row_diagnostic)
        if rc or reason or not obj.is_file():
            if not rc and not obj.is_file():
                for row in rows:
                    row['diagnostic'] = 'compiler succeeded but object missing'
            return rows, src, obj
        object_hash = sha(obj)
        for row in rows:
            row['object_sha256'] = object_hash
        rc, reason, diagnostic = run_process([manifest['objdiff'], 'diff', '-1', module['target_path'], '-2', obj,
                                              '-o', report, '--format', 'json'], proof, scratch, 45, report)
        row_diagnostic = reason or diagnostic
        for row in rows:
            row.update(objdiff_exitcode=rc, diagnostic=row_diagnostic)
        if not rc and not reason:
            report_data = None
            try:
                report_data = json.loads(report.read_text(encoding='utf-8'))
            except Exception as exc:
                for row in rows:
                    row['diagnostic'] = (type(exc).__name__ + ': ' + str(exc))[-2800:]
            if report_data is not None:
                for row in rows:
                    try:
                        row.update(parse_report(report_data, row['function']))
                    except Exception as exc:
                        row['diagnostic'] = (type(exc).__name__ + ': ' + str(exc))[-2800:]
                        row['score'] = None
        if any(sha(p) != binding['files'][p] for p in watched):
            for row in rows:
                row['score'] = None
            raise ValueError('input drift during attempt')
    except Exception as exc:
        for row in rows:
            row['diagnostic'] = (type(exc).__name__ + ': ' + str(exc))[-2800:]
    finally:
        report.unlink(missing_ok=True)
        for row in rows:
            row['seconds'] = round(time.monotonic() - start, 3)
    return rows, src, obj

def attempt_group(manifest, module, functions, group, worker, binding):
    return attempt_functions(manifest, module, list(functions), group, worker, binding)

def retain_rows(rows, src, obj, retained, out, retained_bytes):
    """Keep one shared pair for a group; resumed members do not pay twice."""
    selected = [row for row in rows if eligible(row)]
    if not selected or not src.is_file() or not obj.is_file():
        return retained_bytes
    first = selected[0]
    group = first.get('translation_group')
    identity = (first['module'] + ':translation_group:' + group if group is not None
                else first['module'] + ':' + first['function'])
    stem = hashlib.sha256(identity.encode()).hexdigest()[:24]
    pairs = [(kind, path, retained / (stem + path.suffix))
             for kind, path in [('source', src), ('object', obj)]]
    added_bytes = 0
    for _, path, dest in pairs:
        if dest.exists():
            if sha(dest) != sha(path):
                raise ValueError('immutable retained artifact collision: ' + str(dest))
        else:
            added_bytes += path.stat().st_size
    if retained_bytes + added_bytes > LIMIT:
        for row in selected:
            row['retention_skipped'] = '128MiB cap'
        return retained_bytes
    for kind, path, dest in pairs:
        if not dest.exists():
            with dest.open('xb') as dest_file, path.open('rb') as source_file:
                shutil.copyfileobj(source_file, dest_file)
        for row in selected:
            row['retained_' + kind] = str(dest.relative_to(out))
    return retained_bytes + added_bytes

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('manifest', type=Path)
    parser.add_argument('--resume', action='store_true')
    parser.add_argument('--workers', type=worker_count, default=2)
    args = parser.parse_args()
    m = json.loads(args.manifest.read_text(encoding='utf-8-sig'))
    if m['schema'] != 'rel_first_compile_batch/v1':
        raise ValueError('unsupported manifest schema')
    abi_arguments(m)
    for key in ('root', 'proof_root', 'm2c', 'objdiff', 'context_path', 'prefix_path', 'output_root'):
        if not Path(m[key]).is_absolute():
            raise ValueError('absolute path required: ' + key)
    out = Path(m['output_root'])
    if not out.resolve().is_relative_to((Path(m['root']) / 'build').resolve()):
        raise ValueError('output_root must be within root/build')
    jobs = [(module, name) for module in m['modules'] for name in module['functions']]
    keys = [(mod['name'], name) for mod, name in jobs]
    known_keys = set(keys)
    if len(known_keys) != len(keys):
        raise ValueError('duplicate module/function')
    for mod in m['modules']:
        for p in [mod['target_path'], *mod['assembly_paths']]:
            if not Path(p).is_absolute():
                raise ValueError('absolute input path required: ' + p)
    groups_by_module = {}
    for module in m['modules']:
        groups_by_module[id(module)] = validate_translation_groups(module)
    binding = bindings(m, args.manifest)
    out.mkdir(parents=True, exist_ok=True)
    snapshot, census = out / 'bindings.json', out / 'census.jsonl'
    done = {}
    if args.resume:
        # bindings() has just re-enumerated and hashed the invocation inputs.
        if json.loads(snapshot.read_text(encoding='utf-8')) != binding:
            raise ValueError('resume input binding mismatch')
        if census.exists():
            for line in census.read_text(encoding='utf-8').splitlines():
                row = json.loads(line)
                key = (row['module'], row['function'])
                if key not in known_keys or key in done:
                    raise ValueError('unknown or duplicate census candidate: ' + repr(key))
                for kind in ('source', 'object'):
                    if row.get('retained_' + kind):
                        p = out / row['retained_' + kind]
                        if not p.is_file() or sha(p) != row[kind + '_sha256']:
                            raise ValueError('retained artifact drift: ' + str(p))
                done[key] = row
    elif snapshot.exists() or census.exists():
        raise ValueError('existing batch; use --resume')
    else:
        atomic(snapshot, binding)
    units = []
    for module in m['modules']:
        groups = groups_by_module[id(module)]
        grouped = {function for group in groups for function in group['functions']}
        for name in module['functions']:
            if name not in grouped and (module['name'], name) not in done:
                units.append(('function', module, name))
        for group in groups:
            if any((module['name'], function) not in done for function in group['functions']):
                units.append(('group', module, group['functions'], group['name']))
    pending = iter(units)
    retained = out / 'retained'
    retained.mkdir(exist_ok=True)
    retained_bytes = sum(p.stat().st_size for p in retained.iterdir() if p.is_file())
    def progress():
        state = {'total': len(jobs), 'done': len(done), 'compiled': sum(r.get('compile_exitcode') == 0 for r in done.values()), 'raw100': sum(eligible(r) for r in done.values()), 'retained_bytes': retained_bytes, 'stopped': (out / 'STOP').exists(), 'draft_only': True, 'workers': args.workers}
        atomic(out / 'progress.json', state)
        return state
    with futures.ThreadPoolExecutor(max_workers=args.workers) as pool, census.open('a', encoding='utf-8') as log:
        active = {}
        def dispatch(worker):
            if (out / 'STOP').exists():
                return
            unit = next(pending, None)
            if unit is None:
                return
            if unit[0] == 'function':
                _, module, name = unit
                active[pool.submit(attempt, m, module, name, worker, binding)] = worker
            else:
                _, module, functions, group_name = unit
                active[pool.submit(attempt_group, m, module, functions, group_name, worker, binding)] = worker
        for worker in range(args.workers):
            dispatch(worker)
        while active:
            completed, _ = futures.wait(active, return_when=futures.FIRST_COMPLETED)
            for future in completed:
                worker = active.pop(future)
                result = future.result()
                if isinstance(result[0], list):
                    rows, src, obj = result
                else:
                    row, src, obj = result
                    rows = [row]
                new_rows = []
                for row in rows:
                    key = (row['module'], row['function'])
                    if key not in done:
                        new_rows.append(row)
                if new_rows:
                    retained_bytes = retain_rows(new_rows, src, obj, retained, out, retained_bytes)
                    for row in new_rows:
                        log.write(json.dumps(row) + '\n')
                        log.flush()
                        done[(row['module'], row['function'])] = row
                state = progress()
                if len(done) % 25 == 0:
                    print(json.dumps(state), flush=True)
                clean_scratch(out / ('worker-' + str(worker)))
                dispatch(worker)
    if not unchanged(binding) or sha(args.manifest) != binding['manifest_sha256']:
        atomic(out / 'final-validation.json', {'valid': False, 'reason': 'input drift'})
        raise ValueError('final batch input validation failed: input drift')
    atomic(out / 'final-validation.json', {'valid': True, 'manifest_sha256': binding['manifest_sha256']})
    print(json.dumps(progress()), flush=True)

if __name__ == '__main__':
    main()
