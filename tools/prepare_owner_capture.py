#!/usr/bin/env python3
"""Prepare and run one authenticated, diagnostic MWCC native capture.

This is a small reusable boundary around ``capsule_same_session_capture``.  It
accepts an already expanded compiler argv JSON array; it never parses or
executes a batch file.  Preparation only seals request/trust-root identities.
The ``run`` operation authenticates those identities again, takes the shared
compiler lock, writes one exclusive launch marker, and invokes exactly one
capture.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path
import re
import sys
from collections.abc import Callable, Mapping, Sequence
from typing import Any

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.owner_campaign import _exclusive_lock


SCHEMA = "mwcc_owner_capture_prepare/v1"
PREPARED_STATUS = "PREPARED_NOT_LAUNCHED"
LAUNCH_STATUS = "LAUNCH_STARTED"
DEFAULT_LOCK_TIMEOUT_SECONDS = 30.0
MAX_LOCK_TIMEOUT_SECONDS = 300.0
MAX_COMMAND_BYTES = 1 << 20
MAX_COMMAND_ARGS = 512
CAPTURE_PROFILES = {'default': (), 'gc26-counter-expression': ('--counter-writes', '--expression-origins')}
SHA256_RE = re.compile(r"[0-9a-fA-F]{64}\Z")
FUNCTION_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")
CAPTURE_LIMITATIONS = (
    "The same-session producer does not serialize VarInfo.usage+0x04; "
    "it cannot answer local-FPR usage-ranking questions.",
)


class OwnerCaptureError(ValueError):
    """Raised when a capture request cannot be sealed or launched safely."""


def _profile_flags(profile: str) -> list[str]:
    if profile not in CAPTURE_PROFILES:
        raise OwnerCaptureError('unknown capture profile')
    return list(CAPTURE_PROFILES[profile])


def _profile_call(module: Any, profile: str, operation: Callable, *args: Any, **kwargs: Any) -> Any:
    """Use precisely the central CLI profile selection, restoring it on return.

    Central prepare/authentication still validates compiler identity and every
    hook byte; this adapter neither supplies alternative hooks nor relaxes them.
    """
    flags = _profile_flags(profile)
    if not flags:
        return operation(*args, **kwargs)
    hooks, counter = module.HOOKS, module.COUNTER_WRITES
    try:
        module.HOOKS = module.GC26_EXPRESSION_ORIGIN_HOOKS
        module.COUNTER_WRITES = True
        return operation(*args, **kwargs)
    finally:
        module.HOOKS, module.COUNTER_WRITES = hooks, counter


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise OwnerCaptureError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _read_json(path: Path, label: str) -> Any:
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise OwnerCaptureError(f"cannot read {label}: {path}: {exc}") from exc
    if len(raw) > MAX_COMMAND_BYTES:
        raise OwnerCaptureError(f"{label} exceeds {MAX_COMMAND_BYTES} bytes: {path}")
    try:
        return json.loads(raw.decode("utf-8"), object_pairs_hook=_reject_duplicate_pairs)
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise OwnerCaptureError(f"invalid {label}: {path}: {exc}") from exc


def _sha256_bytes(raw: bytes) -> str:
    return hashlib.sha256(raw).hexdigest()


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _canonical_existing(value: Path | str, label: str, *, directory: bool = False) -> Path:
    if value is None:
        raise OwnerCaptureError(f"{label} is missing")
    path = Path(value)
    if path.is_symlink():
        raise OwnerCaptureError(f"{label} is a symlink: {path}")
    try:
        resolved = path.resolve(strict=True)
    except OSError as exc:
        raise OwnerCaptureError(f"{label} is missing: {path}") from exc
    if resolved.is_symlink():
        raise OwnerCaptureError(f"{label} is a symlink: {path}")
    if directory != resolved.is_dir():
        kind = "directory" if directory else "file"
        raise OwnerCaptureError(f"{label} is not a {kind}: {resolved}")
    return resolved


def _canonical_new_directory(value: Path | str, label: str) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = Path.cwd() / path
    if path.exists() or path.is_symlink():
        raise OwnerCaptureError(f"{label} must be a fresh directory: {path}")
    try:
        parent = path.parent.resolve(strict=True)
    except OSError as exc:
        raise OwnerCaptureError(f"{label} parent is missing: {path.parent}") from exc
    if not parent.is_dir():
        raise OwnerCaptureError(f"{label} parent is not a directory: {parent}")
    return parent / path.name


def _resolve_from_root(value: Path | str, root: Path, label: str) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = root / path
    return _canonical_existing(path, label)


def _descriptor(path: Path, label: str) -> dict[str, Any]:
    path = _canonical_existing(path, label)
    return {"path": str(path), "size": path.stat().st_size, "sha256": _sha256_file(path)}


def _digest(value: Any, label: str) -> str:
    if not isinstance(value, str) or not SHA256_RE.fullmatch(value):
        raise OwnerCaptureError(f"{label} must be a SHA-256 digest")
    return value.lower()


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or value != value.strip() or any(
        char in value for char in "\0\r\n"
    ):
        raise OwnerCaptureError(f"{label} must be non-empty text without control characters")
    return value


def _write_json_new(path: Path, value: Mapping[str, Any]) -> None:
    if path.exists() or path.is_symlink():
        raise OwnerCaptureError(f"output already exists: {path}")
    try:
        with path.open("x", encoding="utf-8", newline="\n") as stream:
            json.dump(value, stream, ensure_ascii=True, indent=2, sort_keys=True)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
    except OSError as exc:
        raise OwnerCaptureError(f"cannot write {path}: {exc}") from exc


def _canonical_hash(value: Any) -> str:
    return _sha256_bytes(
        json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")).encode("utf-8")
    )


def _validate_command(value: Any) -> list[str]:
    if not isinstance(value, list) or not value:
        raise OwnerCaptureError("compiler command must be a non-empty JSON argv array")
    if len(value) > MAX_COMMAND_ARGS:
        raise OwnerCaptureError(f"compiler command exceeds {MAX_COMMAND_ARGS} argv entries")
    for index, item in enumerate(value):
        if not isinstance(item, str) or not item or item != item.strip() or any(
            char in item for char in "\0\r\n"
        ):
            raise OwnerCaptureError(f"compiler argv[{index}] is not a canonical string")
    return list(value)


def _load_command(value: Sequence[str] | Path | str, root: Path) -> tuple[list[str], dict[str, Any]]:
    """Load an argv list or a JSON file containing one; never parse a shell file."""

    command_descriptor: dict[str, Any]
    if isinstance(value, Sequence) and not isinstance(value, (str, bytes, Path)):
        command = _validate_command(list(value))
        command_descriptor = {"kind": "inline", "argv_sha256": _canonical_hash(command)}
        return command, command_descriptor

    if not isinstance(value, (str, Path)):
        raise OwnerCaptureError("compiler command must be an argv array or JSON file")
    text = os.fspath(value)
    if isinstance(value, Path) or not text.lstrip().startswith("["):
        command_path = Path(text)
        if not command_path.is_absolute():
            command_path = root / command_path
        command_path = _canonical_existing(command_path, "compiler command JSON")
        raw = command_path.read_bytes()
        if len(raw) > MAX_COMMAND_BYTES:
            raise OwnerCaptureError(f"compiler command JSON exceeds {MAX_COMMAND_BYTES} bytes: {command_path}")
        try:
            parsed = json.loads(raw.decode("utf-8"), object_pairs_hook=_reject_duplicate_pairs)
        except (UnicodeError, json.JSONDecodeError) as exc:
            raise OwnerCaptureError(f"invalid compiler command JSON: {command_path}: {exc}") from exc
        command = _validate_command(parsed)
        command_descriptor = {
            "kind": "file",
            "path": str(command_path),
            "size": len(raw),
            "sha256": _sha256_bytes(raw),
        }
        return command, command_descriptor

    try:
        parsed = json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
    except json.JSONDecodeError as exc:
        raise OwnerCaptureError(f"invalid inline compiler command JSON: {exc}") from exc
    command = _validate_command(parsed)
    return command, {"kind": "inline", "argv_sha256": _canonical_hash(command)}


def _resolve_file_arg(value: str, root: Path) -> Path | None:
    if not value or value.startswith("-"):
        return None
    path = Path(value)
    if not path.is_absolute():
        path = root / path
    if path.is_symlink() or not path.exists() or not path.is_file():
        return None
    try:
        return path.resolve(strict=True)
    except OSError:
        return None


def _normalise_command(
    command: Sequence[str], *, root: Path, source: Path, object_path: Path
) -> tuple[list[str], Path, Path]:
    values = _validate_command(list(command))
    source_indices = [index for index, item in enumerate(values) if item == "-c"]
    output_indices = [index for index, item in enumerate(values) if item == "-o"]
    if len(source_indices) != 1:
        raise OwnerCaptureError("compiler command must contain exactly one -c operand")
    if len(output_indices) != 1:
        raise OwnerCaptureError("compiler command must contain exactly one -o operand")
    source_index = source_indices[0]
    output_index = output_indices[0]
    for index, label in ((source_index, "-c"), (output_index, "-o")):
        if index + 1 >= len(values) or not values[index + 1] or values[index + 1].startswith("-"):
            raise OwnerCaptureError(f"compiler command {label} operand is missing")

    # Locate the two native images by their actual file identity.  This keeps
    # options and include paths free to move while still requiring the
    # wrapper-first transport expected by the authenticated capture tool.
    executable_candidates: list[tuple[int, Path]] = []
    for index, item in enumerate(values[:source_index]):
        candidate = _resolve_file_arg(item, root)
        if candidate is not None:
            executable_candidates.append((index, candidate))
    if len(executable_candidates) < 2:
        raise OwnerCaptureError("compiler command must name wrapper and compiler executables")
    wrapper_index, wrapper = executable_candidates[0]
    compiler_index, compiler = executable_candidates[1]
    if wrapper_index != 0 or compiler_index <= wrapper_index:
        raise OwnerCaptureError("compiler command must invoke the wrapper as its first argv entry")
    if compiler_index != 1:
        raise OwnerCaptureError("compiler command must place the compiler after the wrapper")
    if wrapper.suffix.lower() in {".bat", ".cmd", ".ps1", ".sh"} or compiler.suffix.lower() in {
        ".bat",
        ".cmd",
        ".ps1",
        ".sh",
    }:
        raise OwnerCaptureError("compiler command must invoke native executables, not shell scripts")

    normalized = list(values)
    normalized[wrapper_index] = str(wrapper)
    normalized[compiler_index] = str(compiler)
    normalized[source_index + 1] = str(source)
    normalized[output_index + 1] = str(object_path)
    return normalized, wrapper, compiler


def _load_capture_tool(tool_root: Path) -> Any:
    path = _canonical_existing(tool_root / "tools" / "capsule_same_session_capture.py", "capture tool")
    module_name = f"owner_capture_tool_{_sha256_file(path)[:16]}"
    spec = importlib.util.spec_from_file_location(module_name, path)
    if spec is None or spec.loader is None:
        raise OwnerCaptureError(f"cannot load capture tool: {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    try:
        spec.loader.exec_module(module)
    except Exception as exc:
        sys.modules.pop(module_name, None)
        raise OwnerCaptureError(f"cannot load capture tool: {path}: {exc}") from exc
    return module


def _function_binding(module: Any, source: Path, function: str) -> tuple[str, dict[str, int]]:
    try:
        source_bytes = source.read_bytes()
        source_text = source_bytes.decode("utf-8")
    except (OSError, UnicodeError) as exc:
        raise OwnerCaptureError(f"cannot read UTF-8 source for {function}: {source}") from exc
    extractor = getattr(getattr(module, "_donor_cfg", None), "_extract_function", None)
    if not callable(extractor):
        raise OwnerCaptureError("capture tool does not expose its canonical function extractor")
    try:
        info = extractor(source_text, function, str(source))
    except Exception as exc:
        raise OwnerCaptureError(f"cannot extract function {function} from {source}: {exc}") from exc
    function_bytes = str(info.function_text).encode("utf-8")
    byte_start = len(source_text[: int(info.source_start)].encode("utf-8"))
    byte_end = len(source_text[: int(info.source_end)].encode("utf-8"))
    if source_bytes[byte_start:byte_end] != function_bytes:
        raise OwnerCaptureError(f"function byte span does not reproduce {function}: {source}")
    return _sha256_bytes(function_bytes), {
        "byte_start": byte_start,
        "byte_end_exclusive": byte_end,
        "line_start": int(info.start_line),
        "line_end": int(info.end_line),
    }


def _check_descriptor(path: Path, expected: Mapping[str, Any], label: str) -> None:
    actual = _descriptor(path, label)
    if dict(expected) != actual:
        raise OwnerCaptureError(f"{label} identity changed: {path}")


def _check_command_descriptor(expected: Mapping[str, Any], label: str) -> None:
    if expected.get("kind") != "file":
        return
    path = _canonical_existing(expected.get("path"), label)
    actual = _descriptor(path, label)
    if any(expected.get(key) != actual[key] for key in ("path", "size", "sha256")):
        raise OwnerCaptureError(f"{label} identity changed: {path}")


def _validate_purpose(value: str | None, function: str) -> str:
    if value is None:
        return f"One unchanged-source {function} diagnostic capture"
    return _text(value, "authority purpose")


def _validate_scratch_basename(value: str) -> str:
    value = _text(value, "scratch basename")
    path = Path(value)
    if value in {".", ".."} or path.name != value or "/" in value or "\\" in value:
        raise OwnerCaptureError("scratch basename must be one plain filename")
    return value


def _native_timeouts(module: Any) -> dict[str, float]:
    values = {
        "startup": float(getattr(module, "MEMEXEC_STARTUP_TIMEOUT_SECONDS")),
        "terminal": float(getattr(module, "NATIVE_CAPTURE_TERMINAL_TIMEOUT_SECONDS")),
    }
    if any(not math.isfinite(value) or value <= 0 or value > 3600 for value in values.values()):
        raise OwnerCaptureError("capture tool exposes an invalid native timeout")
    return values


def prepare_capture(
    root: Path | str,
    source: Path | str,
    function: str,
    expected_source_sha256: str,
    compiler_command: Sequence[str] | Path | str,
    output_root: Path | str,
    *,
    tool_root: Path | str | None = None,
    authority_purpose: str | None = None,
    scratch_basename: str = "capture.o",
    capture_profile: str = "default",
) -> Path:
    """Seal one fresh capture directory without launching a compiler."""

    profile_flags = _profile_flags(capture_profile)
    root_path = _canonical_existing(root, "root", directory=True)
    source_path = _resolve_from_root(source, root_path, "source")
    if not FUNCTION_RE.fullmatch(function):
        raise OwnerCaptureError("function must be one C identifier")
    expected_digest = _digest(expected_source_sha256, "expected source SHA-256")
    source_descriptor = _descriptor(source_path, "source")
    if source_descriptor["sha256"] != expected_digest:
        raise OwnerCaptureError(
            f"unchanged-source guard failed for {source_path}: {source_descriptor['sha256']}"
        )
    output_path = _canonical_new_directory(output_root, "output root")
    tool_path = (
        _canonical_existing(tool_root, "tool root", directory=True)
        if tool_root is not None
        else Path(__file__).resolve().parents[1]
    )
    scratch_name = _validate_scratch_basename(scratch_basename)
    command, command_input = _load_command(compiler_command, root_path)
    object_path = output_path / "object" / scratch_name
    capture_module = _load_capture_tool(tool_path)
    function_sha256, function_span = _function_binding(capture_module, source_path, function)
    normalized_argv, wrapper_path, compiler_path = _normalise_command(
        command, root=root_path, source=source_path, object_path=object_path
    )
    capture_path = _canonical_existing(
        tool_path / "tools" / "capsule_same_session_capture.py", "capture tool"
    )
    layout_path = _canonical_existing(tool_path / "tools" / "mwcc_win32_varinfo.py", "native layout tool")
    inputs = {
        "source": source_descriptor,
        "compiler": _descriptor(compiler_path, "compiler"),
        "wrapper": _descriptor(wrapper_path, "wrapper"),
        "debugger": _descriptor(layout_path, "native layout tool"),
        "transport": _descriptor(capture_path, "capture tool"),
    }
    purpose = _validate_purpose(authority_purpose, function)
    authority: dict[str, Any] = {
        "purpose": purpose,
        "function": function,
        "function_sha256": function_sha256,
        "cwd": str(root_path),
        "argv": normalized_argv,
        **inputs,
        "compiler_command": command_input,
        "function_span": function_span,
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
    }
    if profile_flags:
        authority['capture_profile'] = capture_profile
        authority['capture_profile_flags'] = profile_flags

    output_path.mkdir(parents=False, exist_ok=False)
    (output_path / "object").mkdir()
    authority_path = output_path / "authority.json"
    _write_json_new(authority_path, authority)
    authority_descriptor = _descriptor(authority_path, "authority")
    manifest: dict[str, Any] = {
        "function": function,
        "function_sha256": function_sha256,
        "cwd": str(root_path),
        "argv": normalized_argv,
        **inputs,
        "authority": authority_descriptor,
    }
    manifest_path = output_path / "manifest.json"
    _write_json_new(manifest_path, manifest)
    prelaunch_path = output_path / "prelaunch-trust-root.json"
    _write_json_new(prelaunch_path, manifest)

    capture_dir = output_path / "capture"
    try:
        request_path = _profile_call(capture_module, capture_profile, capture_module.prepare_request,
            manifest_path, capture_dir, external_trust_root=manifest
        )
        trust_root: dict[str, Any] = {**manifest, "request": _descriptor(request_path, "request")}
        trust_path = output_path / "trust-root.json"
        _write_json_new(trust_path, trust_root)
        authenticated = _profile_call(capture_module, capture_profile, capture_module.authenticate_request,
            request_path, require_empty=True, external_trust_root=trust_root
        )
    except Exception as exc:
        if isinstance(exc, OwnerCaptureError):
            raise
        raise OwnerCaptureError(f"capture request preparation failed: {exc}") from exc

    # Seal-time rechecks catch an input replacement during function extraction,
    # manifest publication, or central request authentication.
    _check_descriptor(source_path, source_descriptor, "source")
    for name, before in inputs.items():
        _check_descriptor(Path(before["path"]), before, name)
    _check_command_descriptor(command_input, "compiler command JSON")
    _check_descriptor(authority_path, authority_descriptor, "authority")
    if any((output_path / "object").iterdir()):
        raise OwnerCaptureError(f"scratch object directory is not empty: {output_path / 'object'}")

    request_descriptor = _descriptor(request_path, "request")
    trust_descriptor = _descriptor(trust_path, "trust root")
    capture_args = [
        "capture",
        str(request_path),
        "--trust-root",
        str(trust_path),
        "--partial-evidence-dir",
        str(output_path / "partial-evidence"),
        *profile_flags,
    ]
    prepared: dict[str, Any] = {
        "schema": SCHEMA,
        "capture_profile": capture_profile,
        "status": PREPARED_STATUS,
        "root": str(root_path),
        "output_root": str(output_path),
        "tool_root": str(tool_path),
        "function": function,
        "function_sha256": function_sha256,
        "source": source_descriptor,
        "authority_purpose": purpose,
        "scratch_basename": scratch_name,
        "compiler_command": command_input,
        "authority": authority_descriptor,
        "manifest": _descriptor(manifest_path, "manifest"),
        "prelaunch_trust_root": _descriptor(prelaunch_path, "prelaunch trust root"),
        "request": request_descriptor,
        "trust_root": trust_descriptor,
        "scratch_object": str(object_path),
        "capture_output": str(capture_dir),
        "lock_path": str(root_path / "build" / ".compiler-lane.lock"),
        "launch_marker": str(output_path / "launch-started.json"),
        "capture_argv": capture_args,
        "preflight_argv": [
            "preflight",
            str(request_path),
            "--trust-root",
            str(trust_path),
            *profile_flags,
        ],
        "session_id": authenticated["request"]["session_id"],
        "hook_count": len(authenticated["hooks"]),
        "prelaunch_empty_output_proof": authenticated["prelaunch_empty_output_proof"],
        "native_timeouts_seconds": _native_timeouts(capture_module),
        "limitations": list(CAPTURE_LIMITATIONS),
        "notes": [
            "No compiler or capture is launched during preparation.",
            "The run operation launches exactly one capture under root/build/.compiler-lane.lock.",
        ],
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
    }
    prepared_path = output_path / "prepared.json"
    _write_json_new(prepared_path, prepared)
    return prepared_path


def _read_prepared(path: Path) -> tuple[dict[str, Any], Any, dict[str, Any], Path, Path, Path]:
    prepared_raw = _read_json(path, "prepared capture")
    if not isinstance(prepared_raw, Mapping):
        raise OwnerCaptureError("prepared capture must be an object")
    prepared = dict(prepared_raw)
    if prepared.get("schema") != SCHEMA or prepared.get("status") != PREPARED_STATUS:
        raise OwnerCaptureError("prepared capture is not a fresh PREPARED_NOT_LAUNCHED record")
    if prepared.get("diagnostic_only") is not True or prepared.get("board_admission") is not False:
        raise OwnerCaptureError("prepared capture policy metadata is invalid")
    root = _canonical_existing(prepared.get("root"), "prepared root", directory=True)
    output_root = _canonical_existing(prepared.get("output_root"), "prepared output root", directory=True)
    if path.parent != output_root:
        raise OwnerCaptureError("prepared capture is not rooted at output_root")
    tool_root = _canonical_existing(prepared.get("tool_root"), "prepared tool root", directory=True)
    source_descriptor = prepared.get("source")
    if not isinstance(source_descriptor, Mapping):
        raise OwnerCaptureError("prepared source descriptor is missing")
    source_path = _canonical_existing(source_descriptor.get("path"), "source")
    _check_descriptor(source_path, source_descriptor, "source")
    request_descriptor = prepared.get("request")
    trust_descriptor = prepared.get("trust_root")
    if not isinstance(request_descriptor, Mapping) or not isinstance(trust_descriptor, Mapping):
        raise OwnerCaptureError("prepared capture request/trust-root descriptors are missing")
    request_path = _canonical_existing(request_descriptor.get("path"), "request", directory=False)
    trust_path = _canonical_existing(trust_descriptor.get("path"), "trust root", directory=False)
    _check_descriptor(request_path, request_descriptor, "request")
    _check_descriptor(trust_path, trust_descriptor, "trust root")
    if request_path.parent != output_root / "capture" or trust_path.parent != output_root:
        raise OwnerCaptureError("prepared request/trust-root paths escape output root")
    trust_root = _read_json(trust_path, "trust root")
    return prepared, trust_root, dict(request_descriptor), root, output_root, tool_root


def run_capture(
    prepared_path: Path | str,
    *,
    lock_timeout_seconds: float = DEFAULT_LOCK_TIMEOUT_SECONDS,
    capture_runner: Callable[[Sequence[str]], int] | None = None,
) -> int:
    """Run one prepared capture under the project's shared compiler lock."""

    timeout = float(lock_timeout_seconds)
    if not math.isfinite(timeout) or timeout <= 0 or timeout > MAX_LOCK_TIMEOUT_SECONDS:
        raise OwnerCaptureError(
            f"lock timeout must be finite, positive, and <= {MAX_LOCK_TIMEOUT_SECONDS:g} seconds"
        )
    prepared_file = _canonical_existing(prepared_path, "prepared capture")
    prepared, trust_root, request_descriptor, root, output_root, tool_root = _read_prepared(prepared_file)
    _check_command_descriptor(prepared.get("compiler_command", {}), "compiler command JSON")
    if (output_root / "launch-started.json").exists() or (output_root / "launch-started.json").is_symlink():
        raise OwnerCaptureError(f"capture launch marker already exists: {output_root / 'launch-started.json'}")
    capture_dir = output_root / "capture"
    scratch_name = _validate_scratch_basename(prepared.get("scratch_basename"))
    scratch_object_value = prepared.get("scratch_object")
    if not isinstance(scratch_object_value, str):
        raise OwnerCaptureError("prepared scratch object path is missing")
    object_path = output_root / "object" / scratch_name
    if Path(scratch_object_value) != object_path:
        raise OwnerCaptureError("prepared scratch object path does not match scratch basename")
    if object_path.exists() or object_path.is_symlink():
        raise OwnerCaptureError(f"scratch object already exists: {object_path}")
    if (output_root / "partial-evidence").exists() or (output_root / "partial-evidence").is_symlink():
        raise OwnerCaptureError("partial evidence output already exists")

    capture_module = _load_capture_tool(tool_root)
    profile = prepared.get('capture_profile', 'default')
    profile_flags = _profile_flags(profile)
    authority_descriptor = prepared.get('authority', {})
    authority_path = _canonical_existing(authority_descriptor.get('path'), 'authority')
    _check_descriptor(authority_path, authority_descriptor, 'authority')
    authority = _read_json(authority_path, 'authority')
    if (authority.get('capture_profile', 'default') != profile
            or authority.get('capture_profile_flags', []) != profile_flags):
        raise OwnerCaptureError('prepared capture profile changed')
    request_path = _canonical_existing(request_descriptor["path"], "request")
    try:
        auth = _profile_call(capture_module, profile, capture_module.authenticate_request,
            request_path, require_empty=True, external_trust_root=trust_root
        )
    except Exception as exc:
        if isinstance(exc, OwnerCaptureError):
            raise
        raise OwnerCaptureError(f"prepared capture authentication failed: {exc}") from exc
    request = auth["request"]
    if request.get("function") != prepared.get("function") or request.get("function_sha256") != prepared.get(
        "function_sha256"
    ):
        raise OwnerCaptureError("prepared function binding does not match authenticated request")
    if request.get("session_id") != prepared.get("session_id"):
        raise OwnerCaptureError("prepared session binding does not match authenticated request")

    trust_path = _canonical_existing(output_root / "trust-root.json", "trust root")
    capture_args = [
        "capture",
        str(request_path),
        "--trust-root",
        str(trust_path),
        "--partial-evidence-dir",
        str(output_root / "partial-evidence"),
        *profile_flags,
    ]
    # The prepared argv is a sealed, human-readable record.  Compare by value
    # but invoke the freshly derived argument list, never by positional index.
    if prepared.get("capture_argv") != capture_args:
        raise OwnerCaptureError("prepared capture argv changed")
    if prepared.get('preflight_argv') != ['preflight', str(request_path), '--trust-root', str(trust_path), *profile_flags]:
        raise OwnerCaptureError('prepared preflight argv changed')
    if prepared.get('hook_count') != len(auth['hooks']):
        raise OwnerCaptureError('prepared profile hook count changed')

    build_dir = root / "build"
    if build_dir.exists() and (build_dir.is_symlink() or not build_dir.is_dir()):
        raise OwnerCaptureError(f"compiler lock parent is not a directory: {build_dir}")
    lock_path = root / "build" / ".compiler-lane.lock"
    if prepared.get("lock_path") != str(lock_path):
        raise OwnerCaptureError("prepared compiler lock path changed")
    marker = output_root / "launch-started.json"
    marker_value = {
        "schema": f"{SCHEMA}/launch",
        "status": LAUNCH_STATUS,
        "request": _descriptor(request_path, "request"),
        "session_id": request["session_id"],
        "capture_argv": capture_args,
        "diagnostic_only": True,
        "board_admission": False,
        "exactness_claim": False,
        "authority_advanced": False,
    }
    runner = capture_module.main if capture_runner is None else capture_runner
    if not callable(runner):
        raise OwnerCaptureError("capture runner is not callable")
    with _exclusive_lock(lock_path, timeout):
        # Reauthenticate after acquiring the shared lock so a source/tool
        # replacement cannot race the launch marker or native process.
        _profile_call(capture_module, profile, capture_module.authenticate_request,
                      request_path, require_empty=True, external_trust_root=trust_root)
        _write_json_new(marker, marker_value)
        result = runner(capture_args)
    if isinstance(result, bool) or not isinstance(result, int):
        raise OwnerCaptureError("capture runner returned a non-integer exit code")
    return result


