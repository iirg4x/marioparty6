"""Real compiler/evidence adapter for owner-campaign v2.

The campaign runtime owns the monotonic frontier and the source transaction;
this module owns only the disposable measurement.  It deliberately composes
the repository's pinned ``crack_evidence_bundle`` and ``focus_symbol_report``
implementations so that an object-only measurement has the same objdiff,
relocation, and sibling semantics as the existing recovery tools.

The command never reads or changes STOP/permit state.  It accepts a source
already materialized in the caller's disposable worktree, proves the source,
target, unit, and toolchain bindings, and emits one small self-hashed JSON
measurement.  Raw objdiff reports and physical receipts are temporary and are
removed before the command returns.

``final_owner`` is intentionally stricter than measurement.  It inspects the
actual Ninja command response, requires the selected source-built object to be
an explicit linker input, rejects fallback ``asm``/``NonMatching`` inputs,
checks clean tracked state, runs pinned DTK ``shasum``, and compares linked
and retail bytes.  A caller-supplied source-link manifest is never accepted as
proof.
"""

from __future__ import annotations

import argparse
from contextlib import contextmanager
from dataclasses import dataclass, replace
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import time
from typing import Any, Iterable, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import crack_evidence_bundle as bundle
from tools import focus_symbol_report
from tools import owner_campaign_reconstruction as reconstruction
from tools.crack_contract import is_closed_objdiff_unit_name


MEASUREMENT_SCHEMA = "owner_campaign_measurement/v1"
FOCUS_SCHEMA = "owner_campaign_focus_evidence/v1"
FINAL_OWNER_SCHEMA = "owner_campaign_final_owner/v1"
SOURCE_LINK_PROOF_SCHEMA = "owner_campaign_source_link_proof/v1"
LINKED_PROOF_SCHEMA = "owner_campaign_linked_binary_proof/v1"
OWNER_PROOF_SCHEMA = "owner_campaign_full_owner_proof/v1"
SIBLING_PROOF_SCHEMA = "owner_campaign_sibling_proof/v1"
SHA_RE = re.compile(r"[0-9a-f]{64}\Z")
COMMIT_RE = re.compile(r"[0-9a-f]{40}\Z")
MAX_OUTPUT = 1 << 20
# Frontier/CRACK_REPORT receipts stay small, while focus evidence must retain
# complete residual identity arrays for large owners.  Keep the latter below
# the campaign's cell-temporary budget rather than silently dropping IDs.
# Keep the report/frontier envelope distinct from transient measurement data.
# A measurement carries the complete focus payload and proof receipts, so its
# bound must leave room for those receipts in addition to a 256 KiB focus.
MAX_REPORT_COMPACT = 64 * 1024
MAX_COMPACT = MAX_REPORT_COMPACT  # compatibility for existing callers
MAX_FOCUS_COMPACT = 256 * 1024
MAX_MEASUREMENT_COMPACT = 16 * 1024 * 1024
MAX_STABLE_IDENTITIES = 2048
REPARSE_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)


class MeasurementError(RuntimeError):
    """Evidence cannot be produced without guessing or unsafe I/O."""


@dataclass(frozen=True)
class Identity:
    phase: str
    campaign_id: str
    manifest_sha256: str
    owner: str
    unit: str
    function: str
    source_sha256: str
    target_object_sha256: str
    toolchain_sha256: str
    base_commit: str | None = None
    source_path: str | None = None


class Deadline:
    def __init__(self, seconds: float) -> None:
        if not isinstance(seconds, (int, float)) or seconds <= 0:
            raise MeasurementError("timeout must be positive")
        self.started = time.monotonic()
        self.seconds = float(seconds)

    def remaining(self) -> float:
        left = self.seconds - (time.monotonic() - self.started)
        if left <= 0:
            raise MeasurementError("measurement command deadline exceeded")
        return max(0.05, left)


