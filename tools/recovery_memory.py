#!/usr/bin/env python3
"""Canonical cross-worktree recovery experiment and crack-report memory.

The recovery queue answers *who owns work*.  This module answers *what has
already been tried or proved* across every worktree that shares that queue.
The database deliberately lives beside the canonical queue in Git's common
directory, never under a lane-local ``build`` directory.

The registry is diagnostic and admission authority only.  It does not compile,
edit recovered source, retain candidates, close owners, or promote branches.
"""

from __future__ import annotations

import argparse
import contextlib
import hashlib
import hmac
import json
import os
import re
import sqlite3
import stat
import subprocess
import uuid
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import Any, Iterable, Mapping, Sequence

from tools.agent_queue import (
    QUEUE_ENV,
    QueueError,
    canonical_queue_path,
    git_common_dir,
    queue_location_audit,
    queue_path,
    read_queue,
)
from tools.git_paths import native_git_path


MEMORY_SCHEMA = "recovery_memory/v1"
MEMORY_SCHEMA_VERSION = 2
MEMORY_ENV = "MP6_RECOVERY_MEMORY"
MAX_RECOVERY_MEMORY_BYTES = 64 * 1024 * 1024
MAX_RECOVERY_DATABASE_BYTES = MAX_RECOVERY_MEMORY_BYTES - 1024 * 1024
MAX_COMPACT_FIELD_BYTES = 64 * 1024
REPARSE_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
CENTRAL_REDIRECT_ENV = (
    MEMORY_ENV, QUEUE_ENV, "GIT_DIR", "GIT_COMMON_DIR", "GIT_WORK_TREE",
    # Git sets GIT_INDEX_FILE while invoking hooks.  It selects the pending
    # index but cannot redirect the canonical common-dir queue/memory paths.
    "GIT_OBJECT_DIRECTORY", "GIT_ALTERNATE_OBJECT_DIRECTORIES",
)
ADMISSION_TTL_HOURS = 24
PERMANENT_RECOVERY_REF = "refs/remotes/origin/agent/recovery-context-workflow"
SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
MATCH_SESSION_SCHEMA = "match_workbench_session/v1"
MATCH_INDEX_SCHEMA = "match_workbench_index/v1"
MATCH_CANDIDATE_SCHEMA = "match_workbench_candidate/v1"
MATCH_REQUEST_SCHEMA = "match_workbench_request/v1"
WORKBENCH_SEARCH_DIRS = (".agent-coordination/match", "work", "build")
WORKBENCH_SEARCH_MAX_DEPTH = 6
WORKBENCH_SEARCH_MAX_COUNT = 4096
WORKBENCH_PRUNE_NAMES = frozenset(
    {
        ".git",
        "__pycache__",
        "cas",
        "candidates",
        "diagnostics",
        "gp6e01",
        "include",
        "obj",
        "src",
        "tools",
    }
)
NEGATIVE_STATUSES = frozenset(
    {
        "blocked",
        "failed",
        "neutral",
        "no-go",
        "nogo",
        "nonexact",
        "regressed",
        "rejected",
    }
)
EXACT_STATUSES = frozenset(
    {"exact", "retained", "pass", "closed", "landed", "complete"}
)


class RecoveryMemoryError(ValueError):
    """Invalid, stale, duplicate, or conflicting recovery memory input."""


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _parse_time(value: str) -> datetime:
    try:
        parsed = datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as exc:
        raise RecoveryMemoryError(f"invalid UTC timestamp {value!r}") from exc
    if parsed.tzinfo is None:
        raise RecoveryMemoryError(f"timestamp lacks timezone: {value!r}")
    return parsed.astimezone(timezone.utc)


