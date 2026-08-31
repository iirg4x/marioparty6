"""Small, deterministic replay gate for historical owner results.

The replay is deliberately narrower than the live campaign driver: it builds
one detached checkout from a supplied commit, reconstructs one known source
pair, and lets the current ``owner_campaign`` runtime perform one snapshot and
one exact candidate run.  The detached tree is removed before the compact
receipt is published.  Nothing in this module consults the live campaign
interlock or the legacy signed-run path.

The public functions are useful to tests and to a local release job::

    prepared = prepare_replay(repo, "SetupMgType", output)
    receipt = run_replay(prepared)

An explicit inventory can replace any path, protected-function census, or
source reconstruction input in the built-in fixtures.  This keeps historical
paths out of tests while making the release fixtures auditable and fail-closed.
"""

from __future__ import annotations

import argparse
import copy
import datetime as _datetime
import difflib
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import stat
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Any, Mapping, MutableMapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))


SCHEMA = "owner_campaign_replay/v1"
AGGREGATE_SCHEMA = "owner_campaign_replay_aggregate/v1"
HANDLE_SCHEMA = "owner_campaign_replay_handle/v1"
REPORT_SCHEMA = "CRACK_REPORT/v1"
SHA_RE = re.compile(r"[0-9a-f]{64}\Z")
COMMIT_RE = re.compile(r"[0-9a-f]{40}\Z")
_REPARSE_POINT_ATTRIBUTE = int(
    getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
)
_GENERATOR_SUPPORT_MAX_FILES = 256
_GENERATOR_SUPPORT_MAX_BYTES = 16 << 20


class ReplayError(RuntimeError):
    """The replay cannot be trusted or cannot reach a terminal proof."""


def _canonical(value: Any) -> bytes:
    try:
        return json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise ReplayError(f"value is not canonical JSON: {exc}") from exc


