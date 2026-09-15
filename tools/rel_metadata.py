"""Hash-bound structural evidence for linker-only REL targets.

The optional offline inspector reads a target but never publishes its bytes.
Both snapshot builders use the same fail-closed metadata validator. No module
names or zero-size budgets are sufficient to establish an empty target.
"""
from __future__ import annotations

import hashlib
import re
import struct


def text_hash(value):
    return hashlib.sha256(value.replace('\r\n', '\n').encode('utf-8')).hexdigest()


def inspect_rel_target(raw, expected_sha1, splits_text, symbols_text):
    """Decode a hash-verified version-3 REL into non-binary target metadata."""
    digest = hashlib.sha1(raw).hexdigest()
    if digest != expected_sha1.lower() or len(raw) < 76:
        raise ValueError('REL target hash or header is invalid')
    word = lambda offset: struct.unpack_from('>I', raw, offset)[0]
    count, table = word(12), word(16)
    if word(28) != 3 or count > 255 or table < 76 or table + count * 8 > len(raw):
        raise ValueError('Unsupported or truncated REL section table')
    sections = []
    for index in range(count):
        flags, size = struct.unpack_from('>II', raw, table + index * 8)
        offset = flags & ~3
        if size and offset and (offset < table + count * 8 or offset + size > len(raw)):
            raise ValueError('REL section is outside the target payload')
        sections.append({'index': index, 'offset': offset, 'size': size,
                         'executable': bool(flags & 1), 'reservedFlag': bool(flags & 2),
                         'allZero': bool(offset and not any(raw[offset:offset + size]))})
    header = {name: word(offset) for name, offset in (
        ('moduleId', 0), ('next', 4), ('previous', 8), ('nameOffset', 20), ('nameSize', 24),
        ('bssBytes', 32), ('relocationOffset', 36), ('importOffset', 40), ('importBytes', 44),
        ('prologOffset', 52), ('epilogOffset', 56), ('unresolvedOffset', 60),
        ('alignment', 64), ('bssAlignment', 68), ('fixSize', 72))}
    header.update(dict(zip(('prologSection', 'epilogSection', 'unresolvedSection', 'bssSection'), raw[48:52])))
    return {'schemaVersion': 1, 'format': 'REL', 'sha1': digest, 'fileBytes': len(raw),
            'version': word(28), 'sectionCount': count, 'sectionTableOffset': table,
            'header': header, 'sections': sections,
            'fingerprint': {'splits': text_hash(splits_text), 'symbols': text_hash(symbols_text)}}


def verified_empty_evidence(target, expected_sha1, splits_text, symbols_text):
    """Return zero-recoverable-byte evidence only for the validated null-table layout.

    This conservative profile recognizes the nine-slot REL layout emitted by the
    project's pinned DTK toolchain. Nonempty, unfamiliar, incomplete, or stale
    evidence returns None; callers retain their ordinary unavailable state.
    """
    if not isinstance(target, dict) or not isinstance(expected_sha1, str):
        return None
    if not re.fullmatch(r'[0-9a-fA-F]{40}', expected_sha1):
        return None
    if (target.get('schemaVersion') != 1 or target.get('format') != 'REL'
            or target.get('sha1') != expected_sha1.lower() or target.get('version') != 3
            or target.get('sectionCount') != 9 or target.get('sectionTableOffset') != 76
            or target.get('fingerprint') != {'splits': text_hash(splits_text), 'symbols': text_hash(symbols_text)}):
        return None
    header, sections = target.get('header'), target.get('sections')
    if not isinstance(header, dict) or not isinstance(sections, list) or len(sections) != 9:
        return None
    required = ('moduleId', 'next', 'previous', 'nameOffset', 'nameSize', 'bssBytes',
                'relocationOffset', 'importOffset', 'importBytes', 'prologSection',
                'epilogSection', 'unresolvedSection', 'bssSection', 'prologOffset',
                'epilogOffset', 'unresolvedOffset', 'alignment', 'bssAlignment', 'fixSize')
    if any(type(header.get(key)) is not int or header[key] < 0 for key in required):
        return None
    zero_fields = ('next', 'previous', 'nameOffset', 'bssBytes', 'relocationOffset',
                   'importOffset', 'importBytes', 'prologSection', 'epilogSection',
                   'unresolvedSection', 'bssSection', 'prologOffset', 'epilogOffset', 'unresolvedOffset')
    if any(header[key] != 0 for key in zero_fields):
        return None
    if header['alignment'] != 4 or header['bssAlignment'] != 1:
        return None
    end = 76 + len(sections) * 8
    for index, section in enumerate(sections):
        if not isinstance(section, dict) or section.get('index') != index:
            return None
        if any(type(section.get(key)) is not int or section[key] < 0 for key in ('offset', 'size')):
            return None
        if (type(section.get('executable')) is not bool or section.get('reservedFlag') is not False
                or type(section.get('allZero')) is not bool):
            return None
        if index in (2, 3):
            if (section['offset'] != end or section['size'] != 4
                    or section['executable'] or not section['allZero']):
                return None
            end += 4
        elif section['size'] != 0 or section['offset'] not in (0, 148):
            return None
        elif section['executable'] and index != 1:
            return None
    if type(target.get('fileBytes')) is not int or target['fileBytes'] != end or header['fixSize'] != end:
        return None
    # Bind the interpretation of populated section slots to committed metadata.
    declarations = re.findall(r'^\s*(\.[\w.]+)\s+type:(\w+)', splits_text, re.MULTILINE)
    if declarations != [('.ctors', 'rodata'), ('.dtors', 'rodata')]:
        return None
    if re.search(r'\btype:function\b', symbols_text):
        return None
    return {'classification': 'verified-empty-rel', 'targetSha1': expected_sha1.lower(),
            'fileBytes': end, 'linkerBytes': 8, 'recoverableCodeBytes': 0,
            'recoverableDataBytes': 0, 'functionCount': 0, 'imports': 0, 'entrypoints': 0}
