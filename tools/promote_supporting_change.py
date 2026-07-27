#!/usr/bin/env python3
"""Promote a verified supporting change to a clean ``project/*`` branch.

Supporting changes are the non-C dependencies of recovered source: shared
headers under ``include/**``, symbols/splits/declarations under ``config/**``,
and ``configure.py`` object-status flips. ``tools/promote_recovered_c.py``
deliberately refuses them; this tool gives them the same audited path to
``main``:

* the AI workspace branch is never merged into ``main``;
* the ``project/*`` branch is created directly from ``main``;
* only explicitly declared supporting paths are transferred;
* AI attribution and AI-workspace contamination (prompt-style comments,
  agent/queue identifiers, coordination paths) are rejected, never stripped;
* every transferred blob must exactly equal the verified worker-commit blob;
* every Matching TU that consumes a changed header is listed explicitly and
  must be re-verified before the audit passes;
* ``src/**/*.c`` is refused with a pointer to the recovered-C tool.
"""

from __future__ import annotations

import argparse
import difflib
import hashlib
import json
import re
import sys
from collections import defaultdict, deque
from pathlib import Path, PurePosixPath
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools import agent_queue
from tools.owner_catalog import INCLUDE_RE, CatalogError, _resolve_include, build_catalog
from tools.promote_recovered_c import (
    AI_MARKER,
    COAUTHOR,
    DEFAULT_BASE,
    PromotionError,
    _blob,
    _branch_exists,
    _git_bytes,
    _now,
    _run,
    _worktree_list,
    _write_manifest,
    branch_errors,
    git_root,
    message_errors,
    resolve_ref,
)

RECOVERED_C_TOOL = "tools/promote_recovered_c.py"
REQUIRED_BRANCH_PREFIX = "project/"
SUPPORTING_CHANGE_CLASSES = {"shared-interface", "build-configuration"}
HEADER_SUFFIXES = {".h", ".inc"}
CONTAMINATION = re.compile(
    r"""
    \bTODO\s*\(\s*(?:ai|agent|claude|codex|gpt|llm)[^)]*\)
    |\bagent-coordination\b
    |\bqueue\.json\b
    |\bAI_BASE_COMMIT\b
    |\bagent/recovery-context-workflow\b
    |\b(?:main|REL|unconfigured):[A-Za-z0-9_.-]+(?:[/:][A-Za-z0-9_.-]+)+
    |\bknowledge[\s_-]cards?\b
    |\bprompts?\b
    """,
    re.IGNORECASE | re.VERBOSE,
)


def _normalise_path(value: str) -> str:
    path = PurePosixPath(value.strip().replace("\\", "/"))
    if path.is_absolute() or ".." in path.parts:
        raise PromotionError(f"path must be repository-relative: {value}")
    normalised = path.as_posix()
    if normalised.startswith("src/") and normalised.endswith(".c"):
        raise PromotionError(
            f"{normalised}: recovered C is promoted by {RECOVERED_C_TOOL}, "
            "not the supporting-change tool"
        )
    if normalised.startswith("config/recovery/"):
        raise PromotionError(
            f"{normalised}: config/recovery/ is AI-workspace metadata and never "
            "moves to main"
        )
    allowed = (
        normalised == "configure.py"
        or normalised.startswith("include/")
        or normalised.startswith("config/")
    )
    if not allowed:
        raise PromotionError(
            "supporting promotion accepts only include/**, config/** "
            f"(excluding config/recovery/**), and configure.py, not {normalised}"
        )
    return normalised


def _is_header(path: str) -> bool:
    return (
        path.startswith("include/")
        and PurePosixPath(path).suffix.lower() in HEADER_SUFFIXES
    )


def _kind(path: str) -> str:
    if _is_header(path):
        return "header"
    if path == "configure.py":
        return "build-configuration"
    return "configuration"


