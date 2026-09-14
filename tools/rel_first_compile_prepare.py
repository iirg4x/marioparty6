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
from tools import decompctx

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


def provider_headers(discovery):
    """Choose only unique external-call header providers, never another REL's locals."""
    selected, unresolved = {}, {}
    for name, candidates in sorted(discovery['missing'].items()):
        paths = sorted({row['path'][len('include/'):] for row in candidates
                        if row.get('role') == 'header' and row['path'].startswith('include/')})
        if len(paths) == 1:
            selected.setdefault(paths[0], []).append(name)
        else:
            unresolved[name] = {'reason': 'ambiguous header providers' if paths else 'no header provider',
                                'headers': paths}
    return selected, unresolved


def duplicate_provider_definitions(base_context, expanded_context):
    """Preprocessing does not diagnose duplicate bodies introduced by an include."""
    before = decompctx.declared_functions(base_context)
    after = decompctx.declared_functions(expanded_context)
    return sorted(name for name, rows in after.items()
                  if sum(row['definition'] for row in rows) > max(1, sum(
                      row['definition'] for row in before.get(name, []))))


def provider_conflicts(base_context, expanded_context):
    before = decompctx.declared_functions(base_context)
    after = decompctx.declared_functions(expanded_context)
    conflicts = set(duplicate_provider_definitions(base_context, expanded_context))
    # Conservative textual comparison: alternate parameter names may require
    # review, but differing declarations must never be silently selected.
    shape = lambda row: re.sub(r'\b(?:extern|static|inline)\b|\s+', '', row['declaration'])
    for name in before.keys() & after.keys():
        old = {shape(row) for row in before[name]}
        if any(shape(row) not in old for row in after[name]):
            conflicts.add(name)
    return sorted(conflicts)


def builtin_declaration_supplement(context, names):
    """Copy exact preprocessed declarations only when no extra type context is needed."""
    declarations = decompctx.declared_functions(context)
    lines = []
    for name in names:
        rows = declarations.get(name, [])
        unique = {row['declaration'] for row in rows if row['prototyped'] and not row['definition']}
        if len(unique) != 1 or any(row['definition'] for row in rows):
            return None
        declaration = next(iter(unique))
        rest = re.sub(r'\b'+re.escape(name)+r'\b', '', declaration)
        if not set(re.findall(r'[A-Za-z_]\w*', rest)) <= {
            'extern', 'const', 'volatile', 'void', 'char', 'short', 'int',
            'long', 'signed', 'unsigned', 'float', 'double'}:
            return None
        lines.append(declaration+';')
    return '\n'.join(lines)+'\n'