def _canonical(value: Any) -> bytes:
    try:
        return json.dumps(
            value, ensure_ascii=False, sort_keys=True, separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise MeasurementError(f"value is not canonical JSON: {exc}") from exc


def _sha_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _sha_json(value: Any) -> str:
    return _sha_bytes(_canonical(value))


def _sha_file(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise MeasurementError(f"cannot hash {path}: {exc}") from exc
    return digest.hexdigest()


def _seal(value: Mapping[str, Any], field: str) -> dict[str, Any]:
    """Add a self-digest to a compact proof payload."""

    if field in value:
        raise MeasurementError(f"proof payload already contains {field}")
    body = dict(value)
    return {**body, field: _sha_json(body)}


def _valid_sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or SHA_RE.fullmatch(value) is None:
        raise MeasurementError(f"{label} is not a SHA-256")
    return value


def _valid_commit(value: str | None, label: str = "base_commit") -> str | None:
    if value is None or value == "":
        return None
    if COMMIT_RE.fullmatch(value) is None:
        raise MeasurementError(f"{label} is not a commit SHA")
    return value


def _assert_no_indirection(path: Path, *, missing_leaf: bool = False) -> None:
    """Reject symlink/reparse components before path access.

    This is intentionally the same conservative policy used by the bundle,
    with a missing-leaf option for paths created by the adapter.
    """

    absolute = Path(os.path.abspath(path))
    anchor = Path(absolute.anchor) if absolute.anchor else Path(absolute.parts[0])
    current = anchor
    parts = absolute.parts[1:] if absolute.anchor else absolute.parts[1:]
    for part in parts:
        current /= part
        try:
            info = current.lstat()
        except FileNotFoundError:
            if missing_leaf and current == absolute:
                return
            raise MeasurementError(f"path component does not exist: {current}")
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise MeasurementError(f"path indirection is forbidden: {current}")


def _absolute(raw: str | os.PathLike[str], *, root: Path, label: str,
              allow_external: bool, exists: bool) -> Path:
    if not isinstance(raw, (str, os.PathLike)):
        raise MeasurementError(f"{label} is invalid")
    value = os.fspath(raw)
    if not value or "\x00" in value:
        raise MeasurementError(f"{label} is invalid")
    path = Path(os.path.abspath(Path(value) if Path(value).is_absolute() else root / value))
    root_abs = Path(os.path.abspath(root))
    if not allow_external:
        try:
            path.relative_to(root_abs)
        except ValueError as exc:
            raise MeasurementError(f"{label} escapes disposable root: {path}") from exc
    _assert_no_indirection(path, missing_leaf=not exists)
    if exists and not path.is_file():
        raise MeasurementError(f"{label} is not a regular file: {path}")
    return path


def _safe_dir(path: Path, *, root: Path, label: str) -> Path:
    if not isinstance(path, (str, os.PathLike)):
        raise MeasurementError(f"{label} is invalid")
    value = os.fspath(path)
    if not value or "\x00" in value:
        raise MeasurementError(f"{label} is invalid")
    path = Path(os.path.abspath(Path(value) if Path(value).is_absolute() else root / value))
    root_abs = Path(os.path.abspath(root))
    try:
        path.relative_to(root_abs)
    except ValueError as exc:
        raise MeasurementError(f"{label} escapes disposable root: {path}") from exc

    # Validate the complete existing prefix, then create each missing directory
    # one component at a time and revalidate it.  ``Path.mkdir(parents=True)``
    # alone cannot distinguish an absent clean suffix from a symlink/reparse
    # component, while the old _absolute(..., exists=False) accepted only one
    # missing leaf and rejected clean Ninja output trees several levels deep.
    missing: list[Path] = []
    current = path
    while True:
        try:
            info = current.lstat()
        except FileNotFoundError:
            missing.append(current)
            parent = current.parent
            if parent == current:
                raise MeasurementError(f"cannot resolve existing prefix for {label}: {path}")
            current = parent
            continue
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise MeasurementError(f"path indirection is forbidden: {current}")
        if not current.is_dir():
            raise MeasurementError(f"{label} parent is not a directory: {current}")
        _assert_no_indirection(current)
        break

    for directory in reversed(missing):
        try:
            directory.mkdir()
        except FileExistsError:
            # A concurrent creator is acceptable only when it produced the same
            # ordinary directory; the immediate validation below decides that.
            pass
        except OSError as exc:
            raise MeasurementError(f"cannot create {label}: {directory}: {exc}") from exc
        _assert_no_indirection(directory)
        if not directory.is_dir():
            raise MeasurementError(f"{label} is not a directory: {directory}")
    return path


def _atomic_json(path: Path, value: Mapping[str, Any], *, limit: int | None = None) -> None:
    payload = _canonical(value) + b"\n"
    if limit is not None and len(payload) > limit:
        raise MeasurementError(f"artifact exceeds compact limit {limit}: {path}")
    _assert_no_indirection(path.parent)
    if path.exists() or path.is_symlink():
        _assert_no_indirection(path)
    fd, raw = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temporary = Path(raw)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        _assert_no_indirection(temporary)
        _assert_no_indirection(path.parent)
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _terminate_process(process: subprocess.Popen[bytes]) -> None:
    """Terminate the command and its process group after a deadline."""

    if process.poll() is not None:
        return
    try:
        if os.name == "nt":
            subprocess.run(
                ["taskkill", "/PID", str(process.pid), "/T", "/F"],
                stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL, timeout=2, check=False,
            )
        else:
            import signal

            os.killpg(process.pid, signal.SIGKILL)
    except (OSError, subprocess.SubprocessError):
        try:
            process.kill()
        except OSError:
            pass


def _run_bounded(command: Sequence[str], *, cwd: Path, deadline: Deadline,
                 label: str) -> str:
    normalized: list[str] = []
    for item in command:
        if isinstance(item, os.PathLike):
            item = os.fspath(item)
        if not isinstance(item, str) or not item:
            raise MeasurementError(f"{label} command is invalid")
        normalized.append(item)
    if not normalized:
        raise MeasurementError(f"{label} command is invalid")
    _assert_no_indirection(cwd)
    flags = getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0) if os.name == "nt" else 0
    kwargs: dict[str, Any] = {
        "cwd": cwd, "stdin": subprocess.DEVNULL,
        "stdout": subprocess.PIPE, "stderr": subprocess.PIPE,
        "creationflags": flags,
    }
    if os.name != "nt":
        kwargs["start_new_session"] = True
    try:
        process = subprocess.Popen(normalized, **kwargs)
    except OSError as exc:
        raise MeasurementError(f"{label} could not start: {exc}") from exc
    try:
        stdout, stderr = process.communicate(timeout=deadline.remaining())
    except subprocess.TimeoutExpired as exc:
        _terminate_process(process)
        try:
            process.communicate(timeout=2)
        except subprocess.TimeoutExpired:
            try:
                process.kill()
            except OSError:
                pass
        raise MeasurementError(f"{label} exceeded the bounded deadline") from exc
    if len(stdout) + len(stderr) > MAX_OUTPUT:
        raise MeasurementError(f"{label} output exceeded {MAX_OUTPUT} bytes")
    text = stdout.decode("utf-8", "replace")
    if process.returncode:
        detail = stderr.decode("utf-8", "replace").strip() or text.strip() or "no diagnostic"
        raise MeasurementError(f"{label} failed ({process.returncode}): {detail[:2000]}")
    return text


@contextmanager
def _bounded_bundle_runner(deadline: Deadline):
    """Route all bundle subprocesses through the adapter deadline.

    The bundle's proof/parsing code remains authoritative; only its old
    unbounded process launcher is replaced for this invocation.
    """

    original = bundle._run

    def bounded(command: Sequence[str], *, cwd: Path, label: str) -> str:
        return _run_bounded(command, cwd=cwd, deadline=deadline, label=label)

    bundle._run = bounded
    try:
        yield
    finally:
        bundle._run = original


def _env_or_arg(args: argparse.Namespace, name: str, env_name: str | None = None) -> str | None:
    value = getattr(args, name, None)
    if value is not None:
        return str(value)
    return os.environ.get(env_name or f"OWNER_CAMPAIGN_{name.upper()}")


def _identity(args: argparse.Namespace, phase: str) -> Identity:
    values = {
        "campaign_id": _env_or_arg(args, "campaign_id", "OWNER_CAMPAIGN_ID"),
        "manifest_sha256": _env_or_arg(args, "manifest_sha256"),
        "owner": _env_or_arg(args, "owner"),
        "unit": _env_or_arg(args, "unit"),
        "function": _env_or_arg(args, "function"),
        "source_sha256": _env_or_arg(args, "source_sha256"),
        "target_object_sha256": _env_or_arg(
            args, "target_object_sha256", "OWNER_CAMPAIGN_TARGET_SHA256"
        ),
        "toolchain_sha256": _env_or_arg(args, "toolchain_sha256"),
        "base_commit": _env_or_arg(args, "base_commit"),
    }
    missing = [key for key, value in values.items() if key != "base_commit" and not value]
    if missing:
        raise MeasurementError(f"missing campaign identity: {', '.join(missing)}")
    for key in ("campaign_id", "owner", "function"):
        if not values[key] or "\x00" in values[key]:
            raise MeasurementError(f"campaign identity {key} is invalid")
    unit = str(values["unit"])
    if not is_closed_objdiff_unit_name(unit):
        raise MeasurementError(f"unit is not a closed objdiff unit name: {unit}")
    return Identity(
        phase=phase,
        campaign_id=str(values["campaign_id"]),
        manifest_sha256=_valid_sha(values["manifest_sha256"], "manifest_sha256"),
        owner=str(values["owner"]), unit=unit, function=str(values["function"]),
        source_sha256=_valid_sha(values["source_sha256"], "source_sha256"),
        target_object_sha256=_valid_sha(values["target_object_sha256"], "target_object_sha256"),
        toolchain_sha256=_valid_sha(values["toolchain_sha256"], "toolchain_sha256"),
        base_commit=_valid_commit(values["base_commit"]),
    )


def _toolchain(args: argparse.Namespace, identity: Identity) -> tuple[dict[str, Any], Path, Path, Path, Path]:
    raw = _env_or_arg(args, "toolchain", "OWNER_CAMPAIGN_TOOLCHAIN_PATH")
    if not raw:
        raise MeasurementError("toolchain manifest path is required")
    manifest = _absolute(raw, root=Path.cwd(), label="toolchain manifest",
                         allow_external=True, exists=True)
    # owner_campaign binds the manifest descriptor's file SHA.  The legacy
    # bundle API instead accepts the manifest's internal self-hash/key.  Check
    # both domains explicitly; passing the descriptor SHA to the bundle makes
    # every real v2 run fail even when the manifest itself is valid.
    descriptor_sha = _sha_file(manifest)
    if descriptor_sha != identity.toolchain_sha256:
        raise MeasurementError(
            f"toolchain descriptor hash drifted: {descriptor_sha} != "
            f"{identity.toolchain_sha256}"
        )
    try:
        manifest_value = json.loads(manifest.read_bytes())
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise MeasurementError(f"toolchain manifest is unreadable: {manifest}: {exc}") from exc
    if not isinstance(manifest_value, Mapping) or manifest_value.get("schema") != "mp6_crack_toolchain/v1":
        raise MeasurementError("toolchain manifest schema is invalid")
    unsigned = dict(manifest_value)
    internal_key = unsigned.pop("manifest_sha256", None)
    _valid_sha(internal_key, "toolchain manifest_sha256")
    if _sha_json(unsigned) != internal_key:
        raise MeasurementError("toolchain manifest internal self-hash is invalid")
    try:
        loaded = bundle._load_toolchain(manifest, internal_key)
    except Exception as exc:
        raise MeasurementError(f"toolchain binding failed: {exc}") from exc
    if not isinstance(loaded, Mapping):
        raise MeasurementError("toolchain loader returned an invalid manifest")
    # Preserve the authenticated internal manifest digest for the compact
    # toolchain proof without changing the legacy loader's public shape.
    loaded = {**loaded, "_internal_manifest_sha256": internal_key}
    objdiff = Path(loaded["objdiff"]["path_object"])
    readelf = Path(loaded["binutils"]["path_object"]) / "powerpc-eabi-readelf.exe"
    ninja = Path(loaded["ninja"]["path_object"])
    dtk = Path(loaded["dtk"]["path_object"])
    for path, label in ((objdiff, "objdiff"), (readelf, "readelf"),
                        (ninja, "ninja"), (dtk, "dtk")):
        _absolute(path, root=Path.cwd(), label=label, allow_external=True, exists=True)
    return loaded, objdiff, readelf, ninja, dtk


def _source(args: argparse.Namespace, root: Path, identity: Identity) -> Path:
    raw = _env_or_arg(args, "source", "OWNER_CAMPAIGN_SOURCE_PATH")
    if not raw:
        raise MeasurementError("source path is required")
    path = _absolute(raw, root=root, label="source", allow_external=False, exists=True)
    actual = _sha_file(path)
    if actual != identity.source_sha256:
        raise MeasurementError(f"source hash drifted: {actual} != {identity.source_sha256}")
    return path


def _unit_objects(root: Path, identity: Identity) -> tuple[Path, Path]:
    try:
        target, candidate = bundle._unit_paths(root, identity.unit)
    except Exception as exc:
        raise MeasurementError(f"objdiff unit resolution failed: {exc}") from exc
    target = _absolute(target, root=root, label="target object", allow_external=False, exists=True)
    # A freshly configured Ninja graph need not have created the source-object
    # directory yet.  The candidate path is a build output declared by the
    # sealed objdiff unit, so create only its validated parent before asking
    # Ninja for the first compile.  Requiring every intermediate component to
    # pre-exist here prevented clean historical replays from reaching MWCC.
    _safe_dir(candidate.parent, root=root, label="candidate object directory")
    candidate = _absolute(candidate, root=root, label="candidate object", allow_external=False, exists=False)
    actual = _sha_file(target)
    if actual != identity.target_object_sha256:
        raise MeasurementError(f"target object hash drifted: {actual} != {identity.target_object_sha256}")
    return target, candidate


def _text(value: Any, limit: int = 120) -> str:
    if isinstance(value, str):
        text = value.replace("\\", "/").replace("\r", " ").replace("\n", " ")
    else:
        text = _canonical(value).decode("utf-8", "replace")
    if len(text) <= limit:
        return text
    return text[: max(0, limit - 14)] + "...(truncated)"


def _instruction_row_detail(channel: str, function: str, side: str,
                            row: Mapping[str, Any]) -> str:
    instruction = row.get("instruction")
    if not isinstance(instruction, Mapping):
        instruction = {}
    parts = [
        channel,
        function,
        side,
        f"row={row.get('index')}",
        f"kind={row.get('diff_kind', 'equal')}",
    ]
    for key, label in (("address", "addr"), ("formatted", "form"),
                       ("mnemonic", "mnemonic")):
        if key in instruction:
            parts.append(f"{label}={_text(instruction[key], 128)}")
            if key == "formatted":
                break
    if "arg_diff" in row:
        parts.append(f"arg={_text(row['arg_diff'], 96)}")
    relocation = instruction.get("relocation")
    if isinstance(relocation, Mapping):
        fields = []
        for key in ("type_name", "type", "target_symbol", "symbol", "effective_target", "addend"):
            if key in relocation:
                fields.append(f"{key}={_text(relocation[key], 64)}")
        if fields:
            parts.append("reloc=" + ";".join(fields))
    # The hash authenticates the complete normalized row even if the readable
    # rendering had to be bounded for the compact evidence contract.
    parts.append(f"row_sha256={_sha_json(row)}")
    rendered = "|".join(parts)
    return rendered if len(rendered) <= 512 else rendered[:498] + "...(truncated)"


def _stable_instruction_payload(row: Any) -> dict[str, Any] | None:
    """Return the target-anchored instruction payload used for residual IDs.

    Objdiff alignment indexes and candidate addresses move when a source cell
    changes.  The target instruction does not, so residual identity is derived
    from its complete normalized instruction payload (including a digest of
    omitted ``parts``).  Candidate-only insertions use their surrounding target
    instruction digests as anchors.  The resulting IDs are therefore stable
    across source edits while remaining independent of display formatting.
    """

    if not isinstance(row, Mapping):
        return None
    instruction = row.get("instruction")
    if not isinstance(instruction, Mapping):
        return None
    payload = {
        key: value for key, value in instruction.items() if key != "parts"
    }
    if "parts" in instruction:
        payload["parts_sha256"] = _sha_json(instruction["parts"])
    return payload


def _row_index_map(rows: Any, channel: str, side: str) -> dict[int, Mapping[str, Any]]:
    if not isinstance(rows, list):
        raise MeasurementError(f"focus {channel}.{side}.rows is invalid")
    result: dict[int, Mapping[str, Any]] = {}
    for position, raw in enumerate(rows):
        if not isinstance(raw, Mapping):
            raise MeasurementError(f"focus {channel}.{side}.rows[{position}] is invalid")
        index = raw.get("index", position)
        if isinstance(index, bool) or not isinstance(index, int) or index < 0:
            raise MeasurementError(f"focus {channel}.{side}.rows[{position}] index is invalid")
        if index in result:
            raise MeasurementError(f"focus {channel}.{side}.rows has duplicate index {index}")
        result[index] = raw
    return result


def _stable_row_id(
    channel: str,
    function: str,
    index: int,
    target_row: Mapping[str, Any] | None,
    candidate_row: Mapping[str, Any] | None,
    target_rows: Mapping[int, Mapping[str, Any]],
) -> str:
    """Build one canonical residual identity without candidate addresses/indexes."""

    target_payload = _stable_instruction_payload(target_row)
    candidate_payload = _stable_instruction_payload(candidate_row)
    target_kind = target_row.get("diff_kind") if target_row else None
    candidate_kind = candidate_row.get("diff_kind") if candidate_row else None
    kinds = sorted(
        value for value in (target_kind, candidate_kind)
        if isinstance(value, str) and value
    )
    if target_payload is not None:
        body: dict[str, Any] = {
            "schema": "owner_campaign_target_instruction_identity/v1",
            "channel": channel,
            "function": function,
            "target_instruction": target_payload,
            "target_kind": target_kind if isinstance(target_kind, str) else None,
        }
    else:
        # An INSERT has no target instruction.  Bind it to immutable target
        # neighbours; the candidate instruction is retained only to distinguish
        # two inserts between the same anchors.
        indices = sorted(target_rows)
        before = max((item for item in indices if item < index), default=None)
        after = min((item for item in indices if item > index), default=None)
        before_payload = _stable_instruction_payload(target_rows[before]) if before is not None else None
        after_payload = _stable_instruction_payload(target_rows[after]) if after is not None else None
        body = {
            "schema": "owner_campaign_candidate_insert_identity/v1",
            "channel": channel,
            "function": function,
            "before_target_sha256": _sha_json(before_payload) if before_payload is not None else None,
            "after_target_sha256": _sha_json(after_payload) if after_payload is not None else None,
            "candidate_instruction": candidate_payload,
            "candidate_kind": candidate_kind if isinstance(candidate_kind, str) else None,
        }
    return f"{channel}:instruction:{_sha_json(body)}"


def _stable_row_ids(focus: Mapping[str, Any], channel: str, function: str) -> list[str]:
    material = focus.get("channels", {}).get(channel)
    if not isinstance(material, Mapping):
        raise MeasurementError(f"focus channel is missing: {channel}")
    target_rows = _row_index_map(
        material.get("target", {}).get("rows") if isinstance(material.get("target"), Mapping) else None,
        channel, "target",
    )
    candidate_rows = _row_index_map(
        material.get("candidate", {}).get("rows") if isinstance(material.get("candidate"), Mapping) else None,
        channel, "candidate",
    )
    result: list[str] = []
    seen: set[str] = set()
    for index in sorted(set(target_rows) | set(candidate_rows)):
        target_row = target_rows.get(index)
        candidate_row = candidate_rows.get(index)
        target_kind = target_row.get("diff_kind") if target_row else None
        candidate_kind = candidate_row.get("diff_kind") if candidate_row else None
        if not (
            isinstance(target_kind, str) and target_kind
        ) and not (
            isinstance(candidate_kind, str) and candidate_kind
        ):
            continue
        row_id = _stable_row_id(
            channel, function, index, target_row, candidate_row, target_rows
        )
        if row_id in seen:
            raise MeasurementError(f"focus {channel} residual identity is ambiguous")
        seen.add(row_id)
        result.append(row_id)
    if len(result) > MAX_STABLE_IDENTITIES:
        raise MeasurementError(
            f"focus {channel} has too many stable residual identities "
            f"({len(result)} > {MAX_STABLE_IDENTITIES})"
        )
    metric = material.get("metric")
    if isinstance(metric, Mapping) and metric.get("diff_rows") != len(result):
        raise MeasurementError(
            f"focus {channel} residual identity count disagrees with metric"
        )
    return result


def _relocation_identity(row: Any) -> dict[str, Any]:
    if not isinstance(row, Mapping):
        raise MeasurementError("physical relocation entry is invalid")
    # Match the physical-proof comparator exactly.  Local symbol names and
    # symbol values are attribution metadata: MWCC may call the same .sdata2
    # owner ``lbl_...`` in the retail object and ``@380`` in the reconstructed
    # object while offset/type/effective target are byte-for-byte equivalent.
    # Hashing those display aliases made an already exact relocation set look
    # like a migrated frontier and incorrectly rejected exact candidates.
    required = ("offset", "type", "effective_target")
    if any(key not in row for key in required):
        raise MeasurementError("physical relocation identity is incomplete")
    return {key: row[key] for key in required}


def _physical_identity(focus: Mapping[str, Any], function: str) -> tuple[list[str], str, str]:
    physical = focus.get("physical_relocations")
    if not isinstance(physical, Mapping):
        raise MeasurementError("focus physical relocation payload is missing")
    differences = physical.get("physical_relocation_differences")
    if not isinstance(differences, list):
        raise MeasurementError("focus physical differences are invalid")
    target = physical.get("target")
    candidate = physical.get("candidate")
    if not isinstance(target, Mapping) or not isinstance(candidate, Mapping):
        raise MeasurementError("focus physical relocation sides are invalid")
    target_rows = target.get("physical_relocations")
    candidate_rows = candidate.get("physical_relocations")
    if not isinstance(target_rows, list) or not isinstance(candidate_rows, list):
        raise MeasurementError("focus physical relocation entries are missing")
    # Relocation ordering is a presentation detail.  Canonicalize each side
    # before hashing so a tool/objdiff row reorder cannot look like a new
    # physical frontier while offset/type/effective-target identity remains
    # unchanged.
    target_ids = sorted(
        (_relocation_identity(row) for row in target_rows), key=_canonical
    )
    candidate_ids = sorted(
        (_relocation_identity(row) for row in candidate_rows), key=_canonical
    )
    target_digest = _sha_json(target_ids)
    candidate_digest = _sha_json(candidate_ids)
    result: list[str] = []
    seen: set[str] = set()
    for difference in differences:
        if not isinstance(difference, Mapping):
            raise MeasurementError("physical difference is invalid")
        left = difference.get("target", [])
        right = difference.get("candidate", [])
        if not isinstance(left, list) or not isinstance(right, list):
            raise MeasurementError("physical difference sides are invalid")
        body = {
            "schema": "owner_campaign_physical_difference_identity/v1",
            "function": function,
            "target": sorted((_relocation_identity(row) for row in left), key=_canonical),
            "candidate": sorted((_relocation_identity(row) for row in right), key=_canonical),
        }
        row_id = f"physical:difference:{_sha_json(body)}"
        if row_id in seen:
            raise MeasurementError("physical difference identity is ambiguous")
        seen.add(row_id)
        result.append(row_id)
    if len(result) > MAX_STABLE_IDENTITIES:
        raise MeasurementError(
            f"focus has too many stable physical identities "
            f"({len(result)} > {MAX_STABLE_IDENTITIES})"
        )
    return sorted(result), target_digest, candidate_digest


def _focus_rows(focus: Mapping[str, Any], channel: str, function: str) -> list[str]:
    material = focus.get("channels", {}).get(channel)
    if not isinstance(material, Mapping):
        raise MeasurementError(f"focus channel is missing: {channel}")
    result: list[str] = []
    for side in ("target", "candidate"):
        side_value = material.get(side)
        if not isinstance(side_value, Mapping) or not isinstance(side_value.get("rows"), list):
            raise MeasurementError(f"focus {channel}.{side}.rows is invalid")
        for row in side_value["rows"]:
            if isinstance(row, Mapping) and row.get("diff_kind"):
                result.append(_instruction_row_detail(channel, function, side, row))
    return result


def _physical_row_detail(function: str, index: int, difference: Any) -> str:
    if not isinstance(difference, Mapping):
        return (
            f"physical:{function}:row={index}:invalid_difference="
            f"{_text(difference, 180)}:row_sha256={_sha_json(difference)}"
        )[:512]
    target = difference.get("target")
    candidate = difference.get("candidate")
    target_rows = target if isinstance(target, list) else []
    candidate_rows = candidate if isinstance(candidate, list) else []

    def keyed(rows: Sequence[Any]) -> dict[tuple[Any, Any], Any]:
        result: dict[tuple[Any, Any], Any] = {}
        for row in rows:
            if not isinstance(row, Mapping):
                continue
            key = (row.get("offset"), row.get("type"))
            result[key] = row.get("effective_target")
        return result

    left, right = keyed(target_rows), keyed(candidate_rows)
    changed: list[dict[str, Any]] = []
    for key in sorted(set(left) | set(right), key=lambda value: _canonical(value)):
        if left.get(key) != right.get(key):
            changed.append({
                "offset": key[0], "type": key[1],
                "target": left.get(key), "candidate": right.get(key),
            })
    payload = {
        "schema": "physical_difference_detail/v1",
        "function": function,
        "row": index,
        "mismatches": changed,
        "mismatch_count": len(changed),
        "difference_sha256": _sha_json(difference),
    }
    rendered = _canonical(payload).decode("utf-8", "replace")
    if len(rendered) <= 512:
        return rendered
    # Keep the actual first offsets/effective targets and explicitly identify
    # the omitted tail by digest; never silently turn a bundle mismatch into a
    # single opaque hash.
    for count in range(min(len(changed), 32), -1, -1):
        bounded = dict(payload)
        bounded["mismatches"] = changed[:count]
        bounded["mismatches_omitted"] = len(changed) - count
        rendered = _canonical(bounded).decode("utf-8", "replace")
        if len(rendered) <= 512:
            return rendered
    return (
        f"physical:{function}:row={index}:mismatch_count={len(changed)}:"
        f"difference_sha256={_sha_json(difference)}:detail_omitted=true"
    )[:512]


def _focus_physical_rows(focus: Mapping[str, Any], function: str) -> list[str]:
    physical = focus.get("physical_relocations")
    if not isinstance(physical, Mapping):
        raise MeasurementError("focus physical relocation payload is missing")
    differences = physical.get("physical_relocation_differences")
    if not isinstance(differences, list):
        raise MeasurementError("focus physical differences are invalid")
    return [_physical_row_detail(function, index, row) for index, row in enumerate(differences)]


def _metric(focus: Mapping[str, Any], channel: str) -> Mapping[str, Any]:
    channels = focus.get("channels")
    if not isinstance(channels, Mapping) or not isinstance(channels.get(channel), Mapping):
        raise MeasurementError(f"focus metric channel is missing: {channel}")
    metric = channels[channel].get("metric")
    if not isinstance(metric, Mapping):
        raise MeasurementError(f"focus metric is missing: {channel}")
    for key in ("target_size", "candidate_size", "diff_rows"):
        value = metric.get(key)
        if isinstance(value, bool) or not isinstance(value, int) or value < 0:
            raise MeasurementError(f"focus metric {channel}.{key} is invalid")
    return metric


def _protected(
    focus: Mapping[str, Any], expected_total: int | None,
    expected_names: Sequence[str] | None = None,
    focus_function: str | None = None,
) -> tuple[int, int, list[str], str]:
    """Return the protected census using the campaign's named identities.

    ``focus_symbol_report`` also exposes a total sibling census, but that total
    is not the campaign contract: it grows as other functions become exact.
    A production campaign therefore passes the immutable protected function
    names through ``OWNER_CAMPAIGN_PROTECTED_FUNCTIONS``.  The older numeric
    fallback remains for direct/unit-test callers that have no campaign list.
    """

    channel = focus.get("channels", {}).get("strict")
    if not isinstance(channel, Mapping):
        raise MeasurementError("focus strict channel is missing")
    value = channel.get("protected_siblings")
    if not isinstance(value, Mapping):
        raise MeasurementError("protected sibling census is missing")
    identities = value.get("exact_identities")
    if not isinstance(identities, list) or not all(isinstance(item, str) and item for item in identities):
        raise MeasurementError("protected sibling identities are invalid")
    sibling_count = value.get("sibling_count")
    exact_count = value.get("exact_sibling_count")
    if any(isinstance(item, bool) or not isinstance(item, int) or item < 0
           for item in (sibling_count, exact_count)):
        raise MeasurementError("protected sibling counts are invalid")
    if exact_count != len(identities) or exact_count > sibling_count:
        raise MeasurementError("protected sibling census is inconsistent")
    observed = set(identities)
    if expected_names is not None:
        all_names = list(expected_names)
        if any(not isinstance(item, str) or not item for item in all_names):
            raise MeasurementError("protected function names are invalid")
        if len(all_names) != len(set(all_names)):
            raise MeasurementError("protected function names are duplicated")
        names = [item for item in all_names if item != focus_function]
        if expected_total is not None and expected_total not in {
            len(all_names), len(names)
        }:
            raise MeasurementError("protected total disagrees with protected function names")
        total = len(names)
        losses = len(set(names) - observed)
    else:
        total = expected_total if expected_total is not None else sibling_count
        if total < exact_count:
            raise MeasurementError("protected total is smaller than exact sibling census")
        losses = total - exact_count
    digest = _sha_json(sorted(identities))
    return total, losses, sorted(identities), digest


def _bounded_descriptions(items: Sequence[str], *, label: str, keep: int | None = None) -> list[str]:
    """Bound one compact evidence list while authenticating omitted entries."""

    values = list(items)
    if keep is None or keep >= len(values):
        return values
    keep = max(0, keep)
    visible = values[:keep]
    omitted = values[keep:]
    visible.append(
        f"{label}:omitted={len(omitted)}:omitted_sha256={_sha_json(omitted)}"
    )
    return visible


def _fit_focus_body(body: dict[str, Any], lists: Mapping[str, Sequence[str]]) -> dict[str, Any]:
    """Fit focus evidence under the compact limit with deterministic sentinels."""

    if len(_canonical(body)) <= MAX_FOCUS_COMPACT:
        return body
    keeps = {name: len(values) for name, values in lists.items()}
    # Shrink the largest list first.  The sequence is deterministic and the
    # sentinel authenticates every omitted description.
    while True:
        for name, values in lists.items():
            body[name] = _bounded_descriptions(values, label=name, keep=keeps[name])
        if len(_canonical(body)) <= MAX_FOCUS_COMPACT:
            return body
        candidates = [name for name, value in keeps.items() if value > 0]
        if not candidates:
            raise MeasurementError("compact focus evidence cannot fit its fixed identity fields")
        name = max(candidates, key=lambda item: (keeps[item], item))
        keeps[name] = max(0, keeps[name] - max(1, keeps[name] // 10))


def _expected_total(args: argparse.Namespace) -> int | None:
    raw = _env_or_arg(args, "protected_total")
    if raw in (None, ""):
        return None
    try:
        value = int(raw, 10)
    except ValueError as exc:
        raise MeasurementError("protected_total is not an integer") from exc
    if value < 0:
        raise MeasurementError("protected_total is negative")
    return value


def _expected_protected_names(args: argparse.Namespace) -> list[str] | None:
    """Read the runtime's immutable protected identity list, if supplied."""

    if "OWNER_CAMPAIGN_PROTECTED_FUNCTIONS" in os.environ:
        raw = os.environ["OWNER_CAMPAIGN_PROTECTED_FUNCTIONS"]
        values = [] if raw == "" else raw.split(",")
    else:
        values = list(getattr(args, "protected_function", None) or [])
        if not values:
            return None
    if any(not isinstance(item, str) or not item for item in values):
        raise MeasurementError("protected function names are invalid")
    if len(values) != len(set(values)):
        raise MeasurementError("protected function names are duplicated")
    return values


def _focus_evidence(identity: Identity, focus: Mapping[str, Any], args: argparse.Namespace) -> dict[str, Any]:
    if not identity.source_path:
        raise MeasurementError("focus evidence requires a bound source_path")
    if not identity.base_commit:
        raise MeasurementError("focus evidence requires a bound base_commit")
    _valid_commit(identity.base_commit, "base_commit")
    strict_rows = _focus_rows(focus, "strict", identity.function)
    data_rows = _focus_rows(focus, "data", identity.function)
    physical_rows = _focus_physical_rows(focus, identity.function)
    strict_row_ids = _stable_row_ids(focus, "strict", identity.function)
    data_row_ids = _stable_row_ids(focus, "data", identity.function)
    physical_difference_ids, physical_target_identity_sha256, physical_candidate_identity_sha256 = (
        _physical_identity(focus, identity.function)
    )
    expected = _expected_total(args)
    expected_names = _expected_protected_names(args)
    total, losses, siblings, sibling_digest = _protected(
        focus, expected, expected_names, identity.function
    )
    physical = focus.get("physical_relocations")
    if not isinstance(physical, Mapping):
        raise MeasurementError("focus physical relocation payload is missing")
    target = physical.get("target")
    candidate = physical.get("candidate")
    if not isinstance(target, Mapping) or not isinstance(candidate, Mapping):
        raise MeasurementError("focus physical relocation sides are invalid")
    target_relocations = target.get("physical_relocations")
    candidate_relocations = candidate.get("physical_relocations")
    if not isinstance(target_relocations, list) or not isinstance(candidate_relocations, list):
        raise MeasurementError("focus physical relocation entries are missing")
    if len(target_relocations) > MAX_STABLE_IDENTITIES or len(candidate_relocations) > MAX_STABLE_IDENTITIES:
        raise MeasurementError("focus physical relocation census exceeds compact limit")
    for side, declared, rows in (
        ("target", target.get("physical_relocation_count"), target_relocations),
        ("candidate", candidate.get("physical_relocation_count"), candidate_relocations),
    ):
        if type(declared) is not int or declared < 0 or declared != len(rows):
            raise MeasurementError(f"focus {side} relocation count disagrees with entries")
    body: dict[str, Any] = {
        "schema": FOCUS_SCHEMA,
        "owner": identity.owner,
        "function": identity.function,
        "unit": identity.unit,
        "source_path": identity.source_path,
        "base_commit": identity.base_commit,
        "source_sha256": identity.source_sha256,
        "target_object_sha256": identity.target_object_sha256,
        "strict_rows": strict_rows,
        "data_rows": data_rows,
        "physical_differences": physical_rows,
        "strict_row_ids": strict_row_ids,
        "strict_row_ids_sha256": _sha_json(strict_row_ids),
        "data_row_ids": data_row_ids,
        "data_row_ids_sha256": _sha_json(data_row_ids),
        "physical_difference_ids": physical_difference_ids,
        "physical_difference_ids_sha256": _sha_json(physical_difference_ids),
        "physical_target_identity_sha256": physical_target_identity_sha256,
        "physical_candidate_identity_sha256": physical_candidate_identity_sha256,
        "strict_row_count": len(strict_row_ids),
        "data_row_count": len(data_row_ids),
        "physical_target_count": len(target_relocations),
        "physical_candidate_count": len(candidate_relocations),
        "physical_difference_count": len(physical_difference_ids),
        "protected_total": total,
        "protected_losses": losses,
        "sibling_identities": siblings,
        "sibling_digest": sibling_digest,
    }
    body = _fit_focus_body(
        body,
        {
            "strict_rows": strict_rows,
            "data_rows": data_rows,
            "physical_differences": physical_rows,
        },
    )
    body["focus_evidence_sha256"] = _sha_json(body)
    if len(_canonical(body)) > MAX_FOCUS_COMPACT:
        raise MeasurementError("compact focus evidence exceeds 256 KiB")
    return body


def _metrics(identity: Identity, focus: Mapping[str, Any], args: argparse.Namespace,
             *, source_link_exact: bool) -> tuple[dict[str, Any], dict[str, Any]]:
    strict = _metric(focus, "strict")
    data = _metric(focus, "data")
    physical = focus.get("physical_relocations")
    if not isinstance(physical, Mapping):
        raise MeasurementError("physical focus evidence is missing")
    target = physical.get("target")
    candidate = physical.get("candidate")
    differences = physical.get("physical_relocation_differences")
    if not isinstance(target, Mapping) or not isinstance(candidate, Mapping) or not isinstance(differences, list):
        raise MeasurementError("physical focus evidence is malformed")
    target_count = target.get("physical_relocation_count")
    candidate_count = candidate.get("physical_relocation_count")
    if any(isinstance(item, bool) or not isinstance(item, int) or item < 0
           for item in (target_count, candidate_count)):
        raise MeasurementError("physical relocation counts are invalid")
    total, losses, _siblings, _digest = _protected(
        focus, _expected_total(args), _expected_protected_names(args), identity.function
    )
    metrics = {
        "strict": {
            "target_bytes": strict["target_size"],
            "candidate_bytes": strict["candidate_size"],
            "differences": strict["diff_rows"],
        },
        "data": {
            "target_bytes": data["target_size"],
            "candidate_bytes": data["candidate_size"],
            "differences": data["diff_rows"],
        },
        "physical_target_count": target_count,
        "physical_candidate_count": candidate_count,
        "physical_differences": len(differences),
        "protected_total": total,
        "protected_losses": losses,
        "source_link_exact": source_link_exact,
    }
    receipts = {
        "strict": "",
        "data": "",
        "physical": "",
        "siblings": "",
        "source_link": "",
    }
    return metrics, receipts


def _pending_source_link(identity: Identity, candidate: Path) -> dict[str, Any]:
    body = {
        "schema": "owner_campaign_source_link_pending/v1",
        "status": "not_proven",
        "authority_advanced": False,
        "campaign_id": identity.campaign_id,
        "owner": identity.owner,
        "unit": identity.unit,
        "function": identity.function,
        "source_sha256": identity.source_sha256,
        "candidate_object_sha256": _sha_file(candidate) if candidate.is_file() else None,
        "reason": "measurement phase proves an object, not the final linked binary",
    }
    return _seal(body, "proof_sha256")


def _compact_source_link_proof(proof: Mapping[str, Any]) -> dict[str, Any]:
    """Publish a bounded, self-hashed source/object origin receipt.

    The full Ninja response can be large and is already authenticated by the
    compile response hashes in the adapter.  The compact receipt retains the
    actual paired compiler command plus a digest/count for the complete
    response, allowing an independent verifier to check the source/object
    relationship without retaining a multi-megabyte command transcript.
    """

    if not isinstance(proof, Mapping):
        raise MeasurementError("source-link proof is invalid")
    original_digest = proof.get("proof_sha256")
    _valid_sha(original_digest, "source-link proof_sha256")
    original_body = dict(proof)
    original_body.pop("proof_sha256", None)
    if _sha_json(original_body) != original_digest:
        raise MeasurementError("source-link proof digest is invalid")
    commands = proof.get("compiler_commands", [])
    if not isinstance(commands, list) or any(not isinstance(item, str) for item in commands):
        raise MeasurementError("source-link compiler commands are invalid")
    paired = proof.get("paired_compile_command_sha256")
    _valid_sha(paired, "paired_compile_command_sha256")
    # Keep a bounded copy of the paired command.  The response hash remains the
    # identity of the complete (unretained) command stream.
    paired_commands = [item for item in commands if _sha_bytes(item.encode("utf-8")) == paired]
    if not paired_commands:
        raise MeasurementError(
            "source-link proof paired compiler command is absent from the command stream"
        )
    body: dict[str, Any] = {
        "schema": "owner_campaign_source_link_proof/v1",
        "source_path": proof.get("source_path"),
        "source_sha256": proof.get("source_sha256"),
        "candidate_object_path": proof.get("candidate_object_path"),
        "candidate_object_sha256": proof.get("candidate_object_sha256"),
        "object_origin": proof.get("object_origin"),
        "fallback_asm_used": proof.get("fallback_asm_used"),
        "nonmatching_fallback_linked": proof.get("nonmatching_fallback_linked"),
        "authority_advanced": proof.get("authority_advanced"),
        "original_proof_sha256": original_digest,
        "compiler_command_count": len(commands),
        "compiler_commands_sha256": _sha_json(commands),
        "paired_compile_command_sha256": paired,
        "paired_compile_commands": paired_commands[:2],
        "before_response_sha256": proof.get("before_response_sha256"),
        "after_response_sha256": proof.get("after_response_sha256"),
    }
    for field in ("source_sha256", "candidate_object_sha256"):
        if body[field] is not None:
            _valid_sha(body[field], f"source-link {field}")
    return _seal(body, "proof_sha256")


def _object_proof(identity: Identity, candidate: Path, *, root: Path | None) -> dict[str, Any]:
    if not candidate.is_file():
        raise MeasurementError("candidate object proof requires an object")
    path = candidate
    if root is not None:
        try:
            path_text = candidate.relative_to(root).as_posix()
        except ValueError as exc:
            raise MeasurementError("candidate object proof path escapes root") from exc
    else:
        path_text = path.name
    body = {
        "schema": "owner_campaign_object_proof/v1",
        "owner": identity.owner,
        "unit": identity.unit,
        "function": identity.function,
        "candidate_object_path": path_text,
        "candidate_object_sha256": _sha_file(candidate),
        "candidate_object_size": candidate.stat().st_size,
        "source_sha256": identity.source_sha256,
        "authority_advanced": False,
    }
    return _seal(body, "proof_sha256")


def _toolchain_proof(
    identity: Identity,
    loaded: Mapping[str, Any] | None,
    paths: Sequence[Path] = (),
) -> dict[str, Any]:
    components: dict[str, Any] = {}
    names = ("objdiff", "readelf", "ninja", "dtk")
    for name, path in zip(names, paths):
        components[name] = {
            "path": Path(path).name,
            "sha256": _sha_file(Path(path)),
            "size_bytes": Path(path).stat().st_size,
        }
    internal = loaded.get("_internal_manifest_sha256") if isinstance(loaded, Mapping) else None
    if internal is not None:
        _valid_sha(internal, "toolchain internal manifest_sha256")
    body = {
        "schema": "owner_campaign_toolchain_proof/v1",
        "descriptor_sha256": identity.toolchain_sha256,
        "manifest_sha256": internal,
        "components": components,
        "authority_advanced": False,
    }
    return _seal(body, "proof_sha256")


def _measurement(identity: Identity, focus: Mapping[str, Any], args: argparse.Namespace,
                  *, strict_path: Path, data_path: Path, physical_path: Path,
                  candidate: Path, source_link_exact: bool = False,
                  source_link_receipt: str | None = None,
                  source_link_proof: Mapping[str, Any] | None = None,
                  toolchain_proof: Mapping[str, Any] | None = None,
                  root: Path | None = None) -> dict[str, Any]:
    if identity.base_commit is None:
        raise MeasurementError("measurement requires a bound base_commit")
    if not identity.source_path:
        raise MeasurementError("measurement requires a bound source_path")
    focus_evidence = _focus_evidence(identity, focus, args)
    reconstruction_packet: dict[str, Any] | None = None
    if root is not None:
        source = root / identity.source_path
        try:
            source_text = source.read_text(encoding="utf-8")
            source_span = reconstruction.source_span_metadata(
                source_text, identity.function
            )
            # Build from the complete in-memory focus report, before compact
            # focus fitting can omit descriptions.  Pass canonical row
            # identities through the builder's explicit override contract so
            # it validates the original focus digest first, then reseals the
            # augmented row census.  Mutating the focus mapping here would put
            # the original artifact hash out of sync with its contents.
            reconstruction_packet = reconstruction.build_packet(
                focus,
                {
                    "owner": identity.owner,
                    "unit": identity.unit,
                    "function": identity.function,
                    "source_path": identity.source_path,
                    "source_sha256": identity.source_sha256,
                    "base_commit": identity.base_commit,
                    "target_object_sha256": identity.target_object_sha256,
                    "candidate_object_sha256": _sha_file(candidate),
                    "toolchain_sha256": identity.toolchain_sha256,
                    "frontier_source_sha256": identity.source_sha256,
                },
                source_span,
                strict_row_ids=focus_evidence["strict_row_ids"],
                data_row_ids=focus_evidence["data_row_ids"],
                physical_difference_ids=focus_evidence[
                    "physical_difference_ids"
                ],
            )
            reconstruction.verify_packet(reconstruction_packet)
        except (OSError, UnicodeError, reconstruction.ReconstructionPacketError) as exc:
            raise MeasurementError(
                f"target-first reconstruction packet failed: {exc}"
            ) from exc
    metrics, receipts = _metrics(identity, focus, args, source_link_exact=source_link_exact)
    if source_link_exact and source_link_proof is None and source_link_receipt is None:
        raise MeasurementError(
            "source_link_exact requires an authenticated compiler-source proof"
        )
    if source_link_proof is not None:
        source_link_proof = _compact_source_link_proof(source_link_proof)
        if source_link_proof.get("source_sha256") != identity.source_sha256:
            raise MeasurementError("source-link proof source is not measurement-bound")
        if source_link_proof.get("candidate_object_sha256") != _sha_file(candidate):
            raise MeasurementError("source-link proof object is not measurement-bound")
        source_link_receipt = source_link_proof["proof_sha256"]
    elif source_link_receipt is None:
        source_link_proof = _pending_source_link(identity, candidate)
        source_link_receipt = source_link_proof["proof_sha256"]
    if toolchain_proof is None:
        toolchain_proof = _seal({
            "schema": "owner_campaign_toolchain_proof/v1",
            "descriptor_sha256": identity.toolchain_sha256,
            "manifest_sha256": None,
            "components": {},
            "authority_advanced": False,
        }, "proof_sha256")
    else:
        toolchain_proof = dict(toolchain_proof)
        _valid_sha(toolchain_proof.get("proof_sha256"), "toolchain proof_sha256")
    object_proof = _object_proof(identity, candidate, root=root)
    sibling_proof = {
        "schema": "owner_campaign_sibling_gate/v1",
        "sibling_identities": focus_evidence["sibling_identities"],
        "sibling_digest": focus_evidence["sibling_digest"],
        "protected_total": metrics["protected_total"],
        "protected_losses": metrics["protected_losses"],
        "authority_advanced": False,
    }
    receipts.update({
        "strict": _sha_file(strict_path),
        "data": _sha_file(data_path),
        "physical": _sha_file(physical_path),
        "siblings": _sha_json(sibling_proof),
        "source_link": source_link_receipt,
        "focus": focus_evidence["focus_evidence_sha256"],
        "object": object_proof["proof_sha256"],
        "toolchain": toolchain_proof["proof_sha256"],
    })
    body: dict[str, Any] = {
        "schema": MEASUREMENT_SCHEMA,
        "phase": identity.phase,
        "campaign_id": identity.campaign_id,
        "manifest_sha256": identity.manifest_sha256,
        "owner": identity.owner,
        "unit": identity.unit,
        "function": identity.function,
        "source_path": identity.source_path,
        "base_commit": identity.base_commit,
        "source_sha256": identity.source_sha256,
        "target_object_sha256": identity.target_object_sha256,
        "toolchain_sha256": identity.toolchain_sha256,
        "measurement_producer_sha256": _sha_file(Path(__file__).resolve()),
        "candidate_object_sha256": _sha_file(candidate),
        "metrics": metrics,
        "report_receipts": receipts,
        "proofs": {
            "source_link": source_link_proof,
            "object": object_proof,
            "toolchain": toolchain_proof,
        },
        "focus_evidence": focus_evidence,
        "exact_report": None,
    }
    if reconstruction_packet is not None:
        body["reconstruction_evidence"] = reconstruction_packet
    return {**body, "measurement_sha256": _sha_json(body)}


def _run_reports(*, root: Path, identity: Identity, target: Path, candidate: Path,
                 objdiff: Path, readelf: Path, temp: Path,
                 deadline: Deadline) -> tuple[dict[str, Any], Path, Path, Path]:
    strict_path = temp / "strict.json"
    data_path = temp / "data.json"
    physical_path = temp / "physical.json"
    with _bounded_bundle_runner(deadline):
        bundle._run_objdiff(objdiff, target, candidate, strict_path, data=False, root=root)
        bundle._run_objdiff(objdiff, target, candidate, data_path, data=True, root=root)
        physical = bundle._physical_receipt(target, candidate, identity.function, strict_path, readelf)
        bundle._atomic_json(physical_path, physical)
    try:
        focus = focus_symbol_report.build_from_paths(
            strict_report_path=strict_path,
            data_report_path=data_path,
            function=identity.function,
            expected_strict_report_sha256=_sha_file(strict_path),
            expected_data_report_sha256=_sha_file(data_path),
            physical_receipt_path=physical_path,
            expected_physical_receipt_sha256=_sha_file(physical_path),
            # Snapshot/candidate measurements must preserve and score a
            # nonexact physical frontier.  Physical exactness is mandatory for
            # an exact result, but rejecting it here makes every partial owner
            # impossible to enter the monotonic campaign in the first place.
            require_physical=False,
        )
    except Exception as exc:
        raise MeasurementError(f"focus evidence construction failed: {exc}") from exc
    return focus, strict_path, data_path, physical_path


def _assert_source_compile_provenance(command_text: str, *, root: Path,
                                      source: Path, candidate: Path,
                                      owner: str) -> dict[str, Any]:
    """Bind a produced object to the source compile command Ninja exposes.

    This is deliberately an object-origin proof, not a full linked-DOL proof.
    The latter is performed by :func:`final_owner`.  Requiring source and
    object on the same actual Ninja command prevents a stale object or a
    matching fallback assembly object from satisfying the measurement gate.
    """

    if not command_text.strip():
        raise MeasurementError("Ninja compiler command response is empty")
    lines = [line.strip() for line in command_text.splitlines() if line.strip()]
    fallback: list[str] = []
    for line in lines:
        normalized = line.replace("\\", "/").lower()
        if re.search(r"(?:^|[\s\"'=/:])asm/", normalized) or "nonmatching" in normalized:
            fallback.append(line)
    if fallback:
        raise MeasurementError(
            f"source compile provenance contains fallback asm/NonMatching input for {owner}: "
            f"{fallback[0][:500]}"
        )
    paired = [
        line for line in lines
        if _command_mentions(source, line, root)
        and _command_produces_candidate(source, candidate, line, root)
    ]
    if not paired:
        raise MeasurementError(
            "Ninja compiler response does not bind the source and produced object on one command"
        )
    return _seal({
        "schema": "owner_campaign_source_compile_proof/v1",
        "owner": owner,
        "source_path": source.relative_to(root).as_posix(),
        "source_sha256": _sha_file(source),
        "candidate_object_path": candidate.relative_to(root).as_posix(),
        "candidate_object_sha256": _sha_file(candidate) if candidate.is_file() else None,
        "compiler_commands": lines,
        "paired_compile_command_sha256": _sha_bytes(paired[0].encode("utf-8")),
        "object_origin": "reconstructed_source",
        "fallback_asm_used": False,
        "nonmatching_fallback_linked": False,
        "authority_advanced": False,
    }, "proof_sha256")


def _command_produces_candidate(source: Path, candidate: Path, command: str,
                                root: Path) -> bool:
    """Recognize file- or directory-valued MWCC ``-o`` output bindings.

    The MP6 Ninja rule passes an output *directory* to CodeWarrior and the
    compiler derives ``mgcall.o`` from ``mgcall.c``.  Requiring the final object
    filename to appear literally rejected the real compiler command even
    though Ninja had returned it for that exact output edge.  Directory output
    is accepted only when source and object basenames agree.
    """

    tokens = [item.strip('"') for item in re.findall(r'"[^"]*"|\S+', command)]
    outputs: list[str] = []
    for index, token in enumerate(tokens):
        lowered = token.lower()
        if lowered == "-o" and index + 1 < len(tokens):
            outputs.append(tokens[index + 1])
        elif lowered.startswith("-o="):
            outputs.append(token[3:])
    for raw in outputs:
        output = Path(raw)
        if not output.is_absolute():
            output = root / output
        output = Path(os.path.abspath(output))
        if output == candidate:
            return True
        if output == candidate.parent and source.stem.lower() == candidate.stem.lower():
            return True
    return False


def measure_current(*, root: Path, args: argparse.Namespace) -> dict[str, Any]:
    """Compile and prove one snapshot/candidate in a disposable worktree."""

    root = Path(os.path.abspath(root))
    _assert_no_indirection(root)
    if Path(os.path.abspath(Path.cwd())) != root:
        raise MeasurementError("adapter cwd must be the disposable root")
    phase = str(_env_or_arg(args, "phase") or "")
    if phase not in {"snapshot", "candidate"}:
        raise MeasurementError("measurement phase must be snapshot or candidate")
    identity = _identity(args, phase)
    source = _source(args, root, identity)
    try:
        source_path = source.relative_to(root).as_posix()
    except ValueError as exc:
        raise MeasurementError("source path is not relative to disposable root") from exc
    identity = replace(identity, source_path=source_path)
    timeout_raw = _env_or_arg(args, "timeout")
    timeout = float(timeout_raw) if timeout_raw else 1800.0
    deadline = Deadline(timeout)
    output_raw = _env_or_arg(args, "output", "OWNER_CAMPAIGN_MEASUREMENT_PATH")
    if not output_raw:
        raise MeasurementError("measurement output path is required")
    output = _absolute(output_raw, root=root, label="measurement output",
                       allow_external=False, exists=False)
    _safe_dir(output.parent, root=root, label="measurement output directory")
    if output.exists() or output.is_symlink():
        _assert_no_indirection(output)
        output.unlink()
    staged: Path | None = None
    temp: Path | None = None
    failure: BaseException | None = None
    result: dict[str, Any] | None = None
    try:
        loaded, objdiff, readelf, ninja, _dtk = _toolchain(args, identity)
        dtk = _dtk
        toolchain_proof = _toolchain_proof(
            identity, loaded, (objdiff, readelf, ninja, dtk)
        )
        with _bounded_bundle_runner(deadline):
            try:
                bundle._verify_objdiff(objdiff)
                bundle._verify_readelf(readelf)
                bundle._verify_ninja(ninja)
                staged = bundle._ensure_configured(root, loaded, ninja)
            except Exception as exc:
                raise MeasurementError(f"detached configuration failed: {exc}") from exc
        # Configure/build tools must not be allowed to rewrite the bound source.
        if _sha_file(source) != identity.source_sha256:
            raise MeasurementError("source changed during configuration")
        target, candidate = _unit_objects(root, identity)
        with _bounded_bundle_runner(deadline):
            compile_commands_before = bundle._run(
                [str(ninja), "-t", "commands", str(candidate.relative_to(root))],
                cwd=root, label="Ninja source compiler input response",
            )
        # The object may exist from a previous cell, but it is not accepted as
        # provenance until the current source/object command pair is sealed.
        compile_proof = _assert_source_compile_provenance(
            compile_commands_before, root=root, source=source, candidate=candidate,
            owner=identity.owner,
        )
        temp = Path(tempfile.mkdtemp(prefix=".owner-campaign-measure-", dir=root / "build"))
        _assert_no_indirection(temp)
        if candidate.exists():
            _assert_no_indirection(candidate)
            candidate.unlink()
        with _bounded_bundle_runner(deadline):
            bundle._run([str(ninja), "-j1", str(candidate.relative_to(root))],
                        cwd=root, label="candidate unit build")
        candidate = _absolute(candidate, root=root, label="candidate object",
                              allow_external=False, exists=True)
        with _bounded_bundle_runner(deadline):
            compile_commands_after = bundle._run(
                [str(ninja), "-t", "commands", str(candidate.relative_to(root))],
                cwd=root, label="Ninja source compiler input response after build",
            )
        compile_proof_after = _assert_source_compile_provenance(
            compile_commands_after, root=root, source=source, candidate=candidate,
            owner=identity.owner,
        )
        if compile_proof["compiler_commands"] != compile_proof_after["compiler_commands"]:
            raise MeasurementError("Ninja source compiler provenance changed during the build")
        compile_proof_body = dict(compile_proof_after)
        # Adding the two response digests changes the sealed payload; retain
        # neither the stale digest from ``compile_proof_after`` nor an
        # unauthenticated extension of it.
        compile_proof_body.pop("proof_sha256", None)
        compile_proof = _seal({
            **compile_proof_body,
            "candidate_object_sha256": _sha_file(candidate),
            "before_response_sha256": _sha_bytes(compile_commands_before.encode("utf-8")),
            "after_response_sha256": _sha_bytes(compile_commands_after.encode("utf-8")),
        }, "proof_sha256")
        if _sha_file(source) != identity.source_sha256:
            raise MeasurementError("source changed during candidate build")
        candidate_before_reports_sha = _sha_file(candidate)
        focus, strict_path, data_path, physical_path = _run_reports(
            root=root, identity=identity, target=target, candidate=candidate,
            objdiff=objdiff, readelf=readelf, temp=temp, deadline=deadline,
        )
        # Reports are generated from files in the disposable worktree.  Bind
        # the published measurement to a final stable source/target/object
        # snapshot, and reject any concurrent build or source drift before the
        # compact evidence is constructed or published.
        _assert_bound_snapshot(
            source=source, target=target, candidate=candidate,
            expected_source_sha256=identity.source_sha256,
            expected_target_sha256=identity.target_object_sha256,
            expected_candidate_sha256=candidate_before_reports_sha,
            label="report generation",
        )
        result = _measurement(
            identity, focus, args, strict_path=strict_path, data_path=data_path,
            physical_path=physical_path, candidate=candidate,
            source_link_exact=True, source_link_proof=compile_proof,
            toolchain_proof=toolchain_proof, root=root,
        )
        _atomic_json(output, result, limit=MAX_MEASUREMENT_COMPACT)
        return result
    except BaseException as exc:
        failure = exc
    finally:
        cleanup_errors: list[str] = []
        if staged is not None:
            try:
                bundle._remove_staged_retail(staged)
            except Exception as exc:
                cleanup_errors.append(f"retail cleanup: {exc}")
        if temp is not None:
            try:
                shutil.rmtree(temp)
            except Exception as exc:
                cleanup_errors.append(f"temporary evidence cleanup: {exc}")
        if cleanup_errors and failure is None:
            failure = MeasurementError("; ".join(cleanup_errors))
        elif cleanup_errors and failure is not None:
            failure.add_note("cleanup incomplete: " + "; ".join(cleanup_errors))
        if failure is not None:
            try:
                output.unlink(missing_ok=True)
            except OSError:
                pass
    if failure is not None:
        raise failure
    if result is None:
        raise MeasurementError("measurement ended without a result")
    return result


def _normal(path: Path) -> str:
    return str(path.resolve()).replace("\\", "/").lower()


def _command_mentions(path: Path, command_text: str, root: Path) -> bool:
    values = {
        _normal(path),
        str(path.resolve()).replace("\\", "/").lower(),
        path.relative_to(root).as_posix().lower() if path.is_relative_to(root) else "",
    }
    normalized = command_text.replace("\\", "/").lower()
    return any(value and value in normalized for value in values)


def _owner_fallback_aliases(*, root: Path, candidate: Path, source: Path,
                            owner: str) -> set[str]:
    """Return path aliases that identify fallback input for this owner.

    A Ninja response for a DOL contains commands for every dependency.  The
    presence of an unrelated ``asm/`` input is therefore not evidence that the
    selected owner was linked from fallback assembly.  Derive aliases only
    from the selected source/object and the owner tail, and apply them to
    linker inputs below.
    """

    aliases: set[str] = set()

    def add_path(path: Path) -> None:
        try:
            relative = path.relative_to(root).as_posix().lower()
        except ValueError:
            relative = path.as_posix().lower()
        relative = relative.lstrip("./")
        if relative:
            aliases.add(relative)
            aliases.add("/" + relative)

    for path in (candidate, source):
        add_path(path)
        try:
            relative_path = path.relative_to(root)
        except ValueError:
            relative_path = path
        for suffix in (".o", ".s"):
            add_path(relative_path.with_suffix(suffix))
        parts = list(relative_path.parts)
        for index, part in enumerate(parts):
            if part.lower() == "src":
                parts[index] = "asm"
                for suffix in (".o", ".s"):
                    add_path(Path(*parts).with_suffix(suffix))
                break

    owner_tail = owner.replace("\\", "/").replace(":", "/").split("/")[-1]
    stems = {candidate.stem.lower(), source.stem.lower(), owner_tail.lower()}
    for stem in stems:
        if not stem:
            continue
        for suffix in (".o", ".s"):
            aliases.add(f"asm/board/{stem}{suffix}")
            aliases.add(f"/asm/board/{stem}{suffix}")
    return aliases


def _fallback_input_matches_owner(line: str, aliases: set[str],
                                   stems: set[str]) -> bool:
    normalized = line.replace("\\", "/").lower()
    if not ("nonmatching" in normalized or "asm/" in normalized):
        return False
    # Inspect only fallback-looking command tokens.  Testing the complete
    # linker line would make the selected ``build/<owner>.o`` itself match
    # the alias whenever an unrelated ``asm/`` dependency is present.
    tokens = [
        token.strip("\"'(),[]{}")
        for token in re.split(r"\s+", normalized)
        if "asm/" in token or "nonmatching" in token
    ]
    if any(any(alias in token for alias in aliases) for token in tokens):
        return True
    # Some build graphs label a fallback input as ``NonMatching/<owner>``
    # without preserving its source extension.  Keep the match owner-scoped
    # and require a path/name boundary so ``foo`` cannot match ``foobar``.
    for token in tokens:
        for stem in stems:
            if stem and re.search(
                rf"(?<![a-z0-9_]){re.escape(stem)}(?:\.(?:o|s)|/|$)",
                token,
            ):
                return True
    return False


def _looks_like_linker(line: str) -> bool:
    return bool(re.search(
        r"(?:^|[\s\"'=/:])(?:ld|mwld|link|elf2dol)(?:[.\s\\/]|$)",
        line.lower(),
    ))


def _assert_linker_provenance(command_text: str, *, root: Path, candidate: Path,
                              source: Path, owner: str,
                              link_target: Path | None = None,
                              linked_binary: Path | None = None) -> dict[str, Any]:
    if not command_text.strip():
        raise MeasurementError("Ninja linker response is empty")
    lines = [line.strip() for line in command_text.splitlines() if line.strip()]
    if not _command_mentions(candidate, command_text, root):
        raise MeasurementError("Ninja linker response does not name the selected source-built object")
    link_lines = [
        line for line in lines
        if _command_mentions(candidate, line, root)
        and (
            _looks_like_linker(line)
            or (link_target is not None and _command_mentions(link_target, line, root))
            or (linked_binary is not None and _command_mentions(linked_binary, line, root))
        )
    ]
    if not link_lines:
        raise MeasurementError(
            "Ninja linker response does not show the selected source-built object as a linker input"
        )
    aliases = _owner_fallback_aliases(
        root=root, candidate=candidate, source=source, owner=owner,
    )
    stems = {candidate.stem.lower(), source.stem.lower()}
    owner_tail = owner.replace("\\", "/").replace(":", "/").split("/")[-1]
    if owner_tail:
        stems.add(owner_tail.lower())
    fallback_lines = [
        line for line in link_lines
        if _fallback_input_matches_owner(line, aliases, stems)
    ]
    if fallback_lines:
        raise MeasurementError(
            f"linker provenance contains fallback asm/NonMatching input for {owner}: "
            f"{fallback_lines[0][:500]}"
        )
    source_compile_lines = [
        line for line in lines
        if _command_mentions(source, line, root) and not _looks_like_linker(line)
    ]
    if not source_compile_lines:
        raise MeasurementError(
            "Ninja linker response does not expose a bound source compilation command"
        )
    body = {
        "schema": "owner_campaign_ninja_input_manifest/v1",
        "owner": owner,
        "candidate_object_path": candidate.relative_to(root).as_posix(),
        "candidate_object_sha256": _sha_file(candidate),
        "source_path": source.relative_to(root).as_posix(),
        "source_sha256": _sha_file(source),
        "object_origin": "reconstructed_source",
        "commands": lines,
        "commands_sha256": _sha_bytes(command_text.encode("utf-8")),
        "fallback_asm_used": False,
        "nonmatching_fallback_linked": False,
    }
    return _seal(body, "manifest_sha256")


def _git_clean(root: Path, source: Path, deadline: Deadline) -> bool:
    with _bounded_bundle_runner(deadline):
        status = bundle._run(
            ["git", "status", "--porcelain", "--untracked-files=no", "--"],
            cwd=root, label="clean-build status",
        )
    allowed = source.relative_to(root).as_posix().lower()
    for line in status.splitlines():
        value = line[3:].strip().replace("\\", "/").lower() if len(line) >= 3 else line.strip().lower()
        if value and value != allowed:
            return False
    return True


def _same_file(left: Path, right: Path) -> bool:
    if left.stat().st_size != right.stat().st_size:
        return False
    return _sha_file(left) == _sha_file(right)


def _assert_bound_snapshot(*, source: Path, target: Path, candidate: Path,
                            expected_source_sha256: str,
                            expected_target_sha256: str,
                            expected_candidate_sha256: str,
                            label: str) -> tuple[str, str, str]:
    """Hash all bound inputs at one proof boundary and reject drift."""

    source_sha256 = _sha_file(source)
    target_sha256 = _sha_file(target)
    candidate_sha256 = _sha_file(candidate)
    if source_sha256 != expected_source_sha256:
        raise MeasurementError(f"source changed during {label}")
    if target_sha256 != expected_target_sha256:
        raise MeasurementError(f"target object changed during {label}")
    if candidate_sha256 != expected_candidate_sha256:
        raise MeasurementError(f"candidate object changed during {label}")
    return source_sha256, target_sha256, candidate_sha256


def _verify_function_set(*, root: Path, identity: Identity, target: Path, candidate: Path,
                         functions: Sequence[str], protected: Sequence[str],
                         objdiff: Path, readelf: Path, temp: Path,
                         deadline: Deadline) -> tuple[bool, bool, dict[str, Any], dict[str, Any]]:
    if not functions or len(set(functions)) != len(functions):
        raise MeasurementError("final_owner requires a nonempty unique verify-function census")
    if identity.function not in functions:
        raise MeasurementError("final_owner verify-function census omits the focus function")
    if any(not item or "\x00" in item for item in functions):
        raise MeasurementError("final_owner function census contains an invalid name")
    if not set(protected) <= set(functions):
        raise MeasurementError("final_owner protected function census is outside verify-function census")
    strict_path = temp / "owner-strict.json"
    data_path = temp / "owner-data.json"
    with _bounded_bundle_runner(deadline):
        bundle._run_objdiff(objdiff, target, candidate, strict_path, data=False, root=root)
        bundle._run_objdiff(objdiff, target, candidate, data_path, data=True, root=root)
    summaries: dict[str, Any] = {}
    physical_summaries: dict[str, Any] = {}
    exact_functions: set[str] = set()
    with _bounded_bundle_runner(deadline):
        for function in functions:
            physical_path = temp / f"physical-{len(physical_summaries)}.json"
            physical = bundle._physical_receipt(target, candidate, function, strict_path, readelf)
            bundle._atomic_json(physical_path, physical)
            try:
                focus = focus_symbol_report.build_from_paths(
                    strict_report_path=strict_path,
                    data_report_path=data_path,
                    function=function,
                    expected_strict_report_sha256=_sha_file(strict_path),
                    expected_data_report_sha256=_sha_file(data_path),
                    physical_receipt_path=physical_path,
                    expected_physical_receipt_sha256=_sha_file(physical_path),
                    require_physical=True,
                )
            except Exception as exc:
                raise MeasurementError(f"final_owner focus proof failed for {function}: {exc}") from exc
            strict = _metric(focus, "strict")
            data = _metric(focus, "data")
            physical_focus = focus["physical_relocations"]
            target_relocs = physical_focus["target"]["physical_relocation_count"]
            candidate_relocs = physical_focus["candidate"]["physical_relocation_count"]
            physical_differences = physical_focus["physical_relocation_differences"]
            exact = (
                strict["diff_rows"] == 0 and strict["target_size"] == strict["candidate_size"]
                and data["diff_rows"] == 0 and data["target_size"] == data["candidate_size"]
                and not physical_differences and target_relocs == candidate_relocs
            )
            if exact:
                exact_functions.add(function)
            summaries[function] = {
                "strict_bytes": [strict["target_size"], strict["candidate_size"]],
                "strict_differences": strict["diff_rows"],
                "data_bytes": [data["target_size"], data["candidate_size"]],
                "data_differences": data["diff_rows"],
                "physical_relocations": [target_relocs, candidate_relocs],
                "physical_differences": len(physical_differences),
                "exact": exact,
            }
            physical_summaries[function] = {
                "receipt_sha256": _sha_file(physical_path),
                "difference_sha256": _sha_json(physical_differences),
            }
    all_exact = len(exact_functions) == len(functions)
    protected_exact = all(item in exact_functions for item in protected)
    owner_proof = _seal({
        "schema": OWNER_PROOF_SCHEMA,
        "functions": list(functions),
        "protected_functions": list(protected),
        "summaries": summaries,
        "physical": physical_summaries,
        "all_exact": all_exact,
    }, "proof_sha256")
    sibling_proof = _seal({
        "schema": SIBLING_PROOF_SCHEMA,
        "protected_functions": list(protected),
        "exact_functions": sorted(set(protected) & exact_functions),
        "protected_exact": protected_exact,
    }, "proof_sha256")
    return protected_exact, all_exact, owner_proof, sibling_proof


def final_owner(*, root: Path, args: argparse.Namespace) -> dict[str, Any]:
    """Run and verify the actual final-owner link, without trusting manifests."""

    root = Path(os.path.abspath(root))
    _assert_no_indirection(root)
    if Path(os.path.abspath(Path.cwd())) != root:
        raise MeasurementError("adapter cwd must be the disposable root")
    identity = _identity(args, "final_owner")
    if identity.base_commit is None:
        raise MeasurementError("final_owner requires a bound base_commit")
    source = _source(args, root, identity)
    output_raw = _env_or_arg(args, "output", "OWNER_CAMPAIGN_MEASUREMENT_PATH")
    if not output_raw:
        raise MeasurementError("final_owner output path is required")
    output = _absolute(output_raw, root=root, label="final_owner output",
                       allow_external=False, exists=False)
    _safe_dir(output.parent, root=root, label="final_owner output directory")
    timeout_raw = _env_or_arg(args, "timeout")
    deadline = Deadline(float(timeout_raw) if timeout_raw else 1800.0)
    link_target_raw = _env_or_arg(args, "link_target", "OWNER_CAMPAIGN_LINK_TARGET")
    linked_raw = _env_or_arg(args, "linked_binary", "OWNER_CAMPAIGN_LINKED_BINARY")
    retail_raw = _env_or_arg(args, "retail_binary", "OWNER_CAMPAIGN_RETAIL_BINARY")
    checksum_raw = _env_or_arg(args, "checksum_config", "OWNER_CAMPAIGN_CHECKSUM_CONFIG")
    if not all((link_target_raw, linked_raw, retail_raw, checksum_raw)):
        raise MeasurementError(
            "final_owner requires link_target, linked_binary, retail_binary, and checksum_config"
        )
    link_target = _absolute(link_target_raw, root=root, label="Ninja link target",
                            allow_external=False, exists=False)
    linked = _absolute(linked_raw, root=root, label="linked binary",
                       allow_external=False, exists=False)
    retail = _absolute(retail_raw, root=root, label="retail binary",
                       allow_external=True, exists=True)
    checksum = _absolute(checksum_raw, root=root, label="DTK checksum config",
                         allow_external=True, exists=False)
    verify_functions = list(getattr(args, "verify_function", None) or [])
    env_functions = os.environ.get("OWNER_CAMPAIGN_VERIFY_FUNCTIONS")
    if not verify_functions and env_functions:
        verify_functions = [item for item in env_functions.split(",") if item]
    protected = list(getattr(args, "protected_function", None) or [])
    env_protected = os.environ.get("OWNER_CAMPAIGN_PROTECTED_FUNCTIONS")
    if not protected and env_protected:
        protected = [item for item in env_protected.split(",") if item]
    staged: Path | None = None
    temp: Path | None = None
    failure: BaseException | None = None
    result: dict[str, Any] | None = None
    try:
        loaded, objdiff, readelf, ninja, dtk = _toolchain(args, identity)
        with _bounded_bundle_runner(deadline):
            bundle._verify_objdiff(objdiff)
            bundle._verify_readelf(readelf)
            bundle._verify_ninja(ninja)
            staged = bundle._ensure_configured(root, loaded, ninja)
        if _sha_file(source) != identity.source_sha256:
            raise MeasurementError("source changed during final-owner configuration")
        target, candidate = _unit_objects(root, identity)
        # A final-owner proof must see the candidate object before link and
        # after link.  No caller-supplied source-link manifest participates.
        if not candidate.is_file():
            raise MeasurementError("source-built candidate object is absent before link")
        candidate_sha = _sha_file(candidate)
        if not _git_clean(root, source, deadline):
            raise MeasurementError("tracked source tree is not clean except for the bound source")
        with _bounded_bundle_runner(deadline):
            command_before = bundle._run(
                [str(ninja), "-t", "commands", str(link_target.relative_to(root))],
                cwd=root, label="Ninja linker input response",
            )
        linker_manifest = _assert_linker_provenance(
            command_before, root=root, candidate=candidate, source=source,
            owner=identity.owner, link_target=link_target, linked_binary=linked,
        )
        temp = Path(tempfile.mkdtemp(prefix=".owner-campaign-final-", dir=root / "build"))
        _assert_no_indirection(temp)
        with _bounded_bundle_runner(deadline):
            bundle._run(
                [str(ninja), "-j1", str(link_target.relative_to(root))],
                cwd=root, label="Ninja final-owner link",
            )
            command_after = bundle._run(
                [str(ninja), "-t", "commands", str(link_target.relative_to(root))],
                cwd=root, label="Ninja linker input response after link",
            )
        linker_manifest_after = _assert_linker_provenance(
            command_after, root=root, candidate=candidate, source=source,
            owner=identity.owner, link_target=link_target, linked_binary=linked,
        )
        if linker_manifest["commands"] != linker_manifest_after["commands"]:
            raise MeasurementError("Ninja linker input provenance changed during the link")
        linked = _absolute(linked, root=root, label="linked binary",
                           allow_external=False, exists=True)
        checksum = _absolute(checksum, root=root, label="DTK checksum config",
                             allow_external=True, exists=True)
        if not _same_file(Path(linked), Path(retail)):
            raise MeasurementError("linked binary is not byte-identical to the retail binary")
        dtk_output = _run_bounded(
            [str(dtk), "shasum", "-q", "-c", str(checksum)],
            cwd=root, deadline=deadline, label="DTK linked-binary checksum",
        )
        if _sha_file(candidate) != candidate_sha:
            raise MeasurementError("source-built candidate object changed during link proof")
        if _sha_file(source) != identity.source_sha256:
            raise MeasurementError("source changed during final-owner link proof")
        if not _git_clean(root, source, deadline):
            raise MeasurementError("tracked source tree changed during final-owner link proof")
        if not verify_functions:
            raise MeasurementError("final_owner requires an explicit full function census")
        protected = protected or [item for item in verify_functions if item != identity.function]
        protected_exact, full_exact, owner_proof, sibling_proof = _verify_function_set(
            root=root, identity=identity, target=target, candidate=candidate,
            functions=verify_functions, protected=protected, objdiff=objdiff,
            readelf=readelf, temp=temp, deadline=deadline,
        )
        if not full_exact or not protected_exact:
            raise MeasurementError("final_owner function/sibling objdiff proof is not exact")
        # This is the final source/target/object rehash immediately before
        # constructing and publishing the owner receipt.  The candidate hash
        # must remain the object that was present before linking; the target
        # and source hashes must still match the campaign identity.
        final_source_sha, final_target_sha, final_candidate_sha = _assert_bound_snapshot(
            source=source, target=target, candidate=candidate,
            expected_source_sha256=identity.source_sha256,
            expected_target_sha256=identity.target_object_sha256,
            expected_candidate_sha256=candidate_sha,
            label="final-owner receipt",
        )
        linked_binary_sha = _sha_file(Path(linked))
        linked_proof = _seal({
            "schema": LINKED_PROOF_SCHEMA,
            "linked_binary_sha256": linked_binary_sha,
            "linked_binary_size": Path(linked).stat().st_size,
            "retail_binary_sha256": _sha_file(Path(retail)),
            "retail_binary_size": Path(retail).stat().st_size,
            "byte_identical": True,
            "dtk_command_sha256": _sha_bytes("\0".join(
                [str(dtk), "shasum", "-q", "-c", str(checksum)]
            ).encode("utf-8")),
            "dtk_output_sha256": _sha_bytes(dtk_output.encode("utf-8")),
        }, "proof_sha256")
        source_link_proof = _seal({
            "schema": SOURCE_LINK_PROOF_SCHEMA,
            "campaign_id": identity.campaign_id,
            "manifest_sha256": identity.manifest_sha256,
            "owner": identity.owner,
            "unit": identity.unit,
            "function": identity.function,
            "base_commit": identity.base_commit,
            "source_sha256": identity.source_sha256,
            "candidate_object_path": candidate.relative_to(root).as_posix(),
            "candidate_object_sha256": final_candidate_sha,
            "object_origin": "reconstructed_source",
            "clean_build": True,
            "matching_source": True,
            "fallback_asm_used": False,
            "nonmatching_fallback_linked": False,
            "linker_input_manifest": linker_manifest,
            "linker_input_manifest_sha256": linker_manifest["manifest_sha256"],
            "commands_after_sha256": _sha_bytes(command_after.encode("utf-8")),
            "linked_binary_sha256": linked_binary_sha,
            "dtk_checksum_exact": True,
            "authority_advanced": False,
        }, "proof_sha256")
        body: dict[str, Any] = {
            "schema": FINAL_OWNER_SCHEMA,
            "campaign_id": identity.campaign_id,
            "manifest_sha256": identity.manifest_sha256,
            "owner": identity.owner,
            "unit": identity.unit,
            "source_path": source.relative_to(root).as_posix(),
            "base_commit": identity.base_commit,
            "source_sha256": identity.source_sha256,
            "target_object_sha256": identity.target_object_sha256,
            "toolchain_sha256": identity.toolchain_sha256,
            "source_link_exact": True,
            "protected_exact": True,
            "full_owner_exact": True,
            "linked_exact": True,
            "proof_receipts": {
                "source_link": source_link_proof["proof_sha256"],
                "siblings": sibling_proof["proof_sha256"],
                "full_owner": owner_proof["proof_sha256"],
                "linked": linked_proof["proof_sha256"],
                "dtk": linked_proof["dtk_output_sha256"],
            },
            "source_built_object_sha256": final_candidate_sha,
            "linked_binary_sha256": linked_binary_sha,
            "linker_input_manifest_sha256": linker_manifest["manifest_sha256"],
            "clean_build": True,
            "matching_source": True,
            "fallback_asm_used": False,
            "nonmatching_fallback_linked": False,
            "dtk_checksum_exact": True,
            "completed_at": dt.datetime.now(dt.timezone.utc).isoformat().replace("+00:00", "Z"),
        }
        result = {**body, "final_owner_sha256": _sha_json(body)}
        _atomic_json(output, result, limit=MAX_REPORT_COMPACT)
        return result
    except BaseException as exc:
        failure = exc
    finally:
        cleanup_errors: list[str] = []
        if staged is not None:
            try:
                bundle._remove_staged_retail(staged)
            except Exception as exc:
                cleanup_errors.append(f"retail cleanup: {exc}")
        if temp is not None:
            try:
                shutil.rmtree(temp)
            except Exception as exc:
                cleanup_errors.append(f"temporary proof cleanup: {exc}")
        if cleanup_errors and failure is None:
            failure = MeasurementError("; ".join(cleanup_errors))
        elif cleanup_errors and failure is not None:
            failure.add_note("cleanup incomplete: " + "; ".join(cleanup_errors))
        if failure is not None:
            try:
                output.unlink(missing_ok=True)
            except OSError:
                pass
    if failure is not None:
        raise failure
    if result is None:
        raise MeasurementError("final_owner ended without a receipt")
    return result


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--phase", choices=("snapshot", "candidate", "final_owner"))
    parser.add_argument("--root", default=".")
    parser.add_argument("--output")
    parser.add_argument("--source")
    parser.add_argument("--toolchain")
    parser.add_argument("--campaign-id")
    parser.add_argument("--manifest-sha256")
    parser.add_argument("--owner")
    parser.add_argument("--unit")
    parser.add_argument("--function")
    parser.add_argument("--source-sha256")
    parser.add_argument("--target-object-sha256")
    parser.add_argument("--toolchain-sha256")
    parser.add_argument("--base-commit")
    parser.add_argument("--timeout")
    parser.add_argument("--protected-total")
    parser.add_argument("--link-target")
    parser.add_argument("--linked-binary")
    parser.add_argument("--retail-binary")
    parser.add_argument("--checksum-config")
    parser.add_argument("--verify-function", action="append", default=[])
    parser.add_argument("--protected-function", action="append", default=[])
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        root = Path(os.path.abspath(args.root))
        phase = str(_env_or_arg(args, "phase") or "")
        if phase == "final_owner":
            final_owner(root=root, args=args)
        else:
            measure_current(root=root, args=args)
        return 0
    except Exception as exc:
        print(f"owner_campaign_measure: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
