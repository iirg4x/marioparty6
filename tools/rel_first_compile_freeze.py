"""Freeze a drained first-compile batch's remaining work; never stops a runner."""
from __future__ import annotations
import argparse
import hashlib
import json
import re
import stat
import subprocess
from pathlib import Path

def sha(p):
    h = hashlib.sha256()
    with Path(p).open('rb') as f:
        for chunk in iter(lambda: f.read(1048576), b''):
            h.update(chunk)
    return h.hexdigest()

def no_reparse(p):
    for part in (p, *p.parents):
        if part.exists() and (part.is_symlink() or getattr(part.stat(), 'st_file_attributes', 0) & stat.FILE_ATTRIBUTE_REPARSE_POINT):
            raise ValueError('reparse path forbidden: ' + str(part))

def includes(text):
    return re.findall(r'^\s*#\s*include\s*[<"]([^>"\r\n]+)[>"]', text, re.M)

def remaining(modules, rows):
    done = {(r['module'], r['function']) for r in rows}
    known = {(m['name'], n) for m in modules for n in m['functions']}
    if not done <= known or len(done) != len(rows):
        raise ValueError('census contains unknown or duplicate results')
    return [{**m, 'functions': [n for n in m['functions'] if (m['name'], n) not in done]} for m in modules if any((m['name'], n) not in done for n in m['functions'])]

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('manifest', type=Path)
    ap.add_argument('phase')
    ap.add_argument('--drained', action='store_true', help='Explicit assertion by primary that old runner has exited')
    a = ap.parse_args(argv)
    if not a.drained or not re.fullmatch(r'phase[0-9]+', a.phase):
        raise ValueError('require --drained and a phaseN suffix')
    original = json.loads(a.manifest.read_text(encoding='utf-8-sig'))
    root, proof = Path(original['root']), Path(original['proof_root'])
    base = a.manifest.resolve().parent
    oldout = Path(original['output_root'])
    if not (oldout / 'STOP').is_file():
        raise ValueError('old output_root/STOP must exist; primary must verify drain')
    census = oldout / 'census.jsonl'
    census_hash = sha(census)
    raw = census.read_bytes()
    if raw and not raw.endswith(b'\n'):
        raise ValueError('incomplete census final line')
    rows = [json.loads(line) for line in raw.splitlines()]
    modules = remaining(original['modules'], rows)
    previous = json.loads((oldout / 'bindings.json').read_text(encoding='utf-8'))
    if previous['manifest_sha256'] != sha(a.manifest):
        raise ValueError('original manifest drift')
    for p, h in previous['files'].items():
        if sha(p) != h:
            raise ValueError('original input drift: ' + p)
    phase = base / a.phase
    no_reparse(phase)
    if phase.exists():
        raise ValueError('phase already exists: ' + str(phase))
    phase.mkdir(parents=True)
    inputs = phase / 'inputs'; inputs.mkdir()
    original_hashes, frozen_hashes, copied = {}, {}, {}
    def copy(src, dest, transform=None):
        src, dest = Path(src), Path(dest)
        no_reparse(src); no_reparse(dest)
        if not dest.resolve().is_relative_to(phase.resolve()):
            raise ValueError('destination escape: ' + str(dest))
        before = sha(src)
        data = src.read_bytes()
        if hashlib.sha256(data).hexdigest() != before or sha(src) != before:
            raise ValueError('source changed while reading: ' + str(src))
        original_hashes[str(src)] = before
        if transform:
            data = transform(data)
        dest.parent.mkdir(parents=True, exist_ok=True)
        if dest.exists():
            if dest.read_bytes() != data:
                raise ValueError('snapshot destination collision: ' + str(dest))
        else:
            with dest.open('xb') as f:
                f.write(data)
        if sha(src) != before:
            raise ValueError('source changed while copying: ' + str(src))
        frozen_hashes[str(dest)] = sha(dest)
        copied[str(src)] = str(dest)
        return str(dest)
    flags = list(original['flags'])
    include_roots = []
    for i, flag in enumerate(flags):
        if flag == '-i':
            p = Path(flags[i+1])
            include_roots.append(p if p.is_absolute() else proof / p)
        elif flag.startswith('-i') and flag not in ('-inline', '-ir', '-ipa', '-iso_templates', '-inst') and len(flag) > 2:
            raise ValueError('unsupported attached include flag: ' + flag)
    mapped = {str(p): inputs / ('include-' + str(i)) for i, p in enumerate(include_roots)}
    # Empty trailing roots still appear in argv and must exist.
    for directory in mapped.values():
        directory.mkdir(parents=True, exist_ok=True)
    complete_roots = (root / 'include', proof / 'build/GP6E01/include')
    for p in complete_roots:
        if str(p) not in mapped:
            raise ValueError('expected include root absent: ' + str(p))
        no_reparse(p)
        for file in p.rglob('*'):
            no_reparse(file)
            if file.is_file():
                copy(file, mapped[str(p)] / file.relative_to(p))
    prefixes = {original['prefix_path'], *(m.get('prefix_path', original['prefix_path']) for m in modules)}
    macro = Path(original['m2c']).parent / 'm2c_macros.h'
    macro_dest = inputs / 'm2c_macros.h'
    copy(macro, macro_dest)
    # Walk literal header dependencies from the real prefixes. Only dependencies
    # outside the two complete include trees are copied into the trailing roots.
    queue = [Path(p) for p in prefixes] + [macro]
    seen = set()
    dependencies = {}
    while queue:
        file = queue.pop()
        if str(file) in seen:
            continue
        seen.add(str(file))
        dependencies[str(file)] = []
        for name in includes(file.read_text(encoding='utf-8', errors='replace')):
            rel = Path(name)
            if '..' in rel.parts:
                raise ValueError('parent-traversal include: ' + name)
            options = [rel] if rel.is_absolute() else [file.parent / rel, *(p / rel for p in include_roots)]
            found = next((p for p in options if p.is_file()), None)
            if found is None:
                # Complete include trees contain host-only inactive branches
                # (e.g. stdint.h). No mutable search root survives freezing, so
                # an active missing include will fail compilation, not escape.
                if any(file.resolve().is_relative_to(p.resolve()) for p in complete_roots):
                    continue
                raise ValueError('unresolved literal include ' + name + ' from ' + str(file))
            if found.resolve() == macro.resolve():
                dependencies[str(file)].append(str(macro))
                continue
            if rel.is_absolute():
                raise ValueError('unexpected absolute include: ' + name)
            owner = next((p for p in include_roots if found.resolve().is_relative_to(p.resolve())), None)
            if owner is None:
                raise ValueError('header outside declared include roots: ' + str(found))
            copy(found, mapped[str(owner)] / found.relative_to(owner))
            dependencies[str(file)].append(str(found))
            queue.append(found)
    def header_closure(prefix):
        todo, visited = [prefix, str(macro)], set()
        while todo:
            p = todo.pop()
            if p not in visited:
                visited.add(p)
                todo.extend(dependencies.get(p, []))
        return sorted({copied[p] for p in visited if p != prefix})
    idx = 0
    for i, flag in enumerate(flags):
        if flag == '-i':
            flags[i+1] = str(mapped[str(include_roots[idx])]); idx += 1
    def snapshot(p, prefix=False):
        if str(p) in copied:
            return copied[str(p)]
        dest = inputs / 'artifacts' / (hashlib.sha256(str(p).encode()).hexdigest()[:20] + '-' + Path(p).name)
        def rewrite(data):
            text = data.decode('utf-8')
            for inc in includes(text):
                if Path(inc).name == 'm2c_macros.h':
                    text = text.replace(inc, macro_dest.as_posix())
            return text.encode('utf-8')
        return copy(p, dest, rewrite if prefix else None)
    result = {**original, 'flags': flags, 'modules': modules, 'output_root': str(phase / 'run')}
    result.pop('watched_source_paths', None)
    result['context_path'] = snapshot(original['context_path'])
    result['prefix_path'] = snapshot(original['prefix_path'], True)
    for m in modules:
        m['header_watch_paths'] = header_closure(m.get('prefix_path', original['prefix_path']))
        m['target_path'] = snapshot(m['target_path'])
        m['assembly_paths'] = [snapshot(p) for p in m['assembly_paths']]
        if 'context_path' in m: m['context_path'] = snapshot(m['context_path'])
        if 'prefix_path' in m: m['prefix_path'] = snapshot(m['prefix_path'], True)
        m['function_count'] = len(m['functions'])
    configure = snapshot(root / 'configure.py')
    commit = subprocess.run(['git', '-C', str(root), 'rev-parse', 'HEAD'], check=True, capture_output=True, text=True, timeout=15).stdout.strip()
    result['source_commit'] = commit
    result['configure_snapshot_sha256'] = sha(configure)
    result['source_watch_sha256'] = frozen_hashes
    result['frozen_input_schema'] = 'rel_frozen_inputs/v1'
    result['continuation'] = {'prior_manifest': str(a.manifest.resolve()), 'prior_manifest_sha256': sha(a.manifest), 'prior_census': str(census), 'prior_census_sha256': census_hash, 'prior_completed': len(rows), 'remaining': sum(len(m['functions']) for m in modules), 'original_snapshot_sha256': original_hashes, 'warning': 'Raw first-compile results only; prior census is separate, not new work.'}
    for p, h in original_hashes.items():
        if sha(p) != h:
            raise ValueError('source drift before publication: ' + p)
    if sha(census) != census_hash:
        raise ValueError('census changed; original runner was not drained')
    dest = phase / 'manifest.json'
    temp = phase / 'manifest.json.tmp'
    temp.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    temp.replace(dest)
    print(json.dumps({'manifest': str(dest), 'remaining': result['continuation']['remaining'], 'prior_completed': len(rows), 'prior_census_sha256': census_hash}))

if __name__ == '__main__':
    main()
