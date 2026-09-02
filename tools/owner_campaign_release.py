#!/usr/bin/env python3
"""Install and use one hash-bound owner-campaign workflow release.

The owner lanes intentionally keep their source worktrees independent from the
workflow checkout.  This small, standard-library-only launcher gives them a
stable entry point which first authenticates the workflow checkout, then runs
its absolute ``tools/agent.py`` with an explicit owner ``--root``.  The stable
copy is installed outside the workflow checkout; it is therefore safe to
update a release while owner source worktrees continue to work.

Commands::

    python tools/owner_campaign_release.py install \
        --release-root <workflow-checkout> --install-root <stable-root>
    python <stable-root>/owner_campaign_release.py run \
        --root <owner-worktree> crack loop --campaign ...
    python <stable-root>/owner_campaign_release.py status [--root <owner>]

``run`` is a pass-through command.  It never imports or executes a lane-local
``tools/agent.py`` and refuses a second ``--root`` in the pass-through tail.
The adoption receipt is deliberately compact and lives only below the lane's
``build/owner-campaign`` directory.
"""

from __future__ import annotations

import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import signal
import stat
import subprocess
import sys
import tempfile
import time
from typing import Any, Iterable, Mapping, Sequence


POINTER_SCHEMA = "owner_campaign_release_pointer/v1"
ADOPTION_SCHEMA = "owner_campaign_release_adoption/v1"
ADOPTION_LOCK_SCHEMA = "owner_campaign_release_adoption_lock/v1"
INSTALL_LOCK_SCHEMA = "owner_campaign_release_install_lock/v1"
STATUS_SCHEMA = "owner_campaign_release_status/v1"
POINTER_FILENAME = "release-pointer.json"
LAUNCHER_FILENAME = "owner_campaign_release.py"
ADOPTION_RELATIVE_PATH = Path("build") / "owner-campaign" / "release-adoption.json"
ADOPTION_LOCK_FILENAME = "release-adoption.lock"
INSTALL_LOCK_FILENAME = "release-install.lock"
AGENT_RELATIVE_PATH = Path("tools") / "agent.py"
MEASURE_RELATIVE_PATH = Path("tools") / "owner_campaign_measure.py"
SHA256_LENGTH = 64
COMMIT_LENGTH = 40


class ReleaseError(RuntimeError):
    """An invalid release, pointer, path, or adoption record."""


class ProcessTreeCleanupError(ReleaseError):
    """A child may survive, so the lane lock must remain poisoned."""


def _canonical(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def _sha_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _sha_file(path: Path) -> str:
    try:
        with path.open("rb") as stream:
            digest = hashlib.sha256()
            for block in iter(lambda: stream.read(1 << 20), b""):
                digest.update(block)
    except OSError as exc:
        raise ReleaseError(f"cannot hash {path}: {exc}") from exc
    return digest.hexdigest()


def _digest_json(value: Mapping[str, Any], field: str) -> str:
    payload = {key: item for key, item in value.items() if key != field}
    return _sha_bytes(_canonical(payload))


def _is_reparse(details: os.stat_result) -> bool:
    # FILE_ATTRIBUTE_REPARSE_POINT is available on Windows and harmless on
    # POSIX, where the attribute is normally absent.
    return bool(getattr(details, "st_file_attributes", 0) & 0x400)


def _path_from(raw: Any, label: str, *, require_absolute: bool = True) -> Path:
    if not isinstance(raw, (str, os.PathLike)):
        raise ReleaseError(f"{label} is not a path")
    text = os.fspath(raw)
    if not text or "\x00" in text:
        raise ReleaseError(f"{label} is invalid")
    try:
        path = Path(text).expanduser()
    except (OSError, RuntimeError) as exc:
        raise ReleaseError(f"{label} is invalid: {raw!r}") from exc
    if require_absolute and not path.is_absolute():
        raise ReleaseError(f"{label} must be absolute: {raw!r}")
    if not path.is_absolute():
        path = Path.cwd() / path
    return Path(os.path.abspath(path))


def _check_component(current: Path, label: str, *, directory: bool | None = None) -> None:
    try:
        details = current.lstat()
    except FileNotFoundError as exc:
        raise ReleaseError(f"{label} does not exist: {current}") from exc
    except OSError as exc:
        raise ReleaseError(f"cannot inspect {label} {current}: {exc}") from exc
    if current.is_symlink() or _is_reparse(details):
        raise ReleaseError(f"{label} uses symlink/reparse indirection: {current}")
    if directory is True and not stat.S_ISDIR(details.st_mode):
        raise ReleaseError(f"{label} is not a directory: {current}")
    if directory is False and not stat.S_ISREG(details.st_mode):
        raise ReleaseError(f"{label} is not a regular file: {current}")


def _check_tree(path: Path, label: str, *, allow_missing_leaf: bool = False) -> None:
    """Check every existing component without following links."""

    absolute = _path_from(path, label)
    current = Path(absolute.anchor)
    parts = absolute.relative_to(current).parts
    for index, part in enumerate(parts):
        current /= part
        try:
            details = current.lstat()
        except FileNotFoundError:
            if allow_missing_leaf and index == len(parts) - 1:
                return
            # Missing parents are safe to create only after their existing
            # ancestors have been checked; the creator performs a second check.
            return
        except OSError as exc:
            raise ReleaseError(f"cannot inspect {label} {current}: {exc}") from exc
        if current.is_symlink() or _is_reparse(details):
            raise ReleaseError(f"{label} uses symlink/reparse indirection: {current}")


def _ensure_directory(path: Path, label: str) -> Path:
    """Create a directory tree while rejecting links and non-directories."""

    absolute = _path_from(path, label)
    current = Path(absolute.anchor)
    for part in absolute.relative_to(current).parts:
        current /= part
        try:
            details = current.lstat()
        except FileNotFoundError:
            try:
                current.mkdir()
            except FileExistsError:
                # Recheck the object created by a concurrent writer below.
                pass
            except OSError as exc:
                raise ReleaseError(f"cannot create {label} {current}: {exc}") from exc
            _check_component(current, label, directory=True)
            continue
        except OSError as exc:
            raise ReleaseError(f"cannot inspect {label} {current}: {exc}") from exc
        if current.is_symlink() or _is_reparse(details):
            raise ReleaseError(f"{label} uses symlink/reparse indirection: {current}")
        if not stat.S_ISDIR(details.st_mode):
            raise ReleaseError(f"{label} is not a directory: {current}")
    _check_component(absolute, label, directory=True)
    return absolute


def _existing_directory(raw: Any, label: str) -> Path:
    path = _path_from(raw, label)
    _check_tree(path, label)
    _check_component(path, label, directory=True)
    return path


def _existing_file(path: Path, label: str) -> Path:
    _check_tree(path, label)
    _check_component(path, label, directory=False)
    return path


def _path_key(path: Path) -> str:
    return os.path.normcase(os.path.normpath(str(path)))


def _contains(parent: Path, child: Path) -> bool:
    parent_key = _path_key(parent)
    child_key = _path_key(child)
    try:
        common = os.path.commonpath([parent_key, child_key])
    except ValueError:
        return False
    return common == parent_key


def _reject_overlap(first: Path, second: Path, label: str) -> None:
    if _contains(first, second) or _contains(second, first):
        raise ReleaseError(f"{label} roots overlap")


def _run_git(root: Path, args: Sequence[str]) -> str:
    try:
        completed = subprocess.run(
            ["git", *args],
            cwd=root,
            text=True,
            capture_output=True,
            check=False,
        )
    except OSError as exc:
        raise ReleaseError(f"git could not run in {root}: {exc}") from exc
    if completed.returncode != 0:
        detail = (completed.stderr or completed.stdout).strip()
        raise ReleaseError(f"git {' '.join(args)} failed in {root}: {detail}")
    return completed.stdout.strip()


def _verify_commit(value: str, label: str) -> str:
    result = value.strip().lower()
    if len(result) != COMMIT_LENGTH or any(char not in "0123456789abcdef" for char in result):
        raise ReleaseError(f"{label} is not a full commit SHA-1: {value!r}")
    return result


def _verify_sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or len(value) != SHA256_LENGTH:
        raise ReleaseError(f"{label} is not a SHA-256")
    result = value.lower()
    if any(char not in "0123456789abcdef" for char in result):
        raise ReleaseError(f"{label} is not a SHA-256")
    return result


def verify_release(release_root: Any, *, expected_commit: str | None = None) -> dict[str, str]:
    """Authenticate a clean release checkout and its executable inputs."""

    root = _existing_directory(release_root, "release root")
    # Do not compare ``rev-parse --show-toplevel`` text with ``Path`` here:
    # Git for Windows may return an MSYS (/c/...) spelling while Python uses a
    # native (C:\\...) spelling.  The explicit ``cwd=root`` plus the successful
    # HEAD/status queries already binds these checks to this checkout.
    commit = _verify_commit(_run_git(root, ["rev-parse", "--verify", "HEAD"]), "release HEAD")
    if expected_commit is not None and commit != _verify_commit(expected_commit, "expected release commit"):
        raise ReleaseError(f"release HEAD drift: {commit} != {expected_commit}")
    dirty = _run_git(root, ["status", "--porcelain=v1", "--untracked-files=all"])
    if dirty:
        raise ReleaseError(f"release checkout is not clean: {root}")

    agent = _existing_file(root / AGENT_RELATIVE_PATH, "release agent")
    measure = _existing_file(root / MEASURE_RELATIVE_PATH, "release measurement producer")
    return {
        "release_root": str(root),
        "commit": commit,
        "agent_sha256": _sha_file(agent),
        "measurement_sha256": _sha_file(measure),
    }


def _read_json(path: Path, label: str) -> dict[str, Any]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ReleaseError(f"cannot read {label} {path}: {exc}") from exc
    if not isinstance(payload, dict):
        raise ReleaseError(f"{label} is not a JSON object: {path}")
    return payload


def _write_atomic(path: Path, value: Mapping[str, Any], label: str) -> str:
    """Publish one canonical JSON file, never writing through a symlink."""

    parent = _ensure_directory(path.parent, f"{label} directory")
    _check_tree(path, label, allow_missing_leaf=True)
    try:
        payload = _canonical(value)
        digest = _sha_bytes(payload)
        descriptor = dict(value)
        if "pointer_sha256" in descriptor and descriptor["pointer_sha256"] is None:
            descriptor["pointer_sha256"] = digest
        elif "receipt_sha256" in descriptor and descriptor["receipt_sha256"] is None:
            descriptor["receipt_sha256"] = digest
        else:
            descriptor = dict(value)
        payload = _canonical(descriptor)
        fd, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=parent)
        temporary = Path(temporary_name)
        try:
            with os.fdopen(fd, "wb") as stream:
                stream.write(payload)
                stream.flush()
                os.fsync(stream.fileno())
            # A final leaf check prevents replacing a newly-created symlink.
            _check_tree(path, label, allow_missing_leaf=True)
            os.replace(temporary, path)
            try:
                directory_fd = os.open(parent, os.O_RDONLY)
            except OSError:
                directory_fd = None
            if directory_fd is not None:
                try:
                    os.fsync(directory_fd)
                except OSError:
                    pass
                finally:
                    os.close(directory_fd)
        finally:
            try:
                os.unlink(temporary)
            except FileNotFoundError:
                pass
    except OSError as exc:
        raise ReleaseError(f"atomic {label} publication failed: {path}: {exc}") from exc
    return _sha_bytes(payload)