def parser() -> argparse.ArgumentParser:
    command = argparse.ArgumentParser(
        description=__doc__,
        epilog="Known limitation: " + " ".join(CAPTURE_LIMITATIONS),
    )
    sub = command.add_subparsers(dest="command", required=True)
    prepare = sub.add_parser("prepare", help="seal one fresh capture without launching")
    prepare.add_argument("--root", required=True, type=Path)
    prepare.add_argument("--source", required=True, type=Path)
    prepare.add_argument("--function", required=True)
    prepare.add_argument("--expected-source-sha256", required=True)
    prepare.add_argument(
        "--compiler-command",
        "--compiler-command-json",
        "--command-json",
        dest="compiler_command",
        required=True,
        help="inline JSON argv array or path to a JSON file containing one",
    )
    prepare.add_argument("--output-root", "--output", dest="output_root", required=True, type=Path)
    prepare.add_argument("--tool-root", type=Path)
    prepare.add_argument("--authority-purpose")
    prepare.add_argument("--scratch-basename", default="capture.o")
    prepare.add_argument('--capture-profile', choices=tuple(CAPTURE_PROFILES), default='default')
    run = sub.add_parser("run", help="launch one prepared capture")
    run.add_argument("prepared", type=Path)
    run.add_argument("--lock-timeout", type=float, default=DEFAULT_LOCK_TIMEOUT_SECONDS)
    return command


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "prepare":
            path = prepare_capture(
                args.root,
                args.source,
                args.function,
                args.expected_source_sha256,
                args.compiler_command,
                args.output_root,
                tool_root=args.tool_root,
                authority_purpose=args.authority_purpose,
                scratch_basename=args.scratch_basename,
                capture_profile=args.capture_profile,
            )
            print(json.dumps({
                "schema": f"{SCHEMA}/prepare",
                "status": PREPARED_STATUS,
                "prepared": str(path),
                "limitations": list(CAPTURE_LIMITATIONS),
            }, sort_keys=True))
            return 0
        return run_capture(args.prepared, lock_timeout_seconds=args.lock_timeout)
    except (OSError, OwnerCaptureError, ValueError) as exc:
        status = "NOT_LAUNCHED"
        if getattr(args, "command", None) == "run":
            try:
                status = (
                    "UNKNOWN_AFTER_LAUNCH"
                    if (Path(args.prepared).resolve().parent / "launch-started.json").is_file()
                    else status
                )
            except (OSError, RuntimeError, TypeError, ValueError):
                pass
        print(json.dumps({"schema": f"{SCHEMA}/error", "status": status, "reason": str(exc)}, sort_keys=True), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
