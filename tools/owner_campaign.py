#!/usr/bin/env python3
"""Autonomous, owner-scoped cracking campaign runtime.

This is the v2 hot path.  A campaign manifest grants owner scope once; cells
are hash-bound, compiled in up to five reusable isolated scratch worktrees,
measured once, and atomically retained when they improve the monotonic
frontier.  No STOP file, manager key, approval, permit, or predicted-row packet
is consulted here.
"""

from __future__ import annotations

import argparse
import contextvars
from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager
import datetime as dt
import difflib
import hashlib
import json
import math
import os
from pathlib import Path
from queue import Empty, Queue
import re
import shutil
import signal
import stat
import subprocess
import sys
import tempfile
import threading
import time
from typing import Any, Mapping, Sequence


CAMPAIGN_SCHEMA = "owner_campaign/v1"
FRONTIER_SCHEMA = "crack_frontier/v2"
MEASUREMENT_SCHEMA = "owner_campaign_measurement/v1"
CANDIDATE_SCHEMA = "owner_campaign_candidate/v1"
PENDING_SCHEMA = "crack_frontier_pending/v2"
DEDUPE_SCHEMA = "owner_campaign_candidate_result/v1"
EXACT_MANIFEST_SCHEMA = "owner_campaign_exact_manifest/v1"
REPORT_SCHEMA = "CRACK_REPORT/v1"
SHA_RE = re.compile(r"[0-9a-f]{64}\Z")
COMMIT_RE = re.compile(r"[0-9a-f]{40}\Z")
TOKEN_RE = re.compile(r"[A-Za-z0-9_.-]{1,96}\Z")
UNIT_RE = re.compile(r"[A-Za-z0-9_.-]+(?:/[A-Za-z0-9_.-]+)+\Z")
MAX_OUTPUT = 1 << 20
DEFAULT_COMMAND_TIMEOUT_SECONDS = 1800.0
# Scratch repositories are deliberately reusable and are therefore not safe
# to garbage-collect merely because their campaign is idle.  Bound the
# aggregate of every campaign's scratch tree so abandoned campaigns fail
# closed before filling the volume; the per-campaign manifest limit remains
# the tighter limit for an active campaign.
GLOBAL_SCRATCH_HARD_BYTES = 5 * (512 << 20)
GC_MINIMUM_AGE_SECONDS = 60.0
MAX_TRACKED_CONTEXT_FILES = 32
MAX_TRACKED_CONTEXT_BYTES = 16 << 20

# ``run_candidate`` may call ``snapshot_frontier`` while already holding the
# worker lease.  A context variable keeps that fact local to the executing
# worker without changing the public snapshot function signature (which is
# also used by test and supervisor adapters).
_SCRATCH_LEASE_HELD = contextvars.ContextVar(
    "owner_campaign_scratch_lease_held", default=False
)


class CampaignError(RuntimeError):
    """Invalid campaign input or unsafe terminal state."""