def _release_descriptor(identity: Mapping[str, str]) -> dict[str, str]:
    return {
        "release_root": identity["release_root"],
        "commit": identity["commit"],
        "agent_sha256": identity["agent_sha256"],
        "measurement_sha256": identity["measurement_sha256"],
    }


def _pointer_payload(
    identity: Mapping[str, str],
    launcher_sha256: str,
    installed_at: str,
    *,
    fallback: Mapping[str, str] | None = None,
) -> dict[str, Any]:
    payload: dict[str, Any] = {
        "schema": POINTER_SCHEMA,
        "release_root": identity["release_root"],
        "commit": identity["commit"],
        "agent_sha256": identity["agent_sha256"],
        "measurement_sha256": identity["measurement_sha256"],
        "launcher_path": LAUNCHER_FILENAME,
        "launcher_sha256": launcher_sha256,
        "installed_at": installed_at,
        # Exactly one prior generation is retained.  It is authenticated again
        # at selection time; this descriptor is never authority by itself.
        "fallback": dict(fallback) if fallback is not None else None,
    }
    payload["pointer_sha256"] = _digest_json(payload, "pointer_sha256")
    return payload


def _release_descriptor_fields(value: Any, label: str) -> None:
    expected = {"release_root", "commit", "agent_sha256", "measurement_sha256"}
    if not isinstance(value, Mapping) or set(value) != expected:
        raise ReleaseError(f"{label} has an invalid field set")
    if not isinstance(value["release_root"], str) or not value["release_root"]:
        raise ReleaseError(f"{label} release root is invalid")
    _verify_commit(value["commit"], f"{label} commit")
    _verify_sha(value["agent_sha256"], f"{label} agent_sha256")
    _verify_sha(value["measurement_sha256"], f"{label} measurement_sha256")


def _pointer_fields(pointer: Mapping[str, Any]) -> None:
    expected = {
        "schema",
        "release_root",
        "commit",
        "agent_sha256",
        "measurement_sha256",
        "launcher_path",
        "launcher_sha256",
        "installed_at",
        "pointer_sha256",
        "fallback",
    }
    legacy_expected = expected - {"fallback"}
    if frozenset(pointer) not in {frozenset(expected), frozenset(legacy_expected)}:
        raise ReleaseError("release pointer has an invalid field set")
    if pointer["schema"] != POINTER_SCHEMA:
        raise ReleaseError("release pointer schema is unsupported")
    _verify_commit(pointer["commit"], "pointer commit")
    for field in ("agent_sha256", "measurement_sha256", "launcher_sha256", "pointer_sha256"):
        _verify_sha(pointer[field], f"pointer {field}")
    if pointer["launcher_path"] != LAUNCHER_FILENAME:
        raise ReleaseError("release pointer launcher path is invalid")
    if not isinstance(pointer["release_root"], str) or not pointer["release_root"]:
        raise ReleaseError("release pointer release root is invalid")
    if not isinstance(pointer["installed_at"], str) or not pointer["installed_at"]:
        raise ReleaseError("release pointer install time is invalid")
    fallback = pointer.get("fallback")
    if fallback is not None:
        _release_descriptor_fields(fallback, "pointer fallback")
    if _digest_json(pointer, "pointer_sha256") != pointer["pointer_sha256"]:
        raise ReleaseError("release pointer self-hash mismatch")


def _load_pointer(install_root: Any) -> tuple[Path, dict[str, Any]]:
    root = _existing_directory(install_root, "install root")
    path = root / POINTER_FILENAME
    pointer = _read_json(_existing_file(path, "release pointer"), "release pointer")
    _pointer_fields(pointer)
    return root, pointer


def _verify_stable_pointer(install_root: Any) -> tuple[Path, dict[str, Any]]:
    root, pointer = _load_pointer(install_root)
    launcher = _existing_file(root / LAUNCHER_FILENAME, "stable launcher")
    if _sha_file(launcher) != pointer["launcher_sha256"]:
        raise ReleaseError("stable launcher hash drift")
    return root, pointer


def _verify_release_descriptor(
    descriptor: Mapping[str, Any], label: str
) -> dict[str, str]:
    _release_descriptor_fields(descriptor, label)
    release = verify_release(
        descriptor["release_root"], expected_commit=descriptor["commit"]
    )
    for field in ("agent_sha256", "measurement_sha256"):
        if release[field] != descriptor[field]:
            raise ReleaseError(f"{label} {field} drift")
    return release


def _select_pointer_release(
    install_root: Any,
) -> tuple[Path, dict[str, Any], dict[str, str], str, str | None]:
    """Select current release, or its single verified pre-spawn fallback."""

    root, pointer = _verify_stable_pointer(install_root)
    current = _release_descriptor(pointer)
    try:
        release = _verify_release_descriptor(current, "release pointer")
        return root, pointer, release, "current", None
    except ReleaseError as current_error:
        fallback = pointer.get("fallback")
        if fallback is None:
            raise
        try:
            release = _verify_release_descriptor(fallback, "release pointer fallback")
        except ReleaseError as fallback_error:
            raise ReleaseError(
                f"current release validation failed ({current_error}); "
                f"fallback validation failed ({fallback_error})"
            ) from fallback_error
        return root, pointer, release, "fallback", str(current_error)


def _verify_pointer(install_root: Any) -> tuple[Path, dict[str, Any], dict[str, str]]:
    """Compatibility helper: require the pointer's current release."""

    root, pointer = _verify_stable_pointer(install_root)
    release = _verify_release_descriptor(_release_descriptor(pointer), "release pointer")
    return root, pointer, release


