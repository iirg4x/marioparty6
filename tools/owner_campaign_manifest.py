"""Create hash-bound owner-campaign manifests without manager-side staging.

This module is intentionally an initializer, not a second campaign runtime.
It accepts either a JSON draft or direct CLI fields, derives the three file
bindings from bytes, writes one canonical manifest atomically, and immediately
hands that file to ``owner_campaign.load_campaign`` for the authoritative
validation.  A final-owner command is never invented: owner closure must be
configured explicitly (usually in the draft) and is rejected when absent.
"""

from __future__ import annotations

import datetime as dt
import json
import os
from pathlib import Path
import re
import stat
import sys
import tempfile
from typing import Any, Mapping, Sequence

from . import owner_campaign


MANIFEST_INIT_SCHEMA = "owner_campaign_init/v1"
DEFAULT_MEASUREMENT_RELPATH = "build/owner-campaign/measurement.json"
DEFAULT_FORBIDDEN_CONSTRUCTS = [r"\b(?:asm|volatile|register)\b", r"#\s*pragma"]
DEFAULT_LIMITS = {
    "command_timeout_seconds": 1800,
    # A configured MP6 checkout is roughly 241 MiB before its first candidate
    # object.  Keep the reusable Ninja cache below a bounded soft watermark
    # instead of deleting it after every cell, while retaining a strict 512 MiB
    # transient ceiling per owner lane.
    "scratch_soft_bytes": 384 << 20,
    "scratch_hard_bytes": 512 << 20,
    "cell_temporary_bytes": 64 << 20,
    "focus_evidence_bytes": 256 << 10,
    "frontier_bytes": 64 << 10,
    "report_bytes": 64 << 10,
    "dedupe_bytes": 1 << 20,
    "owner_state_bytes": 16 << 20,
}


class ManifestError(owner_campaign.CampaignError):
    """A draft or direct initializer request is invalid."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _digest_json(value: Any) -> str:
    return owner_campaign._digest_bytes(_canonical(value))


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise ManifestError(f"{label} is required")
    return value.strip()


def _sha(value: Any, label: str) -> str:
    result = _text(value, label).lower()
    if owner_campaign.SHA_RE.fullmatch(result) is None:
        raise ManifestError(f"{label} must be a SHA-256")
    return result


def _safe_path(root: Path, raw: Any, label: str, *, exists: bool) -> Path:
    """Resolve an input path inside the repository without following links."""

    if isinstance(raw, Path):
        value = raw
    elif isinstance(raw, (str, os.PathLike)):
        value = Path(os.fspath(raw))
    else:
        raise ManifestError(f"{label} is invalid")
    if not str(value) or "\x00" in str(value):
        raise ManifestError(f"{label} is invalid")
    path = Path(os.path.abspath(value if value.is_absolute() else root / value))
    try:
        path.relative_to(root)
    except ValueError as exc:
        raise ManifestError(f"{label} escapes the repository: {raw}") from exc
    current = root
    for part in path.relative_to(root).parts:
        current = current / part
        if current.is_symlink():
            raise ManifestError(f"{label} uses symlink indirection: {current}")
    if exists and not path.is_file():
        raise ManifestError(f"{label} is not a file: {path}")
    return path


def _repo_rel(root: Path, path: Path, label: str) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError as exc:
        raise ManifestError(f"{label} escapes the repository") from exc


def _binding(
    root: Path,
    direct: Any,
    drafted: Any,
    label: str,
) -> dict[str, str]:
    """Resolve a file binding and verify a draft's claimed digest."""

    if direct is not None:
        path = _safe_path(root, direct, label, exists=True)
        return {"path": _repo_rel(root, path, label), "sha256": owner_campaign._digest_file(path)}
    if not isinstance(drafted, Mapping) or set(drafted) != {"path", "sha256"}:
        raise ManifestError(f"{label} binding must contain path and sha256")
    path = _safe_path(root, drafted["path"], label, exists=True)
    expected = _sha(drafted["sha256"], f"{label}.sha256")
    actual = owner_campaign._digest_file(path)
    if actual != expected:
        raise ManifestError(f"{label} hash drift: {actual} != {expected}")
    return {"path": _repo_rel(root, path, label), "sha256": actual}


