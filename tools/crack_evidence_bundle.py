#!/usr/bin/env python3
"""Generate the canonical, real-object evidence bundle for one crack cell.

The command is intentionally phase based.  ``baseline`` runs before the
candidate source overlay and seals the retail object, compiled baseline object,
and both objdiff channels.  ``candidate`` verifies that sealed baseline,
rebuilds the selected unit after the overlay, and adds candidate reports plus an
independent ELF relocation receipt.  All output is confined to the explicitly
provided harness output root.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import stat
import struct
import subprocess
import sys
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.crack_contract import is_closed_objdiff_unit_name


SCHEMA = "crack_evidence_bundle_context/v1"
PHASE_RECEIPT_SCHEMA = "crack_evidence_phase_receipt/v1"
PHYSICAL_SCHEMA = "mp6_physical_relocation_receipt/v1"
CONTEXT_SCHEMA = "crack_evidence_context/v1"
OBJDFF_VERSION = "3.8.0"
OBJDFF_SHA256 = "3023818f7fdd2f2dc6ade16e68d2c37f5f5754f96881d18d68ddfce77ced15e1"
DEFAULT_OBJDFF = Path(r"C:\Users\Anony\.codex\tools\objdiff\v3.8.0\objdiff-cli.exe")
DEFAULT_READELF = Path(r"C:\Users\Anony\.codex\tools\mp6\binutils-2.42-1\powerpc-eabi-readelf.exe")
DEFAULT_NINJA = Path(r"C:\Users\Anony\.codex\tools\mp6\ninja-v1.13.2.exe")
DEFAULT_TOOLCHAIN_MANIFEST = Path(r"C:\Users\Anony\.codex\tools\mp6\toolchain.json")
BINUTILS_TAG = "2.42-1"
NINJA_VERSION = "1.13.2"
NINJA_SHA256 = "e52a7ad9538d9618c67a0bd777964e2eec8a30f68b810a2f6adce1f2daf847b8"
SHA_RE = re.compile(r"[0-9a-f]{64}")
REPARSE_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)


class EvidenceError(ValueError):
    """Evidence could not be produced without guessing."""


def _canonical(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def _json_sha(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _file_sha(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _descriptor(path: Path) -> dict[str, Any]:
    return {"path": str(path.resolve()), "sha256": _file_sha(path), "size_bytes": path.stat().st_size}


def _compact_descriptor(path: Path) -> dict[str, Any]:
    return {"sha256": _file_sha(path), "size_bytes": path.stat().st_size}


def _tree_descriptor(path: Path) -> dict[str, Any]:
    if not path.is_dir() or path.is_symlink():
        raise EvidenceError(f"toolchain directory is missing or indirect: {path}")
    digest = hashlib.sha256()
    count = 0
    size = 0
    for child in sorted(path.rglob("*")):
        if child.is_symlink():
            raise EvidenceError(f"toolchain directory contains an indirection: {child}")
        if not child.is_file():
            continue
        relative = child.relative_to(path).as_posix()
        child_sha = _file_sha(child)
        digest.update(relative.encode("utf-8"))
        digest.update(bytes.fromhex(child_sha))
        count += 1
        size += child.stat().st_size
    return {"path": str(path.resolve()), "tree_sha256": digest.hexdigest(), "file_count": count, "size_bytes": size}


def _atomic_copy(source: Path, destination: Path) -> None:
    _assert_no_indirection(source)
    _assert_no_indirection(destination.parent)
    if destination.exists() or destination.is_symlink():
        _assert_no_indirection(destination)
    temp = destination.with_name(destination.name + ".tmp")
    if temp.exists() or temp.is_symlink():
        _assert_no_indirection(temp)
        temp.unlink()
    shutil.copyfile(source, temp)
    _assert_no_indirection(temp)
    _assert_no_indirection(destination.parent)
    os.replace(temp, destination)


def _atomic_json(destination: Path, value: Mapping[str, Any]) -> None:
    _assert_no_indirection(destination.parent)
    if destination.exists() or destination.is_symlink():
        _assert_no_indirection(destination)
    temp = destination.with_name(destination.name + ".tmp")
    if temp.exists() or temp.is_symlink():
        _assert_no_indirection(temp)
        temp.unlink()
    temp.write_bytes(_canonical(value))
    _assert_no_indirection(temp)
    _assert_no_indirection(destination.parent)
    os.replace(temp, destination)


def _inside(root: Path, path: Path, label: str) -> Path:
    root = root.resolve()
    resolved = path.resolve()
    try:
        resolved.relative_to(root)
    except ValueError as exc:
        raise EvidenceError(f"{label} escapes {root}: {resolved}") from exc
    return resolved


def _assert_no_indirection(path: Path, *, missing_leaf: bool = False) -> None:
    """Reject symlink/reparse components before any path resolution."""

    absolute = Path(os.path.abspath(path))
    current = Path(absolute.parts[0])
    for part in absolute.parts[1:]:
        current /= part
        try:
            info = current.lstat()
        except FileNotFoundError:
            if missing_leaf:
                return
            raise EvidenceError(f"path component does not exist: {current}")
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise EvidenceError(f"path indirection is forbidden: {current}")


def _prepare_output_root(root: Path, out_root: Path) -> tuple[Path, Path]:
    """Create one real output directory lexically and physically under root."""

    root_absolute = Path(os.path.abspath(root))
    out_absolute = Path(os.path.abspath(out_root))
    _assert_no_indirection(root_absolute)
    try:
        out_absolute.relative_to(root_absolute)
    except ValueError as exc:
        raise EvidenceError(
            f"output root escapes repository: {out_absolute}"
        ) from exc
    _assert_no_indirection(out_absolute, missing_leaf=True)
    if out_absolute.exists() and not out_absolute.is_dir():
        raise EvidenceError("output root must be a real directory")
    out_absolute.mkdir(parents=True, exist_ok=True)
    _assert_no_indirection(out_absolute)
    root_resolved = root_absolute.resolve()
    out_resolved = out_absolute.resolve()
    try:
        out_resolved.relative_to(root_resolved)
    except ValueError as exc:
        raise EvidenceError(
            f"output root escapes repository: {out_resolved}"
        ) from exc
    return root_resolved, out_resolved


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or SHA_RE.fullmatch(value) is None:
        raise EvidenceError(f"{label} must be a lowercase SHA-256")
    return value


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise EvidenceError(f"{label} must be nonempty text")
    return value.strip()


def _load_json(path: Path, label: str) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_bytes())
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise EvidenceError(f"invalid {label} {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise EvidenceError(f"{label} must be a JSON object")
    return value


def _run(command: Sequence[str], *, cwd: Path, label: str) -> str:
    result = subprocess.run(list(command), cwd=cwd, text=True, capture_output=True, check=False)
    if result.returncode:
        detail = result.stderr.strip() or result.stdout.strip() or "no diagnostic"
        raise EvidenceError(f"{label} failed ({result.returncode}): {detail[:1000]}")
    return result.stdout


def _verify_objdiff(path: Path) -> dict[str, Any]:
    if not path.is_file():
        raise EvidenceError(f"pinned objdiff is missing: {path}")
    actual = _file_sha(path)
    if actual != OBJDFF_SHA256:
        raise EvidenceError(f"objdiff SHA-256 drifted: {actual} != {OBJDFF_SHA256}")
    version = _run([str(path), "--version"], cwd=path.parent, label="objdiff version").strip()
    if version != f"objdiff-cli.exe {OBJDFF_VERSION}" and version != f"objdiff-cli {OBJDFF_VERSION}":
        raise EvidenceError(f"objdiff version drifted: {version!r}")
    result = _descriptor(path)
    result.update({"version": OBJDFF_VERSION, "expected_sha256": OBJDFF_SHA256})
    return result


def _verify_readelf(path: Path) -> dict[str, Any]:
    if not path.is_file():
        raise EvidenceError(f"pinned PowerPC readelf is missing: {path}")
    version = _run([str(path), "--version"], cwd=path.parent, label="readelf version").splitlines()
    first = version[0] if version else ""
    if "GNU readelf" not in first or "2.42" not in first:
        raise EvidenceError(f"PowerPC readelf is not pinned binutils {BINUTILS_TAG}: {first!r}")
    result = _descriptor(path)
    result.update({"binutils_tag": BINUTILS_TAG, "version_line": first})
    return result


def _verify_ninja(path: Path) -> dict[str, Any]:
    if not path.is_file():
        raise EvidenceError(f"pinned Ninja is missing: {path}")
    actual = _file_sha(path)
    if actual != NINJA_SHA256:
        raise EvidenceError(f"Ninja SHA-256 drifted: {actual} != {NINJA_SHA256}")
    version = _run([str(path), "--version"], cwd=path.parent, label="Ninja version").strip()
    if version != NINJA_VERSION:
        raise EvidenceError(f"Ninja version drifted: {version!r}")
    result = _descriptor(path)
    result.update({"version": NINJA_VERSION, "expected_sha256": NINJA_SHA256})
    return result


def _load_environment(root: Path, out_root: Path, explicit_context: Path) -> dict[str, Any]:
    required = (
        "CRACK_HARNESS_PHASE", "CRACK_HARNESS_OWNER", "CRACK_HARNESS_FUNCTION",
        "CRACK_HARNESS_UNIT", "CRACK_HARNESS_SOURCE_PATH", "CRACK_HARNESS_TARGET_SHA256",
        "CRACK_HARNESS_BASE_COMMIT", "CRACK_HARNESS_APPROVAL_SHA256",
        "CRACK_HARNESS_CONTEXT_SHA256",
        "CRACK_HARNESS_ISSUED_AT", "CRACK_HARNESS_PHASE_NONCE",
    )
    missing = [name for name in required if not os.environ.get(name)]
    if missing:
        raise EvidenceError("missing harness evidence environment: " + ", ".join(missing))
    phase = os.environ["CRACK_HARNESS_PHASE"]
    if phase not in {"baseline", "candidate"}:
        raise EvidenceError("phase must be baseline or candidate")
    unit = os.environ["CRACK_HARNESS_UNIT"]
    if not is_closed_objdiff_unit_name(unit):
        raise EvidenceError("unit is not a closed objdiff unit name")
    source_rel = Path(os.environ["CRACK_HARNESS_SOURCE_PATH"])
    if source_rel.is_absolute():
        source = _inside(root, source_rel, "source")
    else:
        source = _inside(root, root / source_rel, "source")
    if not source.is_file():
        raise EvidenceError(f"selected source is missing: {source}")
    target_sha = _sha(os.environ["CRACK_HARNESS_TARGET_SHA256"], "target SHA-256")
    context_path = _inside(out_root, explicit_context, "approval context")
    expected_context_sha = _sha(os.environ["CRACK_HARNESS_CONTEXT_SHA256"], "context SHA-256")
    if not context_path.is_file():
        raise EvidenceError("approval context is missing")
    context = _load_json(context_path, "approval context")
    if context.get("schema") != CONTEXT_SCHEMA:
        raise EvidenceError(f"approval context schema is not {CONTEXT_SCHEMA}")
    unsigned_context = dict(context)
    embedded_context_sha = unsigned_context.pop("context_sha256", None)
    if embedded_context_sha != expected_context_sha or _json_sha(unsigned_context) != expected_context_sha:
        raise EvidenceError("approval context internal hash drifted")
    checks = {
        "owner": os.environ["CRACK_HARNESS_OWNER"],
        "function": os.environ["CRACK_HARNESS_FUNCTION"],
        "unit": unit,
        "source_relpath": source.relative_to(root).as_posix(),
        "target_sha256": target_sha,
        "base_commit": os.environ["CRACK_HARNESS_BASE_COMMIT"],
        "approval_sha256": _sha(os.environ["CRACK_HARNESS_APPROVAL_SHA256"], "approval SHA-256"),
        "issued_at": os.environ["CRACK_HARNESS_ISSUED_AT"],
    }
    for key, expected in checks.items():
        if context.get(key) != expected:
            raise EvidenceError(f"approval context {key} drifted")
    source_key = "base_source_sha256" if phase == "baseline" else "candidate_source_sha256"
    expected_source_sha = _sha(context.get(source_key), f"approval context {source_key}")
    if _file_sha(source) != expected_source_sha:
        raise EvidenceError(f"{phase} source SHA-256 drifted")
    nonce = _sha(os.environ["CRACK_HARNESS_PHASE_NONCE"], "phase nonce")
    expected_nonce = hashlib.sha256(f"{expected_context_sha}:{phase}".encode("utf-8")).hexdigest()
    if nonce != expected_nonce:
        raise EvidenceError("phase nonce drifted")
    return {
        "phase": phase, "owner": checks["owner"], "function": checks["function"],
        "unit": unit, "source": source, "source_relpath": checks["source_relpath"],
        "target_sha256": target_sha, "base_commit": checks["base_commit"],
        "approval_sha256": checks["approval_sha256"], "issued_at": checks["issued_at"],
        "toolchain_key": _sha(context.get("toolchain_key"), "context toolchain_key"),
        "context_path": context_path, "context_sha256": expected_context_sha,
        "phase_nonce": nonce,
    }


def _load_toolchain(manifest_path: Path, expected_key: str) -> dict[str, Any]:
    if not manifest_path.is_file() or manifest_path.is_symlink():
        raise EvidenceError(f"central crack toolchain manifest is missing: {manifest_path}")
    manifest = _load_json(manifest_path, "central crack toolchain manifest")
    if manifest.get("schema") != "mp6_crack_toolchain/v1":
        raise EvidenceError("central crack toolchain schema drifted")
    unsigned = dict(manifest)
    manifest_sha = unsigned.pop("manifest_sha256", None)
    if manifest_sha != _json_sha(unsigned) or manifest_sha != expected_key:
        raise EvidenceError("central crack toolchain manifest hash/key drifted")
    result: dict[str, Any] = {"manifest": _descriptor(manifest_path), "manifest_sha256": manifest_sha}
    for key in ("objdiff", "dtk", "sjiswrap", "ninja"):
        row = manifest.get(key)
        if not isinstance(row, Mapping) or not isinstance(row.get("path"), str):
            raise EvidenceError(f"toolchain {key} descriptor is invalid")
        path = Path(row["path"]).resolve()
        actual = _descriptor(path)
        if actual["sha256"] != row.get("sha256") or actual["size_bytes"] != row.get("size_bytes"):
            raise EvidenceError(f"toolchain {key} file drifted")
        result[key] = {**actual, "path_object": path}
    for key in ("binutils", "compilers", "orig"):
        row = manifest.get(key)
        if not isinstance(row, Mapping) or not isinstance(row.get("path"), str):
            raise EvidenceError(f"toolchain {key} descriptor is invalid")
        path = Path(row["path"]).resolve()
        actual = _tree_descriptor(path)
        for field in ("tree_sha256", "file_count", "size_bytes"):
            if actual[field] != row.get(field):
                raise EvidenceError(f"toolchain {key} tree drifted")
        result[key] = {**actual, "path_object": path}
    result["public"] = {
        key: {name: value for name, value in row.items() if name != "path_object"}
        for key, row in result.items() if isinstance(row, Mapping)
    }
    return result


def _remove_staged_retail(retail_copy: Path) -> None:
    """Remove staged retail bytes while preserving the tracked placeholder."""
    for child in list(retail_copy.iterdir()) if retail_copy.exists() else []:
        if child.name == ".gitkeep" and child.is_file() and not child.is_symlink():
            continue
        if child.is_dir() and not child.is_symlink():
            shutil.rmtree(child)
        else:
            child.unlink(missing_ok=True)


def _ensure_configured(root: Path, toolchain: Mapping[str, Any], ninja: Path) -> Path:
    build_ninja = root / "build.ninja"
    objdiff_config = root / "objdiff.json"
    orig = root / "orig"
    allowed_existing = {orig / "GP6E01" / ".gitkeep"}
    existing = {path for path in orig.rglob("*") if path.is_file()} if orig.exists() else set()
    if existing - allowed_existing or any(path.is_symlink() for path in orig.rglob("*")):
        raise EvidenceError("detached worktree contains unsealed preexisting orig input")
    # Retail input is copied only into the disposable worktree.  The public
    # phase wrapper removes it after all build/proof work, on every exit path.
    orig.mkdir(exist_ok=True)
    retail_copy = orig / "GP6E01"
    retail_copy.mkdir(parents=True, exist_ok=True)
    try:
        shutil.copytree(
            toolchain["orig"]["path_object"], retail_copy,
            copy_function=shutil.copyfile, dirs_exist_ok=True,
        )
        if not build_ninja.is_file() or not objdiff_config.is_file():
            configure = [
                sys.executable, "configure.py", "configure",
                "--binutils", str(toolchain["binutils"]["path_object"]),
                "--compilers", str(toolchain["compilers"]["path_object"]),
                "--dtk", str(toolchain["dtk"]["path_object"]),
                "--sjiswrap", str(toolchain["sjiswrap"]["path_object"]),
            ]
            _run(configure, cwd=root, label="detached worktree configure")
            _run([ninja, "-j1", "build/GP6E01/config.json"], cwd=root, label="retail object split")
            _run(configure, cwd=root, label="detached worktree reconfigure")
    except BaseException:
        _remove_staged_retail(retail_copy)
        raise
    if not build_ninja.is_file() or not objdiff_config.is_file():
        _remove_staged_retail(retail_copy)
        raise EvidenceError("configuration did not publish build.ninja and objdiff.json")
    return retail_copy


def _unit_paths(root: Path, unit_name: str) -> tuple[Path, Path]:
    config_path = root / "objdiff.json"
    config = _load_json(config_path, "objdiff config")
    units = config.get("units")
    if not isinstance(units, list):
        raise EvidenceError("objdiff config units must be an array")
    matches = [row for row in units if isinstance(row, Mapping) and row.get("name") == unit_name]
    if len(matches) != 1:
        raise EvidenceError(f"objdiff unit {unit_name!r} resolved {len(matches)} times")
    row = matches[0]
    target_raw, candidate_raw = row.get("target_path"), row.get("base_path")
    if not isinstance(target_raw, str) or not isinstance(candidate_raw, str):
        raise EvidenceError("selected objdiff unit lacks target_path/base_path")
    return _inside(root, root / target_raw, "target object"), _inside(root, root / candidate_raw, "candidate object")


def _run_objdiff(objdiff: Path, target: Path, candidate: Path, output: Path, *, data: bool, root: Path) -> None:
    _assert_no_indirection(target)
    _assert_no_indirection(candidate)
    _assert_no_indirection(output.parent)
    if output.exists() or output.is_symlink():
        _assert_no_indirection(output)
    temp = output.with_name(output.name + ".tmp")
    if temp.exists() or temp.is_symlink():
        _assert_no_indirection(temp)
        temp.unlink()
    command = [str(objdiff), "diff", "-1", str(target), "-2", str(candidate), "-o", str(temp), "--format", "json-pretty"]
    if data:
        command += ["-c", "functionRelocDiffs=data_value"]
    _run(command, cwd=root, label="objdiff data" if data else "objdiff strict")
    document = _load_json(temp, "objdiff report")
    if not isinstance(document.get("left"), Mapping) or not isinstance(document.get("right"), Mapping):
        raise EvidenceError("objdiff report lacks real left/right object evidence")
    _assert_no_indirection(temp)
    _assert_no_indirection(output.parent)
    os.replace(temp, output)


def _elf_u16(data: bytes, offset: int) -> int:
    return struct.unpack_from(">H", data, offset)[0]


def _elf_u32(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def _cstring(data: bytes, offset: int) -> str:
    end = data.find(b"\0", offset)
    if end < 0:
        raise EvidenceError("unterminated ELF string")
    return data[offset:end].decode("utf-8", errors="strict")


def _parse_elf_relocations(path: Path, function: str) -> dict[str, Any]:
    data = path.read_bytes()
    if len(data) < 52 or data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 2:
        raise EvidenceError(f"{path} is not a big-endian ELF32 object")
    if _elf_u16(data, 18) != 20:
        raise EvidenceError(f"{path} is not PowerPC ELF")
    shoff, shentsize, shnum, shstrndx = _elf_u32(data, 32), _elf_u16(data, 46), _elf_u16(data, 48), _elf_u16(data, 50)
    if shentsize != 40 or shnum == 0 or shstrndx >= shnum:
        raise EvidenceError("unsupported ELF section table")
    sections: list[dict[str, Any]] = []
    for index in range(shnum):
        off = shoff + index * shentsize
        if off + 40 > len(data):
            raise EvidenceError("truncated ELF section table")
        sections.append({
            "name_off": _elf_u32(data, off), "type": _elf_u32(data, off + 4),
            "offset": _elf_u32(data, off + 16), "size": _elf_u32(data, off + 20),
            "link": _elf_u32(data, off + 24), "info": _elf_u32(data, off + 28),
            "entsize": _elf_u32(data, off + 36),
        })
    shstr = sections[shstrndx]
    shnames = data[shstr["offset"]:shstr["offset"] + shstr["size"]]
    for section in sections:
        section["name"] = _cstring(shnames, section["name_off"])
    symtabs = [(i, row) for i, row in enumerate(sections) if row["type"] == 2]
    if len(symtabs) != 1:
        raise EvidenceError("ELF must contain exactly one symbol table")
    sym_index, symtab = symtabs[0]
    if symtab["entsize"] != 16 or symtab["link"] >= shnum:
        raise EvidenceError("unsupported ELF symbol table")
    strtab = sections[symtab["link"]]
    strings = data[strtab["offset"]:strtab["offset"] + strtab["size"]]
    symbols: list[dict[str, Any]] = []
    for off in range(symtab["offset"], symtab["offset"] + symtab["size"], 16):
        if off + 16 > len(data):
            raise EvidenceError("truncated ELF symbol table")
        symbols.append({
            "name": _cstring(strings, _elf_u32(data, off)), "value": _elf_u32(data, off + 4),
            "size": _elf_u32(data, off + 8), "info": data[off + 12], "section": _elf_u16(data, off + 14),
        })
    matches = [row for row in symbols if (row["info"] & 0xF) == 2 and row["name"] == function]
    if len(matches) != 1:
        raise EvidenceError(f"ELF must contain exactly one function {function!r}; found {len(matches)}")
    focus = matches[0]
    section_index, start, size = focus["section"], focus["value"], focus["size"]
    if section_index == 0 or section_index >= shnum or size <= 0 or size % 4:
        raise EvidenceError("focus function has uncertain section/extent")
    focus_section = sections[section_index]
    relocation_sections = [row for row in sections if row["type"] in {4, 9} and row["info"] == section_index]
    if len(relocation_sections) > 1:
        raise EvidenceError("multiple relocation sections target the focus section")
    rows: list[dict[str, Any]] = []
    if relocation_sections:
        relsec = relocation_sections[0]
        if relsec["type"] != 4:
            raise EvidenceError("SHT_REL addends cannot be proven independently; SHT_RELA is required")
        expected_ent = 12
        if relsec["entsize"] != expected_ent or relsec["link"] != sym_index:
            raise EvidenceError("unsupported focus relocation section")
        for off in range(relsec["offset"], relsec["offset"] + relsec["size"], expected_ent):
            if off + expected_ent > len(data):
                raise EvidenceError("truncated ELF relocation section")
            rel_offset, info = _elf_u32(data, off), _elf_u32(data, off + 4)
            if not start <= rel_offset < start + size:
                continue
            symbol_index, rel_type = info >> 8, info & 0xFF
            if symbol_index >= len(symbols):
                raise EvidenceError("relocation symbol index is invalid")
            addend = struct.unpack_from(">i", data, off + 8)[0]
            symbol = symbols[symbol_index]
            symbol_section = symbol["section"]
            if symbol_section == 0:
                effective = {"kind": "undefined", "name": symbol["name"], "addend": addend}
            elif symbol_section == 0xFFF1:
                effective = {"kind": "absolute", "name": symbol["name"], "value": symbol["value"] + addend}
            elif symbol_section < shnum:
                effective = {
                    "kind": "section", "section": sections[symbol_section]["name"],
                    "offset": symbol["value"] + addend,
                }
            else:
                raise EvidenceError("relocation targets an unsupported special section")
            rows.append({
                "offset": rel_offset - start, "type": rel_type,
                "symbol": symbol["name"], "symbol_value": symbol["value"],
                "addend": addend, "effective_target": effective,
            })
    rows.sort(key=lambda row: (row["offset"], row["type"], _canonical(row["effective_target"])))
    return {
        "object": _descriptor(path), "section": focus_section["name"], "offset": start,
        "size": size, "instruction_count": size // 4, "physical_relocation_count": len(rows),
        "physical_relocations": rows,
    }


def _physical_receipt(target: Path, candidate: Path, function: str, strict_report: Path, readelf: Path) -> dict[str, Any]:
    # Run the pinned external tool first.  The independent parser below provides
    # deterministic structured rows and fails if the ELF shape is unsupported.
    for path in (target, candidate):
        _run([str(readelf), "-SWsWr", "--", str(path)], cwd=path.parent, label="PowerPC readelf")
    target_row = _parse_elf_relocations(target, function)
    candidate_row = _parse_elf_relocations(candidate, function)
    differences: list[dict[str, Any]] = []
    target_effective = [{k: row[k] for k in ("offset", "type", "effective_target")} for row in target_row["physical_relocations"]]
    candidate_effective = [{k: row[k] for k in ("offset", "type", "effective_target")} for row in candidate_row["physical_relocations"]]
    if target_effective != candidate_effective:
        differences.append({"target": target_effective, "candidate": candidate_effective})
    receipt: dict[str, Any] = {
        "schema": PHYSICAL_SCHEMA, "authority_advanced": False,
        "report": _descriptor(strict_report), "function": function,
        "target": target_row, "candidate": candidate_row,
        "physical_relocations_exact": not differences,
        "physical_relocation_differences": differences,
        "symbol_attribution_aliases": [],
    }
    receipt["receipt_sha256"] = _json_sha(receipt)
    return receipt


def _phase_receipt(env: Mapping[str, Any], phase: str, artifacts: Mapping[str, Any], tools: Mapping[str, Any]) -> dict[str, Any]:
    receipt: dict[str, Any] = {
        "schema": PHASE_RECEIPT_SCHEMA, "phase": phase, "owner": env["owner"],
        "function": env["function"], "unit": env["unit"], "source_relpath": env["source_relpath"],
        "base_commit": env["base_commit"], "approval_sha256": env["approval_sha256"],
        "approval_context_sha256": env["context_sha256"], "phase_nonce": env["phase_nonce"],
        "issued_at": env["issued_at"], "artifacts": dict(artifacts), "tools": dict(tools),
        "authority_advanced": False,
    }
    receipt["receipt_sha256"] = _json_sha(receipt)
    return receipt


def _run_phase_impl(
    *, root: Path, context_path: Path, out_root: Path, objdiff: Path,
    readelf: Path, ninja: Path, staged_retail: list[Path],
) -> dict[str, Any]:
    root, out_root = _prepare_output_root(root, out_root)
    if not (root / ".git").exists() and not (root / ".git").is_file():
        raise EvidenceError("root is not a Git worktree")
    env = _load_environment(root, out_root, context_path)
    manifest_path = Path(os.environ.get("MP6_CRACK_TOOLCHAIN_MANIFEST", DEFAULT_TOOLCHAIN_MANIFEST))
    toolchain = _load_toolchain(manifest_path, env["toolchain_key"])
    if objdiff.resolve() != toolchain["objdiff"]["path_object"]:
        raise EvidenceError("objdiff path is not the central manifest pin")
    manifest_readelf = toolchain["binutils"]["path_object"] / "powerpc-eabi-readelf.exe"
    if readelf == DEFAULT_READELF:
        readelf = manifest_readelf
    if readelf.resolve() != manifest_readelf.resolve():
        raise EvidenceError("readelf path is not the central manifest pin")
    if ninja.resolve() != toolchain["ninja"]["path_object"]:
        raise EvidenceError("Ninja path is not the central manifest pin")
    objdiff_tool = _verify_objdiff(objdiff.resolve())
    readelf_tool = _verify_readelf(readelf.resolve())
    ninja_tool = _verify_ninja(ninja.resolve())
    staged_retail.append(_ensure_configured(root, toolchain, ninja))
    target, built = _unit_paths(root, env["unit"])
    if not target.is_file() or _file_sha(target) != env["target_sha256"]:
        raise EvidenceError("selected target object is missing or hash-drifted")
    tools = {
        "objdiff": objdiff_tool, "readelf": readelf_tool, "ninja": ninja_tool,
        "bundle": _descriptor(Path(__file__)), "toolchain": toolchain["public"],
    }
    baseline_receipt_path = out_root / "baseline-receipt.json"
    evidence_context_path = out_root / "evidence-context.json"
    if env["phase"] == "baseline":
        forbidden = (
            "target.o", "candidate.o", "baseline-strict.json", "baseline-data.json",
            "baseline-physical.json", "candidate-strict.json", "candidate-data.json", "physical.json",
            "baseline-receipt.json", "candidate-receipt.json", "evidence-context.json",
        )
        stale = [name for name in forbidden if (out_root / name).exists()]
        if stale:
            raise EvidenceError("baseline output root contains stale evidence: " + ", ".join(stale))
        built.unlink(missing_ok=True)
        _run([ninja, "-j1", str(built.relative_to(root))], cwd=root, label="selected owner object build")
        if not built.is_file():
            raise EvidenceError("selected owner build produced no object")
        _atomic_copy(target, out_root / "target.o")
        baseline_object = out_root / "baseline-candidate.o"
        _atomic_copy(built, baseline_object)
        _run_objdiff(objdiff, out_root / "target.o", baseline_object, out_root / "baseline-strict.json", data=False, root=root)
        _run_objdiff(objdiff, out_root / "target.o", baseline_object, out_root / "baseline-data.json", data=True, root=root)
        baseline_physical = _physical_receipt(
            out_root / "target.o", baseline_object, env["function"],
            out_root / "baseline-strict.json", readelf,
        )
        _atomic_json(out_root / "baseline-physical.json", baseline_physical)
        artifacts = {
            name: _compact_descriptor(out_root / name)
            for name in (
                "target.o", "baseline-candidate.o", "baseline-strict.json",
                "baseline-data.json", "baseline-physical.json",
            )
        }
        receipt = _phase_receipt(env, "baseline", artifacts, tools)
        _atomic_json(baseline_receipt_path, receipt)
        context: dict[str, Any] = {
            "schema": SCHEMA, "owner": env["owner"], "function": env["function"], "unit": env["unit"],
            "source_relpath": env["source_relpath"], "target_sha256": env["target_sha256"],
            "base_commit": env["base_commit"], "approval_sha256": env["approval_sha256"],
            "approval_context_sha256": env["context_sha256"],
            "phase_nonces": {"baseline": env["phase_nonce"], "candidate": hashlib.sha256(f"{env['context_sha256']}:candidate".encode()).hexdigest()},
            "baseline_receipt": _compact_descriptor(baseline_receipt_path), "authority_advanced": False,
        }
        context["evidence_context_sha256"] = _json_sha(context)
        _atomic_json(evidence_context_path, context)
        return receipt

    if not baseline_receipt_path.is_file() or not evidence_context_path.is_file():
        raise EvidenceError("candidate phase lacks sealed baseline evidence")
    baseline_receipt = _load_json(baseline_receipt_path, "baseline receipt")
    unsigned = dict(baseline_receipt)
    receipt_digest = unsigned.pop("receipt_sha256", None)
    if receipt_digest != _json_sha(unsigned) or baseline_receipt.get("phase") != "baseline":
        raise EvidenceError("baseline receipt integrity failed")
    context = _load_json(evidence_context_path, "evidence context")
    unsigned_context = dict(context)
    context_digest = unsigned_context.pop("evidence_context_sha256", None)
    if context_digest != _json_sha(unsigned_context):
        raise EvidenceError("evidence context integrity failed")
    for key in ("owner", "function", "unit", "source_relpath", "base_commit", "approval_sha256", "approval_context_sha256"):
        expected = env["context_sha256"] if key == "approval_context_sha256" else env[key]
        if context.get(key) != expected:
            raise EvidenceError(f"candidate phase {key} drifted from baseline")
    if context.get("phase_nonces", {}).get("candidate") != env["phase_nonce"]:
        raise EvidenceError("candidate phase nonce is not baseline-bound")
    for name, descriptor in baseline_receipt.get("artifacts", {}).items():
        if name not in {
            "target.o", "baseline-candidate.o", "baseline-strict.json",
            "baseline-data.json", "baseline-physical.json",
        }:
            raise EvidenceError("baseline receipt contains an unknown artifact")
        path = out_root / name
        if not path.is_file() or _file_sha(path) != descriptor.get("sha256") or path.stat().st_size != descriptor.get("size_bytes"):
            raise EvidenceError(f"baseline artifact drifted: {name}")
    for name in ("candidate.o", "candidate-strict.json", "candidate-data.json", "physical.json", "candidate-receipt.json"):
        if (out_root / name).exists():
            raise EvidenceError(f"candidate output is stale: {name}")
    built.unlink(missing_ok=True)
    _run([ninja, "-j1", str(built.relative_to(root))], cwd=root, label="selected owner object build")
    if not built.is_file():
        raise EvidenceError("candidate build produced no object")
    _atomic_copy(built, out_root / "candidate.o")
    _run_objdiff(objdiff, out_root / "target.o", out_root / "candidate.o", out_root / "candidate-strict.json", data=False, root=root)
    _run_objdiff(objdiff, out_root / "target.o", out_root / "candidate.o", out_root / "candidate-data.json", data=True, root=root)
    physical = _physical_receipt(out_root / "target.o", out_root / "candidate.o", env["function"], out_root / "candidate-strict.json", readelf)
    _atomic_json(out_root / "physical.json", physical)
    artifacts = {name: _compact_descriptor(out_root / name) for name in ("target.o", "candidate.o", "candidate-strict.json", "candidate-data.json", "physical.json")}
    receipt = _phase_receipt(env, "candidate", artifacts, tools)
    _atomic_json(out_root / "candidate-receipt.json", receipt)
    final_context = dict(context)
    final_context["candidate_receipt"] = _compact_descriptor(out_root / "candidate-receipt.json")
    final_context["completed"] = True
    final_context.pop("evidence_context_sha256", None)
    final_context["evidence_context_sha256"] = _json_sha(final_context)
    _atomic_json(evidence_context_path, final_context)
    return receipt


def run_phase(*, root: Path, context_path: Path, out_root: Path, objdiff: Path, readelf: Path, ninja: Path = DEFAULT_NINJA) -> dict[str, Any]:
    staged_retail: list[Path] = []
    try:
        return _run_phase_impl(
            root=root, context_path=context_path, out_root=out_root,
            objdiff=objdiff, readelf=readelf, ninja=ninja,
            staged_retail=staged_retail,
        )
    finally:
        for retail_copy in staged_retail:
            _remove_staged_retail(retail_copy)


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--context", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--objdiff", type=Path, default=DEFAULT_OBJDFF)
    parser.add_argument("--readelf", type=Path, default=DEFAULT_READELF)
    args = parser.parse_args(argv)
    try:
        raw_out = args.out
        env_out = os.environ.get("CRACK_HARNESS_OUT_ROOT")
        if env_out is None or Path(env_out).resolve() != raw_out.resolve():
            raise EvidenceError("--out must equal CRACK_HARNESS_OUT_ROOT")
        receipt = run_phase(
            root=args.root, context_path=args.context, out_root=raw_out,
            objdiff=args.objdiff, readelf=args.readelf, ninja=DEFAULT_NINJA,
        )
        print(json.dumps(receipt, sort_keys=True, separators=(",", ":")))
        return 0
    except (EvidenceError, OSError) as exc:
        print(f"crack evidence bundle: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