def install_release(
    release_root: Any,
    install_root: Any,
    *,
    expected_commit: str | None = None,
) -> dict[str, Any]:
    """Install a stable launcher and publish its pointer last."""

    release = verify_release(release_root, expected_commit=expected_commit)
    release_path = Path(release["release_root"])
    # Reject an overlapping destination before creating any directory.  This
    # prevents a malformed install root from mutating the release checkout.
    install_path = _path_from(install_root, "install root")
    _reject_overlap(release_path, install_path, "release/install")
    install_path = _ensure_directory(install_path, "install root")
    with _install_lock(install_path) as install_lock:
        source_launcher = _existing_file(
            release_path / "tools" / LAUNCHER_FILENAME,
            "release launcher",
        )
        try:
            launcher_bytes = source_launcher.read_bytes()
        except OSError as exc:
            raise ReleaseError(
                f"cannot read release launcher {source_launcher}: {exc}"
            ) from exc
        launcher_sha256 = _sha_bytes(launcher_bytes)
        stable_launcher = install_path / LAUNCHER_FILENAME
        pointer_path = install_path / POINTER_FILENAME
        _check_tree(stable_launcher, "stable launcher", allow_missing_leaf=True)
        _check_tree(pointer_path, "release pointer", allow_missing_leaf=True)

        # Keep byte-exact snapshots so pointer publication/verification is a
        # transaction from the caller's perspective.  The launcher is necessarily
        # published before its pointer, but any later failure restores the former
        # launcher and former pointer (or their prior absence).
        previous_launcher = _optional_file_bytes(stable_launcher, "stable launcher")
        previous_pointer = _optional_file_bytes(pointer_path, "release pointer")
        fallback: Mapping[str, str] | None = None
        if previous_pointer is not None:
            # The prior pointer must itself be structurally and launcher-hash
            # valid before its current generation is admitted as fallback.
            _, prior_pointer = _verify_stable_pointer(install_path)
            fallback = _release_descriptor(prior_pointer)
        pointer = _pointer_payload(
            release,
            launcher_sha256,
            datetime.now(timezone.utc).isoformat(timespec="microseconds"),
            fallback=fallback,
        )
        try:
            # Publish the copied launcher before the pointer; status never observes
            # a pointer whose launcher is not already complete.
            _write_atomic_bytes(stable_launcher, launcher_bytes, "stable launcher")
            _write_atomic(pointer_path, pointer, "release pointer")
            # Re-read and authenticate current generation before claiming success.
            _verify_pointer(install_path)
        except BaseException as exc:
            try:
                _restore_install_state(
                    stable_launcher,
                    pointer_path,
                    previous_launcher=previous_launcher,
                    previous_pointer=previous_pointer,
                )
            except ReleaseError as rollback_exc:
                raise ReleaseError(
                    f"release installation failed ({exc}); rollback failed ({rollback_exc})"
                ) from rollback_exc
            if isinstance(exc, ReleaseError):
                raise
            if isinstance(exc, (KeyboardInterrupt, SystemExit)):
                raise
            raise ReleaseError(f"release installation failed: {exc}") from exc
    result = dict(pointer)
    result.update({"status": "installed", "install_root": str(install_path)})
    if install_lock.cleanup_error is not None:
        result["cleanup_incomplete"] = install_lock.cleanup_error
        result["cleanup_error"] = install_lock.cleanup_error
    return result


def _write_atomic_bytes(path: Path, payload: bytes, label: str) -> str:
    parent = _ensure_directory(path.parent, f"{label} directory")
    _check_tree(path, label, allow_missing_leaf=True)
    fd = -1
    temporary: Path | None = None
    try:
        fd, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=parent)
        temporary = Path(temporary_name)
        with os.fdopen(fd, "wb") as stream:
            fd = -1
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        _check_tree(path, label, allow_missing_leaf=True)
        os.replace(temporary, path)
    except OSError as exc:
        raise ReleaseError(f"atomic {label} publication failed: {path}: {exc}") from exc
    finally:
        if fd >= 0:
            os.close(fd)
        if temporary is not None:
            try:
                os.unlink(temporary)
            except FileNotFoundError:
                pass
    return _sha_bytes(payload)


def _optional_file_bytes(path: Path, label: str) -> bytes | None:
    """Read an optional regular file without accepting path indirection."""

    _check_tree(path, label, allow_missing_leaf=True)
    try:
        details = path.lstat()
    except FileNotFoundError:
        return None
    except OSError as exc:
        raise ReleaseError(f"cannot inspect {label} {path}: {exc}") from exc
    if path.is_symlink() or _is_reparse(details) or not stat.S_ISREG(details.st_mode):
        raise ReleaseError(f"{label} is not a regular file: {path}")
    try:
        return path.read_bytes()
    except OSError as exc:
        raise ReleaseError(f"cannot read {label} {path}: {exc}") from exc


def _remove_optional_file(path: Path, label: str) -> None:
    _check_tree(path, label, allow_missing_leaf=True)
    try:
        details = path.lstat()
    except FileNotFoundError:
        return
    except OSError as exc:
        raise ReleaseError(f"cannot inspect {label} {path}: {exc}") from exc
    if path.is_symlink() or _is_reparse(details) or not stat.S_ISREG(details.st_mode):
        raise ReleaseError(f"{label} is not a regular file: {path}")
    try:
        path.unlink()
    except OSError as exc:
        raise ReleaseError(f"cannot remove {label} {path}: {exc}") from exc


def _restore_install_state(
    stable_launcher: Path,
    pointer_path: Path,
    *,
    previous_launcher: bytes | None,
    previous_pointer: bytes | None,
) -> None:
    """Restore the exact pre-install launcher/pointer pair, pointer last."""

    # Withdraw any possibly-new pointer before changing its launcher.
    _remove_optional_file(pointer_path, "release pointer rollback")
    if previous_launcher is None:
        _remove_optional_file(stable_launcher, "stable launcher rollback")
    else:
        _write_atomic_bytes(
            stable_launcher,
            previous_launcher,
            "stable launcher rollback",
        )
    if previous_pointer is not None:
        _write_atomic_bytes(pointer_path, previous_pointer, "release pointer rollback")


def _iso_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="microseconds")


def _git_head(owner_root: Path) -> str:
    return _verify_commit(_run_git(owner_root, ["rev-parse", "--verify", "HEAD"]), "lane HEAD")


def _argument_value(args: Sequence[str], names: Iterable[str]) -> str | None:
    names_set = set(names)
    for index, item in enumerate(args):
        if item in names_set:
            if index + 1 >= len(args):
                raise ReleaseError(f"{item} requires a path")
            return args[index + 1]
        for name in names_set:
            prefix = f"{name}="
            if item.startswith(prefix):
                return item[len(prefix):]
    return None


def _lane_file_descriptor(
    owner_root: Path,
    raw: Any,
    label: str,
    *,
    require_declared_sha: bool = False,
) -> dict[str, str] | None:
    if raw is None:
        return None
    if isinstance(raw, Mapping):
        raw_path = raw.get("path")
        declared_sha = raw.get("sha256")
    else:
        raw_path = raw
        declared_sha = None
    if not isinstance(raw_path, (str, os.PathLike)):
        raise ReleaseError(f"{label} path is invalid")
    candidate = Path(os.fspath(raw_path))
    if not candidate.is_absolute():
        candidate = owner_root / candidate
    candidate = Path(os.path.abspath(candidate))
    try:
        candidate.relative_to(owner_root)
    except ValueError as exc:
        raise ReleaseError(f"{label} escapes owner root: {candidate}") from exc
    file_path = _existing_file(candidate, label)
    actual_sha = _sha_file(file_path)
    if require_declared_sha:
        expected_sha = _verify_sha(declared_sha, f"{label}.sha256")
        if expected_sha != actual_sha:
            raise ReleaseError(f"{label} declared hash drift: {actual_sha} != {expected_sha}")
    return {
        "path": file_path.relative_to(owner_root).as_posix(),
        "sha256": actual_sha,
    }


def _lane_bindings(owner_root: Path, agent_args: Sequence[str]) -> dict[str, Any]:
    """Capture lane commit, manifest and content-addressed tool identities."""

    lane_head = _git_head(owner_root)
    manifest_raw = _argument_value(
        agent_args,
        ("--campaign", "--manifest", "--campaign-manifest"),
    )
    manifest: dict[str, str] | None = None
    tool_cas: list[dict[str, str]] = []
    if manifest_raw is not None:
        manifest = _lane_file_descriptor(owner_root, manifest_raw, "lane campaign manifest")
        assert manifest is not None
        manifest_path = owner_root / Path(manifest["path"])
        document = _read_json(manifest_path, "lane campaign manifest")
        for role in ("toolchain", "measurement_producer"):
            binding = document.get(role)
            if isinstance(binding, Mapping):
                descriptor = _lane_file_descriptor(
                    owner_root,
                    binding,
                    f"lane {role}",
                    require_declared_sha=True,
                )
                if descriptor is not None:
                    parts = Path(descriptor["path"]).parts
                    if (
                        len(parts) < 5
                        or parts[:3] != ("build", "owner-campaign", "tool-cas")
                        or parts[3] != descriptor["sha256"]
                    ):
                        raise ReleaseError(
                            f"lane {role} is not in canonical tool CAS: {descriptor['path']}"
                        )
                    descriptor["role"] = role
                    tool_cas.append(descriptor)
    return {
        "lane_root": str(owner_root),
        "lane_head": lane_head,
        "manifest": manifest,
        "tool_cas": tool_cas,
    }