def _external_regular_file(raw: Any, label: str) -> Path:
    """Resolve one absolute deployment input without trusting indirection.

    Campaign manifests remain repository-contained.  This helper is used only
    as the read boundary before an external tool is copied into campaign-local
    content-addressed storage.
    """

    if not isinstance(raw, (str, os.PathLike)):
        raise ManifestError(f"{label} path is invalid")
    candidate = Path(raw)
    if not candidate.is_absolute():
        raise ManifestError(f"{label} external path is not absolute")
    path = Path(os.path.abspath(candidate))
    current = Path(path.anchor)
    try:
        parts = path.relative_to(current).parts
    except ValueError as exc:
        raise ManifestError(f"{label} external path is invalid: {raw}") from exc
    for part in parts:
        current = current / part
        try:
            details = current.lstat()
        except OSError as exc:
            raise ManifestError(f"{label} is not a file: {path}") from exc
        if current.is_symlink() or getattr(details, "st_file_attributes", 0) & 0x400:
            raise ManifestError(f"{label} uses symlink/reparse indirection: {current}")
    if not _is_regular_file(path):
        raise ManifestError(f"{label} is not a regular file: {path}")
    return path


def _portable_tool_binding(
    root: Path,
    direct: Any,
    drafted: Any,
    label: str,
    *,
    filename: str,
) -> dict[str, str]:
    """Bind a contained tool or snapshot one absolute deployment input."""

    raw: Any
    expected: str | None
    if direct is not None:
        raw = direct
        expected = None
    else:
        if not isinstance(drafted, Mapping) or set(drafted) != {"path", "sha256"}:
            raise ManifestError(f"{label} binding must contain path and sha256")
        raw = drafted["path"]
        expected = _sha(drafted["sha256"], f"{label}.sha256")

    candidate = Path(raw) if isinstance(raw, (str, os.PathLike)) else Path()
    if not candidate.is_absolute():
        return _binding(root, direct, drafted, label)

    source = _external_regular_file(candidate, label)
    try:
        payload = source.read_bytes()
    except OSError as exc:
        raise ManifestError(f"{label} is unreadable: {source}") from exc
    actual = owner_campaign._digest_bytes(payload)
    if expected is not None and actual != expected:
        raise ManifestError(f"{label} hash drift: {actual} != {expected}")

    relative = (
        Path("build") / "owner-campaign" / "tool-cas" / actual / filename
    )
    cas = _safe_path(root, relative, f"{label} CAS", exists=False)
    if owner_campaign._path_has_indirection(root, cas):
        raise ManifestError(f"{label} CAS uses indirection")
    try:
        cas.lstat()
    except FileNotFoundError:
        pass
    except OSError as exc:
        raise ManifestError(f"{label} CAS is unreadable: {cas}") from exc
    else:
        if not _is_regular_file(cas):
            raise ManifestError(f"{label} CAS is not a regular file")
        if owner_campaign._digest_file(cas) != actual:
            raise ManifestError(f"{label} CAS hash drift")
        return {"path": _repo_rel(root, cas, label), "sha256": actual}

    try:
        cas.parent.mkdir(parents=True, exist_ok=True)
    except OSError as exc:
        raise ManifestError(f"{label} CAS directory is unavailable: {cas.parent}") from exc
    if owner_campaign._path_has_indirection(root, cas):
        raise ManifestError(f"{label} CAS uses indirection")
    fd, raw_temp = tempfile.mkstemp(prefix=f".{filename}.", dir=cas.parent)
    temporary = Path(raw_temp)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        try:
            os.link(temporary, cas)
        except FileExistsError:
            if (
                _is_regular_file(cas)
                and not owner_campaign._path_has_indirection(root, cas)
                and owner_campaign._digest_file(cas) == actual
            ):
                return {"path": _repo_rel(root, cas, label), "sha256": actual}
            raise ManifestError(f"{label} CAS hash drift")
        except OSError as exc:
            raise ManifestError(f"{label} CAS publication failed: {cas}") from exc
    finally:
        temporary.unlink(missing_ok=True)
    return {"path": _repo_rel(root, cas, label), "sha256": actual}


