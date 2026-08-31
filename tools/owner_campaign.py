#!/usr/bin/env python3
"""Autonomous, owner-scoped cracking campaign runtime.

This is the v2 hot path.  A campaign manifest grants owner scope once; cells
are hash-bound, compiled in one reusable scratch worktree, measured once, and
atomically retained when they improve the monotonic frontier.  No STOP file,
manager key, approval, permit, or predicted-row packet is consulted here.
"""

from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager
import datetime as dt
import difflib
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import tempfile
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
LIMIT_FIELDS = {
    "command_timeout_seconds", "scratch_soft_bytes", "scratch_hard_bytes",
    "cell_temporary_bytes", "focus_evidence_bytes", "frontier_bytes",
    "report_bytes", "dedupe_bytes",
    "owner_state_bytes",
}
COMMAND_FIELDS = {"argv", "measurement_relpath"}


def _select_git_executable(candidates: Sequence[Path], *, windows: bool) -> Path:
    """Choose a stable native Git while retaining a portable last-resort fallback."""

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
    if not unique:
        raise CampaignError("native Git executable cannot be resolved")
    if not windows:
        return unique[0]

    def rank(path: Path) -> tuple[int, int]:
        lowered = str(path).replace("/", "\\").lower()
        msys = "\\msys" in lowered or "\\devkitpro\\" in lowered
        git_for_windows = "\\git\\cmd\\git.exe" in lowered or "\\git\\bin\\git.exe" in lowered
        return (0 if git_for_windows and not msys else 2 if msys else 1, len(lowered))

    return min(unique, key=rank)


def _resolve_git_executable() -> tuple[Path, str]:
    candidates: list[Path] = []
    if os.name == "nt":
        for variable in ("ProgramW6432", "ProgramFiles", "LOCALAPPDATA"):
            base = os.environ.get(variable)
            if not base:
                continue
            root = Path(base)
            if variable == "LOCALAPPDATA":
                root = root / "Programs"
            candidates.extend((root / "Git" / "cmd" / "git.exe", root / "Git" / "bin" / "git.exe"))
        for item in os.environ.get("PATH", "").split(os.pathsep):
            if item:
                candidates.append(Path(item) / "git.exe")
    else:
        found = shutil.which("git")
        if found:
            candidates.append(Path(found))
    selected = _select_git_executable(candidates, windows=os.name == "nt")
    probe = subprocess.run(
        [str(selected), "--version"], capture_output=True, text=True, check=False,
    )
    if probe.returncode or not probe.stdout.startswith("git version "):
        raise CampaignError("resolved Git executable failed identity probe")
    return selected, _digest_file(selected)


def _git_argv(campaign: Mapping[str, Any], *arguments: str) -> list[str]:
    # Source/object evidence is byte-bound.  Never allow a user's global or
    # repository autocrlf setting to rewrite the detached campaign checkout.
    return [
        str(campaign["_git_executable"]), "-c", "core.autocrlf=false", *arguments
    ]