def _adoption_path(owner_root: Path, *, create: bool = True) -> Path:
    if create:
        build_dir = _ensure_directory(owner_root / "build", "lane build directory")
        campaign_dir = _ensure_directory(
            build_dir / "owner-campaign", "lane owner-campaign directory"
        )
    else:
        build_dir = owner_root / "build"
        campaign_dir = build_dir / "owner-campaign"
        _check_tree(build_dir, "lane build directory", allow_missing_leaf=True)
        _check_tree(campaign_dir, "lane owner-campaign directory", allow_missing_leaf=True)
    path = campaign_dir / "release-adoption.json"
    _check_tree(path, "adoption receipt", allow_missing_leaf=True)
    return path


class _HeldLock:
    """Identity-bound cross-process lock whose safety state is explicit."""

    def __init__(self, path: Path, label: str, payload: Mapping[str, Any]) -> None:
        self.path = path
        self.label = label
        self.payload = dict(payload)
        self.sha256 = _sha_bytes(_canonical(self.payload))
        self.cleanup_error: str | None = None

    def _publish_state(self, *, recoverable: bool, reason: str | None) -> None:
        updated = dict(self.payload)
        updated["recoverable"] = recoverable
        updated["retained_reason"] = reason
        # Serialize every lock-path mutation.  Hash-then-unlink alone is not
        # sufficient: another contender can replace the pathname between
        # those operations (the classic stale-lock ABA race).
        with _lock_path_guard(self.path, self.label):
            existing = _existing_file(self.path, self.label)
            if _sha_file(existing) != self.sha256:
                raise ReleaseError(f"{self.label} identity changed while held")
            self.sha256 = _write_atomic(self.path, updated, self.label)
        self.payload = updated

    def arm_process_tree(self, child_pid: int | None = None) -> None:
        """Make a crash-stale lane lock nonrecoverable before process spawn."""

        updated = dict(self.payload)
        updated["child_pid"] = child_pid
        self.payload = updated
        self._publish_state(
            recoverable=False,
            reason="released process tree termination is not yet proved",
        )

    def child_started(self, child_pid: int) -> None:
        updated = dict(self.payload)
        updated["child_pid"] = child_pid
        self.payload = updated
        self._publish_state(
            recoverable=False,
            reason="released process tree termination is not yet proved",
        )

    def mark_tree_proved(self) -> None:
        self._publish_state(recoverable=True, reason=None)

    def retain_unproved(self, reason: str) -> None:
        self._publish_state(recoverable=False, reason=reason)


def _read_lock(path: Path, label: str, schema: str) -> dict[str, Any]:
    existing = _existing_file(path, label)
    try:
        raw = existing.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as exc:
        raise ReleaseError(f"cannot read {label} {path}: {exc}") from exc
    try:
        value = json.loads(raw)
    except json.JSONDecodeError as exc:
        raise ReleaseError(f"cannot parse {label} {path}: {exc}") from exc
    # Compatibility with the initial adoption-lock release, which wrote only
    # its PID.  New locks are always fully-written JSON before publication.
    if (
        isinstance(value, int)
        and not isinstance(value, bool)
        and schema == ADOPTION_LOCK_SCHEMA
    ):
        return {
            "schema": schema,
            "pid": value,
            "created_at": "legacy",
            "recoverable": True,
            "retained_reason": None,
            "child_pid": None,
        }
    if not isinstance(value, Mapping):
        raise ReleaseError(f"{label} is not a JSON object")
    expected = {
        "schema",
        "pid",
        "created_at",
        "recoverable",
        "retained_reason",
        "child_pid",
    }
    if set(value) != expected or value.get("schema") != schema:
        raise ReleaseError(f"{label} schema/fields are unsupported")
    pid = value.get("pid")
    if not isinstance(pid, int) or isinstance(pid, bool) or pid <= 0:
        raise ReleaseError(f"{label} owner PID is invalid")
    if not isinstance(value.get("recoverable"), bool):
        raise ReleaseError(f"{label} recoverability is invalid")
    child_pid = value.get("child_pid")
    if child_pid is not None and (
        not isinstance(child_pid, int) or isinstance(child_pid, bool) or child_pid <= 0
    ):
        raise ReleaseError(f"{label} child PID is invalid")
    return value


@contextmanager
def _lock_path_guard(path: Path, label: str) -> Iterable[None]:
    """Kernel-lock a persistent guard while inspecting/mutating ``path``.

    A persistent advisory-lock inode avoids introducing another reclaimable
    pathname whose stale handling would have the same ABA problem.  All
    cooperative acquire, state-update, reclaim, and release operations pass
    through this guard, so the identity check and unlink are one serialized
    transaction.
    """

    guard_key = _sha_bytes(os.path.normcase(os.path.abspath(path)).encode("utf-8"))
    if os.name == "nt":
        import ctypes

        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        create_mutex = kernel32.CreateMutexW
        create_mutex.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_wchar_p]
        create_mutex.restype = ctypes.c_void_p
        wait = kernel32.WaitForSingleObject
        wait.argtypes = [ctypes.c_void_p, ctypes.c_ulong]
        wait.restype = ctypes.c_ulong
        release_mutex = kernel32.ReleaseMutex
        release_mutex.argtypes = [ctypes.c_void_p]
        release_mutex.restype = ctypes.c_int
        close = kernel32.CloseHandle
        close.argtypes = [ctypes.c_void_p]
        close.restype = ctypes.c_int

        handle = create_mutex(None, False, f"Local\\mp6-owner-campaign-{guard_key}")
        if not handle:
            raise ReleaseError(
                f"cannot create {label} guard mutex: {ctypes.get_last_error()}"
            )
        acquired = False
        try:
            outcome = wait(handle, 0xFFFFFFFF)
            if outcome not in {0, 0x80}:  # WAIT_OBJECT_0 / WAIT_ABANDONED
                raise ReleaseError(
                    f"cannot acquire {label} guard mutex: wait result {outcome}"
                )
            acquired = True
            yield
        finally:
            if acquired:
                release_mutex(handle)
            close(handle)
        return

    # POSIX advisory locks need an inode which outlives every contender.  Keep
    # it outside the managed install/lane tree so a failed fresh transaction
    # leaves that tree byte-for-byte empty.
    guard_path = Path(tempfile.gettempdir()) / f"mp6-owner-campaign-{guard_key}.guard"
    _check_tree(guard_path, f"{label} guard", allow_missing_leaf=True)
    flags = os.O_RDWR | os.O_CREAT
    if hasattr(os, "O_NOFOLLOW"):
        flags |= os.O_NOFOLLOW
    try:
        fd = os.open(guard_path, flags, 0o600)
    except OSError as exc:
        raise ReleaseError(f"cannot open {label} guard {guard_path}: {exc}") from exc
    locked = False
    try:
        details = os.fstat(fd)
        if not stat.S_ISREG(details.st_mode) or _is_reparse(details):
            raise ReleaseError(f"{label} guard is not a regular file: {guard_path}")
        if details.st_size == 0:
            os.write(fd, b"\0")
            os.fsync(fd)
        os.lseek(fd, 0, os.SEEK_SET)
        if os.name == "nt":
            import msvcrt

            try:
                msvcrt.locking(fd, msvcrt.LK_LOCK, 1)
            except OSError as exc:
                raise ReleaseError(
                    f"cannot acquire {label} guard {guard_path}: {exc}"
                ) from exc
        else:
            import fcntl

            try:
                fcntl.flock(fd, fcntl.LOCK_EX)
            except OSError as exc:
                raise ReleaseError(
                    f"cannot acquire {label} guard {guard_path}: {exc}"
                ) from exc
        locked = True
        yield
    finally:
        if locked:
            try:
                os.lseek(fd, 0, os.SEEK_SET)
                if os.name == "nt":
                    import msvcrt

                    msvcrt.locking(fd, msvcrt.LK_UNLCK, 1)
                else:
                    import fcntl

                    fcntl.flock(fd, fcntl.LOCK_UN)
            except OSError:
                # Closing the descriptor releases the kernel lock.  The
                # protected outcome stays primary; later contenders still
                # serialize on the same persistent guard inode.
                pass
        os.close(fd)


def _publish_exclusive_lock(path: Path, payload: bytes, label: str) -> bool:
    """Atomically link a fully-written temp file into the lock pathname."""

    parent = _ensure_directory(path.parent, f"{label} directory")
    fd = -1
    temporary: Path | None = None
    try:
        fd, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=parent)
        temporary = Path(temporary_name)
        with os.fdopen(fd, "wb") as stream:
            fd = -1
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        _check_tree(path, label, allow_missing_leaf=True)
        try:
            os.link(temporary, path)
        except FileExistsError:
            return False
        return True
    except OSError as exc:
        raise ReleaseError(f"cannot atomically publish {label} {path}: {exc}") from exc
    finally:
        if fd >= 0:
            os.close(fd)
        if temporary is not None:
            try:
                os.unlink(temporary)
            except FileNotFoundError:
                pass