def _load_draft(root: Path, path: Path) -> dict[str, Any]:
    path = _safe_path(root, path, "campaign draft", exists=True)
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ManifestError(f"campaign draft is unreadable: {path}: {exc}") from exc
    if not isinstance(value, Mapping):
        raise ManifestError("campaign draft must be a JSON object")
    draft = dict(value)
    if draft.get("schema") not in {None, owner_campaign.CAMPAIGN_SCHEMA}:
        raise ManifestError("campaign draft schema is invalid")
    digest = draft.pop("manifest_sha256", None)
    if digest is not None:
        if _sha(digest, "draft manifest_sha256") != _digest_json(draft):
            raise ManifestError("draft manifest_sha256 is invalid")
    allowed = set(owner_campaign.MANIFEST_FIELDS) - {"manifest_sha256"}
    unknown = set(draft) - allowed
    if unknown:
        raise ManifestError("campaign draft has unknown fields: " + ", ".join(sorted(unknown)))
    return draft


def _as_list(value: Any, label: str, *, required: bool = False) -> list[Any] | None:
    if value is None:
        if required:
            raise ManifestError(f"{label} is required")
        return None
    if not isinstance(value, list):
        raise ManifestError(f"{label} must be a list")
    return list(value)


def _normal_source(root: Path, raw: Any) -> str:
    value = _text(raw, "source_relpath")
    path = _safe_path(root, value, "source", exists=True)
    return _repo_rel(root, path, "source")


def _normal_relpaths(root: Path, values: Sequence[Any], label: str) -> list[str]:
    result: list[str] = []
    for value in values:
        path = _safe_path(root, value, label, exists=False)
        result.append(_repo_rel(root, path, label))
    if len(set(result)) != len(result):
        raise ManifestError(f"{label} contains duplicates")
    return result


def _command(
    argv: Sequence[Any], label: str, *, measurement_relpath: Any = None
) -> dict[str, Any]:
    if not isinstance(argv, (list, tuple)) or not argv:
        raise ManifestError(f"{label} command is required")
    values = [_text(item, f"{label} argv item") for item in argv]
    relpath = measurement_relpath or DEFAULT_MEASUREMENT_RELPATH
    if not isinstance(relpath, str) or not relpath.startswith("build/") or not relpath.endswith(".json"):
        raise ManifestError(f"{label} measurement_relpath must be a build JSON path")
    if values.count("{MEASUREMENT_PRODUCER}") != 1:
        raise ManifestError(
            f"{label} command must contain {{MEASUREMENT_PRODUCER}} exactly once"
        )
    return {"argv": values, "measurement_relpath": relpath}


def _default_measurement_command() -> list[str]:
    return [
        sys.executable,
        "{MEASUREMENT_PRODUCER}",
        "--phase",
        "{PHASE}",
        "--root",
        "{SCRATCH_ROOT}",
        "--output",
        DEFAULT_MEASUREMENT_RELPATH,
        "--source",
        "{SOURCE}",
        "--toolchain",
        "{TOOLCHAIN}",
    ]