def extend_call_context(*, root, assembly, context_path, prefix_path, output_base, preprocess, compiler_flags):
    """Add uniquely discovered providers under the caller's unchanged compiler flags.

    Conflicting headers are not included. A narrow declaration-only fallback
    copies builtin-only prototypes into m2c context, not the source prefix.
    """
    text = Path(context_path).read_text(encoding='utf-8')
    prefix = Path(prefix_path).read_text(encoding='utf-8')
    discovery = decompctx.discover_call_context(Path(root), assembly, text, include_shapes=False)
    providers, unresolved = provider_headers(discovery)
    records, supplements = [], []
    for number, (header, names) in enumerate(sorted(providers.items())):
        provider = Path(root)/'include'/header
        provider_sha = sha(provider)
        source = Path(str(output_base)+f'-provider-{number}.h')
        expanded = source.with_suffix('.i')
        source.write_text(prefix+'\n#include "'+provider.resolve().as_posix()+'"\n', encoding='utf-8')
        failed_include = None
        try:
            preprocess(source, expanded)
        except RuntimeError as exc:
            # A conflicting macro can stop preprocessing before we see any
            # declarations. Do not undefine it: inspect the provider alone
            # with the same compiler flags for context-only builtin prototypes.
            failed_include = {'source_path': str(source), 'source_sha256': sha(source),
                              'diagnostic': str(exc)[-1500:]}
            source = Path(str(output_base)+f'-provider-{number}-standalone.h')
            expanded = source.with_suffix('.i')
            source.write_text('#include "'+provider.resolve().as_posix()+'"\n', encoding='utf-8')
            try:
                preprocess(source, expanded)
            except RuntimeError as standalone_error:
                records.append({'header': header, 'calls': names, 'provider_sha256': provider_sha,
                                'mode': 'unresolved-provider-preprocessing',
                                'diagnostic': str(standalone_error)[-1500:]})
                continue
        if sha(provider) != provider_sha:
            raise ValueError('provider changed during preprocessing: '+str(provider))
        expanded_text = expanded.read_text(encoding='utf-8')
        conflicts = provider_conflicts(text, expanded_text+'\n'+''.join(supplements))
        visible = decompctx.declared_functions(expanded_text)
        record = {'header': header, 'calls': names, 'provider_sha256': provider_sha,
                  'source_path': str(source), 'source_sha256': sha(source),
                  'preprocessed_path': str(expanded), 'preprocessed_sha256': sha(expanded),
                  'conflicts': conflicts,
                  'preprocessing_environment': 'standalone-provider-same-flags' if failed_include else 'source-prefix-same-flags'}
        if failed_include:
            record['failed_include'] = failed_include
        if not all(any(r['prototyped'] for r in visible.get(name, [])) for name in names):
            record['mode'] = 'unresolved-conditional-provider'
        elif conflicts or failed_include:
            supplement = builtin_declaration_supplement(expanded_text, names)
            if supplement is None:
                record['mode'] = 'unresolved-provider-conflict'
            else:
                record['mode'] = 'm2c-context-only-declarations'
                record['declarations'] = supplement
                record['omitted_provider_bodies'] = sorted(name for name, rows in visible.items()
                                                           if any(r['definition'] for r in rows))
                supplements.append(supplement)
                text += '\n'+supplement
        else:
            record['mode'] = 'header-include'
            prefix += '\n#include "'+header+'"\n'
            text = expanded_text+'\n'+''.join(supplements)
        records.append(record)
    result = {'providers': records, 'unresolved_external_calls': unresolved,
              'unresolved_local_calls': discovery['local_prototype_review']['unresolved'],
              'base_context_sha256': sha(context_path), 'base_prefix_sha256': sha(prefix_path),
              'compiler_flags_sha256': hashlib.sha256(json.dumps(compiler_flags).encode()).hexdigest()}
    if not any(row['mode'] in {'header-include', 'm2c-context-only-declarations'} for row in records):
        return str(context_path), str(prefix_path), result
    final_context = Path(str(output_base)+'-calls.i')
    final_prefix = Path(str(output_base)+'-calls-prefix.h')
    final_context.write_text(text, encoding='utf-8')
    final_prefix.write_text(prefix, encoding='utf-8')
    result['context_sha256'] = sha(final_context)
    result['prefix_sha256'] = sha(final_prefix)
    return str(final_context), str(final_prefix), result


def prepare(*, root, proof_root, current, flags_json, m2c, output,
            objdiff='C:/Users/Anony/.codex/tools/objdiff/v3.8.0/objdiff-cli.exe',
            ppc_abi='legacy'):
    if ppc_abi not in ('legacy', 'gekko-eabi'):
        raise ValueError('unsupported PPC argument ABI: '+str(ppc_abi))
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

    def preprocess(source, preprocessed):
        run([proof/'build/tools/sjiswrap.exe', proof/'build/compilers/GC/1.3.2/mwcceppc.exe',
             *flags, '-P', '-EP', source, '-o', preprocessed])

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
        row['context_path'], row['prefix_path'], row['call_context'] = extend_call_context(
            root=root, assembly=assembly.read_text(),
            context_path=row.get('context_path', default_context),
            prefix_path=row.get('prefix_path', default_prefix),
            output_base=inputs/name, preprocess=preprocess, compiler_flags=flags)
        return row

    rows.sort(key=lambda row: row['code_bytes'])
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        rows = list(pool.map(prepare_module, rows))
    count = len(config['modules'])
    manifest = {'schema': 'rel_first_compile_batch/v1', 'root': str(root), 'proof_root': str(proof),
                'm2c': str(m2c), 'ppc_abi': ppc_abi, 'objdiff': str(objdiff), 'context_path': default_context,
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
    parser.add_argument('--ppc-abi', choices=('legacy', 'gekko-eabi'), default='legacy',
                        help='Select the verified translator argument profile; legacy keeps older m2c compatible.')
    args = parser.parse_args(argv)
    print(json.dumps(prepare(**vars(args))))


if __name__ == '__main__':
    main()