@contextmanager
def _cross_process_lock(
    directory: Path,
    filename: str,
    label: str,
    schema: str,
    *,
    wait_timeout: float | None = None,
) -> Iterable[_HeldLock]:
    """Acquire a complete O_EXCL-equivalent lock with safe stale recovery."""

    root = _ensure_directory(directory, f"{label} directory")
    lock_path = root / filename
    _check_tree(lock_path, label, allow_missing_leaf=True)
    payload = {
        "schema": schema,
        "pid": os.getpid(),
        "created_at": _iso_now(),
        "recoverable": True,
        "retained_reason": None,
        "child_pid": None,
    }
    encoded = _canonical(payload)
    acquired = False
    stale_attempts = 0
    deadline = None if wait_timeout is None else time.monotonic() + wait_timeout
    while True:
        wait_for_holder = False
        with _lock_path_guard(lock_path, label):
            if _publish_exclusive_lock(lock_path, encoded, label):
                acquired = True
                break
            prior = _read_lock(lock_path, label, schema)
            if _pid_alive(prior["pid"]):
                wait_for_holder = True
            else:
                if not prior["recoverable"]:
                    reason = prior.get("retained_reason") or "unproved prior operation"
                    raise ReleaseError(f"{label} is retained fail-closed: {reason}")
                # The guard makes read/identity decision/unlink indivisible
                # with respect to every cooperative holder and contender.
                try:
                    lock_path.unlink()
                except FileNotFoundError as exc:
                    raise ReleaseError(
                        f"{label} identity vanished during guarded stale recovery"
                    ) from exc
                except OSError as exc:
                    raise ReleaseError(
                        f"cannot recover stale {label} {lock_path}: {exc}"
                    ) from exc
                stale_attempts += 1
                if stale_attempts >= 4:
                    raise ReleaseError(f"stale {label} recovery did not converge")
        if wait_for_holder:
            if deadline is not None and time.monotonic() < deadline:
                time.sleep(0.05)
                continue
            raise ReleaseError(f"{label} is held by live PID {prior['pid']}")
    if not acquired:
        raise ReleaseError(f"cannot acquire {label}")

    held = _HeldLock(lock_path, label, payload)
    primary: BaseException | None = None
    try:
        yield held
    except BaseException as exc:
        primary = exc
        raise
    finally:
        try:
            with _lock_path_guard(lock_path, label):
                existing = _existing_file(lock_path, label)
                if _sha_file(existing) != held.sha256:
                    raise ReleaseError(f"{label} identity changed while held")
                if held.payload["recoverable"]:
                    lock_path.unlink()
                # A deliberately poisoned lock survives manager exit and
                # cannot be reclaimed merely because this PID becomes stale.
        except (OSError, ReleaseError) as cleanup_exc:
            wrapped = (
                cleanup_exc
                if isinstance(cleanup_exc, ReleaseError)
                else ReleaseError(f"cannot release {label} {lock_path}: {cleanup_exc}")
            )
            if primary is not None:
                if hasattr(primary, "add_note"):
                    primary.add_note(f"{label} cleanup also failed: {wrapped}")
            else:
                # The protected operation's result/receipt remains primary.
                # Keep the lock in place so a concurrent caller cannot enter;
                # because the process tree was proved before it became
                # recoverable, a future dead-PID recovery is safe.
                held.cleanup_error = str(wrapped)


def _adoption_lock(owner_root: Path) -> Any:
    receipt_path = _adoption_path(owner_root)
    return _cross_process_lock(
        receipt_path.parent,
        ADOPTION_LOCK_FILENAME,
        "adoption lock",
        ADOPTION_LOCK_SCHEMA,
    )


def _install_lock(install_root: Path) -> Any:
    return _cross_process_lock(
        install_root,
        INSTALL_LOCK_FILENAME,
        "release install lock",
        INSTALL_LOCK_SCHEMA,
        wait_timeout=30.0,
    )


