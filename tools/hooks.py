#!/usr/bin/env python3
"""Install lightweight local hooks for draft-phase agent work."""

from __future__ import annotations

import argparse
import json
import os
import shlex
import stat
import subprocess
import sys
from pathlib import Path
from typing import Any

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.git_paths import native_git_path

MARKER = "# managed-by-mp6-agent-tools"


class HookError(ValueError):
    pass


def _run(root: Path, *args: str, allow_failure: bool = False) -> str:
    result = subprocess.run(
        args, cwd=root, text=True, capture_output=True, check=False
    )
    if result.returncode and not allow_failure:
        raise HookError(
            result.stderr.strip() or "command failed: " + " ".join(args)
        )
    return result.stdout.strip()


def hooks_dir(root: Path) -> Path:
    configured = _run(
        root,
        "git",
        "config",
        "--get",
        "core.hooksPath",
        allow_failure=True,
    )
    if configured:
        path = Path(configured)
        return (path if path.is_absolute() else root / path).resolve()
    common = native_git_path(
        _run(
            root,
            "git",
            "rev-parse",
            "--path-format=absolute",
            "--git-common-dir",
        ),
        relative_to=root,
    )
    return common / "hooks"


def _common(root: Path) -> Path:
    return native_git_path(_run(root, "git", "rev-parse", "--path-format=absolute",
                                "--git-common-dir"), relative_to=root).resolve()


def _script(kind: str, root: Path, common: Path) -> str:
    # These paths are installation-owned literals, never environment overrides.
    # Partial commits may supply a temporary index. It is the authoritative
    # staged tree for pre-commit, but must not leak into pre-push checks.
    preserve_index = '[ "$name" = GIT_INDEX_FILE ] && continue; ' if kind == "pre-commit" else ""
    return (f"#!/bin/sh\n{MARKER}\nset -eu\n"
            'ROOT="$(git rev-parse --show-toplevel)"\n'
            'LOCAL_GIT_ENV="$(git rev-parse --local-env-vars)"\n'
            f'for name in $LOCAL_GIT_ENV; do {preserve_index}unset "$name"; done\n'
            'PYTHON_BIN="${MP6_PYTHON:-python}"\n'
            f'exec "$PYTHON_BIN" {shlex.quote((root / "tools/hooks.py").as_posix())}'
            f' run {kind} --tool-root {shlex.quote(root.as_posix())}'
            f' --common-dir {shlex.quote(common.as_posix())} --root "$ROOT"\n')


def _commit(root: Path, ref: str) -> str:
    resolved = _run(root, "git", "rev-parse", "--verify", ref)
    if _run(root, "git", "cat-file", "-t", resolved) != "commit":
        raise HookError(f"{ref}: expected a commit")
    return resolved


def _ancestor(root: Path, base: str, head: str) -> bool:
    return subprocess.run(("git", "merge-base", "--is-ancestor", base, head),
                          cwd=root, capture_output=True).returncode == 0


def _clean_tree(root: Path, ref: str) -> None:
    paths = _run(root, "git", "ls-tree", "-r", "--name-only", ref).splitlines()
    forbidden = [p for p in paths if p.startswith("config/recovery/") or
                 p in {"AI_WORKSPACE.md", "tools/agent.py", "tools/hooks.py"} or
                 Path(p).name in {"AGENTS.md", "CLAUDE.md"}]
    if forbidden:
        raise HookError(f"{ref}: AI workspace files in public tree: {forbidden}")


def _changes(root: Path, base: str, head: str) -> set[str]:
    rows = _run(root, "git", "diff", "--no-renames", "--name-status", base, head).splitlines()
    paths = set()
    for row in rows:
        status, path = row.split("\t", 1)
        if status not in {"A", "M"}:
            raise HookError(f"public promotion contains unsupported {status}: {path}")
        mode = _run(root, "git", "ls-tree", head, "--", path).split()[0]
        if mode not in {"100644", "100755"}:
            raise HookError(f"public promotion requires a regular blob: {path}")
        paths.add(path)
    return paths


def _sidecars() -> set[str]:
    from tools import progress_gate
    return {progress_gate.STATUS_PATH, "progress.csv"} | progress_gate.COMMON_PROGRESS_PATHS | set(
        progress_gate.CATEGORY_PROGRESS_PATHS.values())