def load_campaign(root: Path, path: Path) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    path = _bound_path(root, str(path), "campaign manifest")
    raw = _closed_keys(_read_json(path, "campaign manifest"), MANIFEST_FIELDS, "campaign manifest")
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
    for label in ("target_object", "toolchain", "measurement_producer"):
        binding = _closed_keys(raw[label], {"path", "sha256"}, label)
        bound = _bound_path(root, binding["path"], label)
        if _digest_file(bound) != _sha(binding["sha256"], f"{label}.sha256"):
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
    git_executable, git_sha256 = _resolve_git_executable()
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
    tracked = subprocess.run(
        [str(git_executable), "status", "--porcelain=v1", "--untracked-files=no"],
        cwd=root, capture_output=True, text=True, check=False,
    )
    if tracked.returncode:
        raise CampaignError("campaign repository cleanliness cannot be verified")
    changed: set[str] = set()
    for line in tracked.stdout.splitlines():
        if len(line) < 4:
            raise CampaignError("campaign repository status is malformed")
        path_text = line[3:]
        if " -> " in path_text:
            path_text = path_text.split(" -> ", 1)[1]
        changed.add(path_text.replace("\\", "/"))
    live_source_sha256 = _digest_file(source)
    if changed:
        if changed != {raw["source_relpath"]}:
            raise CampaignError("campaign repository has unapproved tracked writes")
        owner_state = _state_root(root) / "owners" / _slug(str(raw["owner"]))
        bound_live = False
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
        if not bound_live:
            raise CampaignError(
                "campaign source write is not bound to a retained frontier"
            )
    elif live_source_sha256 != base_source_sha256:
        raise CampaignError("clean campaign source does not match the base blob")
    result = dict(raw)
    result["_root"] = root
    result["_path"] = path
    result["_source"] = source
    result["_target"] = _bound_path(root, raw["target_object"]["path"], "target object")
    result["_toolchain"] = _bound_path(root, raw["toolchain"]["path"], "toolchain")
    result["_producer"] = _bound_path(
        root, raw["measurement_producer"]["path"], "measurement producer"
    )
    result["_base_source_sha256"] = base_source_sha256
    result["_git_executable"] = git_executable
    result["_git_sha256"] = git_sha256
    return result


def _slug(value: str) -> str:
    readable = re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_")[:72] or "owner"
    return f"{readable}-{_digest_bytes(value.encode('utf-8'))[:12]}"


def _state_root(root: Path) -> Path:
    return root / "build" / "owner-campaign"


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
    retained = _tree_size(state_root / "owners") + _tree_size(state_root / "proof-cas")
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


def _verify_publication_sources(
    campaign: Mapping[str, Any], scratch: Path, *, live_sha256: str | None,
    scratch_sha256: str,
) -> None:
    if live_sha256 is not None and _digest_file(campaign["_source"]) != live_sha256:
        raise CampaignError("authoritative source drifted before frontier publication")
    scratch_source = scratch / campaign["source_relpath"]
    if _digest_file(scratch_source) != scratch_sha256:
        raise CampaignError("scratch source drifted before frontier publication")