def _receipt_payload(
    pointer: Mapping[str, Any],
    owner_root: Path,
    agent_args: Sequence[str],
    lane: Mapping[str, Any],
    *,
    release: Mapping[str, str] | None = None,
    workflow_selection: str = "current",
    status: str,
    started_at: str,
    finished_at: str | None,
    terminal_status: str | None,
    exit_code: int | None,
    child_pid: int | None,
    child_argv: Sequence[str],
    error: str | None = None,
    lane_after: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    selected_release = release if release is not None else _release_descriptor(pointer)
    payload: dict[str, Any] = {
        "schema": ADOPTION_SCHEMA,
        "status": status,
        "terminal_status": terminal_status,
        "owner_root": str(owner_root),
        "release_root": selected_release["release_root"],
        "workflow_commit": selected_release["commit"],
        "agent_sha256": selected_release["agent_sha256"],
        "measurement_sha256": selected_release["measurement_sha256"],
        "launcher_sha256": pointer["launcher_sha256"],
        "pointer_sha256": pointer["pointer_sha256"],
        "pointer_commit": pointer["commit"],
        "workflow_selection": workflow_selection,
        "lane": dict(lane),
        "lane_after": dict(lane_after) if lane_after is not None else None,
        "agent_argv": list(agent_args),
        "child_argv": list(child_argv),
        "child_pid": child_pid,
        "command_sha256": _sha_bytes(_canonical(list(agent_args))),
        "started_at": started_at,
        "finished_at": finished_at,
        "exit_code": exit_code,
        "error": error,
    }
    payload["receipt_sha256"] = _digest_json(payload, "receipt_sha256")
    return payload


def _receipt_fields(receipt: Mapping[str, Any]) -> None:
    expected = {
        "schema",
        "status",
        "terminal_status",
        "owner_root",
        "release_root",
        "workflow_commit",
        "agent_sha256",
        "measurement_sha256",
        "launcher_sha256",
        "lane",
        "lane_after",
        "agent_argv",
        "child_argv",
        "child_pid",
        "command_sha256",
        "started_at",
        "finished_at",
        "exit_code",
        "error",
        "receipt_sha256",
        "pointer_sha256",
        "pointer_commit",
        "workflow_selection",
    }
    legacy_expected = expected - {
        "pointer_sha256",
        "pointer_commit",
        "workflow_selection",
    }
    if frozenset(receipt) not in {frozenset(expected), frozenset(legacy_expected)}:
        raise ReleaseError("adoption receipt has an invalid field set")
    if receipt["schema"] != ADOPTION_SCHEMA:
        raise ReleaseError("adoption receipt schema is unsupported")
    if receipt["status"] not in {"active", "terminal"}:
        raise ReleaseError("adoption receipt status is invalid")
    if receipt["status"] == "active":
        if receipt["terminal_status"] is not None or receipt["finished_at"] is not None:
            raise ReleaseError("active adoption receipt is already terminal")
    else:
        if receipt["terminal_status"] not in {
            "completed",
            "failed",
            "timed_out",
            "cleanup_unproved",
        }:
            raise ReleaseError("terminal adoption status is invalid")
        if not isinstance(receipt["finished_at"], str) or not receipt["finished_at"]:
            raise ReleaseError("terminal adoption receipt has no finish time")
    _verify_commit(receipt["workflow_commit"], "adoption workflow commit")
    for field in ("agent_sha256", "measurement_sha256", "launcher_sha256", "command_sha256", "receipt_sha256"):
        _verify_sha(receipt[field], f"adoption {field}")
    if "workflow_selection" in receipt:
        if receipt["workflow_selection"] not in {"current", "fallback"}:
            raise ReleaseError("adoption workflow selection is invalid")
        _verify_sha(receipt["pointer_sha256"], "adoption pointer_sha256")
        _verify_commit(receipt["pointer_commit"], "adoption pointer_commit")
    if not isinstance(receipt["owner_root"], str) or not receipt["owner_root"]:
        raise ReleaseError("adoption owner root is invalid")
    if not isinstance(receipt["release_root"], str) or not receipt["release_root"]:
        raise ReleaseError("adoption release root is invalid")
    if not isinstance(receipt["lane"], Mapping):
        raise ReleaseError("adoption lane binding is invalid")
    if not isinstance(receipt["agent_argv"], list) or not all(
        isinstance(item, str) for item in receipt["agent_argv"]
    ):
        raise ReleaseError("adoption agent argv is invalid")
    if not isinstance(receipt["child_argv"], list) or not all(
        isinstance(item, str) for item in receipt["child_argv"]
    ):
        raise ReleaseError("adoption child argv is invalid")
    if receipt["child_pid"] is not None and (
        not isinstance(receipt["child_pid"], int) or receipt["child_pid"] <= 0
    ):
        raise ReleaseError("adoption child pid is invalid")
    if _digest_json(receipt, "receipt_sha256") != receipt["receipt_sha256"]:
        raise ReleaseError("adoption receipt self-hash mismatch")


def _load_adoption(path: Path) -> dict[str, Any] | None:
    try:
        _check_component(path, "adoption receipt", directory=False)
    except ReleaseError as exc:
        if "does not exist" in str(exc):
            return None
        raise
    receipt = _read_json(path, "adoption receipt")
    _receipt_fields(receipt)
    return receipt


def _receipt_binding(
    receipt: Mapping[str, Any], pointer: Mapping[str, Any], owner_root: Path
) -> None:
    """Ensure a lane receipt belongs to this pointer and lane, not just itself."""

    _receipt_owner_binding(receipt, owner_root)
    if "workflow_selection" in receipt:
        if receipt["pointer_sha256"] != pointer["pointer_sha256"]:
            raise ReleaseError("adoption receipt pointer hash drift")
        if receipt["pointer_commit"] != pointer["commit"]:
            raise ReleaseError("adoption receipt pointer commit drift")
        if receipt["workflow_selection"] == "fallback":
            descriptor = pointer.get("fallback")
            if descriptor is None:
                raise ReleaseError("adoption receipt fallback is no longer declared")
        else:
            descriptor = pointer
    else:
        descriptor = pointer
    for field in (
        ("release_root", "release_root"),
        ("workflow_commit", "commit"),
        ("agent_sha256", "agent_sha256"),
        ("measurement_sha256", "measurement_sha256"),
    ):
        receipt_field, pointer_field = field
        if receipt[receipt_field] != descriptor[pointer_field]:
            raise ReleaseError(f"adoption receipt {receipt_field} drift")
    if receipt["launcher_sha256"] != pointer["launcher_sha256"]:
        raise ReleaseError("adoption receipt launcher_sha256 drift")


def _receipt_owner_binding(receipt: Mapping[str, Any], owner_root: Path) -> None:
    """Bind a valid receipt to its lane without requiring the current release."""

    if _path_key(Path(receipt["owner_root"])) != _path_key(owner_root):
        raise ReleaseError("adoption receipt owner root drift")
    lane = receipt["lane"]
    lane_root_value = lane.get("lane_root") if isinstance(lane, Mapping) else None
    if not isinstance(lane_root_value, str) or not lane_root_value:
        raise ReleaseError("adoption lane root binding is invalid")
    if _path_key(Path(lane_root_value)) != _path_key(owner_root):
        raise ReleaseError("adoption lane root drift")


def _pid_alive(pid: Any) -> bool:
    if not isinstance(pid, int) or pid <= 0:
        return False
    if os.name == "nt":
        # ``os.kill(pid, 0)`` is not a harmless existence probe on Windows;
        # Python routes signals through the Win32 console/process APIs and it
        # can interrupt the probing process itself.  Query a synchronization
        # handle instead and fail closed (alive) when access is denied or the
        # state cannot be proved.
        import ctypes

        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        open_process = kernel32.OpenProcess
        open_process.argtypes = [ctypes.c_ulong, ctypes.c_int, ctypes.c_ulong]
        open_process.restype = ctypes.c_void_p
        wait_for_single_object = kernel32.WaitForSingleObject
        wait_for_single_object.argtypes = [ctypes.c_void_p, ctypes.c_ulong]
        wait_for_single_object.restype = ctypes.c_ulong
        close_handle = kernel32.CloseHandle
        close_handle.argtypes = [ctypes.c_void_p]
        close_handle.restype = ctypes.c_int

        synchronize = 0x00100000
        process_query_limited_information = 0x1000
        handle = open_process(
            synchronize | process_query_limited_information,
            False,
            pid,
        )
        if not handle:
            # ERROR_INVALID_PARAMETER is the documented response for a PID
            # which no longer identifies a process.  Access denied and other
            # failures cannot prove death, so retain the lock/receipt.
            return ctypes.get_last_error() != 87
        try:
            wait_result = wait_for_single_object(handle, 0)
            if wait_result == 0:  # WAIT_OBJECT_0: process is signalled/exited.
                return False
            if wait_result == 0x102:  # WAIT_TIMEOUT: process is still running.
                return True
            return True  # WAIT_FAILED or an unknown state fails closed.
        finally:
            close_handle(handle)
    try:
        os.kill(pid, 0)
    except PermissionError:
        return True
    except (OSError, ProcessLookupError):
        return False
    return True


def _popen_tree_options() -> dict[str, Any]:
    """Create an independently terminable process tree for one lane run."""

    if os.name == "nt":
        return {"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP}
    return {"start_new_session": True}


def _start_released_agent(
    command: Sequence[str],
    release_root: Path,
    environment: Mapping[str, str],
) -> subprocess.Popen[bytes]:
    """Start only the released agent; kept narrow for failure injection."""

    process: subprocess.Popen[bytes] | None = None
    try:
        process = subprocess.Popen(
            list(command),
            cwd=release_root,
            env=dict(environment),
            **_popen_tree_options(),
        )
        if os.name == "nt":
            _attach_windows_job(process)
        return process
    except BaseException as primary:
        if process is not None:
            try:
                _terminate_process_tree(process)
            except BaseException as cleanup:
                raise ProcessTreeCleanupError(
                    f"released agent start failed ({primary}); "
                    f"process-tree cleanup is unproved ({cleanup})"
                ) from primary
        raise


def _attach_windows_job(process: subprocess.Popen[bytes]) -> None:
    """Assign the child to a kill-on-close Job before returning its handle."""

    import ctypes

    class _BasicLimit(ctypes.Structure):
        _fields_ = [
            ("PerProcessUserTimeLimit", ctypes.c_longlong),
            ("PerJobUserTimeLimit", ctypes.c_longlong),
            ("LimitFlags", ctypes.c_uint32),
            ("MinimumWorkingSetSize", ctypes.c_size_t),
            ("MaximumWorkingSetSize", ctypes.c_size_t),
            ("ActiveProcessLimit", ctypes.c_uint32),
            ("Affinity", ctypes.c_size_t),
            ("PriorityClass", ctypes.c_uint32),
            ("SchedulingClass", ctypes.c_uint32),
        ]

    class _IoCounters(ctypes.Structure):
        _fields_ = [
            ("ReadOperationCount", ctypes.c_ulonglong),
            ("WriteOperationCount", ctypes.c_ulonglong),
            ("OtherOperationCount", ctypes.c_ulonglong),
            ("ReadTransferCount", ctypes.c_ulonglong),
            ("WriteTransferCount", ctypes.c_ulonglong),
            ("OtherTransferCount", ctypes.c_ulonglong),
        ]

    class _ExtendedLimit(ctypes.Structure):
        _fields_ = [
            ("BasicLimitInformation", _BasicLimit),
            ("IoInfo", _IoCounters),
            ("ProcessMemoryLimit", ctypes.c_size_t),
            ("JobMemoryLimit", ctypes.c_size_t),
            ("PeakProcessMemoryUsed", ctypes.c_size_t),
            ("PeakJobMemoryUsed", ctypes.c_size_t),
        ]

    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    create_job = kernel32.CreateJobObjectW
    create_job.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p]
    create_job.restype = ctypes.c_void_p
    set_information = kernel32.SetInformationJobObject
    set_information.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
        ctypes.c_void_p,
        ctypes.c_uint32,
    ]
    set_information.restype = ctypes.c_int
    assign = kernel32.AssignProcessToJobObject
    assign.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    assign.restype = ctypes.c_int
    close = kernel32.CloseHandle
    close.argtypes = [ctypes.c_void_p]
    close.restype = ctypes.c_int

    job = create_job(None, None)
    if not job:
        raise ReleaseError(
            f"cannot create released-agent Job Object: {ctypes.get_last_error()}"
        )
    try:
        limits = _ExtendedLimit()
        limits.BasicLimitInformation.LimitFlags = 0x00002000  # KILL_ON_JOB_CLOSE
        if not set_information(job, 9, ctypes.byref(limits), ctypes.sizeof(limits)):
            raise ReleaseError(
                f"cannot configure released-agent Job Object: {ctypes.get_last_error()}"
            )
        process_handle = ctypes.c_void_p(int(getattr(process, "_handle")))
        if not assign(job, process_handle):
            raise ReleaseError(
                f"cannot assign released agent to Job Object: {ctypes.get_last_error()}"
            )
        setattr(process, "_owner_campaign_job", int(job))
        job = None
    finally:
        if job:
            close(job)


def _terminate_windows_job(process: subprocess.Popen[bytes], handle: int) -> None:
    import ctypes

    class _BasicAccounting(ctypes.Structure):
        _fields_ = [
            ("TotalUserTime", ctypes.c_longlong),
            ("TotalKernelTime", ctypes.c_longlong),
            ("ThisPeriodTotalUserTime", ctypes.c_longlong),
            ("ThisPeriodTotalKernelTime", ctypes.c_longlong),
            ("TotalPageFaultCount", ctypes.c_uint32),
            ("TotalProcesses", ctypes.c_uint32),
            ("ActiveProcesses", ctypes.c_uint32),
            ("TotalTerminatedProcesses", ctypes.c_uint32),
        ]

    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    terminate = kernel32.TerminateJobObject
    terminate.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    terminate.restype = ctypes.c_int
    query = kernel32.QueryInformationJobObject
    query.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
        ctypes.c_void_p,
        ctypes.c_uint32,
        ctypes.c_void_p,
    ]
    query.restype = ctypes.c_int
    close = kernel32.CloseHandle
    close.argtypes = [ctypes.c_void_p]
    close.restype = ctypes.c_int
    job = ctypes.c_void_p(handle)
    try:
        if not terminate(job, 1):
            raise ReleaseError(
                f"cannot terminate released-agent Job Object: {ctypes.get_last_error()}"
            )
        deadline = time.monotonic() + 15.0
        while True:
            accounting = _BasicAccounting()
            if not query(job, 1, ctypes.byref(accounting), ctypes.sizeof(accounting), None):
                raise ReleaseError(
                    f"cannot query released-agent Job Object: {ctypes.get_last_error()}"
                )
            if accounting.ActiveProcesses == 0:
                break
            if time.monotonic() >= deadline:
                raise ReleaseError("released-agent Job Object did not quiesce")
            time.sleep(0.05)
    finally:
        close(job)
        try:
            delattr(process, "_owner_campaign_job")
        except AttributeError:
            pass