def contamination_findings(path: str, text: str) -> list[str]:
    """Scan the full text: attribution and workspace identifiers are forbidden
    in supporting files whether they appear in comments, strings, or names."""
    findings: list[str] = []
    for match in AI_MARKER.finditer(text):
        line = text.count("\n", 0, match.start()) + 1
        findings.append(f"line {line}: AI attribution: {match.group(0)}")
    for match in CONTAMINATION.finditer(text):
        line = text.count("\n", 0, match.start()) + 1
        findings.append(
            f"line {line}: AI-workspace contamination: {match.group(0).strip()}"
        )
    if COAUTHOR.search(text):
        findings.append("contains a Co-authored-by trailer")
    return findings


def _diff_stats(base: str | None, current: str) -> dict[str, int]:
    old = base.splitlines() if base else []
    new = current.splitlines()
    added = removed = 0
    for row in difflib.unified_diff(old, new, lineterm="", n=0):
        if row.startswith("+") and not row.startswith("+++"):
            added += 1
        elif row.startswith("-") and not row.startswith("---"):
            removed += 1
    return {"lines_added": added, "lines_removed": removed}


def _header_reverse_includes(root: Path) -> dict[str, set[str]]:
    reverse: dict[str, set[str]] = defaultdict(set)
    include_dir = root / "include"
    if not include_dir.is_dir():
        return reverse
    for file in sorted(include_dir.rglob("*")):
        if not file.is_file() or file.suffix.lower() not in HEADER_SUFFIXES:
            continue
        rel = file.relative_to(root).as_posix()
        text = file.read_text(encoding="utf-8", errors="replace")
        for include in INCLUDE_RE.findall(text):
            reverse[_resolve_include(root, file, include)].add(rel)
    return reverse


def affected_consumers(
    root: Path, headers: Sequence[str]
) -> dict[str, dict[str, list[str]]]:
    """Map each changed header to the TUs that consume it, directly or
    through the transitive header-include closure of the current tree."""
    if not headers:
        return {}
    try:
        catalog = build_catalog(root)
    except CatalogError as exc:
        raise PromotionError(f"cannot compute header fan-out: {exc}") from exc
    status = {
        str(item["id"]): str(item.get("configured_status"))
        for item in catalog["owners"]
    }
    header_consumers: Mapping[str, list[str]] = catalog["header_consumers"]
    reverse = _header_reverse_includes(root)
    result: dict[str, dict[str, list[str]]] = {}
    for header in sorted(headers):
        closure = {header}
        pending = deque([header])
        while pending:
            for parent in reverse.get(pending.popleft(), ()):
                if parent not in closure:
                    closure.add(parent)
                    pending.append(parent)
        owners = sorted(
            {
                owner
                for member in closure
                for owner in header_consumers.get(member, [])
            }
        )
        result[header] = {
            "include_closure": sorted(closure - {header}),
            "matching": [o for o in owners if status.get(o) == "Matching"],
            "other": [o for o in owners if status.get(o) != "Matching"],
        }
    return result


def _matching_consumers(fanout: Mapping[str, Mapping[str, Any]]) -> list[str]:
    return sorted(
        {owner for entry in fanout.values() for owner in entry.get("matching", [])}
    )


def supporting_branch_errors(branch: str) -> list[str]:
    errors = branch_errors(branch)
    if not branch.startswith(REQUIRED_BRANCH_PREFIX):
        errors.append(
            f"supporting-change branch must start with {REQUIRED_BRANCH_PREFIX}"
        )
    return errors


def supporting_message_errors(message: str) -> list[str]:
    """The contamination scan applies to commit messages too: queue owner ids,
    queue.json, coordination paths, and prompt references must not reach a
    main-facing commit message any more than a promoted file."""
    errors = message_errors(message)
    errors.extend(
        "commit message contains AI-workspace contamination: "
        + match.group(0).strip()
        for match in CONTAMINATION.finditer(message)
    )
    return errors