def _expand_argv(
    argv: Sequence[str], *, root: Path, scratch: Path, campaign: Mapping[str, Any],
    function: str, source_sha256: str, phase: str,
) -> list[str]:
    values = {
        "ROOT": str(root), "SCRATCH_ROOT": str(scratch),
        "SOURCE": str(scratch / campaign["source_relpath"]),
        "FUNCTION": function, "OWNER": campaign["owner"], "UNIT": campaign["unit"],
        "TARGET": str(campaign["_target"]), "TOOLCHAIN": str(campaign["_toolchain"]),
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


def _verify_hook_inputs(campaign: Mapping[str, Any]) -> None:
    """Revalidate immutable command inputs immediately before every launch."""

    for label, private in (
        ("target_object", "_target"),
        ("toolchain", "_toolchain"),
        ("measurement_producer", "_producer"),
    ):
        path = campaign[private]
        if not path.is_file() or _digest_file(path) != campaign[label]["sha256"]:
            raise InfrastructureError(f"{label} hash drift before hook execution")


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

    creationflags = subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0
    try:
        process = subprocess.Popen(
            list(argv), cwd=cwd, env=dict(environment), stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, creationflags=creationflags,
            start_new_session=os.name != "nt",
        )
    except OSError as exc:
        raise InfrastructureError(f"command failed to start: {exc}") from exc
    deadline = time.monotonic() + timeout
    failure: str | None = None
    while process.poll() is None:
        if time.monotonic() >= deadline:
            failure = f"command timed out after {timeout:g} seconds"
            break
        if _tree_size(scratch) > scratch_hard_bytes:
            failure = "campaign scratch exceeded hard limit during command"
            break
        if _tree_size(temporary_root) > cell_temporary_bytes:
            failure = "cell temporary storage exceeded limit during command"
            break
        time.sleep(0.05)
    if failure is not None:
        _terminate_process_tree(process)
        stdout, stderr = process.communicate()
        if len(stdout) + len(stderr) > MAX_OUTPUT:
            stdout = stdout[: MAX_OUTPUT // 2]
            stderr = stderr[: MAX_OUTPUT // 2]
        raise InfrastructureError(failure)
    stdout, stderr = process.communicate()
    return subprocess.CompletedProcess(list(argv), process.returncode, stdout, stderr)


def _run_hook(
    root: Path, scratch: Path, campaign: Mapping[str, Any], function: str,
    source_sha256: str, phase: str,
) -> dict[str, Any]:
    _verify_hook_inputs(campaign)
    descriptor = campaign["commands"][phase]
    output = _bound_path(scratch, descriptor["measurement_relpath"], "measurement output", exists=False)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.unlink(missing_ok=True)
    argv = _expand_argv(
        descriptor["argv"], root=root, scratch=scratch, campaign=campaign,
        function=function, source_sha256=source_sha256, phase=phase,
    )
    try:
        environment = dict(os.environ)
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
                len(campaign["protected_exact_functions"])
            ),
            "OWNER_CAMPAIGN_PROTECTED_FUNCTIONS": ",".join(
                campaign["protected_exact_functions"]
            ),
        })
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
    _verify_hook_inputs(campaign)
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
    environment = dict(os.environ)
    environment.update({
        "OWNER_CAMPAIGN_PHASE": "final_owner",
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
            len(campaign["protected_exact_functions"])
        ),
        "OWNER_CAMPAIGN_PROTECTED_FUNCTIONS": ",".join(
            campaign["protected_exact_functions"]
        ),
    })
    try:
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
    if path.is_file():
        if _read_json(path, "focus evidence CAS") != dict(evidence):
            raise CampaignError("focus evidence CAS publication drift")
        return digest
    _ensure_state_write_peak(root, campaign, [(path, _canonical(evidence) + b"\n")])
    _atomic_json(path, evidence, limit=campaign["limits"]["focus_evidence_bytes"])
    if _read_json(path, "focus evidence CAS") != dict(evidence):
        raise CampaignError("focus evidence CAS publication drift")
    return digest


def _gc_focus_evidence(root: Path) -> None:
    """Remove compact focus blobs not referenced by any published frontier."""

    state = _state_root(root)
    for ledger in (state / "owners").rglob("candidate-results.jsonl") if (state / "owners").is_dir() else ():
        try:
            if any(record["status"] == "inflight" for record in _dedupe_records(ledger)):
                return
        except CampaignError:
            return
    referenced: set[str] = set()
    owners = state / "owners"
    if owners.is_dir():
        frontier_paths = [
            *owners.rglob("latest-frontier.json"),
            *owners.rglob("frontier.pending.json"),
        ]
        for frontier_path in frontier_paths:
            try:
                value = _read_json(frontier_path, "frontier GC reference")
                if frontier_path.name == "frontier.pending.json":
                    value = value.get("frontier", {})
                digest = value.get("focus_evidence_sha256")
                if isinstance(digest, str) and SHA_RE.fullmatch(digest):
                    referenced.add(digest)
            except CampaignError:
                # Corrupt retained state must remain available for diagnosis;
                # status validation will fail closed rather than GC hiding it.
                continue
    focus_root = state / "proof-cas" / "focus"
    if focus_root.is_dir():
        for blob in focus_root.rglob("*.json"):
            if blob.stem not in referenced:
                blob.unlink(missing_ok=True)
        for directory in sorted(
            (item for item in focus_root.rglob("*") if item.is_dir()),
            key=lambda item: len(item.parts), reverse=True,
        ):
            try:
                directory.rmdir()
            except OSError:
                pass


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