def _terminate_process_tree(process: subprocess.Popen[bytes]) -> int:
    """Kill the released agent and all descendants, then reap the agent."""

    prior = process.poll()
    if os.name == "nt":
        job_handle = getattr(process, "_owner_campaign_job", None)
        if isinstance(job_handle, int) and job_handle:
            _terminate_windows_job(process, job_handle)
        else:
            try:
                killed = subprocess.run(
                    ["taskkill", "/PID", str(process.pid), "/T", "/F"],
                    text=True,
                    capture_output=True,
                    check=False,
                    timeout=15,
                )
            except (OSError, subprocess.TimeoutExpired) as exc:
                raise ReleaseError(
                    f"cannot terminate released agent process tree: {exc}"
                ) from exc
            if killed.returncode != 0:
                detail = (killed.stderr or killed.stdout).strip()
                raise ReleaseError(
                    "cannot prove released agent descendant termination: "
                    f"taskkill exited {killed.returncode}: {detail}"
                )
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        except OSError as exc:
            if prior is None:
                raise ReleaseError(
                    f"cannot terminate released agent process group {process.pid}: {exc}"
                ) from exc
        deadline = time.monotonic() + 15.0
        while True:
            try:
                os.killpg(process.pid, 0)
            except ProcessLookupError:
                break
            except OSError as exc:
                raise ReleaseError(
                    f"cannot prove process-group termination for {process.pid}: {exc}"
                ) from exc
            if time.monotonic() >= deadline:
                raise ReleaseError(
                    f"released agent process group {process.pid} did not quiesce"
                )
            time.sleep(0.05)
    try:
        observed = process.wait(timeout=15)
        return prior if prior is not None else observed
    except subprocess.TimeoutExpired:
        # A final direct kill is only a fallback for a root which failed to
        # observe its already-issued group/tree termination.
        try:
            process.kill()
            return process.wait(timeout=15)
        except (OSError, subprocess.TimeoutExpired) as exc:
            raise ReleaseError(
                f"released agent did not exit after process-tree termination: {exc}"
            ) from exc


def run_agent(
    install_root: Any,
    owner_root: Any,
    agent_args: Sequence[str],
    *,
    timeout: float | None = None,
) -> dict[str, Any]:
    """Run the released agent and persist active/terminal adoption state."""

    install_path = _existing_directory(install_root, "install root")
    with _install_lock(install_path):
        stable_root, pointer, release, selection, selection_error = (
            _select_pointer_release(install_path)
        )
    lane_root = _existing_directory(owner_root, "owner root")
    _reject_overlap(Path(release["release_root"]), lane_root, "release/owner")
    _reject_overlap(stable_root, lane_root, "install/owner")
    if not agent_args:
        raise ReleaseError("run requires an agent command")
    for index, item in enumerate(agent_args):
        if item == "--root" or item.startswith("--root="):
            raise ReleaseError("pass-through agent arguments must not contain --root")
    with _adoption_lock(lane_root) as held_lock:
        result = _run_agent_locked(
            pointer,
            release,
            selection,
            selection_error,
            lane_root,
            agent_args,
            timeout,
            held_lock,
        )
    if held_lock.cleanup_error is not None:
        result["cleanup_incomplete"] = held_lock.cleanup_error
        result["cleanup_error"] = held_lock.cleanup_error
    return result


def _run_agent_locked(
    pointer: Mapping[str, Any],
    release: Mapping[str, str],
    workflow_selection: str,
    selection_error: str | None,
    lane_root: Path,
    agent_args: Sequence[str],
    timeout: float | None,
    held_lock: _HeldLock,
) -> dict[str, Any]:
    """Execute one lane while its cross-process adoption lock is held."""

    lane = _lane_bindings(lane_root, agent_args)
    receipt_path = _adoption_path(lane_root)
    previous = _load_adoption(receipt_path)
    if previous is not None:
        # A terminal receipt from release A is valid history, not a reason to
        # block adoption of release B.  Bind it to the lane and reject only a
        # live child; status remains strict about current-pointer drift.
        _receipt_owner_binding(previous, lane_root)
        if previous["status"] == "active" and _pid_alive(previous["child_pid"]):
            raise ReleaseError("lane already has an active release adoption")
    command = [
        sys.executable,
        str(Path(release["release_root"]) / AGENT_RELATIVE_PATH),
        "--root",
        str(lane_root),
        *agent_args,
    ]
    started_at = _iso_now()
    active = _receipt_payload(
        pointer,
        lane_root,
        agent_args,
        lane,
        release=release,
        workflow_selection=workflow_selection,
        status="active",
        started_at=started_at,
        finished_at=None,
        terminal_status=None,
        exit_code=None,
        child_pid=None,
        child_argv=command,
    )
    _write_atomic(receipt_path, active, "active adoption receipt")

    environment = os.environ.copy()
    # Do not let a lane-local checkout injected via Python's environment shadow
    # the release package.  agent.py itself adds the release root to sys.path.
    environment.pop("PYTHONPATH", None)
    environment.pop("PYTHONHOME", None)
    environment["PYTHONNOUSERSITE"] = "1"
    process: subprocess.Popen[bytes] | None = None
    child_pid: int | None = None
    terminal_status = "failed"
    error: str | None = None
    exit_code: int | None = None
    primary_exception: BaseException | None = None
    cleanup_failure: BaseException | None = None
    publication_failure: BaseException | None = None
    lock_finalize_failure: BaseException | None = None
    tree_proved = False
    receipt_published = False
    terminal: dict[str, Any] | None = None
    try:
        # Poison the lane lock before Popen.  A manager crash from this point
        # cannot let another launch race an unobserved process tree.
        held_lock.arm_process_tree()
        process = _start_released_agent(
            command,
            Path(release["release_root"]),
            environment,
        )
        child_pid = process.pid
        held_lock.child_started(process.pid)
        active = _receipt_payload(
            pointer,
            lane_root,
            agent_args,
            lane,
            release=release,
            workflow_selection=workflow_selection,
            status="active",
            started_at=started_at,
            finished_at=None,
            terminal_status=None,
            exit_code=None,
            child_pid=process.pid,
            child_argv=command,
        )
        _write_atomic(receipt_path, active, "active adoption receipt")
        exit_code = process.wait(timeout=timeout)
        terminal_status = "completed" if exit_code == 0 else "failed"
    except subprocess.TimeoutExpired as exc:
        terminal_status = "timed_out"
        exit_code = 124
        error = f"released agent exceeded timeout of {timeout}s"
    except OSError as exc:
        terminal_status = "failed"
        exit_code = 127
        error = (
            f"released agent could not start: {exc}"
            if process is None
            else f"released agent execution failed: {exc}"
        )
    except BaseException as exc:
        primary_exception = exc
        terminal_status = "failed"
        exit_code = 130 if isinstance(exc, KeyboardInterrupt) else 1
        error = f"released agent execution was interrupted: {exc}"
    finally:
        if process is not None:
            try:
                observed_exit = _terminate_process_tree(process)
                if exit_code is None:
                    exit_code = observed_exit
                tree_proved = True
            except BaseException as exc:
                cleanup_failure = exc
                error = f"{error + '; ' if error else ''}{exc}"
                try:
                    held_lock.retain_unproved(str(exc))
                except ReleaseError as retain_exc:
                    error = f"{error}; cannot persist fail-closed lane lock: {retain_exc}"
        elif not isinstance(primary_exception, ProcessTreeCleanupError):
            # Popen never succeeded, so there is no child tree to retain.  The
            # poisoned lock is deliberately kept until the terminal receipt
            # has been published below.
            tree_proved = True
        else:
            cleanup_failure = primary_exception
            try:
                held_lock.retain_unproved(str(primary_exception))
            except ReleaseError as retain_exc:
                error = f"{error + '; ' if error else ''}{retain_exc}"
        try:
            lane_after = _lane_bindings(lane_root, agent_args)
        except ReleaseError as exc:
            lane_after = {"error": str(exc)}
        terminal = _receipt_payload(
            pointer,
            lane_root,
            agent_args,
            lane,
            release=release,
            workflow_selection=workflow_selection,
            status="terminal",
            started_at=started_at,
            finished_at=_iso_now(),
            terminal_status=terminal_status,
            exit_code=exit_code,
            child_pid=child_pid,
            child_argv=command,
            error=error,
            lane_after=lane_after,
        )
        try:
            _write_atomic(receipt_path, terminal, "terminal adoption receipt")
            receipt_published = True
        except BaseException as publication_error:
            publication_failure = publication_error
            try:
                held_lock.retain_unproved(
                    f"terminal adoption receipt publication failed: {publication_error}"
                )
            except ReleaseError as retain_exc:
                if hasattr(publication_error, "add_note"):
                    publication_error.add_note(
                        f"cannot persist fail-closed lane lock: {retain_exc}"
                    )
            if primary_exception is not None:
                if hasattr(primary_exception, "add_note"):
                    primary_exception.add_note(
                        f"terminal adoption publication also failed: {publication_error}"
                    )
            elif cleanup_failure is not None:
                if hasattr(cleanup_failure, "add_note"):
                    cleanup_failure.add_note(
                        f"terminal adoption publication also failed: {publication_error}"
                    )
        if tree_proved and receipt_published:
            try:
                held_lock.mark_tree_proved()
            except BaseException as exc:
                # The terminal receipt already preserves the child outcome.
                # Keep/quarantine the lock and report lock finalization
                # separately instead of replacing that outcome.
                lock_finalize_failure = exc
    assert terminal is not None
    if primary_exception is not None:
        if cleanup_failure is not None and cleanup_failure is not primary_exception:
            if hasattr(primary_exception, "add_note"):
                primary_exception.add_note(
                    f"process-tree cleanup also failed: {cleanup_failure}"
                )
        if lock_finalize_failure is not None and hasattr(primary_exception, "add_note"):
            primary_exception.add_note(
                f"lane-lock finalization also failed: {lock_finalize_failure}"
            )
        raise primary_exception
    result = {
        "schema": STATUS_SCHEMA,
        "status": "terminal",
        "terminal_status": terminal_status,
        "exit_code": exit_code,
        "receipt_path": str(receipt_path),
        "receipt_sha256": terminal["receipt_sha256"],
        "workflow_commit": release["commit"],
        "workflow_selection": workflow_selection,
        "pointer_commit": pointer["commit"],
        "pointer_sha256": pointer["pointer_sha256"],
        "selection_error": selection_error,
        "agent_sha256": release["agent_sha256"],
        "measurement_sha256": release["measurement_sha256"],
        "receipt_published": receipt_published,
    }
    if cleanup_failure is not None:
        result["cleanup_incomplete"] = str(cleanup_failure)
        result["cleanup_error"] = str(cleanup_failure)
        result["lane_quarantined"] = True
    if publication_failure is not None:
        result["receipt_publication_error"] = str(publication_failure)
        result["lane_quarantined"] = True
    if lock_finalize_failure is not None:
        result["cleanup_incomplete"] = str(lock_finalize_failure)
        result["cleanup_error"] = str(lock_finalize_failure)
        result["lane_quarantined"] = True
    # Do not swallow an operator interruption which occurred during cleanup or
    # publication.  The terminal receipt/result above still preserves the
    # child's primary status whenever publication was possible.
    for post_spawn_failure in (cleanup_failure, publication_failure, lock_finalize_failure):
        if isinstance(post_spawn_failure, (KeyboardInterrupt, SystemExit)):
            raise post_spawn_failure
    return result


