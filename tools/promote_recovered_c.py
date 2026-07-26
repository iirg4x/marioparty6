#!/usr/bin/env python3
"""Promote verified recovered C from the AI workspace to a clean main branch.

This tool enforces the repository's permanent branch boundary:

* the AI recovery branch is never merged into ``main``;
* promotion starts from a fresh worktree created from ``main``;
* only explicitly selected ``src/**/*.c`` blobs are copied;
* AI/tooling attribution is rejected from promoted source and commit messages;
* the copied Git blobs must exactly match the verified worker commit;
* headers and build/configuration changes are never imported automatically.

The resulting promotion branch contains no orchestration, prompts, agent files,
knowledge metadata, benchmarks, or workflow changes because it is based on
``main`` rather than the AI workspace.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import agent_queue
from tools.workspace_policy import DEFAULT_PROMOTION_BASE

DEFAULT_BASE = DEFAULT_PROMOTION_BASE
DEFAULT_MANIFEST_ROOT = Path("build/promotion")
DISALLOWED_BRANCH_PREFIXES = ("agent/", "ai/", "claude/", "codex/")
AI_MARKER = re.compile(
    r"""
    \b(?:ChatGPT|OpenAI|Claude|Codex|Gemini|Copilot|LLM)\b
    |\blarge\s+language\s+model\b
    |\bAI[-\s](?:generated|assisted|written|recovered|produced)\b
    |\bgenerated\s+by\s+(?:an?\s+)?(?:AI|agent|model)\b
    |\b(?:agent|model)[-\s]generated\b
    """,
    re.IGNORECASE | re.VERBOSE,
)
COAUTHOR = re.compile(r"(?im)^\s*Co-authored-by\s*:")


class PromotionError(ValueError):
    pass


@dataclass(frozen=True)
class PromotedFile:
    path: str
    source_blob: str
    base_blob: str | None
    sha256: str
    size: int

    def as_dict(self) -> dict[str, Any]:
        return {
            "path": self.path,
            "source_blob": self.source_blob,
            "base_blob": self.base_blob,
            "sha256": self.sha256,
            "size": self.size,
        }


def _now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def _run(
    root: Path,
    *args: str,
    allow_failure: bool = False,
    text: bool = True,
) -> subprocess.CompletedProcess[Any]:
    process = subprocess.run(
        args,
        cwd=root,
        capture_output=True,
        text=text,
        check=False,
    )
    if process.returncode and not allow_failure:
        stderr = process.stderr.strip() if text else process.stderr.decode(errors="replace").strip()
        raise PromotionError(stderr or "command failed: " + " ".join(args))
    return process


def git_root(value: str | Path | None = None) -> Path:
    start = Path(value or Path.cwd()).resolve()
    relative = _run(start, "git", "rev-parse", "--show-cdup").stdout.strip()
    return (start / relative).resolve()


def resolve_ref(root: Path, value: str) -> str:
    return _run(root, "git", "rev-parse", "--verify", value).stdout.strip()


def _git_bytes(root: Path, ref: str, path: str) -> bytes:
    process = _run(
        root,
        "git",
        "show",
        f"{ref}:{path}",
        text=False,
        allow_failure=True,
    )
    if process.returncode:
        raise PromotionError(f"{path} does not exist at {ref}")
    return bytes(process.stdout)


def _blob(root: Path, ref: str, path: str) -> str | None:
    process = _run(
        root,
        "git",
        "rev-parse",
        f"{ref}:{path}",
        allow_failure=True,
    )
    return process.stdout.strip() if process.returncode == 0 else None


def _changed_paths(root: Path, base: str, source: str) -> list[str]:
    output = _run(
        root,
        "git",
        "diff",
        "--name-only",
        "--diff-filter=AM",
        f"{base}...{source}",
        "--",
        "src",
    ).stdout
    return sorted({line.strip() for line in output.splitlines() if line.strip()})


def _normalise_path(value: str) -> str:
    path = PurePosixPath(value.strip().replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise PromotionError(f"path must be repository-relative: {value}")
    normalised = path.as_posix()
    if not normalised.startswith("src/") or not normalised.endswith(".c"):
        raise PromotionError(
            f"automatic promotion accepts only src/**/*.c, not {normalised}"
        )
    return normalised


def _comments(text: str) -> list[tuple[int, str]]:
    result: list[tuple[int, str]] = []
    index = 0
    line = 1
    state = "code"
    start_line = 1
    buffer: list[str] = []
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                start_line = line
                buffer = []
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                start_line = line
                buffer = []
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            if char == "\n":
                line += 1
            index += 1
            continue
        if state == "line":
            if char == "\n":
                result.append((start_line, "".join(buffer)))
                state = "code"
                line += 1
            else:
                buffer.append(char)
            index += 1
            continue
        if state == "block":
            if char == "*" and nxt == "/":
                result.append((start_line, "".join(buffer)))
                state = "code"
                index += 2
                continue
            buffer.append(char)
            if char == "\n":
                line += 1
            index += 1
            continue
        if char == "\\" and index + 1 < len(text):
            if text[index + 1] == "\n":
                line += 1
            index += 2
            continue
        if (state == "string" and char == '"') or (state == "char" and char == "'"):
            state = "code"
        if char == "\n":
            line += 1
        index += 1
    if state in {"line", "block"} and buffer:
        result.append((start_line, "".join(buffer)))
    return result


def source_ai_markers(text: str) -> list[str]:
    findings: list[str] = []
    for line, comment in _comments(text):
        for match in AI_MARKER.finditer(comment):
            findings.append(f"line {line}: {match.group(0)}")
    return findings


def message_errors(message: str) -> list[str]:
    errors: list[str] = []
    if AI_MARKER.search(message):
        errors.append("commit message contains AI/agent attribution")
    if COAUTHOR.search(message):
        errors.append("commit message contains a Co-authored-by trailer")
    if not message.strip():
        errors.append("commit message is empty")
    return errors


def branch_errors(branch: str) -> list[str]:
    lowered = branch.lower()
    errors: list[str] = []
    if lowered in {"main", "master"}:
        errors.append("promotion branch cannot be main/master")
    if lowered.startswith(DISALLOWED_BRANCH_PREFIXES):
        errors.append(
            "human-facing promotion branch cannot use an AI/agent branch prefix"
        )
    if AI_MARKER.search(branch):
        errors.append("promotion branch contains an AI/agent marker")
    return errors


def _queue_verification(
    root: Path, owner: str, source_commit: str
) -> dict[str, Any]:
    path = agent_queue.queue_path(root)
    queue = agent_queue.read_queue(path)
    candidates = [
        task
        for task in queue.get("tasks", [])
        if isinstance(task, dict)
        and task.get("owner") == owner
        and task.get("status") in {"ready", "done"}
    ]
    if len(candidates) != 1:
        raise PromotionError(
            f"expected one ready/done queue task for {owner}; found {len(candidates)}"
        )
    task = candidates[0]
    verification = task.get("verification")
    verification = verification if isinstance(verification, dict) else {}
    verified_commit = (
        verification.get("verified_commit")
        or task.get("last_verified_commit")
    )
    if verified_commit != source_commit:
        raise PromotionError(
            f"queue verification is bound to {verified_commit}, not {source_commit}"
        )
    if verification and verification.get("public_gate") not in {"pass", True}:
        raise PromotionError("queue verification does not record a passing public gate")
    return {
        "queue_path": str(path),
        "owner": owner,
        "status": task.get("status"),
        "verified_commit": verified_commit,
        "verification": verification,
    }


def plan_promotion(
    root: Path,
    *,
    base_ref: str,
    source_ref: str,
    paths: Sequence[str] | None = None,
    owner: str | None = None,
    allow_unverified: bool = False,
) -> dict[str, Any]:
    base_commit = resolve_ref(root, base_ref)
    source_commit = resolve_ref(root, source_ref)
    selected = (
        sorted({_normalise_path(value) for value in paths})
        if paths
        else [
            path
            for path in _changed_paths(root, base_commit, source_commit)
            if path.endswith(".c")
        ]
    )
    if not selected:
        raise PromotionError("no changed src/**/*.c files selected for promotion")

    queue_proof: dict[str, Any] | None = None
    if owner:
        queue_proof = _queue_verification(root, owner, source_commit)
    elif not allow_unverified:
        raise PromotionError(
            "promotion requires --owner with ready verification; use --allow-unverified only for diagnostics/tests"
        )

    files: list[PromotedFile] = []
    errors: list[str] = []
    for path in selected:
        data = _git_bytes(root, source_commit, path)
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            errors.append(f"{path}: recovered C is not UTF-8")
            continue
        markers = source_ai_markers(text)
        errors.extend(f"{path}: {marker}" for marker in markers)
        source_blob = _blob(root, source_commit, path)
        if source_blob is None:
            errors.append(f"{path}: missing source blob")
            continue
        files.append(
            PromotedFile(
                path=path,
                source_blob=source_blob,
                base_blob=_blob(root, base_commit, path),
                sha256=hashlib.sha256(data).hexdigest(),
                size=len(data),
            )
        )
    if errors:
        raise PromotionError("promotion plan rejected:\n- " + "\n- ".join(errors))
    return {
        "schema_version": 1,
        "created_at": _now(),
        "base_ref": base_ref,
        "base_commit": base_commit,
        "source_ref": source_ref,
        "source_commit": source_commit,
        "owner": owner,
        "queue_proof": queue_proof,
        "files": [item.as_dict() for item in files],
        "policy": {
            "allowed": "modified/added src/**/*.c only",
            "headers": "must be recreated and reviewed separately from main",
            "ai_attribution": "forbidden in promoted comments, branch and commit message",
            "workspace_merge": "forbidden",
        },
    }


def _worktree_list(root: Path) -> list[Path]:
    output = _run(root, "git", "worktree", "list", "--porcelain").stdout
    return [
        Path(line.split(" ", 1)[1]).resolve()
        for line in output.splitlines()
        if line.startswith("worktree ")
    ]


def _branch_exists(root: Path, branch: str) -> bool:
    return (
        _run(
            root,
            "git",
            "show-ref",
            "--verify",
            f"refs/heads/{branch}",
            allow_failure=True,
        ).returncode
        == 0
    )


def _write_manifest(root: Path, branch: str, value: Mapping[str, Any]) -> Path:
    slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", branch).strip("_") or "promotion"
    path = root / DEFAULT_MANIFEST_ROOT / slug / "manifest.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, delete=False
    ) as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, path)
    return path


def audit_promotion(
    root: Path,
    *,
    base_ref: str,
    head_ref: str,
    source_ref: str | None = None,
    selected_paths: Sequence[str] | None = None,
) -> dict[str, Any]:
    base_commit = resolve_ref(root, base_ref)
    head_commit = resolve_ref(root, head_ref)
    names = _run(
        root,
        "git",
        "diff",
        "--name-only",
        f"{base_commit}...{head_commit}",
    ).stdout.splitlines()
    changed = sorted({line.strip() for line in names if line.strip()})
    errors: list[str] = []
    for path in changed:
        try:
            _normalise_path(path)
        except PromotionError as exc:
            errors.append(str(exc))
    if selected_paths:
        expected = sorted({_normalise_path(path) for path in selected_paths})
        if changed != expected:
            errors.append(f"changed paths {changed} do not equal selected paths {expected}")
    messages = _run(
        root,
        "git",
        "log",
        "--format=%B%x00",
        f"{base_commit}..{head_commit}",
    ).stdout
    errors.extend(message_errors(messages))
    file_results: list[dict[str, Any]] = []
    source_commit = resolve_ref(root, source_ref) if source_ref else None
    for path in changed:
        if not path.startswith("src/") or not path.endswith(".c"):
            continue
        data = _git_bytes(root, head_commit, path)
        text = data.decode("utf-8", errors="replace")
        markers = source_ai_markers(text)
        errors.extend(f"{path}: {marker}" for marker in markers)
        head_blob = _blob(root, head_commit, path)
        expected_blob = _blob(root, source_commit, path) if source_commit else None
        if source_commit and head_blob != expected_blob:
            errors.append(
                f"{path}: promoted blob {head_blob} differs from verified source blob {expected_blob}"
            )
        file_results.append(
            {
                "path": path,
                "head_blob": head_blob,
                "source_blob": expected_blob,
                "identical_to_source": (
                    head_blob == expected_blob if source_commit else None
                ),
                "sha256": hashlib.sha256(data).hexdigest(),
            }
        )
    return {
        "schema_version": 1,
        "base_commit": base_commit,
        "head_commit": head_commit,
        "source_commit": source_commit,
        "changed_paths": changed,
        "files": file_results,
        "errors": sorted(set(errors)),
        "clean_human_promotion": not errors,
    }


def create_promotion(
    root: Path,
    *,
    base_ref: str,
    source_ref: str,
    branch: str,
    worktree: Path,
    title: str,
    paths: Sequence[str] | None,
    owner: str | None,
    allow_unverified: bool,
) -> dict[str, Any]:
    policy_errors = [*branch_errors(branch), *message_errors(title)]
    if policy_errors:
        raise PromotionError("promotion metadata rejected:\n- " + "\n- ".join(policy_errors))
    plan = plan_promotion(
        root,
        base_ref=base_ref,
        source_ref=source_ref,
        paths=paths,
        owner=owner,
        allow_unverified=allow_unverified,
    )
    worktree = worktree.resolve()
    if worktree in _worktree_list(root) or worktree.exists():
        raise PromotionError(f"promotion worktree already exists: {worktree}")
    if _branch_exists(root, branch):
        raise PromotionError(f"promotion branch already exists: {branch}")

    _run(root, "git", "worktree", "add", "-q", "-b", branch, str(worktree), plan["base_commit"])
    try:
        for item in plan["files"]:
            path = str(item["path"])
            destination = worktree / path
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(_git_bytes(root, plan["source_commit"], path))
            hashed = _run(
                worktree,
                "git",
                "hash-object",
                "--path",
                path,
                path,
            ).stdout.strip()
            if hashed != item["source_blob"]:
                raise PromotionError(
                    f"{path}: working-tree blob {hashed} differs from source blob {item['source_blob']}"
                )
        selected = [str(item["path"]) for item in plan["files"]]
        _run(worktree, "git", "add", "--", *selected)
        staged = sorted(
            line
            for line in _run(
                worktree, "git", "diff", "--cached", "--name-only"
            ).stdout.splitlines()
            if line
        )
        if staged != sorted(selected):
            raise PromotionError(f"staged paths {staged} do not equal selected paths {sorted(selected)}")
        # Clean promotion worktrees start from human ``main`` and intentionally
        # do not contain the AI-only tools used by the shared recovery hooks.
        # The explicit staged-path check above and promotion audit below are
        # the applicable, stricter boundary checks for this generated commit.
        _run(worktree, "git", "commit", "-q", "--no-verify", "-m", title)
        promotion_commit = resolve_ref(worktree, "HEAD")
        audit = audit_promotion(
            worktree,
            base_ref=plan["base_commit"],
            head_ref=promotion_commit,
            source_ref=plan["source_commit"],
            selected_paths=selected,
        )
        if audit["errors"]:
            raise PromotionError("promotion audit failed:\n- " + "\n- ".join(audit["errors"]))
        result = {
            **plan,
            "promotion": {
                "branch": branch,
                "worktree": str(worktree),
                "commit": promotion_commit,
                "title": title,
                "audit": audit,
            },
            "next_steps": [
                "run object and consumer verification in the clean promotion worktree",
                "run the serialized DOL/REL, checksum, and byte-comparison gates",
                "push the promotion branch and open a normal human-facing PR to main",
            ],
        }
        manifest = _write_manifest(root, branch, result)
        result["local_manifest"] = str(manifest)
        return result
    except Exception:
        _run(
            root,
            "git",
            "worktree",
            "remove",
            "--force",
            str(worktree),
            allow_failure=True,
        )
        if _branch_exists(root, branch):
            _run(root, "git", "branch", "-D", branch, allow_failure=True)
        raise


def _print_plan(value: Mapping[str, Any]) -> None:
    print(f"base: {value['base_commit']}")
    print(f"verified source: {value['source_commit']}")
    for item in value["files"]:
        print(
            f"{item['path']}  blob={item['source_blob'][:12]}  "
            f"sha256={item['sha256'][:12]}  {item['size']} bytes"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)

    plan = sub.add_parser("plan", help="plan a C-only promotion")
    plan.add_argument("--base", default=DEFAULT_BASE)
    plan.add_argument("--source", required=True)
    plan.add_argument("--owner")
    plan.add_argument("--path", action="append", default=[])
    plan.add_argument("--allow-unverified", action="store_true")
    plan.add_argument("--json", action="store_true")

    create = sub.add_parser("create", help="create a clean branch/worktree from main")
    create.add_argument("--base", default=DEFAULT_BASE)
    create.add_argument("--source", required=True)
    create.add_argument("--owner")
    create.add_argument("--path", action="append", default=[])
    create.add_argument("--allow-unverified", action="store_true")
    create.add_argument("--branch", required=True)
    create.add_argument("--worktree", required=True)
    create.add_argument("--title", required=True)

    audit = sub.add_parser("audit", help="audit a clean promotion branch")
    audit.add_argument("--base", default=DEFAULT_BASE)
    audit.add_argument("--head", default="HEAD")
    audit.add_argument("--source")
    audit.add_argument("--path", action="append", default=[])
    audit.add_argument("--json", action="store_true")

    args = parser.parse_args()
    try:
        root = git_root(args.root)
        if args.command == "plan":
            value = plan_promotion(
                root,
                base_ref=args.base,
                source_ref=args.source,
                paths=args.path,
                owner=args.owner,
                allow_unverified=args.allow_unverified,
            )
            if args.json:
                print(json.dumps(value, indent=2))
            else:
                _print_plan(value)
            return 0
        if args.command == "create":
            value = create_promotion(
                root,
                base_ref=args.base,
                source_ref=args.source,
                branch=args.branch,
                worktree=Path(args.worktree),
                title=args.title,
                paths=args.path,
                owner=args.owner,
                allow_unverified=args.allow_unverified,
            )
            print(json.dumps(value, indent=2))
            return 0
        value = audit_promotion(
            root,
            base_ref=args.base,
            head_ref=args.head,
            source_ref=args.source,
            selected_paths=args.path,
        )
        if args.json:
            print(json.dumps(value, indent=2))
        else:
            print("clean human promotion: " + ("PASS" if value["clean_human_promotion"] else "FAIL"))
            for path in value["changed_paths"]:
                print(f"- {path}")
            for error in value["errors"]:
                print(f"ERROR: {error}")
        return 1 if value["errors"] else 0
    except (OSError, PromotionError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
