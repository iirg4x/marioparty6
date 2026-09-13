"""Prepare REL first-pass inputs from an explicitly selected verified link."""
from pathlib import Path
import argparse
import concurrent.futures
import hashlib
import json
import re
import subprocess
import sys

if __package__ in {None, ''}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.crack_evidence_bundle import _parse_elf_structure

DEFAULT_HEADERS = [
    'game/main.h', 'game/object.h', 'game/audio.h', 'game/charman.h',
    'game/gamemes.h', 'game/hsfex.h', 'game/data.h', 'game/gamework.h',
    'game/memory.h', 'game/mg/seqman.h', 'game/mg/timer.h', 'game/mg/score.h',
    'game/pad.h', 'game/frand.h', 'game/sprite.h', 'game/wipe.h',
    'game/mg/actman.h', 'datadir_enum.h', 'string.h', 'math.h',
    # math.h omits exp; this existing provider supplies its real __P prototype.
    'PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h',
]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def prepare(*, root, proof_root, current, flags_json, m2c, output,
            objdiff='C:/Users/Anony/.codex/tools/objdiff/v3.8.0/objdiff-cli.exe'):
    root, proof, current, flags_json, m2c, out = (
        Path(p).resolve() for p in (root, proof_root, current, flags_json, m2c, output))
    if out.exists():
        raise ValueError('output already exists; require a fresh directory: ' + str(out))
    macro = m2c.parent / 'm2c_macros.h'
    for path in (m2c, macro, flags_json, root/'configure.py',
                 current/'build/GP6E01/config.json', current/'build.ninja'):
        if not path.is_file():
            raise ValueError('missing preparation input: ' + str(path))
    config = json.loads((current/'build/GP6E01/config.json').read_text())
    ninja = (current/'build.ninja').read_text().replace('\\', '/').replace('$\n', '')
    flags = json.loads(flags_json.read_text())['flags']
    if not isinstance(flags, list) or not all(isinstance(flag, str) and flag for flag in flags):
        raise ValueError('flags JSON must contain a list of nonempty flag strings')
    out.mkdir(parents=True)
    inputs = out/'inputs'
    inputs.mkdir()

    def run(argv):
        result = subprocess.run(list(map(str, argv)), cwd=proof, text=True,
                                capture_output=True, timeout=90)
        if result.returncode:
            raise RuntimeError((result.stdout+result.stderr)[-2500:])

    def context(name, extra=None):
        source, preprocessed, prefix = (inputs/(name+suffix) for suffix in ('.h', '.i', '-prefix.h'))
        source.write_text(''.join('#include "'+h+'"\n' for h in DEFAULT_HEADERS+([extra] if extra else [])), encoding='utf-8')
        run([proof/'build/tools/sjiswrap.exe', proof/'build/compilers/GC/1.3.2/mwcceppc.exe',
             *flags, '-P', '-EP', source, '-o', preprocessed])
        prefix.write_text(source.read_text(encoding='utf-8')+'\n#include "'+macro.as_posix()+'"\n', encoding='utf-8')
        return str(preprocessed), str(prefix)

    default_context, default_prefix = context('shared-api')
    headers_by_name = {p.name.lower(): p for p in (root/'include/REL').glob('*.h')}
    skipped, rows = [], []
    for module in config['modules']:
        name = module['name']
        target = current/f'build/GP6E01/{name}/{name}.plf'
        line = next(line for line in ninja.splitlines() if line.startswith(f'build build/GP6E01/{name}/{name}.plf:'))
        linked = set(line.split(' | ', 1)[0].split(': link ', 1)[1].split())
        definitions, unit_count = set(), 0
        for unit in module['units']:
            original = unit['object'].replace('\\', '/')
            if original not in linked or not unit.get('code_size'):
                continue
            parsed = _parse_elf_structure(current/original)
            unit_count += 1
            for symbol in parsed['symbols']:
                if (symbol['info'] & 15) == 2 and 0 < symbol['section'] < len(parsed['sections']) and symbol['size']:
                    if symbol['name'].startswith(('__', '_save', '_rest')) or symbol['name'] in ('_prolog', '_epilog'):
                        continue
                    if re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', symbol['name']):
                        definitions.add(symbol['name'])
        target_data = _parse_elf_structure(target)
        symbols = {s['name']: s for s in target_data['symbols'] if (s['info'] & 15) == 2 and s['section'] and s['size']}
        names = sorted(definitions.intersection(symbols), key=lambda n: (symbols[n]['size'], symbols[n]['value']))
        if not names:
            skipped.append({'module': name, 'reason': 'no unselected application/function candidates',
                            'target_sha256': sha(target), 'selected_or_empty': True})
            continue
        rows.append({'name': name, 'target_path': str(target), 'target_sha256': sha(target),
                     'functions': names, 'function_count': len(names), 'unselected_code_units': unit_count,
                     'code_bytes': sum(symbols[n]['size'] for n in names)})

    def prepare_module(row):
        name = row['name']
        assembly = inputs/(name+'.s')
        run([proof/'build/tools/dtk.exe', 'elf', 'disasm', row['target_path'], assembly])
        defined = set(re.findall(r'^\.fn (\w+), (?:global|local)', assembly.read_text(), re.M))
        if not set(row['functions']) <= defined:
            raise ValueError('missing disassembled functions: ' + name)
        row['assembly_paths'] = [str(assembly)]
        header = headers_by_name.get((name+'.h').lower())
        if header:
            row['context_path'], row['prefix_path'] = context(name, header.relative_to(root/'include').as_posix())
        return row

    rows.sort(key=lambda row: row['code_bytes'])
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        rows = list(pool.map(prepare_module, rows))
    count = len(config['modules'])
    manifest = {'schema': 'rel_first_compile_batch/v1', 'root': str(root), 'proof_root': str(proof),
                'm2c': str(m2c), 'objdiff': str(objdiff), 'context_path': default_context,
                'prefix_path': default_prefix, 'flags': flags, 'output_root': str(out/'run'), 'modules': rows,
                'source_watch_sha256': {str(root/'configure.py'): sha(root/'configure.py')},
                'selection': {'config_path': str(current/'build/GP6E01/config.json'),
                              'config_sha256': sha(current/'build/GP6E01/config.json'),
                              'ninja_sha256': sha(current/'build.ninja'),
                              'source_commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip(),
                              'module_count': count, 'skipped_modules': skipped,
                              'known_runtime_and_startup_names_excluded': True,
                              'warning': 'Unnamed runtime functions may remain in census. Raw first-pass scores are never source/whole-module recovery credit.'}}
    path = out/'manifest.json'
    path.write_text(json.dumps(manifest, indent=2)+'\n', encoding='utf-8')
    return {'manifest': str(path), 'modules_total': count, 'modules_with_candidates': len(rows),
            'modules_skipped_selected_or_empty': len(skipped), 'functions': sum(r['function_count'] for r in rows),
            'input_bytes': sum(p.stat().st_size for p in inputs.iterdir()),
            'first_modules': [(r['name'], r['function_count']) for r in rows[:8]]}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('root', 'proof-root', 'current', 'flags-json', 'm2c', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    parser.add_argument('--objdiff', default='C:/Users/Anony/.codex/tools/objdiff/v3.8.0/objdiff-cli.exe')
    args = parser.parse_args(argv)
    print(json.dumps(prepare(**vars(args))))


if __name__ == '__main__':
    main()