def _manifests(tool_root: Path) -> list[dict[str, Any]]:
    result = []
    common = _common(tool_root)
    roots = [native_git_path(line[len("worktree "):], relative_to=tool_root)
             for line in _run(tool_root, "git", "worktree", "list", "--porcelain").splitlines()
             if line.startswith("worktree ")]
    for root in roots:
        if not (root / "tools/agent.py").is_file():
            continue
        try:
            if _common(root) != common:
                continue
        except (HookError, OSError):
            # Registered but abandoned worktrees can retain a broken .git
            # pointer (including an old MSYS /home path). They are not trusted
            # manifest sources and must not veto another valid promotion.
            # A missing selected manifest still fails closed in _public_range.
            continue
        for path in (root / "build/promotion").glob("*/manifest.json"):
            try:
                value = json.loads(path.read_text(encoding="utf-8"))
            except (OSError, ValueError):
                # A selected missing/invalid binding fails below; unrelated old
                # scratch manifests must not veto a different promotion.
                continue
            if not isinstance(value, dict):
                continue
            value["_manifest_root"] = root
            result.append(value)
    return result


def _audit_manifest(tool_root: Path, root: Path, value: dict[str, Any], main: str) -> str:
    from tools import promote_recovered_c as recovered, promote_supporting_change as supporting
    promotion = value.get("promotion", {})
    head = _commit(root, promotion["commit"])
    base = _commit(root, value["base_commit"])
    if head != promotion["commit"] or base != value["base_commit"]:
        raise HookError("promotion manifest must bind exact commits")
    if _run(root, "git", "show", "-s", "--format=%P", head) != base or not _ancestor(root, base, main):
        raise HookError(f"{head}: promotion must be a single commit directly from clean main ancestry")
    _clean_tree(root, base)
    _clean_tree(root, head)
    branch = promotion["branch"]
    module = supporting if branch.startswith("project/") else recovered
    errors = (supporting.supporting_branch_errors(branch) if module is supporting else
              recovered.branch_errors(branch))
    if errors:
        raise HookError("; ".join(errors))
    selected = [item["path"] for item in value["files"]]
    if not selected or _changes(root, base, head) != set(selected):
        raise HookError(f"{head}: promotion diff does not equal manifest files")
    # Reconstruct the plan from live queue/source evidence. Stored proof booleans
    # and caller-supplied metadata are not authorization to skip verification.
    policy_root = value["_manifest_root"]
    policy_path = "config/recovery/exceptions.json"
    policy_file = policy_root / policy_path
    policy_blob = recovered._blob(tool_root, value["source_commit"], policy_path)
    if policy_blob:
        if not policy_file.is_file() or policy_file.read_bytes() != recovered._git_bytes(
                tool_root, value["source_commit"], policy_path):
            raise HookError("promotion policy must equal its verified source-commit blob")
    elif policy_file.exists():
        raise HookError("promotion policy is not present in its verified source commit")
    plan = module.plan_promotion(policy_root, base_ref=base, source_ref=value["source_commit"],
                                 paths=selected, owner=value.get("owner"))
    if plan["source_commit"] != value["source_commit"] or plan["files"] != value["files"]:
        raise HookError(f"{head}: promotion manifest does not match verified source plan")
    kwargs = {} if module is supporting else {"policy_root": policy_root}
    audit = module.audit_promotion(root, base_ref=base, head_ref=head,
                                  source_ref=plan["source_commit"], selected_paths=selected, **kwargs)
    if audit["errors"]:
        raise HookError("; ".join(audit["errors"]))
    return head