def _validate_measurement(
    value: Any, *, campaign: Mapping[str, Any], function: str,
    phase: str, source_sha256: str,
) -> dict[str, Any]:
    value = _closed_keys(value, MEASUREMENT_FIELDS, "campaign measurement")
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
    if metrics["protected_total"] != len(campaign["protected_exact_functions"]):
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
    if not set(campaign["protected_exact_functions"]) <= set(
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
    if base_focus is not None and candidate_focus is not None:
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


FRONTIER_FIELDS = {
    "schema", "campaign_id", "manifest_sha256", "owner", "unit", "function",
    "source_relpath", "source_sha256", "target_object_sha256", "toolchain_sha256",
    "candidate_object_sha256", "metrics", "report_receipts",
    "focus_evidence_sha256", "parent_frontier_sha256",
    "generation", "retained_at", "frontier_sha256",
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
    return {**body, "frontier_sha256": _digest_json(body)}


def _validate_frontier(value: Any, campaign: Mapping[str, Any], function: str) -> dict[str, Any]:
    value = _closed_keys(value, FRONTIER_FIELDS, "frontier")
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
    metrics = _validate_metrics(value["metrics"])
    if metrics["protected_total"] != len(campaign["protected_exact_functions"]):
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
) -> dict[str, Any]:
    _check_cancelled(root, campaign)
    if function not in campaign["functions"]:
        raise CampaignError(f"function is outside campaign scope: {function}")

    # Establish a versioned read before doing the expensive measurement.  A
    # later writer may advance either source or latest while the hook runs;
    # the second locked read below treats that as a stale snapshot.
    with _frontier_lock_chain(root, campaign, function):
        _recover_pending_locked(root, campaign, function)
        initial_frontier = _read_latest_frontier(root, campaign, function)
        live_sha = _digest_file(campaign["_source"])
        if initial_frontier is not None:
            if initial_frontier["source_sha256"] != live_sha:
                raise CampaignError("latest frontier is inconsistent with live source")
            if not force:
                return initial_frontier
        initial_frontier_sha = (
            initial_frontier["frontier_sha256"] if initial_frontier is not None else None
        )

    scratch = _ensure_scratch(root, campaign)
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
    frontier = _frontier_from_measurement(campaign, function, measurement, parent=None)

    with _frontier_lock_chain(root, campaign, function):
        _recover_pending_locked(root, campaign, function)
        current_frontier = _read_latest_frontier(root, campaign, function)
        locked_live_sha = _digest_file(campaign["_source"])
        latest = _function_root(root, campaign, function) / "latest-frontier.json"

        if current_frontier is not None:
            if current_frontier["source_sha256"] != locked_live_sha:
                raise CampaignError("latest frontier is inconsistent with live source")
            if (
                current_frontier["frontier_sha256"] != initial_frontier_sha
                or locked_live_sha != live_sha
            ):
                # Another snapshot/retention won while this one was running.
                # Return the winner, never overwrite it with this stale result.
                return current_frontier
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
        _atomic_json(latest, frontier, limit=campaign["limits"]["frontier_bytes"])
        _gc_focus_evidence(root)
    _check_limits(root, campaign)
    return frontier


CANDIDATE_FIELDS = {
    "schema", "campaign_id", "function", "base_frontier_sha256",
    "candidate_source", "function_span", "hypothesis_family", "natural_c", "created_at",
    "candidate_sha256",
}
FUNCTION_SPAN_FIELDS = {
    "base_start_line", "base_end_line", "candidate_start_line",
    "candidate_end_line", "base_sha256", "candidate_sha256",
}


def _load_candidate(
    root: Path, path: Path, campaign: Mapping[str, Any], frontier: Mapping[str, Any],
) -> dict[str, Any]:
    path = _bound_path(root, str(path), "candidate descriptor")
    value = _closed_keys(_read_json(path, "candidate descriptor"), CANDIDATE_FIELDS, "candidate descriptor")
    body = dict(value)
    digest = _sha(body.pop("candidate_sha256", None), "candidate descriptor digest")
    if digest != _digest_json(body):
        raise CampaignError("candidate descriptor digest is invalid")
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
    base_bytes = campaign["_source"].read_bytes()
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
    if (
        base_lines[: base_start - 1] != candidate_lines[: candidate_start - 1]
        or base_lines[base_end:] != candidate_lines[candidate_end:]
    ):
        raise CampaignError("candidate edits escape the claimed function span")
    matcher = difflib.SequenceMatcher(a=base_lines, b=candidate_lines)
    added: list[str] = []
    changed = False
    for tag, _a0, _a1, b0, b1 in matcher.get_opcodes():
        if tag != "equal":
            if (
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
        or result.get("protected_total") != len(campaign["protected_exact_functions"])
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
        "protected_total": len(campaign["protected_exact_functions"]),
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
    if not set(campaign["protected_exact_functions"]) <= set(
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
    # Focus blobs are published and referenced while the same global CAS lock
    # is held.  GC must participate in that lock domain or it can delete a
    # blob between publication and the frontier reference being observed.
    with _exclusive_lock(
        _state_root(root) / "proof-cas" / "focus-cas.lock",
        _command_timeout_seconds(campaign),
    ):
        _gc_focus_evidence(root)
    scratch_root = (
        _state_root(root) / "scratch" / _slug(str(campaign["campaign_id"]))
    )
    scratch_size = _tree_size(scratch_root)
    if scratch_size > campaign["limits"]["scratch_soft_bytes"]:
        for repo in scratch_root.glob("repo-*"):
            if not repo.is_dir():
                continue
            for raw in campaign["allowed_build_paths"]:
                relative = Path(raw)
                target = repo / relative
                if target.is_dir() and _inside(repo, target):
                    shutil.rmtree(target)
                elif target.is_file() and _inside(repo, target):
                    target.unlink()
        scratch_size = _tree_size(scratch_root)
    if scratch_size > campaign["limits"]["scratch_hard_bytes"]:
        raise InfrastructureError("campaign scratch exceeds hard limit")
    owner_size = _tree_size(_owner_root(root, campaign))
    if owner_size > campaign["limits"]["owner_state_bytes"]:
        raise CampaignError("retained owner state exceeds hard limit")
    retained_global = (
        _tree_size(_state_root(root) / "owners")
        + _tree_size(_state_root(root) / "proof-cas")
    )
    if retained_global > 64 << 20:
        raise CampaignError("retained global campaign state exceeds 64 MiB")


def _cleanup_cell_outputs(
    scratch: Path, campaign: Mapping[str, Any]
) -> None:
    """Delete hook-owned raw cell output while preserving the worker checkout."""

    parents: set[Path] = set()
    for descriptor in campaign["commands"].values():
        output = _bound_path(
            scratch, descriptor["measurement_relpath"],
            "cell output cleanup", exists=False,
        )
        parents.add(output.parent)
    for parent in sorted(parents, key=lambda item: len(item.parts), reverse=True):
        if parent.exists() and parent != scratch and _inside(scratch, parent):
            shutil.rmtree(parent)


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
            "function_span": hint.get("function_span"),
        },
        "cleanup_status": "complete", "cleanup_errors": [],
        "authority_advanced": False,
    })