class InfrastructureError(CampaignError):
    """Retryable command, scratch, or cleanup failure."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def _digest_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _digest_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _digest_json(value: Any) -> str:
    return _digest_bytes(_canonical(value))


def _now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat().replace("+00:00", "Z")


def _parse_time(value: Any, label: str) -> dt.datetime:
    if not isinstance(value, str):
        raise CampaignError(f"{label} is not a timestamp")
    try:
        parsed = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as exc:
        raise CampaignError(f"{label} is not a timestamp") from exc
    if parsed.tzinfo is None:
        raise CampaignError(f"{label} is not timezone-aware")
    return parsed.astimezone(dt.timezone.utc)


def _read_json(path: Path, label: str) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise CampaignError(f"{label} is unreadable: {path}: {exc}") from exc


def _atomic_bytes(path: Path, value: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, raw = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temp = Path(raw)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(value)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temp, path)
    finally:
        temp.unlink(missing_ok=True)


def _atomic_json(path: Path, value: Any, *, limit: int | None = None) -> None:
    payload = _canonical(value) + b"\n"
    if limit is not None and len(payload) > limit:
        raise CampaignError(f"compact artifact exceeds {limit} bytes: {path}")
    _atomic_bytes(path, payload)


def _inside(root: Path, path: Path) -> bool:
    try:
        path.relative_to(root)
        return True
    except ValueError:
        return False


def _bound_path(
    root: Path, raw: Any, label: str, *, exists: bool = True
) -> Path:
    if not isinstance(raw, str) or not raw or "\x00" in raw:
        raise CampaignError(f"{label} is invalid")
    candidate = Path(raw)
    if candidate.is_absolute():
        path = Path(os.path.abspath(candidate))
    else:
        path = Path(os.path.abspath(root / candidate))
    if not _inside(root, path):
        raise CampaignError(f"{label} escapes the campaign root: {raw}")
    current = root
    for part in path.relative_to(root).parts:
        current = current / part
        if current.exists() and current.is_symlink():
            raise CampaignError(f"{label} uses symlink indirection: {current}")
    if exists and not path.is_file():
        raise CampaignError(f"{label} is not a file: {path}")
    return path


def _sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or SHA_RE.fullmatch(value) is None:
        raise CampaignError(f"{label} is not a SHA-256")
    return value


def _closed_keys(value: Any, fields: set[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping) or set(value) != fields:
        raise CampaignError(f"{label} is not a strict closed object")
    return value


def _closed_keys_with_optional(
    value: Any, required: set[str], optional: set[str], label: str,
) -> Mapping[str, Any]:
    if (
        not isinstance(value, Mapping)
        or not required <= set(value)
        or not set(value) <= required | optional
    ):
        raise CampaignError(f"{label} is not a strict closed object")
    return value


def _command_timeout_seconds(campaign: Mapping[str, Any]) -> float:
    """Return the validated lock timeout, including for lightweight callers.

    Loaded manifests always carry a fully validated ``limits`` object.  A few
    supervisor/status API callers intentionally provide a minimal campaign
    mapping, however, and still need to inspect frontier state.  Locking is
    safe for those callers with the same bounded default used by manifests;
    malformed values remain an explicit error instead of silently becoming an
    unbounded wait.
    """

    limits = campaign.get("limits")
    if limits is None:
        return DEFAULT_COMMAND_TIMEOUT_SECONDS
    if not isinstance(limits, Mapping):
        raise CampaignError("campaign limits are invalid")
    value = limits.get(
        "command_timeout_seconds", DEFAULT_COMMAND_TIMEOUT_SECONDS
    )
    if isinstance(value, bool):
        raise CampaignError("campaign command timeout is invalid")
    try:
        timeout = float(value)
    except (TypeError, ValueError) as exc:
        raise CampaignError("campaign command timeout is invalid") from exc
    if not math.isfinite(timeout) or timeout <= 0:
        raise CampaignError("campaign command timeout is invalid")
    return timeout


MANIFEST_FIELDS = {
    "schema", "campaign_id", "owner", "unit", "source_relpath",
    "base_commit", "target_object", "toolchain", "measurement_producer", "functions",
    "protected_exact_functions", "allowed_source_paths", "allowed_build_paths",
    "forbidden_constructs", "commands", "cancellation_epoch", "limits",
    "manifest_sha256",
}
MANIFEST_OPTIONAL_FIELDS = {"tracked_context"}
TRACKED_CONTEXT_FIELDS = {"path", "sha256", "size", "executable"}
LIMIT_FIELDS = {
    "command_timeout_seconds", "scratch_soft_bytes", "scratch_hard_bytes",
    "cell_temporary_bytes", "focus_evidence_bytes", "frontier_bytes",
    "report_bytes", "dedupe_bytes",
    "owner_state_bytes",
}
COMMAND_FIELDS = {"argv", "measurement_relpath"}


def _unique_git_executables(candidates: Sequence[Path]) -> list[Path]:
    unique: list[Path] = []
    seen: set[str] = set()
    for raw in candidates:
        try:
            candidate = Path(raw).resolve(strict=True)
        except (OSError, RuntimeError):
            continue
        key = os.path.normcase(str(candidate))
        if key in seen or not candidate.is_file():
            continue
        seen.add(key)
        unique.append(candidate)
    return unique


def _git_executable_rank(path: Path, *, windows: bool) -> tuple[int, int]:
    lowered = str(path).replace("/", "\\").lower()
    msys = "\\msys" in lowered or "\\devkitpro\\" in lowered
    git_for_windows = "\\git\\cmd\\git.exe" in lowered or "\\git\\bin\\git.exe" in lowered
    return (0 if git_for_windows and not msys else 2 if msys else 1, len(lowered))


def _select_git_executable(candidates: Sequence[Path], *, windows: bool) -> Path:
    """Choose the preferred Git path without probing a repository."""

    unique = _unique_git_executables(candidates)
    if not unique:
        raise CampaignError("native Git executable cannot be resolved")
    if not windows:
        return unique[0]

    return min(unique, key=lambda path: _git_executable_rank(path, windows=True))


def _resolve_git_executable(repository_root: Path | None = None) -> tuple[Path, str]:
    """Resolve Git and verify that it can read the campaign repository.

    Windows installations commonly expose a native Git before the MSYS Git
    used by the project. ``--version`` only proves that an executable can be
    launched; it does not prove that it understands the repository's ``.git``
    representation. Probe each candidate in preference order and retain the
    first one that can perform a repository ``rev-parse`` in the actual
    campaign root.
    """

    candidates: list[Path] = []
    if os.name == "nt":
        for variable in ("ProgramW6432", "ProgramFiles", "LOCALAPPDATA"):
            install_base = os.environ.get(variable)
            if not install_base:
                continue
            install_root = Path(install_base)
            if variable == "LOCALAPPDATA":
                install_root = install_root / "Programs"
            candidates.extend(
                (
                    install_root / "Git" / "cmd" / "git.exe",
                    install_root / "Git" / "bin" / "git.exe",
                )
            )
        for variable in ("DEVKITPRO", "MSYS2_HOME", "MSYS2_ROOT"):
            install_base = os.environ.get(variable)
            if not install_base:
                continue
            install_root = Path(install_base)
            candidates.extend(
                (
                    install_root / "msys2" / "usr" / "bin" / "git.exe",
                    install_root / "usr" / "bin" / "git.exe",
                )
            )
        for item in os.environ.get("PATH", "").split(os.pathsep):
            if item:
                candidates.append(Path(item) / "git.exe")
    else:
        found = shutil.which("git")
        if found:
            candidates.append(Path(found))
    unique = _unique_git_executables(candidates)
    if not unique:
        raise CampaignError("native Git executable cannot be resolved")
    ordered = (
        sorted(unique, key=lambda path: _git_executable_rank(path, windows=True))
        if os.name == "nt"
        else unique
    )
    repository = Path(os.path.abspath(repository_root)) if repository_root is not None else None
    version_ok = False
    repository_failures: list[str] = []
    for candidate in ordered:
        try:
            version = subprocess.run(
                [str(candidate), "--version"],
                capture_output=True,
                text=True,
                check=False,
            )
        except OSError as exc:
            repository_failures.append(f"{candidate}: {exc}")
            continue
        if version.returncode or not version.stdout.startswith("git version "):
            continue
        version_ok = True
        if repository is not None:
            try:
                repository_probe = subprocess.run(
                    [str(candidate), "rev-parse", "--git-dir"],
                    cwd=repository,
                    capture_output=True,
                    text=True,
                    check=False,
                )
            except OSError as exc:
                repository_failures.append(f"{candidate}: {exc}")
                continue
            if repository_probe.returncode or not repository_probe.stdout.strip():
                detail = (
                    repository_probe.stderr.strip()
                    or repository_probe.stdout.strip()
                    or str(repository_probe.returncode)
                )[:200]
                repository_failures.append(f"{candidate}: {detail}")
                continue
        return candidate, _digest_file(candidate)
    if not version_ok:
        raise CampaignError("resolved Git executable failed identity probe")
    if repository is not None:
        raise CampaignError(
            "no resolved Git executable can read campaign repository"
            + (f": {'; '.join(repository_failures)}" if repository_failures else "")
        )
    raise CampaignError("resolved Git executable failed identity probe")


def _git_argv(campaign: Mapping[str, Any], *arguments: str) -> list[str]:
    # Source/object evidence is byte-bound.  Never allow a user's global or
    # repository autocrlf setting to rewrite the detached campaign checkout.
    return [
        str(campaign["_git_executable"]), "-c", "core.autocrlf=false", *arguments
    ]


def _tracked_worktree_changes(
    root: Path, git_executable: Path,
) -> dict[str, str]:
    """Return tracked regular-file changes without pathname quoting ambiguity."""

    status = subprocess.run(
        [
            str(git_executable), "-c", "core.quotepath=false", "status",
            "--porcelain=v1", "-z", "--untracked-files=no",
        ],
        cwd=root, capture_output=True, check=False,
    )
    if status.returncode:
        raise CampaignError("campaign repository cleanliness cannot be verified")
    entries = status.stdout.split(b"\0")
    if entries and entries[-1] == b"":
        entries.pop()
    changes: dict[str, str] = {}
    for entry in entries:
        if len(entry) < 4 or entry[2:3] != b" ":
            raise CampaignError("campaign repository status is malformed")
        try:
            state = entry[:2].decode("ascii")
            path_text = os.fsdecode(entry[3:]).replace("\\", "/")
        except (UnicodeDecodeError, ValueError) as exc:
            raise CampaignError("campaign repository status is malformed") from exc
        if not path_text or path_text in changes:
            raise CampaignError("campaign repository status is ambiguous")
        # Deletions, renames, copies, conflicts, and type changes cannot be
        # materialized as a byte-stable regular-file overlay.
        if any(marker not in {" ", "M", "A"} for marker in state):
            raise CampaignError(
                f"campaign tracked context has unsupported status {state}: {path_text}"
            )
        changes[path_text] = state
    return changes


def _tracked_context_cas_path(root: Path, sha256: str) -> Path:
    return _bound_path(
        root,
        (Path("build") / "owner-campaign" / "context-cas" / sha256 / "payload").as_posix(),
        "tracked context CAS",
        exists=False,
    )


def _load_tracked_context(
    root: Path, raw: Any,
) -> list[dict[str, Any]]:
    if raw is None:
        return []
    if not isinstance(raw, list) or len(raw) > MAX_TRACKED_CONTEXT_FILES:
        raise CampaignError("campaign tracked_context is invalid")
    result: list[dict[str, Any]] = []
    seen: set[str] = set()
    total = 0
    for index, item in enumerate(raw):
        descriptor = _closed_keys(
            item, TRACKED_CONTEXT_FIELDS, f"tracked_context[{index}]"
        )
        path_text = descriptor["path"]
        if (
            not isinstance(path_text, str)
            or not path_text
            or Path(path_text).is_absolute()
            or path_text == ".git"
            or path_text.startswith(".git/")
        ):
            raise CampaignError("campaign tracked context path is invalid")
        bound = _bound_path(root, path_text, "tracked context path", exists=False)
        canonical = bound.relative_to(root).as_posix()
        if canonical != path_text or canonical in seen:
            raise CampaignError("campaign tracked context path is ambiguous")
        seen.add(canonical)
        expected = _sha(descriptor["sha256"], "tracked context sha256")
        size = descriptor["size"]
        executable = descriptor["executable"]
        if type(size) is not int or size < 0 or type(executable) is not bool:
            raise CampaignError("campaign tracked context descriptor is invalid")
        total += size
        if total > MAX_TRACKED_CONTEXT_BYTES:
            raise CampaignError("campaign tracked context exceeds compact byte limit")
        cas = _tracked_context_cas_path(root, expected)
        if (
            _path_has_indirection(root, cas)
            or not _is_regular_file(cas)
            or cas.stat().st_size != size
            or _digest_file(cas) != expected
        ):
            raise CampaignError("campaign tracked context CAS hash drift")
        result.append({
            "path": canonical,
            "sha256": expected,
            "size": size,
            "executable": executable,
            "_cas": cas,
        })
    if [item["path"] for item in result] != sorted(seen):
        raise CampaignError("campaign tracked_context is not canonically ordered")
    return result


def _load_campaign(
    root: Path,
    path: Path,
    *,
    allow_unbound_live_source: bool = False,
) -> dict[str, Any]:
    if type(allow_unbound_live_source) is not bool:
        raise CampaignError("campaign source-binding mode is invalid")
    root = Path(os.path.abspath(root))
    path = _bound_path(root, str(path), "campaign manifest")
    raw = _closed_keys_with_optional(
        _read_json(path, "campaign manifest"),
        MANIFEST_FIELDS,
        MANIFEST_OPTIONAL_FIELDS,
        "campaign manifest",
    )
    body = dict(raw)
    manifest_sha = _sha(body.pop("manifest_sha256", None), "manifest_sha256")
    if _digest_json(body) != manifest_sha:
        raise CampaignError("campaign manifest digest is invalid")
    if raw["schema"] != CAMPAIGN_SCHEMA:
        raise CampaignError("campaign manifest schema is invalid")
    if not isinstance(raw["campaign_id"], str) or TOKEN_RE.fullmatch(raw["campaign_id"]) is None:
        raise CampaignError("campaign_id is invalid")
    if not isinstance(raw["owner"], str) or not raw["owner"]:
        raise CampaignError("campaign owner is invalid")
    if not isinstance(raw["unit"], str) or UNIT_RE.fullmatch(raw["unit"]) is None:
        raise CampaignError("campaign unit is invalid")
    if not isinstance(raw["base_commit"], str) or COMMIT_RE.fullmatch(raw["base_commit"]) is None:
        raise CampaignError("campaign base_commit is invalid")
    if Path(str(raw["source_relpath"])).is_absolute():
        raise CampaignError("campaign source_relpath must be repository-relative")
    source = _bound_path(root, raw["source_relpath"], "campaign source")
    allowed_sources = raw["allowed_source_paths"]
    if not isinstance(allowed_sources, list) or raw["source_relpath"] not in allowed_sources:
        raise CampaignError("campaign source is outside allowed_source_paths")
    for item in allowed_sources:
        _bound_path(root, item, "allowed source", exists=False)
    builds = raw["allowed_build_paths"]
    if not isinstance(builds, list) or not builds:
        raise CampaignError("campaign allowed_build_paths is invalid")
    for item in builds:
        _bound_path(root, item, "allowed build path", exists=False)
    functions = raw["functions"]
    protected = raw["protected_exact_functions"]
    if (
        not isinstance(functions, list) or not functions
        or len(set(functions)) != len(functions)
        or not all(isinstance(item, str) and item for item in functions)
        or not isinstance(protected, list) or len(set(protected)) != len(protected)
        or not set(protected) <= set(functions)
    ):
        raise CampaignError("campaign function inventory is invalid")
    producer: Path | None = None
    for label in ("target_object", "toolchain", "measurement_producer"):
        binding = _closed_keys(raw[label], {"path", "sha256"}, label)
        expected_sha256 = _sha(binding["sha256"], f"{label}.sha256")
        if label == "measurement_producer":
            # The configured path is mutable deployment state.  A producer
            # update must not block an existing campaign, but it may only be
            # replaced by an immutable, exact-hash snapshot from campaign
            # state.  Keep the configured path itself contained and free of
            # indirection even when it has been removed or changed.
            bound = _bound_path(
                root, binding["path"], label, exists=False
            )
            if _is_regular_file(bound) and _digest_file(bound) == expected_sha256:
                producer = bound
            else:
                producer = _resolve_measurement_producer_cas(root, expected_sha256)
            continue
        bound = _bound_path(root, binding["path"], label)
        if _digest_file(bound) != expected_sha256:
            raise CampaignError(f"{label} hash drift")
    commands = raw["commands"]
    if not isinstance(commands, Mapping) or set(commands) != {"snapshot", "candidate", "final_owner"}:
        raise CampaignError("campaign commands are invalid")
    for name, descriptor in commands.items():
        descriptor = _closed_keys(descriptor, COMMAND_FIELDS, f"{name} command")
        argv = descriptor["argv"]
        if not isinstance(argv, list) or not argv or not all(isinstance(x, str) and x for x in argv):
            raise CampaignError(f"{name} command argv is invalid")
        if (
            argv.count("{MEASUREMENT_PRODUCER}") != 1
            or argv.index("{MEASUREMENT_PRODUCER}") not in {0, 1}
        ):
            raise CampaignError(
                f"{name} command must execute the bound measurement producer exactly once"
            )
        measurement = _bound_path(root, descriptor["measurement_relpath"], f"{name} measurement", exists=False)
        if not any(_inside(_bound_path(root, p, "allowed build", exists=False), measurement) for p in builds):
            raise CampaignError(f"{name} measurement is outside allowed build paths")
    limits = _closed_keys(raw["limits"], LIMIT_FIELDS, "campaign limits")
    maxima = {
        "command_timeout_seconds": 1800, "scratch_soft_bytes": 384 << 20,
        "scratch_hard_bytes": 512 << 20, "cell_temporary_bytes": 64 << 20,
        "focus_evidence_bytes": 256 << 10,
        "frontier_bytes": 64 << 10, "report_bytes": 64 << 10,
        "dedupe_bytes": 1 << 20, "owner_state_bytes": 16 << 20,
    }
    for key, maximum in maxima.items():
        value = limits[key]
        if type(value) is not int or value <= 0 or value > maximum:
            raise CampaignError(f"campaign limit {key} is invalid")
    if limits["scratch_soft_bytes"] > limits["scratch_hard_bytes"]:
        raise CampaignError("scratch soft limit exceeds hard limit")
    constructs = raw["forbidden_constructs"]
    if not isinstance(constructs, list) or not all(isinstance(x, str) and x for x in constructs):
        raise CampaignError("forbidden_constructs is invalid")
    if type(raw["cancellation_epoch"]) is not int or raw["cancellation_epoch"] < 0:
        raise CampaignError("cancellation_epoch is invalid")
    git_executable, git_sha256 = _resolve_git_executable(root)
    tracked_context = _load_tracked_context(root, raw.get("tracked_context"))
    check = subprocess.run(
        [str(git_executable), "cat-file", "-t", raw["base_commit"]],
        cwd=root, capture_output=True, text=True, check=False,
    )
    if check.returncode or check.stdout.strip() != "commit":
        raise CampaignError(
            "campaign base_commit does not resolve: "
            + (check.stderr.strip() or check.stdout.strip() or str(check.returncode))[:500]
        )
    head = subprocess.run(
        [str(git_executable), "rev-parse", "HEAD"], cwd=root, capture_output=True,
        text=True, check=False,
    )
    if head.returncode or not head.stdout.strip():
        raise CampaignError(
            "campaign repository HEAD cannot be resolved: "
            + (head.stderr.strip() or head.stdout.strip() or str(head.returncode))[:500]
        )
    head_commit = head.stdout.strip()
    ancestry = subprocess.run(
        [str(git_executable), "merge-base", "--is-ancestor", raw["base_commit"], head_commit],
        cwd=root, capture_output=True, text=True, check=False,
    )
    if ancestry.returncode:
        if ancestry.returncode == 1:
            raise CampaignError(
                "campaign base_commit is not an ancestor of the repository HEAD"
            )
        raise CampaignError(
            "campaign base_commit ancestry cannot be verified: "
            + (ancestry.stderr.strip() or ancestry.stdout.strip() or str(ancestry.returncode))[:500]
        )
    blob = subprocess.run(
        [str(git_executable), "show", f"{raw['base_commit']}:{raw['source_relpath']}"],
        cwd=root, capture_output=True, check=False,
    )
    if blob.returncode:
        raise CampaignError("campaign source does not exist in base_commit")
    base_source_sha256 = _digest_bytes(blob.stdout)
    changed = set(_tracked_worktree_changes(root, git_executable))
    context_by_path = {item["path"]: item for item in tracked_context}
    # The owner source has its own retained-frontier/initial-snapshot binding
    # below.  Preserve that more precise error and compatibility path while
    # requiring every other tracked write to be immutable context.
    unapproved = changed - set(context_by_path) - {raw["source_relpath"]}
    if unapproved:
        raise CampaignError("campaign repository has unapproved tracked writes")
    live_source_sha256 = _digest_file(source)
    initial_source = context_by_path.get(raw["source_relpath"])
    initial_source_bound = (
        initial_source is not None
        and initial_source["sha256"] == live_source_sha256
    )
    source_requires_binding = (
        raw["source_relpath"] in changed
        or live_source_sha256 != base_source_sha256
    )
    bound_live = False
    if source_requires_binding:
        owner_state = _state_root(root) / "owners" / _slug(str(raw["owner"]))
        for state_path in [
            *owner_state.rglob("latest-frontier.json"),
            *owner_state.rglob("frontier.pending.json"),
        ]:
            try:
                state = _read_json(state_path, "retained source binding")
                if state_path.name == "frontier.pending.json":
                    state = state.get("frontier", {})
                frontier_body = dict(state) if isinstance(state, Mapping) else {}
                frontier_digest = frontier_body.pop("frontier_sha256", None)
                if (
                    isinstance(state, Mapping)
                    and SHA_RE.fullmatch(str(frontier_digest)) is not None
                    and _digest_json(frontier_body) == frontier_digest
                    and state.get("campaign_id") == raw["campaign_id"]
                    and state.get("manifest_sha256") == raw["manifest_sha256"]
                    and state.get("source_sha256") == live_source_sha256
                ):
                    bound_live = True
                    break
            except CampaignError:
                continue
    if raw["source_relpath"] in changed:
        if (
            not initial_source_bound
            and not bound_live
            and not allow_unbound_live_source
        ):
            raise CampaignError(
                "campaign source write is not bound to a retained frontier"
            )
    elif (
        live_source_sha256 != base_source_sha256
        and not initial_source_bound
        and not bound_live
        and not allow_unbound_live_source
    ):
        raise CampaignError("clean campaign source does not match the base blob")
    result = dict(raw)
    result["_root"] = root
    result["_path"] = path
    result["_source"] = source
    result["_target"] = _bound_path(root, raw["target_object"]["path"], "target object")
    result["_target_size"] = result["_target"].stat().st_size
    result["_toolchain"] = _bound_path(root, raw["toolchain"]["path"], "toolchain")
    if producer is None:
        raise CampaignError("measurement producer could not be resolved")
    result["_producer"] = producer
    result["_base_source_sha256"] = base_source_sha256
    result["_tracked_context"] = tracked_context
    result["_git_executable"] = git_executable
    result["_git_sha256"] = git_sha256
    result["_live_source_sha256"] = live_source_sha256
    return result


def load_campaign(root: Path, path: Path) -> dict[str, Any]:
    """Load a campaign for any operation that can measure or mutate state.

    This public loader deliberately retains the strict live-source binding.
    Read-only inspection of an older retained frontier uses the separate
    ``load_retained_frontier_campaign`` entry point below.
    """

    return _load_campaign(root, path)


def load_retained_frontier_campaign(
    root: Path,
    path: Path,
    function: str,
) -> dict[str, Any]:
    """Bind a read-only campaign view to its canonical retained frontier.

    A lane may have a newer live source while an earlier function frontier is
    still the evidence to triage.  This loader permits that source drift only
    after normal manifest/repository/path/toolchain validation, and binds the
    exact canonical ``latest-frontier.json`` for one function.  No snapshot,
    proposal, compile, or retention path calls this loader.
    """

    campaign = _load_campaign(
        root,
        path,
        allow_unbound_live_source=True,
    )
    if not isinstance(function, str) or not function:
        raise CampaignError("retained frontier function is invalid")
    if function not in campaign["functions"]:
        raise CampaignError(f"function is outside campaign scope: {function}")
    frontier = _read_latest_frontier(Path(os.path.abspath(root)), campaign, function)
    if frontier is None:
        raise CampaignError(f"current frontier is unavailable for {function}")
    campaign["_retained_frontier"] = dict(frontier)
    campaign["_retained_frontier_sha256"] = frontier["frontier_sha256"]
    campaign["_retained_frontier_function"] = function
    campaign["_retained_frontier_read_only"] = True
    return campaign


def _slug(value: str) -> str:
    readable = re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_")[:72] or "owner"
    return f"{readable}-{_digest_bytes(value.encode('utf-8'))[:12]}"


def _state_root(root: Path) -> Path:
    return root / "build" / "owner-campaign"


def _is_regular_file(path: Path) -> bool:
    """Return whether *path* is a regular file without following links."""

    try:
        return stat.S_ISREG(path.lstat().st_mode)
    except (FileNotFoundError, OSError):
        return False


def _resolve_measurement_producer_cas(root: Path, expected_sha256: str) -> Path:
    """Resolve an exact immutable producer snapshot from campaign-local CAS.

    The manifest continues to name the configured deployment path and its
    expected digest.  If that path drifts, only the canonical producer
    filename below the digest directory is eligible as a replacement.  The
    final path is checked both lexically and with the existing filesystem
    indirection guard before its bytes are hashed.
    """

    state_root = _state_root(root)
    relative = Path("tool-cas") / expected_sha256 / "owner_campaign_measure.py"
    try:
        snapshot = _bound_path(
            state_root, relative.as_posix(),
            "measurement producer CAS", exists=True,
        )
    except CampaignError as exc:
        raise CampaignError(
            "measurement producer hash drift; exact CAS snapshot is unavailable"
        ) from exc
    if _path_has_indirection(root, snapshot) or not _is_regular_file(snapshot):
        raise CampaignError(
            "measurement producer hash drift; CAS snapshot is not a regular file"
        )
    if _digest_file(snapshot) != expected_sha256:
        raise CampaignError(
            "measurement producer hash drift; CAS snapshot hash drift"
        )
    return snapshot


def _owner_root(root: Path, campaign: Mapping[str, Any]) -> Path:
    return _state_root(root) / "owners" / _slug(str(campaign["owner"]))


def _function_root(root: Path, campaign: Mapping[str, Any], function: str) -> Path:
    return _owner_root(root, campaign) / _slug(function)


def _scratch_repo(root: Path, campaign: Mapping[str, Any], worker: int = 0) -> Path:
    if type(worker) is not int or not 0 <= worker < 5:
        raise CampaignError("campaign worker index is outside 0..4")
    return (
        _state_root(root) / "scratch" / _slug(str(campaign["campaign_id"]))
        / f"repo-{worker}"
    )


def _scratch_identity(campaign: Mapping[str, Any], scratch: Path) -> dict[str, Any]:
    body = {
        "schema": "owner_campaign_scratch/v1",
        "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"],
        "base_commit": campaign["base_commit"],
        "scratch_path": str(Path(os.path.abspath(scratch))),
        "git_sha256": campaign["_git_sha256"],
    }
    return {**body, "scratch_identity_sha256": _digest_json(body)}


def _path_has_indirection(root: Path, path: Path) -> bool:
    root = Path(os.path.abspath(root))
    path = Path(os.path.abspath(path))
    if not _inside(root, path):
        return True
    current = root
    for part in path.relative_to(root).parts:
        current = current / part
        try:
            details = current.lstat()
        except FileNotFoundError:
            continue
        if current.is_symlink() or getattr(details, "st_file_attributes", 0) & 0x400:
            return True
    return False


def _scratch_is_owned(campaign: Mapping[str, Any], scratch: Path) -> bool:
    if _path_has_indirection(_state_root(Path(campaign["_root"])), scratch):
        return False
    marker = scratch / ".owner-campaign-identity.json"
    try:
        value = _read_json(marker, "scratch identity")
    except CampaignError:
        return False
    if not isinstance(value, Mapping):
        return False
    return dict(value) == _scratch_identity(campaign, scratch)


def _read_owned_scratch_identity(scratch: Path) -> dict[str, str] | None:
    """Read a self-bound scratch marker without trusting its path."""

    marker = scratch / ".owner-campaign-identity.json"
    try:
        if _path_has_indirection(scratch, marker):
            return None
        value = _read_json(marker, "scratch identity")
        if not isinstance(value, Mapping):
            return None
        fields = {
            "schema", "campaign_id", "manifest_sha256", "base_commit",
            "scratch_path", "git_sha256", "scratch_identity_sha256",
        }
        if set(value) != fields or value["schema"] != "owner_campaign_scratch/v1":
            return None
        body = dict(value)
        digest = body.pop("scratch_identity_sha256", None)
        if not isinstance(digest, str) or digest != _digest_json(body):
            return None
        for field in ("campaign_id", "scratch_path"):
            if not isinstance(value[field], str) or not value[field]:
                return None
        if (
            COMMIT_RE.fullmatch(str(value["base_commit"])) is None
            or SHA_RE.fullmatch(str(value["manifest_sha256"])) is None
            or SHA_RE.fullmatch(str(value["git_sha256"])) is None
        ):
            return None
        if str(value["scratch_path"]) != str(Path(os.path.abspath(scratch))):
            return None
        if _path_has_indirection(scratch, scratch):
            return None
        return {field: str(value[field]) for field in fields - {"scratch_identity_sha256"}}
    except (CampaignError, OSError, TypeError):
        return None


def _scratch_has_active_state(root: Path) -> bool:
    """Return true when any campaign has state that makes scratch deletion unsafe."""

    owners = _state_root(root) / "owners"
    if not owners.is_dir() or _path_has_indirection(root, owners):
        return True if owners.exists() else False
    try:
        for ledger in owners.rglob("candidate-results.jsonl"):
            if _path_has_indirection(root, ledger):
                return True
            if any(
                record["status"] == "inflight"
                for record in _dedupe_records(ledger)
            ):
                return True
        for pending in owners.rglob("frontier.pending.json"):
            if _path_has_indirection(root, pending):
                return True
            # A pending frontier is a live source-publication transaction even
            # if its JSON is temporarily unreadable.  Keep all old scratch
            # until the transaction is reconciled.
            return True
    except (CampaignError, OSError):
        return True
    return False


def _registered_scratch_worktree(
    root: Path, campaign: Mapping[str, Any], scratch: Path,
) -> bool | None:
    """Return Git registration state, or None when it cannot be proven."""

    try:
        result = subprocess.run(
            _git_argv(campaign, "worktree", "list", "--porcelain"),
            cwd=root, capture_output=True, text=True, check=False,
            timeout=_command_timeout_seconds(campaign),
        )
    except (OSError, subprocess.TimeoutExpired):
        return None
    if result.returncode:
        return None
    wanted = os.path.normcase(os.path.abspath(str(scratch)))
    target_tail = "/".join(
        part.lower() for part in scratch.relative_to(root).parts
    ).rstrip("/")
    registered: list[str] = []
    for line in result.stdout.splitlines():
        if line.startswith("worktree "):
            value = line[len("worktree "):].strip()
            if value.startswith('"') or value.endswith('"'):
                return None
            registered.append(value)
    for value in registered:
        try:
            normalized = os.path.normcase(os.path.abspath(value))
            if normalized == wanted:
                return True
            # Git for Windows may report a drive through an MSYS-style alias.
            # A matching path suffix is enough to prove that the target may be
            # registered, but not enough to authorize removal, so fail closed.
            raw_normalized = value.replace("\\", "/").lower().rstrip("/")
            if raw_normalized.endswith("/" + target_tail):
                return None
        except (TypeError, ValueError):
            return None
    return False


def _gc_obsolete_scratch(
    root: Path, campaign: Mapping[str, Any], *,
    minimum_age_seconds: float = GC_MINIMUM_AGE_SECONDS,
) -> list[str]:
    """Retire identity-bound inactive scratch repos from old campaigns.

    Only a marker whose digest, campaign slug, and exact path all validate is
    eligible.  A worker lease must be acquired first; any active/inflight
    owner state, path indirection, Git-list ambiguity, or removal failure
    skips the repo and leaves it for a later maintenance pass.
    """

    scratch_root = _state_root(root) / "scratch"
    if not scratch_root.is_dir() or _path_has_indirection(root, scratch_root):
        return []
    if _scratch_has_active_state(root):
        return []
    current_slug = _slug(str(campaign["campaign_id"]))
    now = time.time()
    removed: list[str] = []
    try:
        campaign_dirs = list(scratch_root.iterdir())
    except OSError:
        return []
    for campaign_dir in campaign_dirs:
        if (
            not campaign_dir.is_dir() or campaign_dir.is_symlink()
            or campaign_dir.name == current_slug
            or _path_has_indirection(root, campaign_dir)
        ):
            continue
        try:
            entries = list(campaign_dir.iterdir())
        except OSError:
            continue
        for scratch in entries:
            if (
                not scratch.is_dir() or scratch.is_symlink()
                or not re.fullmatch(r"repo-[0-4]", scratch.name)
                or _path_has_indirection(root, scratch)
            ):
                continue
            identity = _read_owned_scratch_identity(scratch)
            if identity is None or _slug(identity["campaign_id"]) != campaign_dir.name:
                continue
            try:
                marker_age = now - (scratch / ".owner-campaign-identity.json").stat().st_mtime
            except OSError:
                continue
            if marker_age < max(0.0, minimum_age_seconds):
                continue
            lease = scratch.with_name(f"{scratch.name}.lease")
            try:
                with _exclusive_lock(lease, 0.0):
                    if _scratch_has_active_state(root):
                        continue
                    registration = _registered_scratch_worktree(root, campaign, scratch)
                    if registration is None:
                        continue
                    if registration:
                        try:
                            result = subprocess.run(
                                _git_argv(campaign, "worktree", "remove", "--force", str(scratch)),
                                cwd=root, capture_output=True, text=True, check=False,
                                timeout=_command_timeout_seconds(campaign),
                            )
                        except (OSError, subprocess.TimeoutExpired):
                            continue
                        if result.returncode or scratch.exists():
                            continue
                    else:
                        if _path_has_indirection(root, scratch):
                            continue
                        try:
                            shutil.rmtree(scratch)
                        except OSError:
                            continue
                        if scratch.exists():
                            continue
                    removed.append(str(scratch))
            except InfrastructureError:
                # A held lease means an active worker owns this repo.  A
                # non-blocking probe must not turn that into a maintenance
                # failure or delete the active checkout.
                continue
            try:
                lease.unlink(missing_ok=True)
            except OSError:
                pass
        try:
            campaign_dir.rmdir()
        except OSError:
            pass
    return removed


def _tree_size(path: Path) -> int:
    if not path.exists():
        return 0
    total = 0
    for item in path.rglob("*"):
        try:
            if item.is_file() and not item.is_symlink():
                total += item.stat().st_size
        except OSError:
            continue
    return total


def _ensure_state_write_peak(
    root: Path, campaign: Mapping[str, Any], writes: Sequence[tuple[Path, bytes]],
) -> None:
    """Fail before publication if atomic temp files would exceed retained caps."""

    owner_root = _owner_root(root, campaign)
    state_root = _state_root(root)
    owner_extra = 0
    global_extra = 0
    for path, payload in writes:
        if not _inside(state_root, path):
            raise CampaignError("retained state write escapes campaign state root")
        global_extra += len(payload)
        if path == owner_root or _inside(owner_root, path):
            owner_extra += len(payload)
    if _tree_size(owner_root) + owner_extra > campaign["limits"]["owner_state_bytes"]:
        raise CampaignError("retained owner state would exceed peak hard limit")
    retained = (
        _tree_size(state_root / "owners")
        + _tree_size(state_root / "proof-cas")
        + _tree_size(state_root / "inbox")
        + _tree_size(state_root / "tool-cas")
    )
    if retained + global_extra > 64 << 20:
        raise CampaignError("retained global campaign state would exceed peak hard limit")


def _check_cancelled(root: Path, campaign: Mapping[str, Any]) -> None:
    control = _owner_root(root, campaign) / "campaign-control.json"
    if not control.exists():
        return
    value = _read_json(control, "campaign control")
    if (
        not isinstance(value, Mapping)
        or set(value) != {"schema", "owner", "cancellation_epoch", "cancelled_at", "control_sha256"}
    ):
        raise CampaignError("campaign control is invalid")
    body = dict(value)
    digest = body.pop("control_sha256", None)
    if digest != _digest_json(body):
        raise CampaignError("campaign control digest is invalid")
    if value["owner"] != campaign["owner"]:
        raise CampaignError("campaign control owner mismatch")
    if value["cancellation_epoch"] >= campaign["cancellation_epoch"]:
        raise CampaignError("campaign is cancelled at the active epoch")


def cancel_campaign(root: Path, campaign: Mapping[str, Any], epoch: int) -> dict[str, Any]:
    if type(epoch) is not int or epoch < campaign["cancellation_epoch"]:
        raise CampaignError("cancellation epoch must be at least the manifest epoch")
    body = {
        "schema": "owner_campaign_control/v1", "owner": campaign["owner"],
        "cancellation_epoch": epoch, "cancelled_at": _now(),
    }
    value = {**body, "control_sha256": _digest_json(body)}
    _atomic_json(_owner_root(root, campaign) / "campaign-control.json", value)
    return value


def _verify_tracked_context_inputs(
    campaign: Mapping[str, Any], scratch: Path | None = None,
) -> None:
    root = Path(campaign["_root"])
    for descriptor in campaign.get("_tracked_context", []):
        cas = Path(descriptor["_cas"])
        if (
            _path_has_indirection(root, cas)
            or not _is_regular_file(cas)
            or cas.stat().st_size != descriptor["size"]
            or _digest_file(cas) != descriptor["sha256"]
        ):
            raise InfrastructureError("tracked context CAS drift before hook execution")
        if scratch is None or descriptor["path"] == campaign["source_relpath"]:
            continue
        destination = _bound_path(
            scratch, descriptor["path"], "scratch tracked context", exists=False
        )
        if (
            _path_has_indirection(scratch, destination)
            or not _is_regular_file(destination)
            or destination.stat().st_size != descriptor["size"]
            or _digest_file(destination) != descriptor["sha256"]
        ):
            raise InfrastructureError("scratch tracked context hash drift")
        executable = bool(destination.stat().st_mode & 0o111)
        if executable != descriptor["executable"]:
            raise InfrastructureError("scratch tracked context mode drift")


def _materialize_scratch_context(
    root: Path, scratch: Path, campaign: Mapping[str, Any],
) -> None:
    for descriptor in campaign.get("_tracked_context", []):
        if descriptor["path"] == campaign["source_relpath"]:
            continue
        cas = Path(descriptor["_cas"])
        if (
            _path_has_indirection(root, cas)
            or not _is_regular_file(cas)
            or cas.stat().st_size != descriptor["size"]
            or _digest_file(cas) != descriptor["sha256"]
        ):
            raise InfrastructureError("tracked context CAS drift during scratch bootstrap")
        payload = cas.read_bytes()
        destination = _bound_path(
            scratch, descriptor["path"], "scratch tracked context", exists=False
        )
        if _path_has_indirection(scratch, destination):
            raise InfrastructureError("scratch tracked context uses indirection")
        if _tree_size(scratch) + len(payload) > campaign["limits"]["scratch_hard_bytes"]:
            raise InfrastructureError("tracked context exceeds scratch hard limit")
        _atomic_bytes(destination, payload)
        mode = destination.stat().st_mode
        destination.chmod(
            mode | 0o111 if descriptor["executable"] else mode & ~0o111
        )
    _verify_tracked_context_inputs(campaign, scratch)


def _ensure_scratch(
    root: Path, campaign: Mapping[str, Any], worker: int = 0
) -> Path:
    scratch = _scratch_repo(root, campaign, worker)
    scratch_parent = _state_root(root) / "scratch" / _slug(str(campaign["campaign_id"]))
    if scratch.parent != scratch_parent or not _inside(_state_root(root), scratch):
        raise InfrastructureError("scratch path is outside campaign-owned containment")

    def valid() -> bool:
        if (
            not scratch.is_dir() or not (scratch / ".git").is_file()
            or not _scratch_is_owned(campaign, scratch)
        ):
            return False
        head = subprocess.run(
            _git_argv(campaign, "rev-parse", "HEAD"), cwd=scratch, capture_output=True,
            text=True, check=False,
        )
        if head.returncode or head.stdout.strip() != campaign["base_commit"]:
            return False
        # The exact scratch pathname is selected by this process.  On Windows,
        # Git may print its MSYS `/home/...` alias for --show-toplevel, so a
        # textual path comparison is not a stable identity check.  A worktree
        # marker file plus the exact detached HEAD is unambiguous at this
        # already-owned path.
        return True

    if valid():
        reset = subprocess.run(
            _git_argv(campaign, "reset", "--hard", campaign["base_commit"]), cwd=scratch,
            capture_output=True, text=True, check=False,
        )
        if reset.returncode:
            raise InfrastructureError(
                f"scratch reset failed: {reset.stderr.strip()[:1000]}"
            )
        if _digest_file(scratch / campaign["source_relpath"]) != campaign["_base_source_sha256"]:
            raise InfrastructureError("scratch base source identity drift")
        _materialize_scratch_context(root, scratch, campaign)
        _materialize_scratch_target(root, scratch, campaign)
        return scratch
    if scratch.exists():
        if not _scratch_is_owned(campaign, scratch):
            raise InfrastructureError("existing scratch lacks campaign-owned identity")
        removed = subprocess.run(
            _git_argv(campaign, "worktree", "remove", "--force", str(scratch)), cwd=root,
            capture_output=True, text=True, check=False,
        )
        if scratch.exists():
            reason = (removed.stderr or removed.stdout).strip()[:1000]
            raise InfrastructureError(
                "campaign-owned scratch removal did not complete"
                + (f": {reason}" if reason else "")
            )
    scratch.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        _git_argv(campaign, "worktree", "add", "--detach", str(scratch), campaign["base_commit"]),
        cwd=root, capture_output=True, text=True, check=False,
    )
    if result.returncode:
        raise InfrastructureError(f"scratch worktree creation failed: {result.stderr.strip()[:1000]}")
    _atomic_json(scratch / ".owner-campaign-identity.json", _scratch_identity(campaign, scratch))
    if not valid():
        raise InfrastructureError("scratch worktree identity verification failed")
    if _digest_file(scratch / campaign["source_relpath"]) != campaign["_base_source_sha256"]:
        raise InfrastructureError("scratch source does not match manifest base blob")
    _materialize_scratch_context(root, scratch, campaign)
    _materialize_scratch_target(root, scratch, campaign)
    return scratch


@contextmanager
def _exclusive_lock(path: Path, timeout: float):
    """Hold a bounded cross-process byte lock without deleting its inode."""

    path.parent.mkdir(parents=True, exist_ok=True)
    stream = path.open("a+b")
    if path.stat().st_size == 0:
        stream.write(b"\0")
        stream.flush()
    deadline = time.monotonic() + timeout
    locked = False
    try:
        while not locked:
            try:
                stream.seek(0)
                if os.name == "nt":
                    import msvcrt

                    msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
                else:
                    import fcntl

                    fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                locked = True
            except OSError:
                if time.monotonic() >= deadline:
                    raise InfrastructureError(f"frontier CAS lock timed out: {path}")
                time.sleep(0.02)
        yield
    finally:
        if locked:
            stream.seek(0)
            if os.name == "nt":
                import msvcrt

                msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
            else:
                import fcntl

                fcntl.flock(stream.fileno(), fcntl.LOCK_UN)
        stream.close()


def _scratch_lease_path(
    root: Path, campaign: Mapping[str, Any], worker: int,
) -> Path:
    """Return the stable cross-process lease for one reusable worker checkout."""

    return _scratch_repo(root, campaign, worker).with_name(
        f"repo-{worker}.lease"
    )


@contextmanager
def _scratch_lease(
    root: Path, campaign: Mapping[str, Any], worker: int,
):
    """Serialize reset/sync/hook/cleanup for a campaign worker.

    Reusable scratch repositories are intentionally shared by sequential
    cells, but they must never be shared concurrently.  The lease lives next
    to (rather than inside) the worktree, so Git worktree removal cannot
    remove the lock inode while another process is waiting on it.
    """

    if type(worker) is not int or not 0 <= worker < 5:
        raise CampaignError("scratch lease worker index must be between 0 and 4")
    timeout = _command_timeout_seconds(campaign)
    with _exclusive_lock(_scratch_lease_path(root, campaign, worker), timeout):
        yield


@contextmanager
def _frontier_lock_chain(
    root: Path, campaign: Mapping[str, Any], function: str,
):
    """Acquire frontier state locks in the one canonical order.

    Source publication and the per-function frontier are one CAS domain.  The
    focus CAS is taken last because recovery/publication may also materialize
    the focus blob.  Keeping this order shared by snapshot, recovery, status,
    and retention prevents a snapshot that was measured against an old source
    from racing a retained candidate.
    """

    timeout = _command_timeout_seconds(campaign)
    directory = _function_root(root, campaign, function)
    with _exclusive_lock(_owner_root(root, campaign) / "source-cas.lock", timeout):
        with _exclusive_lock(directory / "frontier-cas.lock", timeout):
            with _exclusive_lock(
                _state_root(root) / "proof-cas" / "focus-cas.lock", timeout
            ):
                yield


def _sync_scratch_source(root: Path, scratch: Path, campaign: Mapping[str, Any], source_bytes: bytes) -> Path:
    relative = campaign["source_relpath"]
    destination = _bound_path(scratch, relative, "scratch source", exists=False)
    # Atomic replacement briefly holds both the old destination and the temp
    # file. Reject before writing when that peak would exceed the hard cap.
    if _tree_size(scratch) + len(source_bytes) > campaign["limits"]["scratch_hard_bytes"]:
        raise InfrastructureError("scratch source publication exceeds peak hard limit")
    _atomic_bytes(destination, source_bytes)
    return destination


def _scratch_target_path(
    root: Path, scratch: Path, campaign: Mapping[str, Any]
) -> Path:
    """Map the root-contained manifest target to the same scratch-relative path."""

    target = Path(os.path.abspath(campaign["_target"]))
    try:
        relative = target.relative_to(root)
    except ValueError as exc:
        # load_campaign currently rejects external bindings.  Keep that
        # contract fail-closed if a synthetic/in-memory campaign bypasses load.
        raise InfrastructureError(
            "target object is outside the campaign root"
        ) from exc
    destination = _bound_path(
        scratch, relative.as_posix(), "scratch target object", exists=False
    )
    if _path_has_indirection(scratch, destination):
        raise InfrastructureError(
            "scratch target path uses symlink/reparse indirection"
        )
    return destination


def _verify_scratch_target(
    root: Path, scratch: Path, campaign: Mapping[str, Any]
) -> Path:
    destination = _scratch_target_path(root, scratch, campaign)
    if _path_has_indirection(scratch, destination):
        raise InfrastructureError(
            "scratch target path uses symlink/reparse indirection"
        )
    if not _is_regular_file(destination):
        raise InfrastructureError("scratch target object is not a regular file")
    try:
        size = destination.stat().st_size
        digest = _digest_file(destination)
    except OSError as exc:
        raise InfrastructureError(
            f"scratch target object cannot be verified: {exc}"
        ) from exc
    if size != campaign["_target_size"]:
        raise InfrastructureError("scratch target object size drift")
    if digest != campaign["target_object"]["sha256"]:
        raise InfrastructureError("scratch target object hash drift")
    return destination


def _materialize_scratch_target(
    root: Path, scratch: Path, campaign: Mapping[str, Any]
) -> Path:
    """Atomically seed one worker with the immutable manifest-bound target."""

    source = Path(campaign["_target"])
    if _path_has_indirection(root, source) or not _is_regular_file(source):
        raise InfrastructureError(
            "target object uses indirection or is not regular during scratch bootstrap"
        )
    try:
        before = source.stat()
        payload = source.read_bytes()
        after = source.stat()
    except OSError as exc:
        raise InfrastructureError(
            f"target object cannot be read during scratch bootstrap: {exc}"
        ) from exc
    expected_size = campaign["_target_size"]
    expected_sha = campaign["target_object"]["sha256"]
    identity_before = (
        before.st_dev, before.st_ino, before.st_size,
        before.st_mtime_ns, before.st_ctime_ns,
    )
    identity_after = (
        after.st_dev, after.st_ino, after.st_size,
        after.st_mtime_ns, after.st_ctime_ns,
    )
    if (
        identity_before != identity_after
        or len(payload) != expected_size
        or _digest_bytes(payload) != expected_sha
    ):
        raise InfrastructureError("target object drifted during scratch bootstrap")
    # Recheck the source path after the read so a concurrent replacement cannot
    # silently turn the read-once payload into evidence for a different inode.
    if (
        _path_has_indirection(root, source)
        or not _is_regular_file(source)
    ):
        raise InfrastructureError("target object changed during scratch bootstrap")
    destination = _scratch_target_path(root, scratch, campaign)
    destination.parent.mkdir(parents=True, exist_ok=True)
    if _path_has_indirection(scratch, destination):
        raise InfrastructureError(
            "scratch target parent uses symlink/reparse indirection"
        )
    # Atomic replacement briefly owns both the existing destination and temp.
    if _tree_size(scratch) + len(payload) > campaign["limits"]["scratch_hard_bytes"]:
        raise InfrastructureError(
            "scratch target materialization exceeds peak hard limit"
        )
    _atomic_bytes(destination, payload)
    return _verify_scratch_target(root, scratch, campaign)


def _verify_publication_sources(
    campaign: Mapping[str, Any], scratch: Path, *, live_sha256: str | None,
    scratch_sha256: str,
) -> None:
    if live_sha256 is not None and _digest_file(campaign["_source"]) != live_sha256:
        raise CampaignError("authoritative source drifted before frontier publication")
    scratch_source = scratch / campaign["source_relpath"]
    if _digest_file(scratch_source) != scratch_sha256:
        raise CampaignError("scratch source drifted before frontier publication")
    _verify_tracked_context_inputs(campaign, scratch)
    _verify_scratch_target(Path(campaign["_root"]), scratch, campaign)


def _expand_argv(
    argv: Sequence[str], *, root: Path, scratch: Path, campaign: Mapping[str, Any],
    function: str, source_sha256: str, phase: str,
) -> list[str]:
    values = {
        "ROOT": str(root), "SCRATCH_ROOT": str(scratch),
        "SOURCE": str(scratch / campaign["source_relpath"]),
        "FUNCTION": function, "OWNER": campaign["owner"], "UNIT": campaign["unit"],
        "TARGET": str(_scratch_target_path(root, scratch, campaign)),
        "TOOLCHAIN": str(campaign["_toolchain"]),
        "MEASUREMENT_PRODUCER": str(campaign["_producer"]),
        "SOURCE_SHA256": source_sha256, "PHASE": phase,
    }
    expanded: list[str] = []
    for item in argv:
        try:
            expanded.append(item.format_map(values))
        except KeyError as exc:
            raise CampaignError(f"command uses unknown placeholder: {exc.args[0]}") from exc
    return expanded


def _verify_hook_inputs(
    campaign: Mapping[str, Any], scratch: Path | None = None,
) -> None:
    """Revalidate immutable command inputs immediately before every launch."""

    root = Path(campaign["_root"])
    for label, private in (
        ("target_object", "_target"),
        ("toolchain", "_toolchain"),
        ("measurement_producer", "_producer"),
    ):
        path = campaign[private]
        # ``Path.is_file`` follows symlinks and Windows reparse points.  The
        # manifest is loaded before a hook starts, so a path can be replaced
        # after load even though its original binding was safe.  Re-check the
        # whole path chain and use lstat-based regular-file validation before
        # opening anything for hashing.  This is deliberately an
        # InfrastructureError: the immutable input changed and the caller may
        # retry after repairing the campaign-owned CAS, but no subprocess may
        # observe the replacement.
        if _path_has_indirection(root, path):
            raise InfrastructureError(
                f"{label} path uses symlink/reparse indirection before hook execution"
            )
        if not _is_regular_file(path):
            raise InfrastructureError(
                f"{label} is not a regular file before hook execution"
            )
        try:
            digest = _digest_file(path)
        except OSError as exc:
            raise InfrastructureError(
                f"{label} cannot be read before hook execution: {exc}"
            ) from exc
        if digest != campaign[label]["sha256"]:
            raise InfrastructureError(f"{label} hash drift before hook execution")
    _verify_tracked_context_inputs(campaign, scratch)


def _hook_environment(
    scratch: Path,
    campaign: Mapping[str, Any],
    function: str,
    source_sha256: str,
    phase: str,
) -> dict[str, str]:
    """Build the sealed environment for a campaign measurement subprocess.

    Measurement producers are executed from campaign CAS by absolute path,
    while their imports must resolve against the hash-bound detached scratch
    checkout.  Do not inherit a caller-controlled ``PYTHONPATH``: it can
    shadow the scratch ``tools`` package and make a measurement depend on the
    manager's ambient environment.
    """

    environment = dict(os.environ)
    environment["PYTHONPATH"] = str(scratch.resolve())
    environment["PYTHONNOUSERSITE"] = "1"
    environment.update({
        "OWNER_CAMPAIGN_PHASE": phase,
        "OWNER_CAMPAIGN_ID": str(campaign["campaign_id"]),
        "OWNER_CAMPAIGN_MANIFEST_SHA256": str(campaign["manifest_sha256"]),
        "OWNER_CAMPAIGN_OWNER": str(campaign["owner"]),
        "OWNER_CAMPAIGN_UNIT": str(campaign["unit"]),
        "OWNER_CAMPAIGN_FUNCTION": function,
        "OWNER_CAMPAIGN_SOURCE_SHA256": source_sha256,
        "OWNER_CAMPAIGN_TARGET_SHA256": str(campaign["target_object"]["sha256"]),
        "OWNER_CAMPAIGN_TOOLCHAIN_SHA256": str(campaign["toolchain"]["sha256"]),
        "OWNER_CAMPAIGN_BASE_COMMIT": str(campaign["base_commit"]),
        "OWNER_CAMPAIGN_SOURCE_PATH": str(campaign["source_relpath"]),
        "OWNER_CAMPAIGN_MEASUREMENT_PRODUCER_SHA256": str(
            campaign["measurement_producer"]["sha256"]
        ),
        "OWNER_CAMPAIGN_PROTECTED_TOTAL": str(
            len(_protected_sibling_functions(campaign, function))
        ),
        "OWNER_CAMPAIGN_PROTECTED_FUNCTIONS": ",".join(
            campaign["protected_exact_functions"]
        ),
        "OWNER_CAMPAIGN_RECONSTRUCTION": "1",
    })
    return environment


def _terminate_process_tree(process: subprocess.Popen[bytes]) -> None:
    """Best-effort bounded termination of the hook and every descendant."""

    if process.poll() is not None:
        return
    if os.name == "nt":
        subprocess.run(
            ["taskkill", "/PID", str(process.pid), "/T", "/F"],
            capture_output=True, check=False,
        )
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except (ProcessLookupError, PermissionError):
            process.kill()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def _run_bounded_process(
    argv: Sequence[str], *, cwd: Path, environment: Mapping[str, str],
    timeout: float, scratch: Path, temporary_root: Path,
    scratch_hard_bytes: int, cell_temporary_bytes: int,
) -> subprocess.CompletedProcess[bytes]:
    """Run one hook while enforcing elapsed-time and peak-storage ceilings."""

    from tools import bounded_process

    def check_storage() -> None:
        if _tree_size(scratch) > scratch_hard_bytes:
            raise InfrastructureError("campaign scratch exceeded hard limit during command")
        if _tree_size(temporary_root) > cell_temporary_bytes:
            raise InfrastructureError("cell temporary storage exceeded limit during command")

    try:
        return bounded_process.run(argv, cwd=cwd, env=dict(environment), timeout=timeout,
                                   max_output=MAX_OUTPUT, check=check_storage)
    except (OSError, bounded_process.ProcessLimitError) as exc:
        raise InfrastructureError(str(exc)) from exc


def _run_hook(
    root: Path, scratch: Path, campaign: Mapping[str, Any], function: str,
    source_sha256: str, phase: str,
) -> dict[str, Any]:
    _verify_hook_inputs(campaign, scratch)
    _verify_scratch_target(root, scratch, campaign)
    descriptor = campaign["commands"][phase]
    output = _bound_path(scratch, descriptor["measurement_relpath"], "measurement output", exists=False)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.unlink(missing_ok=True)
    argv = _expand_argv(
        descriptor["argv"], root=root, scratch=scratch, campaign=campaign,
        function=function, source_sha256=source_sha256, phase=phase,
    )
    try:
        environment = _hook_environment(
            scratch, campaign, function, source_sha256, phase
        )
        # Recheck after all command construction and directly before process
        # creation.  The earlier check rejects stale state before touching the
        # scratch output; this final check closes the pre-launch replacement
        # window for target/toolchain/CAS producer inputs.
        _verify_hook_inputs(campaign, scratch)
        _verify_scratch_target(root, scratch, campaign)
        result = _run_bounded_process(
            argv, cwd=scratch, environment=environment,
            timeout=float(campaign["limits"]["command_timeout_seconds"]),
            scratch=scratch, temporary_root=output.parent,
            scratch_hard_bytes=campaign["limits"]["scratch_hard_bytes"],
            cell_temporary_bytes=campaign["limits"]["cell_temporary_bytes"],
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        raise InfrastructureError(f"{phase} command failed to terminate: {exc}") from exc
    if len(result.stdout) + len(result.stderr) > MAX_OUTPUT:
        raise InfrastructureError(f"{phase} command output exceeded {MAX_OUTPUT} bytes")
    if result.returncode:
        reason = (result.stderr or result.stdout).decode("utf-8", "replace")[:1000]
        raise InfrastructureError(f"{phase} command exited {result.returncode}: {reason}")
    if not output.is_file():
        raise InfrastructureError(f"{phase} command did not publish measurement: {output}")
    if _tree_size(output.parent) > campaign["limits"]["cell_temporary_bytes"]:
        raise InfrastructureError(f"{phase} measurement exceeds cell temporary limit")
    try:
        return _validate_measurement(
            _read_json(output, f"{phase} measurement"), campaign=campaign,
            function=function, phase=phase, source_sha256=source_sha256,
        )
    finally:
        output.unlink(missing_ok=True)


def _run_final_owner(
    root: Path, scratch: Path, campaign: Mapping[str, Any], function: str,
    source_sha256: str,
) -> dict[str, Any]:
    _verify_hook_inputs(campaign, scratch)
    descriptor = campaign["commands"]["final_owner"]
    output = _bound_path(
        scratch, descriptor["measurement_relpath"], "final owner output", exists=False
    )
    output.parent.mkdir(parents=True, exist_ok=True)
    output.unlink(missing_ok=True)
    argv = _expand_argv(
        descriptor["argv"], root=root, scratch=scratch, campaign=campaign,
        function=function, source_sha256=source_sha256, phase="final_owner",
    )
    environment = _hook_environment(
        scratch, campaign, function, source_sha256, "final_owner"
    )
    try:
        _verify_hook_inputs(campaign, scratch)
        result = _run_bounded_process(
            argv, cwd=scratch, environment=environment,
            timeout=float(campaign["limits"]["command_timeout_seconds"]),
            scratch=scratch, temporary_root=output.parent,
            scratch_hard_bytes=campaign["limits"]["scratch_hard_bytes"],
            cell_temporary_bytes=campaign["limits"]["cell_temporary_bytes"],
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        raise InfrastructureError(f"final_owner command failed to terminate: {exc}") from exc
    if len(result.stdout) + len(result.stderr) > MAX_OUTPUT:
        raise InfrastructureError("final_owner command output exceeded compact limit")
    if result.returncode:
        reason = (result.stderr or result.stdout).decode("utf-8", "replace")[:1000]
        raise InfrastructureError(
            f"final_owner command exited {result.returncode}: {reason}"
        )
    if not output.is_file():
        raise InfrastructureError("final_owner command omitted its proof receipt")
    if output.stat().st_size > campaign["limits"]["report_bytes"]:
        raise InfrastructureError("final_owner proof receipt exceeds compact report limit")
    try:
        return _validate_final_owner_receipt(
            _read_json(output, "final owner receipt"), campaign, source_sha256
        )
    finally:
        output.unlink(missing_ok=True)


CHANNEL_FIELDS = {"target_bytes", "candidate_bytes", "differences"}
METRIC_FIELDS = {
    "strict", "data", "physical_target_count", "physical_candidate_count",
    "physical_differences", "protected_total", "protected_losses", "source_link_exact",
}
MEASUREMENT_FIELDS = {
    "schema", "phase", "campaign_id", "manifest_sha256", "owner", "unit", "function",
    "source_path", "base_commit", "source_sha256", "target_object_sha256", "toolchain_sha256",
    "measurement_producer_sha256",
    "candidate_object_sha256", "metrics", "report_receipts", "proofs", "focus_evidence",
    "exact_report",
    "measurement_sha256",
}
MEASUREMENT_OPTIONAL_FIELDS = {"reconstruction_evidence"}
FOCUS_FIELDS = {
    "schema", "owner", "function", "unit", "source_path", "base_commit",
    "source_sha256", "target_object_sha256",
    "strict_rows", "data_rows", "physical_differences", "sibling_identities",
    "strict_row_ids", "strict_row_ids_sha256", "data_row_ids",
    "data_row_ids_sha256", "physical_difference_ids",
    "physical_difference_ids_sha256", "physical_target_identity_sha256",
    "physical_candidate_identity_sha256", "strict_row_count", "data_row_count",
    "physical_target_count", "physical_candidate_count",
    "physical_difference_count", "protected_total", "protected_losses",
    "sibling_digest",
    "focus_evidence_sha256",
}


def _measurement_requires_reconstruction(
    root: Path, campaign: Mapping[str, Any]
) -> bool:
    """Return whether the campaign pins this checkout's packet producer.

    Older campaigns execute a content-addressed historical producer and must
    remain readable.  A campaign pinned to the current adapter, however, may
    not strip the packet and reseal its measurement as a legacy envelope.
    """

    current = root / "tools" / "owner_campaign_measure.py"
    return (
        current.is_file()
        and _digest_file(current) == campaign["measurement_producer"]["sha256"]
    )


def _validate_focus_evidence(
    value: Any, campaign: Mapping[str, Any], function: str, source_sha256: str,
) -> dict[str, Any]:
    value = _closed_keys(value, FOCUS_FIELDS, "focus evidence")
    body = dict(value)
    digest = _sha(
        body.pop("focus_evidence_sha256", None), "focus_evidence_sha256"
    )
    if digest != _digest_json(body):
        raise CampaignError("focus evidence digest is invalid")
    if (
        value["schema"] != "owner_campaign_focus_evidence/v1"
        or value["owner"] != campaign["owner"] or value["function"] != function
        or value["unit"] != campaign["unit"]
        or value["source_path"] != campaign["source_relpath"]
        or value["base_commit"] != campaign["base_commit"]
        or value["source_sha256"] != source_sha256
        or value["target_object_sha256"] != campaign["target_object"]["sha256"]
    ):
        raise CampaignError("focus evidence identity is invalid")
    for field in (
        "strict_rows", "data_rows", "physical_differences", "sibling_identities",
        "strict_row_ids", "data_row_ids", "physical_difference_ids",
    ):
        rows = value[field]
        if (
            not isinstance(rows, list) or len(rows) > 2048
            or len(rows) != len(set(rows))
            or any(not isinstance(item, str) or not item or len(item) > 512 for item in rows)
        ):
            raise CampaignError(f"focus evidence {field} is invalid")
    for field, digest_field in (
        ("strict_row_ids", "strict_row_ids_sha256"),
        ("data_row_ids", "data_row_ids_sha256"),
        ("physical_difference_ids", "physical_difference_ids_sha256"),
    ):
        if _digest_json(value[field]) != _sha(value[digest_field], digest_field):
            raise CampaignError(f"focus evidence {field} digest is invalid")
    _sha(value["physical_target_identity_sha256"], "physical target identity")
    _sha(value["physical_candidate_identity_sha256"], "physical candidate identity")
    expected_counts = {
        "strict_row_count": len(value["strict_row_ids"]),
        "data_row_count": len(value["data_row_ids"]),
        "physical_difference_count": len(value["physical_difference_ids"]),
    }
    if any(value[field] != count for field, count in expected_counts.items()):
        raise CampaignError("focus evidence row counts do not match identities")
    for field in (
        "physical_target_count", "physical_candidate_count", "protected_total",
        "protected_losses",
    ):
        if type(value[field]) is not int or value[field] < 0:
            raise CampaignError(f"focus evidence {field} is invalid")
    _sha(value["sibling_digest"], "focus evidence sibling_digest")
    if len(_canonical(value)) > campaign["limits"]["focus_evidence_bytes"]:
        raise CampaignError("focus evidence exceeds campaign compact limit")
    return dict(value)


def _publish_focus_evidence(
    root: Path, campaign: Mapping[str, Any], evidence: Mapping[str, Any]
) -> str:
    digest = evidence["focus_evidence_sha256"]
    path = (
        _state_root(root) / "proof-cas" / "focus" / digest[:2]
        / f"{digest}.json"
    )
    marker = _evidence_gc_marker(path)
    with _exclusive_lock(
        _evidence_blob_lock(root, "focus", digest),
        _command_timeout_seconds(campaign),
    ):
        if path.is_file():
            if _read_json(path, "focus evidence CAS") != dict(evidence):
                raise CampaignError("focus evidence CAS publication drift")
            marker.unlink(missing_ok=True)
            os.utime(path, None)
            return digest
        _ensure_state_write_peak(
            root, campaign, [(path, _canonical(evidence) + b"\n")]
        )
        _atomic_json(
            path, evidence, limit=campaign["limits"]["focus_evidence_bytes"]
        )
        if _read_json(path, "focus evidence CAS") != dict(evidence):
            raise CampaignError("focus evidence CAS publication drift")
        marker.unlink(missing_ok=True)
    return digest


def _publish_reconstruction_evidence(
    root: Path, campaign: Mapping[str, Any], evidence: Mapping[str, Any]
) -> str:
    """Publish one independently verified target-first packet to proof CAS."""

    try:
        from tools.owner_campaign_reconstruction import (
            MAX_OUTPUT_BYTES, ReconstructionPacketError, verify_packet,
        )

        verify_packet(evidence)
    except ReconstructionPacketError as exc:
        raise CampaignError(
            f"reconstruction evidence failed publication verification: {exc}"
        ) from exc
    digest = _sha(evidence.get("packet_sha256"), "reconstruction packet_sha256")
    path = (
        _state_root(root) / "proof-cas" / "reconstruction" / digest[:2]
        / f"{digest}.json"
    )
    payload = _canonical(evidence) + b"\n"
    if len(payload) > MAX_OUTPUT_BYTES + 1:
        raise CampaignError("reconstruction evidence exceeds compact CAS limit")
    marker = _evidence_gc_marker(path)
    with _exclusive_lock(
        _evidence_blob_lock(root, "reconstruction", digest),
        _command_timeout_seconds(campaign),
    ):
        if path.is_file():
            if _read_json(path, "reconstruction evidence CAS") != dict(evidence):
                raise CampaignError("reconstruction evidence CAS publication drift")
            marker.unlink(missing_ok=True)
            os.utime(path, None)
            return digest
        _ensure_state_write_peak(root, campaign, [(path, payload)])
        _atomic_json(path, evidence, limit=MAX_OUTPUT_BYTES)
        if _read_json(path, "reconstruction evidence CAS") != dict(evidence):
            raise CampaignError("reconstruction evidence CAS publication drift")
        marker.unlink(missing_ok=True)
    return digest


GC_MARKER_SCHEMA = "owner_campaign_evidence_gc_marker/v1"


def _evidence_blob_lock(root: Path, kind: str, digest: str) -> Path:
    return (
        _state_root(root) / "proof-cas" / "blob-locks" / kind / digest[:2]
        / f"{digest}.lock"
    )


def _evidence_gc_marker(blob: Path) -> Path:
    return blob.with_suffix(".gc")


def _gc_now_ns() -> int:
    return time.time_ns()


def _evidence_gc_references(
    root: Path, digest_field: str, label: str,
) -> set[str] | None:
    """Return retained references, or None when state is unsafe to collect."""

    state = _state_root(root)
    owners = state / "owners"
    for ledger in owners.rglob("candidate-results.jsonl") if owners.is_dir() else ():
        try:
            if any(record["status"] == "inflight" for record in _dedupe_records(ledger)):
                return None
        except CampaignError:
            return None
    referenced: set[str] = set()
    if owners.is_dir():
        for frontier_path in [
            *owners.rglob("latest-frontier.json"),
            *owners.rglob("frontier.pending.json"),
        ]:
            try:
                value = _read_json(frontier_path, f"{label} GC reference")
                if frontier_path.name == "frontier.pending.json":
                    value = value.get("frontier", {})
                digest = value.get(digest_field)
                if isinstance(digest, str) and SHA_RE.fullmatch(digest):
                    referenced.add(digest)
            except CampaignError:
                # Corrupt retained state must remain available for diagnosis;
                # status validation will fail closed rather than GC hiding it.
                return None
    # Candidate selection sidecars retain focus/reconstruction/physical CAS
    # blobs while a proposal is still in flight.  These files live in the
    # inbox rather than under an owner's latest frontier, so omitting them
    # from the reference scan could delete evidence needed to validate a
    # queued proposal.  Scan only the known sidecar names; descriptors and
    # rebase receipts do not carry CAS references.
    kind_by_field = {
        "focus_evidence_sha256": "focus_artifact",
        "reconstruction_evidence_sha256": "reconstruction",
        "physical_summary_sha256": "physical_artifact",
    }
    reference_key = kind_by_field.get(digest_field)
    inbox = state / "inbox"
    if reference_key is not None and inbox.is_dir():
        try:
            sidecars = [
                item for item in inbox.rglob("*.json")
                if item.name.endswith(".selection.json")
                or item.parent.name == "selection"
            ]
        except OSError:
            return None
        for sidecar in sidecars:
            try:
                if _path_has_indirection(root, sidecar):
                    return None
                value = _read_json(sidecar, f"{label} inbox reference")
                if not isinstance(value, Mapping):
                    return None
                reference = value.get(reference_key)
                if reference is None:
                    continue
                if not isinstance(reference, Mapping):
                    return None
                digest = reference.get("sha256", reference.get("file_sha256"))
                if not isinstance(digest, str) or SHA_RE.fullmatch(digest) is None:
                    return None
                referenced.add(digest)
            except (CampaignError, OSError):
                # A live sidecar that cannot be authenticated must prevent
                # collection; losing a blob is worse than deferring GC.
                return None
    return referenced


def _gc_evidence_kind(
    root: Path, *, kind: str, digest_field: str, label: str,
    minimum_age_seconds: float,
) -> None:
    """Mark then sweep unreferenced CAS blobs without the publication lock.

    A first pass only creates a generation marker.  Publication of the same
    digest removes that marker under a short per-blob lock.  A later pass may
    delete only when the blob identity is unchanged and the marker has aged,
    which protects readers that obtained the prior frontier immediately before
    a concurrent retention replaced it.
    """

    referenced = _evidence_gc_references(root, digest_field, label)
    if referenced is None:
        return
    evidence_root = _state_root(root) / "proof-cas" / kind
    if evidence_root.is_dir():
        now_ns = _gc_now_ns()
        minimum_age_ns = max(0, int(minimum_age_seconds * 1_000_000_000))
        for blob in list(evidence_root.rglob("*.json")):
            digest = blob.stem
            if not SHA_RE.fullmatch(digest):
                continue
            marker = _evidence_gc_marker(blob)
            with _exclusive_lock(
                _evidence_blob_lock(root, kind, digest),
                1.0,
            ):
                if not blob.is_file():
                    marker.unlink(missing_ok=True)
                    continue
                if digest in referenced:
                    marker.unlink(missing_ok=True)
                    continue
                stat = blob.stat()
                marker_value: Mapping[str, Any] | None = None
                if marker.is_file():
                    try:
                        candidate = _read_json(marker, f"{label} GC marker")
                        marker_body = dict(candidate)
                        marker_digest = marker_body.pop("marker_sha256", None)
                        if (
                            set(candidate) == {
                                "schema", "kind", "digest", "size", "mtime_ns",
                                "marked_at_ns", "marker_sha256",
                            }
                            and candidate["schema"] == GC_MARKER_SCHEMA
                            and candidate["kind"] == kind
                            and candidate["digest"] == digest
                            and candidate["size"] == stat.st_size
                            and candidate["mtime_ns"] == stat.st_mtime_ns
                            and type(candidate["marked_at_ns"]) is int
                            and marker_digest == _digest_json(marker_body)
                        ):
                            marker_value = candidate
                    except (CampaignError, OSError, TypeError):
                        marker_value = None
                if marker_value is None:
                    marker_body = {
                        "schema": GC_MARKER_SCHEMA, "kind": kind,
                        "digest": digest, "size": stat.st_size,
                        "mtime_ns": stat.st_mtime_ns, "marked_at_ns": now_ns,
                    }
                    _atomic_json(marker, {
                        **marker_body, "marker_sha256": _digest_json(marker_body),
                    })
                    continue
                if now_ns - marker_value["marked_at_ns"] < minimum_age_ns:
                    continue
                current = blob.stat()
                if (
                    current.st_size != marker_value["size"]
                    or current.st_mtime_ns != marker_value["mtime_ns"]
                ):
                    marker.unlink(missing_ok=True)
                    continue
                blob.unlink()
                marker.unlink(missing_ok=True)
        for directory in sorted(
            (item for item in evidence_root.rglob("*") if item.is_dir()),
            key=lambda item: len(item.parts), reverse=True,
        ):
            try:
                directory.rmdir()
            except OSError:
                pass


def _gc_focus_evidence(
    root: Path, *, minimum_age_seconds: float = GC_MINIMUM_AGE_SECONDS,
) -> None:
    _gc_evidence_kind(
        root, kind="focus", digest_field="focus_evidence_sha256",
        label="focus evidence", minimum_age_seconds=minimum_age_seconds,
    )


def _gc_reconstruction_evidence(
    root: Path, *, minimum_age_seconds: float = GC_MINIMUM_AGE_SECONDS,
) -> None:
    _gc_evidence_kind(
        root, kind="reconstruction",
        digest_field="reconstruction_evidence_sha256",
        label="reconstruction evidence",
        minimum_age_seconds=minimum_age_seconds,
    )


def _gc_physical_evidence(
    root: Path, *, minimum_age_seconds: float = GC_MINIMUM_AGE_SECONDS,
) -> None:
    """Collect unreferenced compact physical proof projections."""

    _gc_evidence_kind(
        root, kind="physical", digest_field="physical_summary_sha256",
        label="physical evidence", minimum_age_seconds=minimum_age_seconds,
    )


def _validate_metrics(value: Any) -> dict[str, Any]:
    value = _closed_keys(value, METRIC_FIELDS, "measurement metrics")
    result: dict[str, Any] = {}
    for name in ("strict", "data"):
        channel = _closed_keys(value[name], CHANNEL_FIELDS, f"{name} metrics")
        for key, number in channel.items():
            if type(number) is not int or number < 0:
                raise CampaignError(f"{name}.{key} is invalid")
        result[name] = dict(channel)
    for name in (
        "physical_target_count", "physical_candidate_count", "physical_differences",
        "protected_total", "protected_losses",
    ):
        number = value[name]
        if type(number) is not int or number < 0:
            raise CampaignError(f"metrics.{name} is invalid")
        result[name] = number
    if type(value["source_link_exact"]) is not bool:
        raise CampaignError("metrics.source_link_exact is invalid")
    result["source_link_exact"] = value["source_link_exact"]
    return result


def _protected_sibling_functions(
    campaign: Mapping[str, Any], function: str,
) -> tuple[str, ...]:
    """Return protected exact identities other than the selected focus.

    The campaign inventory may include the function currently being measured,
    but ``focus_symbol_report`` deliberately reports only protected *siblings*
    because the focus has its own strict/data/physical gates.  Keep those
    contracts separate: every other protected identity remains mandatory, and
    the focus remains protected by its own measurement metrics.
    """

    return tuple(
        name for name in campaign["protected_exact_functions"]
        if name != function
    )


def _validate_measurement(
    value: Any, *, campaign: Mapping[str, Any], function: str,
    phase: str, source_sha256: str,
) -> dict[str, Any]:
    observed_fields = set(value) if isinstance(value, Mapping) else set()
    if (
        not MEASUREMENT_FIELDS <= observed_fields
        or observed_fields - MEASUREMENT_FIELDS > MEASUREMENT_OPTIONAL_FIELDS
    ):
        raise CampaignError("campaign measurement has noncanonical fields")
    value = _closed_keys(value, observed_fields, "campaign measurement")
    if (
        _measurement_requires_reconstruction(Path(campaign["_root"]), campaign)
        and "reconstruction_evidence" not in value
    ):
        raise CampaignError(
            "current measurement producer omitted reconstruction evidence"
        )
    body = dict(value)
    digest = _sha(body.pop("measurement_sha256", None), "measurement_sha256")
    if _digest_json(body) != digest:
        raise CampaignError("measurement digest is invalid")
    expected = {
        "schema": MEASUREMENT_SCHEMA, "phase": phase,
        "campaign_id": campaign["campaign_id"], "manifest_sha256": campaign["manifest_sha256"],
        "owner": campaign["owner"], "unit": campaign["unit"], "function": function,
        "source_path": campaign["source_relpath"],
        "base_commit": campaign["base_commit"],
        "source_sha256": source_sha256,
        "target_object_sha256": campaign["target_object"]["sha256"],
        "toolchain_sha256": campaign["toolchain"]["sha256"],
        "measurement_producer_sha256": campaign["measurement_producer"]["sha256"],
    }
    if any(value.get(key) != item for key, item in expected.items()):
        raise CampaignError("measurement identity does not match the campaign")
    _sha(value["candidate_object_sha256"], "candidate_object_sha256")
    metrics = _validate_metrics(value["metrics"])
    protected_siblings = _protected_sibling_functions(campaign, function)
    if metrics["protected_total"] != len(protected_siblings):
        raise CampaignError("measurement protected sibling census is incomplete")
    if _is_exact(metrics) and metrics["protected_losses"] != 0:
        raise CampaignError("exact measurement protected sibling census has losses")
    receipts = value["report_receipts"]
    required = {"strict", "data", "physical", "siblings", "source_link"}
    if not isinstance(receipts, Mapping) or not required <= set(receipts) or len(receipts) > 16:
        raise CampaignError("measurement proof receipts are incomplete")
    for name, receipt in receipts.items():
        _sha(receipt, f"report_receipts.{name}")
    focus = _validate_focus_evidence(
        value["focus_evidence"], campaign, function, source_sha256
    )
    if (
        focus["strict_row_count"] != metrics["strict"]["differences"]
        or focus["data_row_count"] != metrics["data"]["differences"]
        or focus["physical_target_count"] != metrics["physical_target_count"]
        or focus["physical_candidate_count"] != metrics["physical_candidate_count"]
        or focus["physical_difference_count"] != metrics["physical_differences"]
        or focus["protected_total"] != metrics["protected_total"]
        or focus["protected_losses"] != metrics["protected_losses"]
    ):
        raise CampaignError("measurement metrics drift from focus identities")
    proofs = value["proofs"]
    if not isinstance(proofs, Mapping) or set(proofs) != {"source_link", "object", "toolchain"}:
        raise CampaignError("measurement proofs are incomplete")
    for name, proof in proofs.items():
        if not isinstance(proof, Mapping):
            raise CampaignError(f"measurement {name} proof is invalid")
        proof_body = dict(proof)
        proof_sha = _sha(proof_body.pop("proof_sha256", None), f"{name} proof_sha256")
        if _digest_json(proof_body) != proof_sha:
            raise CampaignError(f"measurement {name} proof digest is invalid")
    if not set(protected_siblings) <= set(
        focus["sibling_identities"]
    ):
        raise CampaignError("measurement protected sibling identities are incomplete")
    if phase == "snapshot" and value["exact_report"] is not None:
        raise CampaignError("snapshot measurement cannot publish an exact report")
    # The independent verifier owns semantic proof binding (source/object/
    # toolchain and canonical row/physical identities).  Core admission must
    # not reduce that contract to merely checking self-hashes.
    try:
        from tools.owner_campaign_verify import VerificationError, verify_measurement

        verify_measurement(value, expected=expected)
    except VerificationError as exc:
        raise CampaignError(f"measurement independent verification failed: {exc}") from exc
    return dict(value)


def _is_exact(metrics: Mapping[str, Any]) -> bool:
    return (
        metrics["strict"]["differences"] == 0
        and metrics["strict"]["target_bytes"] == metrics["strict"]["candidate_bytes"]
        and metrics["data"]["differences"] == 0
        and metrics["data"]["target_bytes"] == metrics["data"]["candidate_bytes"]
        and metrics["physical_differences"] == 0
        and metrics["physical_target_count"] == metrics["physical_candidate_count"]
        and metrics["protected_losses"] == 0
        and metrics["source_link_exact"] is True
    )


def assess_gain(
    base: Mapping[str, Any], candidate: Mapping[str, Any], *,
    base_focus: Mapping[str, Any] | None = None,
    candidate_focus: Mapping[str, Any] | None = None,
) -> str:
    """Return ``exact``, ``improved``, or ``no_gain`` under v2 monotonic rules."""

    base_metrics = _validate_metrics(base)
    metrics = _validate_metrics(candidate)
    if (base_focus is None) != (candidate_focus is None):
        raise CampaignError("gain identity comparison requires both focus artifacts")
    improvements = (
        metrics["strict"]["differences"] < base_metrics["strict"]["differences"],
        metrics["data"]["differences"] < base_metrics["data"]["differences"],
        abs(metrics["strict"]["target_bytes"] - metrics["strict"]["candidate_bytes"])
        < abs(base_metrics["strict"]["target_bytes"] - base_metrics["strict"]["candidate_bytes"]),
        abs(metrics["data"]["target_bytes"] - metrics["data"]["candidate_bytes"])
        < abs(base_metrics["data"]["target_bytes"] - base_metrics["data"]["candidate_bytes"]),
        metrics["physical_differences"] < base_metrics["physical_differences"],
        abs(metrics["physical_target_count"] - metrics["physical_candidate_count"])
        < abs(base_metrics["physical_target_count"] - base_metrics["physical_candidate_count"]),
    )
    any_quantitative_gain = any(improvements)
    if base_focus is not None and candidate_focus is not None:
        # Objdiff identities are derived from instruction alignment and may
        # legitimately be renumbered/re-keyed when an earlier mismatch is
        # removed.  A quantitative improvement is the authoritative gain
        # signal; requiring residual IDs to be a set subset would reject real
        # owner improvements such as 25 -> 13 rows after realignment. Without
        # any quantitative gain, retain the identity-subset guard so neutral
        # or unsupported migrations cannot become frontiers.
        if not any_quantitative_gain:
            for field in (
                "strict_row_ids", "data_row_ids", "physical_difference_ids"
            ):
                base_ids = set(base_focus[field])
                candidate_ids = set(candidate_focus[field])
                if not candidate_ids <= base_ids:
                    return "no_gain"
        if (
            candidate_focus["physical_target_identity_sha256"]
            != base_focus["physical_target_identity_sha256"]
        ):
            return "no_gain"
        base_physical = set(base_focus["physical_difference_ids"])
        candidate_physical = set(candidate_focus["physical_difference_ids"])
        if (
            candidate_physical == base_physical
            and candidate_focus["physical_candidate_identity_sha256"]
            != base_focus["physical_candidate_identity_sha256"]
        ):
            return "no_gain"
        if (
            not candidate_physical
            and candidate_focus["physical_candidate_identity_sha256"]
            != base_focus["physical_target_identity_sha256"]
        ):
            return "no_gain"
    if metrics["protected_losses"] != 0:
        return "no_gain"
    for name in ("strict", "data"):
        if base_metrics[name]["differences"] == 0 and metrics[name]["differences"] != 0:
            return "no_gain"
    if (
        metrics["strict"]["differences"] > base_metrics["strict"]["differences"]
        or metrics["data"]["differences"] > base_metrics["data"]["differences"]
        or abs(metrics["strict"]["target_bytes"] - metrics["strict"]["candidate_bytes"])
        > abs(base_metrics["strict"]["target_bytes"] - base_metrics["strict"]["candidate_bytes"])
        or abs(metrics["data"]["target_bytes"] - metrics["data"]["candidate_bytes"])
        > abs(base_metrics["data"]["target_bytes"] - base_metrics["data"]["candidate_bytes"])
        or metrics["physical_differences"] > base_metrics["physical_differences"]
        or abs(metrics["physical_target_count"] - metrics["physical_candidate_count"])
        > abs(base_metrics["physical_target_count"] - base_metrics["physical_candidate_count"])
    ):
        return "no_gain"
    if not any(improvements):
        return "no_gain"
    return "exact" if _is_exact(metrics) else "improved"


def _frontier_focus(
    root: Path, campaign: Mapping[str, Any], frontier: Mapping[str, Any]
) -> dict[str, Any]:
    digest = frontier["focus_evidence_sha256"]
    path = (
        _state_root(root) / "proof-cas" / "focus" / digest[:2]
        / f"{digest}.json"
    )
    focus = _validate_focus_evidence(
        _read_json(path, "frontier focus evidence"), campaign,
        frontier["function"], frontier["source_sha256"],
    )
    if focus["focus_evidence_sha256"] != digest:
        raise CampaignError("frontier focus evidence CAS binding drift")
    return focus


def _frontier_reconstruction(
    root: Path, frontier: Mapping[str, Any]
) -> dict[str, Any] | None:
    digest = frontier.get("reconstruction_evidence_sha256")
    if digest is None:
        return None
    path = (
        _state_root(root) / "proof-cas" / "reconstruction" / digest[:2]
        / f"{digest}.json"
    )
    evidence = _read_json(path, "frontier reconstruction evidence")
    try:
        from tools.owner_campaign_reconstruction import (
            ReconstructionPacketError, verify_packet,
        )

        verify_packet(evidence)
    except ReconstructionPacketError as exc:
        raise CampaignError(
            f"frontier reconstruction evidence is invalid: {exc}"
        ) from exc
    if evidence.get("packet_sha256") != digest:
        raise CampaignError("frontier reconstruction evidence CAS binding drift")
    expected = {
        "owner": frontier["owner"], "unit": frontier["unit"],
        "function": frontier["function"],
        "source_path": frontier["source_relpath"],
        "source_sha256": frontier["source_sha256"],
        "frontier_source_sha256": frontier["source_sha256"],
        "target_object_sha256": frontier["target_object_sha256"],
        "candidate_object_sha256": frontier["candidate_object_sha256"],
        "toolchain_sha256": frontier["toolchain_sha256"],
        "status": frontier["reconstruction_status"],
    }
    if any(evidence.get(key) != value for key, value in expected.items()):
        raise CampaignError("frontier reconstruction evidence identity drift")
    return dict(evidence)


FRONTIER_FIELDS = {
    "schema", "campaign_id", "manifest_sha256", "owner", "unit", "function",
    "source_relpath", "source_sha256", "target_object_sha256", "toolchain_sha256",
    "candidate_object_sha256", "metrics", "report_receipts",
    "focus_evidence_sha256", "parent_frontier_sha256",
    "generation", "retained_at", "frontier_sha256",
}
FRONTIER_OPTIONAL_FIELDS = {
    "reconstruction_evidence_sha256", "reconstruction_status",
}


def _frontier_from_measurement(
    campaign: Mapping[str, Any], function: str, measurement: Mapping[str, Any],
    *, parent: Mapping[str, Any] | None,
) -> dict[str, Any]:
    body = {
        "schema": FRONTIER_SCHEMA, "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"], "owner": campaign["owner"],
        "unit": campaign["unit"], "function": function,
        "source_relpath": campaign["source_relpath"],
        "source_sha256": measurement["source_sha256"],
        "target_object_sha256": measurement["target_object_sha256"],
        "toolchain_sha256": measurement["toolchain_sha256"],
        "candidate_object_sha256": measurement["candidate_object_sha256"],
        "metrics": measurement["metrics"], "report_receipts": measurement["report_receipts"],
        "focus_evidence_sha256": measurement["focus_evidence"]["focus_evidence_sha256"],
        "parent_frontier_sha256": parent["frontier_sha256"] if parent else None,
        "generation": parent["generation"] + 1 if parent else 0,
        "retained_at": _now(),
    }
    reconstruction = measurement.get("reconstruction_evidence")
    if isinstance(reconstruction, Mapping):
        body["reconstruction_evidence_sha256"] = reconstruction["packet_sha256"]
        body["reconstruction_status"] = reconstruction["status"]
    return {**body, "frontier_sha256": _digest_json(body)}


def _validate_frontier(value: Any, campaign: Mapping[str, Any], function: str) -> dict[str, Any]:
    observed_fields = set(value) if isinstance(value, Mapping) else set()
    if (
        not FRONTIER_FIELDS <= observed_fields
        or observed_fields - FRONTIER_FIELDS > FRONTIER_OPTIONAL_FIELDS
    ):
        raise CampaignError("frontier has noncanonical fields")
    value = _closed_keys(value, observed_fields, "frontier")
    body = dict(value)
    digest = _sha(body.pop("frontier_sha256", None), "frontier_sha256")
    if digest != _digest_json(body):
        raise CampaignError("frontier digest is invalid")
    expected = {
        "schema": FRONTIER_SCHEMA, "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"], "owner": campaign["owner"],
        "unit": campaign["unit"], "function": function,
        "source_relpath": campaign["source_relpath"],
        "target_object_sha256": campaign["target_object"]["sha256"],
        "toolchain_sha256": campaign["toolchain"]["sha256"],
    }
    if any(value.get(key) != expected_value for key, expected_value in expected.items()):
        raise CampaignError("frontier identity mismatch")
    _sha(value["source_sha256"], "frontier source_sha256")
    _sha(value["candidate_object_sha256"], "frontier candidate_object_sha256")
    _sha(value["focus_evidence_sha256"], "focus_evidence_sha256")
    if "reconstruction_evidence_sha256" in value:
        _sha(
            value["reconstruction_evidence_sha256"],
            "reconstruction_evidence_sha256",
        )
        if value.get("reconstruction_status") not in {"READY", "UNKNOWN"}:
            raise CampaignError("frontier reconstruction_status is invalid")
    elif "reconstruction_status" in value:
        raise CampaignError("frontier reconstruction status lacks evidence")
    metrics = _validate_metrics(value["metrics"])
    protected_siblings = _protected_sibling_functions(campaign, function)
    if metrics["protected_total"] != len(protected_siblings):
        raise CampaignError("frontier protected sibling census is incomplete")
    receipts = value["report_receipts"]
    required = {"strict", "data", "physical", "siblings", "source_link"}
    if not isinstance(receipts, Mapping) or not required <= set(receipts) or len(receipts) > 16:
        raise CampaignError("frontier proof receipts are incomplete")
    for name, receipt in receipts.items():
        _sha(receipt, f"frontier report_receipts.{name}")
    if value["parent_frontier_sha256"] is not None:
        _sha(value["parent_frontier_sha256"], "parent_frontier_sha256")
    if type(value["generation"]) is not int or value["generation"] < 0:
        raise CampaignError("frontier generation is invalid")
    return dict(value)


def _read_latest_frontier(
    root: Path, campaign: Mapping[str, Any], function: str,
) -> dict[str, Any] | None:
    path = _function_root(root, campaign, function) / "latest-frontier.json"
    if not path.is_file():
        return None
    return _validate_frontier(
        _read_json(path, "latest frontier"), campaign, function
    )


def _recover_pending_locked(
    root: Path, campaign: Mapping[str, Any], function: str,
) -> dict[str, Any] | None:
    """Recover a source-CAS interruption while the canonical locks are held.

    A pending frontier is allowed to complete only when the live source still
    identifies that pending write.  If another writer already retained a
    frontier, that frontier wins and the stale pending record is discarded; a
    recovery must never overwrite a newer retained state.
    """

    directory = _function_root(root, campaign, function)
    pending = directory / "frontier.pending.json"
    if not pending.exists():
        return None
    value = _read_json(pending, "pending frontier")
    fields = {
        "schema", "base_source_sha256", "candidate_source_sha256", "frontier",
        "exact_report", "final_owner_receipt", "pending_sha256",
    }
    value = _closed_keys(value, fields, "pending frontier")
    body = dict(value)
    digest = body.pop("pending_sha256", None)
    if digest != _digest_json(body) or value["schema"] != PENDING_SCHEMA:
        raise CampaignError("pending frontier digest is invalid")
    frontier = _validate_frontier(value["frontier"], campaign, function)
    live = _digest_file(campaign["_source"])
    latest = _read_latest_frontier(root, campaign, function)
    if latest is not None and latest["frontier_sha256"] != frontier["frontier_sha256"]:
        # A different writer has already published a frontier.  Never replace
        # it with this pending record, even if both happen to use the same
        # source bytes; the publication order is the authority.
        if latest["generation"] >= frontier["generation"]:
            pending.unlink()
            return latest
    if live == value["base_source_sha256"]:
        pending.unlink()
        return latest
    if live != value["candidate_source_sha256"] or frontier["source_sha256"] != live:
        raise CampaignError("pending frontier cannot be reconciled with live source")
    if value["exact_report"] is not None:
        # The frontier and report were prepared before the interruption, but
        # their separately published focus evidence is still part of the
        # exact proof.  Revalidate that CAS dependency before making either
        # the frontier or exact manifest authoritative.
        focus = _frontier_focus(root, campaign, frontier)
        try:
            from tools.owner_campaign_verify import VerificationError, verify_report

            verify_report(
                value["exact_report"],
                focus_evidence=focus,
                expected={
                    "owner": campaign["owner"],
                    "function": function,
                    "campaign_id": campaign["campaign_id"],
                    "manifest_sha256": campaign["manifest_sha256"],
                    "unit": campaign["unit"],
                    "source_path": campaign["source_relpath"],
                    "base_commit": campaign["base_commit"],
                    "source_sha256": frontier["source_sha256"],
                    "target_object_sha256": campaign["target_object"]["sha256"],
                    "candidate_object_sha256": frontier["candidate_object_sha256"],
                    "toolchain_sha256": campaign["toolchain"]["sha256"],
                },
            )
        except VerificationError as exc:
            raise CampaignError(
                f"pending exact report independent verification failed: {exc}"
            ) from exc
    _atomic_json(directory / "latest-frontier.json", frontier, limit=campaign["limits"]["frontier_bytes"])
    if value["exact_report"] is not None:
        _publish_exact(
            root, campaign, frontier, value["exact_report"],
            final_owner_receipt=value["final_owner_receipt"],
        )
    pending.unlink()
    return frontier


def _recover_pending(root: Path, campaign: Mapping[str, Any], function: str) -> dict[str, Any] | None:
    """Recover pending state under source→function→focus CAS locks."""

    with _frontier_lock_chain(root, campaign, function):
        return _recover_pending_locked(root, campaign, function)


def snapshot_frontier(
    root: Path, campaign: Mapping[str, Any], function: str, *, force: bool = False,
    worker: int = 0, _defer_maintenance: bool = False,
) -> dict[str, Any]:
    if campaign.get("_retained_frontier_read_only") is True:
        raise CampaignError(
            "read-only retained frontier campaign cannot snapshot"
        )
    _check_cancelled(root, campaign)
    if function not in campaign["functions"]:
        raise CampaignError(f"function is outside campaign scope: {function}")
    if type(worker) is not int or not 0 <= worker < 5:
        raise CampaignError("snapshot worker index must be between 0 and 4")

    # Establish a versioned read before doing the expensive measurement.  A
    # later writer may advance either source or latest while the hook runs;
    # the second locked read below treats that as a stale snapshot.
    with _frontier_lock_chain(root, campaign, function):
        _recover_pending_locked(root, campaign, function)
        initial_frontier = _read_latest_frontier(root, campaign, function)
        live_sha = _digest_file(campaign["_source"])
        if initial_frontier is not None:
            # A retained gain in another function advances the shared TU
            # source.  That makes this function's last measured frontier
            # stale, not corrupt.  Refresh it from the new live source instead
            # of forcing every parallel worker through a serialized manual
            # re-baseline step.  A frontier already bound to the live source
            # remains the fast path.
            if initial_frontier["source_sha256"] == live_sha and not force:
                return initial_frontier
        initial_frontier_sha = (
            initial_frontier["frontier_sha256"] if initial_frontier is not None else None
        )

    def measure_on_worker() -> tuple[Path, dict[str, Any]]:
        scratch = _ensure_scratch(root, campaign, worker)
        live_bytes = campaign["_source"].read_bytes()
        _sync_scratch_source(root, scratch, campaign, live_bytes)
        try:
            measurement = _run_hook(
                root, scratch, campaign, function, live_sha, "snapshot"
            )
            _verify_publication_sources(
                campaign, scratch, live_sha256=live_sha, scratch_sha256=live_sha,
            )
        finally:
            _cleanup_cell_outputs(scratch, campaign)
        return scratch, measurement

    if _SCRATCH_LEASE_HELD.get():
        scratch, measurement = measure_on_worker()
    else:
        with _scratch_lease(root, campaign, worker):
            scratch, measurement = measure_on_worker()
    frontier = _frontier_from_measurement(
        campaign, function, measurement, parent=initial_frontier
    )

    with _frontier_lock_chain(root, campaign, function):
        _recover_pending_locked(root, campaign, function)
        current_frontier = _read_latest_frontier(root, campaign, function)
        locked_live_sha = _digest_file(campaign["_source"])
        latest = _function_root(root, campaign, function) / "latest-frontier.json"

        if current_frontier is not None:
            if current_frontier["frontier_sha256"] != initial_frontier_sha:
                # Another snapshot/retention won while this measurement was
                # running.  It is usable only when it describes the source
                # that is live now; otherwise neither result may overwrite
                # the newer publication.
                if current_frontier["source_sha256"] == locked_live_sha:
                    return current_frontier
                raise CampaignError(
                    "frontier advanced without matching the live source"
                )
            if locked_live_sha != live_sha:
                raise CampaignError("frontier snapshot became stale before publication")
        elif initial_frontier_sha is not None:
            raise CampaignError("latest frontier disappeared during snapshot")
        if locked_live_sha != live_sha:
            raise CampaignError("frontier snapshot became stale before publication")
        _verify_publication_sources(
            campaign, scratch, live_sha256=live_sha, scratch_sha256=live_sha,
        )
        _ensure_state_write_peak(
            root, campaign,
            [(latest, _canonical(frontier) + b"\n")],
        )
        _publish_focus_evidence(root, campaign, measurement["focus_evidence"])
        if isinstance(measurement.get("reconstruction_evidence"), Mapping):
            _publish_reconstruction_evidence(
                root, campaign, measurement["reconstruction_evidence"]
            )
        _atomic_json(latest, frontier, limit=campaign["limits"]["frontier_bytes"])
    if not _defer_maintenance:
        _check_limits(root, campaign)
    return frontier


def snapshot_frontiers(
    root: Path, campaign: Mapping[str, Any], functions: Sequence[str], *,
    force: bool = False, workers: int | None = None,
) -> dict[str, dict[str, Any]]:
    """Prepare each unique function baseline with up to five isolated workers.

    Function order is preserved for deterministic result mapping.  Any worker
    failure propagates after the bounded executor joins; no partial mapping is
    returned to candidate arbitration.
    """

    unique = list(dict.fromkeys(functions))
    for function in unique:
        if not isinstance(function, str) or not function:
            raise CampaignError("snapshot function identity is invalid")
        if function not in campaign["functions"]:
            raise CampaignError(f"function is outside campaign scope: {function}")
    if workers is not None and (type(workers) is not int or not 1 <= workers <= 5):
        raise CampaignError("snapshot worker count is outside 1..5")
    if not unique:
        return {}
    worker_count = min(5 if workers is None else workers, len(unique))

    pending: Queue[tuple[int, str]] = Queue(maxsize=len(unique))
    for index, function in enumerate(unique):
        pending.put_nowait((index, function))
    indexed: dict[str, dict[str, Any]] = {}
    worker_errors: list[tuple[int, BaseException]] = []
    outcome_lock = threading.Lock()
    stop = threading.Event()

    def run_worker(worker: int) -> None:
        while not stop.is_set():
            try:
                index, function = pending.get_nowait()
            except Empty:
                return
            try:
                if stop.is_set():
                    return
                _check_cancelled(root, campaign)
                frontier = snapshot_frontier(
                    root, campaign, function, force=force, worker=worker,
                    _defer_maintenance=True,
                )
                with outcome_lock:
                    indexed[function] = frontier
            except BaseException as exc:
                with outcome_lock:
                    worker_errors.append((index, exc))
                stop.set()
                return
            finally:
                pending.task_done()

    primary_error: BaseException | None = None
    try:
        with ThreadPoolExecutor(max_workers=worker_count) as executor:
            list(executor.map(run_worker, range(worker_count)))
        if worker_errors:
            primary_error = min(worker_errors, key=lambda item: item[0])[1]
    except BaseException as exc:
        primary_error = exc
    try:
        _check_limits(root, campaign)
    except BaseException as maintenance_error:
        if primary_error is None:
            raise
        try:
            primary_error.add_note(
                f"snapshot batch maintenance failed: {maintenance_error}"
            )
        except AttributeError:
            pass
    if primary_error is not None:
        raise primary_error
    return {function: indexed[function] for function in unique}


CANDIDATE_FIELDS = {
    "schema", "campaign_id", "function", "base_frontier_sha256",
    "base_source", "candidate_source", "function_span", "hypothesis_family",
    "natural_c", "rebase_depth", "created_at", "candidate_sha256",
}
# Candidate descriptors are deliberately closed, but the adjacent-helper
# scope is an opt-in extension.  Keep the legacy field set stable so old
# descriptors and their tests remain byte-compatible; loaders which accept
# the extension should validate against ``CANDIDATE_ALL_FIELDS``.
CANDIDATE_OPTIONAL_FIELDS = {"candidate_scope"}
CANDIDATE_ALL_FIELDS = CANDIDATE_FIELDS | CANDIDATE_OPTIONAL_FIELDS
FUNCTION_SPAN_FIELDS = {
    "base_start_line", "base_end_line", "candidate_start_line",
    "candidate_end_line", "base_sha256", "candidate_sha256",
}

ADJACENT_HELPER_SCOPE_KIND = "function_plus_adjacent_static_inline"
ADJACENT_HELPER_SCOPE_FIELDS = {
    "kind", "base_source_sha256", "candidate_source_sha256",
    "base_insertion", "helper", "use_sites",
}
ADJACENT_HELPER_INSERTION_FIELDS = {"line", "sha256"}
ADJACENT_HELPER_FIELDS = {"name", "start_line", "end_line", "sha256"}
ADJACENT_HELPER_USE_FIELDS = {"line", "name", "column"}
ADJACENT_HELPER_MAX_HUNKS = 3
ADJACENT_HELPER_MAX_CHANGED_LINES = 80


def _mask_c_source(text: str) -> str:
    """Blank comments and literals while preserving source offsets/newlines."""

    chars = list(text)
    index = 0
    while index < len(chars):
        if chars[index] == "/" and index + 1 < len(chars) and chars[index + 1] == "/":
            chars[index] = chars[index + 1] = " "
            index += 2
            while index < len(chars) and chars[index] not in "\r\n":
                chars[index] = " "
                index += 1
            continue
        if chars[index] == "/" and index + 1 < len(chars) and chars[index + 1] == "*":
            chars[index] = chars[index + 1] = " "
            index += 2
            while index < len(chars):
                if chars[index] == "*" and index + 1 < len(chars) and chars[index + 1] == "/":
                    chars[index] = chars[index + 1] = " "
                    index += 2
                    break
                if chars[index] not in "\r\n":
                    chars[index] = " "
                index += 1
            continue
        if chars[index] in {"\"", "'"}:
            quote = chars[index]
            chars[index] = " "
            index += 1
            while index < len(chars):
                token = chars[index]
                escaped = token == "\\"
                if chars[index] not in "\r\n":
                    chars[index] = " "
                index += 1
                if escaped and index < len(chars):
                    if chars[index] not in "\r\n":
                        chars[index] = " "
                    index += 1
                elif token == quote:
                    break
            continue
        index += 1
    return "".join(chars)


def _balanced_c_delimiter(masked: str, opening: int, left: str, right: str) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        token = masked[index]
        if token == left:
            depth += 1
        elif token == right:
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                return None
    return None


def _find_function_span(
    text: str, function: str, label: str = "function"
) -> tuple[int, int, bytes]:
    """Find one C function definition and return inclusive line bounds."""

    if not isinstance(function, str) or not function:
        raise CampaignError(f"{label} name is invalid")
    masked = _mask_c_source(text)
    pattern = re.compile(r"\b" + re.escape(function) + r"\s*\(")
    matches: list[tuple[int, int]] = []
    for match in pattern.finditer(masked):
        opening = masked.find("(", match.start(), match.end())
        closing = _balanced_c_delimiter(masked, opening, "(", ")")
        if closing is None:
            continue
        after = closing + 1
        while after < len(masked) and masked[after].isspace():
            after += 1
        if after >= len(masked) or masked[after] != "{":
            continue
        body_close = _balanced_c_delimiter(masked, after, "{", "}")
        if body_close is None:
            raise CampaignError(f"{label} has an unterminated body")
        boundary = max(
            masked.rfind("}", 0, match.start()),
            masked.rfind(";", 0, match.start()),
        )
        first = match.start() if boundary < 0 else boundary + 1
        while first < match.start() and masked[first].isspace():
            first += 1
        start_offset = text.rfind("\n", 0, first) + 1
        start_line = text.count("\n", 0, start_offset) + 1
        end_line = text.count("\n", 0, body_close) + 1
        matches.append((start_line, end_line))
    if len(matches) != 1:
        if not matches:
            raise CampaignError(f"{label} definition is not found")
        raise CampaignError(f"{label} definition is ambiguous")
    start_line, end_line = matches[0]
    lines = text.splitlines(keepends=True)
    return start_line, end_line, "".join(lines[start_line - 1:end_line]).encode("utf-8")


def _line_column(text: str, offset: int) -> tuple[int, int]:
    line = text.count("\n", 0, offset) + 1
    prior = text.rfind("\n", 0, offset)
    return line, offset - prior - 1


def _brace_depth(masked: str, offset: int) -> int:
    return masked[:offset].count("{") - masked[:offset].count("}")


def validate_candidate_scope(
    *,
    base_text: str,
    candidate_text: str,
    function: str,
    base_start_line: int,
    base_end_line: int,
    candidate_start_line: int,
    candidate_end_line: int,
    base_source_sha256: str,
    candidate_source_sha256: str,
    scope: Mapping[str, Any],
    forbidden_constructs: Sequence[str] = (),
) -> dict[str, Any]:
    """Validate the opt-in one-helper candidate scope.

    The helper is an intentionally tiny source extension: it is inserted at
    the exact beginning of the target function, remains file-scope, and every
    reference outside its own definition must be in that function.  All
    checks are structural and hash-bound; this routine does not infer a
    compiler result or grant retention authority.
    """

    if not isinstance(scope, Mapping):
        raise CampaignError("candidate scope is invalid")
    if set(scope) != ADJACENT_HELPER_SCOPE_FIELDS:
        raise CampaignError("candidate scope fields are not closed")
    if scope.get("kind") != ADJACENT_HELPER_SCOPE_KIND:
        raise CampaignError("candidate scope kind is invalid")
    if not isinstance(base_text, str) or not isinstance(candidate_text, str):
        raise CampaignError("candidate scope sources are invalid")
    if not isinstance(function, str) or not function:
        raise CampaignError("candidate scope function is invalid")
    if any(
        type(value) is not int
        for value in (
            base_start_line,
            base_end_line,
            candidate_start_line,
            candidate_end_line,
        )
    ):
        raise CampaignError("candidate scope line binding is invalid")
    if (
        base_start_line < 1
        or base_end_line < base_start_line
        or candidate_start_line < 1
        or candidate_end_line < candidate_start_line
    ):
        raise CampaignError("candidate scope line binding is invalid")
    if _digest_bytes(base_text.encode("utf-8")) != _sha(
        base_source_sha256, "candidate scope base source sha256"
    ):
        raise CampaignError("candidate scope base source hash drift")
    if _digest_bytes(candidate_text.encode("utf-8")) != _sha(
        candidate_source_sha256, "candidate scope candidate source sha256"
    ):
        raise CampaignError("candidate scope candidate source hash drift")

    insertion = scope.get("base_insertion")
    if not isinstance(insertion, Mapping) or set(insertion) != ADJACENT_HELPER_INSERTION_FIELDS:
        raise CampaignError("candidate scope base insertion is invalid")
    if insertion.get("line") != base_start_line:
        raise CampaignError("candidate scope insertion is not at the function boundary")
    if _sha(insertion.get("sha256"), "candidate scope insertion sha256") != _digest_bytes(b""):
        raise CampaignError("candidate scope base insertion is not zero-width")

    helper = scope.get("helper")
    if not isinstance(helper, Mapping) or set(helper) != ADJACENT_HELPER_FIELDS:
        raise CampaignError("candidate scope helper binding is invalid")
    name = helper.get("name")
    if not isinstance(name, str) or re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name) is None:
        raise CampaignError("candidate scope helper name is invalid")
    helper_start = helper.get("start_line")
    helper_end = helper.get("end_line")
    if (
        type(helper_start) is not int
        or type(helper_end) is not int
        or helper_start != base_start_line
        or helper_end < helper_start
        or candidate_start_line <= helper_end
    ):
        raise CampaignError("candidate scope helper is not immediately adjacent")
    candidate_lines = candidate_text.splitlines(keepends=True)
    base_lines = base_text.splitlines(keepends=True)
    if (
        base_start_line < 1
        or base_end_line < base_start_line
        or base_end_line > len(base_lines)
        or candidate_start_line < 1
        or candidate_end_line < candidate_start_line
        or candidate_end_line > len(candidate_lines)
        or helper_end > len(candidate_lines)
    ):
        raise CampaignError("candidate scope line binding is invalid")
    helper_bytes = "".join(candidate_lines[helper_start - 1:helper_end]).encode("utf-8")
    if _digest_bytes(helper_bytes) != _sha(helper.get("sha256"), "candidate scope helper sha256"):
        raise CampaignError("candidate scope helper hash drift")
    if base_lines[:base_start_line - 1] != candidate_lines[:helper_start - 1]:
        raise CampaignError("candidate scope insertion changes the TU prefix")
    if base_lines[base_end_line:] != candidate_lines[candidate_end_line:]:
        raise CampaignError("candidate scope changes the TU suffix")
    if any(line.strip() for line in candidate_lines[helper_end:candidate_start_line - 1]):
        raise CampaignError("candidate scope helper is not immediately adjacent")

    base_masked = _mask_c_source(base_text)
    candidate_masked = _mask_c_source(candidate_text)
    helper_start_offset = sum(len(line) for line in candidate_lines[:helper_start - 1])
    helper_end_offset = sum(len(line) for line in candidate_lines[:helper_end])
    candidate_function_start_offset = sum(
        len(line) for line in candidate_lines[:candidate_start_line - 1]
    )
    candidate_function_end_offset = sum(
        len(line) for line in candidate_lines[:candidate_end_line]
    )
    if _brace_depth(candidate_masked, helper_start_offset) != 0:
        raise CampaignError("candidate scope helper is not file-scope")
    helper_masked = candidate_masked[helper_start_offset:helper_end_offset]
    if re.search(r"#|\b(?:asm|__asm|volatile|register|__attribute__)\b", helper_masked):
        raise CampaignError("candidate scope helper contains a forbidden construct")
    inline_defs = list(re.finditer(r"\bstatic\s+inline\b", helper_masked))
    if len(inline_defs) != 1:
        raise CampaignError("candidate scope must add exactly one static inline helper")
    helper_definition = re.search(
        r"\bstatic\s+inline\b[\s\S]*?\b" + re.escape(name) + r"\s*\(",
        helper_masked,
    )
    if helper_definition is None:
        raise CampaignError("candidate scope helper definition is not static inline")
    if helper_masked[:helper_definition.start()].strip():
        raise CampaignError("candidate scope helper span contains extra code")
    opening = helper_masked.find("(", helper_definition.start(), helper_definition.end())
    closing = _balanced_c_delimiter(helper_masked, opening, "(", ")")
    if closing is None:
        raise CampaignError("candidate scope helper signature is unterminated")
    body_open = closing + 1
    while body_open < len(helper_masked) and helper_masked[body_open].isspace():
        body_open += 1
    if body_open >= len(helper_masked) or helper_masked[body_open] != "{":
        raise CampaignError("candidate scope helper has no function body")
    body_close = _balanced_c_delimiter(helper_masked, body_open, "{", "}")
    if body_close is None or helper_masked[body_close + 1:].strip():
        raise CampaignError("candidate scope helper span contains extra code")
    if base_masked and re.search(r"\b" + re.escape(name) + r"\b", base_masked):
        raise CampaignError("candidate scope helper name already exists in the base")
    for pattern in forbidden_constructs:
        try:
            if re.search(pattern, helper_bytes.decode("utf-8")):
                raise CampaignError(f"candidate contains forbidden construct: {pattern}")
        except re.error as exc:
            raise CampaignError(f"invalid forbidden construct regex: {pattern}") from exc

    name_matches = list(re.finditer(r"\b" + re.escape(name) + r"\b", candidate_masked))
    definition_abs = helper_start_offset + helper_definition.start()
    definition_name = candidate_masked.find(name, definition_abs, helper_end_offset)
    if definition_name < 0:
        raise CampaignError("candidate scope helper definition name is missing")
    use_matches: list[tuple[int, int]] = []
    for match in name_matches:
        offset = match.start()
        if helper_start_offset <= offset < helper_end_offset:
            if offset != definition_name:
                raise CampaignError("candidate scope helper has an extra self-reference")
            continue
        if not candidate_function_start_offset <= offset < candidate_function_end_offset:
            raise CampaignError("candidate scope helper use escapes the target function")
        use_matches.append(_line_column(candidate_text, offset))
    if not use_matches:
        raise CampaignError("candidate scope helper has no target-function use")
    raw_sites = scope.get("use_sites")
    if not isinstance(raw_sites, list) or not raw_sites:
        raise CampaignError("candidate scope use_sites is invalid")
    normalized_sites: list[dict[str, Any]] = []
    for raw in raw_sites:
        if not isinstance(raw, Mapping):
            raise CampaignError("candidate scope use site is invalid")
        keys = set(raw)
        if keys != ADJACENT_HELPER_USE_FIELDS:
            raise CampaignError("candidate scope use site fields are invalid")
        line = raw.get("line")
        if type(line) is not int or line < candidate_start_line or line > candidate_end_line:
            raise CampaignError("candidate scope use site line is invalid")
        if raw.get("name") != name:
            raise CampaignError("candidate scope use site name is invalid")
        column = raw.get("column")
        if type(column) is not int or column < 0:
            raise CampaignError("candidate scope use site column is invalid")
        item: dict[str, Any] = {"line": line, "name": name, "column": column}
        normalized_sites.append(item)
    actual_sites = sorted(use_matches)
    supplied_sites = sorted((item["line"], item["column"]) for item in normalized_sites)
    if supplied_sites != actual_sites:
        raise CampaignError("candidate scope use sites do not match source uses")

    changes = [
        opcode for opcode in difflib.SequenceMatcher(
            a=base_lines, b=candidate_lines, autojunk=False
        ).get_opcodes()
        if opcode[0] != "equal"
    ]
    if not changes or len(changes) > ADJACENT_HELPER_MAX_HUNKS:
        raise CampaignError("candidate adjacent-helper edit exceeds three hunks")
    changed_lines = sum(max(opcode[2] - opcode[1], opcode[4] - opcode[3]) for opcode in changes)
    if changed_lines > ADJACENT_HELPER_MAX_CHANGED_LINES:
        raise CampaignError("candidate adjacent-helper edit exceeds changed-line limit")
    for _tag, base_a, base_b, candidate_a, candidate_b in changes:
        base_allowed = (
            base_start_line - 1 <= base_a <= base_b <= base_end_line
            or (base_a == base_start_line - 1 and base_b == base_a)
        )
        candidate_allowed = (
            candidate_start_line - 1 <= candidate_a <= candidate_b <= candidate_end_line
            or (candidate_a >= helper_start - 1 and candidate_b <= candidate_end_line)
        )
        if not base_allowed or not candidate_allowed:
            raise CampaignError("candidate adjacent-helper edit escapes its bound scope")
    return {
        "kind": ADJACENT_HELPER_SCOPE_KIND,
        "base_source_sha256": base_source_sha256,
        "candidate_source_sha256": candidate_source_sha256,
        "base_insertion": {"line": base_start_line, "sha256": _digest_bytes(b"")},
        "helper": {
            "name": name,
            "start_line": helper_start,
            "end_line": helper_end,
            "sha256": _digest_bytes(helper_bytes),
        },
        "use_sites": normalized_sites,
    }


def _load_candidate(
    root: Path, path: Path, campaign: Mapping[str, Any], frontier: Mapping[str, Any],
) -> dict[str, Any]:
    path = _bound_path(root, str(path), "candidate descriptor")
    raw = _read_json(path, "candidate descriptor")
    observed_fields = set(raw) if isinstance(raw, Mapping) else set()
    if (
        not CANDIDATE_FIELDS <= observed_fields
        or observed_fields - CANDIDATE_FIELDS > CANDIDATE_OPTIONAL_FIELDS
    ):
        raise CampaignError("candidate descriptor is not a strict closed object")
    value = _closed_keys(raw, observed_fields, "candidate descriptor")
    body = dict(value)
    digest = _sha(body.pop("candidate_sha256", None), "candidate descriptor digest")
    if digest != _digest_json(body):
        raise CampaignError("candidate descriptor digest is invalid")
    if type(value["rebase_depth"]) is not int or not 0 <= value["rebase_depth"] <= 5:
        raise CampaignError("candidate rebase_depth must be between 0 and 5")
    if (
        value["schema"] != CANDIDATE_SCHEMA
        or value["campaign_id"] != campaign["campaign_id"]
        or value["function"] != frontier["function"]
        or value["base_frontier_sha256"] != frontier["frontier_sha256"]
        or value["natural_c"] is not True
        or not isinstance(value["hypothesis_family"], str)
        or not value["hypothesis_family"]
    ):
        raise CampaignError("candidate descriptor is not frontier-bound")
    if _digest_file(campaign["_source"]) != frontier["source_sha256"]:
        raise CampaignError("live source no longer matches the current frontier")
    base_binding = _closed_keys(
        value["base_source"], {"path", "sha256"}, "candidate base source"
    )
    base_source = _bound_path(root, base_binding["path"], "candidate base source")
    allowed_build_roots = [
        _bound_path(root, item, "allowed candidate base path", exists=False)
        for item in campaign["allowed_build_paths"]
    ]
    if not any(
        base_source == allowed or _inside(allowed, base_source)
        for allowed in allowed_build_roots
    ):
        raise CampaignError("candidate base source is outside campaign allowed build paths")
    base_source_sha = _sha(
        base_binding["sha256"], "candidate base source sha256"
    )
    if base_source_sha != frontier["source_sha256"]:
        raise CampaignError("candidate base source does not match the frontier")
    if _digest_file(base_source) != base_source_sha:
        raise CampaignError("candidate base source hash drift")
    base_bytes = base_source.read_bytes()
    binding = _closed_keys(value["candidate_source"], {"path", "sha256"}, "candidate source")
    source = _bound_path(root, binding["path"], "candidate source")
    allowed_roots = [
        _bound_path(root, item, "allowed candidate path", exists=False)
        for item in [*campaign["allowed_source_paths"], *campaign["allowed_build_paths"]]
    ]
    if not any(source == allowed or _inside(allowed, source) for allowed in allowed_roots):
        raise CampaignError("candidate source is outside campaign allowed paths")
    source_sha = _sha(binding["sha256"], "candidate source sha256")
    if _digest_file(source) != source_sha:
        raise CampaignError("candidate source hash drift")
    candidate_bytes = source.read_bytes()
    try:
        candidate_text = candidate_bytes.decode("utf-8")
        base_text = base_bytes.decode("utf-8")
    except UnicodeError as exc:
        raise CampaignError("candidate source is not UTF-8 natural C") from exc
    if "\x00" in candidate_text:
        raise CampaignError("candidate source contains NUL")
    span = _closed_keys(value["function_span"], FUNCTION_SPAN_FIELDS, "function span")
    base_lines = base_text.splitlines(keepends=True)
    candidate_lines = candidate_text.splitlines(keepends=True)
    coordinates: list[int] = []
    for field in (
        "base_start_line", "base_end_line", "candidate_start_line",
        "candidate_end_line",
    ):
        coordinate = span[field]
        if type(coordinate) is not int or coordinate < 1:
            raise CampaignError(f"function span {field} is invalid")
        coordinates.append(coordinate)
    base_start, base_end, candidate_start, candidate_end = coordinates
    if base_start > base_end or base_end > len(base_lines):
        raise CampaignError("base function span is outside the live source")
    if candidate_start > candidate_end or candidate_end > len(candidate_lines):
        raise CampaignError("candidate function span is outside the candidate source")
    base_span = b"".join(
        line.encode("utf-8") for line in base_lines[base_start - 1:base_end]
    )
    candidate_span = b"".join(
        line.encode("utf-8")
        for line in candidate_lines[candidate_start - 1:candidate_end]
    )
    if _digest_bytes(base_span) != _sha(span["base_sha256"], "base span sha256"):
        raise CampaignError("base function span hash drift")
    if _digest_bytes(candidate_span) != _sha(
        span["candidate_sha256"], "candidate span sha256"
    ):
        raise CampaignError("candidate function span hash drift")
    validated_scope: dict[str, Any] | None = None
    if "candidate_scope" in value:
        validated_scope = validate_candidate_scope(
            base_text=base_text,
            candidate_text=candidate_text,
            function=value["function"],
            base_start_line=base_start,
            base_end_line=base_end,
            candidate_start_line=candidate_start,
            candidate_end_line=candidate_end,
            base_source_sha256=base_source_sha,
            candidate_source_sha256=source_sha,
            scope=value["candidate_scope"],
            forbidden_constructs=campaign["forbidden_constructs"],
        )
        if validated_scope != value["candidate_scope"]:
            raise CampaignError("candidate scope is not canonical")
    elif (
        base_lines[: base_start - 1] != candidate_lines[: candidate_start - 1]
        or base_lines[base_end:] != candidate_lines[candidate_end:]
    ):
        raise CampaignError("candidate edits escape the claimed function span")
    matcher = difflib.SequenceMatcher(a=base_lines, b=candidate_lines)
    added: list[str] = []
    changed = False
    for tag, _a0, _a1, b0, b1 in matcher.get_opcodes():
        if tag != "equal":
            if validated_scope is None and (
                _a0 < base_start - 1 or _a1 > base_end
                or b0 < candidate_start - 1 or b1 > candidate_end
            ):
                raise CampaignError("candidate edit crosses the claimed function span")
            changed = True
            added.extend(candidate_lines[b0:b1])
    if not changed:
        raise CampaignError("candidate source is byte-identical to the frontier")
    added_text = "\n".join(added)
    for pattern in campaign["forbidden_constructs"]:
        try:
            matched = re.search(pattern, added_text)
        except re.error as exc:
            raise CampaignError(f"invalid forbidden construct regex: {pattern}: {exc}") from exc
        if matched:
            raise CampaignError(f"candidate contains forbidden construct: {pattern}")
    result = dict(value)
    result["_path"] = path
    result["_source"] = source
    result["_source_sha256"] = source_sha
    result["_source_bytes"] = candidate_bytes
    result["_base_source"] = base_source
    result["_base_source_sha256"] = base_source_sha
    result["_base_source_bytes"] = base_bytes
    return result


def _cleanup_candidate_artifacts(
    root: Path, campaign: Mapping[str, Any], paths: Sequence[Path]
) -> None:
    """Delete terminal cell inputs only when they are under an allowed build root."""

    build_roots = [
        _bound_path(root, item, "allowed build cleanup root", exists=False)
        for item in campaign["allowed_build_paths"]
    ]
    for raw in paths:
        path = _bound_path(root, str(raw), "candidate cleanup path", exists=False)
        if path.is_file() and any(path == base or _inside(base, path) for base in build_roots):
            path.unlink()


def _candidate_key(campaign: Mapping[str, Any], frontier: Mapping[str, Any], candidate: Mapping[str, Any]) -> str:
    return _digest_json({
        "campaign": campaign["manifest_sha256"], "function": frontier["function"],
        "frontier": frontier["frontier_sha256"], "candidate": candidate["_source_sha256"],
        "target": campaign["target_object"]["sha256"], "toolchain": campaign["toolchain"]["sha256"],
        "unit": campaign["unit"],
    })


def _dedupe_path(root: Path, campaign: Mapping[str, Any], function: str) -> Path:
    return _function_root(root, campaign, function) / "candidate-results.jsonl"


DEDUPE_FIELDS = {
    "schema", "candidate_key", "function", "base_frontier_sha256",
    "candidate_source_sha256", "status", "strict_difference_delta",
    "data_difference_delta", "physical_difference_delta", "finished_at",
    "result_sha256",
}


def _validate_dedupe_record(value: Any) -> dict[str, Any]:
    value = _closed_keys(value, DEDUPE_FIELDS, "candidate dedupe record")
    body = dict(value)
    digest = _sha(body.pop("result_sha256", None), "dedupe result_sha256")
    if digest != _digest_json(body):
        raise CampaignError("candidate dedupe record digest is invalid")
    if (
        value["schema"] != DEDUPE_SCHEMA
        or value["status"] not in {
            "inflight", "exact", "improved", "no_gain", "stale"
        }
    ):
        raise CampaignError("candidate dedupe record identity is invalid")
    for field in ("candidate_key", "base_frontier_sha256", "candidate_source_sha256"):
        _sha(value[field], f"dedupe {field}")
    if not isinstance(value["function"], str) or not value["function"]:
        raise CampaignError("candidate dedupe function is invalid")
    for field in (
        "strict_difference_delta", "data_difference_delta",
        "physical_difference_delta",
    ):
        if type(value[field]) is not int:
            raise CampaignError(f"candidate dedupe {field} is invalid")
    _parse_time(value["finished_at"], "candidate dedupe finished_at")
    return dict(value)


def _dedupe_records(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    records: list[dict[str, Any]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        try:
            value = json.loads(line)
        except json.JSONDecodeError as exc:
            raise CampaignError(f"candidate dedupe ledger is corrupt: {path}") from exc
        records.append(_validate_dedupe_record(value))
    keys = [record["candidate_key"] for record in records]
    if len(keys) != len(set(keys)):
        raise CampaignError("candidate dedupe ledger contains duplicate keys")
    return records


def _write_dedupe(
    campaign: Mapping[str, Any], path: Path, records: Sequence[Mapping[str, Any]],
) -> None:
    for record in records:
        _validate_dedupe_record(record)
    if not records:
        # An infrastructure failure is retryable and must leave no durable
        # candidate identity behind.  Removing the empty ledger also makes
        # that property observable across processes instead of encoding it as
        # a zero-byte implementation detail.
        path.unlink(missing_ok=True)
        return
    payload = b"".join(_canonical(record) + b"\n" for record in records)
    if len(payload) > campaign["limits"]["dedupe_bytes"]:
        raise CampaignError("candidate dedupe ledger exceeds per-function limit")
    _atomic_bytes(path, payload)


def _dedupe_record(
    *, key: str, function: str, frontier: Mapping[str, Any],
    candidate_source_sha256: str, status: str,
    strict_delta: int = 0, data_delta: int = 0, physical_delta: int = 0,
) -> dict[str, Any]:
    body = {
        "schema": DEDUPE_SCHEMA, "candidate_key": key, "function": function,
        "base_frontier_sha256": frontier["frontier_sha256"],
        "candidate_source_sha256": candidate_source_sha256, "status": status,
        "strict_difference_delta": strict_delta,
        "data_difference_delta": data_delta,
        "physical_difference_delta": physical_delta, "finished_at": _now(),
    }
    return {**body, "result_sha256": _digest_json(body)}


def _reserve_candidate(
    root: Path, campaign: Mapping[str, Any], function: str, key: str,
    frontier: Mapping[str, Any], candidate_source_sha256: str,
) -> bool:
    path = _dedupe_path(root, campaign, function)
    path.parent.mkdir(parents=True, exist_ok=True)
    lock = path.parent / "dedupe.lock"
    with _exclusive_lock(lock, float(campaign["limits"]["command_timeout_seconds"])):
        records = _dedupe_records(path)
        expiry = dt.timedelta(
            seconds=max(60, 2 * campaign["limits"]["command_timeout_seconds"])
        )
        now = dt.datetime.now(dt.timezone.utc)
        records = [
            record for record in records
            if not (
                record["status"] == "inflight"
                and now - _parse_time(record["finished_at"], "inflight started_at")
                > expiry
            )
        ]
        if any(record["candidate_key"] == key for record in records):
            return False
        records.append(_dedupe_record(
            key=key, function=function, frontier=frontier,
            candidate_source_sha256=candidate_source_sha256, status="inflight",
        ))
        _write_dedupe(campaign, path, records)
        return True


def _finish_candidate_reservation(
    root: Path, campaign: Mapping[str, Any], function: str, key: str,
    record: Mapping[str, Any] | None,
) -> None:
    path = _dedupe_path(root, campaign, function)
    lock = path.parent / "dedupe.lock"
    with _exclusive_lock(lock, float(campaign["limits"]["command_timeout_seconds"])):
        records = _dedupe_records(path)
        matches = [index for index, item in enumerate(records) if item["candidate_key"] == key]
        if len(matches) != 1 or records[matches[0]]["status"] != "inflight":
            raise CampaignError("candidate reservation is missing or terminal")
        if record is None:
            del records[matches[0]]
        else:
            records[matches[0]] = _validate_dedupe_record(record)
        _write_dedupe(campaign, path, records)


def _validate_exact_report(
    value: Any, campaign: Mapping[str, Any], frontier: Mapping[str, Any],
) -> dict[str, Any]:
    fields = {
        "schema", "status", "completed", "authority_advanced", "owner", "function",
        "campaign_id", "manifest_sha256", "unit", "source_path", "base_commit",
        "frontier_sha256", "source_sha256", "target_object_sha256",
        "candidate_object_sha256", "toolchain_sha256", "result", "proof_receipts",
        "evidence", "completed_at", "report_sha256",
    }
    value = _closed_keys(value, fields, "exact report")
    body = dict(value)
    digest = _sha(body.pop("report_sha256", None), "report_sha256")
    if digest != _digest_json(body):
        raise CampaignError("exact report digest is invalid")
    protected_siblings = _protected_sibling_functions(
        campaign, frontier["function"]
    )
    if (
        value["schema"] != REPORT_SCHEMA or value["status"] != "exact"
        or value["completed"] is not True or value["authority_advanced"] is not False
        or value["owner"] != campaign["owner"] or value["function"] != frontier["function"]
        or value["campaign_id"] != campaign["campaign_id"]
        or value["manifest_sha256"] != campaign["manifest_sha256"]
        or value["unit"] != campaign["unit"]
        or value["source_path"] != campaign["source_relpath"]
        or value["base_commit"] != campaign["base_commit"]
        or value["frontier_sha256"] != frontier["frontier_sha256"]
        or value["source_sha256"] != frontier["source_sha256"]
        or value["target_object_sha256"] != campaign["target_object"]["sha256"]
        or value["candidate_object_sha256"] != frontier["candidate_object_sha256"]
        or value["toolchain_sha256"] != campaign["toolchain"]["sha256"]
    ):
        raise CampaignError("exact report binding is invalid")
    result = value["result"]
    if (
        not isinstance(result, Mapping)
        or set(result) != {
            "strict_percent", "data_percent", "target_bytes", "candidate_bytes",
            "strict_difference_count", "data_difference_count",
            "strict_row_ids_sha256", "data_row_ids_sha256",
            "physical_target_count", "physical_candidate_count",
            "physical_difference_count", "physical_difference_ids_sha256",
            "protected_total", "protected_losses", "protected_sibling_digest",
            "source_link_exact",
        }
        or result.get("strict_percent") != 100 or result.get("data_percent") != 100
        or result.get("target_bytes") != result.get("candidate_bytes")
        or result.get("strict_difference_count") != 0
        or result.get("data_difference_count") != 0
        or result.get("physical_difference_count") != 0
        or result.get("physical_target_count") != result.get("physical_candidate_count")
        or result.get("protected_losses") != 0
        or result.get("protected_total") != len(protected_siblings)
        or result.get("source_link_exact") is not True
    ):
        raise CampaignError("exact report does not prove every exactness gate")
    if (
        not isinstance(value["proof_receipts"], Mapping)
        or dict(value["proof_receipts"]) != dict(frontier["report_receipts"])
    ):
        raise CampaignError("exact report proof receipts are incomplete")
    for name, receipt in value["proof_receipts"].items():
        _sha(receipt, f"exact report proof receipt {name}")
    evidence = value["evidence"]
    evidence_fields = {
        "schema", "owner", "function", "unit", "source_path", "base_commit",
        "source_sha256", "target_object_sha256", "candidate_object_sha256",
        "focus_evidence_sha256", "strict_row_count", "strict_row_ids_sha256",
        "data_row_count", "data_row_ids_sha256", "physical_target_count",
        "physical_candidate_count", "physical_difference_count",
        "physical_difference_ids_sha256", "protected_total", "protected_losses",
        "protected_sibling_identities", "protected_sibling_digest", "proofs",
    }
    evidence = _closed_keys(evidence, evidence_fields, "exact report evidence")
    expected_evidence = {
        "schema": "owner_campaign_report_evidence/v1",
        "owner": campaign["owner"], "function": frontier["function"],
        "unit": campaign["unit"], "source_path": campaign["source_relpath"],
        "base_commit": campaign["base_commit"],
        "source_sha256": frontier["source_sha256"],
        "target_object_sha256": campaign["target_object"]["sha256"],
        "candidate_object_sha256": frontier["candidate_object_sha256"],
        "focus_evidence_sha256": frontier["focus_evidence_sha256"],
        "strict_row_count": 0, "data_row_count": 0,
        "physical_difference_count": 0,
        "protected_total": len(protected_siblings),
        "protected_losses": 0,
    }
    if any(evidence.get(key) != expected for key, expected in expected_evidence.items()):
        raise CampaignError("exact report evidence binding is invalid")
    if (
        evidence["physical_target_count"] != evidence["physical_candidate_count"]
        or evidence["strict_row_ids_sha256"] != result["strict_row_ids_sha256"]
        or evidence["data_row_ids_sha256"] != result["data_row_ids_sha256"]
        or evidence["physical_difference_ids_sha256"]
        != result["physical_difference_ids_sha256"]
        or evidence["protected_sibling_digest"]
        != result["protected_sibling_digest"]
    ):
        raise CampaignError("exact report evidence drifts from result")
    if not set(protected_siblings) <= set(
        evidence["protected_sibling_identities"]
    ):
        raise CampaignError("exact report protected sibling identities are incomplete")
    if not isinstance(evidence["proofs"], Mapping) or set(evidence["proofs"]) != {
        "source_link", "object", "toolchain"
    }:
        raise CampaignError("exact report embedded proofs are incomplete")
    return dict(value)


EXACT_MANIFEST_FIELDS = {
    "schema", "campaign_id", "manifest_sha256", "owner", "exact", "total",
    "owner_closure", "updated_at", "exact_manifest_sha256",
}
EXACT_ENTRY_FIELDS = {"source_sha256", "frontier_sha256", "report_sha256"}
FINAL_OWNER_FIELDS = {
    "schema", "campaign_id", "manifest_sha256", "owner", "unit", "source_path",
    "base_commit", "source_sha256",
    "target_object_sha256", "toolchain_sha256", "source_link_exact",
    "protected_exact", "full_owner_exact", "linked_exact", "proof_receipts",
    "source_built_object_sha256", "linked_binary_sha256",
    "linker_input_manifest_sha256", "clean_build", "matching_source",
    "fallback_asm_used", "nonmatching_fallback_linked", "dtk_checksum_exact",
    "completed_at", "final_owner_sha256",
}


def _validate_final_owner_receipt(
    value: Any, campaign: Mapping[str, Any], source_sha256: str,
) -> dict[str, Any]:
    value = _closed_keys(value, FINAL_OWNER_FIELDS, "final owner receipt")
    body = dict(value)
    digest = _sha(body.pop("final_owner_sha256", None), "final_owner_sha256")
    if digest != _digest_json(body):
        raise CampaignError("final owner receipt digest is invalid")
    expected = {
        "schema": "owner_campaign_final_owner/v1",
        "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"], "owner": campaign["owner"],
        "unit": campaign["unit"], "source_sha256": source_sha256,
        "source_path": campaign["source_relpath"],
        "base_commit": campaign["base_commit"],
        "target_object_sha256": campaign["target_object"]["sha256"],
        "toolchain_sha256": campaign["toolchain"]["sha256"],
        "source_link_exact": True, "protected_exact": True,
        "full_owner_exact": True, "linked_exact": True, "clean_build": True,
        "matching_source": True, "fallback_asm_used": False,
        "nonmatching_fallback_linked": False, "dtk_checksum_exact": True,
    }
    if any(value.get(key) != expected_value for key, expected_value in expected.items()):
        raise CampaignError("final owner receipt does not prove owner closure")
    for field in (
        "source_built_object_sha256", "linked_binary_sha256",
        "linker_input_manifest_sha256",
    ):
        _sha(value[field], f"final owner {field}")
    receipts = value["proof_receipts"]
    if (
        not isinstance(receipts, Mapping)
        or not {"source_link", "siblings", "full_owner", "linked"} <= set(receipts)
        or len(receipts) > 16
    ):
        raise CampaignError("final owner proof receipts are incomplete")
    for name, receipt in receipts.items():
        _sha(receipt, f"final owner proof receipt {name}")
    return dict(value)


def _validate_exact_manifest(
    root: Path, campaign: Mapping[str, Any], value: Any,
) -> dict[str, Any]:
    value = _closed_keys(value, EXACT_MANIFEST_FIELDS, "exact manifest")
    body = dict(value)
    digest = _sha(body.pop("exact_manifest_sha256", None), "exact_manifest_sha256")
    if digest != _digest_json(body):
        raise CampaignError("exact manifest digest is invalid")
    if (
        value["schema"] != EXACT_MANIFEST_SCHEMA
        or value["campaign_id"] != campaign["campaign_id"]
        or value["manifest_sha256"] != campaign["manifest_sha256"]
        or value["owner"] != campaign["owner"]
        or value["total"] != len(campaign["functions"])
        or not isinstance(value["exact"], Mapping)
        or not set(value["exact"]) <= set(campaign["functions"])
    ):
        raise CampaignError("exact manifest identity is invalid")
    for function, raw_entry in value["exact"].items():
        entry = _closed_keys(raw_entry, EXACT_ENTRY_FIELDS, f"exact entry {function}")
        source_sha = _sha(entry["source_sha256"], f"{function} source_sha256")
        frontier_sha = _sha(entry["frontier_sha256"], f"{function} frontier_sha256")
        report_sha = _sha(entry["report_sha256"], f"{function} report_sha256")
        report_path = (
            _state_root(root) / "proof-cas" / "reports" / report_sha[:2]
            / f"{report_sha}.json"
        )
        report = _read_json(report_path, f"exact report {function}")
        if not isinstance(report, Mapping):
            raise CampaignError(f"exact report {function} is invalid")
        synthetic_frontier = {
            "function": function, "frontier_sha256": frontier_sha,
            "source_sha256": source_sha,
            "candidate_object_sha256": report.get("candidate_object_sha256"),
            "report_receipts": report.get("proof_receipts"),
            "focus_evidence_sha256": (
                report.get("evidence", {}).get("focus_evidence_sha256")
                if isinstance(report.get("evidence"), Mapping) else None
            ),
        }
        validated_report = _validate_exact_report(
            report, campaign, synthetic_frontier
        )
        if validated_report["report_sha256"] != report_sha:
            raise CampaignError(f"exact manifest report binding drift: {function}")
    closes_owner = len(value["exact"]) == len(campaign["functions"])
    if closes_owner:
        closure = _validate_final_owner_receipt(
            value["owner_closure"], campaign, _digest_file(campaign["_source"])
        )
        if closure["source_sha256"] != _digest_file(campaign["_source"]):
            raise CampaignError("exact manifest owner closure source drift")
    elif value["owner_closure"] is not None:
        raise CampaignError("partial exact manifest cannot claim owner closure")
    return dict(value)


def _validate_exact_manifest_progress(
    campaign: Mapping[str, Any], value: Any,
) -> dict[str, Any]:
    """Validate the self-contained manifest envelope without loading proof CAS."""

    value = _closed_keys(value, EXACT_MANIFEST_FIELDS, "exact manifest")
    body = dict(value)
    digest = _sha(body.pop("exact_manifest_sha256", None), "exact_manifest_sha256")
    if digest != _digest_json(body):
        raise CampaignError("exact manifest digest is invalid")
    if (
        value["schema"] != EXACT_MANIFEST_SCHEMA
        or value["campaign_id"] != campaign["campaign_id"]
        or value["manifest_sha256"] != campaign["manifest_sha256"]
        or value["owner"] != campaign["owner"]
        or value["total"] != len(campaign["functions"])
        or not isinstance(value["exact"], Mapping)
        or not set(value["exact"]) <= set(campaign["functions"])
    ):
        raise CampaignError("exact manifest identity is invalid")
    for function, raw_entry in value["exact"].items():
        entry = _closed_keys(raw_entry, EXACT_ENTRY_FIELDS, f"exact entry {function}")
        _sha(entry["source_sha256"], f"{function} source_sha256")
        _sha(entry["frontier_sha256"], f"{function} frontier_sha256")
        _sha(entry["report_sha256"], f"{function} report_sha256")
    closes_owner = len(value["exact"]) == len(campaign["functions"])
    if closes_owner:
        closure = value["owner_closure"]
        if not isinstance(closure, Mapping):
            raise CampaignError("closed exact manifest lacks owner closure")
        closure_source = _sha(
            closure.get("source_sha256"), "final owner source_sha256"
        )
        _validate_final_owner_receipt(closure, campaign, closure_source)
    elif value["owner_closure"] is not None:
        raise CampaignError("partial exact manifest cannot claim owner closure")
    return dict(value)


def _publish_exact(
    root: Path, campaign: Mapping[str, Any], frontier: Mapping[str, Any], report_raw: Any,
    *, final_owner_receipt: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    report = _validate_exact_report(report_raw, campaign, frontier)
    report_sha = report["report_sha256"]
    report_path = _state_root(root) / "proof-cas" / "reports" / report_sha[:2] / f"{report_sha}.json"
    owner = _owner_root(root, campaign)
    manifest_path = owner / "exact-manifest.json"
    exact: dict[str, Any] = {}
    if manifest_path.exists():
        current = _validate_exact_manifest(
            root, campaign, _read_json(manifest_path, "exact manifest")
        )
        exact.update(current["exact"])
    exact[frontier["function"]] = {
        "source_sha256": frontier["source_sha256"], "frontier_sha256": frontier["frontier_sha256"],
        "report_sha256": report_sha,
    }
    closes_owner = len(exact) == len(campaign["functions"])
    if closes_owner and final_owner_receipt is None:
        raise CampaignError("full owner closure requires final_owner proof")
    if not closes_owner and final_owner_receipt is not None:
        raise CampaignError("final_owner proof supplied before owner closure")
    body = {
        "schema": EXACT_MANIFEST_SCHEMA, "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"], "owner": campaign["owner"],
        "exact": dict(sorted(exact.items())), "total": len(campaign["functions"]),
        "owner_closure": dict(final_owner_receipt) if final_owner_receipt else None,
        "updated_at": _now(),
    }
    value = {**body, "exact_manifest_sha256": _digest_json(body)}
    writes = [(manifest_path, _canonical(value) + b"\n")]
    if not report_path.is_file():
        writes.append((report_path, _canonical(report) + b"\n"))
    _ensure_state_write_peak(root, campaign, writes)
    if report_path.is_file():
        if _read_json(report_path, "exact report CAS") != report:
            raise CampaignError("exact report CAS publication drift")
    else:
        _atomic_json(report_path, report, limit=campaign["limits"]["report_bytes"])
    _atomic_json(manifest_path, value, limit=campaign["limits"]["frontier_bytes"])
    _validate_exact_manifest(root, campaign, value)
    return {"report_path": str(report_path), "report_sha256": report_sha, "exact_count": len(exact), "total": len(campaign["functions"])}


def _build_exact_report(
    campaign: Mapping[str, Any], frontier: Mapping[str, Any],
    measurement: Mapping[str, Any],
) -> dict[str, Any]:
    metrics = frontier["metrics"]
    focus = measurement["focus_evidence"]
    body = {
        "schema": REPORT_SCHEMA, "status": "exact", "completed": True,
        "authority_advanced": False, "owner": campaign["owner"],
        "function": frontier["function"], "campaign_id": campaign["campaign_id"],
        "manifest_sha256": campaign["manifest_sha256"],
        "unit": campaign["unit"], "source_path": campaign["source_relpath"],
        "base_commit": campaign["base_commit"],
        "frontier_sha256": frontier["frontier_sha256"],
        "source_sha256": frontier["source_sha256"],
        "target_object_sha256": frontier["target_object_sha256"],
        "candidate_object_sha256": frontier["candidate_object_sha256"],
        "toolchain_sha256": campaign["toolchain"]["sha256"],
        "result": {
            "strict_percent": 100, "data_percent": 100,
            "target_bytes": metrics["strict"]["target_bytes"],
            "candidate_bytes": metrics["strict"]["candidate_bytes"],
            "strict_difference_count": 0, "data_difference_count": 0,
            "strict_row_ids_sha256": focus["strict_row_ids_sha256"],
            "data_row_ids_sha256": focus["data_row_ids_sha256"],
            "physical_target_count": metrics["physical_target_count"],
            "physical_candidate_count": metrics["physical_candidate_count"],
            "physical_difference_count": 0,
            "physical_difference_ids_sha256": focus["physical_difference_ids_sha256"],
            "protected_total": metrics["protected_total"],
            "protected_losses": metrics["protected_losses"],
            "protected_sibling_digest": focus["sibling_digest"],
            "source_link_exact": metrics["source_link_exact"],
        },
        "proof_receipts": frontier["report_receipts"],
        "evidence": {
            "schema": "owner_campaign_report_evidence/v1",
            "owner": campaign["owner"], "function": frontier["function"],
            "unit": campaign["unit"], "source_path": campaign["source_relpath"],
            "base_commit": campaign["base_commit"],
            "source_sha256": frontier["source_sha256"],
            "target_object_sha256": frontier["target_object_sha256"],
            "candidate_object_sha256": frontier["candidate_object_sha256"],
            "focus_evidence_sha256": frontier["focus_evidence_sha256"],
            "strict_row_count": 0,
            "strict_row_ids_sha256": focus["strict_row_ids_sha256"],
            "data_row_count": 0,
            "data_row_ids_sha256": focus["data_row_ids_sha256"],
            "physical_target_count": metrics["physical_target_count"],
            "physical_candidate_count": metrics["physical_candidate_count"],
            "physical_difference_count": 0,
            "physical_difference_ids_sha256": focus["physical_difference_ids_sha256"],
            "protected_total": metrics["protected_total"],
            "protected_losses": metrics["protected_losses"],
            "protected_sibling_identities": focus["sibling_identities"],
            "protected_sibling_digest": focus["sibling_digest"],
            "proofs": measurement["proofs"],
        },
        "completed_at": _now(),
    }
    return {**body, "report_sha256": _digest_json(body)}


def _retain(
    root: Path, campaign: Mapping[str, Any], base: Mapping[str, Any],
    candidate: Mapping[str, Any], measurement: Mapping[str, Any], status: str,
    *, final_owner_receipt: Mapping[str, Any] | None = None,
) -> tuple[dict[str, Any] | None, dict[str, Any] | None]:
    function = base["function"]
    directory = _function_root(root, campaign, function)
    frontier = _frontier_from_measurement(campaign, function, measurement, parent=base)
    report = None
    if status == "exact":
        report = measurement["exact_report"]
        if report is None:
            report = _build_exact_report(campaign, frontier, measurement)
        _validate_exact_report(report, campaign, frontier)
        if dict(report["evidence"]["proofs"]) != dict(measurement["proofs"]):
            raise CampaignError("exact report proofs drift from verified measurement")
        try:
            from tools.owner_campaign_verify import VerificationError, verify_report

            verify_report(
                report, focus_evidence=measurement["focus_evidence"],
                expected={
                    "owner": campaign["owner"], "function": function,
                    "campaign_id": campaign["campaign_id"],
                    "manifest_sha256": campaign["manifest_sha256"],
                    "unit": campaign["unit"], "source_path": campaign["source_relpath"],
                    "base_commit": campaign["base_commit"],
                    "source_sha256": frontier["source_sha256"],
                    "target_object_sha256": campaign["target_object"]["sha256"],
                    "candidate_object_sha256": frontier["candidate_object_sha256"],
                    "toolchain_sha256": campaign["toolchain"]["sha256"],
                },
            )
        except VerificationError as exc:
            raise CampaignError(f"exact report independent verification failed: {exc}") from exc
    timeout = float(campaign["limits"]["command_timeout_seconds"])
    with _frontier_lock_chain(root, campaign, function):
        latest_path = directory / "latest-frontier.json"
        if not latest_path.is_file():
            return None, None
        current = _validate_frontier(
            _read_json(latest_path, "latest frontier"), campaign, function
        )
        if (
            current["frontier_sha256"] != base["frontier_sha256"]
            or _digest_file(campaign["_source"]) != base["source_sha256"]
        ):
            return None, None
        _publish_focus_evidence(root, campaign, measurement["focus_evidence"])
        if isinstance(measurement.get("reconstruction_evidence"), Mapping):
            _publish_reconstruction_evidence(
                root, campaign, measurement["reconstruction_evidence"]
            )
        pending_body = {
            "schema": PENDING_SCHEMA,
            "base_source_sha256": base["source_sha256"],
            "candidate_source_sha256": candidate["_source_sha256"],
            "frontier": frontier, "exact_report": report,
            "final_owner_receipt": (
                dict(final_owner_receipt) if final_owner_receipt else None
            ),
        }
        pending = {**pending_body, "pending_sha256": _digest_json(pending_body)}
        pending_path = directory / "frontier.pending.json"
        _ensure_state_write_peak(
            root, campaign,
            [
                (pending_path, _canonical(pending) + b"\n"),
                (latest_path, _canonical(frontier) + b"\n"),
            ],
        )
        _atomic_json(
            pending_path, pending, limit=campaign["limits"]["frontier_bytes"]
        )
        _atomic_bytes(campaign["_source"], candidate["_source_bytes"])
        if _digest_file(campaign["_source"]) != candidate["_source_sha256"]:
            raise CampaignError("frontier source publication failed")
        _atomic_json(
            latest_path, frontier, limit=campaign["limits"]["frontier_bytes"]
        )
        exact_receipt = None
        if status == "exact":
            assert report is not None
            exact_receipt = _publish_exact(
                root, campaign, frontier, report,
                final_owner_receipt=final_owner_receipt,
            )
        pending_path.unlink()
        return frontier, exact_receipt


def _check_limits(root: Path, campaign: Mapping[str, Any]) -> None:
    # Directory scanning/deletion belongs to a maintenance-only domain.  CAS
    # publication uses short per-blob locks and never waits behind this scan.
    maintenance_age = max(
        GC_MINIMUM_AGE_SECONDS, _command_timeout_seconds(campaign)
    )
    with _exclusive_lock(
        _state_root(root) / "proof-cas" / "maintenance.lock",
        _command_timeout_seconds(campaign),
    ):
        _gc_focus_evidence(root, minimum_age_seconds=maintenance_age)
        _gc_reconstruction_evidence(root, minimum_age_seconds=maintenance_age)
        _gc_physical_evidence(root, minimum_age_seconds=maintenance_age)
        _gc_obsolete_scratch(
            root, campaign, minimum_age_seconds=maintenance_age
        )
    scratch_root = (
        _state_root(root) / "scratch" / _slug(str(campaign["campaign_id"]))
    )
    scratch_soft = campaign["limits"]["scratch_soft_bytes"]
    scratch_hard = campaign["limits"]["scratch_hard_bytes"]
    for worker in range(5):
        repo = _scratch_repo(root, campaign, worker)
        repo_size = _tree_size(repo)
        if repo_size > scratch_soft:
            if not _scratch_is_owned(campaign, repo):
                raise InfrastructureError(
                    f"campaign worker {worker} scratch is not owned"
                )
            _cleanup_cell_outputs(repo, campaign)
            repo_size = _tree_size(repo)
        if repo_size > scratch_hard:
            raise InfrastructureError(
                f"campaign worker {worker} scratch exceeds hard limit"
            )
    # The manifest limits are per reusable worker.  Stray state outside the
    # five repositories is still bounded by the maximum total worker budget.
    if _tree_size(scratch_root) > 5 * scratch_hard:
        raise InfrastructureError("campaign aggregate scratch exceeds hard limit")
    owner_size = _tree_size(_owner_root(root, campaign))
    if owner_size > campaign["limits"]["owner_state_bytes"]:
        raise CampaignError("retained owner state exceeds hard limit")
    retained_global = (
        _tree_size(_state_root(root) / "owners")
        + _tree_size(_state_root(root) / "proof-cas")
        + _tree_size(_state_root(root) / "inbox")
        + _tree_size(_state_root(root) / "tool-cas")
    )
    if retained_global > 64 << 20:
        raise CampaignError("retained global campaign state exceeds 64 MiB")
    scratch_global = _tree_size(_state_root(root) / "scratch")
    if scratch_global > GLOBAL_SCRATCH_HARD_BYTES:
        raise InfrastructureError(
            "all campaign scratch repositories exceed global hard limit"
        )


def _cleanup_cell_outputs(
    scratch: Path, campaign: Mapping[str, Any]
) -> None:
    """Delete hook-owned raw cell output while preserving the worker checkout."""

    outputs: set[Path] = set()
    for descriptor in campaign["commands"].values():
        output = _bound_path(
            scratch, descriptor["measurement_relpath"],
            "cell output cleanup", exists=False,
        )
        outputs.add(output)
    allowed_roots = {
        _bound_path(scratch, raw, "allowed build cleanup", exists=False)
        for raw in campaign["allowed_build_paths"]
    }
    protected = {
        _bound_path(
            scratch, campaign["source_relpath"], "scratch source", exists=False
        ),
        _scratch_target_path(Path(campaign["_root"]), scratch, campaign),
    }
    parents = {output.parent for output in outputs}
    for parent in sorted(parents, key=lambda item: len(item.parts), reverse=True):
        bounded_parent = (
            parent != scratch
            and _inside(scratch, parent)
            and any(parent != root and _inside(root, parent) for root in allowed_roots)
            and not any(_inside(parent, item) for item in protected)
        )
        if bounded_parent and parent.exists():
            shutil.rmtree(parent)
            continue
        # A command may place its result directly in an allowed build root.
        # Remove only that exact hook-owned file; the root may contain the
        # configured compiler mapping or other persistent worker state.
        for output in outputs:
            if output.parent != parent or not output.exists():
                continue
            if output.is_file() and not output.is_symlink():
                output.unlink()


def _would_close_owner(
    root: Path, campaign: Mapping[str, Any], function: str,
) -> bool:
    path = _owner_root(root, campaign) / "exact-manifest.json"
    exact: Mapping[str, Any] = {}
    if path.is_file():
        manifest = _validate_exact_manifest(
            root, campaign, _read_json(path, "exact manifest")
        )
        exact = manifest["exact"]
    return function not in exact and len(exact) + 1 == len(campaign["functions"])


def _sealed_outcome(body: Mapping[str, Any]) -> dict[str, Any]:
    value = dict(body)
    value.pop("result_sha256", None)
    return {**value, "result_sha256": _digest_json(value)}


def _rebase_outcome(
    root: Path, candidate_path: Path, hint: Mapping[str, Any],
    frontier: Mapping[str, Any],
) -> dict[str, Any]:
    binding = hint.get("candidate_source")
    source_path = binding.get("path") if isinstance(binding, Mapping) else None
    source_sha = binding.get("sha256") if isinstance(binding, Mapping) else None
    base_binding = hint.get("base_source")
    base_source_path = (
        base_binding.get("path") if isinstance(base_binding, Mapping) else None
    )
    base_source_sha = (
        base_binding.get("sha256") if isinstance(base_binding, Mapping) else None
    )
    return _sealed_outcome({
        "schema": "owner_campaign_result/v1", "status": "stale_rebase",
        "function": frontier["function"],
        "frontier_sha256": frontier["frontier_sha256"],
        "source_sha256": frontier["source_sha256"],
        "rebase_input": {
            "descriptor_path": Path(candidate_path).relative_to(root).as_posix(),
            "descriptor_sha256": _digest_file(candidate_path),
            "candidate_source_path": source_path,
            "candidate_source_sha256": source_sha,
            "base_source_path": base_source_path,
            "base_source_sha256": base_source_sha,
            "rebase_depth": hint.get("rebase_depth"),
            "function_span": hint.get("function_span"),
        },
        "cleanup_status": "complete", "cleanup_errors": [],
        "authority_advanced": False,
    })


def _post_candidate_cleanup(
    root: Path, campaign: Mapping[str, Any], scratch: Path | None,
    candidate_paths: Sequence[Path], *, preserve_candidate: bool,
    run_maintenance: bool = True,
) -> list[str]:
    errors: list[str] = []
    if not preserve_candidate:
        try:
            _cleanup_candidate_artifacts(root, campaign, candidate_paths)
        except BaseException as exc:
            errors.append(f"candidate input cleanup failed: {exc}")
    if scratch is not None:
        try:
            live_bytes = campaign["_source"].read_bytes()
            _sync_scratch_source(root, scratch, campaign, live_bytes)
        except BaseException as exc:
            errors.append(f"scratch source restoration failed: {exc}")
        try:
            _cleanup_cell_outputs(scratch, campaign)
        except BaseException as exc:
            errors.append(f"cell output cleanup failed: {exc}")
    if run_maintenance:
        try:
            _check_limits(root, campaign)
        except BaseException as exc:
            errors.append(f"campaign maintenance failed: {exc}")
    return errors


def _run_candidate_unleased(
    root: Path, campaign: Mapping[str, Any], candidate_path: Path,
    *, worker: int = 0, _defer_maintenance: bool = False,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    _check_cancelled(root, campaign)
    candidate_hint = _read_json(_bound_path(root, str(candidate_path), "candidate descriptor"), "candidate descriptor")
    if not isinstance(candidate_hint, Mapping) or not isinstance(candidate_hint.get("function"), str):
        raise CampaignError("candidate descriptor function is missing")
    function = candidate_hint["function"]
    # Candidate workers own the complete snapshot→compile sequence on their
    # assigned scratch checkout.  Maintenance is deferred to the candidate
    # tail (direct calls) or the single batch tail (run_loop).
    frontier = snapshot_frontier(
        root, campaign, function, worker=worker, _defer_maintenance=True,
    )
    if candidate_hint.get("base_frontier_sha256") != frontier["frontier_sha256"]:
        outcome = _rebase_outcome(
            root, Path(candidate_path), candidate_hint, frontier
        )
        if not _defer_maintenance:
            _check_limits(root, campaign)
        return outcome
    candidate = _load_candidate(root, candidate_path, campaign, frontier)
    key = _candidate_key(campaign, frontier, candidate)
    if not _reserve_candidate(
        root, campaign, function, key, frontier, candidate["_source_sha256"]
    ):
        _cleanup_candidate_artifacts(
            root, campaign, [
                candidate["_path"], candidate["_source"],
                candidate["_base_source"],
            ]
        )
        if not _defer_maintenance:
            _check_limits(root, campaign)
        return {"schema": "owner_campaign_result/v1", "status": "deduplicated", "candidate_key": key, "function": function, "authority_advanced": False}
    scratch: Path | None = None
    reservation_finished = False
    result: dict[str, Any] | None = None
    preserve_candidate = False
    candidate_paths = [
        candidate["_path"], candidate["_source"], candidate["_base_source"]
    ]
    try:
        scratch = _ensure_scratch(root, campaign, worker)
        _sync_scratch_source(root, scratch, campaign, candidate["_source_bytes"])
        measurement = _run_hook(root, scratch, campaign, function, candidate["_source_sha256"], "candidate")
        _verify_publication_sources(
            campaign, scratch, live_sha256=None,
            scratch_sha256=candidate["_source_sha256"],
        )
        status = assess_gain(
            frontier["metrics"], measurement["metrics"],
            base_focus=_frontier_focus(root, campaign, frontier),
            candidate_focus=measurement["focus_evidence"],
        )
        retained = None
        exact_receipt = None
        final_owner_receipt = None
        if status == "exact" and _would_close_owner(root, campaign, function):
            final_owner_receipt = _run_final_owner(
                root, scratch, campaign, function, candidate["_source_sha256"]
            )
        if status in {"improved", "exact"}:
            _verify_publication_sources(
                campaign, scratch, live_sha256=None,
                scratch_sha256=candidate["_source_sha256"],
            )
            retained, exact_receipt = _retain(
                root, campaign, frontier, candidate, measurement, status,
                final_owner_receipt=final_owner_receipt,
            )
            if retained is None:
                status = "stale"
                preserve_candidate = True
        record = _dedupe_record(
            key=key, function=function, frontier=frontier,
            candidate_source_sha256=candidate["_source_sha256"], status=status,
            strict_delta=(
                measurement["metrics"]["strict"]["differences"]
                - frontier["metrics"]["strict"]["differences"]
            ),
            data_delta=(
                measurement["metrics"]["data"]["differences"]
                - frontier["metrics"]["data"]["differences"]
            ),
            physical_delta=(
                measurement["metrics"]["physical_differences"]
                - frontier["metrics"]["physical_differences"]
            ),
        )
        _finish_candidate_reservation(
            root, campaign, function, key, record
        )
        reservation_finished = True
        public_status = "stale_rebase" if status == "stale" else status
        result = _sealed_outcome({
            "schema": "owner_campaign_result/v1", "status": status, "candidate_key": key,
            "function": function, "frontier_sha256": retained["frontier_sha256"] if retained else frontier["frontier_sha256"],
            "source_sha256": retained["source_sha256"] if retained else frontier["source_sha256"],
            "metrics": measurement["metrics"], "exact": exact_receipt,
            "cleanup_status": "complete", "cleanup_errors": [],
            "authority_advanced": False,
        })
        result["status"] = public_status
        if preserve_candidate:
            result["rebase_input"] = {
                "descriptor_path": candidate["_path"].relative_to(root).as_posix(),
                "descriptor_sha256": _digest_file(candidate["_path"]),
                "candidate_source_path": candidate["_source"].relative_to(root).as_posix(),
                "candidate_source_sha256": candidate["_source_sha256"],
                "base_source_path": candidate["_base_source"].relative_to(root).as_posix(),
                "base_source_sha256": candidate["_base_source_sha256"],
                "rebase_depth": candidate["rebase_depth"],
                "function_span": candidate["function_span"],
            }
            result = _sealed_outcome(result)
    except BaseException as exc:
        if not reservation_finished:
            _finish_candidate_reservation(
                root, campaign, function, key, None
            )
            reservation_finished = True
        cleanup_errors = _post_candidate_cleanup(
            root, campaign, scratch, candidate_paths, preserve_candidate=True,
            run_maintenance=not _defer_maintenance,
        )
        for error in cleanup_errors:
            try:
                exc.add_note(error)
            except AttributeError:
                pass
        raise
    assert result is not None
    cleanup_errors = _post_candidate_cleanup(
        root, campaign, scratch, candidate_paths,
        preserve_candidate=preserve_candidate,
        run_maintenance=not _defer_maintenance,
    )
    if cleanup_errors:
        if result["status"] not in {"exact", "improved"}:
            raise CampaignError("terminal cleanup failed: " + "; ".join(cleanup_errors))
        result["cleanup_status"] = "cleanup_incomplete"
        result["cleanup_errors"] = cleanup_errors[:8]
        result = _sealed_outcome(result)
    return result


def run_candidate(
    root: Path, campaign: Mapping[str, Any], candidate_path: Path,
    *, worker: int = 0, _defer_maintenance: bool = False,
) -> dict[str, Any]:
    """Run one candidate while exclusively owning its reusable scratch repo."""

    root = Path(os.path.abspath(root))
    with _scratch_lease(root, campaign, worker):
        token = _SCRATCH_LEASE_HELD.set(True)
        try:
            return _run_candidate_unleased(
                root, campaign, candidate_path, worker=worker,
                _defer_maintenance=_defer_maintenance,
            )
        finally:
            _SCRATCH_LEASE_HELD.reset(token)


def campaign_terminal_progress(
    root: Path, campaign: Mapping[str, Any],
) -> dict[str, Any]:
    """Return constant-cost exact progress for supervisor polling.

    This intentionally reads only the exact manifest.  Full frontier, proof
    CAS, scratch, and retained-size validation remains campaign_status work.
    """

    total = len(campaign["functions"])
    manifest_path = _owner_root(root, campaign) / "exact-manifest.json"
    exact_count = 0
    with _exclusive_lock(
        _owner_root(root, campaign) / "source-cas.lock",
        _command_timeout_seconds(campaign),
    ):
        if manifest_path.is_file():
            manifest = _validate_exact_manifest_progress(
                campaign, _read_json(manifest_path, "exact manifest")
            )
            exact_count = len(manifest["exact"])
    return {
        "exact_count": exact_count,
        "total": total,
        "closed": exact_count == total,
    }


def campaign_status(root: Path, campaign: Mapping[str, Any]) -> dict[str, Any]:
    owner = _owner_root(root, campaign)
    exact_manifest = owner / "exact-manifest.json"
    exact: Mapping[str, Any] = {}
    functions: dict[str, Any] = {}
    focus_evidence: dict[str, Any] = {}
    reconstruction_evidence: dict[str, Any] = {}
    for function in campaign["functions"]:
        # Recovery and status reads share the writer's complete lock order;
        # otherwise status could validate a pending source against a frontier
        # while retention is in the middle of its source CAS.
        with _frontier_lock_chain(root, campaign, function):
            _recover_pending_locked(root, campaign, function)
            path = _function_root(root, campaign, function) / "latest-frontier.json"
            frontier = (
                _validate_frontier(_read_json(path, "latest frontier"), campaign, function)
                if path.is_file() else None
            )
            functions[function] = frontier
            if frontier is not None:
                digest = frontier["focus_evidence_sha256"]
                evidence_path = (
                    _state_root(root) / "proof-cas" / "focus" / digest[:2]
                    / f"{digest}.json"
                )
                evidence = _validate_focus_evidence(
                    _read_json(evidence_path, f"focus evidence {function}"),
                    campaign, function, frontier["source_sha256"],
                )
                if evidence["focus_evidence_sha256"] != digest:
                    raise CampaignError("frontier focus evidence binding drift")
                focus_evidence[function] = {
                    "sha256": digest,
                    "path": evidence_path.relative_to(root).as_posix(),
                }
                reconstruction = _frontier_reconstruction(root, frontier)
                if reconstruction is not None:
                    reconstruction_digest = frontier[
                        "reconstruction_evidence_sha256"
                    ]
                    reconstruction_path = (
                        _state_root(root) / "proof-cas" / "reconstruction"
                        / reconstruction_digest[:2]
                        / f"{reconstruction_digest}.json"
                    )
                    reconstruction_evidence[function] = {
                        "sha256": reconstruction_digest,
                        "path": reconstruction_path.relative_to(root).as_posix(),
                        "status": reconstruction["status"],
                    }
    # The owner exact manifest is updated under source-cas by _publish_exact;
    # validate it under that same lock rather than observing a partial replace.
    with _exclusive_lock(
        _owner_root(root, campaign) / "source-cas.lock",
        _command_timeout_seconds(campaign),
    ):
        if exact_manifest.is_file():
            value = _validate_exact_manifest(
                root, campaign, _read_json(exact_manifest, "exact manifest")
            )
            exact = value["exact"]
    return {
        "schema": "owner_campaign_status/v1", "campaign_id": campaign["campaign_id"],
        "owner": campaign["owner"], "exact_count": len(exact), "total": len(campaign["functions"]),
        "functions": functions, "focus_evidence": focus_evidence,
        "reconstruction_evidence": reconstruction_evidence,
        "scratch_bytes": _tree_size(
            _state_root(root) / "scratch" / _slug(str(campaign["campaign_id"]))
        ),
        "retained_bytes": _tree_size(owner), "authority_advanced": False,
    }


def run_loop(
    root: Path, campaign: Mapping[str, Any], candidate_paths: Sequence[Path],
) -> list[dict[str, Any]]:
    """Run each sealed candidate once; infrastructure failures remain retryable."""

    paths = [Path(path) for path in candidate_paths]
    if not paths:
        return []
    # Read immutable descriptor identities and deterministically assign
    # identical cells to their first input slot before launching workers.  Each
    # worker performs its own hash-bound snapshot→compile sequence; there is no
    # whole-batch snapshot barrier, so a ready function can compile while an
    # unrelated function is still measuring its baseline.
    hints: list[Mapping[str, Any]] = []
    signatures: dict[tuple[str, str, str], int] = {}
    duplicate_of: dict[int, int] = {}
    active_indices: list[int] = []
    for path in paths:
        hint = _read_json(_bound_path(root, str(path), "candidate descriptor"), "candidate descriptor")
        if not isinstance(hint, Mapping) or not isinstance(hint.get("function"), str):
            raise CampaignError("candidate descriptor function is missing")
        hints.append(hint)
    for index, hint in enumerate(hints):
        source_binding = hint.get("candidate_source")
        source_sha = source_binding.get("sha256") if isinstance(source_binding, Mapping) else None
        if isinstance(source_sha, str) and SHA_RE.fullmatch(source_sha):
            signature = (
                hint["function"], str(hint.get("base_frontier_sha256")), source_sha
            )
            if signature in signatures:
                duplicate_of[index] = signatures[signature]
                continue
            signatures[signature] = index
        active_indices.append(index)
    worker_count = min(5, len(active_indices))
    indexed_results: list[tuple[int, dict[str, Any]]] = []
    pending: Queue[int] = Queue(maxsize=len(active_indices))
    for index in active_indices:
        pending.put_nowait(index)
    worker_errors: list[tuple[int, BaseException]] = []
    outcome_lock = threading.Lock()
    stop = threading.Event()

    def run_worker(worker: int) -> None:
        while not stop.is_set():
            try:
                index = pending.get_nowait()
            except Empty:
                return
            try:
                if stop.is_set():
                    return
                path = paths[index]
                _check_cancelled(root, campaign)
                try:
                    result = run_candidate(
                        root, campaign, path, worker=worker,
                        _defer_maintenance=True,
                    )
                except InfrastructureError as exc:
                    result = {
                        "schema": "owner_campaign_result/v1", "status": "infra_retry",
                        "candidate": str(path), "reason": str(exc)[:1000],
                        "authority_advanced": False,
                    }
                with outcome_lock:
                    indexed_results.append((index, result))
            except BaseException as exc:
                with outcome_lock:
                    worker_errors.append((index, exc))
                stop.set()
                return
            finally:
                pending.task_done()

    primary_error: BaseException | None = None
    results: list[dict[str, Any]] = []
    try:
        with ThreadPoolExecutor(max_workers=worker_count) as executor:
            list(executor.map(run_worker, range(worker_count)))
        if worker_errors:
            raise min(worker_errors, key=lambda item: item[0])[1]
        by_index = dict(indexed_results)
        for index, original in duplicate_of.items():
            primary = by_index[original]
            if primary["status"] == "infra_retry":
                duplicate = dict(primary)
                duplicate["candidate"] = str(paths[index])
            else:
                duplicate = {
                    "schema": "owner_campaign_result/v1", "status": "deduplicated",
                    "candidate_key": primary.get("candidate_key"),
                    "function": hints[index]["function"], "authority_advanced": False,
                }
                cleanup = [paths[index]]
                binding = hints[index].get("candidate_source")
                if (
                    isinstance(binding, Mapping)
                    and isinstance(binding.get("path"), str)
                ):
                    cleanup.append(Path(binding["path"]))
                base_binding = hints[index].get("base_source")
                if (
                    isinstance(base_binding, Mapping)
                    and isinstance(base_binding.get("path"), str)
                ):
                    cleanup.append(Path(base_binding["path"]))
                _cleanup_candidate_artifacts(root, campaign, cleanup)
            indexed_results.append((index, duplicate))
        indexed_results.sort(key=lambda item: item[0])
        results = [item[1] for item in indexed_results]
    except BaseException as exc:
        primary_error = exc
    try:
        _check_limits(root, campaign)
    except BaseException as exc:
        if primary_error is not None:
            try:
                primary_error.add_note(f"batch maintenance failed: {exc}")
            except AttributeError:
                pass
            raise primary_error
        error = CampaignError(
            "batch maintenance failed after candidate results were finalized: "
            f"{exc}"
        )
        # Preserve primary outcomes for callers that need to reconcile retained
        # gains after a terminal maintenance failure.  The normal return schema
        # stays unchanged, and direct run_candidate callers keep their existing
        # per-candidate cleanup contract.
        error.candidate_results = tuple(results)  # type: ignore[attr-defined]
        raise error from exc
    if primary_error is not None:
        raise primary_error
    return results


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    sub = parser.add_subparsers(dest="command", required=True)
    loop = sub.add_parser("loop")
    loop.add_argument("--campaign", required=True)
    loop.add_argument("--candidate", action="append", default=[])
    snapshot = sub.add_parser("snapshot")
    snapshot.add_argument("--campaign", required=True)
    snapshot.add_argument("--function", required=True)
    status_parser = sub.add_parser("status")
    status_parser.add_argument("--campaign", required=True)
    cancel = sub.add_parser("cancel")
    cancel.add_argument("--campaign", required=True)
    cancel.add_argument("--epoch", required=True, type=int)
    args = parser.parse_args(argv)
    root = Path(args.root).resolve()
    campaign = load_campaign(root, Path(args.campaign))
    try:
        if args.command == "loop":
            value: Any = run_loop(root, campaign, [Path(item) for item in args.candidate])
        elif args.command == "snapshot":
            value = snapshot_frontier(root, campaign, args.function)
        elif args.command == "status":
            value = campaign_status(root, campaign)
        else:
            value = cancel_campaign(root, campaign, args.epoch)
        print(json.dumps(value, indent=2, sort_keys=True))
        return 0
    except CampaignError as exc:
        parser.error(str(exc))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
