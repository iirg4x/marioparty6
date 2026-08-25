"""Materialize and validate a closed Player GC/2.7 diagnostic package.

This module deliberately does not launch a compiler or a capture backend.  It
turns a sealed forensic package into a fresh, immutable-to-overwrite package
whose one-shot command is ready for a separately authorized execution step.
The materializer is intentionally conservative: only the six authenticated
input assets are copied, every path is checked for symlinks and containment,
and every generated JSON document is revalidated by :func:`validate_package`.

The command line is:

    python -B tools/player_gc27_runtime_package.py materialize \
        --forensic-root OLD --output-root NEW --current-tool TOOL \
        --source-span-template TEMPLATE [--source SOURCE] \
        [--expected-object-path ABSOLUTE_HASH_IDENTICAL_OBJECT] [--session-id ID]

    python -B tools/player_gc27_runtime_package.py validate \
        --package-root NEW

``materialize`` and ``validate`` are diagnostic-only.  Neither mode executes
the one-shot command embedded in the receipt.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Iterable, Mapping, Sequence


SCHEMA = "player_gc27_current_source_capture_package/v2"
REQUEST_SCHEMA = "player_gc27_phys_reg_capture_request/v490"
MANIFEST_SCHEMA = "player_gc27_tracer_runtime_manifest/v2"
RECEIPT_SCHEMA = SCHEMA
EXPECTED_OBJECT_BINDING_SCHEMA = "player_gc27_expected_object_binding/v1"
STALE_HOOK_MARKERS = ("0x004d03e8", "0x4d03e8", "004d03e8")
OLD_TOOL_HASH_KEY = "debugger"
PACKAGE_SESSION_RE = re.compile(r"^session-[0-9a-f]{16}$")
LAUNCHER_ROOT_RE = re.compile(r'(?m)^(?P<indent>\s*)ROOT\s*=\s*Path\(r(?P<quote>["\'])(?P<root>.+?)(?P=quote)\)\s*$')
LAUNCHER_LOCK_RE = re.compile(r'(?m)^\s*COMPILER_LANE_LOCK\s*=\s*ROOT\s*/\s*["\']build/\.compiler-lane\.lock["\']\s*$')

HOOK_ORDER = (
    "function_filter",
    "allocation_pre",
    "allocation_post",
    "object_write_0",
    "object_write_1",
    "object_write_2",
    "regalloc",
    "physical_pair_commit",
    "physical_single_commit",
    "precolored_commit",
    "pcode_color_pre",
    "pcode_color_post",
    "gc27_machine_emit",
)
EXPECTED_HOOKS = {
    "function_filter": (0x00433492, "8b400e8b5006eb08", "function_filter"),
    "allocation_pre": (0x0043367E, "e87d650c00598a44240450", "numeric_stack_alloc_pre"),
    "allocation_post": (0x00433683, "598a4424045068404e4300", "numeric_stack_alloc_post"),
    "object_write_0": (0x004f9d74, "89432e8b530e8b420201e84821f00105e40c", "object_stack_write"),
    "object_write_1": (0x004f9e11, "89432e8b4b0e8b410201e84821f00105dc0c", "object_stack_write"),
    "object_write_2": (0x004f9e98, "89432e8b4b0e8b410201e84821f00105d80c", "object_stack_write"),
    "regalloc": (0x0043598b, "ff74240ce89ca809", "regalloc"),
    "physical_pair_commit": (0x004d0e65, "5d5f5e5bc3", "regalloc_post"),
    "physical_single_commit": (0x004d0f6e, "5d5f5e5bc3", "regalloc_post"),
    "precolored_commit": (0x004d0a7b, "eb768d4000", "regalloc_post"),
    "pcode_color_pre": (0x005086c4, "6689420483c20c83", "pcode_color_diagnostic"),
    "pcode_color_post": (0x005086c8, "83c20c83ed0173d3", "pcode_color_diagnostic"),
    "gc27_machine_emit": (0x004eb21f, "8b178b0a030dd00b5e0001e989018b43", "machine_emit"),
}
PHYSICAL_HOOK_ORDER = ("physical_pair_commit", "physical_single_commit", "precolored_commit")
KNOWN_ASSET_ROLES = ("source", "template", "template_manifest", "launcher", "backend", "adapter")
JSON_GENERATED_FILES = (
    "manifest.json",
    "backend-request.json",
    "preflight-trust-root.json",
    "trust-root.json",
    "launch-execution-plan.json",
    "package-receipt.json",
)
COMMON_COPIED_RELATIVE_FILES = (
    "inputs/player.c",
    "inputs/source-span-template-manifest.json",
    "inputs/launch_movenum_capture.py",
    "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py",
    "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py",
)

PLAYER_SOURCE_SHA256 = "077aad5cf4d2670d301bf8a73d447c34222e9f0f1bbd38700d6c3d97946d9b68"
PLAYER_SOURCE_SIZE = 114878
PLAYER_OBJECT_SHA256 = "e0806715086606bc51162bbbb90208aa1aa5cd99944738f79a8c515d58cde686"
PLAYER_OBJECT_SIZE = 96360
PLAYER_TARGET_SHA256 = "b609fe14d1a40162ba42a5ebc61e000a22082a9c348ca8d271a99ac15ac388dd"
PLAYER_TARGET_SIZE = 98840

# These are deliberately closed profiles, not user-extensible configuration.
# Adding a function requires authenticated source/function/body/template facts
# and tests which prove that no stale MoveNum guard survives the rewrite.
FUNCTION_PROFILES: dict[str, dict[str, Any]] = {
    "MoveNumOMExec": {
        "source_sha256": PLAYER_SOURCE_SHA256,
        "source_size": PLAYER_SOURCE_SIZE,
        "object_sha256": PLAYER_OBJECT_SHA256,
        "object_size": PLAYER_OBJECT_SIZE,
        "target_sha256": PLAYER_TARGET_SHA256,
        "target_size": PLAYER_TARGET_SIZE,
        "function_sha256": "19ade5267f17fe6a3e17109837790ce39598fbef9fe2c597b03d860328ae0580",
        "body_sha256": "e0173530638fa0e810954797cbed2ade8eb2491d255fef87afb588c236098436",
        "template_input_sha256": "8cf62d6743b443f3778e549f35366ad506818a3205bc573bae7ba7313f16e7fb",
        "template_span_count": 31,
        "template_source_sha256": PLAYER_SOURCE_SHA256,
        "byte_delta": 0,
        "line_delta": 0,
        "output_object_name": "movenum.o",
    },
    "GetBiriQEffectRadius": {
        "source_sha256": PLAYER_SOURCE_SHA256,
        "source_size": PLAYER_SOURCE_SIZE,
        "object_sha256": PLAYER_OBJECT_SHA256,
        "object_size": PLAYER_OBJECT_SIZE,
        "target_sha256": PLAYER_TARGET_SHA256,
        "target_size": PLAYER_TARGET_SIZE,
        "function_sha256": "2d4049d30f77f5fdb02b4211d75f0c742daf48b7ffc71db5d276140bbb2cdd7d",
        "body_sha256": "a11f9479732af0227bd7d6066b402570f584de24687d5451f4b17b6a1793ede4",
        "template_input_sha256": "8afd01dc64849a7741693741421f44d3816a390bd397ece73d200228b9c37f98",
        "template_span_count": 49,
        "template_source_sha256": "d4a97feebb263f768c041c624f03850005cde03a871940c886be2f849dd0c11d",
        "byte_delta": -35,
        "line_delta": -1,
        "output_object_name": "radius.o",
    },
}
BACKEND_DERIVED_SESSION = '''    session = "session-" + hashlib.sha256(
        (label + str(source_descriptor["sha256"])).encode()
    ).hexdigest()[:16]
'''
BACKEND_REQUEST_SESSION = '''    session = str(request.get("session_id", ""))
    if (
        len(session) != 24
        or not session.startswith("session-")
        or any(character not in "0123456789abcdef" for character in session[8:])
    ):
        raise BackendError("request session_id is not canonical")
'''


class PackageError(RuntimeError):
    """Raised when a package cannot be authenticated or is not closed."""


def _fail(message: str) -> "NoReturn":
    raise PackageError(message)


def _regular(path: Path, *, label: str, allow_missing: bool = False) -> None:
    if not path.exists():
        if allow_missing:
            return
        _fail(f"{label} does not exist: {path}")
    if path.is_symlink() or os.path.islink(path):
        _fail(f"{label} is a symlink: {path}")
    if not path.is_file():
        _fail(f"{label} is not a regular file: {path}")


def _no_symlinks(root: Path, *, label: str) -> None:
    if not root.exists():
        return
    if root.is_symlink():
        _fail(f"{label} is a symlink: {root}")
    for current, dirs, files in os.walk(root, followlinks=False):
        current_path = Path(current)
        for name in tuple(dirs) + tuple(files):
            candidate = current_path / name
            if candidate.is_symlink() or os.path.islink(candidate):
                _fail(f"{label} contains a symlink: {candidate}")


def _sha256(path: Path) -> str:
    _regular(path, label="hashed file")
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _descriptor(path: Path, *, display: str | None = None) -> dict[str, Any]:
    _regular(path, label="descriptor source")
    return {
        "path": display if display is not None else str(path),
        "sha256": _sha256(path),
        "size": path.stat().st_size,
    }


def _expect_descriptor(path: Path, expected: Mapping[str, Any], *, label: str) -> None:
    _regular(path, label=label)
    expected_hash = expected.get("sha256")
    expected_size = expected.get("size")
    if not isinstance(expected_hash, str) or not re.fullmatch(r"[0-9a-fA-F]{64}", expected_hash):
        _fail(f"{label} descriptor has no valid sha256")
    actual_hash = _sha256(path)
    if actual_hash.lower() != expected_hash.lower():
        _fail(f"{label} hash drift: expected {expected_hash}, got {actual_hash}")
    if expected_size is not None and int(expected_size) != path.stat().st_size:
        _fail(f"{label} size drift: expected {expected_size}, got {path.stat().st_size}")


def _read_json(path: Path, *, label: str) -> dict[str, Any]:
    _regular(path, label=label)
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        _fail(f"{label} is not valid UTF-8 JSON: {exc}")
    if not isinstance(value, dict):
        _fail(f"{label} must contain a JSON object")
    return value


def _write_json(path: Path, value: Mapping[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")


def _canonical_json(value: Any) -> bytes:
    # Matches the copied adapter's canonical_hash exactly: compact sorted JSON,
    # ASCII escaping, and no trailing newline.
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode("utf-8")


def _request_digest(request: Mapping[str, Any]) -> str:
    copy = json.loads(json.dumps(request))
    if isinstance(copy, dict):
        copy.pop("request_sha256", None)
    return hashlib.sha256(_canonical_json(copy)).hexdigest()


def _path_in_root(root: Path, value: Any, *, label: str) -> Path:
    if not isinstance(value, str) or not value:
        _fail(f"{label} path is missing")
    raw = Path(value)
    if raw.is_absolute():
        try:
            relative = raw.resolve(strict=False).relative_to(root.resolve())
        except ValueError:
            _fail(f"{label} is outside forensic package root: {value}")
    else:
        relative = raw
    if any(part in ("", ".", "..") for part in relative.parts):
        _fail(f"{label} contains unsafe path components: {value}")
    result = root.joinpath(*relative.parts)
    try:
        result.resolve(strict=False).relative_to(root.resolve())
    except ValueError:
        _fail(f"{label} escapes forensic package root: {value}")
    return result


def _package_relative(root: Path, path: Path) -> str:
    try:
        rel = path.resolve(strict=False).relative_to(root.resolve())
    except ValueError:
        _fail(f"asset is outside package root: {path}")
    if any(part in ("", ".", "..") for part in rel.parts):
        _fail(f"asset has unsafe relative path: {path}")
    return rel.as_posix()


def _relative_descriptor_path(root: Path, value: Any, *, label: str) -> Path:
    if not isinstance(value, str):
        _fail(f"{label} path is missing")
    path = Path(value)
    if path.is_absolute():
        # Package manifests often store the old absolute package root.  Do not
        # accept an arbitrary absolute path merely because its suffix matches.
        root_text = str(root.resolve()).rstrip("\\/")
        value_norm = str(path)
        if value_norm.casefold().startswith((root_text + os.sep).casefold()):
            return root / Path(value_norm[len(root_text) + 1 :])
        _fail(f"{label} absolute path is not inside forensic root: {value}")
    return _path_in_root(root, value, label=label)


def _replace_text(value: str, old_root: str, new_root: str, old_session: str | None, session: str) -> str:
    replaced = value.replace(old_root, new_root).replace(old_root.replace("\\", "/"), new_root.replace("\\", "/"))
    if old_session:
        replaced = replaced.replace(old_session, session)
    return replaced


def _rebase(value: Any, old_root: str, new_root: str, old_session: str | None, session: str, *, key: str | None = None) -> Any:
    if isinstance(value, str):
        return _replace_text(value, old_root, new_root, old_session, session)
    if isinstance(value, list):
        return [_rebase(item, old_root, new_root, old_session, session) for item in value]
    if isinstance(value, dict):
        return {
            k: (_rebase(v, old_root, new_root, old_session, session, key=k) if k != "session_id" else session)
            for k, v in value.items()
        }
    return value


def _find_old_session(value: Any) -> str | None:
    found: list[str] = []
    def visit(item: Any) -> None:
        if isinstance(item, dict):
            for key, child in item.items():
                if key == "session_id" and isinstance(child, str) and child != "<CAPTURE_SESSION_ID>":
                    found.append(child)
                visit(child)
        elif isinstance(item, list):
            for child in item:
                visit(child)
    visit(value)
    return found[0] if found else None


def _validate_session_id(session: str) -> None:
    if not isinstance(session, str) or not PACKAGE_SESSION_RE.fullmatch(session):
        _fail(f"invalid session ID: {session!r}")


def _hook_row(name: str, *, central: bool = True) -> dict[str, Any]:
    address, prefix, role = EXPECTED_HOOKS[name]
    row: dict[str, Any] = {
        "address": address,
        "id": name,
        "prefix": prefix,
        "role": role,
    }
    if central:
        row["lane"] = "stack" if name in ("function_filter", "allocation_pre", "allocation_post", "object_write_0", "object_write_1", "object_write_2") else "pcode"
    return row


def _physical_hook_row(name: str) -> dict[str, Any]:
    """Return the exact serialized row emitted by the copied adapter."""
    address, prefix, _ = EXPECTED_HOOKS[name]
    return {
        "id": name,
        "address": address,
        "address_hex": f"0x{address:08x}",
        "prefix": prefix,
        "commit_kind": "pair" if name == "physical_pair_commit" else "single" if name == "physical_single_commit" else "precolored",
        "optional": name == "precolored_commit",
    }


def _validate_hook_profile(manifest: Mapping[str, Any], request: Mapping[str, Any], receipt: Mapping[str, Any] | None = None) -> None:
    rows = manifest.get("hooks")
    if not isinstance(rows, list) or [row.get("id") for row in rows if isinstance(row, dict)] != list(HOOK_ORDER):
        _fail("manifest does not contain the exact 13-hook order")
    if len(rows) != 13:
        _fail("manifest has a non-13 hook count")
    for name, row in zip(HOOK_ORDER, rows):
        if not isinstance(row, dict):
            _fail(f"hook row is not an object: {name}")
        expected = _hook_row(name)
        for key in ("address", "id", "prefix", "role"):
            if row.get(key) != expected[key]:
                _fail(f"hook profile mismatch at {name}.{key}")
    union = request.get("custom_hook_union")
    if not isinstance(union, dict) or union.get("count") != 13:
        _fail("request custom_hook_union is not the exact 13-hook profile")
    union_rows = union.get("rows")
    if not isinstance(union_rows, list) or [row.get("id") for row in union_rows if isinstance(row, dict)] != list(HOOK_ORDER):
        _fail("request custom hook order is not exact")
    for name, row in zip(HOOK_ORDER, union_rows):
        expected = _hook_row(name)
        for key in ("address", "id", "prefix", "role"):
            if row.get(key) != expected[key]:
                _fail(f"request hook profile mismatch at {name}.{key}")
    physical = request.get("hooks")
    if not isinstance(physical, list) or [row.get("id") for row in physical if isinstance(row, dict)] != list(PHYSICAL_HOOK_ORDER):
        _fail("request physical hook list is not the exact three-hook subset")
    for name, row in zip(PHYSICAL_HOOK_ORDER, physical):
        expected = _hook_row(name, central=False)
        if row.get("id") != name or row.get("address") != expected["address"] or row.get("prefix") != expected["prefix"]:
            _fail(f"request physical hook mismatch at {name}")
    text = json.dumps({"manifest": manifest, "request": request}, sort_keys=True).casefold()
    if any(marker in text for marker in STALE_HOOK_MARKERS):
        _fail("stale 0x004D03E8 hook marker is present")
    if receipt is not None:
        hooks = receipt.get("hooks")
        if not isinstance(hooks, dict) or hooks.get("central_count") != 13 or hooks.get("adapter_count") != 13:
            _fail("receipt hook counts are not 13")
        if hooks.get("ordered_equal") is not True or hooks.get("stale_0x004D03E8_absent") is not True:
            _fail("receipt hook gate is not closed")


def _validate_materialized_physical_hook_serialization(request: Mapping[str, Any]) -> None:
    """Keep the materialized request byte-shape aligned with adapter ``as_dict``."""
    physical = request.get("hooks")
    if not isinstance(physical, list) or len(physical) != len(PHYSICAL_HOOK_ORDER):
        _fail("request physical hook serialization is not the authenticated three-hook table")
    for name, row in zip(PHYSICAL_HOOK_ORDER, physical):
        if row != _physical_hook_row(name):
            _fail(f"request physical hook serialization mismatch at {name}")


def _json_session_ids(value: Any, *, path: str) -> set[str]:
    result: set[str] = set()
    def visit(item: Any) -> None:
        if isinstance(item, dict):
            for key, child in item.items():
                if key == "session_id" and isinstance(child, str) and child != "<CAPTURE_SESSION_ID>":
                    result.add(child)
                visit(child)
        elif isinstance(item, list):
            for child in item:
                visit(child)
    visit(value)
    return result


def _asset_file_descriptor(root: Path, rel: str) -> dict[str, Any]:
    path = root / Path(rel)
    return _descriptor(path, display=rel)


def _load_forensic(root: Path) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    if not root.exists() or root.is_symlink() or not root.is_dir():
        _fail(f"forensic root is not a directory: {root}")
    _no_symlinks(root, label="forensic root")
    manifest = _read_json(root / "manifest.json", label="forensic manifest")
    request = _read_json(root / "backend-request.json", label="forensic backend request")
    receipt = _read_json(root / "package-receipt.json", label="forensic package receipt")
    if receipt.get("schema") not in ("player_gc27_current_source_capture_package/v1", SCHEMA):
        _fail("unsupported forensic package receipt schema")
    return manifest, request, receipt


def _asset_paths(root: Path, manifest: Mapping[str, Any], request: Mapping[str, Any], receipt: Mapping[str, Any]) -> dict[str, Path]:
    paths: dict[str, Path] = {}
    for role in ("backend", "adapter", "launcher", "template", "template_manifest"):
        descriptor = receipt.get(role)
        if not isinstance(descriptor, dict):
            _fail(f"forensic receipt lacks {role} descriptor")
        paths[role] = _relative_descriptor_path(root, descriptor.get("path"), label=f"forensic {role}")
    source_snapshot = receipt.get("source_snapshot")
    source_value = source_snapshot.get("path") if isinstance(source_snapshot, Mapping) else None
    if source_value is None:
        try:
            source_value = request["sources"]["retained"]["path"]
        except (KeyError, TypeError):
            source_value = manifest.get("source", {}).get("path")
    paths["source"] = _relative_descriptor_path(root, source_value, label="forensic source")
    return paths


def _validate_forensic_assets(root: Path, manifest: Mapping[str, Any], request: Mapping[str, Any], receipt: Mapping[str, Any], paths: Mapping[str, Path]) -> None:
    descriptor_by_role = {role: receipt.get(role) for role in KNOWN_ASSET_ROLES}
    if isinstance(receipt.get("source_snapshot"), Mapping):
        descriptor_by_role["source"] = receipt.get("source_snapshot")
    for role, path in paths.items():
        expected = descriptor_by_role.get(role)
        if not isinstance(expected, dict):
            _fail(f"missing forensic descriptor for {role}")
        _expect_descriptor(path, expected, label=f"forensic {role}")
    _validate_hook_profile(manifest, request)
    source_desc = manifest.get("source")
    if isinstance(source_desc, dict) and not isinstance(receipt.get("source_snapshot"), Mapping):
        _expect_descriptor(paths["source"], source_desc, label="forensic manifest source")
    if manifest.get("diagnostic_only") is not True or manifest.get("board_admission") is not False:
        _fail("forensic manifest admission flags are not diagnostic-only")
    if request.get("diagnostic_only") is not True or request.get("board_admission") is not False:
        _fail("forensic request admission flags are not diagnostic-only")
    old_tool = manifest.get("debugger")
    if not isinstance(old_tool, dict):
        _fail("forensic manifest lacks debugger descriptor")
    old_hash = old_tool.get("sha256")
    if not isinstance(old_hash, str) or not re.fullmatch(r"[0-9a-fA-F]{64}", old_hash):
        _fail("forensic debugger hash is invalid")
    return None


def _replace_tool_hash(source: Path, destination: Path, old_hash: str, new_hash: str, *, label: str) -> None:
    _regular(source, label=label)
    raw = source.read_bytes()
    old_bytes = old_hash.encode("ascii")
    count = raw.count(old_bytes)
    if count != 1:
        _fail(f"{label} must contain exactly one authenticated old tool hash, got {count}")
    new = raw.replace(old_bytes, new_hash.encode("ascii"))
    _copy_bytes_to_content(destination, new)


def _function_profile(name: str) -> dict[str, Any]:
    profile = FUNCTION_PROFILES.get(name)
    if profile is None:
        _fail(f"unsupported authenticated function profile: {name!r}")
    return dict(profile)


def _template_relative(function: str) -> str:
    return f"inputs/{function}.source-spans.unsealed.json"


def _copied_relative_files(function: str) -> tuple[str, ...]:
    return (COMMON_COPIED_RELATIVE_FILES[0], _template_relative(function), *COMMON_COPIED_RELATIVE_FILES[1:])


def _expected_relative_files(function: str) -> frozenset[str]:
    return frozenset(JSON_GENERATED_FILES + _copied_relative_files(function))


def _rewrite_adapter_function_guard(content: bytes, old_function: str, new_function: str) -> bytes:
    try:
        text = content.decode("utf-8")
    except UnicodeError as exc:
        _fail(f"forensic adapter is not UTF-8: {exc}")
    old_guard = (
        f'    if request.get("function") != "{old_function}":\n'
        f'        raise ValidationError("request is not bound to {old_function}")\n'
    )
    if text.count(old_guard) != 1:
        _fail("forensic adapter function guard is missing or ambiguous")
    new_guard = (
        f'    if request.get("function") != "{new_function}":\n'
        f'        raise ValidationError("request is not bound to {new_function}")\n'
    )
    return text.replace(old_guard, new_guard, 1).encode("utf-8")


def _rewrite_adapter(
    source: Path,
    destination: Path,
    old_hash: str,
    new_hash: str,
    old_function: str,
    new_function: str,
) -> None:
    _regular(source, label="forensic adapter")
    raw = source.read_bytes()
    if raw.count(old_hash.encode("ascii")) != 1:
        _fail("forensic adapter must contain exactly one authenticated old tool hash")
    raw = raw.replace(old_hash.encode("ascii"), new_hash.encode("ascii"), 1)
    raw = _rewrite_adapter_function_guard(raw, old_function, new_function)
    _copy_bytes_to_content(destination, raw)


def _rewrite_backend(
    source: Path,
    destination: Path,
    old_hash: str,
    new_hash: str,
    old_function: str,
    new_function: str,
) -> None:
    """Bind the copied backend to both the current tool and package session."""

    _regular(source, label="forensic backend")
    text = source.read_text(encoding="utf-8")
    if text.count(old_hash) != 1:
        _fail(
            "forensic backend must contain exactly one authenticated old tool hash, "
            f"got {text.count(old_hash)}"
        )
    if text.count(BACKEND_DERIVED_SESSION) != 1:
        _fail("forensic backend session derivation marker is missing or ambiguous")
    if BACKEND_REQUEST_SESSION in text:
        _fail("forensic backend already contains a request-session binding")
    old_function_binding = f'FUNCTION = "{old_function}"'
    if text.count(old_function_binding) != 1:
        _fail("forensic backend function binding is missing or ambiguous")
    transformed = text.replace(old_hash, new_hash, 1).replace(
        BACKEND_DERIVED_SESSION,
        BACKEND_REQUEST_SESSION,
        1,
    ).replace(old_function_binding, f'FUNCTION = "{new_function}"', 1)
    _copy_bytes_to_content(destination, transformed.encode("utf-8"))


def _validate_copied_backend_session_binding(root: Path) -> None:
    path = root / "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py"
    _regular(path, label="copied backend session binding")
    text = path.read_text(encoding="utf-8")
    if BACKEND_DERIVED_SESSION in text:
        _fail("copied backend still derives reusable source-based sessions")
    if text.count(BACKEND_REQUEST_SESSION) != 1:
        _fail("copied backend request-session binding is missing or ambiguous")


def _validate_copied_function_guards(root: Path, function: str) -> None:
    backend = root / "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py"
    adapter = root / "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"
    backend_text = backend.read_text(encoding="utf-8")
    adapter_text = adapter.read_text(encoding="utf-8")
    if backend_text.count(f'FUNCTION = "{function}"') != 1:
        _fail("copied backend function binding is missing or ambiguous")
    guard = (
        f'    if request.get("function") != "{function}":\n'
        f'        raise ValidationError("request is not bound to {function}")\n'
    )
    if adapter_text.count(guard) != 1:
        _fail("copied adapter function guard is missing or ambiguous")
    if function != "MoveNumOMExec":
        stale_guard = 'if request.get("function") != "MoveNumOMExec"'
        if stale_guard in adapter_text or 'FUNCTION = "MoveNumOMExec"' in backend_text:
            _fail("copied Radius producer retains a stale MoveNum function guard")


def _copy_bytes_to_content(destination: Path, content: bytes) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists() or destination.is_symlink():
        _fail(f"refusing to overwrite package asset: {destination}")
    destination.write_bytes(content)


def _runtime_worktree_from_tool(tool: Path) -> Path:
    """Derive the runtime worktree from ``.../tools/<tool>``."""
    resolved = tool.resolve(strict=True)
    tools_dir = resolved.parent
    if tools_dir.name.casefold() != "tools":
        _fail(f"current tool is not under a worktree tools directory: {tool}")
    worktree = tools_dir.parent
    if not worktree.is_dir() or worktree.is_symlink():
        _fail(f"derived runtime worktree is not a regular directory: {worktree}")
    return worktree


def _rewrite_launcher(source: Path, destination: Path, runtime_worktree: Path, *, expected: Mapping[str, Any], expected_old_root: str) -> dict[str, Any]:
    """Authenticate and rebase the launcher’s one exact ``ROOT`` literal."""
    _expect_descriptor(source, expected, label="forensic launcher")
    try:
        text = source.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as exc:
        _fail(f"forensic launcher is not readable UTF-8 Python: {exc}")
    matches = list(LAUNCHER_ROOT_RE.finditer(text))
    if len(matches) != 1:
        _fail(f"forensic launcher must contain exactly one ROOT literal, got {len(matches)}")
    lock_matches = list(LAUNCHER_LOCK_RE.finditer(text))
    if len(lock_matches) != 1:
        _fail(f"forensic launcher must contain exactly one compiler-lane lock binding, got {len(lock_matches)}")
    old_root = matches[0].group("root")
    if not old_root:
        _fail("forensic launcher ROOT literal is empty")
    if old_root.casefold() == str(runtime_worktree).casefold():
        _copy_bytes_to_content(destination, text.encode("utf-8"))
        return _descriptor(destination, display=_package_relative(destination.parent.parent.parent, destination))
    if old_root.casefold() != expected_old_root.casefold():
        _fail(f"forensic launcher ROOT literal drift: expected {expected_old_root!r}, got {old_root!r}")
    replacement = f'{matches[0].group("indent")}ROOT = Path(r"{runtime_worktree}")'
    rewritten = text[: matches[0].start()] + replacement + text[matches[0].end() :]
    if len(LAUNCHER_ROOT_RE.findall(rewritten)) != 1:
        _fail("rewritten launcher ROOT literal is not unique")
    if old_root.casefold() in rewritten.casefold():
        _fail("rewritten launcher retains the stale owner ROOT literal")
    if str(runtime_worktree).casefold() not in rewritten.casefold():
        _fail("rewritten launcher does not contain the derived runtime worktree ROOT")
    lock_match = LAUNCHER_LOCK_RE.search(rewritten)
    if lock_match is None or str(runtime_worktree).casefold() not in rewritten[lock_match.start() : lock_match.end()].casefold():
        # The lock is intentionally expressed through ROOT; this check binds
        # the exact derived ROOT rather than accepting an arbitrary path.
        if 'COMPILER_LANE_LOCK = ROOT / "build/.compiler-lane.lock"' not in rewritten and "COMPILER_LANE_LOCK = ROOT / 'build/.compiler-lane.lock'" not in rewritten:
            _fail("rewritten launcher has no private build/.compiler-lane.lock binding")
    _copy_bytes_to_content(destination, rewritten.encode("utf-8"))
    return _descriptor(destination, display=_package_relative(destination.parent.parent.parent, destination))


def _current_descriptor(path: Path) -> dict[str, Any]:
    return _descriptor(path)


def _ensure_no_stale_markers(root: Path) -> None:
    for current, dirs, files in os.walk(root, followlinks=False):
        for name in files:
            path = Path(current) / name
            if path.is_symlink() or os.path.islink(path):
                _fail(f"package contains a symlink: {path}")
            try:
                raw = path.read_bytes()
            except OSError as exc:
                _fail(f"cannot read package file {path}: {exc}")
            lowered = raw.decode("utf-8", errors="ignore").casefold()
            # The closed receipt intentionally names the boolean gate
            # ``stale_0x004D03E8_absent``.  Remove that key name before
            # scanning values for an actual stale hook address.
            lowered = lowered.replace("stale_0x004d03e8_absent", "")
            if any(marker in lowered for marker in STALE_HOOK_MARKERS):
                _fail(f"stale 0x004D03E8 marker in package file: {path}")


def _set_session_fields(value: Any, session: str) -> Any:
    if isinstance(value, dict):
        return {key: (session if key == "session_id" else _set_session_fields(child, session)) for key, child in value.items()}
    if isinstance(value, list):
        return [_set_session_fields(child, session) for child in value]
    return value


def _validate_external_descriptor(descriptor: Mapping[str, Any], *, label: str) -> None:
    path_value = descriptor.get("path")
    if not isinstance(path_value, str):
        _fail(f"{label} path missing")
    path = Path(path_value)
    _expect_descriptor(path, descriptor, label=label)


def _object_descriptor_identity(
    value: Any,
    *,
    label: str,
    require_path: bool = True,
) -> dict[str, Any]:
    """Return the closed identity fields of an expected-object descriptor.

    The forensic object may have moved since the receipt was sealed.  Its
    descriptor is therefore authenticated as immutable metadata here; the
    bytes at its path are authenticated only when that path is the bound
    materialization path.
    """
    if not isinstance(value, Mapping):
        _fail(f"{label} descriptor is not an object")
    path = value.get("path")
    digest = value.get("sha256")
    size = value.get("size")
    if require_path and (not isinstance(path, str) or not path):
        _fail(f"{label} descriptor path is missing")
    if path is not None and (not isinstance(path, str) or not path):
        _fail(f"{label} descriptor path is invalid")
    if not isinstance(digest, str) or not re.fullmatch(r"[0-9a-fA-F]{64}", digest):
        _fail(f"{label} descriptor has no valid sha256")
    if isinstance(size, bool) or not isinstance(size, int) or size < 0:
        _fail(f"{label} descriptor has no valid size")
    result: dict[str, Any] = {"sha256": digest, "size": size}
    if path is not None:
        result["path"] = path
    if "both_mirrored_lanes" in value:
        if not isinstance(value["both_mirrored_lanes"], bool):
            _fail(f"{label} descriptor has invalid both_mirrored_lanes")
        result["both_mirrored_lanes"] = value["both_mirrored_lanes"]
    return result


def _object_identity_equal(left: Mapping[str, Any], right: Mapping[str, Any]) -> bool:
    if ("path" in left or "path" in right) and left.get("path") != right.get("path"):
        return False
    if str(left.get("sha256", "")).casefold() != str(right.get("sha256", "")).casefold() or left.get("size") != right.get("size"):
        return False
    if "both_mirrored_lanes" in left or "both_mirrored_lanes" in right:
        return left.get("both_mirrored_lanes") == right.get("both_mirrored_lanes")
    return True


def _validate_closed_object_override(path_value: str | os.PathLike[str], expected: Mapping[str, Any], *, label: str) -> dict[str, Any]:
    """Authenticate an explicit, absolute, non-indirect object path.

    No output directory is created by this helper.  Checking every existing
    ancestor as well as the leaf rejects symlinked directory components and
    keeps the override closed to the exact regular file named by the caller.
    """
    path = Path(path_value)
    if not path.is_absolute():
        _fail(f"{label} path must be absolute: {path}")
    if any(part in ("", ".", "..") for part in path.parts):
        _fail(f"{label} path has unsafe components: {path}")
    _regular(path, label=label)
    for ancestor in (path, *path.parents):
        if ancestor.is_symlink() or os.path.islink(ancestor):
            _fail(f"{label} path contains a symlink: {ancestor}")
    expected_identity = _object_descriptor_identity(expected, label=f"{label} expected", require_path=False)
    actual = _descriptor(path, display=str(path))
    if actual["sha256"].casefold() != expected_identity["sha256"].casefold():
        _fail(
            f"{label} hash drift: expected {expected_identity['sha256']}, got {actual['sha256']}"
        )
    if actual["size"] != expected_identity["size"]:
        _fail(
            f"{label} size drift: expected {expected_identity['size']}, got {actual['size']}"
        )
    if "both_mirrored_lanes" in expected_identity:
        actual["both_mirrored_lanes"] = expected_identity["both_mirrored_lanes"]
    return actual


def _forensic_expected_object_descriptor(request: Mapping[str, Any], receipt: Mapping[str, Any]) -> dict[str, Any]:
    receipt_expected = _object_descriptor_identity(
        receipt.get("expected_object"),
        label="forensic expected object",
        require_path=False,
    )
    sources = request.get("sources")
    if not isinstance(sources, Mapping):
        _fail("forensic request sources are missing")
    seen = False
    for name, source_value in sources.items():
        if not isinstance(source_value, Mapping) or "expected_object" not in source_value:
            continue
        request_expected = _object_descriptor_identity(
            source_value.get("expected_object"),
            label=f"forensic {name} expected object",
            require_path=False,
        )
        if request_expected["sha256"].casefold() != receipt_expected["sha256"].casefold() or request_expected["size"] != receipt_expected["size"]:
            _fail("forensic request/receipt expected-object identity mismatch")
        seen = True
    if not seen:
        _fail("forensic request expected-object descriptor is missing")
    return receipt_expected


def _expected_object_binding(
    forensic_expected: Mapping[str, Any],
    bound_expected: Mapping[str, Any],
    *,
    override: bool,
) -> dict[str, Any]:
    binding: dict[str, Any] = {
        "schema": EXPECTED_OBJECT_BINDING_SCHEMA,
        "mode": "path_override" if override else "forensic_path",
        "forensic": dict(forensic_expected),
        "bound": dict(bound_expected),
    }
    if override:
        binding["override"] = dict(bound_expected)
    return binding


def _validate_expected_object_binding(
    owner: Mapping[str, Any],
    *,
    forensic_expected: Mapping[str, Any],
    bound_expected: Mapping[str, Any],
    label: str,
) -> None:
    binding = owner.get("expected_object_binding")
    if not isinstance(binding, Mapping) or binding.get("schema") != EXPECTED_OBJECT_BINDING_SCHEMA:
        _fail(f"{label} expected-object binding schema is not authenticated")
    mode = binding.get("mode")
    if mode not in ("forensic_path", "path_override"):
        _fail(f"{label} expected-object binding mode is invalid")
    owner_forensic = _object_descriptor_identity(
        owner.get("forensic_expected_object"),
        label=f"{label} forensic expected object",
        require_path=False,
    )
    binding_forensic = _object_descriptor_identity(
        binding.get("forensic"),
        label=f"{label} binding forensic expected object",
        require_path=False,
    )
    if not _object_identity_equal(owner_forensic, forensic_expected) or not _object_identity_equal(binding_forensic, forensic_expected):
        _fail(f"{label} forensic expected-object binding mismatch")
    owner_bound = _object_descriptor_identity(owner.get("expected_object"), label=f"{label} expected object")
    binding_bound = _object_descriptor_identity(binding.get("bound"), label=f"{label} binding expected object")
    if not _object_identity_equal(owner_bound, bound_expected) or not _object_identity_equal(binding_bound, bound_expected):
        _fail(f"{label} bound expected-object binding mismatch")
    if mode == "path_override":
        override = _object_descriptor_identity(binding.get("override"), label=f"{label} expected-object override")
        if not _object_identity_equal(override, bound_expected):
            _fail(f"{label} expected-object override binding mismatch")
        if not Path(bound_expected["path"]).is_absolute():
            _fail(f"{label} expected-object override path is not absolute")
    elif "override" in binding:
        _fail(f"{label} forensic expected-object binding unexpectedly contains an override")


def _safe_resolved(path: Path) -> Path:
    return path.resolve(strict=False)


def _distinct_paths(paths: Iterable[Path], *, label: str) -> None:
    seen: dict[str, Path] = {}
    for path in paths:
        key = str(_safe_resolved(path)).casefold()
        prior = seen.get(key)
        if prior is not None:
            _fail(f"{label} collision: {prior} and {path}")
        seen[key] = path


def _build_one_shot(root: Path, launcher_rel: str, request_rel: str, backend_rel: str, *, interpreter: Path | None = None) -> tuple[list[str], Path, Path]:
    python_path = Path(interpreter or sys.executable).resolve(strict=True)
    output = root / "backend-output"
    plan_output = root / "live-execution-plan.json"
    command = [
        str(python_path),
        "-B",
        str(root / launcher_rel),
        "--request",
        str(root / request_rel),
        "--backend",
        str(root / backend_rel),
        "--run-output",
        str(output),
        "--plan-output",
        str(plan_output),
        "--execute",
    ]
    return command, output, plan_output


def _validate_copied_adapter(root: Path, request: Mapping[str, Any]) -> None:
    """Run only the copied adapter's pure request validator.

    The adapter is authenticated by the receipt file-descriptor closure before
    this function is reached.  Import it from that exact package path without
    changing ``sys.path`` or launching any backend/compiler entry point.
    """
    adapter_path = root / "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"
    _regular(adapter_path, label="copied physical-register adapter")
    module_name = "_player_gc27_runtime_adapter_" + _sha256(adapter_path)[:16]
    if module_name in sys.modules:
        _fail(f"copied physical-register adapter module-name collision: {module_name}")
    spec = importlib.util.spec_from_file_location(module_name, adapter_path)
    if spec is None or spec.loader is None:
        _fail("copied physical-register adapter cannot be imported safely")
    module = importlib.util.module_from_spec(spec)
    write_bytecode = sys.dont_write_bytecode
    sys.dont_write_bytecode = True
    sys.modules[module_name] = module
    try:
        try:
            spec.loader.exec_module(module)
        except BaseException as exc:
            if isinstance(exc, PackageError):
                raise
            _fail(f"copied physical-register adapter import failed: {exc}")
        try:
            validator = getattr(module, "validate_request", None)
            if not callable(validator):
                _fail("copied physical-register adapter lacks validate_request")
            validator(request)
        except BaseException as exc:
            if isinstance(exc, PackageError):
                raise
            _fail(f"copied physical-register adapter rejected request: {exc}")
    finally:
        sys.modules.pop(module_name, None)
        sys.dont_write_bytecode = write_bytecode


def _shift_range_fields(value: dict[str, Any], *, byte_delta: int, line_delta: int) -> None:
    for key in ("byte_start", "byte_end", "body_byte_start", "body_byte_end"):
        if key in value:
            if isinstance(value[key], bool) or not isinstance(value[key], int):
                _fail(f"template {key} is not an integer")
            value[key] += byte_delta
    for key in ("line_start", "line_end", "body_line_start", "body_line_end"):
        if key in value:
            if isinstance(value[key], bool) or not isinstance(value[key], int):
                _fail(f"template {key} is not an integer")
            value[key] += line_delta


def _prepare_template(
    template: Path,
    selected_source: Path,
    function: str,
    profile: Mapping[str, Any],
) -> dict[str, Any]:
    if _sha256(template) != profile["template_input_sha256"]:
        _fail(f"{function} template hash is not authenticated")
    value = _read_json(template, label=f"{function} source-span template")
    if value.get("function") != function or value.get("function_sha256") != profile["function_sha256"]:
        _fail(f"{function} template function identity is not authenticated")
    if value.get("unsealed") is not True or value.get("authority_advanced") is not False:
        _fail(f"{function} template is not unsealed authority-free metadata")
    spans = value.get("spans")
    if not isinstance(spans, list) or len(spans) != profile["template_span_count"]:
        _fail(f"{function} template span count is not authenticated")
    source_desc = value.get("source")
    if not isinstance(source_desc, Mapping) or source_desc.get("sha256") != profile["template_source_sha256"]:
        _fail(f"{function} template source identity is not authenticated")
    if _sha256(selected_source) != profile["source_sha256"] or selected_source.stat().st_size != profile["source_size"]:
        _fail("selected source is not the authenticated immutable Player source")

    prepared = json.loads(json.dumps(value))
    byte_delta = int(profile["byte_delta"])
    line_delta = int(profile["line_delta"])
    anchor = prepared.get("source_anchor")
    if not isinstance(anchor, dict):
        _fail(f"{function} template source anchor is missing")
    _shift_range_fields(anchor, byte_delta=byte_delta, line_delta=line_delta)
    for row in prepared["spans"]:
        if not isinstance(row, dict):
            _fail(f"{function} template has a non-object span")
        _shift_range_fields(row, byte_delta=byte_delta, line_delta=line_delta)
    prepared["source"] = _descriptor(selected_source, display=str(selected_source))

    source_bytes = selected_source.read_bytes()
    checks = (
        ("function", anchor.get("byte_start"), anchor.get("byte_end"), profile["function_sha256"]),
        ("body", anchor.get("body_byte_start"), anchor.get("body_byte_end"), profile["body_sha256"]),
    )
    for label, start, end, digest in checks:
        if not isinstance(start, int) or not isinstance(end, int) or start < 0 or end <= start or end > len(source_bytes):
            _fail(f"{function} {label} source range is invalid after rebase")
        if hashlib.sha256(source_bytes[start:end]).hexdigest() != digest:
            _fail(f"{function} {label} source hash does not match after rebase")
    if anchor.get("text_sha256") != profile["function_sha256"] or anchor.get("body_sha256") != profile["body_sha256"]:
        _fail(f"{function} template anchor hashes are not authenticated")
    for index, row in enumerate(prepared["spans"]):
        start, end, digest = row.get("byte_start"), row.get("byte_end"), row.get("text_sha256")
        if not isinstance(start, int) or not isinstance(end, int) or start < 0 or end <= start or end > len(source_bytes):
            _fail(f"{function} span {index} range is invalid after rebase")
        if not isinstance(digest, str) or hashlib.sha256(source_bytes[start:end]).hexdigest() != digest:
            _fail(f"{function} span {index} source hash does not match after rebase")
    return prepared


def _prepare_template_manifest(
    manifest: Mapping[str, Any],
    *,
    function: str,
    profile: Mapping[str, Any],
    input_template: Path,
    prepared_template_path: Path,
    selected_source: Path,
) -> dict[str, Any]:
    value = json.loads(json.dumps(manifest))
    rows = value.get("functions")
    if not isinstance(rows, list):
        _fail("source-span template manifest has no function inventory")
    matches = [row for row in rows if isinstance(row, dict) and row.get("function") == function]
    if len(matches) != 1:
        _fail(f"template manifest must contain exactly one {function} row")
    row = matches[0]
    if row.get("template_sha256") not in (None, _sha256(input_template)):
        _fail(f"template manifest does not authenticate {function} input template")
    if row.get("function_sha256") != profile["function_sha256"] or row.get("body_sha256") != profile["body_sha256"]:
        _fail(f"template manifest {function} function/body identity mismatch")
    if row.get("span_count") != profile["template_span_count"]:
        _fail(f"template manifest {function} span count mismatch")
    row["template"] = prepared_template_path.name
    row["template_sha256"] = _sha256(prepared_template_path)
    row["source_rebase"] = {"byte_delta": profile["byte_delta"], "line_delta": profile["line_delta"]}
    value["source"] = _descriptor(selected_source, display=str(selected_source))
    value["diagnostic_only"] = True
    value["authority_advanced"] = False
    return value


def materialize_package(
    forensic_root: str | os.PathLike[str],
    output_root: str | os.PathLike[str],
    current_tool: str | os.PathLike[str],
    corrected_source_span_template: str | os.PathLike[str],
    *,
    source: str | os.PathLike[str] | None = None,
    template_manifest: str | os.PathLike[str] | None = None,
    session_id: str | None = None,
    expected_object_path: str | os.PathLike[str] | None = None,
    expected_object: str | os.PathLike[str] | None = None,
    expected_object_override: str | os.PathLike[str] | None = None,
    function_profile: str | None = None,
) -> dict[str, Any]:
    """Create one closed package and return its generated receipt."""
    forensic_input = Path(forensic_root)
    output_input = Path(output_root)
    tool_input = Path(current_tool)
    template_input_path = Path(corrected_source_span_template)
    _regular(tool_input, label="current same-session tool")
    _regular(template_input_path, label="corrected source-span template")
    # Materialization is the only point where caller-relative paths are
    # meaningful. Resolve them here so the sealed package remains portable
    # across working directories and independent validators never depend on
    # the producer process cwd.
    forensic = forensic_input.resolve(strict=True)
    output = output_input.resolve(strict=False)
    tool = tool_input.resolve(strict=True)
    template = template_input_path.resolve(strict=True)
    template_input = _read_json(template, label="corrected source-span template")
    function = function_profile or template_input.get("function")
    if not isinstance(function, str):
        _fail("function profile is missing")
    profile = _function_profile(function)
    if template_input.get("function") != function:
        _fail(f"template function {template_input.get('function')!r} does not match requested profile {function!r}")
    _no_symlinks(tool.parent, label="current tool parent") if False else None
    manifest_old, request_old, receipt_old = _load_forensic(forensic)
    paths = _asset_paths(forensic, manifest_old, request_old, receipt_old)
    _validate_forensic_assets(forensic, manifest_old, request_old, receipt_old, paths)
    forensic_expected = _forensic_expected_object_descriptor(request_old, receipt_old)
    if forensic_expected["sha256"].casefold() != profile["object_sha256"] or forensic_expected["size"] != profile["object_size"]:
        _fail(f"{function} forensic expected object is not authenticated")
    target_value = request_old.get("target")
    if not isinstance(target_value, Mapping) or target_value.get("sha256") != profile["target_sha256"] or target_value.get("size") != profile["target_size"]:
        _fail(f"{function} forensic target is not authenticated")
    override_values = [
        ("expected_object_path", expected_object_path),
        ("expected_object", expected_object),
        ("expected_object_override", expected_object_override),
    ]
    selected_overrides = [(name, value) for name, value in override_values if value is not None]
    if len(selected_overrides) > 1:
        first_name, first_value = selected_overrides[0]
        if any(Path(value) != Path(first_value) for _, value in selected_overrides[1:]):
            _fail(f"conflicting expected-object override paths: {first_name} and {selected_overrides[1][0]}")
    override_path = Path(selected_overrides[0][1]) if selected_overrides else None
    if override_path is not None:
        bound_expected = _validate_closed_object_override(
            override_path,
            forensic_expected,
            label="expected-object override",
        )
        expected_object_was_overridden = True
    else:
        # Preserve the existing no-override contract, but authenticate the
        # mutable external path before creating any output directory.
        _validate_external_descriptor(forensic_expected, label="forensic expected object")
        bound_expected = dict(forensic_expected)
        expected_object_was_overridden = False
    object_binding = _expected_object_binding(
        forensic_expected,
        bound_expected,
        override=expected_object_was_overridden,
    )
    old_function = manifest_old.get("function")
    if not isinstance(old_function, str):
        _fail("forensic package function identity is missing")
    current_template_manifest = Path(template_manifest) if template_manifest is not None else template.parent / "manifest.json"
    if not current_template_manifest.exists():
        current_template_manifest = paths["template_manifest"]
    _regular(current_template_manifest, label="source-span template manifest")
    current_template_manifest = current_template_manifest.resolve(strict=True)
    template_manifest_value = _read_json(current_template_manifest, label="source-span template manifest")
    old_source = paths["source"]
    template_source_value = template_input.get("source", {}).get("path") if isinstance(template_input.get("source"), dict) else None
    template_source = Path(template_source_value) if isinstance(template_source_value, str) else None
    selected_source = Path(source) if source is not None else (template_source or old_source)
    if source is None and profile["byte_delta"] != 0:
        _fail(f"{function} requires an explicit authenticated current source for template rebase")
    if profile["byte_delta"] == 0 and template_source is not None and selected_source.resolve(strict=False) != template_source.resolve(strict=False):
        _fail("selected source path does not exactly match corrected template source path")
    _regular(selected_source, label="selected source")
    selected_source = selected_source.resolve(strict=True)
    template_value = _prepare_template(template, selected_source, function, profile)
    if output.exists() or output.is_symlink():
        _fail(f"refusing to overwrite existing output root: {output}")
    forensic_resolved = forensic.resolve()
    output_resolved = output.resolve(strict=False)
    if output_resolved == forensic_resolved or output_resolved.is_relative_to(forensic_resolved) or forensic_resolved.is_relative_to(output_resolved):
        _fail("forensic and output roots overlap")
    _validate_session_id(session_id) if session_id is not None else None
    old_session = _find_old_session(receipt_old) or _find_old_session(manifest_old) or _find_old_session(request_old)
    if session_id is None:
        seed = f"{_sha256(tool)}:{_sha256(template)}:{profile['function_sha256']}:{_sha256(selected_source)}"
        session_id = "session-" + hashlib.sha256(seed.encode("ascii")).hexdigest()[:16]
    _validate_session_id(session_id)
    output.mkdir(parents=True, exist_ok=False)
    try:
        # Copy only the six authenticated, closed assets.
        rel_source = "inputs/player.c"
        rel_template = _template_relative(function)
        rel_template_manifest = "inputs/source-span-template-manifest.json"
        rel_launcher = "inputs/launch_movenum_capture.py"
        rel_backend = "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py"
        rel_adapter = "backend/movenum-pcode-color-v523/player-phys-reg-trace-v490/gc27_phys_reg_adapter.py"
        _copy_bytes_to_content(output / rel_source, selected_source.read_bytes())
        _write_json(output / rel_template, template_value)
        prepared_manifest = _prepare_template_manifest(
            template_manifest_value,
            function=function,
            profile=profile,
            input_template=template,
            prepared_template_path=output / rel_template,
            selected_source=selected_source,
        )
        _write_json(output / rel_template_manifest, prepared_manifest)
        _rewrite_launcher(
            paths["launcher"],
            output / rel_launcher,
            _runtime_worktree_from_tool(tool),
            expected=receipt_old["launcher"],
            expected_old_root=str(manifest_old.get("cwd") or request_old.get("cwd") or ""),
        )
        old_hash = str(manifest_old["debugger"]["sha256"])
        new_hash = _sha256(tool)
        _rewrite_backend(paths["backend"], output / rel_backend, old_hash, new_hash, old_function, function)
        _rewrite_adapter(paths["adapter"], output / rel_adapter, old_hash, new_hash, old_function, function)

        old_root_text = str(forensic.resolve())
        new_root_text = str(output.resolve())
        manifest = _set_session_fields(_rebase(manifest_old, old_root_text, new_root_text, old_session, session_id), session_id)
        manifest["session_id"] = session_id
        manifest["schema"] = MANIFEST_SCHEMA
        manifest["function"] = function
        manifest["function_sha256"] = profile["function_sha256"]
        manifest["function_body_sha256"] = profile["body_sha256"]
        manifest["function_profile"] = function
        # Keep capture source identity bound to the immutable source path
        # supplied by the corrected template.  The copied player.c is an
        # audit snapshot only; compiling it would fail the source-path gate.
        source_descriptor = _descriptor(selected_source, display=str(selected_source))
        manifest["source"] = source_descriptor
        manifest["expected_object"] = dict(bound_expected)
        manifest["forensic_expected_object"] = dict(forensic_expected)
        manifest["expected_object_binding"] = dict(object_binding)
        if expected_object_was_overridden:
            manifest["expected_object_override"] = dict(bound_expected)
        else:
            manifest.pop("expected_object_override", None)
        manifest["debugger"] = _current_descriptor(tool)
        manifest["debugger"]["path"] = str(tool)
        manifest["same_session_capture"] = dict(manifest["debugger"])
        manifest["hooks"] = [_hook_row(name) for name in HOOK_ORDER]
        manifest_argv = list(manifest.get("argv", []))
        for index, item in enumerate(manifest_argv):
            if isinstance(item, str) and (item.endswith("\\inputs\\player.c") or item.endswith("/inputs/player.c")):
                manifest_argv[index] = str(selected_source)
            elif isinstance(item, str) and item.endswith("backend-output\\movenum.o"):
                manifest_argv[index] = str(output / "backend-output" / profile["output_object_name"])
        manifest["argv"] = manifest_argv
        manifest["diagnostic_only"] = True
        manifest["board_admission"] = False
        manifest["exactness_claim"] = False
        manifest["authority_advanced"] = False
        manifest["compiler_or_capture_run"] = False
        manifest["compiler_run"] = False
        manifest["capture_run"] = False

        request = _set_session_fields(_rebase(request_old, old_root_text, new_root_text, old_session, session_id), session_id)
        request["session_id"] = session_id
        request["schema"] = REQUEST_SCHEMA
        request["function"] = function
        request["function_sha256"] = profile["function_sha256"]
        request["function_body_sha256"] = profile["body_sha256"]
        request["function_profile"] = function
        if function != old_function:
            for stale_key in ("base_request", "candidate_artifact", "historical_request_identity"):
                request.pop(stale_key, None)
            request["authentication_basis"] = "authenticated Player GC/2.7 function profile and immutable source/template/object/target bindings"
        request["diagnostic_only"] = True
        request["board_admission"] = False
        request["exactness_claim"] = False
        request["compiler_or_capture_run"] = False
        request["compiler_run"] = False
        request["capture_run"] = False
        request["same_session"] = dict(request.get("same_session", {}))
        request["same_session"]["session_id"] = session_id
        request["debugger"] = _current_descriptor(tool)
        request["same_session_capture"] = _current_descriptor(tool)
        # Rebase compiler argv and replace its source/output paths without
        # touching external target/object/toolchain identities.
        argv = list(request.get("argv", []))
        old_source_path = str(paths["source"])
        for index, item in enumerate(argv):
            if isinstance(item, str):
                argv[index] = _replace_text(item, old_root_text, new_root_text, old_session, session_id)
        request["argv"] = argv
        request_sources = request.get("sources")
        if isinstance(request_sources, dict):
            for source_value in request_sources.values():
                if isinstance(source_value, dict) and "path" in source_value:
                    source_value["path"] = str(selected_source)
                    source_value["sha256"] = _sha256(selected_source)
                    source_value["size"] = selected_source.stat().st_size
                if isinstance(source_value, dict) and "expected_object" in source_value:
                    source_value["expected_object"] = dict(bound_expected)
        if not isinstance(request_sources, dict):
            _fail("request sources are missing")
        request["expected_object"] = dict(bound_expected)
        request["forensic_expected_object"] = dict(forensic_expected)
        request["expected_object_binding"] = dict(object_binding)
        if expected_object_was_overridden:
            request["expected_object_override"] = dict(bound_expected)
        else:
            request.pop("expected_object_override", None)
        request["source"] = source_descriptor
        request["cwd"] = _replace_text(str(request.get("cwd", "")), old_root_text, new_root_text, old_session, session_id)
        request["outputs"] = {
            "manifest": str(output / "backend-output" / "manifest.json"),
            "run_output": str(output / "backend-output"),
            "sealed_envelope": str(output / "backend-output" / "physical-reg.envelope.json"),
            # The launcher must write its execution plan only to this reserved
            # path when the one-shot command is eventually executed.  It is
            # deliberately absent from a READY_NOT_EXECUTED package.
            "plan": str(output / "live-execution-plan.json"),
        }
        request["custom_hook_union"] = {"count": 13, "machine_emit": _hook_row("gc27_machine_emit"), "rows": [_hook_row(name) for name in HOOK_ORDER]}
        request["hooks"] = [_physical_hook_row(name) for name in PHYSICAL_HOOK_ORDER]
        request["function_sha256"] = profile["function_sha256"]
        # Make all package-owned path values canonical and deterministic.
        request["adapter"] = _descriptor(output / rel_adapter, display=str(output / rel_adapter)) | {"schema": request.get("adapter", {}).get("schema", "player_gc27_phys_reg_adapter/v490")}
        request["backend"] = _descriptor(output / rel_backend, display=str(output / rel_backend)) | {"schema": request.get("backend", {}).get("schema", "player_gc27_phys_reg_backend/v523")}
        request["transport"] = _current_descriptor(tool)
        request["same_session_capture"] = {
            "path": str(tool),
            "sha256": new_hash,
            "expected_sha256": new_hash,
            "size": tool.stat().st_size,
            "schema": "mwcc_capsule_same_session_capture_request/v1",
        }
        request["request_sha256"] = ""

        # Compiler argv must use the immutable source path and the uncreated
        # private output directory. Paths outside the package
        # (compiler/target/object) remain authenticated external identities.
        for index, item in enumerate(request["argv"]):
            if isinstance(item, str) and (item == old_source_path or item.endswith("\\inputs\\player.c") or item.endswith("/inputs/player.c")):
                request["argv"][index] = str(selected_source)
            elif isinstance(item, str) and item.endswith("backend-output\\movenum.o"):
                request["argv"][index] = str(output / "backend-output" / profile["output_object_name"])
        request["request_sha256"] = _request_digest(request)
        manifest["request_sha256"] = request["request_sha256"]

        _write_json(output / "manifest.json", manifest)
        _write_json(output / "backend-request.json", request)
        # Trust roots are generated from the authenticated old roots, then
        # point at the new request/tool/source.  They are not execution proof.
        old_trust = _read_json(forensic / "trust-root.json", label="forensic trust root")
        old_preflight = _read_json(forensic / "preflight-trust-root.json", label="forensic preflight trust root")
        def trust(value: Mapping[str, Any]) -> dict[str, Any]:
            result = _set_session_fields(_rebase(value, old_root_text, new_root_text, old_session, session_id), session_id)
            result["session_id"] = session_id
            result["diagnostic_only"] = True
            result["board_admission"] = False
            result["exactness_claim"] = False
            result["authority_advanced"] = False
            result["compiler_or_capture_run"] = False
            result["compiler_run"] = False
            result["capture_run"] = False
            result["request"] = _descriptor(output / "backend-request.json", display=str(output / "backend-request.json"))
            result["debugger"] = _current_descriptor(tool)
            result["source"] = _descriptor(output / rel_source, display=str(output / rel_source))
            result["function"] = function
            result["function_sha256"] = profile["function_sha256"]
            return result
        _write_json(output / "trust-root.json", trust(old_trust))
        _write_json(output / "preflight-trust-root.json", trust(old_preflight))
        launcher_command, backend_output, plan_output = _build_one_shot(output, rel_launcher, "backend-request.json", rel_backend)
        plan = {
            "schema": "player_gc27_phys_reg_launch_plan/v490",
            "status": "READY_NOT_EXECUTED",
            "execute_requested": False,
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "compiler_or_capture_run": False,
            "session_id": session_id,
            "function": function,
            "request": _descriptor(output / "backend-request.json", display=str(output / "backend-request.json")),
            "output_root": str(backend_output),
            "plan_output": str(plan_output),
            "compiler_invocation": request.get("argv", []),
            "sources": {"source": _descriptor(output / rel_source, display=str(output / rel_source)), "template": _descriptor(output / rel_template, display=str(output / rel_template))},
            "target": request.get("target", {}),
        }
        _write_json(output / "launch-execution-plan.json", plan)

        files = {rel: _asset_file_descriptor(output, rel) for rel in _copied_relative_files(function)}
        files.update({rel: _asset_file_descriptor(output, rel) for rel in JSON_GENERATED_FILES[:-1]})
        receipt = {
            "schema": RECEIPT_SCHEMA,
            "status": "READY_NOT_EXECUTED",
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "authority_advanced": False,
            "compiler_or_capture_run": False,
            "compiler_or_live_capture_run": False,
            "compiler_run": False,
            "capture_run": False,
            "root": str(output),
            "function": function,
            "function_sha256": profile["function_sha256"],
            "function_body_sha256": profile["body_sha256"],
            "function_profile": function,
            "session_id": session_id,
            "source": source_descriptor,
            "source_snapshot": _asset_file_descriptor(output, rel_source),
            "forensic_expected_object": dict(forensic_expected),
            "expected_object_binding": dict(object_binding),
            "template": _descriptor(output / rel_template, display=str(output / rel_template)) | {"span_count": len(template_value.get("spans", []))},
            "template_manifest": _descriptor(output / rel_template_manifest, display=str(output / rel_template_manifest)),
            "target": request.get("target", {}),
            "expected_object": dict(bound_expected),
            "current_tool": _current_descriptor(tool),
            "launcher": _asset_file_descriptor(output, rel_launcher),
            "backend": _asset_file_descriptor(output, rel_backend),
            "adapter": _asset_file_descriptor(output, rel_adapter),
            "manifest": _asset_file_descriptor(output, "manifest.json"),
            "request": _asset_file_descriptor(output, "backend-request.json"),
            "preflight_trust_root": _asset_file_descriptor(output, "preflight-trust-root.json"),
            "trust_root": _asset_file_descriptor(output, "trust-root.json"),
            "launch_plan": _asset_file_descriptor(output, "launch-execution-plan.json"),
            "request_sha256": request["request_sha256"],
            "hooks": {
                "central_count": 13,
                "adapter_count": 13,
                "ordered_equal": True,
                "order": list(HOOK_ORDER),
                "allocation_pre": "0x0043367E",
                "allocation_pre_prefix": EXPECTED_HOOKS["allocation_pre"][1],
                "object_write_addresses": ["0x004F9D74", "0x004F9E11", "0x004F9E98"],
                "stale_0x004D03E8_absent": True,
                "stale_object_write_addresses_absent": True,
                "machine_address": "0x004EB21F",
                "machine_prefix": EXPECTED_HOOKS["gc27_machine_emit"][1],
            },
            "one_shot": {
                "argv": launcher_command,
                "execute_requested": True,
                "python_bytecode_disabled": True,
                "interpreter": launcher_command[0],
                "run_output": str(backend_output),
                "plan_output": str(plan_output),
            },
            "validation": {
                "preflight": "READY",
                "compiler_or_capture_run": False,
                "compiler_run": False,
                "capture_run": False,
                "mwcc_launched": False,
                "live_capture_run": False,
                "backend_output_root_absent": True,
                "output_files_absent": True,
                "template_tokens_fabricated": False,
                "stale_hook_absent": True,
                "session_consistent": True,
            },
            "files": files,
            "notes": "Closed diagnostic package; materialization and validation never execute the one-shot command. Fresh capture-local tokens are required before sealing source spans.",
        }
        if expected_object_was_overridden:
            receipt["expected_object_override"] = dict(bound_expected)
        _write_json(output / "package-receipt.json", receipt)
        result = validate_package(output)
        return result
    except Exception:
        # The output root is intentionally not removed: retaining a failed
        # package gives an operator an auditable failure image and prevents a
        # later retry from silently overwriting it.
        raise


def _relative_output_files(root: Path) -> list[str]:
    result: list[str] = []
    for current, dirs, files in os.walk(root, followlinks=False):
        current_path = Path(current)
        for name in files:
            path = current_path / name
            result.append(_package_relative(root, path))
        for name in dirs:
            path = current_path / name
            if path.is_symlink() or os.path.islink(path):
                _fail(f"package contains a symlink directory: {path}")
    return sorted(result)


def validate_package(package_root: str | os.PathLike[str]) -> dict[str, Any]:
    """Rehash and validate a materialized package without executing it."""
    root = Path(package_root)
    if not root.exists() or not root.is_dir() or root.is_symlink():
        _fail(f"package root is not a regular directory: {root}")
    _no_symlinks(root, label="package root")
    root = root.resolve(strict=True)
    live_plan_path = root / "live-execution-plan.json"
    if live_plan_path.exists() or live_plan_path.is_symlink():
        _fail("live execution plan must be absent before one-shot launch")
    fallback_plan_path = root / "inputs" / "launch-plan.json"
    if fallback_plan_path.exists() or fallback_plan_path.is_symlink():
        _fail("launcher fallback inputs/launch-plan.json is forbidden")
    receipt = _read_json(root / "package-receipt.json", label="package receipt")
    if receipt.get("schema") != RECEIPT_SCHEMA:
        _fail("unsupported package receipt schema")
    receipt_root = receipt.get("root")
    if not isinstance(receipt_root, str) or not Path(receipt_root).is_absolute():
        _fail("package receipt root is not an absolute path")
    if Path(receipt_root).resolve(strict=True) != root:
        _fail("package receipt root does not resolve to the validated package")
    function = receipt.get("function")
    if not isinstance(function, str):
        _fail("package receipt function profile is missing")
    profile = _function_profile(function)
    if receipt.get("function_profile") != function:
        _fail("package receipt function profile binding is missing")
    files = _relative_output_files(root)
    expected_relative_files = _expected_relative_files(function)
    if set(files) != expected_relative_files:
        extra = sorted(set(files) - expected_relative_files)
        missing = sorted(expected_relative_files - set(files))
        _fail(f"package file closure mismatch; extra={extra}, missing={missing}")
    forensic_expected = _object_descriptor_identity(
        receipt.get("forensic_expected_object"),
        label="receipt forensic expected object",
        require_path=False,
    )
    receipt_expected = _object_descriptor_identity(
        receipt.get("expected_object"),
        label="receipt expected object",
    )
    if receipt.get("diagnostic_only") is not True or receipt.get("authority_advanced") is not False or receipt.get("compiler_or_capture_run") is not False or receipt.get("compiler_or_live_capture_run") is not False or receipt.get("compiler_run") is not False or receipt.get("capture_run") is not False:
        _fail("package receipt admission/run flags are not closed diagnostic-only")
    session = receipt.get("session_id")
    _validate_session_id(session)
    for rel in JSON_GENERATED_FILES:
        value = _read_json(root / rel, label=f"package {rel}")
        sessions = _json_session_ids(value, path=rel)
        if sessions - {session}:
            _fail(f"session mismatch in {rel}: {sorted(sessions)}")
        if value.get("diagnostic_only") is not True and rel != "inputs/source-span-template-manifest.json":
            _fail(f"{rel} is not diagnostic-only")
    manifest = _read_json(root / "manifest.json", label="package manifest")
    request = _read_json(root / "backend-request.json", label="package backend request")
    if manifest.get("schema") != MANIFEST_SCHEMA:
        _fail("package manifest schema is not authenticated")
    if request.get("schema") != REQUEST_SCHEMA:
        _fail("package backend request schema is not authenticated")
    _validate_expected_object_binding(
        manifest,
        forensic_expected=forensic_expected,
        bound_expected=receipt_expected,
        label="package manifest",
    )
    _validate_expected_object_binding(
        request,
        forensic_expected=forensic_expected,
        bound_expected=receipt_expected,
        label="package backend request",
    )
    _validate_expected_object_binding(
        receipt,
        forensic_expected=forensic_expected,
        bound_expected=receipt_expected,
        label="package receipt",
    )
    binding_mode = receipt["expected_object_binding"]["mode"]
    if binding_mode == "path_override":
        _validate_closed_object_override(
            receipt_expected["path"],
            forensic_expected,
            label="bound expected-object override",
        )
        for owner, label in (
            (manifest, "package manifest"),
            (request, "package backend request"),
            (receipt, "package receipt"),
        ):
            override = owner.get("expected_object_override")
            if not isinstance(override, Mapping) or not _object_identity_equal(
                _object_descriptor_identity(override, label=f"{label} expected-object override"),
                receipt_expected,
            ):
                _fail(f"{label} expected-object override descriptor mismatch")
    else:
        if "expected_object_override" in receipt or "expected_object_override" in manifest or "expected_object_override" in request:
            _fail("forensic expected-object binding unexpectedly contains an override descriptor")
    _validate_hook_profile(manifest, request, receipt)
    _validate_materialized_physical_hook_serialization(request)
    if manifest.get("session_id") != session or request.get("session_id") != session:
        _fail("manifest/request session mismatch")
    if request.get("request_sha256") != _request_digest(request):
        _fail("backend request digest mismatch")
    descriptor_map = receipt.get("files")
    if not isinstance(descriptor_map, dict):
        _fail("receipt file descriptor map is missing")
    expected_descriptor_files = expected_relative_files - {"package-receipt.json"}
    if set(descriptor_map) != expected_descriptor_files:
        _fail("receipt file descriptor closure mismatch")
    for rel, descriptor in descriptor_map.items():
        if not isinstance(descriptor, dict):
            _fail(f"file descriptor is not an object: {rel}")
        if descriptor.get("path") != rel:
            _fail(f"file descriptor path mismatch: {rel}")
        _expect_descriptor(root / rel, descriptor, label=f"package asset {rel}")
    for role in ("current_tool", "source", "target", "expected_object"):
        descriptor = receipt.get(role)
        if not isinstance(descriptor, dict):
            _fail(f"receipt lacks {role} descriptor")
        descriptor_path = descriptor.get("path")
        if not isinstance(descriptor_path, str) or not Path(descriptor_path).is_absolute():
            _fail(f"receipt {role} path is not absolute")
        _validate_external_descriptor(descriptor, label=f"receipt {role}")
    if receipt["source"].get("sha256") != profile["source_sha256"] or receipt["source"].get("size") != profile["source_size"]:
        _fail(f"{function} package source profile mismatch")
    if receipt["target"].get("sha256") != profile["target_sha256"] or receipt["target"].get("size") != profile["target_size"]:
        _fail(f"{function} package target profile mismatch")
    if receipt_expected.get("sha256") != profile["object_sha256"] or receipt_expected.get("size") != profile["object_size"]:
        _fail(f"{function} package object profile mismatch")
    if receipt.get("function_sha256") != profile["function_sha256"] or receipt.get("function_body_sha256") != profile["body_sha256"]:
        _fail(f"{function} package function/body profile mismatch")
    current_tool = receipt["current_tool"]
    if manifest.get("debugger", {}).get("sha256") != current_tool.get("sha256") or request.get("debugger", {}).get("sha256") != current_tool.get("sha256"):
        _fail("current tool descriptor mismatch")
    capture_tool = request.get("same_session_capture")
    if (
        not isinstance(capture_tool, Mapping)
        or capture_tool.get("schema") != "mwcc_capsule_same_session_capture_request/v1"
        or capture_tool.get("path") != current_tool.get("path")
        or capture_tool.get("sha256") != current_tool.get("sha256")
        or capture_tool.get("expected_sha256") != current_tool.get("sha256")
        or capture_tool.get("size") != current_tool.get("size")
    ):
        _fail("request same-session capture binding is not exact")
    source_descriptor = receipt["source"]
    if manifest.get("source", {}).get("path") != source_descriptor.get("path"):
        _fail("manifest source path is not the immutable capture source")
    if not any(isinstance(item, str) and item == source_descriptor.get("path") for item in manifest.get("argv", [])):
        _fail("manifest compiler argv does not use the immutable capture source")
    request_sources = request.get("sources")
    if not isinstance(request_sources, dict):
        _fail("request sources are missing")
    retained_source = request_sources.get("retained")
    if not isinstance(retained_source, dict) or retained_source.get("path") != source_descriptor.get("path"):
        _fail("request retained source path is not the immutable capture source")
    for source_value in request_sources.values():
        if isinstance(source_value, dict) and source_value.get("path") != source_descriptor.get("path"):
            _fail("mirrored source path mismatch")
        if isinstance(source_value, dict) and "expected_object" in source_value:
            source_expected = _object_descriptor_identity(
                source_value.get("expected_object"),
                label="request source expected object",
            )
            if not _object_identity_equal(source_expected, receipt_expected):
                _fail("mirrored expected-object descriptor mismatch")
    request_expected = _object_descriptor_identity(request.get("expected_object"), label="request expected object")
    manifest_expected = _object_descriptor_identity(manifest.get("expected_object"), label="manifest expected object")
    if not _object_identity_equal(request_expected, receipt_expected) or not _object_identity_equal(manifest_expected, receipt_expected):
        _fail("manifest/request/receipt expected-object descriptor mismatch")
    if not any(isinstance(item, str) and item == source_descriptor.get("path") for item in request.get("argv", [])):
        _fail("compiler argv does not use the immutable capture source")
    template_path = root / _template_relative(function)
    template = _read_json(template_path, label="package source-span template")
    if template.get("unsealed") is not True or template.get("authority_advanced") is not False or template.get("session_id") != "<CAPTURE_SESSION_ID>":
        _fail("template is sealed, authorized, or has fabricated capture session")
    if _sha256(template_path) != receipt.get("template", {}).get("sha256"):
        _fail("template hash mismatch")
    if template.get("function") != receipt.get("function") or template.get("function_sha256") != receipt.get("function_sha256"):
        _fail("template function identity mismatch")
    if request.get("function") != function or manifest.get("function") != function:
        _fail("function identity mismatch")
    for owner, label in ((request, "request"), (manifest, "manifest")):
        if owner.get("function_profile") != function or owner.get("function_sha256") != profile["function_sha256"] or owner.get("function_body_sha256") != profile["body_sha256"]:
            _fail(f"{label} function profile mismatch")
    target = request.get("target")
    expected = request.get("sources", {}).get("retained", {}).get("expected_object")
    if not isinstance(target, dict) or not isinstance(expected, dict):
        _fail("request target/expected-object descriptors are missing")
    if target.get("sha256") != receipt["target"].get("sha256") or expected.get("sha256") != receipt["expected_object"].get("sha256") or expected.get("size") != receipt["expected_object"].get("size"):
        _fail("receipt target/object identity mismatch")
    _validate_external_descriptor(target, label="request target")
    _validate_external_descriptor(expected, label="request expected object")
    output_root = root / "backend-output"
    if output_root.exists() or output_root.is_symlink():
        _fail("request/output collision: backend-output already exists")
    outputs = request.get("outputs", {})
    if not isinstance(outputs, dict):
        _fail("request outputs missing")
    request_path = _safe_resolved(root / "backend-request.json")
    output_paths = [Path(value) for value in outputs.values() if isinstance(value, str)]
    _distinct_paths([root / "backend-request.json", *output_paths], label="request/output")
    expected_plan = root / "live-execution-plan.json"
    if outputs.get("plan") != str(expected_plan):
        _fail("request plan output is not the reserved live execution plan path")
    if expected_plan.exists() or expected_plan.is_symlink():
        _fail("live execution plan must be absent before one-shot launch")
    one_shot = receipt.get("one_shot")
    if not isinstance(one_shot, dict) or one_shot.get("execute_requested") is not True:
        _fail("receipt one-shot command is missing")
    command = one_shot.get("argv")
    expected_command, expected_output, expected_plan = _build_one_shot(root, "inputs/launch_movenum_capture.py", "backend-request.json", "backend/movenum-pcode-color-v523/physical_capture_backend_v523.py")
    if (
        command != expected_command
        or one_shot.get("interpreter") != expected_command[0]
        or one_shot.get("run_output") != str(expected_output)
        or one_shot.get("plan_output") != str(expected_plan)
    ):
        _fail("one-shot command is not canonical")
    launch_plan = _read_json(root / "launch-execution-plan.json", label="launch plan")
    if launch_plan.get("execute_requested") is not False or launch_plan.get("session_id") != session:
        _fail("launch plan is executable or session-inconsistent")
    _ensure_no_stale_markers(root)
    _validate_copied_backend_session_binding(root)
    _validate_copied_adapter(root, request)
    _validate_copied_function_guards(root, function)
    return receipt


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    materialize = sub.add_parser("materialize", help="create a closed package; never execute")
    materialize.add_argument("--forensic-root", required=True, type=Path)
    materialize.add_argument("--output-root", required=True, type=Path)
    materialize.add_argument("--current-tool", "--tool", required=True, type=Path)
    materialize.add_argument("--source-span-template", "--template", required=True, type=Path)
    materialize.add_argument("--source", type=Path)
    materialize.add_argument("--template-manifest", type=Path)
    materialize.add_argument(
        "--function-profile",
        choices=tuple(FUNCTION_PROFILES),
        help="closed authenticated function profile; otherwise inferred from the template",
    )
    materialize.add_argument(
        "--expected-object-path",
        "--expected-object",
        "--expected-object-override",
        dest="expected_object_path",
        type=Path,
        help="absolute hash-identical object path to bind without changing the forensic receipt",
    )
    materialize.add_argument("--session-id")
    validate = sub.add_parser("validate", help="rehash and validate; never execute")
    validate.add_argument("--package-root", required=True, type=Path)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _build_parser().parse_args(argv)
    try:
        if args.mode == "materialize":
            receipt = materialize_package(
                args.forensic_root,
                args.output_root,
                args.current_tool,
                args.source_span_template,
                source=args.source,
                template_manifest=args.template_manifest,
                session_id=args.session_id,
                expected_object_path=args.expected_object_path,
                function_profile=args.function_profile,
            )
            print(json.dumps({"status": "READY_NOT_EXECUTED", "package_root": receipt.get("root"), "session_id": receipt.get("session_id"), "request_sha256": receipt.get("request_sha256")}, sort_keys=True))
        else:
            receipt = validate_package(args.package_root)
            print(json.dumps({"status": "VALID", "package_root": receipt.get("root"), "session_id": receipt.get("session_id"), "request_sha256": receipt.get("request_sha256")}, sort_keys=True))
        return 0
    except PackageError as exc:
        print(f"REJECTED: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