def _public_range(tool_root: Path, root: Path, branch: str, head: str,
                  remote_base: str | None = None, *, check_progress: bool = True) -> str:
    from tools import progress_gate
    main = _commit(root, "refs/heads/main")
    _clean_tree(root, main)
    _clean_tree(root, head)
    manifests = _manifests(tool_root)
    if branch == "main":
        if not remote_base or set(remote_base) == {"0"}:
            raise HookError("cannot validate a new main branch without a base")
        base = _commit(root, remote_base)
        _clean_tree(root, base)
        if not _ancestor(root, base, head):
            raise HookError("main push must descend from its exact remote base")
        # GitHub merges/squashes change promotion commit identities. The
        # recovery/project ref gate authenticates source transfers beforehand;
        # main retains its existing progress gate plus the clean boundary.
        from tools import promote_recovered_c as recovered, promote_supporting_change as supporting
        for commit in _run(root, "git", "rev-list", f"{base}..{head}").splitlines():
            _clean_tree(root, commit)
        for path in _changes(root, base, head) - _sidecars():
            try:
                recovered._normalise_path(path)
            except recovered.PromotionError:
                supporting._normalise_path(path)
        errors = progress_gate.check_range(root, base, head)
        if errors:
            raise HookError("; ".join(errors))
        return base
    else:
        candidates = [m for m in manifests if m.get("promotion", {}).get("branch") == branch
                      and _ancestor(root, str(m["promotion"].get("commit", "")), head)]
        if len(candidates) != 1:
            raise HookError(f"{branch}: expected one exact promotion manifest, found {len(candidates)}")
        _audit_manifest(tool_root, root, candidates[0], main)
        base = candidates[0]["base_commit"]
    for commit in _run(root, "git", "rev-list", "--reverse", f"{base}..{head}").splitlines():
        parents = _run(root, "git", "show", "-s", "--format=%P", commit).split()
        _clean_tree(root, commit)
        if len(parents) != 1:
            if branch != "main" or len(parents) != 2:
                raise HookError(f"{commit}: unsupported public merge history")
            # Normal human PR merges are allowed, but conflict resolution may
            # not introduce an unverified source/config blob of its own.
            for path in _changes(root, parents[0], commit) - _sidecars():
                blob = _run(root, "git", "rev-parse", f"{commit}:{path}")
                if not any(blob == _run(root, "git", "rev-parse", f"{parent}:{path}",
                                        allow_failure=True) for parent in parents):
                    raise HookError(f"{commit}: merge introduced an unverified blob: {path}")
            continue
        paths = _changes(root, parents[0], commit)
        if paths <= _sidecars():
            continue
        matches = [m for m in manifests if m.get("promotion", {}).get("commit") == commit]
        if len(matches) != 1:
            raise HookError(f"{commit}: public source/config changes lack an exact promotion manifest")
        _audit_manifest(tool_root, root, matches[0], main)
    # A source-only recovery PR precedes its separate public status integration.
    # Supporting/public-main updates, including descendant sidecars, keep the
    # existing progress gate; none of this replaces object/consumer/retail proof.
    if check_progress and (branch == "main" or branch.startswith("project/")):
        errors = progress_gate.check_range(root, base, head)
        if errors:
            raise HookError("; ".join(errors))
    return base


def _ai_checks(tool_root: Path, root: Path, kind: str) -> None:
    # AI work validates the checked-out candidate tooling, not the version
    # pinned for dispatching clean worktrees that intentionally lack it.
    tool_root = root
    def command(*args: str) -> None:
        result = subprocess.run((sys.executable, *args), cwd=root, check=False)
        if result.returncode:
            raise HookError("hook check failed: " + " ".join(args))
    command(str(tool_root / "tools/agent.py"), "--root", str(root), "memory", "startup-check", "--no-sync")
    if kind == "pre-commit":
        args = ("--base", os.environ["MP6_AGENT_BASE"]) if os.environ.get("MP6_AGENT_BASE") else ()
        command(str(tool_root / "tools/claim_diff.py"), "--root", str(root), *args)
        changed = _run(root, "git", "diff", "--cached", "--name-only", "--diff-filter=ACMR").splitlines()
        python = [p for p in changed if p.endswith(".py")]
        if python:
            command("-m", "py_compile", *python)
    else:
        for script, arg in (("recovery_index.py", "check"), ("knowledge_cards.py", "check"),
                            ("blind_recovery.py", "audit")):
            command(str(tool_root / "tools" / script), "--root", str(root), arg)
        command("-m", "unittest", "discover", "-s", str(tool_root / "tools/tests"), "-v")