def _commands(
    draft: Mapping[str, Any],
    snapshot_command: Sequence[Any] | None,
    candidate_command: Sequence[Any] | None,
    final_owner_command: Sequence[Any] | None,
) -> dict[str, Any]:
    drafted = draft.get("commands")
    if drafted is not None and (
        not isinstance(drafted, Mapping)
        or not {"snapshot", "candidate", "final_owner"} <= set(drafted)
    ):
        raise ManifestError("draft commands must include snapshot, candidate, and final_owner")

    def selected(name: str, explicit: Sequence[Any] | None) -> Mapping[str, Any] | Sequence[Any] | None:
        if explicit:
            return explicit
        return drafted.get(name) if isinstance(drafted, Mapping) else None

    values: dict[str, Any] = {}
    for name, explicit in (("snapshot", snapshot_command), ("candidate", candidate_command)):
        chosen = selected(name, explicit)
        if chosen is None:
            chosen = _default_measurement_command()
        if isinstance(chosen, Mapping):
            values[name] = _command(
                chosen.get("argv"),
                name,
                measurement_relpath=chosen.get("measurement_relpath"),
            )
        else:
            values[name] = _command(chosen, name)

    chosen_final = selected("final_owner", final_owner_command)
    if chosen_final is None:
        raise ManifestError(
            "final_owner command is required; provide it in --draft or --final-owner-command"
        )
    if isinstance(chosen_final, Mapping):
        values["final_owner"] = _command(
            chosen_final.get("argv"),
            "final_owner",
            measurement_relpath=chosen_final.get("measurement_relpath"),
        )
    else:
        values["final_owner"] = _command(chosen_final, "final_owner")
    return values