def _post_candidate_cleanup(
    root: Path, campaign: Mapping[str, Any], scratch: Path | None,
    candidate_paths: Sequence[Path], *, preserve_candidate: bool,
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
    try:
        _check_limits(root, campaign)
    except BaseException as exc:
        errors.append(f"campaign maintenance failed: {exc}")
    return errors


def run_candidate(
    root: Path, campaign: Mapping[str, Any], candidate_path: Path,
    *, worker: int = 0,
) -> dict[str, Any]:
    root = Path(os.path.abspath(root))
    _check_cancelled(root, campaign)
    candidate_hint = _read_json(_bound_path(root, str(candidate_path), "candidate descriptor"), "candidate descriptor")
    if not isinstance(candidate_hint, Mapping) or not isinstance(candidate_hint.get("function"), str):
        raise CampaignError("candidate descriptor function is missing")
    function = candidate_hint["function"]
    frontier = snapshot_frontier(root, campaign, function)
    if candidate_hint.get("base_frontier_sha256") != frontier["frontier_sha256"]:
        return _rebase_outcome(root, Path(candidate_path), candidate_hint, frontier)
    candidate = _load_candidate(root, candidate_path, campaign, frontier)
    key = _candidate_key(campaign, frontier, candidate)
    if not _reserve_candidate(
        root, campaign, function, key, frontier, candidate["_source_sha256"]
    ):
        _cleanup_candidate_artifacts(
            root, campaign, [candidate["_path"], candidate["_source"]]
        )
        return {"schema": "owner_campaign_result/v1", "status": "deduplicated", "candidate_key": key, "function": function, "authority_advanced": False}
    scratch: Path | None = None
    reservation_finished = False
    result: dict[str, Any] | None = None
    preserve_candidate = False
    candidate_paths = [candidate["_path"], candidate["_source"]]
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
            root, campaign, scratch, candidate_paths, preserve_candidate=True
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
    )
    if cleanup_errors:
        if result["status"] not in {"exact", "improved"}:
            raise CampaignError("terminal cleanup failed: " + "; ".join(cleanup_errors))
        result["cleanup_status"] = "cleanup_incomplete"
        result["cleanup_errors"] = cleanup_errors[:8]
        result = _sealed_outcome(result)
    return result