def run_hook(kind: str, *, root: Path, tool_root: Path, common: Path, stdin: str = "") -> None:
    root, tool_root, common = root.resolve(), tool_root.resolve(), common.resolve()
    if (Path(__file__).resolve() != tool_root / "tools/hooks.py" or
            not (tool_root / "tools/agent.py").is_file() or
            _common(tool_root) != common or _common(root) != common):
        raise HookError("missing or untrusted installed tooling root/common Git directory")
    if kind == "pre-commit":
        branch = _run(root, "git", "symbolic-ref", "--short", "HEAD")
        if branch == "main":
            raise HookError("never edit directly on main")
        if branch.startswith(("recovery/", "project/")):
            head = _commit(root, "HEAD")
            base = _public_range(tool_root, root, branch, head, check_progress=False)
            tree = _run(root, "git", "write-tree")
            _clean_tree(root, tree)
            if not _changes(root, head, tree) <= _sidecars():
                raise HookError("clean promotion edits require a fresh verified promotion; only progress sidecars may follow")
            if branch.startswith("project/"):
                from tools import progress_gate
                errors = progress_gate.check_range(root, base, tree)
                if errors:
                    raise HookError("; ".join(errors))
            return
        if not (root / "tools/agent.py").is_file():
            raise HookError(f"unrecognized clean branch: {branch}")
        _ai_checks(tool_root, root, kind)
        return
    if kind != "pre-push":
        raise HookError(f"unsupported hook {kind}")
    ai = public = ai_head_mismatch = False
    for line in stdin.splitlines():
        fields = line.split()
        if len(fields) != 4:
            raise HookError(f"malformed pre-push ref line: {line}")
        _, sha, remote_ref, remote_sha = fields
        if set(sha) == {"0"}:
            if remote_ref == "refs/heads/main":
                raise HookError("deleting main is not a promotion")
            continue
        head = _commit(root, sha)
        if not remote_ref.startswith("refs/heads/"):
            raise HookError(f"unrecognized public ref: {remote_ref}")
        branch = remote_ref[len("refs/heads/"):]
        if branch == "main" or branch.startswith(("recovery/", "project/")):
            _public_range(tool_root, root, branch, head, remote_sha)
            public = True
        elif _run(root, "git", "ls-tree", head, "--", "tools/agent.py"):
            ai = True
            if head != _commit(root, "HEAD") or not (root / "tools/agent.py").is_file():
                # Defer until all refs have been classified so mixed pushes
                # receive their more useful boundary error first.
                ai_head_mismatch = True
        else:
            raise HookError(f"unrecognized clean/public ref: {remote_ref}")
    if ai and public:
        raise HookError("push AI workspace and public promotion refs separately")
    if ai:
        if ai_head_mismatch:
            raise HookError("push AI refs from their checked-out source worktree/HEAD")
        _ai_checks(tool_root, root, kind)


def install_hooks(root: Path, *, force: bool = False) -> list[Path]:
    root = root.resolve()
    if not (root / "tools/hooks.py").is_file() or not (root / "tools/agent.py").is_file():
        raise HookError(f"install hooks from a trusted AI tooling worktree: {root}")
    directory = hooks_dir(root)
    directory.mkdir(parents=True, exist_ok=True)
    installed: list[Path] = []
    for name in ("pre-commit", "pre-push"):
        path = directory / name
        if (
            path.exists()
            and MARKER
            not in path.read_text(encoding="utf-8", errors="replace")
            and not force
        ):
            raise HookError(
                f"refusing to replace unmanaged hook {path}; use --force"
            )
        path.write_text(_script(name, root, _common(root)), encoding="utf-8", newline="\n")
        path.chmod(
            path.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH
        )
        installed.append(path)
    return installed


def uninstall_hooks(root: Path) -> list[Path]:
    removed: list[Path] = []
    for name in ("pre-commit", "pre-push"):
        path = hooks_dir(root) / name
        if path.is_file() and MARKER in path.read_text(
            encoding="utf-8", errors="replace"
        ):
            path.unlink()
            removed.append(path)
    return removed


def hook_status(root: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for name in ("pre-commit", "pre-push"):
        path = hooks_dir(root) / name
        if not path.exists():
            result[name] = "missing"
        elif MARKER in path.read_text(encoding="utf-8", errors="replace"):
            result[name] = "managed"
        else:
            result[name] = "unmanaged"
    return result


def add_hooks_parser(subparsers: Any) -> argparse.ArgumentParser:
    parser = subparsers.add_parser(
        "hooks", help="install/status local agent hooks"
    )
    commands = parser.add_subparsers(dest="hooks_command", required=True)
    install = commands.add_parser("install")
    install.add_argument("--force", action="store_true")
    commands.add_parser("status")
    commands.add_parser("uninstall")
    return parser


def run_hooks_command(args: argparse.Namespace, *, root: Path) -> int:
    if args.hooks_command == "install":
        paths = install_hooks(root, force=args.force)
        for path in paths:
            print(f"installed {path}")
    elif args.hooks_command == "uninstall":
        paths = uninstall_hooks(root)
        for path in paths:
            print(f"removed {path}")
    else:
        for name, status in hook_status(root).items():
            print(f"{name}: {status}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["run"])
    parser.add_argument("kind", choices=["pre-commit", "pre-push"])
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--tool-root", type=Path, required=True)
    parser.add_argument("--common-dir", type=Path, required=True)
    args = parser.parse_args()
    try:
        run_hook(args.kind, root=args.root, tool_root=args.tool_root,
                 common=args.common_dir, stdin=sys.stdin.read() if args.kind == "pre-push" else "")
    except (ValueError, KeyError, TypeError, OSError) as exc:
        print(f"MP6 hook rejected: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