def _sha_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _sha_file(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as exc:
        raise ReplayError(f"cannot hash {path}: {exc}") from exc
    return digest.hexdigest()


def _sha_json(value: Any) -> str:
    return _sha_bytes(_canonical(value))


def _now() -> str:
    return _datetime.datetime.now(_datetime.timezone.utc).isoformat().replace(
        "+00:00", "Z"
    )


def _valid_sha(value: Any, label: str) -> str:
    if not isinstance(value, str) or SHA_RE.fullmatch(value) is None:
        raise ReplayError(f"{label} is not a SHA-256")
    return value


def _valid_commit(value: Any, label: str = "commit") -> str:
    if not isinstance(value, str) or COMMIT_RE.fullmatch(value) is None:
        raise ReplayError(f"{label} is not a commit SHA")
    return value


def _read_json(path: Path, label: str) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ReplayError(f"{label} is unreadable: {path}: {exc}") from exc


def _write_json(path: Path, value: Mapping[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = _canonical(value) + b"\n"
    fd, raw = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    temporary = Path(raw)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def _inside(root: Path, path: Path) -> bool:
    try:
        path.relative_to(root)
        return True
    except ValueError:
        return False


def _is_reparse(info: os.stat_result) -> bool:
    return bool(
        getattr(info, "st_file_attributes", 0) & _REPARSE_POINT_ATTRIBUTE
    )


def _real_path(path: Path, label: str, *, directory: bool = False) -> Path:
    """Return a path whose complete component chain is non-indirect.

    ``Path.is_symlink`` is insufficient on Windows because junctions and other
    reparse points can redirect a copy outside the replay root.  Inspect every
    component with ``lstat`` so support inputs are validated without resolving
    them into an uncontrolled location.
    """

    path = Path(os.path.abspath(path))
    try:
        info = path.lstat()
    except OSError as exc:
        kind = "directory" if directory else "file"
        raise ReplayError(f"{label} is not a regular {kind}: {path}") from exc
    expected = stat.S_ISDIR(info.st_mode) if directory else stat.S_ISREG(info.st_mode)
    if not expected or stat.S_ISLNK(info.st_mode) or _is_reparse(info):
        kind = "directory" if directory else "file"
        raise ReplayError(f"{label} is not a regular {kind}: {path}")
    current = path.anchor and Path(path.anchor) or Path(path.parts[0])
    for part in path.relative_to(current).parts:
        current /= part
        try:
            component = current.lstat()
        except OSError as exc:
            raise ReplayError(f"{label} is unreadable: {current}") from exc
        if stat.S_ISLNK(component.st_mode) or _is_reparse(component):
            raise ReplayError(f"{label} uses indirect path component: {current}")
    return path


def _real_directory(path: Path, label: str) -> Path:
    return _real_path(path, label, directory=True)


def _real_file(path: Path, label: str) -> Path:
    return _real_path(path, label)


def _repository(raw: str | os.PathLike[str]) -> Path:
    """Validate a checkout without assuming ``.git`` is a regular file."""

    repository = Path(os.path.abspath(raw))
    if not repository.is_dir() or repository.is_symlink():
        raise ReplayError(f"repository is not a real directory: {repository}")
    marker = repository / ".git"
    if not (marker.is_dir() or marker.is_file()) or marker.is_symlink():
        raise ReplayError(f"repository has no usable Git metadata: {repository}")
    return repository


def _copy_bytes(data: bytes, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    fd, raw = tempfile.mkstemp(prefix=f".{destination.name}.", dir=destination.parent)
    temporary = Path(raw)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, destination)
    finally:
        temporary.unlink(missing_ok=True)


def _copy_file(source: Path, destination: Path) -> None:
    _real_file(source, "source input")
    _copy_bytes(source.read_bytes(), destination)


def _ensure_directory(path: Path, label: str) -> Path:
    """Create a directory without traversing a symlink/reparse component."""

    path = Path(os.path.abspath(path))
    missing: list[Path] = []
    current = path
    while True:
        try:
            info = current.lstat()
        except FileNotFoundError:
            missing.append(current)
            parent = current.parent
            if parent == current:
                raise ReplayError(f"{label} has no usable parent: {path}")
            current = parent
            continue
        except OSError as exc:
            raise ReplayError(f"{label} is unreadable: {current}") from exc
        if stat.S_ISLNK(info.st_mode) or _is_reparse(info):
            raise ReplayError(f"{label} uses indirect path component: {current}")
        if not stat.S_ISDIR(info.st_mode):
            raise ReplayError(f"{label} is not a directory: {current}")
        break
    _real_directory(current, label)
    for child in reversed(missing):
        try:
            child.mkdir()
        except FileExistsError:
            pass
        _real_directory(child, label)
    return path


def _copy_bound_file(
    source: Path,
    destination: Path,
    *,
    label: str,
    relative: str,
) -> dict[str, Any]:
    """Copy one support file and bind both sides to one immutable digest."""

    source = _real_file(source, label)
    try:
        data = source.read_bytes()
    except OSError as exc:
        raise ReplayError(f"{label} is unreadable: {source}: {exc}") from exc
    _ensure_directory(destination.parent, f"{label} destination parent")
    _copy_bytes(data, destination)
    destination = _real_file(destination, f"{label} destination")
    expected = _sha_bytes(data)
    if _sha_file(destination) != expected:
        raise ReplayError(f"{label} destination hash drift")
    # Read the source again after the copy.  This turns a concurrent source
    # edit into a deterministic replay failure rather than binding a mixed
    # support tree to the generated candidate.
    if _sha_file(source) != expected:
        raise ReplayError(f"{label} changed during copy")
    return {"path": relative, "size": len(data), "sha256": expected}


def _copy_support_tree(
    source: Path,
    destination: Path,
    *,
    repository: Path,
    label: str,
) -> list[dict[str, Any]]:
    """Copy a bounded generator support directory without following indirection."""

    source = _real_directory(source, label)
    destination = Path(os.path.abspath(destination))
    repository = Path(os.path.abspath(repository))
    if not _inside(repository, destination):
        raise ReplayError(f"{label} destination escapes replay clone")
    _ensure_directory(destination, f"{label} destination")
    pending: list[tuple[Path, Path]] = [(source, destination)]
    bindings: list[dict[str, Any]] = []
    total_bytes = 0
    while pending:
        current_source, current_destination = pending.pop()
        _real_directory(current_source, label)
        _ensure_directory(current_destination, f"{label} destination")
        try:
            with os.scandir(current_source) as scan:
                entries = sorted(scan, key=lambda item: item.name)
        except OSError as exc:
            raise ReplayError(f"{label} is unreadable: {current_source}: {exc}") from exc
        for entry in entries:
            source_entry = Path(entry.path)
            try:
                info = source_entry.lstat()
            except OSError as exc:
                raise ReplayError(f"{label} entry is unreadable: {source_entry}") from exc
            if stat.S_ISLNK(info.st_mode) or _is_reparse(info):
                raise ReplayError(f"{label} uses indirect path component: {source_entry}")
            relative_path = source_entry.relative_to(source).as_posix()
            destination_entry = destination / Path(*Path(relative_path).parts)
            if not _inside(destination, destination_entry):
                raise ReplayError(f"{label} destination escapes replay clone")
            if stat.S_ISDIR(info.st_mode):
                pending.append((source_entry, destination_entry))
                continue
            if not stat.S_ISREG(info.st_mode):
                raise ReplayError(f"{label} contains unsupported entry: {source_entry}")
            try:
                entry_size = info.st_size
            except AttributeError:
                entry_size = source_entry.stat().st_size
            if type(entry_size) is not int or entry_size < 0:
                raise ReplayError(f"{label} entry size is invalid: {source_entry}")
            total_bytes += entry_size
            if len(bindings) >= _GENERATOR_SUPPORT_MAX_FILES:
                raise ReplayError(f"{label} exceeds file limit")
            if total_bytes > _GENERATOR_SUPPORT_MAX_BYTES:
                raise ReplayError(f"{label} exceeds byte limit")
            bindings.append(
                _copy_bound_file(
                    source_entry,
                    destination_entry,
                    label=f"{label} file",
                    relative=relative_path,
                )
            )
    bindings.sort(key=lambda item: str(item["path"]))
    return bindings


def _remove_tree(path: Path) -> None:
    """Remove one replay-owned tree, including read-only clone objects."""

    def onerror(function: Any, name: str, info: Any) -> None:
        try:
            os.chmod(name, stat.S_IWRITE | stat.S_IREAD)
            function(name)
        except OSError:
            raise

    shutil.rmtree(path, onerror=onerror)


def _run(
    argv: Sequence[str],
    *,
    cwd: Path,
    timeout: float,
    env: Mapping[str, str] | None = None,
    input_bytes: bytes | None = None,
) -> subprocess.CompletedProcess[bytes]:
    """Run one bounded, non-shell subprocess and preserve its diagnostics.

    ``subprocess.run(timeout=...)`` only terminates the direct child.  Replay
    inputs include launchers and generators that may leave descendants behind,
    so use a process group/job boundary and explicitly tear down the whole
    tree on timeout.  Every command in this module comes through this helper
    (including Git), which keeps terminal classification deterministic.
    """

    if type(timeout) not in (int, float) or timeout <= 0:
        raise ReplayError("command timeout must be positive")
    process: subprocess.Popen[bytes] | None = None

    def terminate_tree() -> None:
        if process is None or process.poll() is not None:
            return
        if os.name == "nt":
            # ``taskkill /T`` handles launcher/manual-map descendants.  It is
            # itself bounded so a broken system utility cannot turn cleanup
            # into an unbounded wait.
            try:
                subprocess.run(
                    ["taskkill", "/PID", str(process.pid), "/T", "/F"],
                    capture_output=True,
                    timeout=5,
                    check=False,
                )
            except (OSError, subprocess.TimeoutExpired):
                pass
        else:
            try:
                os.killpg(os.getpgid(process.pid), signal.SIGKILL)
            except OSError:
                pass
        try:
            process.kill()
        except OSError:
            pass
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            pass

    popen_options: dict[str, Any] = {}
    if os.name == "nt":
        popen_options["creationflags"] = subprocess.CREATE_NEW_PROCESS_GROUP
    else:
        popen_options["start_new_session"] = True
    try:
        process = subprocess.Popen(
            list(argv),
            cwd=cwd,
            env=dict(env) if env is not None else None,
            stdin=subprocess.PIPE if input_bytes is not None else None,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            **popen_options,
        )
        stdout, stderr = process.communicate(input=input_bytes, timeout=timeout)
        result = subprocess.CompletedProcess(list(argv), process.returncode, stdout, stderr)
    except subprocess.TimeoutExpired as exc:
        terminate_tree()
        try:
            stdout, stderr = process.communicate(timeout=5) if process is not None else (b"", b"")
        except (OSError, subprocess.TimeoutExpired):
            stdout, stderr = b"", b""
        command = " ".join(argv[:4])
        raise ReplayError(
            f"command timed out after {timeout:.3f}s: {command}; "
            f"stdout={stdout[-1000:]!r} stderr={stderr[-1000:]!r}"
        ) from exc
    except OSError as exc:
        raise ReplayError(f"command failed to terminate: {' '.join(argv[:4])}: {exc}") from exc
    if result.returncode:
        stderr = result.stderr.decode("utf-8", "replace")[-1000:]
        stdout = result.stdout.decode("utf-8", "replace")[-1000:]
        raise ReplayError(
            f"command failed ({result.returncode}): {' '.join(argv[:4])}; "
            f"stdout={stdout!r} stderr={stderr!r}"
        )
    return result


_NATIVE_GIT_CANDIDATES = (
    Path(os.environ.get("ProgramW6432", r"C:\Program Files")) / "Git" / "cmd" / "git.exe",
    Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "Git" / "cmd" / "git.exe",
    Path(os.environ.get("ProgramW6432", r"C:\Program Files")) / "Git" / "bin" / "git.exe",
    Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "Git" / "bin" / "git.exe",
)


def _is_devkit_msys_git(path: Path) -> bool:
    normalized = str(path.absolute()).replace("/", "\\").casefold()
    return (
        "\\devkitpro\\msys2\\" in normalized
        or normalized.endswith("\\devkitpro\\msys2\\usr\\bin\\git.exe")
    )


def _git_argv(*args: str) -> list[str]:
    """Return Git argv using native Git for Windows, never devkitPro MSYS.

    The ambient PATH in the recovery environment places devkitPro's MSYS Git
    ahead of Git for Windows.  That executable has a finite process-table
    budget and previously left replay tests stuck in reconnect/fork failures.
    Resolve an existing native installation first, and reject an unsafe PATH
    fallback rather than silently selecting it.
    """

    for raw in _NATIVE_GIT_CANDIDATES:
        candidate = Path(raw)
        try:
            if candidate.is_file() and not candidate.is_symlink() and not _is_devkit_msys_git(candidate):
                return [str(candidate), *args]
        except OSError:
            continue
    found = shutil.which("git")
    if found:
        candidate = Path(found)
        try:
            if candidate.is_file() and not candidate.is_symlink() and not _is_devkit_msys_git(candidate):
                return [str(candidate), *args]
        except OSError:
            pass
    raise ReplayError(
        "native Git for Windows is unavailable; refusing devkitPro MSYS Git"
    )


def _git_show(repository: Path, commit: str, relpath: str) -> bytes:
    result = _run(
        _git_argv("show", f"{commit}:{relpath}"),
        cwd=repository,
        timeout=30,
    )
    return result.stdout


def _git_commit_exists(repository: Path, commit: str) -> None:
    commit = _valid_commit(commit)
    try:
        result = _run(
            # Pass the object name as a separate argv cell.  The ``^{commit}``
            # revision suffix is shell-sensitive on Windows and is unnecessary
            # after the strict 40-hex validation above.
            _git_argv("cat-file", "-t", commit),
            cwd=repository,
            timeout=30,
        )
    except ReplayError as exc:
        raise ReplayError(f"release commit is unavailable: {commit}: {exc}") from exc
    stdout = result.stdout.decode("utf-8", "replace")
    if stdout.strip() != "commit":
        raise ReplayError(
            f"release commit is unavailable: {commit}: "
            f"{stdout.strip()}"
        )


def _function_span(text: str, function: str) -> tuple[int, int]:
    """Return the complete definition span for a C function.

    This is intentionally a small lexical scanner rather than a C parser.  It
    skips comments and strings while locating the definition and matching
    braces, which is enough to splice a same-file historical donor without
    accidentally selecting a prototype or a call site.
    """

    if not isinstance(function, str) or not function or "\x00" in function:
        raise ReplayError("function name is invalid")

    def skip_space(index: int) -> int:
        while index < len(text) and text[index].isspace():
            index += 1
        return index

    cursor = 0
    while True:
        match = re.search(rf"(?<![A-Za-z0-9_]){re.escape(function)}\s*\(", text[cursor:])
        if match is None:
            break
        name_start = cursor + match.start()
        open_paren = cursor + match.end() - 1
        depth = 0
        index = open_paren
        quote: str | None = None
        escape = False
        line_comment = False
        block_comment = False
        while index < len(text):
            char = text[index]
            nxt = text[index + 1] if index + 1 < len(text) else ""
            if line_comment:
                if char == "\n":
                    line_comment = False
            elif block_comment:
                if char == "*" and nxt == "/":
                    block_comment = False
                    index += 1
            elif quote is not None:
                if escape:
                    escape = False
                elif char == "\\":
                    escape = True
                elif char == quote:
                    quote = None
            elif char == "/" and nxt == "/":
                line_comment = True
                index += 1
            elif char == "/" and nxt == "*":
                block_comment = True
                index += 1
            elif char in "\"'":
                quote = char
            elif char == "(":
                depth += 1
            elif char == ")":
                depth -= 1
                if depth == 0:
                    break
            index += 1
        if index >= len(text) or depth != 0:
            cursor = open_paren + 1
            continue
        body_open = skip_space(index + 1)
        if body_open >= len(text) or text[body_open] != "{":
            cursor = body_open + 1
            continue

        body_depth = 0
        index = body_open
        quote = None
        escape = False
        line_comment = False
        block_comment = False
        while index < len(text):
            char = text[index]
            nxt = text[index + 1] if index + 1 < len(text) else ""
            if line_comment:
                if char == "\n":
                    line_comment = False
            elif block_comment:
                if char == "*" and nxt == "/":
                    block_comment = False
                    index += 1
            elif quote is not None:
                if escape:
                    escape = False
                elif char == "\\":
                    escape = True
                elif char == quote:
                    quote = None
            elif char == "/" and nxt == "/":
                line_comment = True
                index += 1
            elif char == "/" and nxt == "*":
                block_comment = True
                index += 1
            elif char in "\"'":
                quote = char
            elif char == "{":
                body_depth += 1
            elif char == "}":
                body_depth -= 1
                if body_depth == 0:
                    start = text.rfind("\n", 0, name_start) + 1
                    # A declaration may put its return type on the previous
                    # line.  Include contiguous declaration lines, but never
                    # consume a preceding comment or a different statement.
                    prefix = text[start:name_start]
                    while "\n" in prefix and prefix.strip() == "":
                        start = text.rfind("\n", 0, start - 1) + 1
                        prefix = text[start:name_start]
                    return start, index + 1
            index += 1
        cursor = body_open + 1
    raise ReplayError(f"function definition not found: {function}")


def reconstruct_function(
    candidate_bytes: bytes,
    donor_bytes: bytes,
    function: str,
    *,
    expected_sha256: str | None = None,
) -> bytes:
    """Replace one same-file function in ``candidate_bytes`` with a donor."""

    try:
        candidate_text = candidate_bytes.decode("utf-8")
        donor_text = donor_bytes.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ReplayError(f"source is not UTF-8: {exc}") from exc
    candidate_start, candidate_end = _function_span(candidate_text, function)
    donor_start, donor_end = _function_span(donor_text, function)
    result = (
        candidate_text[:candidate_start]
        + donor_text[donor_start:donor_end]
        + candidate_text[candidate_end:]
    ).encode("utf-8")
    if expected_sha256 is not None and _sha_bytes(result) != _valid_sha(
        expected_sha256, "reconstructed source"
    ):
        raise ReplayError(
            "reconstructed source hash mismatch: "
            f"{_sha_bytes(result)} != {expected_sha256}"
        )
    return result


CAPTRAP_TARGET_SHA = "7a3936f588e7df0a248b8acba822e37d9673acbb29e5a548e1f13a6dee84e2cb"
MG_TARGET_SHA = "bf3e63fded9b5402eabfe6cb28a64ec1e0aae625a51353413c5cc00a52dd4690"
TOOLCHAIN_KEY = "b6764a1e5883ea1a096bfe4f8b888b93f1740f0f4046eb6149e0fe1d64cc6d90"
TOOLCHAIN_PATH = r"C:\Users\Anony\.codex\tools\mp6\toolchain.json"
MG_REPOSITORY = r"D:\Games\Emulation\GameCube-Wii\mp6-ai-board-mgcall-luna5-owner-v1"
DFORM_REPOSITORY = r"D:\Games\Emulation\GameCube-Wii\mp6-ai-dform-scalar-owner-v1"
DONOR_REPOSITORY = r"D:\Games\Emulation\GameCube-Wii\mp6-target-recovery-9dd-v1"
MG_TARGET = MG_REPOSITORY + r"\build\GP6E01\obj\board\mgcall.o"
CAPTRAP_TARGET = MG_REPOSITORY + r"\build\GP6E01\obj\board\captrap.o"
BOHEI_CANDIDATE = (
    DFORM_REPOSITORY
    + r"\build\requests\captrap-omexec-time4-frontier\candidate-bomheimove-exact-sealed.c"
)
BOBLE_CANDIDATE = (
    DFORM_REPOSITORY
    + r"\build\requests\captrap-omexec-data-order-after-bomheimove-r1\candidate.c"
)
BOBLE_GENERATOR = (
    DFORM_REPOSITORY
    + r"\build\requests\captrap-omexec-data-order-after-bomheimove-r1\generate.py"
)


_FIXTURE_DEFAULTS: dict[str, dict[str, Any]] = {
    "SetupMgType": {
        "repository": MG_REPOSITORY,
        "release_commit": "23ec92731a5e5b9d74ab35fde8a8014ab0e796f0",
        "owner": "main:board/mgcall",
        "unit": "main/board/mgcall",
        "function": "SetupMgType",
        "source_relpath": "src/board/mgcall.c",
        "base": {
            "kind": "git",
            "commit": "23ec92731a5e5b9d74ab35fde8a8014ab0e796f0",
            "path": "src/board/mgcall.c",
            "sha256": "760af915904cde830453d3ce60cf5e2b6de30b36c4152a1ef74ae2fc4483557f",
        },
        "candidate": {
            "kind": "git",
            "commit": "11023bff66d250c25ef4d9be91b4bf98c95da90d",
            "path": "src/board/mgcall.c",
            "sha256": "c8ab3d189d5e8b3d1d53f31f0958d83bad8cf4f0294a8ba38ded2156adc946e3",
        },
        # Bind the replay to the immutable target snapshot used by the
        # historical report rather than a mutable normal-build output.  The
        # two files are currently byte-identical, but the snapshot path makes
        # the provenance explicit and prevents a later rebuild from silently
        # changing the replay target.
        "target_path": MG_REPOSITORY + r"\build\current-residual\mgcall-SetupMgType-23ec927.json.target.o",
        "target_sha256": MG_TARGET_SHA,
        "candidate_object_sha256": "afb0a40d0cd3e69ccebf3c603894759bcca4355fccb07fecd2840ed70f0df4b2",
        "protected_exact_functions": [
            "MgCallBattleCoinGet", "MgCallBattleMesGet", "MgCallHisCheck",
            "MgCallVsEffCreate", "MgCallVsEffKill", "MgCallVsEffOMExec",
            "MgNameColorGet", "MgRouletteCreate", "MgRouletteFocus",
            "MgRouletteKill", "MgRouletteOMExec", "MgRouletteSlide",
            "MgRouletteSlideCheck", "SetupTeam", "mbMgCallDataClose",
            "mbMgCallInit", "mbMgCallSingleOnCheck", "mbMgRouletteFocusKill",
            "mbMgRouletteNumGet", "mbev_MgCall", "mbev_MgCallDonkey",
            "mbev_MgCallKettou", "mbev_MgCallKoopa", "mbev_MgCallSingle",
            "mbev_MgCallSingleKoopa", "mbev_MgCallTutorial",
        ],
    },
    "mbev_CapBomheiMove": {
        "repository": MG_REPOSITORY,
        "release_commit": "3704d27932397b4692d323e48d5fb6f71e869fd5",
        "owner": "main:board/captrap",
        "unit": "main/board/captrap",
        "function": "mbev_CapBomheiMove",
        "source_relpath": "src/board/captrap.c",
        "base": {
            "kind": "reconstruct",
            "candidate_path": BOHEI_CANDIDATE,
            "donor_path": DONOR_REPOSITORY + r"\src\board\captrap.c",
            "donor_function": "mbev_CapBomheiMove",
            "sha256": "5c6e5319e6c5eeaf24e12effcda78afe01e1f638553a9e098a3a5d8e3e3def77",
        },
        "candidate": {
            "kind": "file",
            "path": BOHEI_CANDIDATE,
            "sha256": "02536756180a460c9fa4bbb40befa8095c247d4d54ca282626de68df48146676",
        },
        "target_path": CAPTRAP_TARGET,
        "target_sha256": CAPTRAP_TARGET_SHA,
        "candidate_object_sha256": "e018774e30a12a5cc3988d2434e127d2ffb971a69fc32fd1751080bfe257c7f0",
        "protected_exact_functions": [
            "mbev_CapBiriQ", "mbev_CapBiriQKill", "mbev_CapBiriQTrap",
            "mbev_CapBobleKill", "mbev_CapBobleMove", "mbev_CapBobleTrap",
            "mbev_CapBomheiKill", "mbev_CapBomheiTrap",
            "mbev_CapDossunKill", "mbev_CapDossunTrap",
            "mbev_CapTumujikunKill",
        ],
    },
    "ev_CapBobleOMExec": {
        "repository": MG_REPOSITORY,
        "release_commit": "3704d27932397b4692d323e48d5fb6f71e869fd5",
        "owner": "main:board/captrap",
        "unit": "main/board/captrap",
        "function": "ev_CapBobleOMExec",
        "source_relpath": "src/board/captrap.c",
        "base": {
            "kind": "file",
            "path": BOHEI_CANDIDATE,
            "sha256": "02536756180a460c9fa4bbb40befa8095c247d4d54ca282626de68df48146676",
        },
        "candidate": {
            "kind": "generator",
            "path": BOBLE_CANDIDATE,
            "generator_path": BOBLE_GENERATOR,
            "generated_relpath": "build/requests/captrap-omexec-data-order-after-bomheimove-r1/candidate.c",
            "sha256": "891a960ca1f0f8f8eca81f9cec5f8e39c5a0a7dc63aa6ab5f1370f98f3caa679",
        },
        "target_path": CAPTRAP_TARGET,
        "target_sha256": CAPTRAP_TARGET_SHA,
        "candidate_object_sha256": "7f5140b08eb6a3928fbe46eaa433d22600e0ca3534a9a009650cacbe6beeb955",
        "protected_exact_functions": [
            "mbev_CapBiriQ", "mbev_CapBiriQKill", "mbev_CapBiriQTrap",
            "mbev_CapBobleKill", "mbev_CapBobleMove", "mbev_CapBobleTrap",
            "mbev_CapBomheiKill", "mbev_CapBomheiTrap",
            "mbev_CapDossunKill", "mbev_CapDossunTrap",
            "mbev_CapTumujikunKill", "mbev_CapBomheiMove",
        ],
    },
}


def fixture_names() -> tuple[str, ...]:
    return tuple(_FIXTURE_DEFAULTS)


def _merge(left: Mapping[str, Any], right: Mapping[str, Any]) -> dict[str, Any]:
    result = copy.deepcopy(dict(left))
    for key, value in right.items():
        if isinstance(value, Mapping) and isinstance(result.get(key), Mapping):
            result[key] = _merge(result[key], value)
        else:
            result[key] = copy.deepcopy(value)
    return result


def fixture_spec(
    name: str | Mapping[str, Any],
    *,
    inventory: Mapping[str, Any] | None = None,
    root: Path | str | None = None,
) -> dict[str, Any]:
    """Return one fixture with optional explicit inventory overrides."""

    if isinstance(name, Mapping):
        value = copy.deepcopy(dict(name))
        fixture_name = str(value.get("name") or value.get("function") or "custom")
    else:
        fixture_name = str(name)
        if fixture_name not in _FIXTURE_DEFAULTS:
            raise ReplayError(f"unknown replay fixture: {fixture_name}")
        value = copy.deepcopy(_FIXTURE_DEFAULTS[fixture_name])
    if inventory:
        chosen: Mapping[str, Any] = inventory
        fixtures = inventory.get("fixtures")
        if isinstance(fixtures, Mapping) and fixture_name in fixtures:
            chosen = fixtures[fixture_name]
        elif fixture_name in inventory and isinstance(inventory[fixture_name], Mapping):
            chosen = inventory[fixture_name]
        value = _merge(value, chosen)
    value.setdefault("name", fixture_name)
    if root is not None:
        # Re-root built-in repository-owned paths as well as the checkout
        # itself.  Without this, ``--root`` changed only Git operations while
        # target/config/source inputs continued to come from the historical
        # default checkout.  Explicit inventory paths outside the old
        # repository (for example a shared toolchain) remain explicit.
        old_repository = Path(str(value.get("repository", ""))).absolute()
        new_repository = Path(root).absolute()

        def reroot_path(raw: Any) -> Any:
            if not isinstance(raw, str):
                return raw
            path = Path(raw)
            if not path.is_absolute():
                return raw
            try:
                relative = path.absolute().relative_to(old_repository)
            except ValueError:
                return raw
            return str(new_repository / relative)

        for key in ("target_path", "objdiff_path", "configure_path"):
            if key in value:
                value[key] = reroot_path(value[key])
        for descriptor_name in ("base", "candidate"):
            descriptor = value.get(descriptor_name)
            if not isinstance(descriptor, Mapping):
                continue
            descriptor = dict(descriptor)
            if descriptor.get("kind") == "file" and "path" in descriptor:
                descriptor["path"] = reroot_path(descriptor["path"])
            if descriptor.get("kind") == "reconstruct" and "candidate_path" in descriptor:
                descriptor["candidate_path"] = reroot_path(descriptor["candidate_path"])
            value[descriptor_name] = descriptor
        # An explicit CLI/API root is authoritative.  ``setdefault`` here
        # used to leave built-in fixtures pointed at the historical default
        # checkout, making ``--root`` appear to work while still reading and
        # validating objects from another repository.
        value["repository"] = str(Path(root).absolute())
    required = ("repository", "release_commit", "owner", "unit", "function", "source_relpath")
    for key in required:
        if not isinstance(value.get(key), str) or not value[key]:
            raise ReplayError(f"fixture field is missing: {key}")
    _valid_commit(value["release_commit"], "release_commit")
    return value


def _resolve_source(repository: Path, descriptor: Mapping[str, Any], label: str) -> bytes:
    kind = descriptor.get("kind")
    expected = _valid_sha(descriptor.get("sha256"), f"{label}.sha256")
    if kind == "git":
        commit = _valid_commit(descriptor.get("commit"), f"{label}.commit")
        raw_path = descriptor.get("path")
        if not isinstance(raw_path, str) or not raw_path or Path(raw_path).is_absolute():
            raise ReplayError(f"{label}.path must be repository-relative")
        value = _git_show(repository, commit, raw_path)
    elif kind == "file":
        raw_path = descriptor.get("path")
        if not isinstance(raw_path, str):
            raise ReplayError(f"{label}.path is missing")
        value = _real_file(Path(raw_path), label).read_bytes()
    elif kind == "generator":
        generator_path = descriptor.get("generator_path")
        generated_relpath = descriptor.get("generated_relpath")
        if not isinstance(generator_path, str) or not isinstance(generated_relpath, str):
            raise ReplayError(f"{label} generator descriptor is incomplete")
        if Path(generated_relpath).is_absolute() or ".." in Path(generated_relpath).parts:
            raise ReplayError(f"{label}.generated_relpath is unsafe")
        generated = _run_source_generator(
            repository,
            _real_file(Path(generator_path), f"{label} generator"),
            Path(generated_relpath),
        )
        value = _real_file(generated, label).read_bytes()
    else:
        raise ReplayError(f"unsupported {label} kind: {kind!r}")
    actual = _sha_bytes(value)
    if actual != expected:
        raise ReplayError(f"{label} hash drift: {actual} != {expected}")
    return value


def _run_source_generator(
    repository: Path, generator_path: Path, generated_relpath: Path
) -> Path:
    """Run a historical source generator with all writes redirected to clone.

    Historical replay fixtures occasionally retain a generator rather than
    its candidate file.  Such scripts often hard-code their original checkout
    and request directory.  Execute a narrowly rewritten copy: its ``ROOT``
    points at the disposable clone, its immutable support inputs are copied
    into that clone, and approval validation is disabled because replay will
    bind the resulting bytes again through its own campaign descriptor.
    """

    try:
        script = generator_path.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as exc:
        raise ReplayError(f"source generator is unreadable: {generator_path}: {exc}") from exc
    root_match = re.search(
        r"(?m)^ROOT\s*=\s*Path\(r?(['\"])(.*?)\1\)\s*$", script
    )
    if root_match is None:
        raise ReplayError("source generator does not expose a fixed ROOT")
    historical_root = Path(root_match.group(2)).absolute()
    replacement = f"ROOT = Path({str(repository)!r})"
    script = script[: root_match.start()] + replacement + script[root_match.end() :]
    # The generator's own approval loader is bound to its original controller
    # and would reject a disposable clone.  Its output is revalidated by the
    # replay descriptor, so make this one generation step source-only.
    script = script.replace("load_approval(ROOT, draft_path)", "None", 1)
    # Prefer the generator's current helper implementation when the release
    # checkout contains an older or incomplete copy.
    script = script.replace(
        "sys.path.insert(0, str(ROOT))",
        f"sys.path.insert(0, {str(historical_root)!r})",
        1,
    )

    support_patterns = (
        ("file", r"(?m)^RESIDUAL\s*=\s*ROOT\s*/\s*(['\"])(.*?)\1\s*$"),
        ("directory", r"(?m)^TEMPLATE_DIR\s*=\s*ROOT\s*/\s*(['\"])(.*?)\1\s*$"),
    )
    support_bindings: list[dict[str, Any]] = []
    for kind, pattern in support_patterns:
        match = re.search(pattern, script)
        if match is None:
            continue
        relative = Path(match.group(2))
        if relative.is_absolute() or ".." in relative.parts:
            raise ReplayError("generator support input path is unsafe")
        source = historical_root / relative
        destination = Path(os.path.abspath(repository / relative))
        if not _inside(repository, destination):
            raise ReplayError("generator support input escapes replay clone")
        if kind == "file":
            support_bindings.append(
                _copy_bound_file(
                    source,
                    destination,
                    label="generator support input",
                    relative=relative.as_posix(),
                )
            )
        else:
            tree_bindings = _copy_support_tree(
                source,
                destination,
                repository=repository,
                label="generator template directory",
            )
            for binding in tree_bindings:
                binding["path"] = (
                    relative / Path(str(binding["path"]))
                ).as_posix()
            support_bindings.extend(tree_bindings)

    raw_script = generator_path.parent / ".replay-generator.py"
    # Keep the rewritten script outside the clone's tracked tree.  The clone
    # is the only cwd and all generated request files remain disposable.
    raw_script = repository.parent / f".{repository.name}.replay-generator.py"
    _copy_bytes(script.encode("utf-8"), raw_script)
    generated = Path(os.path.abspath(repository / generated_relpath))
    if not _inside(repository, generated):
        raise ReplayError("generated source escapes replay clone")
    # Historical generators commonly write a sealed base/candidate pair into
    # the request directory without creating that directory themselves.  It is
    # a replay-owned, already-contained destination, so establish the exact
    # parent before execution rather than failing before any compiler proof.
    _ensure_directory(generated.parent, "generated source parent")
    try:
        environment = dict(os.environ)
        # Historical generators may invoke ``git`` themselves.  Keep those
        # nested calls on the same native executable selected by the replay
        # gate; otherwise an ambient devkitPro MSYS entry can reintroduce the
        # process-table exhaustion that this resolver is designed to avoid.
        native_git = Path(_git_argv("--version")[0]).parent
        path_entries = [str(native_git)]
        for entry in environment.get("PATH", "").split(os.pathsep):
            if not entry:
                continue
            try:
                if _is_devkit_msys_git(Path(entry) / "git.exe"):
                    continue
            except OSError:
                continue
            if entry.casefold() not in {item.casefold() for item in path_entries}:
                path_entries.append(entry)
        environment["PATH"] = os.pathsep.join(path_entries)
        existing_pythonpath = environment.get("PYTHONPATH", "")
        environment["PYTHONPATH"] = str(historical_root) + (
            os.pathsep + existing_pythonpath if existing_pythonpath else ""
        )
        _run(
            [sys.executable, str(raw_script)],
            cwd=repository,
            timeout=120,
            env=environment,
        )
    finally:
        raw_script.unlink(missing_ok=True)
    # Bind every immutable support input again after generation.  A generator
    # that mutates its template/residual inputs, or a source race during the
    # copy, must fail closed rather than yielding a candidate assembled from
    # mixed identities.
    for binding in support_bindings:
        relative = Path(str(binding["path"]))
        source = _real_file(historical_root / relative, "generator support input")
        destination = _real_file(repository / relative, "generator support destination")
        expected = _valid_sha(binding.get("sha256"), "generator support hash")
        if _sha_file(source) != expected or _sha_file(destination) != expected:
            raise ReplayError(f"generator support input drift: {relative.as_posix()}")
    return generated


def _materialize_sources(
    spec: Mapping[str, Any], *, repository: Path | None = None
) -> tuple[bytes, bytes]:
    """Resolve the source pair from ``repository`` without mutating it.

    ``repository`` is normally the disposable replay clone.  Keeping the
    resolver's source repository explicit is important for historical Git
    descriptors: a replay must not accidentally read a different object from
    the authoritative checkout after the clone has been made.
    """

    source_repository = repository or _repository(str(spec["repository"]))
    base_descriptor = spec.get("base")
    candidate_descriptor = spec.get("candidate")
    if not isinstance(base_descriptor, Mapping) or not isinstance(candidate_descriptor, Mapping):
        raise ReplayError("fixture source descriptors are incomplete")

    if candidate_descriptor.get("kind") == "generator":
        # A retained historical cell can be the base for the next exact crack.
        # Its generator expects ROOT/source_relpath to contain that retained
        # frontier, not whatever older file happens to live at release HEAD.
        # Materialize the immutable base into the disposable clone before the
        # generator runs.  Reconstructed bases depend on candidate bytes and
        # therefore cannot form this pre-generation context.
        if base_descriptor.get("kind") == "reconstruct":
            raise ReplayError("generated candidate cannot use a reconstructed base")
        base = _resolve_source(source_repository, base_descriptor, "base source")
        source_relpath = spec.get("source_relpath")
        if (
            not isinstance(source_relpath, str)
            or not source_relpath
            or Path(source_relpath).is_absolute()
            or ".." in Path(source_relpath).parts
        ):
            raise ReplayError("generator base source_relpath is unsafe")
        _copy_bytes(base, source_repository / source_relpath)
        candidate = _resolve_source(
            source_repository, candidate_descriptor, "candidate source"
        )
    else:
        candidate = _resolve_source(source_repository, candidate_descriptor, "candidate source")
        if base_descriptor.get("kind") == "reconstruct":
            donor_path = base_descriptor.get("donor_path")
            donor_function = base_descriptor.get("donor_function")
            if not isinstance(donor_path, str) or not isinstance(donor_function, str):
                raise ReplayError("reconstructed base donor is incomplete")
            donor = _real_file(Path(donor_path), "donor source").read_bytes()
            base = reconstruct_function(
                candidate,
                donor,
                donor_function,
                expected_sha256=str(base_descriptor.get("sha256")),
            )
        else:
            base = _resolve_source(source_repository, base_descriptor, "base source")
    if _sha_bytes(base) != _valid_sha(base_descriptor.get("sha256"), "base source.sha256"):
        raise ReplayError("base source hash drift")
    return base, candidate


def _source_function_span(
    source: bytes, function: str, label: str
) -> dict[str, Any]:
    """Return the line/hash binding required by ``owner_campaign``.

    Candidate descriptors are consumed by the production campaign loader, not
    just by this replay module.  Consequently the descriptor must bind both
    sides of the exact function span, including the line numbers in each
    source and the bytes covered by those lines.  Deriving this from the
    materialized source pair keeps the binding correct for historical donors
    whose candidate and base line counts differ.
    """

    try:
        text = source.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ReplayError(f"{label} is not UTF-8 natural C") from exc
    start, end = _function_span(text, function)
    lines = text.splitlines(keepends=True)
    start_line = text.count("\n", 0, start) + 1
    # ``end`` points just after the closing brace.  Count the line containing
    # the brace; a trailing newline belongs to that same source line.
    end_line = text.count("\n", 0, max(start, end - 1)) + 1
    covered = "".join(lines[start_line - 1:end_line]).encode("utf-8")
    if not covered:
        raise ReplayError(f"{label} function span is empty")
    return {
        "start_line": start_line,
        "end_line": end_line,
        "sha256": _sha_bytes(covered),
    }


def _source_cell_span(
    base_source: bytes, candidate_source: bytes, function: str
) -> dict[str, Any]:
    """Bind the smallest contiguous source cell containing function and edits.

    Most replay cells edit only the named function, but a target-authenticated
    translation-unit owner can move across the function boundary (for example,
    a static data producer relocated from immediately before a callback to
    immediately after it).  The production campaign validator accepts a
    bounded source cell, not an unbounded file rewrite.  Derive that cell from
    the named function span plus every non-equal ``SequenceMatcher`` opcode in
    both line-coordinate systems.

    Prefix and suffix equality are checked here as well as by the production
    runtime.  This makes the replay handle a closed binding: every changed line
    is inside the claimed envelope, while every line outside it is identical.
    ``function_span`` remains the descriptor field name for schema
    compatibility even though the bound region may include adjacent TU owners.
    """

    base_function = _source_function_span(base_source, function, "base source")
    candidate_function = _source_function_span(
        candidate_source, function, "candidate source"
    )
    try:
        base_text = base_source.decode("utf-8")
        candidate_text = candidate_source.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ReplayError("source cell is not UTF-8 natural C") from exc
    base_lines = base_text.splitlines(keepends=True)
    candidate_lines = candidate_text.splitlines(keepends=True)

    # Half-open coordinates match SequenceMatcher and the campaign runtime.
    base_start = int(base_function["start_line"]) - 1
    base_end = int(base_function["end_line"])
    candidate_start = int(candidate_function["start_line"]) - 1
    candidate_end = int(candidate_function["end_line"])
    opcodes = difflib.SequenceMatcher(a=base_lines, b=candidate_lines).get_opcodes()
    for tag, a0, a1, b0, b1 in opcodes:
        if tag == "equal":
            continue
        base_start = min(base_start, a0)
        base_end = max(base_end, a1)
        candidate_start = min(candidate_start, b0)
        candidate_end = max(candidate_end, b1)

    if (
        base_lines[:base_start] != candidate_lines[:candidate_start]
        or base_lines[base_end:] != candidate_lines[candidate_end:]
    ):
        raise ReplayError("source cell envelope does not isolate every candidate edit")
    for tag, a0, a1, b0, b1 in opcodes:
        if tag != "equal" and (
            a0 < base_start
            or a1 > base_end
            or b0 < candidate_start
            or b1 > candidate_end
        ):
            raise ReplayError("source cell edit escapes the derived envelope")

    base_covered = "".join(base_lines[base_start:base_end]).encode("utf-8")
    candidate_covered = "".join(
        candidate_lines[candidate_start:candidate_end]
    ).encode("utf-8")
    if not base_covered or not candidate_covered:
        raise ReplayError("source cell span is empty")
    return {
        "base_start_line": base_start + 1,
        "base_end_line": base_end,
        "candidate_start_line": candidate_start + 1,
        "candidate_end_line": candidate_end,
        "base_sha256": _sha_bytes(base_covered),
        "candidate_sha256": _sha_bytes(candidate_covered),
    }


def _objdiff_paths(config: Path, unit: str) -> tuple[str, str]:
    value = _read_json(config, "objdiff config")
    units = value.get("units") if isinstance(value, Mapping) else None
    if not isinstance(units, list):
        raise ReplayError("objdiff config has no units")
    matches = [item for item in units if isinstance(item, Mapping) and item.get("name") == unit]
    if len(matches) != 1:
        raise ReplayError(f"objdiff unit {unit!r} resolved {len(matches)} times")
    target = matches[0].get("target_path")
    base = matches[0].get("base_path")
    if not isinstance(target, str) or not isinstance(base, str):
        raise ReplayError("objdiff unit lacks target_path/base_path")
    if Path(target).is_absolute() or Path(base).is_absolute():
        raise ReplayError("objdiff paths must be repository-relative")
    return target, base


def _git_stage_commit(
    repository: Path,
    worktree: Path,
    release_commit: str,
    paths: Sequence[str],
    index_path: Path,
) -> str:
    environment = dict(os.environ)
    environment["GIT_INDEX_FILE"] = str(index_path)
    if index_path.exists():
        index_path.unlink()
    _run(_git_argv("read-tree", release_commit), cwd=repository, timeout=30, env=environment)
    # Replay assets intentionally live under ignored build paths.  Force-add
    # only the sealed explicit path list; never broaden this to ``git add -f
    # --all`` without pathspecs, which could pull unrelated scratch files into
    # the synthetic campaign commit.
    _run(
        _git_argv("add", "--force", "--all", "--", *paths),
        cwd=worktree,
        timeout=60,
        env=environment,
    )
    tree = _run(_git_argv("write-tree"), cwd=repository, timeout=30, env=environment).stdout.decode().strip()
    if not re.fullmatch(r"[0-9a-f]{40}", tree):
        raise ReplayError("temporary replay tree is invalid")
    environment.update({
        "GIT_AUTHOR_NAME": "owner-campaign-replay",
        "GIT_AUTHOR_EMAIL": "owner-campaign-replay@invalid",
        "GIT_COMMITTER_NAME": "owner-campaign-replay",
        "GIT_COMMITTER_EMAIL": "owner-campaign-replay@invalid",
        "GIT_AUTHOR_DATE": "2000-01-01T00:00:00Z",
        "GIT_COMMITTER_DATE": "2000-01-01T00:00:00Z",
    })
    commit = _run(
        _git_argv("commit-tree", tree, "-p", release_commit),
        cwd=repository,
        timeout=30,
        env=environment,
        input_bytes=b"owner campaign replay assets\n",
    ).stdout.decode().strip()
    return _valid_commit(commit, "temporary campaign commit")


def _clone_repository(repository: Path, destination: Path) -> Path:
    """Create an independent object database for a replay.

    ``git worktree add`` alone shares the source checkout's object database,
    and ``commit-tree`` would therefore leave replay-only commits in the
    authoritative repository.  A no-local/no-hardlinks clone keeps all
    synthetic objects and refs disposable while retaining the pinned release
    history needed by the campaign runtime.
    """

    if destination.exists():
        raise ReplayError(f"replay clone destination already exists: {destination}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    _run(
        [
            *_git_argv(
                "clone", "-c", "core.autocrlf=false", "--no-local", "--no-hardlinks"
            ),
            str(repository), str(destination),
        ],
        cwd=repository.parent,
        timeout=120,
    )
    return _repository(destination)


def _copy_runtime(runtime_root: Path, worktree: Path) -> list[str]:
    source_tools = runtime_root / "tools"
    if not source_tools.is_dir():
        raise ReplayError(f"runtime tools directory is missing: {source_tools}")
    paths: list[str] = []
    for source in sorted(source_tools.glob("*.py")):
        destination = worktree / "tools" / source.name
        _copy_file(source, destination)
        paths.append(destination.relative_to(worktree).as_posix())
    if not paths:
        raise ReplayError("runtime tools package is empty")
    return paths


def _build_manifest(
    spec: Mapping[str, Any],
    *,
    campaign_commit: str,
    source_relpath: str,
    target_relpath: str,
    toolchain_relpath: str,
    producer_relpath: str,
    target_sha256: str,
    toolchain_sha256: str,
    producer_sha256: str,
) -> dict[str, Any]:
    function = str(spec["function"])
    unresolved = spec.get("unresolved_functions", ["__unresolved_owner__"])
    if not isinstance(unresolved, list) or not all(isinstance(item, str) and item for item in unresolved):
        raise ReplayError("unresolved function census is invalid")
    functions = list(spec.get("functions", [function]))
    if function not in functions:
        functions.insert(0, function)
    protected = list(spec.get("protected_exact_functions", []))
    # Built-in historical inventories carry the authoritative protected
    # census without repeating the full TU function list.  Include those
    # identities in the campaign scope before validating the subset relation.
    for item in protected:
        if item not in functions:
            functions.append(item)
    for item in unresolved:
        if item not in functions:
            functions.append(item)
    if not all(isinstance(item, str) and item in functions for item in protected):
        raise ReplayError("protected function census is invalid")
    limits = {
        "command_timeout_seconds": int(spec.get("command_timeout_seconds", 120)),
        "scratch_soft_bytes": int(spec.get("scratch_soft_bytes", 384 << 20)),
        "scratch_hard_bytes": int(spec.get("scratch_hard_bytes", 512 << 20)),
        "cell_temporary_bytes": int(spec.get("cell_temporary_bytes", 64 << 20)),
        "focus_evidence_bytes": int(spec.get("focus_evidence_bytes", 256 << 10)),
        "frontier_bytes": int(spec.get("frontier_bytes", 64 << 10)),
        "report_bytes": int(spec.get("report_bytes", 64 << 10)),
        "dedupe_bytes": int(spec.get("dedupe_bytes", 1 << 20)),
        "owner_state_bytes": int(spec.get("owner_state_bytes", 16 << 20)),
    }
    command = [
        sys.executable,
        "{MEASUREMENT_PRODUCER}",
        "--phase", "{PHASE}",
        "--root", "{SCRATCH_ROOT}",
        "--output", "build/owner-campaign/measurement.json",
        "--source", "{SOURCE}",
        "--toolchain", "{TOOLCHAIN}",
    ]
    body: dict[str, Any] = {
        "schema": "owner_campaign/v1",
        "campaign_id": str(spec.get("campaign_id", f"replay-{function}")),
        "owner": str(spec["owner"]),
        "unit": str(spec["unit"]),
        "source_relpath": source_relpath,
        "base_commit": campaign_commit,
        "target_object": {"path": target_relpath, "sha256": target_sha256},
        "toolchain": {"path": toolchain_relpath, "sha256": toolchain_sha256},
        "measurement_producer": {"path": producer_relpath, "sha256": producer_sha256},
        "functions": functions,
        "protected_exact_functions": protected,
        "allowed_source_paths": [source_relpath],
        "allowed_build_paths": ["build"],
        "forbidden_constructs": list(spec.get("forbidden_constructs", [r"\b(?:asm|volatile|register)\b", r"#\s*pragma"])),
        "commands": {
            "snapshot": {"argv": command, "measurement_relpath": "build/owner-campaign/measurement.json"},
            "candidate": {"argv": command, "measurement_relpath": "build/owner-campaign/measurement.json"},
            # Required by the runtime schema but never selected because the
            # unresolved census prevents owner closure in this replay.
            "final_owner": {"argv": command, "measurement_relpath": "build/owner-campaign/final.json"},
        },
        "cancellation_epoch": 0,
        "limits": limits,
    }
    return {**body, "manifest_sha256": _sha_json(body)}


def _candidate_descriptor(
    spec: Mapping[str, Any], campaign: Mapping[str, Any], frontier: Mapping[str, Any],
    path: str, sha256: str, function_span: Mapping[str, Any],
) -> dict[str, Any]:
    function = spec.get("function")
    if not isinstance(function, str) or not function:
        raise ReplayError("candidate function is missing")
    if campaign.get("function") not in (None, function) and function not in campaign.get("functions", []):
        raise ReplayError("candidate function is outside campaign scope")
    body = {
        "schema": "owner_campaign_candidate/v1",
        "campaign_id": campaign["campaign_id"],
        "function": function,
        "base_frontier_sha256": frontier["frontier_sha256"],
        "candidate_source": {"path": path, "sha256": sha256},
        "function_span": dict(function_span),
        "hypothesis_family": f"historical-replay-{function}",
        "natural_c": True,
        "created_at": _now(),
    }
    return {**body, "candidate_sha256": _sha_json(body)}


def _storage_bytes(path: Path) -> int:
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


_HANDLE_PATH_FIELDS = (
    "repository", "worktree", "raw_root", "index_path", "manifest_path",
    "candidate_path", "base_path", "source_path", "target_path",
    "toolchain_path",
)
_HANDLE_FIELDS = {
    "schema", "fixture", "owner", "unit", "function", "source_relpath",
    "release_commit", "campaign_commit", "paths", "target_input_path",
    "hashes", "function_span", "candidate_object_expected_sha256",
    "expected_report_sha256", "peak_bytes", "storage_cap_bytes", "created_at",
    "handle_sha256",
}


def _handle_relative_path(root: Path, value: Any, label: str) -> str:
    path = Path(str(value)).absolute()
    if not _inside(root, path):
        raise ReplayError(f"{label} escapes replay output")
    return path.relative_to(root).as_posix()


def _handle_path(root: Path, value: Any, label: str, *, require_file: bool = False) -> Path:
    if not isinstance(value, str) or not value or Path(value).is_absolute():
        raise ReplayError(f"{label} is not a replay-relative path")
    path = (root / value).absolute()
    if not _inside(root, path):
        raise ReplayError(f"{label} escapes replay output")
    current = root
    for part in path.relative_to(root).parts:
        current /= part
        if current.is_symlink():
            raise ReplayError(f"{label} uses symlink indirection: {current}")
    if require_file:
        _real_file(path, label)
    return path


def _replay_handle_body(prepared: Mapping[str, Any]) -> dict[str, Any]:
    output = Path(str(prepared["output"])).absolute()
    paths = {
        key: _handle_relative_path(output, prepared[key], f"handle {key}")
        for key in _HANDLE_PATH_FIELDS
    }
    expected_object = prepared.get("candidate_object_expected_sha256")
    expected_report = prepared.get("expected_report_sha256")
    if expected_object is not None:
        expected_object = _valid_sha(expected_object, "expected candidate object")
    if expected_report is not None:
        expected_report = _valid_sha(expected_report, "expected report")
    body: dict[str, Any] = {
        "schema": HANDLE_SCHEMA,
        "fixture": str(prepared["fixture"]),
        "owner": str(prepared["spec"]["owner"]),
        "unit": str(prepared["spec"]["unit"]),
        "function": str(prepared["spec"]["function"]),
        "source_relpath": str(prepared["spec"]["source_relpath"]),
        "release_commit": _valid_commit(prepared["release_commit"], "release_commit"),
        "campaign_commit": _valid_commit(prepared["campaign_commit"], "campaign_commit"),
        "paths": paths,
        "target_input_path": str(Path(str(prepared["target_input_path"])).absolute()),
        "hashes": {
            "base_source_sha256": _valid_sha(prepared["base_source_sha256"], "base source"),
            "candidate_source_sha256": _valid_sha(prepared["candidate_source_sha256"], "candidate source"),
            "target_sha256": _valid_sha(prepared["target_sha256"], "target object"),
            "toolchain_sha256": _valid_sha(prepared["toolchain_sha256"], "toolchain"),
            "producer_sha256": _valid_sha(prepared["producer_sha256"], "producer"),
        },
        "function_span": dict(prepared["function_span"]),
        "candidate_object_expected_sha256": expected_object or "",
        "expected_report_sha256": expected_report or "",
        "peak_bytes": int(prepared.get("peak_bytes", 0)),
        "storage_cap_bytes": prepared.get("storage_cap_bytes"),
        "created_at": _now(),
    }
    return body


def _write_replay_handle(prepared: MutableMapping[str, Any]) -> Path:
    output = Path(str(prepared["output"])).absolute()
    handle_path = output / "replay-handle.json"
    body = _replay_handle_body(prepared)
    value = {**body, "handle_sha256": _sha_json(body)}
    _write_json(handle_path, value)
    prepared["handle_path"] = str(handle_path)
    prepared["handle_sha256"] = value["handle_sha256"]
    return handle_path


def _validate_handle_expectations(
    value: Mapping[str, Any],
    *,
    expected_fixture: str | None = None,
    expected_campaign_commit: str | None = None,
    expected_source_sha256: str | None = None,
    expected_candidate_sha256: str | None = None,
) -> None:
    if value.get("schema") != HANDLE_SCHEMA:
        raise ReplayError("replay handle schema is invalid")
    body = dict(value)
    handle_sha = _valid_sha(body.pop("handle_sha256", None), "handle_sha256")
    if _sha_json(body) != handle_sha:
        raise ReplayError("replay handle digest is invalid")
    if expected_fixture is not None and value.get("fixture") != expected_fixture:
        raise ReplayError("replay handle fixture is stale")
    if expected_campaign_commit is not None and value.get("campaign_commit") != expected_campaign_commit:
        raise ReplayError("replay handle campaign commit is stale")
    hashes = value.get("hashes")
    if not isinstance(hashes, Mapping):
        raise ReplayError("replay handle hashes are missing")
    if expected_source_sha256 is not None and hashes.get("candidate_source_sha256") != expected_source_sha256:
        raise ReplayError("replay handle source binding is stale")
    if expected_candidate_sha256 is not None and hashes.get("candidate_source_sha256") != expected_candidate_sha256:
        raise ReplayError("replay handle candidate binding is stale")
    for key in (
        "base_source_sha256", "candidate_source_sha256", "target_sha256",
        "toolchain_sha256", "producer_sha256",
    ):
        _valid_sha(hashes.get(key), f"handle hashes.{key}")
    _valid_commit(value.get("release_commit"), "handle release_commit")
    _valid_commit(value.get("campaign_commit"), "handle campaign_commit")
    span = value.get("function_span")
    if not isinstance(span, Mapping):
        raise ReplayError("replay handle function span is missing")
    if set(span) != {
        "base_start_line", "base_end_line", "candidate_start_line",
        "candidate_end_line", "base_sha256", "candidate_sha256",
    }:
        raise ReplayError("replay handle function span is not closed")
    for key in (
        "base_start_line", "base_end_line", "candidate_start_line", "candidate_end_line",
    ):
        if type(span[key]) is not int or span[key] < 1:
            raise ReplayError(f"replay handle function span {key} is invalid")
    _valid_sha(span["base_sha256"], "handle base function span")
    _valid_sha(span["candidate_sha256"], "handle candidate function span")
    if type(value.get("peak_bytes")) is not int or value["peak_bytes"] < 0:
        raise ReplayError("replay handle peak_bytes is invalid")
    cap = value.get("storage_cap_bytes")
    if cap is not None and (type(cap) is not int or cap <= 0):
        raise ReplayError("replay handle storage cap is invalid")
    if not isinstance(value.get("created_at"), str) or not value["created_at"]:
        raise ReplayError("replay handle created_at is invalid")


def load_replay_handle(
    path: Path | str,
    *,
    expected_fixture: str | None = None,
    expected_campaign_commit: str | None = None,
    expected_source_sha256: str | None = None,
    expected_candidate_sha256: str | None = None,
) -> dict[str, Any]:
    """Reload a prepared replay after a process restart.

    The handle stores only paths relative to its output directory for the
    disposable clone/worktree.  Reloading verifies that the synthetic commit,
    worktree registration, and every source/toolchain hash still match before
    returning a runtime-compatible in-memory handle.
    """

    handle_path = _real_file(Path(path), "replay handle")
    output = handle_path.parent.absolute()
    value = _read_json(handle_path, "replay handle")
    if not isinstance(value, Mapping):
        raise ReplayError("replay handle is not an object")
    if set(value) != _HANDLE_FIELDS:
        raise ReplayError("replay handle fields are not closed")
    _validate_handle_expectations(
        value,
        expected_fixture=expected_fixture,
        expected_campaign_commit=expected_campaign_commit,
        expected_source_sha256=expected_source_sha256,
        expected_candidate_sha256=expected_candidate_sha256,
    )
    for key in ("fixture", "owner", "unit", "function", "source_relpath"):
        if not isinstance(value.get(key), str) or not value[key]:
            raise ReplayError(f"replay handle {key} is invalid")
    source_relpath = Path(value["source_relpath"])
    if source_relpath.is_absolute() or ".." in source_relpath.parts:
        raise ReplayError("replay handle source_relpath is unsafe")
    paths = value["paths"]
    if not isinstance(paths, Mapping) or set(paths) != set(_HANDLE_PATH_FIELDS):
        raise ReplayError("replay handle paths are not closed")
    resolved = {
        key: _handle_path(output, paths[key], f"handle {key}")
        for key in _HANDLE_PATH_FIELDS
    }
    repository = _repository(resolved["repository"])
    _git_commit_exists(repository, str(value["campaign_commit"]))
    if not resolved["worktree"].is_dir():
        raise ReplayError("replay handle worktree is unavailable")
    if resolved["worktree"] not in _worktree_entries(repository):
        raise ReplayError("replay handle worktree is not registered")
    head = _run(_git_argv("rev-parse", "HEAD"), cwd=resolved["worktree"], timeout=30).stdout.decode().strip()
    if head != value["campaign_commit"]:
        raise ReplayError("replay handle worktree HEAD is stale")
    target_input = _real_file(Path(str(value["target_input_path"])), "handle target input")
    hashes = value["hashes"]
    expected_object = value["candidate_object_expected_sha256"] or None
    expected_report = value["expected_report_sha256"] or None
    if expected_object is not None:
        _valid_sha(expected_object, "handle expected candidate object")
    if expected_report is not None:
        _valid_sha(expected_report, "handle expected report")
    spec = {
        "name": value["fixture"],
        "owner": value["owner"],
        "unit": value["unit"],
        "function": value["function"],
        "source_relpath": value["source_relpath"],
        "candidate_object_sha256": expected_object,
        "report_sha256": expected_report,
    }
    prepared = {
        "schema": SCHEMA,
        "fixture": value["fixture"],
        "spec": spec,
        "repository": str(repository),
        "authoritative_repository": None,
        "isolated_repository": str(repository),
        "output": str(output),
        "release_commit": value["release_commit"],
        "campaign_commit": value["campaign_commit"],
        **{key: str(resolved[key]) for key in _HANDLE_PATH_FIELDS},
        "target_input_path": str(target_input),
        "base_source_sha256": hashes["base_source_sha256"],
        "candidate_source_sha256": hashes["candidate_source_sha256"],
        "target_sha256": hashes["target_sha256"],
        "toolchain_sha256": hashes["toolchain_sha256"],
        "producer_sha256": hashes["producer_sha256"],
        "function_span": dict(value["function_span"]),
        "candidate_object_expected_sha256": expected_object,
        "expected_report_sha256": expected_report,
        "storage_cap_bytes": value["storage_cap_bytes"],
        "peak_bytes": value["peak_bytes"],
        "handle_path": str(handle_path),
        "handle_sha256": value["handle_sha256"],
        "cleaned": False,
    }
    # A handle is an admission boundary, not merely a path cache.  Recheck all
    # bound bytes before allowing a resumed runtime to execute; this catches
    # stale/replaced worktree inputs after a process restart.
    _verify_replay_inputs(prepared, phase="replay handle validation")
    actual_span = _source_cell_span(
        Path(prepared["base_path"]).read_bytes(),
        Path(prepared["candidate_path"]).read_bytes(),
        str(prepared["spec"]["function"]),
    )
    if actual_span != prepared["function_span"]:
        raise ReplayError("replay handle function span is stale")
    return prepared


def _load_replay_receipt(path: Path) -> dict[str, Any]:
    value = _read_json(path, "replay receipt")
    if not isinstance(value, Mapping) or value.get("schema") != SCHEMA:
        raise ReplayError("replay receipt schema is invalid")
    body = dict(value)
    receipt_sha = _valid_sha(body.pop("receipt_sha256", None), "receipt_sha256")
    if _sha_json(body) != receipt_sha:
        raise ReplayError("replay receipt digest is invalid")
    proof = value.get("proof")
    if not isinstance(proof, Mapping) or proof.get("exact") is not True:
        raise ReplayError("replay receipt is not exact")
    return dict(value)


def _validate_replay_receipt_binding(
    receipt: Mapping[str, Any], handle: Mapping[str, Any]
) -> None:
    """Bind an idempotent terminal receipt to the restart handle.

    A self-hashed receipt is not sufficient evidence after a restart: a stale
    or copied exact receipt must not be accepted for another fixture/campaign.
    Keep this check compact and independent of the runtime so it remains safe
    when the disposable clone has already been cleaned.
    """

    for key in ("fixture", "owner", "unit", "function", "release_commit", "campaign_commit"):
        if receipt.get(key) != handle.get(key):
            raise ReplayError(f"replay result {key} is stale")
    source = receipt.get("source")
    hashes = handle.get("hashes")
    if not isinstance(source, Mapping) or not isinstance(hashes, Mapping):
        raise ReplayError("replay result source binding is missing")
    if source.get("path") != handle.get("source_relpath"):
        raise ReplayError("replay result source path is stale")
    if source.get("base_sha256") != hashes.get("base_source_sha256"):
        raise ReplayError("replay result base source binding is stale")
    if source.get("candidate_sha256") != hashes.get("candidate_source_sha256"):
        raise ReplayError("replay result candidate source binding is stale")
    if receipt.get("target_object_sha256") != hashes.get("target_sha256"):
        raise ReplayError("replay result target binding is stale")
    expected_object = handle.get("candidate_object_expected_sha256") or None
    if expected_object is not None and receipt.get("candidate_object_sha256") != expected_object:
        raise ReplayError("replay result candidate object binding is stale")
    expected_report = handle.get("expected_report_sha256") or None
    report = receipt.get("report")
    if not isinstance(report, Mapping):
        raise ReplayError("replay result report binding is missing")
    if expected_report is not None and report.get("sha256") != expected_report:
        raise ReplayError("replay result report binding is stale")


def resume_replay(
    handle: Path | str,
    *,
    runtime: Any | None = None,
    clean: bool = True,
    expected_fixture: str | None = None,
    expected_campaign_commit: str | None = None,
    expected_source_sha256: str | None = None,
    expected_candidate_sha256: str | None = None,
) -> dict[str, Any]:
    """Resume or idempotently return one replay across process restarts."""

    handle_path = _real_file(Path(handle), "replay handle")
    result_path = handle_path.parent / "replay-result.json"
    handle_value = _read_json(handle_path, "replay handle")
    if not isinstance(handle_value, Mapping) or set(handle_value) != _HANDLE_FIELDS:
        raise ReplayError("replay handle fields are not closed")
    _validate_handle_expectations(
        handle_value,
        expected_fixture=expected_fixture,
        expected_campaign_commit=expected_campaign_commit,
        expected_source_sha256=expected_source_sha256,
        expected_candidate_sha256=expected_candidate_sha256,
    )
    if result_path.is_file():
        result = _load_replay_receipt(result_path)
        _validate_replay_receipt_binding(result, handle_value)
        if expected_fixture is not None and result.get("fixture") != expected_fixture:
            raise ReplayError("replay result fixture is stale")
        return result
    prepared = load_replay_handle(
        handle_path,
        expected_fixture=expected_fixture,
        expected_campaign_commit=expected_campaign_commit,
        expected_source_sha256=expected_source_sha256,
        expected_candidate_sha256=expected_candidate_sha256,
    )
    return run_replay(prepared, runtime=runtime, clean=clean)


def prepare_replay(
    root: Path | str | None,
    fixture: str | Mapping[str, Any],
    output: Path | str,
    *,
    inventory: Mapping[str, Any] | None = None,
    storage_cap_bytes: int | None = None,
) -> dict[str, Any]:
    """Prepare one detached replay and return a run handle.

    Preparation performs no compile.  It only validates/reconstructs source,
    verifies the release commit and target/toolchain inputs, and creates a
    temporary commit containing the current runtime and replay assets.
    """

    spec = fixture_spec(fixture, inventory=inventory, root=root)
    authoritative_repository = _repository(str(spec["repository"]))
    release_commit = _valid_commit(spec["release_commit"], "release_commit")
    _git_commit_exists(authoritative_repository, release_commit)
    output_path = Path(output).absolute()
    if output_path.exists() and any(output_path.iterdir()):
        raise ReplayError(f"replay output must be empty: {output_path}")
    output_path.mkdir(parents=True, exist_ok=True)
    # Keep every disposable component deliberately short.  The repository
    # contains deep MSL include paths and Windows worktree checkout still hits
    # legacy path ceilings even when long-path support is enabled.
    raw_root = output_path / ".r"
    raw_root.mkdir()
    worktree = raw_root / "w"
    index_path = raw_root / "i"
    isolated_repository: Path | None = None
    worktree_registered = False
    try:
        # All replay-only Git state lives below this output directory.  The
        # authoritative checkout is read-only for the complete preparation/run.
        clone_path = raw_root / "g"
        isolated_repository = _clone_repository(authoritative_repository, clone_path)
        _git_commit_exists(isolated_repository, release_commit)
        # Source generators are part of the historical fixture and may inspect
        # ``HEAD`` to authenticate the source context they reconstruct.  A
        # normal clone checks out the authoritative repository's *current*
        # branch, which can be newer than the fixture's pinned release.  Detach
        # the disposable clone at the release before running any generator;
        # this changes only replay-owned state and makes its view agree with
        # the worktree/campaign parent used below.
        _run(
            _git_argv("checkout", "--detach", release_commit),
            cwd=isolated_repository,
            timeout=60,
        )

        base_bytes, candidate_bytes = _materialize_sources(
            spec, repository=isolated_repository
        )
        base_sha = _sha_bytes(base_bytes)
        candidate_sha = _sha_bytes(candidate_bytes)
        function = str(spec["function"])
        function_span = _source_cell_span(base_bytes, candidate_bytes, function)
        target_input = _real_file(Path(spec["target_path"]), "target object")
        target_sha = _valid_sha(
            spec.get("target_sha256", CAPTRAP_TARGET_SHA), "target object.sha256"
        )
        actual_target = _sha_file(target_input)
        if actual_target != target_sha:
            raise ReplayError(f"target object hash drift: {actual_target} != {target_sha}")
        toolchain_input = _real_file(
            Path(spec.get("toolchain_path", TOOLCHAIN_PATH)),
            "toolchain descriptor",
        )
        toolchain_sha = _sha_file(toolchain_input)
        expected_toolchain_sha = spec.get("toolchain_sha256")
        if expected_toolchain_sha is not None and toolchain_sha != _valid_sha(
            expected_toolchain_sha, "toolchain descriptor.sha256"
        ):
            raise ReplayError("toolchain descriptor hash drift")

        # Mark before invoking Git: a failed ``worktree add`` can still leave
        # a registration behind, and the rollback must attempt removal in
        # that partial-registration case too.
        worktree_registered = True
        _run(
            _git_argv("worktree", "add", "--detach", str(worktree), release_commit),
            cwd=isolated_repository,
            timeout=60,
        )
        source_relpath = str(spec["source_relpath"]).replace("\\", "/")
        if Path(source_relpath).is_absolute() or ".." in Path(source_relpath).parts:
            raise ReplayError("source_relpath is unsafe")
        source_path = worktree / source_relpath
        _copy_bytes(base_bytes, source_path)

        config_input = _real_file(
            Path(spec.get("objdiff_path", authoritative_repository / "objdiff.json")),
            "objdiff config",
        )
        _copy_file(config_input, worktree / "objdiff.json")
        config_target, _config_base = _objdiff_paths(
            worktree / "objdiff.json", str(spec["unit"])
        )
        target_destination = worktree / config_target
        _copy_file(target_input, target_destination)
        # A release commit usually contains configure.py.  If a test fixture
        # does not, an explicit inventory may supply it without modifying the
        # authoritative checkout.
        if not (worktree / "configure.py").is_file() and spec.get("configure_path"):
            _copy_file(
                _real_file(Path(spec["configure_path"]), "configure script"),
                worktree / "configure.py",
            )

        toolchain_relpath = "build/owner-replay/toolchain.json"
        _copy_file(toolchain_input, worktree / toolchain_relpath)
        runtime_root = Path(__file__).resolve().parents[1]
        runtime_files = _copy_runtime(runtime_root, worktree)
        producer_relpath = "tools/owner_campaign_measure.py"
        if producer_relpath not in runtime_files:
            raise ReplayError("current measurement adapter is not staged")
        candidate_relpath = "build/owner-replay/input/candidate.c"
        base_relpath = "build/owner-replay/input/base.c"
        _copy_bytes(candidate_bytes, worktree / candidate_relpath)
        _copy_bytes(base_bytes, worktree / base_relpath)
        staged_paths = [
            source_relpath,
            "objdiff.json",
            config_target,
            *runtime_files,
            toolchain_relpath,
            candidate_relpath,
            base_relpath,
        ]
        if (worktree / "configure.py").is_file():
            staged_paths.append("configure.py")
        campaign_commit = _git_stage_commit(
            isolated_repository,
            worktree,
            release_commit,
            sorted(set(staged_paths)),
            index_path,
        )
        # The replay root must be a clean checkout of the synthetic campaign
        # commit.  Runtime scratch worktrees are then based on exactly the
        # same source/tooling tree and the manifest's base_commit is a real
        # commit in the disposable object database.
        _run(
            _git_argv("reset", "--hard", campaign_commit),
            cwd=worktree,
            timeout=60,
        )
        producer_sha = _sha_file(worktree / producer_relpath)
        manifest = _build_manifest(
            spec,
            campaign_commit=campaign_commit,
            source_relpath=source_relpath,
            target_relpath=config_target,
            toolchain_relpath=toolchain_relpath,
            producer_relpath=producer_relpath,
            target_sha256=target_sha,
            toolchain_sha256=toolchain_sha,
            producer_sha256=producer_sha,
        )
        manifest_path = worktree / "build/owner-replay/campaign.json"
        _write_json(manifest_path, manifest)
        prepared: dict[str, Any] = {
            "schema": SCHEMA,
            "fixture": str(spec.get("name", spec["function"])),
            "spec": spec,
            # Runtime Git operations must use the disposable clone.  The
            # authoritative checkout is retained only as provenance.
            "repository": str(isolated_repository),
            "authoritative_repository": str(authoritative_repository),
            "isolated_repository": str(isolated_repository),
            "release_commit": release_commit,
            "campaign_commit": campaign_commit,
            "worktree": str(worktree),
            "raw_root": str(raw_root),
            "output": str(output_path),
            "index_path": str(index_path),
            "manifest_path": str(manifest_path),
            "candidate_path": str(worktree / candidate_relpath),
            "base_path": str(worktree / base_relpath),
            "source_path": str(source_path),
            "target_path": str(target_destination),
            "target_input_path": str(target_input),
            "toolchain_path": str(worktree / toolchain_relpath),
            "base_source_sha256": base_sha,
            "candidate_source_sha256": candidate_sha,
            "target_sha256": target_sha,
            "toolchain_sha256": toolchain_sha,
            "producer_sha256": producer_sha,
            "function_span": function_span,
            "candidate_object_expected_sha256": spec.get("candidate_object_sha256"),
            "expected_report_sha256": spec.get("report_sha256"),
            "peak_bytes": _storage_bytes(raw_root),
            "storage_cap_bytes": (
                storage_cap_bytes
                if storage_cap_bytes is not None
                else spec.get("replay_storage_cap_bytes")
            ),
            "cleaned": False,
        }
        storage_cap = prepared["storage_cap_bytes"]
        if storage_cap is not None:
            if type(storage_cap) is not int or storage_cap <= 0:
                raise ReplayError("replay storage cap must be a positive integer")
            if prepared["peak_bytes"] > storage_cap:
                raise ReplayError(
                    f"replay storage cap exceeded during preparation: "
                    f"{prepared['peak_bytes']} > {storage_cap}"
                )
        _write_replay_handle(prepared)
        return prepared
    except BaseException as primary:
        # The worktree is registered only for this invocation.  Preserve a
        # cleanup failure instead of silently presenting the primary error as
        # the complete terminal state.
        cleanup_errors: list[str] = []
        if isolated_repository is not None and worktree_registered:
            try:
                _run(
                    _git_argv("worktree", "remove", "--force", str(worktree)),
                    cwd=isolated_repository,
                    timeout=60,
                )
            except Exception as exc:
                cleanup_errors.append(f"worktree cleanup: {exc}")
        try:
            if not cleanup_errors:
                _remove_tree(raw_root)
            elif raw_root.exists():
                # Keep the raw tree available for deterministic diagnosis when
                # the registered worktree could not be removed.
                cleanup_errors.append("raw replay cleanup skipped because worktree remains")
        except OSError as exc:
            cleanup_errors.append(f"raw replay cleanup: {exc}")
        if cleanup_errors:
            raise ReplayError(
                f"{primary}; preparation cleanup failed: {'; '.join(cleanup_errors)}"
            ) from primary
        raise


def _report_from_result(result: Mapping[str, Any]) -> tuple[Path, dict[str, Any]]:
    exact = result.get("exact")
    if not isinstance(exact, Mapping) or not isinstance(exact.get("report_path"), str):
        raise ReplayError("exact result did not publish a CRACK_REPORT")
    report_path = _real_file(Path(exact["report_path"]), "CRACK_REPORT")
    report = _read_json(report_path, "CRACK_REPORT")
    if not isinstance(report, Mapping) or report.get("schema") != REPORT_SCHEMA:
        raise ReplayError("published report schema is invalid")
    body = dict(report)
    report_sha = _valid_sha(body.pop("report_sha256", None), "report_sha256")
    if _sha_json(body) != report_sha or report_sha != exact.get("report_sha256"):
        raise ReplayError("published report digest is invalid")
    if report.get("status") != "exact" or report.get("completed") is not True:
        raise ReplayError("published report is not a completed exact report")
    return report_path, dict(report)


def _assert_metrics(
    result: Mapping[str, Any], report: Mapping[str, Any], prepared: Mapping[str, Any]
) -> dict[str, Any]:
    metrics = result.get("metrics")
    if not isinstance(metrics, Mapping):
        raise ReplayError("runtime result has no metrics")
    for channel in ("strict", "data"):
        value = metrics.get(channel)
        if not isinstance(value, Mapping) or value.get("differences") != 0:
            raise ReplayError(f"{channel} proof is not exact")
        if value.get("target_bytes") != value.get("candidate_bytes"):
            raise ReplayError(f"{channel} byte sizes differ")
    if metrics.get("physical_differences") != 0:
        raise ReplayError("physical relocation proof is not exact")
    if metrics.get("physical_target_count") != metrics.get("physical_candidate_count"):
        raise ReplayError("physical relocation counts differ")
    if metrics.get("protected_losses") != 0 or metrics.get("source_link_exact") is not True:
        raise ReplayError("protected/source-link proof is incomplete")
    report_result = report.get("result")
    if not isinstance(report_result, Mapping):
        raise ReplayError("CRACK_REPORT result is missing")
    if report.get("owner") != prepared["spec"]["owner"] or report.get("function") != prepared["spec"]["function"]:
        raise ReplayError("CRACK_REPORT identity mismatch")
    if report.get("source_sha256") != prepared["candidate_source_sha256"]:
        raise ReplayError("CRACK_REPORT source hash mismatch")
    if report.get("target_object_sha256") != prepared["target_sha256"]:
        raise ReplayError("CRACK_REPORT target hash mismatch")
    candidate_object = report.get("candidate_object_sha256")
    _valid_sha(candidate_object, "CRACK_REPORT candidate_object_sha256")
    expected_object = prepared.get("candidate_object_expected_sha256")
    if expected_object is not None and candidate_object != _valid_sha(expected_object, "expected candidate object"):
        raise ReplayError(f"candidate object hash mismatch: {candidate_object} != {expected_object}")
    return {
        "strict": dict(metrics["strict"]),
        "data": dict(metrics["data"]),
        "physical_target_count": metrics["physical_target_count"],
        "physical_candidate_count": metrics["physical_candidate_count"],
        "physical_differences": metrics["physical_differences"],
        "protected_total": metrics["protected_total"],
        "protected_losses": metrics["protected_losses"],
        "source_link_exact": metrics["source_link_exact"],
        "candidate_object_sha256": candidate_object,
    }


def _update_peak(prepared: MutableMapping[str, Any]) -> int:
    raw = Path(str(prepared["raw_root"]))
    current = _storage_bytes(raw)
    prepared["peak_bytes"] = max(int(prepared.get("peak_bytes", 0)), current)
    cap = prepared.get("storage_cap_bytes")
    if cap is not None:
        if type(cap) is not int or cap <= 0:
            raise ReplayError("replay storage cap must be a positive integer")
        if prepared["peak_bytes"] > cap:
            raise ReplayError(
                f"replay storage cap exceeded: {prepared['peak_bytes']} > {cap}"
            )
    return current


def _verify_replay_inputs(
    prepared: Mapping[str, Any], *, phase: str, allow_candidate_cleanup: bool = False
) -> None:
    """Rehash every replay input at a compiler boundary.

    The candidate file is intentionally removed by the campaign runtime after
    a successful cell.  In that case its already-authenticated descriptor is
    the post-run witness; if it remains, it must still carry the exact hash.
    All other files must remain present and byte-identical.
    """

    checks = (
        ("base source", "base_path", prepared["base_source_sha256"]),
        ("target object", "target_path", prepared["target_sha256"]),
        ("target input", "target_input_path", prepared["target_sha256"]),
        ("toolchain descriptor", "toolchain_path", prepared["toolchain_sha256"]),
    )
    for label, key, expected in checks:
        path = Path(str(prepared[key]))
        try:
            actual = _sha_file(_real_file(path, label))
        except ReplayError as exc:
            raise ReplayError(f"{phase}: {exc}") from exc
        if actual != _valid_sha(expected, f"{label} expected hash"):
            raise ReplayError(f"{phase}: {label} hash drift: {actual} != {expected}")

    # A monotonic improved/exact campaign result intentionally retains the
    # candidate in the live owner source.  Before execution only the base is
    # valid; after execution the source must be exactly either the untouched
    # base (no gain) or the authenticated candidate (retained gain).  Treating
    # candidate retention as source drift made every real exact replay fail
    # after it had already produced the correct object and report.
    campaign_source = Path(str(prepared["source_path"]))
    try:
        actual_campaign_source = _sha_file(_real_file(campaign_source, "campaign source"))
    except ReplayError as exc:
        raise ReplayError(f"{phase}: {exc}") from exc
    allowed_campaign_sources = {
        _valid_sha(prepared["base_source_sha256"], "campaign source base hash")
    }
    if allow_candidate_cleanup:
        allowed_campaign_sources.add(
            _valid_sha(
                prepared["candidate_source_sha256"],
                "campaign source candidate hash",
            )
        )
    if actual_campaign_source not in allowed_campaign_sources:
        raise ReplayError(
            f"{phase}: campaign source hash drift: {actual_campaign_source} not in "
            f"{sorted(allowed_campaign_sources)}"
        )

    candidate = Path(str(prepared["candidate_path"]))
    if candidate.exists():
        actual = _sha_file(_real_file(candidate, "candidate source"))
        expected = _valid_sha(
            prepared["candidate_source_sha256"], "candidate source expected hash"
        )
        if actual != expected:
            raise ReplayError(f"{phase}: candidate source hash drift: {actual} != {expected}")
    elif not allow_candidate_cleanup:
        raise ReplayError(f"{phase}: candidate source is missing: {candidate}")


def _worktree_entries(repository: Path) -> list[Path]:
    result = _run(
        _git_argv("worktree", "list", "--porcelain"),
        cwd=repository,
        timeout=30,
    )
    entries: list[Path] = []
    for line in result.stdout.decode("utf-8", "replace").splitlines():
        if line.startswith("worktree "):
            entries.append(Path(line[9:]).absolute())
    return entries


def cleanup_replay(prepared: MutableMapping[str, Any]) -> dict[str, Any]:
    """Remove all registered replay worktrees and raw inputs."""

    if prepared.get("cleaned"):
        return dict(prepared.get("cleanup", {}))
    repository = Path(str(prepared["repository"])).absolute()
    worktree = Path(str(prepared["worktree"])).absolute()
    raw_root = Path(str(prepared["raw_root"])).absolute()
    errors: list[str] = []
    try:
        entries = _worktree_entries(repository)
        selected = [entry for entry in entries if entry == worktree or _inside(worktree, entry)]
        for entry in sorted(selected, key=lambda item: len(item.parts), reverse=True):
            try:
                _run(_git_argv("worktree", "remove", "--force", str(entry)), cwd=repository, timeout=60)
            except Exception as exc:
                errors.append(f"worktree {entry}: {exc}")
        if worktree.exists():
            errors.append(f"worktree remains after registered removal: {worktree}")
        if not errors:
            try:
                _run(_git_argv("worktree", "prune"), cwd=repository, timeout=30)
            except Exception as exc:
                errors.append(f"worktree prune: {exc}")
    except Exception as exc:
        errors.append(str(exc))
    if not errors:
        try:
            _remove_tree(raw_root)
        except OSError as exc:
            errors.append(f"raw replay cleanup: {exc}")
    if errors:
        raise ReplayError("replay cleanup failed: " + "; ".join(errors))
    prepared["cleaned"] = True
    prepared["cleanup"] = {
        "status": "complete",
        "worktree_removed": True,
        "raw_removed": True,
    }
    return dict(prepared["cleanup"])


def _run_replay_once(
    prepared: MutableMapping[str, Any],
    *,
    runtime: Any | None = None,
    clean: bool = True,
    max_seconds: float | None = None,
) -> dict[str, Any]:
    """Run one snapshot plus one candidate through the production runtime."""

    started = time.monotonic()
    worktree = Path(str(prepared["worktree"]))
    output = Path(str(prepared["output"]))
    if prepared.get("cleaned") or not worktree.is_dir():
        raise ReplayError("replay worktree is unavailable")
    if max_seconds is not None and (
        type(max_seconds) not in (int, float) or max_seconds <= 0
    ):
        raise ReplayError("replay timing cap must be positive")
    _verify_replay_inputs(prepared, phase="before replay")
    if runtime is None:
        try:
            from tools import owner_campaign as runtime  # type: ignore[no-redef]
        except ImportError as exc:
            raise ReplayError(f"owner_campaign runtime is unavailable: {exc}") from exc
    manifest_path = Path(str(prepared["manifest_path"]))
    campaign = runtime.load_campaign(worktree, manifest_path)
    function = str(prepared["spec"]["function"])
    frontier = runtime.snapshot_frontier(worktree, campaign, function)
    _update_peak(prepared)
    candidate_path = Path(str(prepared["candidate_path"]))
    descriptor = _candidate_descriptor(
        prepared["spec"], campaign, frontier,
        candidate_path.relative_to(worktree).as_posix(),
        str(prepared["candidate_source_sha256"]),
        prepared["function_span"],
    )
    descriptor_path = worktree / "build/owner-replay/candidate.json"
    _write_json(descriptor_path, descriptor)
    result = runtime.run_candidate(worktree, campaign, descriptor_path)
    _update_peak(prepared)
    _verify_replay_inputs(
        prepared, phase="after candidate build", allow_candidate_cleanup=True
    )
    elapsed = time.monotonic() - started
    if max_seconds is not None:
        if type(max_seconds) not in (int, float) or max_seconds <= 0:
            raise ReplayError("replay timing cap must be positive")
        if elapsed > float(max_seconds):
            raise ReplayError(
                f"replay timing cap exceeded: {elapsed:.6f}s > {float(max_seconds):.6f}s"
            )
    if not isinstance(result, Mapping) or result.get("status") != "exact":
        raise ReplayError(f"historical replay did not reach exact: {result.get('status') if isinstance(result, Mapping) else result!r}")
    report_path, report = _report_from_result(result)
    metrics = _assert_metrics(result, report, prepared)
    expected_report = prepared.get("expected_report_sha256")
    if expected_report is not None and report["report_sha256"] != _valid_sha(expected_report, "expected report"):
        raise ReplayError("CRACK_REPORT hash differs from the fixture expectation")
    report_copy = output / f"CRACK_REPORT-{function}.json"
    _copy_file(report_path, report_copy)
    finished = time.monotonic()
    wall_seconds = finished - started
    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "fixture": prepared["fixture"],
        "owner": prepared["spec"]["owner"],
        "unit": prepared["spec"]["unit"],
        "function": function,
        "release_commit": prepared["release_commit"],
        "campaign_commit": prepared["campaign_commit"],
        "source": {
            "path": prepared["spec"]["source_relpath"],
            "base_sha256": prepared["base_source_sha256"],
            "candidate_sha256": prepared["candidate_source_sha256"],
        },
        "target_object_sha256": prepared["target_sha256"],
        "candidate_object_sha256": metrics["candidate_object_sha256"],
        "report": {
            "path": report_copy.name,
            "sha256": _sha_file(report_copy),
        },
        "metrics": metrics,
        "timing": {
            "wall_seconds": wall_seconds,
            "active_seconds": wall_seconds,
            "active_definition": "single-process replay interval",
            "max_seconds": max_seconds,
            "within_cap": max_seconds is None or wall_seconds <= float(max_seconds),
        },
        "storage": {
            "peak_bytes": int(prepared.get("peak_bytes", 0)),
            "final_bytes": 0,
            "cap_bytes": prepared.get("storage_cap_bytes"),
            "within_cap": prepared.get("storage_cap_bytes") is None
            or int(prepared.get("peak_bytes", 0)) <= int(prepared["storage_cap_bytes"]),
        },
        "proof": {
            "report_schema": REPORT_SCHEMA,
            "completed": True,
            "exact": True,
        },
        "cleanup": {"status": "pending"},
    }
    if clean:
        cleanup_replay(prepared)
        receipt["cleanup"] = dict(prepared["cleanup"])
        receipt["storage"]["final_bytes"] = _storage_bytes(output)
    else:
        receipt["cleanup"] = {"status": "retained_for_inspection"}
        receipt["storage"]["final_bytes"] = _storage_bytes(output)
    body = dict(receipt)
    receipt["receipt_sha256"] = _sha_json(body)
    _write_json(output / "replay-result.json", receipt)
    return receipt


def run_replay(
    prepared: MutableMapping[str, Any],
    *,
    runtime: Any | None = None,
    clean: bool = True,
    max_seconds: float | None = None,
    storage_cap_bytes: int | None = None,
) -> dict[str, Any]:
    """Run one replay and clean its detached inputs on every terminal path."""

    if storage_cap_bytes is not None:
        if type(storage_cap_bytes) is not int or storage_cap_bytes <= 0:
            raise ReplayError("replay storage cap must be a positive integer")
        existing = prepared.get("storage_cap_bytes")
        if existing is not None and int(existing) != storage_cap_bytes:
            raise ReplayError("replay storage cap conflicts with prepared handle")
        prepared["storage_cap_bytes"] = storage_cap_bytes
    try:
        return _run_replay_once(
            prepared,
            runtime=runtime,
            clean=clean,
            max_seconds=max_seconds,
        )
    except BaseException as primary:
        if clean and not prepared.get("cleaned"):
            try:
                cleanup_replay(prepared)
            except Exception as cleanup_error:
                # Cleanup is part of the replay admission boundary.  A
                # successful proof with an unremoved worktree/raw clone is not
                # an admissible replay, and the cleanup failure must remain
                # visible alongside the primary proof error.
                raise ReplayError(
                    f"replay failed: {primary}; cleanup failed: {cleanup_error}"
                ) from primary
        raise


def replay(
    root: Path | str | None,
    fixture: str | Mapping[str, Any],
    output: Path | str,
    *,
    inventory: Mapping[str, Any] | None = None,
    runtime: Any | None = None,
) -> dict[str, Any]:
    prepared = prepare_replay(root, fixture, output, inventory=inventory)
    return run_replay(prepared, runtime=runtime)


def _runtime_for(
    runtime_factory: Any, fixture: str | Mapping[str, Any], index: int
) -> Any | None:
    if runtime_factory is None:
        return None
    if isinstance(runtime_factory, Mapping):
        name = str(fixture.get("name") if isinstance(fixture, Mapping) else fixture)
        return runtime_factory.get(name)
    if callable(runtime_factory):
        # Batch callers receive a fresh runtime per isolated replay.  The
        # index is optional for callers that want deterministic assignment.
        try:
            return runtime_factory(fixture, index)
        except TypeError as exc:
            # A one-argument factory is supported without masking errors from
            # a factory that accepted both arguments and failed internally.
            try:
                import inspect

                signature = inspect.signature(runtime_factory)
                if len(signature.parameters) != 1:
                    raise exc
            except (TypeError, ValueError):
                raise exc
            return runtime_factory(fixture)
    raise ReplayError("runtime_factory must be callable or a mapping")


def _aggregate_receipt(
    *,
    output: Path,
    mode: str,
    fixtures: Sequence[str],
    results: Sequence[Mapping[str, Any]],
    started: float,
    errors: Sequence[Mapping[str, Any]] = (),
    storage_cap_bytes: int | None = None,
    max_wall_seconds: float | None = None,
) -> dict[str, Any]:
    finished = time.monotonic()
    active = [
        float(item.get("timing", {}).get("active_seconds", 0.0))
        for item in results
    ]
    peaks = [int(item.get("storage", {}).get("peak_bytes", 0)) for item in results]
    # Sequential children are cleaned before the next child starts, so their
    # peak footprints never coexist.  Concurrent children do coexist and must
    # be charged as a sum.  Treating sequential peaks as cumulative falsely
    # rejected three individually compliant replays as an over-cap lane.
    max_child_peak = max(peaks, default=0)
    aggregate_peak = sum(peaks) if mode == "concurrent" else max_child_peak
    body: dict[str, Any] = {
        "schema": AGGREGATE_SCHEMA,
        "status": "exact" if not errors and len(results) == len(fixtures) else "failed",
        "mode": mode,
        "fixtures": list(fixtures),
        "results": [
            {
                "fixture": item.get("fixture"),
                "function": item.get("function"),
                "status": item.get("proof", {}).get("exact") and "exact" or "failed",
                "receipt_sha256": item.get("receipt_sha256"),
                "report_sha256": item.get("report", {}).get("sha256"),
                "timing": item.get("timing", {}),
                "storage": item.get("storage", {}),
            }
            for item in results
        ],
        "errors": [dict(item) for item in errors],
        "aggregate": {
            "requested": len(fixtures),
            "completed": len(results),
            "exact": sum(1 for item in results if item.get("proof", {}).get("exact") is True),
            "all_exact": len(results) == len(fixtures)
            and all(item.get("proof", {}).get("exact") is True for item in results),
        },
        "timing": {
            "wall_seconds": finished - started,
            "sum_active_seconds": sum(active),
            "max_active_seconds": max(active, default=0.0),
            "active_definition": "sum of child replay intervals",
            "max_wall_seconds": max_wall_seconds,
            "within_cap": max_wall_seconds is None
            or (finished - started) <= float(max_wall_seconds),
        },
        "storage": {
            "peak_bytes": aggregate_peak,
            "max_child_peak_bytes": max_child_peak,
            "final_bytes_before_aggregate_receipt": _storage_bytes(output),
            "cap_bytes": storage_cap_bytes,
            "within_cap": storage_cap_bytes is None
            or max_child_peak <= storage_cap_bytes,
        },
    }
    receipt = {**body, "aggregate_sha256": _sha_json(body)}
    _write_json(output / "replay-aggregate.json", receipt)
    return receipt


def _check_batch_output(output: Path | str) -> Path:
    path = Path(output).absolute()
    if path.exists() and any(path.iterdir()):
        raise ReplayError(f"replay batch output must be empty: {path}")
    path.mkdir(parents=True, exist_ok=True)
    return path


def run_replay_batch(
    root: Path | str | None,
    fixtures: Sequence[str | Mapping[str, Any]],
    output: Path | str,
    *,
    inventory: Mapping[str, Any] | None = None,
    runtime_factory: Any = None,
    concurrent: bool = False,
    clean: bool = True,
    storage_cap_bytes: int | None = None,
    max_wall_seconds: float | None = None,
) -> dict[str, Any]:
    """Run independent historical replays and publish one compact receipt.

    Every child receives a separate output directory and disposable clone.
    ``concurrent=True`` therefore exercises isolation rather than sharing a
    compile/worktree lock.  A failed child raises after its own cleanup and
    never produces an all-exact aggregate receipt.
    """

    if not isinstance(fixtures, Sequence) or isinstance(fixtures, (str, bytes)):
        raise ReplayError("fixtures must be a non-empty sequence")
    if not fixtures:
        raise ReplayError("fixtures must not be empty")
    output_path = _check_batch_output(output)
    names = [
        str(item.get("name") or item.get("function") or "custom")
        if isinstance(item, Mapping)
        else str(item)
        for item in fixtures
    ]
    if len(set(names)) != len(names):
        raise ReplayError("replay batch fixture names must be unique")
    if storage_cap_bytes is not None and (
        type(storage_cap_bytes) is not int or storage_cap_bytes <= 0
    ):
        raise ReplayError("replay batch storage cap must be a positive integer")
    if max_wall_seconds is not None and (
        type(max_wall_seconds) not in (int, float) or max_wall_seconds <= 0
    ):
        raise ReplayError("replay batch timing cap must be positive")

    def one(index: int) -> dict[str, Any]:
        fixture = fixtures[index]
        # Keep the disposable checkout path short.  Windows' legacy path
        # ceiling is reached easily by the repository's deep MSL include
        # hierarchy when a descriptive fixture name is repeated below a
        # caller-selected batch directory.  The aggregate receipt already
        # binds the ordered fixture name, so the child directory needs only a
        # stable ordinal.
        child = output_path / f"{index:02d}"
        prepared = prepare_replay(
            root,
            fixture,
            child,
            inventory=inventory,
            storage_cap_bytes=storage_cap_bytes,
        )
        return run_replay(
            prepared,
            runtime=_runtime_for(runtime_factory, fixture, index),
            clean=clean,
            max_seconds=max_wall_seconds,
            storage_cap_bytes=storage_cap_bytes,
        )

    started = time.monotonic()
    results: list[Mapping[str, Any]] = []
    errors: list[Mapping[str, Any]] = []
    if concurrent:
        with ThreadPoolExecutor(max_workers=len(fixtures), thread_name_prefix="owner-replay") as pool:
            futures = {
                pool.submit(one, index): index for index in range(len(fixtures))
            }
            completed: dict[int, Mapping[str, Any]] = {}
            failed: dict[int, Mapping[str, Any]] = {}
            for future in as_completed(futures):
                index = futures[future]
                try:
                    completed[index] = future.result()
                except BaseException as exc:
                    failed[index] = {"fixture": names[index], "error": str(exc)}
            results.extend(completed[index] for index in sorted(completed))
            errors.extend(failed[index] for index in sorted(failed))
    else:
        for index in range(len(fixtures)):
            try:
                results.append(one(index))
            except BaseException as exc:
                errors.append({"fixture": names[index], "error": str(exc)})
                # Preserve deterministic sequential semantics: once a child
                # fails, later children are not allowed to run under a
                # partially valid aggregate gate.
                break

    # Future completion order is deliberately not part of the receipt.  Use
    # the requested fixture order so replay reports are stable across runs.
    results.sort(key=lambda item: names.index(str(item.get("fixture"))))

    receipt = _aggregate_receipt(
        output=output_path,
        mode="concurrent" if concurrent else "sequential",
        fixtures=names,
        results=results,
        started=started,
        errors=errors,
        storage_cap_bytes=storage_cap_bytes,
        max_wall_seconds=max_wall_seconds,
    )
    if receipt["timing"]["within_cap"] is not True:
        raise ReplayError("replay batch timing cap exceeded")
    if receipt["storage"]["within_cap"] is not True:
        raise ReplayError("replay batch storage cap exceeded")
    if receipt["aggregate"]["all_exact"] is not True:
        raise ReplayError("replay batch did not reach exact for every fixture")
    return receipt


def run_three_replay_gate(
    root: Path | str | None,
    output: Path | str,
    *,
    inventory: Mapping[str, Any] | None = None,
    runtime_factory: Any = None,
    storage_cap_bytes: int | None = None,
    max_wall_seconds: float | None = None,
) -> dict[str, Any]:
    """Run the required SetupMgType/BomheiMove/BobleOMExec gate sequentially."""

    return run_replay_batch(
        root,
        ("SetupMgType", "mbev_CapBomheiMove", "ev_CapBobleOMExec"),
        output,
        inventory=inventory,
        runtime_factory=runtime_factory,
        concurrent=False,
        storage_cap_bytes=storage_cap_bytes,
        max_wall_seconds=max_wall_seconds,
    )


def run_concurrent_replays(
    root: Path | str | None,
    fixtures: Sequence[str | Mapping[str, Any]],
    output: Path | str,
    *,
    inventory: Mapping[str, Any] | None = None,
    runtime_factory: Any = None,
    storage_cap_bytes: int | None = None,
    max_wall_seconds: float | None = None,
) -> dict[str, Any]:
    """Run a concurrent isolation gate for independent fixture replays."""

    return run_replay_batch(
        root,
        fixtures,
        output,
        inventory=inventory,
        runtime_factory=runtime_factory,
        concurrent=True,
        storage_cap_bytes=storage_cap_bytes,
        max_wall_seconds=max_wall_seconds,
    )


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixture", choices=fixture_names())
    parser.add_argument(
        "--three-replay-gate",
        action="store_true",
        help="run the three required historical fixtures sequentially",
    )
    parser.add_argument(
        "--concurrent-replay-gate",
        action="store_true",
        help="run the three required historical fixtures concurrently",
    )
    parser.add_argument("--root", type=Path, help="repository containing the release commit")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--inventory", type=Path)
    parser.add_argument("--retain-raw", action="store_true")
    parser.add_argument("--storage-cap-bytes", type=int)
    parser.add_argument("--max-wall-seconds", type=float)
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    inventory = _read_json(args.inventory, "inventory") if args.inventory else None
    if inventory is not None and not isinstance(inventory, Mapping):
        raise ReplayError("inventory must be a JSON object")
    if args.three_replay_gate or args.concurrent_replay_gate:
        if args.three_replay_gate and args.concurrent_replay_gate:
            raise ReplayError("replay gate modes are mutually exclusive")
        if args.concurrent_replay_gate:
            result = run_concurrent_replays(
                args.root,
                ("SetupMgType", "mbev_CapBomheiMove", "ev_CapBobleOMExec"),
                args.output,
                inventory=inventory,
                storage_cap_bytes=args.storage_cap_bytes,
                max_wall_seconds=args.max_wall_seconds,
            )
        else:
            result = run_three_replay_gate(
                args.root,
                args.output,
                inventory=inventory,
                storage_cap_bytes=args.storage_cap_bytes,
                max_wall_seconds=args.max_wall_seconds,
            )
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0
    if args.fixture is None:
        raise ReplayError(
            "one of --fixture, --three-replay-gate, or --concurrent-replay-gate is required"
        )
    prepared = prepare_replay(
        args.root,
        args.fixture,
        args.output,
        inventory=inventory,
        storage_cap_bytes=args.storage_cap_bytes,
    )
    result = run_replay(
        prepared,
        clean=not args.retain_raw,
        max_seconds=args.max_wall_seconds,
        storage_cap_bytes=args.storage_cap_bytes,
    )
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ReplayError as exc:
        print(f"owner replay failed: {exc}", file=sys.stderr)
        raise SystemExit(2)


__all__ = [
    "ReplayError",
    "SCHEMA",
    "AGGREGATE_SCHEMA",
    "HANDLE_SCHEMA",
    "CAPTRAP_TARGET_SHA",
    "MG_TARGET_SHA",
    "fixture_names",
    "fixture_spec",
    "reconstruct_function",
    "prepare_replay",
    "run_replay",
    "load_replay_handle",
    "resume_replay",
    "cleanup_replay",
    "replay",
    "run_replay_batch",
    "run_three_replay_gate",
    "run_concurrent_replays",
]