def release_status(install_root: Any, owner_root: Any | None = None) -> dict[str, Any]:
    """Return authenticated pointer/release and optional lane status."""

    try:
        install_path = _existing_directory(install_root, "install root")
        with _install_lock(install_path) as status_lock:
            stable_root, pointer, release, selection, selection_error = (
                _select_pointer_release(install_path)
            )
    except ReleaseError as exc:
        return {
            "schema": STATUS_SCHEMA,
            "status": "drift",
            "error": str(exc),
        }
    result: dict[str, Any] = {
        "schema": STATUS_SCHEMA,
        "status": "ready",
        "install_root": str(stable_root),
        "release_root": release["release_root"],
        "workflow_commit": release["commit"],
        "workflow_selection": selection,
        "pointer_commit": pointer["commit"],
        "pointer_sha256": pointer["pointer_sha256"],
        "selection_error": selection_error,
        "agent_sha256": release["agent_sha256"],
        "measurement_sha256": release["measurement_sha256"],
        "launcher_sha256": pointer["launcher_sha256"],
    }
    if status_lock.cleanup_error is not None:
        result["cleanup_incomplete"] = status_lock.cleanup_error
        result["cleanup_error"] = status_lock.cleanup_error
    if owner_root is not None:
        try:
            lane_root = _existing_directory(owner_root, "owner root")
            lock_path = _adoption_path(lane_root, create=False).parent / ADOPTION_LOCK_FILENAME
            try:
                lock_record = _read_lock(lock_path, "adoption lock", ADOPTION_LOCK_SCHEMA)
            except ReleaseError as lock_error:
                if "does not exist" not in str(lock_error):
                    raise
            else:
                result["adoption_lock"] = lock_record
                if not lock_record["recoverable"] and not _pid_alive(lock_record["pid"]):
                    result["status"] = "drift"
                    result["error"] = (
                        "adoption lock is retained fail-closed: "
                        f"{lock_record.get('retained_reason')}"
                    )
            receipt_path = _adoption_path(lane_root, create=False)
            receipt = _load_adoption(receipt_path)
            if receipt is not None:
                _receipt_binding(receipt, pointer, lane_root)
                if receipt["status"] == "active" and not _pid_alive(receipt["child_pid"]):
                    result["status"] = "drift"
                    result["error"] = "active adoption child is not alive"
                    stale = dict(receipt)
                    stale["status"] = "stale_active"
                    result["adoption"] = stale
                    return result
                current_lane = _lane_bindings(lane_root, receipt["agent_argv"])
                expected_lane = receipt["lane"]
                if receipt["status"] == "terminal":
                    lane_after = receipt.get("lane_after")
                    if not isinstance(lane_after, Mapping) or "error" in lane_after:
                        raise ReleaseError(
                            "terminal adoption has no valid final lane binding"
                        )
                    expected_lane = lane_after
                if current_lane != expected_lane:
                    raise ReleaseError("adoption lane binding drift")
            result["adoption"] = receipt
        except ReleaseError as exc:
            result["status"] = "drift"
            result["error"] = str(exc)
    return result


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)

    install = commands.add_parser("install", help="install a stable release launcher")
    install.add_argument("--release-root", required=True, type=Path)
    install.add_argument("--install-root", required=True, type=Path)
    install.add_argument("--commit", dest="expected_commit")
    install.add_argument("--json", action="store_true")

    status = commands.add_parser("status", help="verify release and adoption state")
    status.add_argument("--install-root", type=Path)
    status.add_argument("--root", dest="owner_root", type=Path)
    status.add_argument("--json", action="store_true")

    run = commands.add_parser("run", help="run agent.py from the installed release")
    run.add_argument("--install-root", type=Path)
    run.add_argument("--root", dest="owner_root", required=True, type=Path)
    run.add_argument("--timeout", type=float)
    run.add_argument("--json", action="store_true")
    run.add_argument("agent_args", nargs=argparse.REMAINDER)
    return parser


def _default_install_root(explicit: Path | None) -> Path:
    if explicit is not None:
        return _path_from(explicit, "install root")
    current = Path(__file__).resolve().parent
    # A source checkout's tools directory is not a stable install root.  A
    # copied launcher lives directly under its stable root and may default to
    # that directory without an extra option.
    if current.name == "tools" and (current / "agent.py").is_file():
        raise ReleaseError("source launcher requires --install-root")
    return _path_from(current, "install root")


def _print_result(value: Mapping[str, Any]) -> None:
    print(json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2, default=str))


def main(argv: Sequence[str] | None = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(list(argv) if argv is not None else None)
    try:
        if args.command == "install":
            result = install_release(
                args.release_root,
                args.install_root,
                expected_commit=args.expected_commit,
            )
            _print_result(result)
            return 0
        if args.command == "status":
            result = release_status(
                _default_install_root(args.install_root),
                args.owner_root,
            )
            _print_result(result)
            return 0 if result["status"] == "ready" else 1
        # ``argparse.REMAINDER`` retains a conventional ``--`` separator.
        agent_args = list(args.agent_args)
        if agent_args and agent_args[0] == "--":
            agent_args = agent_args[1:]
        result = run_agent(
            _default_install_root(args.install_root),
            args.owner_root,
            agent_args,
            timeout=args.timeout,
        )
        _print_result(result)
        exit_code = result.get("exit_code")
        if not isinstance(exit_code, int) or isinstance(exit_code, bool):
            # A terminal run without a concrete child/infrastructure exit can
            # never be reported as CLI success.
            return 2
        return exit_code
    except ReleaseError as exc:
        print(f"owner-campaign-release: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