def _canonical(value: Any) -> bytes:
    try:
        return (
            json.dumps(
                value,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
                allow_nan=False,
            )
            + "\n"
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise RecoveryMemoryError(f"cannot serialize canonical JSON: {exc}") from exc


def _digest(value: Any) -> str:
    return hashlib.sha256(_canonical(value)).hexdigest()


def _load_json_object(path: Path, label: str) -> dict[str, Any]:
    if not path.is_file() or path.is_symlink():
        raise RecoveryMemoryError(f"{label} must be a regular non-symlink file: {path}")
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise RecoveryMemoryError(f"cannot read {label} {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise RecoveryMemoryError(f"{label} must be a JSON object: {path}")
    return value


def _verify_self_hash(
    value: Mapping[str, Any], field: str, label: str
) -> str:
    claimed = _sha(value.get(field), f"{label}.{field}")
    body = dict(value)
    body.pop(field, None)
    actual = _digest(body)
    if actual != claimed:
        raise RecoveryMemoryError(
            f"{label}.{field} does not match its canonical payload"
        )
    return claimed


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _required_text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip() or "\x00" in value:
        raise RecoveryMemoryError(f"{label} must be a non-empty string")
    return value.strip()


def _compact_text(value: Any, label: str) -> str:
    text = _required_text(value, label)
    if len(text.encode("utf-8")) > MAX_COMPACT_FIELD_BYTES:
        raise RecoveryMemoryError(f"{label} exceeds the compact 64 KiB field cap")
    return text


def _canonical_owner(value: Any, label: str = "owner") -> str:
    owner = _required_text(value, label)
    if owner.startswith("main:board/") and "#" in owner:
        owner = owner.split("#", 1)[0]
    return owner


def _optional_text(value: Any, label: str) -> str | None:
    if value is None:
        return None
    return _required_text(value, label)


def _sha(value: Any, label: str, *, optional: bool = False) -> str | None:
    if value is None and optional:
        return None
    text = _required_text(value, label).lower()
    if not SHA256_RE.fullmatch(text):
        raise RecoveryMemoryError(f"{label} must be a lowercase SHA-256")
    return text


def _status(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", _required_text(value, "status").lower()).strip("-")


def _git(root: Path, *args: str, check: bool = True) -> str:
    process = subprocess.run(
        ["git", *args],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    if check and process.returncode:
        raise RecoveryMemoryError(
            process.stderr.strip() or "git command failed: git " + " ".join(args)
        )
    return process.stdout.strip()


def is_git_worktree(root: Path) -> bool:
    process = subprocess.run(
        ["git", "rev-parse", "--is-inside-work-tree"],
        cwd=root,
        text=True,
        capture_output=True,
        check=False,
    )
    return process.returncode == 0 and process.stdout.strip() == "true"


def _canonical_workflow_root(value: str | Path) -> Path:
    """Return an authenticated, canonical Git worktree root.

    Unlike the lane root, a separately supplied workflow root is a trust input.
    It may not rely on the caller's current directory, a symlink, a parent
    search, or a subdirectory alias.
    """

    supplied = Path(value)
    if not supplied.is_absolute():
        raise RecoveryMemoryError("workflow root must be an absolute canonical path")
    try:
        resolved = supplied.resolve(strict=True)
    except OSError as exc:
        raise RecoveryMemoryError(
            f"workflow root does not exist or cannot be resolved: {supplied}"
        ) from exc
    if supplied != resolved or supplied.is_symlink():
        raise RecoveryMemoryError(
            f"workflow root must not use a symlink or path alias: {supplied}"
        )
    if not resolved.is_dir() or not is_git_worktree(resolved):
        raise RecoveryMemoryError(
            f"workflow root is not a Git worktree: {resolved}"
        )
    top_level = native_git_path(
        _git(resolved, "rev-parse", "--show-toplevel"),
        relative_to=resolved,
    ).resolve(strict=True)
    if top_level != resolved:
        raise RecoveryMemoryError(
            f"workflow root must be the canonical Git worktree root: {resolved}"
        )
    return resolved


def _canonical_git_object_directory(root: Path) -> Path:
    """Resolve the canonical Git object directory used as a read alternate."""

    raw = native_git_path(
        _git(
            root,
            "rev-parse",
            "--path-format=absolute",
            "--git-path",
            "objects",
        ),
        relative_to=root,
    )
    try:
        resolved = raw.resolve(strict=True)
    except OSError as exc:
        raise RecoveryMemoryError(
            f"workflow Git object directory is unavailable: {raw}"
        ) from exc
    if raw != resolved or raw.is_symlink() or not resolved.is_dir():
        raise RecoveryMemoryError(
            f"workflow Git object directory is not canonical: {raw}"
        )
    return resolved


def _workflow_alternate_environment(workflow: Path) -> dict[str, str]:
    """Expose workflow objects read-only without replacing caller alternates."""

    objects = _canonical_git_object_directory(workflow)
    environment = os.environ.copy()
    existing = environment.get("GIT_ALTERNATE_OBJECT_DIRECTORIES")
    environment["GIT_ALTERNATE_OBJECT_DIRECTORIES"] = (
        str(objects) + os.pathsep + existing if existing else str(objects)
    )
    return environment


def _workflow_metadata_hashes(
    root: Path,
    data: Mapping[str, Any],
    *,
    require_freshness: bool,
) -> dict[str, str]:
    """Hash the authenticated recovery metadata supplied by ``root``.

    Explicit workflow roots are required to supply tracked, HEAD-identical
    metadata.  The omitted-option path retains the historical permissive
    freshness-file behavior for compatibility.
    """

    files = data.get("project", {}).get("files", {})
    patterns_relative = (
        str(
            files.get(
                "compiler_patterns",
                "config/recovery/compiler_patterns.json",
            )
        )
        if isinstance(files, Mapping)
        else "config/recovery/compiler_patterns.json"
    )
    freshness_relative = (
        str(
            files.get(
                "knowledge_freshness",
                "config/recovery/knowledge_freshness.json",
            )
        )
        if isinstance(files, Mapping)
        else "config/recovery/knowledge_freshness.json"
    )
    if not require_freshness:
        return {
            str((root / relative).relative_to(root)): _sha256_file(root / relative)
            for relative in (patterns_relative, freshness_relative)
            if (root / relative).is_file()
        }

    project = root / "config/recovery/project.json"
    if not project.is_file() or project.is_symlink():
        raise RecoveryMemoryError(
            f"workflow root is missing regular metadata config/recovery/project.json: {root}"
        )
    metadata_paths = {Path(path) for path in data.get("metadata_paths", [])}
    freshness_path = root / freshness_relative
    if freshness_path.is_file():
        metadata_paths.add(freshness_path)
    elif require_freshness:
        raise RecoveryMemoryError(
            f"workflow root is missing knowledge metadata {freshness_relative}: {root}"
        )

    hashes: dict[str, str] = {}
    relative_paths: list[str] = []
    for path in sorted(metadata_paths, key=lambda item: str(item).casefold()):
        if not path.is_file() or path.is_symlink():
            raise RecoveryMemoryError(
                f"workflow metadata must be a regular non-symlink file: {path}"
            )
        try:
            relative = path.resolve(strict=True).relative_to(root).as_posix()
        except (OSError, ValueError) as exc:
            raise RecoveryMemoryError(
                f"workflow metadata escapes the workflow root: {path}"
            ) from exc
        if Path(relative).is_absolute() or ".." in Path(relative).parts:
            raise RecoveryMemoryError(
                f"workflow metadata path is not canonical: {path}"
            )
        hashes[relative] = _sha256_file(path)
        relative_paths.append(relative)

    if require_freshness:
        for relative in relative_paths:
            tracked = subprocess.run(
                ["git", "ls-files", "--error-unmatch", "--", relative],
                cwd=root,
                text=True,
                capture_output=True,
                check=False,
            )
            if tracked.returncode:
                raise RecoveryMemoryError(
                    f"workflow metadata is not tracked at HEAD: {relative}"
                )
            head_blob = _git(root, "rev-parse", f"HEAD:{relative}")
            worktree_blob = _git(
                root,
                "hash-object",
                f"--path={relative}",
                relative,
            )
            if worktree_blob != head_blob:
                raise RecoveryMemoryError(
                    f"workflow metadata does not match authenticated HEAD: {relative}"
                )
        status = _git(
            root,
            "status",
            "--porcelain=v1",
            "--untracked-files=all",
            "--",
            *relative_paths,
        )
        if status:
            raise RecoveryMemoryError(
                "workflow metadata differs from authenticated HEAD:\n" + status
            )
    return dict(sorted(hashes.items()))


def _invalid_central_redirect_environment(root: Path) -> tuple[str, ...]:
    """Return environment redirects that do not describe this worktree.

    Git legitimately exports ``GIT_DIR`` while running hooks.  Accept only
    Git's own canonical directory/worktree values; explicit queue, memory, or
    object-store redirects remain forbidden.
    """

    invalid = [
        name
        for name in (
            MEMORY_ENV,
            QUEUE_ENV,
            "GIT_OBJECT_DIRECTORY",
            "GIT_ALTERNATE_OBJECT_DIRECTORIES",
        )
        if os.environ.get(name)
    ]
    checks = {
        "GIT_DIR": "--git-dir",
        "GIT_COMMON_DIR": "--git-common-dir",
        "GIT_WORK_TREE": "--show-toplevel",
    }
    clean_env = dict(os.environ)
    for name in CENTRAL_REDIRECT_ENV:
        clean_env.pop(name, None)
    for name, argument in checks.items():
        raw = os.environ.get(name)
        if not raw:
            continue
        process = subprocess.run(
            ["git", "rev-parse", "--path-format=absolute", argument],
            cwd=root,
            text=True,
            capture_output=True,
            check=False,
            env=clean_env,
        )
        if process.returncode != 0:
            invalid.append(name)
            continue
        expected = native_git_path(process.stdout.strip(), relative_to=root).resolve()
        actual = native_git_path(raw, relative_to=root).resolve()
        if actual != expected:
            invalid.append(name)
    return tuple(sorted(set(invalid)))


def recovery_memory_path(
    root: Path, override: str | Path | None = None
) -> Path:
    root = root.resolve()
    redirected = _invalid_central_redirect_environment(root)
    if override is not None or redirected:
        raise RecoveryMemoryError(
            "central recovery queue/memory paths are fixed; overrides are forbidden"
            + (f" ({', '.join(redirected)})" if redirected else "")
        )
    return git_common_dir(root.resolve()) / "agent-coordination" / "recovery-memory.sqlite3"


def _assert_plain_components(path: Path, *, missing_leaf: bool = False) -> None:
    absolute = Path(os.path.abspath(path))
    current = Path(absolute.parts[0])
    for index, part in enumerate(absolute.parts[1:], 1):
        current /= part
        try:
            info = current.lstat()
        except FileNotFoundError:
            if missing_leaf:
                return
            raise RecoveryMemoryError(f"central path component is missing: {current}")
        if stat.S_ISLNK(info.st_mode) or bool(
            getattr(info, "st_file_attributes", 0) & REPARSE_ATTRIBUTE
        ):
            raise RecoveryMemoryError(f"central path indirection is forbidden: {current}")


def recovery_memory_available(root: Path) -> bool:
    """Return whether global memory is enforceable for this root.

    Standalone match-workbench fixtures intentionally operate outside Git.  A
    real recovery lane is always a Git worktree and therefore must use the
    central registry.
    """

    return is_git_worktree(root.resolve())


class RecoveryMemory:
    """Small SQLite-backed canonical registry shared by all Board lanes."""

    def __init__(self, path: Path):
        raw_path = Path(os.path.abspath(path))
        _assert_plain_components(raw_path, missing_leaf=True)
        self.path = raw_path
        self.path.parent.mkdir(parents=True, exist_ok=True)
        _assert_plain_components(self.path.parent)
        if self._storage_bytes() > MAX_RECOVERY_MEMORY_BYTES:
            raise RecoveryMemoryError("central recovery memory exceeds the hard 64 MiB cap")
        self._initialize()

    @classmethod
    def for_root(
        cls, root: Path, override: str | Path | None = None
    ) -> "RecoveryMemory":
        root = root.resolve()
        redirected = _invalid_central_redirect_environment(root)
        if override is not None or redirected:
            raise RecoveryMemoryError(
                "central recovery queue/memory paths are fixed; overrides are forbidden"
                + (f" ({', '.join(redirected)})" if redirected else "")
            )
        # Normal use must fail while a shadow queue exists.  This couples the
        # registry to exactly one queue namespace.
        queue_path(root)
        return cls(recovery_memory_path(root, override))

    def _storage_bytes(self) -> int:
        return sum(
            item.stat().st_size
            for item in (
                self.path, Path(str(self.path) + "-wal"), Path(str(self.path) + "-shm")
            )
            if item.is_file()
        )

    def _connect(self) -> sqlite3.Connection:
        connection = sqlite3.connect(self.path, timeout=15.0)
        connection.row_factory = sqlite3.Row
        connection.execute("PRAGMA foreign_keys = ON")
        connection.execute("PRAGMA busy_timeout = 15000")
        connection.execute("PRAGMA journal_mode = DELETE")
        connection.execute("PRAGMA synchronous = FULL")
        page_size = int(connection.execute("PRAGMA page_size").fetchone()[0])
        connection.execute(
            f"PRAGMA max_page_count = {MAX_RECOVERY_DATABASE_BYTES // page_size}"
        )
        if self._storage_bytes() > MAX_RECOVERY_MEMORY_BYTES:
            connection.close()
            raise RecoveryMemoryError("central recovery memory exceeds the hard 64 MiB cap")
        return connection

    def _initialize(self) -> None:
        with contextlib.closing(self._connect()) as connection, connection:
            connection.executescript(
                """
                CREATE TABLE IF NOT EXISTS metadata (
                    key TEXT PRIMARY KEY,
                    value TEXT NOT NULL
                );
                CREATE TABLE IF NOT EXISTS admissions (
                    input_key TEXT PRIMARY KEY,
                    token TEXT NOT NULL UNIQUE,
                    owner TEXT NOT NULL,
                    function_name TEXT NOT NULL,
                    base_commit TEXT NOT NULL,
                    toolchain_key TEXT NOT NULL,
                    target_sha256 TEXT NOT NULL,
                    compiler_sha256 TEXT,
                    context_key TEXT NOT NULL,
                    source_sha256 TEXT NOT NULL,
                    shape_key TEXT,
                    constraint_key TEXT,
                    hypothesis TEXT,
                    axis TEXT,
                    requester TEXT NOT NULL,
                    source_path TEXT,
                    state TEXT NOT NULL,
                    created_at TEXT NOT NULL,
                    expires_at TEXT NOT NULL,
                    consumed_at TEXT
                );
                CREATE INDEX IF NOT EXISTS admissions_constraint_idx
                    ON admissions(constraint_key, state);
                CREATE TABLE IF NOT EXISTS experiments (
                    input_key TEXT PRIMARY KEY,
                    owner TEXT NOT NULL,
                    function_name TEXT NOT NULL,
                    base_commit TEXT NOT NULL,
                    toolchain_key TEXT NOT NULL,
                    target_sha256 TEXT NOT NULL,
                    compiler_sha256 TEXT,
                    context_key TEXT NOT NULL,
                    source_sha256 TEXT NOT NULL,
                    shape_key TEXT,
                    constraint_key TEXT,
                    hypothesis TEXT,
                    axis TEXT,
                    object_sha256 TEXT NOT NULL,
                    status TEXT NOT NULL,
                    is_negative INTEGER NOT NULL,
                    reason TEXT NOT NULL,
                    candidate_id TEXT,
                    candidate_record_sha256 TEXT,
                    strict_report_sha256 TEXT,
                    data_report_sha256 TEXT,
                    report_sha256 TEXT,
                    workspace TEXT,
                    source_path TEXT,
                    recorded_at TEXT NOT NULL,
                    record_sha256 TEXT NOT NULL
                );
                CREATE INDEX IF NOT EXISTS experiments_owner_function_idx
                    ON experiments(owner, function_name, recorded_at);
                CREATE INDEX IF NOT EXISTS experiments_constraint_idx
                    ON experiments(constraint_key, is_negative);
                CREATE TABLE IF NOT EXISTS experiment_observations (
                    candidate_record_sha256 TEXT PRIMARY KEY,
                    input_key TEXT NOT NULL REFERENCES experiments(input_key),
                    object_sha256 TEXT NOT NULL,
                    status TEXT NOT NULL,
                    reason TEXT NOT NULL,
                    candidate_id TEXT,
                    strict_report_sha256 TEXT,
                    data_report_sha256 TEXT,
                    workspace TEXT NOT NULL,
                    source_path TEXT,
                    observed_at TEXT NOT NULL
                );
                CREATE INDEX IF NOT EXISTS experiment_observations_input_idx
                    ON experiment_observations(input_key, object_sha256);
                CREATE TABLE IF NOT EXISTS reports (
                    report_sha256 TEXT PRIMARY KEY,
                    owner TEXT NOT NULL,
                    function_name TEXT NOT NULL,
                    report_path TEXT NOT NULL,
                    report_format TEXT NOT NULL,
                    exact INTEGER NOT NULL,
                    payload_json TEXT NOT NULL,
                    ingested_at TEXT NOT NULL,
                    record_sha256 TEXT NOT NULL
                );
                CREATE INDEX IF NOT EXISTS reports_owner_function_idx
                    ON reports(owner, function_name, ingested_at);
                CREATE TABLE IF NOT EXISTS report_constraints (
                    constraint_key TEXT PRIMARY KEY,
                    report_sha256 TEXT NOT NULL REFERENCES reports(report_sha256),
                    sequence_no INTEGER NOT NULL,
                    kind TEXT NOT NULL,
                    status TEXT,
                    payload_json TEXT NOT NULL
                );
                CREATE INDEX IF NOT EXISTS report_constraints_report_idx
                    ON report_constraints(report_sha256, sequence_no);
                CREATE TABLE IF NOT EXISTS lane_snapshots (
                    lane_root TEXT PRIMARY KEY,
                    branch TEXT NOT NULL,
                    head_commit TEXT NOT NULL,
                    permanent_ref TEXT NOT NULL,
                    permanent_commit TEXT NOT NULL,
                    queue_path TEXT NOT NULL,
                    memory_path TEXT NOT NULL,
                    knowledge_sha256 TEXT NOT NULL,
                    checked_at TEXT NOT NULL,
                    snapshot_sha256 TEXT NOT NULL
                );
                """
            )
            existing = connection.execute(
                "SELECT value FROM metadata WHERE key = 'schema_version'"
            ).fetchone()
            if existing is not None and int(existing["value"]) not in {
                1,
                MEMORY_SCHEMA_VERSION,
            }:
                raise RecoveryMemoryError(
                    "unsupported recovery memory schema version " + existing["value"]
                )
            connection.execute(
                "INSERT OR REPLACE INTO metadata(key, value) VALUES(?, ?)",
                ("schema", MEMORY_SCHEMA),
            )
            connection.execute(
                "INSERT OR REPLACE INTO metadata(key, value) VALUES(?, ?)",
                ("schema_version", str(MEMORY_SCHEMA_VERSION)),
            )

    @staticmethod
    def identity(
        *,
        owner: str,
        function: str,
        base_commit: str,
        toolchain_key: str,
        target_sha256: str,
        source_sha256: str,
        compiler_sha256: str | None = None,
        context_key: str | None = None,
        shape_key: str | None = None,
        hypothesis: str | None = None,
        axis: str | None = None,
    ) -> dict[str, Any]:
        owner_value = _canonical_owner(owner)
        _compact_text(owner_value, "owner")
        function_value = _compact_text(function, "function")
        base_value = _compact_text(base_commit, "base_commit")
        toolchain_value = _compact_text(toolchain_key, "toolchain_key")
        target_value = _sha(target_sha256, "target_sha256")
        source_value = _sha(source_sha256, "source_sha256")
        compiler_value = _sha(
            compiler_sha256, "compiler_sha256", optional=True
        )
        context_value = context_key or _digest(
            {
                "base_commit": base_value,
                "toolchain_key": toolchain_value,
                "target_sha256": target_value,
                "compiler_sha256": compiler_value,
            }
        )
        _sha(context_value, "context_key")
        shape_value = _optional_text(shape_key, "shape_key")
        hypothesis_value = _optional_text(hypothesis, "hypothesis")
        axis_value = _optional_text(axis, "axis")
        core = {
            "owner": owner_value,
            "function": function_value,
            "base_commit": base_value,
            "toolchain_key": toolchain_value,
            "target_sha256": target_value,
            "compiler_sha256": compiler_value,
            "context_key": context_value,
            "source_sha256": source_value,
        }
        input_key = _digest(core)
        constraint_key = None
        if shape_value:
            constraint_key = _digest(
                {
                    key: core[key]
                    for key in (
                        "owner",
                        "function",
                        "base_commit",
                        "toolchain_key",
                        "target_sha256",
                        "compiler_sha256",
                        "context_key",
                    )
                }
                | {"shape_key": shape_value}
            )
        return {
            **core,
            "input_key": input_key,
            "shape_key": shape_value,
            "constraint_key": constraint_key,
            "hypothesis": hypothesis_value,
            "axis": axis_value,
        }

    def admit(
        self,
        identity: Mapping[str, Any],
        *,
        requester: str,
        source_path: str | None = None,
    ) -> dict[str, Any]:
        requester_value = _compact_text(requester, "requester")
        if source_path is not None:
            _compact_text(source_path, "source_path")
        now = datetime.now(timezone.utc).replace(microsecond=0)
        expires = now + timedelta(hours=ADMISSION_TTL_HOURS)
        input_key = _sha(identity.get("input_key"), "input_key")
        with contextlib.closing(self._connect()) as connection, connection:
            connection.execute("BEGIN IMMEDIATE")
            connection.execute(
                "UPDATE admissions SET state='expired' "
                "WHERE state='pending' AND expires_at < ?",
                (now.isoformat(),),
            )
            connection.execute("DELETE FROM admissions WHERE state <> 'pending'")
            connection.execute(
                "DELETE FROM admissions WHERE rowid NOT IN "
                "(SELECT rowid FROM admissions ORDER BY created_at DESC, rowid DESC LIMIT 1024)"
            )
            connection.execute(
                "DELETE FROM admissions WHERE owner=? AND function_name=? AND rowid NOT IN "
                "(SELECT rowid FROM admissions WHERE owner=? AND function_name=? "
                "ORDER BY created_at DESC, rowid DESC LIMIT 1)",
                (identity["owner"], identity["function"], identity["owner"], identity["function"]),
            )
            experiment = connection.execute(
                "SELECT * FROM experiments WHERE input_key = ?", (input_key,)
            ).fetchone()
            if experiment is not None:
                if experiment["status"] == "historical-conflict":
                    observations = [
                        dict(item)
                        for item in connection.execute(
                            "SELECT * FROM experiment_observations "
                            "WHERE input_key=? ORDER BY observed_at, "
                            "candidate_record_sha256",
                            (input_key,),
                        ).fetchall()
                    ]
                    return {
                        "status": "conflicting_historical_source",
                        "skip_compile": True,
                        "input_key": input_key,
                        "experiment": dict(experiment),
                        "observations": observations,
                        "reason": experiment["reason"],
                        "authority_advanced": False,
                    }
                return {
                    "status": "known_global_source",
                    "skip_compile": True,
                    "input_key": input_key,
                    "experiment": dict(experiment),
                    "authority_advanced": False,
                }
            constraint_key = identity.get("constraint_key")
            if constraint_key:
                negative = connection.execute(
                    "SELECT * FROM experiments WHERE constraint_key = ? "
                    "AND is_negative = 1 ORDER BY recorded_at DESC LIMIT 1",
                    (constraint_key,),
                ).fetchone()
                if negative is not None:
                    return {
                        "status": "known_negative_shape",
                        "skip_compile": True,
                        "input_key": input_key,
                        "constraint_key": constraint_key,
                        "experiment": dict(negative),
                        "authority_advanced": False,
                    }
            pending = connection.execute(
                "SELECT * FROM admissions WHERE input_key = ?", (input_key,)
            ).fetchone()
            if pending is not None and pending["state"] == "pending":
                if pending["requester"] == requester_value:
                    return {
                        "status": "admitted",
                        "reused": True,
                        "skip_compile": False,
                        "input_key": input_key,
                        "admission_token": pending["token"],
                        "expires_at": pending["expires_at"],
                        "authority_advanced": False,
                    }
                return {
                    "status": "pending_in_other_lane",
                    "skip_compile": True,
                    "input_key": input_key,
                    "requester": pending["requester"],
                    "expires_at": pending["expires_at"],
                    "authority_advanced": False,
                }
            owner_pending = connection.execute(
                "SELECT * FROM admissions WHERE owner=? AND function_name=? "
                "AND state='pending' LIMIT 1",
                (identity["owner"], identity["function"]),
            ).fetchone()
            if owner_pending is not None:
                if owner_pending["requester"] != requester_value:
                    return {
                        "status": "pending_in_other_lane", "skip_compile": True,
                        "input_key": owner_pending["input_key"],
                        "requester": owner_pending["requester"],
                        "expires_at": owner_pending["expires_at"],
                        "authority_advanced": False,
                    }
                connection.execute(
                    "DELETE FROM admissions WHERE owner=? AND function_name=?",
                    (identity["owner"], identity["function"]),
                )
            token = uuid.uuid4().hex
            values = (
                input_key,
                token,
                identity["owner"],
                identity["function"],
                identity["base_commit"],
                identity["toolchain_key"],
                identity["target_sha256"],
                identity.get("compiler_sha256"),
                identity["context_key"],
                identity["source_sha256"],
                identity.get("shape_key"),
                identity.get("constraint_key"),
                identity.get("hypothesis"),
                identity.get("axis"),
                requester_value,
                source_path,
                "pending",
                now.isoformat(),
                expires.isoformat(),
            )
            connection.execute(
                """
                INSERT OR REPLACE INTO admissions(
                    input_key, token, owner, function_name, base_commit,
                    toolchain_key, target_sha256, compiler_sha256, context_key,
                    source_sha256, shape_key, constraint_key, hypothesis, axis,
                    requester, source_path, state, created_at, expires_at,
                    consumed_at
                ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NULL)
                """,
                values,
            )
        return {
            "status": "admitted",
            "reused": False,
            "skip_compile": False,
            "input_key": input_key,
            "admission_token": token,
            "expires_at": expires.isoformat(),
            "authority_advanced": False,
        }

    def record(
        self,
        identity: Mapping[str, Any],
        *,
        requester: str,
        object_sha256: str,
        status: str,
        reason: str,
        admission_token: str | None = None,
        candidate_id: str | None = None,
        candidate_record_sha256: str | None = None,
        strict_report_sha256: str | None = None,
        data_report_sha256: str | None = None,
        report_sha256: str | None = None,
        workspace: str | None = None,
        source_path: str | None = None,
    ) -> dict[str, Any]:
        requester_value = _compact_text(requester, "requester")
        object_value = _sha(object_sha256, "object_sha256")
        status_value = _status(status)
        if status_value not in {"improved", "exact"}:
            raise RecoveryMemoryError(
                "central recovery memory records retained improved/exact outcomes only"
            )
        reason_value = _compact_text(reason, "reason")
        for label, value in (
            ("candidate_id", candidate_id), ("workspace", workspace),
            ("source_path", source_path),
        ):
            if value is not None:
                _compact_text(value, label)
        input_key = _sha(identity.get("input_key"), "input_key")
        now = _now()
        with contextlib.closing(self._connect()) as connection, connection:
            connection.execute("BEGIN IMMEDIATE")
            existing = connection.execute(
                "SELECT * FROM experiments WHERE input_key = ?", (input_key,)
            ).fetchone()
            if existing is not None:
                if (
                    existing["object_sha256"] == object_value
                    and existing["source_sha256"] == identity["source_sha256"]
                ):
                    return {
                        "status": "unchanged",
                        "experiment": dict(existing),
                        "authority_advanced": False,
                    }
                raise RecoveryMemoryError(
                    "same global source/context produced a different object; "
                    "the compiler context is incomplete or nondeterministic"
                )
            admission = connection.execute(
                "SELECT * FROM admissions WHERE input_key = ?", (input_key,)
            ).fetchone()
            if admission is None or admission["state"] != "pending":
                raise RecoveryMemoryError(
                    "candidate has no pending central pre-compile admission; "
                    "run match lookup or candidate_compile_admission.py admit first"
                )
            if admission["requester"] != requester_value:
                raise RecoveryMemoryError(
                    "candidate admission belongs to another lane: "
                    + admission["requester"]
                )
            if admission_token is not None and admission["token"] != admission_token:
                raise RecoveryMemoryError("candidate admission token does not match")
            if _parse_time(admission["expires_at"]) < datetime.now(timezone.utc):
                connection.execute(
                    "UPDATE admissions SET state='expired' WHERE input_key = ?",
                    (input_key,),
                )
                raise RecoveryMemoryError("candidate admission expired; run lookup again")
            effective_shape = identity.get("shape_key") or admission["shape_key"]
            effective_constraint = (
                identity.get("constraint_key") or admission["constraint_key"]
            )
            effective_hypothesis = (
                identity.get("hypothesis") or admission["hypothesis"]
            )
            effective_axis = identity.get("axis") or admission["axis"]
            stale_keys = [
                row["input_key"] for row in connection.execute(
                    "SELECT input_key FROM experiments WHERE owner=? AND function_name=? "
                    "AND input_key<>?",
                    (identity["owner"], identity["function"], input_key),
                )
            ]
            for stale_key in stale_keys:
                connection.execute(
                    "DELETE FROM experiment_observations WHERE input_key=?", (stale_key,)
                )
            connection.execute(
                "DELETE FROM experiments WHERE owner=? AND function_name=? AND input_key<>?",
                (identity["owner"], identity["function"], input_key),
            )
            record_body = {
                "schema": "recovery_experiment/v1",
                "input_key": input_key,
                "owner": identity["owner"],
                "function": identity["function"],
                "base_commit": identity["base_commit"],
                "toolchain_key": identity["toolchain_key"],
                "target_sha256": identity["target_sha256"],
                "compiler_sha256": identity.get("compiler_sha256"),
                "context_key": identity["context_key"],
                "source_sha256": identity["source_sha256"],
                "shape_key": effective_shape,
                "constraint_key": effective_constraint,
                "hypothesis": effective_hypothesis,
                "axis": effective_axis,
                "object_sha256": object_value,
                "status": status_value,
                "reason": reason_value,
                "candidate_id": candidate_id,
                "candidate_record_sha256": candidate_record_sha256,
                "strict_report_sha256": strict_report_sha256,
                "data_report_sha256": data_report_sha256,
                "report_sha256": report_sha256,
                "workspace": workspace,
                "source_path": source_path,
                "recorded_at": now,
                "authority_advanced": False,
            }
            record_sha = _digest(record_body)
            negative = 1 if status_value in NEGATIVE_STATUSES else 0
            connection.execute(
                """
                INSERT INTO experiments(
                    input_key, owner, function_name, base_commit, toolchain_key,
                    target_sha256, compiler_sha256, context_key, source_sha256,
                    shape_key, constraint_key, hypothesis, axis, object_sha256,
                    status, is_negative, reason, candidate_id,
                    candidate_record_sha256, strict_report_sha256,
                    data_report_sha256, report_sha256, workspace, source_path,
                    recorded_at, record_sha256
                ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                """,
                (
                    input_key,
                    identity["owner"],
                    identity["function"],
                    identity["base_commit"],
                    identity["toolchain_key"],
                    identity["target_sha256"],
                    identity.get("compiler_sha256"),
                    identity["context_key"],
                    identity["source_sha256"],
                    effective_shape,
                    effective_constraint,
                    effective_hypothesis,
                    effective_axis,
                    object_value,
                    status_value,
                    negative,
                    reason_value,
                    candidate_id,
                    candidate_record_sha256,
                    strict_report_sha256,
                    data_report_sha256,
                    report_sha256,
                    workspace,
                    source_path,
                    now,
                    record_sha,
                ),
            )
            connection.execute("DELETE FROM admissions WHERE input_key=?", (input_key,))
            row = connection.execute(
                "SELECT * FROM experiments WHERE input_key = ?", (input_key,)
            ).fetchone()
        return {
            "status": "recorded",
            "experiment": dict(row),
            "authority_advanced": False,
        }

    def invalidate_retained(
        self,
        *,
        input_key: str,
        owner: str,
        function: str,
        source_sha256: str,
        object_sha256: str,
        candidate_record_sha256: str,
        status: str,
    ) -> dict[str, Any]:
        """Delete exactly one split-brain retained record during journal recovery."""

        binding = {
            "input_key": _sha(input_key, "input_key"),
            "owner": _canonical_owner(owner),
            "function_name": _required_text(function, "function"),
            "source_sha256": _sha(source_sha256, "source_sha256"),
            "object_sha256": _sha(object_sha256, "object_sha256"),
            "candidate_record_sha256": _sha(
                candidate_record_sha256, "candidate_record_sha256"
            ),
            "status": _status(status),
        }
        if binding["status"] not in {"improved", "exact"}:
            raise RecoveryMemoryError("only retained outcomes can be invalidated")
        with contextlib.closing(self._connect()) as connection, connection:
            connection.execute("BEGIN IMMEDIATE")
            row = connection.execute(
                "SELECT * FROM experiments WHERE input_key=?",
                (binding["input_key"],),
            ).fetchone()
            if row is None:
                return {
                    "status": "missing", "input_key": binding["input_key"],
                    "authority_advanced": False,
                }
            for key, expected in binding.items():
                if row[key] != expected:
                    raise RecoveryMemoryError(
                        f"retained experiment does not match recovery binding {key}"
                    )
            connection.execute(
                "DELETE FROM experiments WHERE input_key=?",
                (binding["input_key"],),
            )
        return {
            "status": "invalidated", "input_key": binding["input_key"],
            "authority_advanced": False,
        }

    def retained_matches(self, **binding: Any) -> bool:
        """Return whether the exact journal-bound retained row exists."""

        expected = {
            "input_key": _sha(binding.get("input_key"), "input_key"),
            "owner": _canonical_owner(binding.get("owner")),
            "function_name": _required_text(binding.get("function"), "function"),
            "source_sha256": _sha(binding.get("source_sha256"), "source_sha256"),
            "object_sha256": _sha(binding.get("object_sha256"), "object_sha256"),
            "candidate_record_sha256": _sha(
                binding.get("candidate_record_sha256"), "candidate_record_sha256"
            ),
            "status": _status(binding.get("status")),
        }
        with contextlib.closing(self._connect()) as connection:
            row = connection.execute(
                "SELECT * FROM experiments WHERE input_key=?",
                (expected["input_key"],),
            ).fetchone()
        return row is not None and all(row[key] == value for key, value in expected.items())

    def discard(
        self, identity: Mapping[str, Any], *, requester: str,
        admission_token: str,
    ) -> dict[str, Any]:
        """Delete one unretained pending admission without recording history."""

        input_key = _sha(identity.get("input_key"), "input_key")
        requester_value = _compact_text(requester, "requester")
        token_value = _required_text(admission_token, "admission_token")
        with contextlib.closing(self._connect()) as connection, connection:
            connection.execute("BEGIN IMMEDIATE")
            admission = connection.execute(
                "SELECT * FROM admissions WHERE input_key=?", (input_key,)
            ).fetchone()
            if admission is None:
                return {
                    "status": "missing", "input_key": input_key,
                    "authority_advanced": False,
                }
            if admission["requester"] != requester_value or not hmac.compare_digest(
                admission["token"], token_value
            ):
                raise RecoveryMemoryError("candidate discard authority does not match admission")
            connection.execute("DELETE FROM admissions WHERE input_key=?", (input_key,))
        return {
            "status": "discarded", "input_key": input_key,
            "authority_advanced": False,
        }

    def require_pending_admission(
        self, identity: Mapping[str, Any], *, requester: str
    ) -> dict[str, Any]:
        """Prove that this lane performed the mandatory pre-compile lookup."""

        input_key = _sha(identity.get("input_key"), "input_key")
        requester_value = _compact_text(requester, "requester")
        with contextlib.closing(self._connect()) as connection, connection:
            experiment = connection.execute(
                "SELECT * FROM experiments WHERE input_key=?", (input_key,)
            ).fetchone()
            if experiment is not None:
                raise RecoveryMemoryError(
                    "candidate source/context is already recorded globally; "
                    "reuse the central experiment instead of recording another compile"
                )
            admission = connection.execute(
                "SELECT * FROM admissions WHERE input_key=?", (input_key,)
            ).fetchone()
            if admission is None or admission["state"] != "pending":
                raise RecoveryMemoryError(
                    "candidate has no pending central pre-compile admission; "
                    "run match lookup with --source before compiling"
                )
            if admission["requester"] != requester_value:
                raise RecoveryMemoryError(
                    "candidate admission belongs to another lane: "
                    + admission["requester"]
                )
            if _parse_time(admission["expires_at"]) < datetime.now(timezone.utc):
                raise RecoveryMemoryError("candidate admission expired; run lookup again")
        return {
            "status": "admission_verified",
            "input_key": input_key,
            "admission_token": admission["token"],
            "authority_advanced": False,
        }

    def import_historical_experiment(
        self,
        identity: Mapping[str, Any],
        *,
        object_sha256: str,
        status: str,
        reason: str,
        candidate_id: str | None,
        candidate_record_sha256: str,
        strict_report_sha256: str | None,
        data_report_sha256: str | None,
        workspace: str,
        source_path: str | None,
    ) -> dict[str, Any]:
        """Import only the latest retained pre-registry result without append history."""

        input_key = _sha(identity.get("input_key"), "input_key")
        object_value = _sha(object_sha256, "object_sha256")
        candidate_record_value = _sha(
            candidate_record_sha256, "candidate_record_sha256"
        )
        strict_value = _sha(
            strict_report_sha256, "strict_report_sha256", optional=True
        )
        data_value = _sha(
            data_report_sha256, "data_report_sha256", optional=True
        )
        status_value = _status(status)
        if status_value not in {"improved", "exact"}:
            raise RecoveryMemoryError(
                "historical import accepts retained improved/exact outcomes only"
            )
        reason_value = _compact_text(reason, "reason")
        workspace_value = _compact_text(workspace, "workspace")
        if candidate_id is not None:
            _compact_text(candidate_id, "candidate_id")
        if source_path is not None:
            _compact_text(source_path, "source_path")
        now = _now()
        record_body = {
            "schema": "recovery_experiment_import/v1",
            "input_key": input_key,
            "source_sha256": identity["source_sha256"],
            "object_sha256": object_value,
            "candidate_record_sha256": candidate_record_value,
            "workspace": workspace_value,
            "status": status_value,
            "reason": reason_value,
            "authority_advanced": False,
        }
        record_sha = _digest(record_body)
        with contextlib.closing(self._connect()) as connection, connection:
            connection.execute("BEGIN IMMEDIATE")
            existing = connection.execute(
                "SELECT * FROM experiments WHERE input_key=?", (input_key,)
            ).fetchone()
            if (
                existing is not None
                and existing["candidate_record_sha256"] != candidate_record_value
            ):
                raise RecoveryMemoryError(
                    "append-only historical observations are forbidden; keep one latest retained row"
                )
            if existing is None:
                stale_keys = [
                    row["input_key"] for row in connection.execute(
                        "SELECT input_key FROM experiments WHERE owner=? AND function_name=?",
                        (identity["owner"], identity["function"]),
                    )
                ]
                for stale_key in stale_keys:
                    connection.execute(
                        "DELETE FROM experiment_observations WHERE input_key=?",
                        (stale_key,),
                    )
                connection.execute(
                    "DELETE FROM experiments WHERE owner=? AND function_name=?",
                    (identity["owner"], identity["function"]),
                )
                connection.execute(
                    """
                    INSERT INTO experiments(
                        input_key, owner, function_name, base_commit, toolchain_key,
                        target_sha256, compiler_sha256, context_key, source_sha256,
                        shape_key, constraint_key, hypothesis, axis, object_sha256,
                        status, is_negative, reason, candidate_id,
                        candidate_record_sha256, strict_report_sha256,
                        data_report_sha256, report_sha256, workspace, source_path,
                        recorded_at, record_sha256
                    ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, NULL, NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NULL, ?, ?, ?, ?)
                    """,
                    (
                        input_key,
                        identity["owner"],
                        identity["function"],
                        identity["base_commit"],
                        identity["toolchain_key"],
                        identity["target_sha256"],
                        identity.get("compiler_sha256"),
                        identity["context_key"],
                        identity["source_sha256"],
                        identity.get("hypothesis"),
                        identity.get("axis"),
                        object_value,
                        status_value,
                        1 if status_value in NEGATIVE_STATUSES else 0,
                        reason_value,
                        candidate_id,
                        candidate_record_value,
                        strict_value,
                        data_value,
                        workspace_value,
                        source_path,
                        now,
                        record_sha,
                    ),
                )
                import_status = "imported"
            else:
                import_status = "unchanged"

            observation = connection.execute(
                "SELECT * FROM experiment_observations "
                "WHERE candidate_record_sha256=?",
                (candidate_record_value,),
            ).fetchone()
            observation_added = False
            if observation is not None:
                if (
                    observation["input_key"] != input_key
                    or observation["object_sha256"] != object_value
                ):
                    raise RecoveryMemoryError(
                        "candidate record hash maps to conflicting historical evidence"
                    )
            else:
                observation_added = True
                connection.execute(
                    """
                    INSERT INTO experiment_observations(
                        candidate_record_sha256, input_key, object_sha256,
                        status, reason, candidate_id, strict_report_sha256,
                        data_report_sha256, workspace, source_path, observed_at
                    ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                    """,
                    (
                        candidate_record_value,
                        input_key,
                        object_value,
                        status_value,
                        reason_value,
                        candidate_id,
                        strict_value,
                        data_value,
                        workspace_value,
                        source_path,
                        now,
                    ),
                )
                if import_status == "unchanged":
                    import_status = "observation_imported"

            object_rows = connection.execute(
                "SELECT DISTINCT object_sha256 FROM experiment_observations "
                "WHERE input_key=? ORDER BY object_sha256",
                (input_key,),
            ).fetchall()
            object_values = [str(item["object_sha256"]) for item in object_rows]
            if existing is not None and existing["object_sha256"] not in object_values:
                object_values.append(str(existing["object_sha256"]))
                object_values.sort()
            if len(object_values) > 1:
                conflict_reason = (
                    "immutable historical records disagree for one source/context: "
                    + ", ".join(value[:12] for value in object_values)
                    + "; compiler context or historical output provenance is incomplete"
                )
                conflict_body = {
                    "schema": "recovery_experiment_conflict/v1",
                    "input_key": input_key,
                    "objects": object_values,
                    "reason": conflict_reason,
                    "authority_advanced": False,
                }
                if (
                    existing is None
                    or existing["status"] != "historical-conflict"
                    or observation_added
                ):
                    connection.execute(
                        "UPDATE experiments SET status='historical-conflict', "
                        "is_negative=1, reason=?, record_sha256=? WHERE input_key=?",
                        (conflict_reason, _digest(conflict_body), input_key),
                    )
                    import_status = "conflict_imported"
            row = connection.execute(
                "SELECT * FROM experiments WHERE input_key=?", (input_key,)
            ).fetchone()
            observations = connection.execute(
                "SELECT * FROM experiment_observations WHERE input_key=? "
                "ORDER BY observed_at, candidate_record_sha256",
                (input_key,),
            ).fetchall()
        return {
            "status": import_status,
            "experiment": dict(row),
            "observations": [dict(item) for item in observations],
            "authority_advanced": False,
        }

    def ingest_report(self, path: Path) -> dict[str, Any]:
        path = path.expanduser().resolve()
        if not path.is_file():
            raise RecoveryMemoryError(f"crack report does not exist: {path}")
        raw = path.read_bytes()
        if len(raw) > MAX_COMPACT_FIELD_BYTES:
            raise RecoveryMemoryError("crack report exceeds the compact 64 KiB import cap")
        report_sha = hashlib.sha256(raw).hexdigest()
        parsed = parse_crack_report(path, raw)
        payload = parsed["payload"]
        record_body = {
            "schema": "recovery_report_record/v1",
            "report_sha256": report_sha,
            "owner": parsed["owner"],
            "function": parsed["function"],
            "report_path": str(path),
            "report_format": parsed["format"],
            "exact": parsed["exact"],
            "payload": payload,
            "authority_advanced": False,
        }
        record_sha = _digest(record_body)
        now = _now()
        payload_json = _canonical(payload).decode("utf-8").rstrip("\n")
        with contextlib.closing(self._connect()) as connection, connection:
            connection.execute("BEGIN IMMEDIATE")
            existing = connection.execute(
                "SELECT * FROM reports WHERE report_sha256 = ?", (report_sha,)
            ).fetchone()
            if existing is not None:
                if existing["record_sha256"] != record_sha:
                    raise RecoveryMemoryError(
                        "report hash already maps to different parsed evidence"
                    )
                return {
                    "status": "unchanged",
                    "report": dict(existing),
                    "constraints": connection.execute(
                        "SELECT COUNT(*) FROM report_constraints WHERE report_sha256=?",
                        (report_sha,),
                    ).fetchone()[0],
                    "authority_advanced": False,
                }
            stale_reports = [
                row["report_sha256"] for row in connection.execute(
                    "SELECT report_sha256 FROM reports WHERE owner=? AND function_name=?",
                    (parsed["owner"], parsed["function"]),
                )
            ]
            for stale_report in stale_reports:
                connection.execute(
                    "DELETE FROM report_constraints WHERE report_sha256=?",
                    (stale_report,),
                )
            connection.execute(
                "DELETE FROM reports WHERE owner=? AND function_name=?",
                (parsed["owner"], parsed["function"]),
            )
            connection.execute(
                """
                INSERT INTO reports(
                    report_sha256, owner, function_name, report_path,
                    report_format, exact, payload_json, ingested_at,
                    record_sha256
                ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)
                """,
                (
                    report_sha,
                    parsed["owner"],
                    parsed["function"],
                    str(path),
                    parsed["format"],
                    1 if parsed["exact"] else 0,
                    payload_json,
                    now,
                    record_sha,
                ),
            )
            constraints = parsed.get("constraints", [])
            for index, item in enumerate(constraints):
                constraint_payload = _canonical(item).decode("utf-8").rstrip("\n")
                constraint_key = _digest(
                    {
                        "report_sha256": report_sha,
                        "sequence_no": index,
                        "payload": item,
                    }
                )
                connection.execute(
                    """
                    INSERT INTO report_constraints(
                        constraint_key, report_sha256, sequence_no, kind,
                        status, payload_json
                    ) VALUES(?, ?, ?, ?, ?, ?)
                    """,
                    (
                        constraint_key,
                        report_sha,
                        index,
                        str(item.get("kind", "finding")),
                        item.get("status"),
                        constraint_payload,
                    ),
                )
            report = connection.execute(
                "SELECT * FROM reports WHERE report_sha256 = ?", (report_sha,)
            ).fetchone()
        return {
            "status": "ingested",
            "report": dict(report),
            "constraints": len(parsed.get("constraints", [])),
            "authority_advanced": False,
        }

    def context_memory(
        self, owner: str, function: str | None = None, *, limit: int = 12
    ) -> dict[str, Any]:
        owner_value = _canonical_owner(owner)
        parameters: list[Any] = [owner_value]
        function_clause = ""
        if function:
            function_clause = " AND function_name = ?"
            parameters.append(_required_text(function, "function"))
        parameters.append(max(1, min(int(limit), 100)))
        with contextlib.closing(self._connect()) as connection, connection:
            experiments = [
                dict(row)
                for row in connection.execute(
                    "SELECT * FROM experiments WHERE owner = ?"
                    + function_clause
                    + " ORDER BY recorded_at DESC LIMIT ?",
                    parameters,
                ).fetchall()
            ]
            for experiment in experiments:
                experiment["observations"] = [
                    dict(item)
                    for item in connection.execute(
                        "SELECT * FROM experiment_observations WHERE input_key=? "
                        "ORDER BY observed_at, candidate_record_sha256",
                        (experiment["input_key"],),
                    ).fetchall()
                ]
            reports = []
            report_rows = connection.execute(
                "SELECT * FROM reports WHERE owner = ?"
                + function_clause
                + " ORDER BY ingested_at DESC LIMIT ?",
                parameters,
            ).fetchall()
            for row in report_rows:
                value = dict(row)
                value["payload"] = json.loads(value.pop("payload_json"))
                value["constraints"] = [
                    json.loads(item["payload_json"])
                    for item in connection.execute(
                        "SELECT payload_json FROM report_constraints "
                        "WHERE report_sha256=? ORDER BY sequence_no",
                        (value["report_sha256"],),
                    ).fetchall()
                ]
                reports.append(value)
        return {
            "schema": "recovery_context_memory/v1",
            "owner": owner_value,
            "function": function,
            "experiments": experiments,
            "reports": reports,
            "authority_advanced": False,
        }

    def status(self) -> dict[str, Any]:
        with contextlib.closing(self._connect()) as connection, connection:
            counts = {
                name: connection.execute(f"SELECT COUNT(*) FROM {name}").fetchone()[0]
                for name in (
                    "experiments",
                    "experiment_observations",
                    "admissions",
                    "reports",
                    "report_constraints",
                    "lane_snapshots",
                )
            }
            pending = connection.execute(
                "SELECT COUNT(*) FROM admissions WHERE state='pending'"
            ).fetchone()[0]
            historical_conflicts = connection.execute(
                "SELECT COUNT(*) FROM experiments "
                "WHERE status='historical-conflict'"
            ).fetchone()[0]
        return {
            "schema": MEMORY_SCHEMA,
            "schema_version": MEMORY_SCHEMA_VERSION,
            "path": str(self.path),
            "counts": counts,
            "pending_admissions": pending,
            "historical_conflicts": historical_conflicts,
            "sha256": _sha256_file(self.path),
            "authority_advanced": False,
        }

    def record_lane_snapshot(self, snapshot: Mapping[str, Any]) -> None:
        for key in (
            "lane_root", "branch", "head_commit", "permanent_ref",
            "permanent_commit", "queue_path", "memory_path", "checked_at",
        ):
            _compact_text(snapshot.get(key), f"snapshot.{key}")
        with contextlib.closing(self._connect()) as connection, connection:
            connection.execute(
                """
                INSERT OR REPLACE INTO lane_snapshots(
                    lane_root, branch, head_commit, permanent_ref,
                    permanent_commit, queue_path, memory_path,
                    knowledge_sha256, checked_at, snapshot_sha256
                ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                """,
                (
                    snapshot["lane_root"],
                    snapshot["branch"],
                    snapshot["head_commit"],
                    snapshot["permanent_ref"],
                    snapshot["permanent_commit"],
                    snapshot["queue_path"],
                    snapshot["memory_path"],
                    snapshot["knowledge_sha256"],
                    snapshot["checked_at"],
                    snapshot["snapshot_sha256"],
                ),
            )
            connection.execute(
                "DELETE FROM lane_snapshots WHERE lane_root NOT IN "
                "(SELECT lane_root FROM lane_snapshots ORDER BY checked_at DESC LIMIT 128)"
            )


def _match_context_key(
    session: Mapping[str, Any], source_sha256: str | None = None
) -> str:
    request = session["request"]
    context = request["context"]
    compiler = context.get("compiler")
    compiler_sha = compiler.get("sha256") if isinstance(compiler, Mapping) else None
    input_hashes = sorted(
        str(item.get("sha256"))
        for item in context.get("compile_inputs", [])
        if isinstance(item, Mapping)
        and item.get("sha256")
        and item.get("sha256") != source_sha256
    )
    return _digest(
        {
            "base_commit": context["base_commit"],
            "toolchain_key": context["toolchain_key"],
            "target_sha256": request["target"]["sha256"],
            "compiler_sha256": compiler_sha,
            "compile_input_sha256": input_hashes,
            "build_rule_sha256": (
                context.get("build_rule", {}).get("sha256")
                if isinstance(context.get("build_rule"), Mapping)
                else None
            ),
        }
    )


def match_identity(
    session: Mapping[str, Any],
    source_sha256: str,
    *,
    shape_key: str | None = None,
    hypothesis: str | None = None,
    axis: str | None = None,
) -> dict[str, Any]:
    request = session["request"]
    context = request["context"]
    compiler = context.get("compiler")
    return RecoveryMemory.identity(
        owner=str(request["owner"]),
        function=str(request["function"]),
        base_commit=str(context["base_commit"]),
        toolchain_key=str(context["toolchain_key"]),
        target_sha256=str(request["target"]["sha256"]),
        compiler_sha256=(
            str(compiler["sha256"]) if isinstance(compiler, Mapping) else None
        ),
        context_key=_match_context_key(session, source_sha256),
        source_sha256=source_sha256,
        shape_key=shape_key,
        hypothesis=hypothesis,
        axis=axis,
    )


def match_precompile_admission(
    root: Path,
    session: Mapping[str, Any],
    source_sha256: str,
    *,
    source_path: str | None = None,
    shape_key: str | None = None,
    hypothesis: str | None = None,
    axis: str | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    if not recovery_memory_available(root):
        return {
            "status": "unavailable_non_git_fixture",
            "skip_compile": False,
            "authority_advanced": False,
        }
    store = RecoveryMemory.for_root(root)
    identity = match_identity(
        session,
        source_sha256,
        shape_key=shape_key,
        hypothesis=hypothesis,
        axis=axis,
    )
    result = store.admit(
        identity, requester=str(root), source_path=source_path
    )
    result["memory_path"] = str(store.path)
    return result


def match_require_precompile_admission(
    root: Path,
    session: Mapping[str, Any],
    source_sha256: str,
) -> dict[str, Any]:
    root = root.resolve()
    if not recovery_memory_available(root):
        return {
            "status": "unavailable_non_git_fixture",
            "authority_advanced": False,
        }
    store = RecoveryMemory.for_root(root)
    result = store.require_pending_admission(
        match_identity(session, source_sha256), requester=str(root)
    )
    result["memory_path"] = str(store.path)
    return result


def match_record_experiment(
    root: Path,
    session: Mapping[str, Any],
    record: Mapping[str, Any],
    *,
    workspace: str | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    if not recovery_memory_available(root):
        return {
            "status": "unavailable_non_git_fixture",
            "authority_advanced": False,
        }
    store = RecoveryMemory.for_root(root)
    source = record["source"]
    object_descriptor = record["object"]
    hypothesis = record.get("hypothesis", {})
    identity = match_identity(
        session,
        str(source["sha256"]),
        hypothesis=str(hypothesis.get("name") or "candidate"),
        axis=str(hypothesis.get("axis") or "unspecified"),
    )
    reports = record.get("reports", {})
    strict = reports.get("strict", {}) if isinstance(reports, Mapping) else {}
    data = reports.get("data", {}) if isinstance(reports, Mapping) else {}
    result = store.record(
        identity,
        requester=str(root),
        object_sha256=str(object_descriptor["sha256"]),
        status=str(record.get("outcome", {}).get("status") or "measured"),
        reason=str(record.get("outcome", {}).get("reason") or "candidate measured"),
        candidate_id=str(record.get("candidate_id") or "") or None,
        candidate_record_sha256=record.get("record_sha256"),
        strict_report_sha256=(
            strict.get("raw_sha256") if isinstance(strict, Mapping) else None
        ),
        data_report_sha256=(
            data.get("raw_sha256") if isinstance(data, Mapping) else None
        ),
        workspace=workspace or str(record.get("workspace") or "") or None,
        source_path=str(source.get("path") or "") or None,
    )
    result["memory_path"] = str(store.path)
    return result


def _match_focus_exact(record: Mapping[str, Any], function: str) -> bool:
    reports = record.get("reports")
    strict = reports.get("strict") if isinstance(reports, Mapping) else None
    compact = strict.get("compact") if isinstance(strict, Mapping) else None
    if not isinstance(compact, Mapping):
        return False
    focuses = compact.get("focuses")
    if not isinstance(focuses, list):
        focus = compact.get("focus")
        focuses = [focus] if isinstance(focus, Mapping) else []
    matching = [
        item
        for item in focuses
        if isinstance(item, Mapping) and item.get("name") == function
    ]
    return bool(matching) and all(
        item.get("exact") is True and item.get("diff_rows") in (None, 0)
        for item in matching
    )


def _contained_workbench_path(
    workspace: Path, relative: Any, expected: str
) -> Path:
    text = _required_text(relative, "workbench candidate path").replace("\\", "/")
    if text != expected or text.startswith("/") or ".." in Path(text).parts:
        raise RecoveryMemoryError(
            f"workbench candidate path must be exactly {expected!r}"
        )
    result = (workspace / Path(text)).resolve()
    try:
        result.relative_to(workspace)
    except ValueError as exc:
        raise RecoveryMemoryError(
            f"workbench candidate escapes its workspace: {relative}"
        ) from exc
    return result


def import_match_workbench(
    root: Path,
    workspace: Path,
    *,
    store: RecoveryMemory | None = None,
) -> dict[str, Any]:
    """Import one immutable match-workbench index into central memory.

    This is the migration path for candidates compiled before central
    admission existed.  It validates the immutable session/index/record hash
    chain and never fabricates an admission or advances source authority.
    """

    root = root.resolve()
    workspace = workspace.expanduser().resolve()
    try:
        workspace.relative_to(root)
    except ValueError as exc:
        raise RecoveryMemoryError(
            f"match workbench must be beneath its lane root: {workspace}"
        ) from exc
    if workspace.is_symlink():
        raise RecoveryMemoryError(f"match workbench cannot be a symlink: {workspace}")

    session = _load_json_object(workspace / "session.json", "match session")
    session_sha = _verify_self_hash(session, "session_sha256", "match session")
    if session.get("schema") != MATCH_SESSION_SCHEMA or session.get("schema_version") != 1:
        raise RecoveryMemoryError("unsupported match-workbench session schema")
    if Path(str(session.get("root", ""))).resolve() != root:
        raise RecoveryMemoryError("match session belongs to a different lane root")
    if Path(str(session.get("workspace", ""))).resolve() != workspace:
        raise RecoveryMemoryError("match session belongs to a different workspace")
    request = session.get("request")
    if not isinstance(request, Mapping) or request.get("schema") != MATCH_REQUEST_SCHEMA:
        raise RecoveryMemoryError("match session request is missing or unsupported")
    owner = _canonical_owner(request.get("owner"), "match request owner")
    function = _required_text(request.get("function"), "match request function")
    target = request.get("target")
    context = request.get("context")
    if not isinstance(target, Mapping) or not isinstance(context, Mapping):
        raise RecoveryMemoryError("match request target/context is missing")
    _sha(target.get("sha256"), "match request target.sha256")
    _required_text(context.get("base_commit"), "match context base_commit")
    _required_text(context.get("toolchain_key"), "match context toolchain_key")

    index = _load_json_object(workspace / "index.json", "match index")
    _verify_self_hash(index, "index_sha256", "match index")
    if index.get("schema") != MATCH_INDEX_SCHEMA or index.get("schema_version") != 1:
        raise RecoveryMemoryError("unsupported match-workbench index schema")
    if index.get("session_sha256") != session_sha:
        raise RecoveryMemoryError("match index belongs to a different session")
    candidate_paths = index.get("candidates")
    if not isinstance(candidate_paths, Mapping):
        raise RecoveryMemoryError("match index candidates must be an object")
    sequence = index.get("sequence")
    if isinstance(sequence, bool) or not isinstance(sequence, int) or sequence < 0:
        raise RecoveryMemoryError("match index sequence must be a nonnegative integer")

    prepared: list[dict[str, Any]] = []
    for candidate_id, relative in candidate_paths.items():
        candidate_value = _required_text(candidate_id, "candidate id")
        expected = f"candidates/{candidate_value}.json"
        candidate_path = _contained_workbench_path(workspace, relative, expected)
        record = _load_json_object(candidate_path, f"candidate {candidate_value}")
        record_sha = _verify_self_hash(
            record, "record_sha256", f"candidate {candidate_value}"
        )
        if (
            record.get("schema") != MATCH_CANDIDATE_SCHEMA
            or record.get("schema_version") != 1
            or record.get("candidate_id") != candidate_value
            or record.get("session_sha256") != session_sha
        ):
            raise RecoveryMemoryError(
                f"candidate {candidate_value} identity/session is invalid"
            )
        ordinal = record.get("ordinal")
        if isinstance(ordinal, bool) or not isinstance(ordinal, int) or ordinal < 1:
            raise RecoveryMemoryError(
                f"candidate {candidate_value} ordinal is invalid"
            )
        source = record.get("source")
        object_descriptor = record.get("object")
        if not isinstance(source, Mapping) or not isinstance(object_descriptor, Mapping):
            raise RecoveryMemoryError(
                f"candidate {candidate_value} lacks source/object descriptors"
            )
        source_sha = _sha(source.get("sha256"), "candidate source.sha256")
        object_sha = _sha(object_descriptor.get("sha256"), "candidate object.sha256")
        hypothesis = record.get("hypothesis")
        outcome = record.get("outcome")
        if not isinstance(hypothesis, Mapping) or not isinstance(outcome, Mapping):
            raise RecoveryMemoryError(
                f"candidate {candidate_value} lacks hypothesis/outcome evidence"
            )
        hypothesis_name = _required_text(
            hypothesis.get("name"), "candidate hypothesis.name"
        )
        axis = _required_text(hypothesis.get("axis"), "candidate hypothesis.axis")
        local_status = _required_text(outcome.get("status"), "candidate outcome.status")
        local_reason = _required_text(outcome.get("reason"), "candidate outcome.reason")
        reports = record.get("reports")
        strict = reports.get("strict") if isinstance(reports, Mapping) else None
        data = reports.get("data") if isinstance(reports, Mapping) else None
        strict_sha = (
            _sha(strict.get("raw_sha256"), "candidate strict raw_sha256")
            if isinstance(strict, Mapping)
            else None
        )
        data_sha = (
            _sha(data.get("raw_sha256"), "candidate data raw_sha256")
            if isinstance(data, Mapping)
            else None
        )
        attestation = record.get("compile_attestation")
        if attestation is not None:
            if not isinstance(attestation, Mapping):
                raise RecoveryMemoryError(
                    f"candidate {candidate_value} compile attestation is invalid"
                )
            _verify_self_hash(
                attestation,
                "attestation_sha256",
                f"candidate {candidate_value} compile attestation",
            )
            attested_source = attestation.get("source")
            attested_object = attestation.get("object")
            if (
                not isinstance(attested_source, Mapping)
                or not isinstance(attested_object, Mapping)
                or attested_source.get("sha256") != source_sha
                or attested_object.get("sha256") != object_sha
            ):
                raise RecoveryMemoryError(
                    f"candidate {candidate_value} attestation artifact mismatch"
                )
        exact = _match_focus_exact(record, function)
        reason = (
            f"historical workbench outcome={local_status}; exact={str(exact).lower()}; "
            f"{local_reason}"
        )
        identity = match_identity(
            session,
            source_sha,
            hypothesis=hypothesis_name,
            axis=axis,
        )
        prepared.append(
            {
                "ordinal": ordinal,
                "record_sha256": record_sha,
                "previous_record_sha256": record.get("previous_record_sha256"),
                "identity": identity,
                "object_sha256": object_sha,
                "status": "exact" if exact else "nonexact",
                "reason": reason,
                "candidate_id": candidate_value,
                "strict_report_sha256": strict_sha,
                "data_report_sha256": data_sha,
                "source_path": str(source.get("path") or "") or None,
            }
        )

    prepared.sort(key=lambda item: item["ordinal"])
    if [item["ordinal"] for item in prepared] != list(range(1, sequence + 1)):
        raise RecoveryMemoryError(
            "match candidate ordinal chain is incomplete or noncanonical"
        )
    previous: str | None = None
    for item in prepared:
        if item["previous_record_sha256"] != previous:
            raise RecoveryMemoryError(
                f"candidate {item['candidate_id']} breaks the immutable record chain"
            )
        previous = item["record_sha256"]
    if index.get("last_record_sha256") != previous:
        raise RecoveryMemoryError("match index last_record_sha256 is inconsistent")

    memory = store or RecoveryMemory.for_root(root)
    imported = 0
    unchanged = 0
    observations_imported = 0
    conflicts = 0
    retained_items = [item for item in prepared if item["status"] == "exact"][-1:]
    for item in retained_items:
        result = memory.import_historical_experiment(
            item["identity"],
            object_sha256=item["object_sha256"],
            status=item["status"],
            reason=item["reason"],
            candidate_id=item["candidate_id"],
            candidate_record_sha256=item["record_sha256"],
            strict_report_sha256=item["strict_report_sha256"],
            data_report_sha256=item["data_report_sha256"],
            workspace=str(workspace),
            source_path=item["source_path"],
        )
        if result["status"] == "imported":
            imported += 1
        elif result["status"] == "observation_imported":
            observations_imported += 1
        elif result["status"] == "conflict_imported":
            conflicts += 1
        else:
            unchanged += 1
    return {
        "schema": "recovery_workbench_import/v1",
        "status": (
            "imported"
            if imported or observations_imported or conflicts
            else "unchanged"
        ),
        "root": str(root),
        "workspace": str(workspace),
        "owner": owner,
        "function": function,
        "session_sha256": session_sha,
        "candidate_count": len(retained_items),
        "imported": imported,
        "observations_imported": observations_imported,
        "conflicts": conflicts,
        "unchanged": unchanged,
        "memory_path": str(memory.path),
        "authority_advanced": False,
    }


def discover_match_workbenches(root: Path) -> list[Path]:
    """Find lane-local immutable workbenches without descending into CAS trees."""

    root = root.resolve()
    found: set[Path] = set()
    visited = 0
    for relative in WORKBENCH_SEARCH_DIRS:
        base = (root / relative).resolve()
        if not base.is_dir() or base.is_symlink():
            continue
        pending: list[tuple[Path, int]] = [(base, 0)]
        while pending:
            directory, depth = pending.pop()
            visited += 1
            if visited > WORKBENCH_SEARCH_MAX_COUNT:
                raise RecoveryMemoryError(
                    "workbench discovery exceeded its bounded directory census"
                )
            if (directory / "session.json").is_file() and (
                directory / "index.json"
            ).is_file():
                found.add(directory.resolve())
                continue
            if depth >= WORKBENCH_SEARCH_MAX_DEPTH:
                continue
            try:
                children = [
                    Path(entry.path)
                    for entry in os.scandir(directory)
                    if entry.is_dir(follow_symlinks=False)
                    and entry.name.casefold() not in WORKBENCH_PRUNE_NAMES
                ]
            except OSError as exc:
                raise RecoveryMemoryError(
                    f"cannot scan match-workbench directory {directory}: {exc}"
                ) from exc
            pending.extend((child, depth + 1) for child in children)
    return sorted(found, key=lambda value: str(value).casefold())


def sync_match_workbenches(root: Path, *, strict: bool = True) -> dict[str, Any]:
    root = root.resolve()
    store = RecoveryMemory.for_root(root)
    workspaces = discover_match_workbenches(root)
    rows: list[dict[str, Any]] = []
    failures: list[dict[str, str]] = []
    for workspace in workspaces:
        try:
            rows.append(import_match_workbench(root, workspace, store=store))
        except (OSError, RecoveryMemoryError) as exc:
            failures.append({"workspace": str(workspace), "error": str(exc)})
    if strict and failures:
        raise RecoveryMemoryError(
            "match-workbench synchronization failed:\n- "
            + "\n- ".join(
                f"{item['workspace']}: {item['error']}" for item in failures
            )
        )
    return {
        "schema": "recovery_workbench_sync/v1",
        "memory_path": str(store.path),
        "discovered": len(workspaces),
        "workbenches": rows,
        "failures": failures,
        "imported": sum(int(item["imported"]) for item in rows),
        "observations_imported": sum(
            int(item["observations_imported"]) for item in rows
        ),
        "conflicts": sum(int(item["conflicts"]) for item in rows),
        "unchanged": sum(int(item["unchanged"]) for item in rows),
        "authority_advanced": False,
    }


def _heading_sections(text: str) -> dict[str, str]:
    matches = list(re.finditer(r"(?m)^\s{0,3}#{1,6}\s+(.+?)\s*$", text))
    result: dict[str, str] = {}
    for index, match in enumerate(matches):
        end = matches[index + 1].start() if index + 1 < len(matches) else len(text)
        key = re.sub(r"[^a-z0-9]+", " ", match.group(1).lower()).strip()
        result[key] = text[match.end() : end].strip()
    return result


def _find_section(sections: Mapping[str, str], *terms: str) -> str | None:
    for key, value in sections.items():
        if all(term in key for term in terms):
            return value[:20000]
    return None


def _markdown_owner_function(text: str) -> tuple[str, str]:
    owner = None
    function = None
    owner_match = re.search(
        r"(?im)^\s*(?:[-*]\s*)?(?:\*\*)?owner(?:\*\*)?\s*:\s*`?([A-Za-z0-9:./_-]+)",
        text,
    )
    function_match = re.search(
        r"(?im)(?:^|[;|])\s*(?:[-*]\s*)?(?:\*\*)?function(?:\*\*)?\s*:\s*`?([A-Za-z_][A-Za-z0-9_]*)",
        text,
    )
    if owner_match:
        owner = owner_match.group(1).strip(" `.;")
    if function_match:
        function = function_match.group(1).strip(" `.;")
    combined = re.search(
        r"(?im)^\s*(?:[-*]\s*)?owner\s*/\s*function\s*:\s*`?([^`\s]+)`?\s*(?:[-—/])+\s*`?([A-Za-z_][A-Za-z0-9_]*)`?",
        text,
    )
    if combined:
        owner = owner or combined.group(1)
        function = function or combined.group(2)
    title = re.search(
        r"(?im)^.*?(main:board/[a-z0-9_.-]+)\s*(?:/|[-—])\s*`?([A-Za-z_][A-Za-z0-9_]*)`?.*$",
        text,
    )
    if title:
        owner = owner or title.group(1)
        function = function or title.group(2)
    if not owner:
        any_owner = re.search(r"main:board/[a-z0-9_.-]+", text, re.I)
        if any_owner:
            owner = any_owner.group(0)
    if not function:
        heading = re.search(
            r"(?im)^#\s+CRACK_REPORT/v1\s*(?:[-—:]\s*)?`?([A-Za-z_][A-Za-z0-9_]*)`?\s*$",
            text,
        )
        if heading:
            function = heading.group(1)
    if not function:
        functions_block = re.search(
            r"(?ims)functions?\s+first\s+made\s+exact[^:]*:\s*(.*?)(?:\n\s*-[^`\n]|\n##|\Z)",
            text,
        )
        if functions_block:
            functions = re.findall(
                r"(?m)^\s*-\s*`([A-Za-z_][A-Za-z0-9_]*)`",
                functions_block.group(1),
            )
            if functions:
                function = ",".join(functions)
    if not owner or not function:
        raise RecoveryMemoryError(
            "CRACK_REPORT markdown must identify both owner and function"
        )
    return _canonical_owner(owner, "report owner"), _required_text(
        function, "report function"
    )


def parse_crack_report(path: Path, raw: bytes) -> dict[str, Any]:
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise RecoveryMemoryError(f"crack report is not UTF-8: {path}") from exc
    if path.suffix.lower() == ".json":
        try:
            value = json.loads(text)
        except json.JSONDecodeError as exc:
            raise RecoveryMemoryError(
                f"invalid crack report JSON {path}:{exc.lineno}:{exc.colno}"
            ) from exc
        if not isinstance(value, Mapping) or value.get("schema") != "CRACK_REPORT/v1":
            raise RecoveryMemoryError("JSON report is not CRACK_REPORT/v1")
        owner = _canonical_owner(value.get("owner"), "report owner")
        function = _required_text(value.get("function"), "report function")
        result = value.get("result") or value.get("exactness") or value.get("proof")
        if not isinstance(result, Mapping):
            raise RecoveryMemoryError("CRACK_REPORT/v1 result is missing")
        strict = result.get("strict_percent")
        data = result.get("data_percent")
        exact = strict == 100 or strict == 100.0
        exact = exact and (data == 100 or data == 100.0)
        if not exact:
            raise RecoveryMemoryError("only completed exact CRACK_REPORT/v1 is ingestible")
        attempts = value.get("chronological_attempt_ledger", value.get("attempt_ledger", []))
        if attempts is not None and not isinstance(attempts, list):
            raise RecoveryMemoryError("chronological_attempt_ledger must be a list")
        constraints = []
        for attempt in attempts or []:
            if isinstance(attempt, Mapping):
                decision = str(attempt.get("decision") or attempt.get("result") or "")
                constraints.append(
                    {
                        "kind": "attempt",
                        "status": (
                            "rejected"
                            if re.search(r"reject|no-go|regress|neutral|nonexact", decision, re.I)
                            else "retained"
                            if re.search(r"retain|exact", decision, re.I)
                            else "measured"
                        ),
                        "attempt": dict(attempt),
                    }
                )
        generalized = value.get(
            "generalized_improvement_request", value.get("generalized_improvement")
        )
        if isinstance(generalized, Mapping):
            constraints.append(
                {"kind": "reusable_improvement", "status": "proposed", "finding": dict(generalized)}
            )
        payload = {
            "result": dict(result),
            "bound_artifacts": value.get("bound_artifacts") or value.get("identity") or value.get("artifacts") or value.get("evidence"),
            "retained_natural_c_hunk": value.get("retained_natural_c_hunk") or value.get("retained_natural_c"),
            "causal_explanation": value.get("causal_explanation") or value.get("causal_evidence"),
            "counterfactual_shortest_path": value.get("counterfactual_shortest_path"),
            "generalized_improvement_request": generalized,
            "attempt_count": len(attempts or []),
        }
        return {
            "format": "json",
            "owner": owner,
            "function": function,
            "exact": True,
            "payload": payload,
            "constraints": constraints,
        }
    if "CRACK_REPORT/v1" not in text:
        raise RecoveryMemoryError("markdown report lacks CRACK_REPORT/v1 marker")
    owner, function = _markdown_owner_function(text)
    exact = bool(
        re.search(r"(?i)(?:strict[^\n]{0,40}100|100%[^\n]{0,40}strict)", text)
        and re.search(r"(?i)(?:data[^\n]{0,40}100|100%[^\n]{0,40}data)", text)
    )
    if not exact:
        exact = bool(
            re.search(r"(?i)strict\s*/\s*data[^\n]{0,100}exact", text)
            and re.search(r"(?i)zero\s+(?:diff\s+)?rows", text)
        )
    if not exact:
        raise RecoveryMemoryError("only completed exact CRACK_REPORT/v1 is ingestible")
    sections = _heading_sections(text)
    retained = _find_section(sections, "retained") or _find_section(
        sections, "admissibility"
    )
    causal = _find_section(sections, "causal")
    attempts = _find_section(sections, "attempt") or _find_section(
        sections, "chronological"
    )
    counterfactual = _find_section(sections, "counterfactual")
    improvement = _find_section(sections, "generalized") or _find_section(
        sections, "improvement"
    )
    constraints: list[dict[str, Any]] = []
    if attempts:
        constraints.append(
            {"kind": "attempt_ledger", "status": "mixed", "text": attempts}
        )
    if improvement:
        constraints.append(
            {
                "kind": "reusable_improvement",
                "status": "proposed",
                "text": improvement,
            }
        )
    payload = {
        "proof_summary": next(
            (
                line.strip()
                for line in text.splitlines()
                if re.search(r"(?i)(proof|result).*100", line)
            ),
            None,
        ),
        "retained_natural_c": retained,
        "causal_explanation": causal,
        "counterfactual_shortest_path": counterfactual,
        "generalized_improvement": improvement,
        "attempt_ledger": attempts,
    }
    return {
        "format": "markdown",
        "owner": owner,
        "function": function,
        "exact": True,
        "payload": payload,
        "constraints": constraints,
    }


def _report_paths_from_queue(queue: Mapping[str, Any]) -> list[Path]:
    paths: set[Path] = set()
    for task in queue.get("tasks", []):
        if not isinstance(task, Mapping):
            continue
        strings: list[str] = []
        for key in ("source", "object_report", "report_path"):
            value = task.get(key)
            if isinstance(value, str):
                strings.append(value)
        note = task.get("note")
        if isinstance(note, str):
            strings += re.findall(
                r"(?:[A-Za-z]:[/\\][^\r\n;|]+?\.(?:json|md))", note
            )
        for raw in strings:
            if "CRACK_REPORT" not in raw.upper() and "crack_reports" not in raw.lower() and "crack-reports" not in raw.lower():
                continue
            candidate = Path(raw)
            if candidate.is_file() and candidate.suffix.lower() in {".json", ".md"}:
                paths.add(candidate.resolve())
    return sorted(paths, key=lambda value: str(value).casefold())


def sync_crack_reports(root: Path, *, strict: bool = False) -> dict[str, Any]:
    root = root.resolve()
    store = RecoveryMemory.for_root(root)
    queue_file = queue_path(root)
    queue = read_queue(queue_file)
    paths = _report_paths_from_queue(queue)
    ingested: list[dict[str, Any]] = []
    failures: list[dict[str, str]] = []
    for path in paths:
        try:
            result = store.ingest_report(path)
            ingested.append(
                {
                    "path": str(path),
                    "status": result["status"],
                    "report_sha256": result["report"]["report_sha256"],
                    "owner": result["report"]["owner"],
                    "function": result["report"]["function_name"],
                }
            )
        except (OSError, RecoveryMemoryError) as exc:
            failures.append({"path": str(path), "error": str(exc)})
    if strict and failures:
        raise RecoveryMemoryError(
            "crack-report synchronization failed:\n- "
            + "\n- ".join(f"{item['path']}: {item['error']}" for item in failures)
        )
    return {
        "schema": "recovery_report_sync/v1",
        "queue_path": str(queue_file),
        "memory_path": str(store.path),
        "discovered": len(paths),
        "ingested": ingested,
        "failures": failures,
        "authority_advanced": False,
    }


def startup_check(
    root: Path,
    *,
    sync_reports: bool = True,
    strict_reports: bool = True,
    permanent_ref: str = PERMANENT_RECOVERY_REF,
    workflow_root: str | Path | None = None,
) -> dict[str, Any]:
    root = root.resolve()
    if not is_git_worktree(root):
        raise RecoveryMemoryError(f"lane startup root is not a Git worktree: {root}")
    try:
        locations = queue_location_audit(root)
    except QueueError as exc:
        raise RecoveryMemoryError(str(exc)) from exc
    if locations.get("shadows"):
        raise RecoveryMemoryError(
            "lane startup rejected shadow queues: "
            + ", ".join(str(item["path"]) for item in locations["shadows"])
        )
    queue_file = queue_path(root)
    head = _git(root, "rev-parse", "HEAD")
    explicit_workflow_root = workflow_root is not None
    workflow = (
        _canonical_workflow_root(workflow_root)
        if explicit_workflow_root
        else root
    )
    permanent_commit = _git(workflow, "rev-parse", permanent_ref)
    workflow_head = _git(workflow, "rev-parse", "HEAD")
    if explicit_workflow_root:
        if workflow_head != permanent_commit:
            raise RecoveryMemoryError(
                f"workflow HEAD ({workflow_head[:12]}) must equal released "
                f"{permanent_ref} ({permanent_commit[:12]})"
            )
        workflow_status = _git(
            workflow,
            "status",
            "--porcelain=v1",
            "--untracked-files=all",
        )
        if workflow_status:
            raise RecoveryMemoryError(
                "explicit workflow worktree is not clean:\n" + workflow_status
            )
        merge_base_process = subprocess.run(
            ["git", "merge-base", "--all", head, workflow_head],
            cwd=root,
            text=True,
            capture_output=True,
            check=False,
            env=_workflow_alternate_environment(workflow),
        )
        merge_bases = sorted(
            {
                line.strip().lower()
                for line in merge_base_process.stdout.splitlines()
                if re.fullmatch(r"[0-9a-fA-F]{40}", line.strip())
            }
        )
        if merge_base_process.returncode or len(merge_bases) != 1:
            detail = (
                merge_base_process.stderr.strip()
                or f"found {len(merge_bases)} merge bases"
            )
            raise RecoveryMemoryError(
                "lane and workflow histories lack one deterministic common "
                f"merge-base: {detail}"
            )
        merge_base = merge_bases[0]
        for checkout, checkout_head, label in (
            (root, head, "lane"),
            (workflow, workflow_head, "workflow"),
        ):
            contains_base = subprocess.run(
                ["git", "merge-base", "--is-ancestor", merge_base, checkout_head],
                cwd=checkout,
                capture_output=True,
                check=False,
            )
            if contains_base.returncode:
                raise RecoveryMemoryError(
                    f"deterministic merge-base is not contained by {label} HEAD"
                )
    else:
        ancestor = subprocess.run(
            ["git", "merge-base", "--is-ancestor", permanent_commit, head],
            cwd=root,
            capture_output=True,
            check=False,
        )
        if ancestor.returncode != 0:
            raise RecoveryMemoryError(
                f"lane is stale: {permanent_ref} ({permanent_commit[:12]}) "
                f"is not an ancestor of HEAD ({head[:12]})"
            )
        merge_base = permanent_commit
    from tools.knowledge_freshness import validate_freshness
    from tools.recovery_core import load
    from tools.recovery_knowledge import validate_knowledge

    try:
        data = load(workflow, validate=False)
        knowledge_errors = sorted(
            set([*validate_knowledge(data), *validate_freshness(data)])
        )
        workflow_files = _workflow_metadata_hashes(
            workflow,
            data,
            require_freshness=explicit_workflow_root,
        )
    except (OSError, ValueError) as exc:
        if isinstance(exc, RecoveryMemoryError):
            raise
        raise RecoveryMemoryError(
            f"workflow recovery metadata is invalid: {exc}"
        ) from exc
    if knowledge_errors:
        metadata_owner = "workflow" if explicit_workflow_root else "lane"
        raise RecoveryMemoryError(
            f"{metadata_owner} knowledge/freshness metadata is invalid:\n- "
            + "\n- ".join(knowledge_errors)
        )
    project_files = data.get("project", {}).get("files", {})
    if not isinstance(project_files, Mapping):
        project_files = {}
    knowledge_relatives = (
        str(
            project_files.get(
                "compiler_patterns",
                "config/recovery/compiler_patterns.json",
            )
        ),
        str(
            project_files.get(
                "knowledge_freshness",
                "config/recovery/knowledge_freshness.json",
            )
        ),
    )
    knowledge_hash = (
        _digest(
            {
                Path(relative).as_posix(): workflow_files[
                    Path(relative).as_posix()
                ]
                for relative in knowledge_relatives
                if Path(relative).as_posix() in workflow_files
            }
        )
        if explicit_workflow_root
        else _digest(workflow_files)
    )
    workflow_root_body = {
        "workflow_root": str(workflow),
        "lane_head": head,
        "workflow_head": workflow_head,
        "permanent_ref": permanent_ref,
        "permanent_ref_commit": permanent_commit,
        "merge_base": merge_base,
        "workflow_files": workflow_files,
    }
    workflow_root_sha256 = _digest(workflow_root_body)
    store = RecoveryMemory.for_root(root)
    report_sync = (
        sync_crack_reports(root, strict=strict_reports)
        if sync_reports
        else {"status": "skipped"}
    )
    workbench_sync = (
        sync_match_workbenches(root, strict=True)
        if sync_reports
        else {"status": "skipped"}
    )
    checked_at = _now()
    snapshot_body = {
        "schema": "recovery_lane_startup/v1",
        "lane_root": str(root),
        "branch": _git(root, "branch", "--show-current") or "DETACHED",
        "head_commit": head,
        "lane_head": head,
        "workflow_root": str(workflow),
        "workflow_head_commit": workflow_head,
        "workflow_head": workflow_head,
        "workflow_root_sha256": workflow_root_sha256,
        "workflow_files": workflow_files,
        "permanent_ref": permanent_ref,
        "permanent_commit": permanent_commit,
        "permanent_ref_commit": permanent_commit,
        "merge_base": merge_base,
        "queue_path": str(queue_file),
        "memory_path": str(store.path),
        "knowledge_sha256": knowledge_hash,
        "checked_at": checked_at,
    }
    snapshot = {**snapshot_body, "snapshot_sha256": _digest(snapshot_body)}
    store.record_lane_snapshot(snapshot)
    return {
        "status": "pass",
        **snapshot,
        "queue_locations": locations,
        "report_sync": report_sync,
        "workbench_sync": workbench_sync,
        "authority_advanced": False,
    }


def render_context_memory(value: Mapping[str, Any]) -> str:
    lines = [
        "## Central recovery memory",
        "",
        "Cross-worktree experiments and completed crack reports from the canonical registry. Check these constraints before compiling a candidate.",
    ]
    experiments = value.get("experiments", [])
    reports = value.get("reports", [])
    if not experiments and not reports:
        lines.append("- No central experiments or completed reports match this target yet.")
        return "\n".join(lines)
    for experiment in experiments[:8]:
        lines.append(
            "- Experiment "
            f"`{str(experiment.get('input_key', ''))[:12]}`: "
            f"{experiment.get('status')} — {experiment.get('axis') or 'unclassified axis'}; "
            f"{experiment.get('reason')}"
        )
        observations = experiment.get("observations", [])
        distinct_objects = {
            item.get("object_sha256")
            for item in observations
            if isinstance(item, Mapping) and item.get("object_sha256")
        }
        if len(distinct_objects) > 1:
            lines.append(
                "  - Quarantined history: "
                f"{len(observations)} immutable records contain "
                f"{len(distinct_objects)} object results; do not reuse or recompile "
                "until the missing compiler/provenance input is identified."
            )
    for report in reports[:6]:
        payload = report.get("payload", {})
        causal = payload.get("causal_explanation") if isinstance(payload, Mapping) else None
        generalized = (
            payload.get("generalized_improvement_request")
            or payload.get("generalized_improvement")
            if isinstance(payload, Mapping)
            else None
        )
        lines.append(
            f"- Exact report `{report.get('function_name')}` "
            f"`{str(report.get('report_sha256', ''))[:12]}` at `{report.get('report_path')}`"
        )
        if causal:
            lines.append("  - Cause: " + re.sub(r"\s+", " ", str(causal))[:800])
        if generalized:
            if isinstance(generalized, Mapping):
                generalized = generalized.get("requested_behavior") or generalized.get("title")
            lines.append("  - Reuse: " + re.sub(r"\s+", " ", str(generalized))[:600])
        rejected = [
            item
            for item in report.get("constraints", [])
            if isinstance(item, Mapping) and item.get("status") == "rejected"
        ]
        if rejected:
            lines.append(
                f"  - Negative controls: {len(rejected)} retained centrally; do not rediscover them."
            )
    return "\n".join(lines)


def _identity_from_args(args: argparse.Namespace) -> dict[str, Any]:
    return RecoveryMemory.identity(
        owner=args.owner,
        function=args.function,
        base_commit=args.base_commit,
        toolchain_key=args.toolchain_key,
        target_sha256=args.target_sha256,
        compiler_sha256=args.compiler_sha256,
        context_key=args.context_key,
        source_sha256=args.source_sha256,
        shape_key=args.shape_key,
        hypothesis=args.hypothesis,
        axis=args.axis,
    )


def add_memory_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser(
        "memory", help="use canonical cross-worktree recovery memory"
    )
    commands = parser.add_subparsers(dest="memory_command", required=True)
    status = commands.add_parser("status")
    status.add_argument("--json", action="store_true")
    startup = commands.add_parser("startup-check")
    startup.add_argument("--no-sync", action="store_true")
    startup.add_argument(
        "--strict-reports",
        action="store_true",
        help="explicitly request the default fail-closed report synchronization",
    )
    startup.add_argument(
        "--allow-report-backlog",
        action="store_true",
        help="diagnostic escape hatch: report parse failures are returned but do not fail startup",
    )
    startup.add_argument("--permanent-ref", default=PERMANENT_RECOVERY_REF)
    startup.add_argument(
        "--workflow-root",
        help=(
            "absolute canonical Git worktree supplying authenticated "
            "config/recovery knowledge metadata"
        ),
    )
    startup.add_argument("--json", action="store_true")
    ingest = commands.add_parser("ingest-report")
    ingest.add_argument("report")
    ingest.add_argument("--json", action="store_true")
    sync = commands.add_parser("sync-reports")
    sync.add_argument("--strict", action="store_true")
    sync.add_argument("--json", action="store_true")
    import_workbench = commands.add_parser("import-workbench")
    import_workbench.add_argument("workspace")
    import_workbench.add_argument("--json", action="store_true")
    sync_workbenches = commands.add_parser("sync-workbenches")
    sync_workbenches.add_argument("--allow-failures", action="store_true")
    sync_workbenches.add_argument("--json", action="store_true")
    context = commands.add_parser("context")
    context.add_argument("--owner", required=True)
    context.add_argument("--function")
    context.add_argument("--limit", type=int, default=12)
    context.add_argument("--json", action="store_true")
    for name in ("admit", "record"):
        command = commands.add_parser(name)
        command.add_argument("--owner", required=True)
        command.add_argument("--function", required=True)
        command.add_argument("--base-commit", required=True)
        command.add_argument("--toolchain-key", required=True)
        command.add_argument("--target-sha256", required=True)
        command.add_argument("--compiler-sha256")
        command.add_argument("--context-key")
        command.add_argument("--source-sha256", required=True)
        command.add_argument("--shape-key")
        command.add_argument("--hypothesis")
        command.add_argument("--axis")
        command.add_argument("--requester")
        command.add_argument("--source-path")
        command.add_argument("--json", action="store_true")
        if name == "record":
            command.add_argument("--object-sha256", required=True)
            command.add_argument("--status", required=True)
            command.add_argument("--reason", required=True)
            command.add_argument("--admission-token")
            command.add_argument("--candidate-id")
            command.add_argument("--candidate-record-sha256")
            command.add_argument("--strict-report-sha256")
            command.add_argument("--data-report-sha256")
            command.add_argument("--report-sha256")
            command.add_argument("--workspace")
    return parser


def run_memory_command(args: argparse.Namespace, *, root: Path) -> int:
    root = root.resolve()
    if args.memory_command == "startup-check":
        result = startup_check(
            root,
            sync_reports=not args.no_sync,
            strict_reports=not args.allow_report_backlog,
            permanent_ref=args.permanent_ref,
            workflow_root=args.workflow_root,
        )
    elif args.memory_command == "sync-reports":
        result = sync_crack_reports(root, strict=args.strict)
    elif args.memory_command == "import-workbench":
        result = import_match_workbench(root, Path(args.workspace))
    elif args.memory_command == "sync-workbenches":
        result = sync_match_workbenches(
            root, strict=not args.allow_failures
        )
    else:
        store = RecoveryMemory.for_root(root)
        if args.memory_command == "status":
            result = store.status()
        elif args.memory_command == "ingest-report":
            result = store.ingest_report(Path(args.report))
        elif args.memory_command == "context":
            result = store.context_memory(
                args.owner, args.function, limit=args.limit
            )
        else:
            identity = _identity_from_args(args)
            requester = args.requester or str(root)
            if args.memory_command == "admit":
                result = store.admit(
                    identity,
                    requester=requester,
                    source_path=args.source_path,
                )
            elif args.memory_command == "discard":
                result = store.discard(
                    identity, requester=requester,
                    admission_token=args.admission_token,
                )
            else:
                result = store.record(
                    identity,
                    requester=requester,
                    object_sha256=args.object_sha256,
                    status=args.status,
                    reason=args.reason,
                    admission_token=args.admission_token,
                    candidate_id=args.candidate_id,
                    candidate_record_sha256=args.candidate_record_sha256,
                    strict_report_sha256=args.strict_report_sha256,
                    data_report_sha256=args.data_report_sha256,
                    report_sha256=args.report_sha256,
                    workspace=args.workspace,
                    source_path=args.source_path,
                )
    if getattr(args, "json", False):
        print(json.dumps(result, indent=2))
    else:
        print(result.get("status") or result.get("schema") or "ok")
    return 0


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    subparsers = parser.add_subparsers(dest="command", required=True)
    add_memory_parser(subparsers)
    args = parser.parse_args(argv)
    try:
        return run_memory_command(args, root=Path(args.root))
    except (OSError, QueueError, RecoveryMemoryError) as exc:
        parser.error(str(exc))
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