def _atomic_json(path: Path, value: Mapping[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = _canonical(value) + b"\n"
    fd, raw = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temp = Path(raw)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temp, path)
    finally:
        temp.unlink(missing_ok=True)


def _is_regular_file(path: Path) -> bool:
    """Return whether *path* is a regular file without following links."""

    try:
        return stat.S_ISREG(path.lstat().st_mode)
    except (FileNotFoundError, OSError):
        return False


def _producer_cas_path(root: Path, expected_sha256: str) -> Path:
    relative = (
        Path("build") / "owner-campaign" / "tool-cas"
        / expected_sha256 / "owner_campaign_measure.py"
    )
    path = _safe_path(root, relative, "measurement producer CAS", exists=False)
    if owner_campaign._path_has_indirection(root, path):
        raise ManifestError("measurement producer CAS uses indirection")
    return path


def _snapshot_measurement_producer(
    root: Path, binding: Mapping[str, Any]
) -> Path:
    """Publish the bound producer bytes into content-addressed campaign CAS."""

    expected = _sha(binding.get("sha256"), "measurement producer.sha256")
    source = _safe_path(
        root, binding.get("path"), "measurement producer", exists=False
    )
    if owner_campaign._path_has_indirection(root, source):
        raise ManifestError("measurement producer uses indirection")
    cas = _producer_cas_path(root, expected)

    try:
        cas.lstat()
    except FileNotFoundError:
        pass
    except OSError as exc:
        raise ManifestError(f"measurement producer CAS is unreadable: {cas}") from exc
    else:
        if not _is_regular_file(cas):
            raise ManifestError("measurement producer CAS is not a regular file")
        if owner_campaign._digest_file(cas) != expected:
            raise ManifestError("measurement producer CAS hash drift")
        return cas

    if not _is_regular_file(source):
        raise ManifestError("measurement producer is not a regular file")
    try:
        payload = source.read_bytes()
    except OSError as exc:
        raise ManifestError(f"measurement producer is unreadable: {source}") from exc
    if owner_campaign._digest_bytes(payload) != expected:
        raise ManifestError("measurement producer hash drift before CAS snapshot")

    try:
        cas.parent.mkdir(parents=True, exist_ok=True)
    except OSError as exc:
        raise ManifestError(f"measurement producer CAS directory is unavailable: {cas.parent}") from exc
    if owner_campaign._path_has_indirection(root, cas):
        raise ManifestError("measurement producer CAS uses indirection")

    fd, raw = tempfile.mkstemp(
        prefix=".owner_campaign_measure.", dir=cas.parent
    )
    temporary = Path(raw)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        try:
            # Linking the fully-written temporary file creates the final CAS
            # path atomically without ever overwriting a concurrent object.
            os.link(temporary, cas)
        except FileExistsError:
            if (
                _is_regular_file(cas)
                and not owner_campaign._path_has_indirection(root, cas)
                and owner_campaign._digest_file(cas) == expected
            ):
                return cas
            raise ManifestError("measurement producer CAS hash drift")
        except OSError as exc:
            raise ManifestError(f"measurement producer CAS publication failed: {cas}") from exc
        return cas
    finally:
        temporary.unlink(missing_ok=True)


def _identity(root: Path, path: Path, campaign: Mapping[str, Any]) -> dict[str, Any]:
    loaded = owner_campaign.load_campaign(root, path)
    return {
        "schema": MANIFEST_INIT_SCHEMA,
        "status": "initialized",
        "campaign_id": loaded["campaign_id"],
        "owner": loaded["owner"],
        "unit": loaded["unit"],
        "manifest_path": path.relative_to(root).as_posix(),
        "manifest_sha256": loaded["manifest_sha256"],
        "authority_advanced": False,
    }


def initialize_campaign(
    root: Path,
    campaign: Path | str | None = None,
    output: Path | str | None = None,
    draft: Path | str | None = None,
    campaign_id: str | None = None,
    owner: str | None = None,
    unit: str | None = None,
    source_relpath: str | None = None,
    base_commit: str | None = None,
    target_object: Path | str | None = None,
    toolchain: Path | str | None = None,
    measurement_producer: Path | str | None = None,
    functions: Sequence[str] | None = None,
    protected_exact_functions: Sequence[str] | None = None,
    allowed_source_paths: Sequence[str] | None = None,
    allowed_build_paths: Sequence[str] | None = None,
    forbidden_constructs: Sequence[str] | None = None,
    snapshot_command: Sequence[str] | None = None,
    candidate_command: Sequence[str] | None = None,
    final_owner_command: Sequence[str] | None = None,
    cancellation_epoch: int | None = None,
    **_: Any,
) -> dict[str, Any]:
    """Create or idempotently validate one owner-campaign manifest."""

    root = Path(os.path.abspath(root))
    if not root.is_dir():
        raise ManifestError(f"campaign root does not exist: {root}")
    draft_value = _load_draft(root, Path(draft)) if draft is not None else {}

    selected_output = output if output is not None else campaign
    if selected_output is not None:
        output_path = _safe_path(root, selected_output, "campaign manifest output", exists=False)
    else:
        chosen_id = campaign_id or draft_value.get("campaign_id")
        if not isinstance(chosen_id, str) or not chosen_id:
            raise ManifestError("campaign output or campaign_id is required")
        output_path = _safe_path(
            root,
            Path("build") / "owner-campaign" / f"{chosen_id}.json",
            "campaign manifest output",
            exists=False,
        )

    if output_path.exists():
        if draft is not None or any(
            value is not None
            for value in (
                campaign_id, owner, unit, source_relpath, base_commit,
                target_object, toolchain, measurement_producer, functions,
                protected_exact_functions, allowed_source_paths,
                allowed_build_paths, forbidden_constructs, snapshot_command,
                candidate_command, final_owner_command, cancellation_epoch,
            )
        ):
            raise ManifestError(f"campaign manifest already exists: {output_path}")
        return _identity(root, output_path, {})

    def choose(name: str, direct: Any, *, required: bool = True) -> Any:
        value = direct if direct is not None else draft_value.get(name)
        if value is None and required:
            raise ManifestError(f"{name} is required")
        return value

    selected_campaign_id = _text(choose("campaign_id", campaign_id), "campaign_id")
    selected_owner = _text(choose("owner", owner), "owner")
    selected_unit = _text(choose("unit", unit), "unit")
    selected_source = _normal_source(root, choose("source_relpath", source_relpath))
    selected_base = _text(choose("base_commit", base_commit), "base_commit").lower()
    if owner_campaign.COMMIT_RE.fullmatch(selected_base) is None:
        raise ManifestError("base_commit must be a 40-character hexadecimal commit")

    source_allowed_raw = choose("allowed_source_paths", allowed_source_paths, required=False)
    if source_allowed_raw is None:
        source_allowed = [selected_source]
    else:
        source_allowed = _normal_relpaths(
            root, _as_list(source_allowed_raw, "allowed_source_paths", required=True) or [],
            "allowed source path",
        )
    build_allowed_raw = choose("allowed_build_paths", allowed_build_paths, required=False)
    if build_allowed_raw is None:
        build_allowed = ["build"]
    else:
        build_allowed = _normal_relpaths(
            root, _as_list(build_allowed_raw, "allowed_build_paths", required=True) or [],
            "allowed build path",
        )
    if selected_source not in source_allowed:
        raise ManifestError("source_relpath must be included in allowed_source_paths")

    selected_functions = _as_list(choose("functions", functions), "functions", required=True)
    selected_protected = _as_list(
        choose("protected_exact_functions", protected_exact_functions, required=False) or [],
        "protected_exact_functions",
    ) or []
    selected_forbidden = _as_list(
        choose("forbidden_constructs", forbidden_constructs, required=False)
        or DEFAULT_FORBIDDEN_CONSTRUCTS,
        "forbidden_constructs",
    ) or []
    for label, values in (
        ("functions", selected_functions),
        ("protected_exact_functions", selected_protected),
        ("forbidden_constructs", selected_forbidden),
    ):
        if not all(isinstance(item, str) and item.strip() for item in values):
            raise ManifestError(f"{label} contains an invalid value")

    target_binding = _binding(
        root, target_object, draft_value.get("target_object"), "target object"
    )
    toolchain_binding = _portable_tool_binding(
        root,
        toolchain,
        draft_value.get("toolchain"),
        "toolchain",
        filename="toolchain.json",
    )
    producer_binding = _portable_tool_binding(
        root,
        measurement_producer,
        draft_value.get("measurement_producer"),
        "measurement producer",
        filename="owner_campaign_measure.py",
    )
    commands = _commands(
        draft_value, snapshot_command, candidate_command, final_owner_command
    )
    epoch = cancellation_epoch
    if epoch is None:
        epoch = draft_value.get("cancellation_epoch", 0)
    if type(epoch) is not int or epoch < 0:
        raise ManifestError("cancellation_epoch must be a non-negative integer")
    limits = draft_value.get("limits", DEFAULT_LIMITS)
    if not isinstance(limits, Mapping):
        raise ManifestError("limits must be an object")
    limits_value = dict(limits)
    for key, default in DEFAULT_LIMITS.items():
        limits_value.setdefault(key, default)

    body: dict[str, Any] = {
        "schema": owner_campaign.CAMPAIGN_SCHEMA,
        "campaign_id": selected_campaign_id,
        "owner": selected_owner,
        "unit": selected_unit,
        "source_relpath": selected_source,
        "base_commit": selected_base,
        "target_object": target_binding,
        "toolchain": toolchain_binding,
        "measurement_producer": producer_binding,
        "functions": selected_functions,
        "protected_exact_functions": selected_protected,
        "allowed_source_paths": source_allowed,
        "allowed_build_paths": build_allowed,
        "forbidden_constructs": selected_forbidden,
        "commands": commands,
        "cancellation_epoch": epoch,
        "limits": limits_value,
    }
    manifest = {**body, "manifest_sha256": _digest_json(body)}
    _snapshot_measurement_producer(root, producer_binding)
    _atomic_json(output_path, manifest)
    try:
        return _identity(root, output_path, manifest)
    except BaseException:
        output_path.unlink(missing_ok=True)
        raise


__all__ = ["MANIFEST_INIT_SCHEMA", "ManifestError", "initialize_campaign"]