def campaign_status(root: Path, campaign: Mapping[str, Any]) -> dict[str, Any]:
    owner = _owner_root(root, campaign)
    exact_manifest = owner / "exact-manifest.json"
    exact: Mapping[str, Any] = {}
    functions: dict[str, Any] = {}
    focus_evidence: dict[str, Any] = {}
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
    # Establish each current baseline exactly once before workers race candidates,
    # and deterministically assign identical cells to their first input slot.
    hints: list[Mapping[str, Any]] = []
    signatures: dict[tuple[str, str, str], int] = {}
    duplicate_of: dict[int, int] = {}
    active_indices: list[int] = []
    for index, path in enumerate(paths):
        hint = _read_json(_bound_path(root, str(path), "candidate descriptor"), "candidate descriptor")
        if not isinstance(hint, Mapping) or not isinstance(hint.get("function"), str):
            raise CampaignError("candidate descriptor function is missing")
        snapshot_frontier(root, campaign, hint["function"])
        hints.append(hint)
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

    def run_group(worker: int) -> list[tuple[int, dict[str, Any]]]:
        group: list[tuple[int, dict[str, Any]]] = []
        for offset in range(worker, len(active_indices), worker_count):
            index = active_indices[offset]
            path = paths[index]
            _check_cancelled(root, campaign)
            try:
                result = run_candidate(root, campaign, path, worker=worker)
            except InfrastructureError as exc:
                result = {
                    "schema": "owner_campaign_result/v1", "status": "infra_retry",
                    "candidate": str(path), "reason": str(exc)[:1000],
                    "authority_advanced": False,
                }
            group.append((index, result))
        return group

    with ThreadPoolExecutor(max_workers=worker_count) as executor:
        for group in executor.map(run_group, range(worker_count)):
            indexed_results.extend(group)
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
            if isinstance(binding, Mapping) and isinstance(binding.get("path"), str):
                cleanup.append(Path(binding["path"]))
            _cleanup_candidate_artifacts(root, campaign, cleanup)
        indexed_results.append((index, duplicate))
    indexed_results.sort(key=lambda item: item[0])
    return [item[1] for item in indexed_results]


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