def _queue_verification(
    root: Path, owner: str, source_commit: str, selected: Sequence[str]
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
    verified_commit = verification.get("verified_commit") or task.get(
        "last_verified_commit"
    )
    if verified_commit != source_commit:
        raise PromotionError(
            f"queue verification is bound to {verified_commit}, not {source_commit}"
        )
    if verification.get("public_gate") not in {"pass", True}:
        raise PromotionError("queue verification does not record a passing public gate")
    change_class = str(task.get("change_class", "private-source"))
    if change_class not in SUPPORTING_CHANGE_CLASSES:
        raise PromotionError(
            f"queue task {owner} has change class {change_class}; a supporting "
            "promotion requires shared-interface or build-configuration"
        )
    if not verification.get("consumers"):
        raise PromotionError(
            f"queue verification for {owner} records no consumer results; "
            "shared-interface/build-configuration proof requires them"
        )
    declared = set(task.get("shared_files", []))
    source_path = task.get("source")
    if isinstance(source_path, str):
        declared.add(source_path)
    undeclared = [value for value in selected if value not in declared]
    if undeclared:
        raise PromotionError(
            "paths not declared in the queue claim: " + ", ".join(undeclared)
        )
    return {
        "queue_path": str(path),
        "owner": owner,
        "status": task.get("status"),
        "change_class": change_class,
        "verified_commit": verified_commit,
        "verification": verification,
    }


def plan_promotion(
    root: Path,
    *,
    base_ref: str,
    source_ref: str,
    paths: Sequence[str],
    owner: str | None = None,
    allow_unverified: bool = False,
) -> dict[str, Any]:
    base_commit = resolve_ref(root, base_ref)
    source_commit = resolve_ref(root, source_ref)
    if not paths:
        raise PromotionError(
            "supporting promotion requires explicit --path declarations; "
            "there is no automatic path selection for shared interfaces"
        )
    selected = sorted({_normalise_path(value) for value in paths})

    queue_proof: dict[str, Any] | None = None
    if owner:
        queue_proof = _queue_verification(root, owner, source_commit, selected)
    elif not allow_unverified:
        raise PromotionError(
            "promotion requires --owner with ready verification; "
            "use --allow-unverified only for diagnostics/tests"
        )

    files: list[dict[str, Any]] = []
    errors: list[str] = []
    for path in selected:
        data = _git_bytes(root, source_commit, path)
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            errors.append(f"{path}: supporting change is not UTF-8")
            continue
        errors.extend(
            f"{path}: {finding}" for finding in contamination_findings(path, text)
        )
        source_blob = _blob(root, source_commit, path)
        base_blob = _blob(root, base_commit, path)
        if source_blob is None:
            errors.append(f"{path}: missing source blob")
            continue
        if source_blob == base_blob:
            errors.append(f"{path}: identical to base; nothing to promote")
            continue
        base_text = (
            _git_bytes(root, base_commit, path).decode("utf-8", errors="replace")
            if base_blob
            else None
        )
        files.append(
            {
                "path": path,
                "kind": _kind(path),
                "source_blob": source_blob,
                "base_blob": base_blob,
                "sha256": hashlib.sha256(data).hexdigest(),
                "size": len(data),
                "diff": _diff_stats(base_text, text),
            }
        )
    if errors:
        raise PromotionError("promotion plan rejected:\n- " + "\n- ".join(errors))

    fanout = affected_consumers(
        root, [item["path"] for item in files if _is_header(str(item["path"]))]
    )
    return {
        "schema_version": 1,
        "created_at": _now(),
        "base_ref": base_ref,
        "base_commit": base_commit,
        "source_ref": source_ref,
        "source_commit": source_commit,
        "owner": owner,
        "queue_proof": queue_proof,
        "files": files,
        "affected_consumers": fanout,
        "affected_matching_consumers": _matching_consumers(fanout),
        "policy": {
            "allowed": "declared include/**, config/** (never config/recovery/**), configure.py",
            "recovered_c": f"promoted separately by {RECOVERED_C_TOOL}",
            "ai_attribution": "forbidden; findings refuse promotion, nothing is stripped",
            "workspace_merge": "forbidden",
            "consumer_reverification": "every affected Matching TU must be re-verified",
        },
    }


def verification_checklist(
    selected: Sequence[str], matching: Sequence[str]
) -> list[str]:
    audit = (
        "python <ai-workspace>/tools/promote_supporting_change.py audit "
        "--root . --base main --head HEAD --source <verified-worker-commit>"
    )
    audit += "".join(f" --path {path}" for path in selected)
    audit += " --build-gate pass --checksum pass"
    audit += "".join(f" --consumer {owner}=exact" for owner in matching)
    return [
        "python configure.py",
        "ninja -j1",
        "build/tools/dtk shasum -q -c config/GP6E01/build.sha1",
        "cmp build/GP6E01/main.dol orig/GP6E01/sys/main.dol  # plus every affected REL",
        *(
            f"# relocation-aware object re-verification required: {owner}"
            for owner in matching
        ),
        audit,
    ]


def evidence_errors(
    matching: Sequence[str], evidence: Mapping[str, Any] | None
) -> list[str]:
    evidence = evidence or {}
    errors: list[str] = []
    if evidence.get("build_gate") != "pass":
        errors.append("serialized build evidence missing: pass --build-gate pass")
    if evidence.get("checksum") != "pass":
        errors.append("DTK checksum evidence missing: pass --checksum pass")
    consumers = evidence.get("consumers") or {}
    for owner in matching:
        if consumers.get(owner) != "exact":
            errors.append(
                f"affected Matching consumer not re-verified: pass --consumer {owner}=exact"
            )
    return errors


def audit_promotion(
    root: Path,
    *,
    base_ref: str,
    head_ref: str,
    source_ref: str | None = None,
    selected_paths: Sequence[str] | None = None,
    evidence: Mapping[str, Any] | None = None,
    require_evidence: bool = False,
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
            errors.append(
                f"changed paths {changed} do not equal declared paths {expected}"
            )
    messages = _run(
        root,
        "git",
        "log",
        "--format=%B%x00",
        f"{base_commit}..{head_commit}",
    ).stdout
    errors.extend(supporting_message_errors(messages))

    source_commit = resolve_ref(root, source_ref) if source_ref else None
    file_results: list[dict[str, Any]] = []
    for path in changed:
        try:
            _normalise_path(path)
        except PromotionError:
            continue
        data = _git_bytes(root, head_commit, path)
        text = data.decode("utf-8", errors="replace")
        errors.extend(
            f"{path}: {finding}" for finding in contamination_findings(path, text)
        )
        head_blob = _blob(root, head_commit, path)
        expected_blob = _blob(root, source_commit, path) if source_commit else None
        if source_commit and head_blob != expected_blob:
            errors.append(
                f"{path}: promoted blob {head_blob} differs from verified "
                f"source blob {expected_blob}"
            )
        file_results.append(
            {
                "path": path,
                "kind": _kind(path),
                "head_blob": head_blob,
                "source_blob": expected_blob,
                "identical_to_source": (
                    head_blob == expected_blob if source_commit else None
                ),
                "sha256": hashlib.sha256(data).hexdigest(),
            }
        )

    fanout = affected_consumers(
        root, [str(item["path"]) for item in file_results if _is_header(str(item["path"]))]
    )
    matching = _matching_consumers(fanout)
    checklist = verification_checklist(
        [str(item["path"]) for item in file_results], matching
    )
    if require_evidence:
        errors.extend(evidence_errors(matching, evidence))
    return {
        "schema_version": 1,
        "base_commit": base_commit,
        "head_commit": head_commit,
        "source_commit": source_commit,
        "changed_paths": changed,
        "files": file_results,
        "affected_consumers": fanout,
        "affected_matching_consumers": matching,
        "verification_checklist": checklist,
        "evidence": dict(evidence or {}),
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
    paths: Sequence[str],
    owner: str | None,
    allow_unverified: bool,
) -> dict[str, Any]:
    policy_errors = [
        *supporting_branch_errors(branch),
        *supporting_message_errors(title),
    ]
    if policy_errors:
        raise PromotionError(
            "promotion metadata rejected:\n- " + "\n- ".join(policy_errors)
        )
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

    _run(
        root,
        "git",
        "worktree",
        "add",
        "-q",
        "-b",
        branch,
        str(worktree),
        plan["base_commit"],
    )
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
                    f"{path}: working-tree blob {hashed} differs from source "
                    f"blob {item['source_blob']}"
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
            raise PromotionError(
                f"staged paths {staged} do not equal declared paths {sorted(selected)}"
            )
        # The clean worktree starts from human ``main`` and does not contain the
        # AI-only shared hooks; the staged-path check above and the structural
        # audit below are the applicable boundary checks for this commit.
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
            raise PromotionError(
                "promotion audit failed:\n- " + "\n- ".join(audit["errors"])
            )
        result = {
            **plan,
            "promotion": {
                "branch": branch,
                "worktree": str(worktree),
                "commit": promotion_commit,
                "title": title,
                "audit": audit,
            },
            "next_steps": audit["verification_checklist"],
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


def _parse_consumers(values: Sequence[str]) -> dict[str, str]:
    consumers: dict[str, str] = {}
    for value in values:
        owner, separator, result = value.partition("=")
        if not separator or not owner or not result:
            raise PromotionError(f"--consumer expects <owner>=<result>, not {value}")
        consumers[owner] = result
    return consumers


def _print_plan(value: Mapping[str, Any]) -> None:
    print(f"base: {value['base_commit']}")
    print(f"verified source: {value['source_commit']}")
    for item in value["files"]:
        diff = item["diff"]
        print(
            f"{item['path']}  [{item['kind']}]  blob={item['source_blob'][:12]}  "
            f"sha256={item['sha256'][:12]}  +{diff['lines_added']}/-{diff['lines_removed']}"
        )
    for header, entry in value["affected_consumers"].items():
        print(f"{header} affected Matching consumers:")
        for owner in entry["matching"]:
            print(f"  - {owner}")
        if not entry["matching"]:
            print("  - none configured Matching")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)

    plan = sub.add_parser("plan", help="plan a supporting-change promotion")
    plan.add_argument("--base", default=DEFAULT_BASE)
    plan.add_argument("--source", required=True)
    plan.add_argument("--owner")
    plan.add_argument("--path", action="append", default=[])
    plan.add_argument("--allow-unverified", action="store_true")
    plan.add_argument("--json", action="store_true")

    create = sub.add_parser("create", help="create a clean project/* branch from main")
    create.add_argument("--base", default=DEFAULT_BASE)
    create.add_argument("--source", required=True)
    create.add_argument("--owner")
    create.add_argument("--path", action="append", default=[])
    create.add_argument("--allow-unverified", action="store_true")
    create.add_argument("--branch", required=True)
    create.add_argument("--worktree", required=True)
    create.add_argument("--title", required=True)

    audit = sub.add_parser("audit", help="audit a clean project/* branch")
    audit.add_argument("--base", default=DEFAULT_BASE)
    audit.add_argument("--head", default="HEAD")
    audit.add_argument("--source")
    audit.add_argument("--path", action="append", default=[])
    audit.add_argument("--build-gate")
    audit.add_argument("--checksum")
    audit.add_argument("--consumer", action="append", default=[])
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
            evidence={
                "build_gate": args.build_gate,
                "checksum": args.checksum,
                "consumers": _parse_consumers(args.consumer),
            },
            require_evidence=True,
        )
        if args.json:
            print(json.dumps(value, indent=2))
        else:
            print(
                "clean supporting promotion: "
                + ("PASS" if value["clean_human_promotion"] else "FAIL")
            )
            for path in value["changed_paths"]:
                print(f"- {path}")
            for error in value["errors"]:
                print(f"ERROR: {error}")
            print("required verification:")
            for step in value["verification_checklist"]:
                print(f"  {step}")
        return 1 if value["errors"] else 0
    except (OSError, PromotionError) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
