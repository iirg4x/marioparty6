#!/usr/bin/env python3
"""Unified entry point for recovery context, queueing, worktrees, and checks."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Mapping, Sequence

if __package__ in {None, ""}:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from tools.agent_queue import (
    QueueError,
    add_queue_parser,
    check_diff_claim,
    queue_health,
    read_queue,
    queue_path,
    run_queue_command,
)
from tools.context_engine import (
    build_context,
    context_token_estimate,
    render_compact_knowledge,
    select_context_knowledge,
)
from tools.hooks import HookError, add_hooks_parser, hook_status, run_hooks_command
from tools.integration_finalize import add_integration_parser, run_integration_command
from tools.knowledge_freshness import (
    FreshnessError,
    all_freshness,
    render_freshness_report,
    validate_freshness,
)
from tools.match_workbench import MatchError, add_match_parser, run_match_command
from tools.owner_catalog import CatalogError, build_catalog, find_owner, write_catalog
from tools.recovery_pass import (
    add_recovery_pass_parser,
    atomic_json,
    file_hash,
    run_recovery_pass,
    serialized_build_lock,
)
from tools.recovery_core import load, quality_findings, root_from
from tools.recovery_data import RecoveryError, validate_data
from tools.recovery_knowledge import (
    build_recovery_index,
    knowledge_audit,
    recovery_report,
    render_knowledge_audit,
    resolve_context_target,
    validate_knowledge,
)
from tools.worktree_manager import (
    WorktreeError,
    add_worktree_parser,
    run_worktree_command,
)

MIN_PYTHON = (3, 10)
DEFAULT_FORBIDDEN = [".github.example", ".vscode.example", "README.example.md"]
REQUIRED_AGENT_FILES = [
    "AGENTS.md",
    "CONTRIBUTING.md",
    "docs/agent_quickstart.md",
    "docs/match_workbench.md",
    "docs/recovery_standard.md",
    "docs/concurrent_agents.md",
    "config/recovery/project.json",
    "config/recovery/compiler_patterns.json",
    "config/recovery/knowledge_freshness.json",
    "tools/agent_queue.py",
    "tools/owner_catalog.py",
    "tools/context_engine.py",
    "tools/local_evidence.py",
    "tools/match_workbench.py",
    "tools/knowledge_freshness.py",
    "tools/integration_finalize.py",
    "tools/worktree_manager.py",
    "tools/hooks.py",
    "tools/knowledge_cards.py",
    ".github/PULL_REQUEST_TEMPLATE.md",
]
GENERATED_PATHS = ["build", "build.ninja", "objdiff.json", "ctx.c"]
PROBE_HISTORY = Path("build/board-autonomy/batch-history.json")
PROBE_SCHEMA_VERSION = 2


@dataclass(frozen=True)
class Check:
    name: str
    status: str
    detail: str


class ProbeError(RecoveryError):
    """Invalid or conflicting probe ledger input."""


def _run(
    command: Sequence[str], *, root: Path, capture: bool = True
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(command), cwd=root, text=True, capture_output=capture, check=False
    )


def _git(root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return _run(["git", *args], root=root)


def _project_list(data: dict[str, Any], key: str, fallback: list[str]) -> list[str]:
    readiness = data["project"].get("agent_readiness", {})
    if not isinstance(readiness, dict):
        return fallback
    value = readiness.get(key, fallback)
    if not isinstance(value, list) or not all(isinstance(item, str) for item in value):
        return fallback
    return value


def _metadata_errors(data: dict[str, Any]) -> list[str]:
    return sorted(
        set(
            [
                *validate_data(data),
                *validate_knowledge(data),
                *validate_freshness(data),
            ]
        )
    )


def _catalog(data: dict[str, Any]) -> dict[str, Any]:
    return build_catalog(data["root"], reviewed=data.get("owners", []))


def doctor_checks(data: dict[str, Any]) -> list[Check]:
    root: Path = data["root"]
    checks: list[Check] = []
    python_ok = sys.version_info >= MIN_PYTHON
    checks.append(
        Check(
            "python",
            "pass" if python_ok else "fail",
            f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro} "
            f"(requires >= {MIN_PYTHON[0]}.{MIN_PYTHON[1]})",
        )
    )

    errors = _metadata_errors(data)
    checks.append(
        Check(
            "recovery metadata",
            "pass" if not errors else "fail",
            (
                f"{len(data['owners'])} reviewed owners, "
                f"{len(data['patterns'])} knowledge cards"
            )
            if not errors
            else "; ".join(errors[:5]),
        )
    )
    try:
        catalog = _catalog(data)
        checks.append(
            Check(
                "owner catalog",
                "pass",
                f"{len(catalog['owners'])} operational owners",
            )
        )
    except CatalogError as exc:
        checks.append(Check("owner catalog", "fail", str(exc)))

    required = _project_list(data, "required_files", REQUIRED_AGENT_FILES)
    missing = [path for path in required if not (root / path).is_file()]
    checks.append(
        Check(
            "agent entrypoints",
            "pass" if not missing else "fail",
            "all required files present"
            if not missing
            else "missing: " + ", ".join(missing),
        )
    )
    forbidden = _project_list(data, "forbidden_paths", DEFAULT_FORBIDDEN)
    present = [path for path in forbidden if (root / path).exists()]
    checks.append(
        Check(
            "template cleanup",
            "pass" if not present else "fail",
            "no inactive template workspaces"
            if not present
            else "remove: " + ", ".join(present),
        )
    )

    git_path = shutil.which("git")
    checks.append(Check("git", "pass" if git_path else "fail", git_path or "not found"))
    if git_path:
        branch = _git(root, "branch", "--show-current")
        branch_name = branch.stdout.strip() if branch.returncode == 0 else "unknown"
        checks.append(
            Check(
                "branch isolation",
                "warn" if branch_name in {"main", "master", ""} else "pass",
                branch_name or "detached HEAD",
            )
        )
        status = _git(root, "status", "--porcelain")
        dirty = [line for line in status.stdout.splitlines() if line.strip()]
        checks.append(
            Check(
                "working tree",
                "pass" if not dirty else "warn",
                "clean" if not dirty else f"{len(dirty)} changed paths",
            )
        )
        tracked: list[str] = []
        for path in GENERATED_PATHS:
            result = _git(root, "ls-files", "--", path)
            tracked.extend(line for line in result.stdout.splitlines() if line)
        checks.append(
            Check(
                "generated files",
                "pass" if not tracked else "fail",
                "generated outputs are untracked"
                if not tracked
                else "tracked: " + ", ".join(sorted(set(tracked))[:8]),
            )
        )
        queue_status, queue_detail = queue_health(root)
        checks.append(Check("claim queue", queue_status, queue_detail))
        hook_values = hook_status(root)
        managed = sum(value == "managed" for value in hook_values.values())
        checks.append(
            Check(
                "local hooks",
                "pass" if managed == len(hook_values) else "warn",
                f"{managed}/{len(hook_values)} managed; run `python tools/agent.py hooks install`",
            )
        )

    ninja_path = shutil.which("ninja")
    checks.append(
        Check(
            "ninja",
            "pass" if ninja_path else "warn",
            ninja_path or "not found; public metadata tools still work",
        )
    )
    main_dol = root / "orig/GP6E01/sys/main.dol"
    checks.append(
        Check(
            "private retail inputs",
            "pass" if main_dol.is_file() else "warn",
            "GP6E01 main.dol present"
            if main_dol.is_file()
            else "not present; retail build and checksum gates are unavailable",
        )
    )
    freshness = all_freshness(data)
    stale = [
        card
        for card, value in freshness.items()
        if value.get("effective_status") != "active"
    ]
    checks.append(
        Check(
            "knowledge freshness",
            "pass" if not stale else "warn",
            "all cards active" if not stale else "review: " + ", ".join(stale[:5]),
        )
    )
    return checks


def _print_checks(checks: list[Check], *, as_json: bool = False) -> None:
    if as_json:
        print(json.dumps([asdict(item) for item in checks], indent=2))
        return
    width = max(len(item.name) for item in checks)
    for item in checks:
        print(f"[{item.status.upper():4}] {item.name:<{width}}  {item.detail}")


def _safe_name(value: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", value).strip("_") or "context"


def _probe_text(value: str | None, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ProbeError(f"{label} is required")
    return value.strip()


def _probe_sha256(value: str, label: str) -> str:
    result = _probe_text(value, label)
    if not re.fullmatch(r"[0-9a-fA-F]{64}", result):
        raise ProbeError(f"{label} must be 64 hexadecimal characters")
    return result.lower()


def _probe_key(owner: str, symbol: str, probe_key: str) -> str:
    return "|".join(
        (
            _probe_text(owner, "owner"),
            _probe_text(symbol, "symbol"),
            _probe_text(probe_key, "probe-key"),
        )
    )


def _probe_history_path(root: Path, history: str | Path | None) -> Path:
    path = Path(history or PROBE_HISTORY).expanduser()
    return (path if path.is_absolute() else root / path).resolve()


def _probe_empty_history() -> dict[str, Any]:
    return {
        "schema_version": PROBE_SCHEMA_VERSION,
        "batches": [],
        "probes": {},
        "result_index": {},
    }


def _probe_result_key(record: Mapping[str, Any]) -> str | None:
    outputs = record.get("outputs")
    strict = outputs.get("strict") if isinstance(outputs, Mapping) else None
    strict_sha = strict.get("sha256") if isinstance(strict, Mapping) else None
    owner = record.get("owner")
    profile = record.get("profile")
    target_sha = record.get("target_sha256")
    if not all(
        isinstance(value, str) and value
        for value in (owner, profile, target_sha, strict_sha)
    ):
        return None
    result = "|".join((owner, profile, target_sha, strict_sha))
    value = outputs.get("value") if isinstance(outputs, Mapping) else None
    value_sha = value.get("sha256") if isinstance(value, Mapping) else None
    if isinstance(value_sha, str) and value_sha:
        result += "|" + value_sha
    return result


def _probe_migrate(value: Any) -> dict[str, Any]:
    if not isinstance(value, Mapping):
        raise ProbeError("probe history root must be an object")
    version = value.get("schema_version", 1)
    if version not in {1, PROBE_SCHEMA_VERSION}:
        raise ProbeError(f"unsupported probe history schema_version: {version}")
    history = dict(value)
    batches = history.get("batches", [])
    if not isinstance(batches, list):
        raise ProbeError("probe history batches must be a list")
    probes = history.get("probes", {})
    if not isinstance(probes, Mapping):
        raise ProbeError("probe history probes must be an object")
    result_index = history.get("result_index", {})
    if not isinstance(result_index, Mapping):
        raise ProbeError("probe history result_index must be an object")
    history["schema_version"] = PROBE_SCHEMA_VERSION
    history["batches"] = batches
    history["probes"] = dict(probes)
    history["result_index"] = dict(result_index)
    for key, record in history["probes"].items():
        if not isinstance(key, str) or not isinstance(record, Mapping):
            continue
        result_key = _probe_result_key(record)
        if result_key:
            history["result_index"].setdefault(result_key, key)
    return history


def _probe_read(path: Path) -> dict[str, Any]:
    if not path.exists():
        return _probe_empty_history()
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ProbeError(
            f"invalid probe history JSON {path}:{exc.lineno}:{exc.colno}: {exc.msg}"
        ) from exc
    return _probe_migrate(value)


def _probe_hash(root: Path, report: str | Path, label: str) -> tuple[str, str]:
    artifact = str(report)
    path = Path(report).expanduser()
    path = path if path.is_absolute() else root / path
    digest = file_hash(path)
    if digest is None:
        raise ProbeError(f"{label} does not exist: {report}")
    return digest, artifact


def _probe_metrics(values: Sequence[str] | None) -> dict[str, str]:
    metrics: dict[str, str] = {}
    for value in values or []:
        if "=" not in value:
            raise ProbeError(f"metric must use key=value: {value}")
        key, metric = value.split("=", 1)
        key = _probe_text(key, "metric key")
        metrics[key] = metric
    return metrics


def probe_lookup(
    root: Path,
    owner: str,
    symbol: str,
    probe_key: str,
    input_key: str,
    *,
    history: str | Path | None = None,
) -> dict[str, Any]:
    root = Path(root)
    key = _probe_key(owner, symbol, probe_key)
    input_value = _probe_text(input_key, "input-key")
    ledger = _probe_read(_probe_history_path(root, history))
    record = ledger["probes"].get(key)
    if isinstance(record, Mapping):
        if record.get("input_key") == input_value:
            return {"status": "known", "record": dict(record)}
        return {
            "status": "conflict",
            "record": dict(record),
            "reason": (
                "probe key already belongs to a different input_key; stop or use a "
                "new evidence-descriptive probe key before compiling"
            ),
        }
    return {"status": "new", "record": None}


def probe_record(
    root: Path,
    owner: str,
    symbol: str,
    probe_key: str,
    input_key: str,
    profile: str,
    toolchain_key: str,
    target_sha256: str,
    candidate_sha256: str,
    strict_report: str | Path,
    status: str,
    reason: str,
    *,
    value_report: str | Path | None = None,
    commit: str | None = None,
    metrics: Mapping[str, Any] | None = None,
    history: str | Path | None = None,
) -> dict[str, Any]:
    root = Path(root)
    owner_value = _probe_text(owner, "owner")
    symbol_value = _probe_text(symbol, "symbol")
    probe_value = _probe_text(probe_key, "probe-key")
    input_value = _probe_text(input_key, "input-key")
    profile_value = _probe_text(profile, "profile")
    toolchain_value = _probe_text(toolchain_key, "toolchain-key")
    target_value = _probe_sha256(target_sha256, "target-sha256")
    candidate_value = _probe_sha256(candidate_sha256, "candidate-sha256")
    status_value = _probe_text(status, "status")
    reason_value = _probe_text(reason, "reason")
    strict_sha, strict_artifact = _probe_hash(
        root, strict_report, "strict-report"
    )
    value_output = None
    if value_report is not None:
        value_sha, value_artifact = _probe_hash(
            root, value_report, "value-report"
        )
        value_output = {"sha256": value_sha, "artifact": value_artifact}
    outputs: dict[str, Any] = {
        "strict": {"sha256": strict_sha, "artifact": strict_artifact}
    }
    if value_output is not None:
        outputs["value"] = value_output
    key = _probe_key(owner_value, symbol_value, probe_value)
    result_key = "|".join((owner_value, profile_value, target_value, strict_sha))
    if value_output is not None:
        result_key += "|" + value_output["sha256"]
    candidate: dict[str, Any] = {
        "owner": owner_value,
        "symbol": symbol_value,
        "probe_key": probe_value,
        "input_key": input_value,
        "profile": profile_value,
        "toolchain_key": toolchain_value,
        "target_sha256": target_value,
        "candidate_sha256": candidate_value,
        "outputs": outputs,
        "status": status_value,
        "duplicate_of": None,
        "metrics": dict(metrics or {}),
        "reason": reason_value,
        "commit": commit,
        "recorded_at": datetime.now(timezone.utc).replace(microsecond=0).isoformat(),
    }
    path = _probe_history_path(root, history)
    lock = Path(str(path) + ".lock")
    try:
        with serialized_build_lock(lock, 8.0):
            ledger = _probe_read(path)
            existing = ledger["probes"].get(key)
            if isinstance(existing, Mapping):
                existing_result = _probe_result_key(existing)
                if (
                    existing.get("owner") == owner_value
                    and existing.get("symbol") == symbol_value
                    and existing.get("probe_key") == probe_value
                    and existing.get("input_key") == input_value
                    and existing.get("profile") == profile_value
                    and existing.get("toolchain_key") == toolchain_value
                    and existing.get("target_sha256") == target_value
                    and existing.get("candidate_sha256") == candidate_value
                    and existing_result == result_key
                ):
                    return {"status": "unchanged", "record": dict(existing)}
                raise ProbeError(
                    f"probe key already records conflicting evidence: {key}"
                )
            duplicate_of = ledger["result_index"].get(result_key)
            if duplicate_of and duplicate_of != key:
                candidate["duplicate_of"] = duplicate_of
                result_status = "duplicate"
            else:
                ledger["result_index"][result_key] = key
                result_status = "recorded"
            ledger["probes"][key] = candidate
            atomic_json(path, ledger)
    except ValueError as exc:
        raise ProbeError(str(exc)) from exc
    return {"status": result_status, "record": candidate}


def _with_operational_context_owner(
    data: dict[str, Any], owner_id: str | None
) -> dict[str, Any]:
    """Add an in-memory, non-semantic owner when only the catalog knows it."""

    if not owner_id or any(
        owner.get("id") == owner_id for owner in data.get("owners", [])
    ):
        return data
    matches = find_owner(_catalog(data), owner_id)
    if len(matches) != 1:
        return data
    record = matches[0]
    configured = str(record.get("configured_status") or "Unknown")
    owner = {
        "id": record["id"],
        "module": record.get("module"),
        "source": record["source"],
        "summary": (
            "Operational-catalog fallback only; no reviewed semantic owner "
            "metadata exists for this source."
        ),
        "compiler": None,
        "tags": ["operational-catalog-fallback"],
        "status": {
            "binary": "exact" if configured == "Matching" else "partial",
            "source_shape": "plausible",
            "semantics": "partial",
            "naming": "partially_semantic",
            "data": "typed_partial",
        },
        "evidence": [
            {
                "kind": "operational_catalog",
                "confidence": "confirmed",
                "accepted": True,
                "summary": f"configure.py status is {configured}",
                "reference": "build/context/owner-catalog.json",
            }
        ],
        "debt": [
            {
                "kind": "owner_metadata",
                "priority": "normal",
                "summary": "Reviewed structured owner metadata has not been authored.",
            }
        ],
        "constraints": [],
        "context": {"reports": []},
    }
    result = dict(data)
    result["owners"] = [*data.get("owners", []), owner]
    return result


def _write_context(data: dict[str, Any], args: argparse.Namespace) -> Path | None:
    context_data = _with_operational_context_owner(data, args.owner)
    text = build_context(
        context_data,
        args.kind,
        args.target,
        owner_id=args.owner,
        budget=args.budget,
        knowledge_limit=args.knowledge_limit,
        symptoms=args.symptom,
        local_evidence=args.local_evidence,
        reports=args.report,
    )
    if args.stdout:
        print(text, end="")
        return None
    root: Path = context_data["root"]
    destination = root / (
        args.output
        or f"build/context/{args.kind}-{_safe_name(args.owner or '')}-{_safe_name(args.target)}.md"
    )
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(text, encoding="utf-8")
    print(
        f"wrote {destination.relative_to(root)} "
        f"({context_token_estimate(text)} estimated tokens)"
    )
    return destination


def _changed_forbidden(root: Path, base: str) -> list[str]:
    process = _git(root, "diff", "--name-only", f"{base}...HEAD")
    if process.returncode:
        raise RecoveryError(process.stderr.strip() or f"git diff failed for {base}")
    disallowed = ("orig/", "build/")
    return [
        path
        for path in process.stdout.splitlines()
        if path in {"build.ninja", "objdiff.json", "ctx.c"}
        or path.startswith(disallowed)
    ]


def public_check(data: dict[str, Any], *, base: str | None) -> int:
    root: Path = data["root"]
    failed = False
    checks = doctor_checks(data)
    _print_checks(checks)
    if any(item.status == "fail" for item in checks):
        failed = True

    for command in (
        [sys.executable, "-m", "compileall", "-q", "tools"],
        [sys.executable, "-m", "unittest", "discover", "-s", "tools/tests", "-v"],
    ):
        print(f"\n$ {' '.join(command)}")
        if _run(command, root=root, capture=False).returncode:
            failed = True

    errors = _metadata_errors(data)
    if errors:
        for error in errors:
            print(f"metadata: {error}")
        failed = True
    else:
        catalog = _catalog(data)
        catalog_path = root / "build/context/owner-catalog.json"
        write_catalog(catalog, catalog_path)
        print(f"catalog: {len(catalog['owners'])} owners")
        database = root / "build/context/recovery.sqlite"
        counts = build_recovery_index(data, database)
        print(
            "index: "
            + ", ".join(
                f"{key}={value}" for key, value in sorted(counts.items())
            )
        )
        report = root / "build/context/recovery-report.md"
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text(
            recovery_report(data).rstrip()
            + "\n\n"
            + render_freshness_report(data)
            + "\n",
            encoding="utf-8",
        )
        model = next(
            (
                item
                for item in data["owners"]
                if "model-owner" in item.get("tags", [])
            ),
            None,
        )
        if model:
            smoke = root / "build/context/agent-smoke-context.md"
            smoke.write_text(
                build_context(data, "owner", str(model["id"]), budget=6000),
                encoding="utf-8",
            )

    if base:
        print(f"\nchanged-line review against {base}")
        diff_check = _git(root, "diff", "--check", f"{base}...HEAD")
        if diff_check.returncode:
            print(diff_check.stdout or diff_check.stderr)
            failed = True
        forbidden = _changed_forbidden(root, base)
        if forbidden:
            print("generated/private paths changed: " + ", ".join(forbidden))
            failed = True
        findings = quality_findings(data, base=base)
        for item in findings:
            print(
                f"{item['path']}:{item['line']}: {item['rule']}: "
                f"{item['message']} ({item['classification']})"
            )
        if any(item["classification"] == "unreviewed" for item in findings):
            failed = True
        try:
            queue = read_queue(queue_path(root))
            if queue.get("tasks"):
                claim = check_diff_claim(root, base=base, require_claim=False)
                if claim.get("task") and claim.get("errors"):
                    print("claim diff invalid:\n- " + "\n- ".join(claim["errors"]))
                    failed = True
        except QueueError as exc:
            print(f"queue: {exc}")
            failed = True

    print(
        "\npublic agent gate: "
        + ("FAILED" if failed else "PASS")
        + "\nprivate DOL/REL and retail checksum gates are separate"
    )
    return 1 if failed else 0


def _print_knowledge(data: dict[str, Any], args: argparse.Namespace) -> int:
    if args.kind == "audit":
        audit = knowledge_audit(data)
        print(
            json.dumps(audit, indent=2)
            if args.json
            else render_knowledge_audit(audit, max_items=args.max_items),
            end="" if not args.json else "\n",
        )
        return 0
    if not args.target:
        raise RecoveryError(f"knowledge {args.kind} requires a target")
    owner, stable = resolve_context_target(
        data, args.kind, args.target, args.owner
    )
    matches = select_context_knowledge(
        data,
        owner,
        stable_identity=stable,
        symptoms=args.symptom,
        limit=args.limit,
    )
    if args.json:
        print(json.dumps([match.as_dict() for match in matches], indent=2))
    else:
        print(render_compact_knowledge(data, matches))
    return 0


def _add_probe_parser(sub: Any) -> None:
    parser = sub.add_parser("probe", help="record/query compiler probe evidence")
    commands = parser.add_subparsers(dest="probe_command", required=True)

    lookup = commands.add_parser("lookup")
    lookup.add_argument("--owner", required=True)
    lookup.add_argument("--symbol", required=True)
    lookup.add_argument("--probe-key", required=True)
    lookup.add_argument("--input-key", required=True)
    lookup.add_argument("--history")
    lookup.add_argument("--json", action="store_true")

    record = commands.add_parser("record")
    for name in (
        "owner",
        "symbol",
        "probe-key",
        "input-key",
        "profile",
        "toolchain-key",
        "target-sha256",
        "candidate-sha256",
        "strict-report",
        "status",
        "reason",
    ):
        record.add_argument(f"--{name}", required=True)
    record.add_argument("--value-report")
    record.add_argument("--commit")
    record.add_argument("--metric", action="append", default=[])
    record.add_argument("--history")
    record.add_argument("--json", action="store_true")


def _print_probe_result(result: Mapping[str, Any], *, as_json: bool) -> None:
    if as_json:
        print(json.dumps(result, indent=2))
        return
    print(result["status"])


def _add_catalog_parser(sub: Any) -> None:
    parser = sub.add_parser(
        "catalog", help="build/query operational owner inventory"
    )
    commands = parser.add_subparsers(dest="catalog_command", required=True)
    build = commands.add_parser("build")
    build.add_argument("--output", default="build/context/owner-catalog.json")
    query = commands.add_parser("query")
    query.add_argument("value")
    query.add_argument("--json", action="store_true")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root")
    sub = parser.add_subparsers(dest="command", required=True)
    doctor = sub.add_parser("doctor")
    doctor.add_argument("--json", action="store_true")
    doctor.add_argument("--strict", action="store_true")
    add_queue_parser(sub)
    add_worktree_parser(sub)
    add_hooks_parser(sub)
    add_integration_parser(sub)
    _add_probe_parser(sub)
    _add_catalog_parser(sub)
    add_recovery_pass_parser(sub)
    add_match_parser(sub)

    context = sub.add_parser("context")
    context.add_argument("kind", choices=["function", "owner"])
    context.add_argument("target")
    context.add_argument("--owner")
    context.add_argument("--budget", type=int, default=12000)
    context.add_argument("--knowledge-limit", type=int)
    context.add_argument("--symptom", action="append", default=[])
    context.add_argument("--local-evidence", action="store_true")
    context.add_argument("--report", action="append", default=[])
    context.add_argument("--output")
    context.add_argument("--stdout", action="store_true")

    knowledge = sub.add_parser("knowledge")
    knowledge.add_argument("kind", choices=["function", "owner", "audit"])
    knowledge.add_argument("target", nargs="?")
    knowledge.add_argument("--owner")
    knowledge.add_argument("--symptom", action="append", default=[])
    knowledge.add_argument("--limit", type=int, default=5)
    knowledge.add_argument("--max-items", type=int, default=20)
    knowledge.add_argument("--json", action="store_true")

    check = sub.add_parser("check")
    check.add_argument("--base")
    report = sub.add_parser("report")
    report.add_argument("--output", default="build/context/recovery-report.md")

    args = parser.parse_args()
    try:
        if args.command == "match":
            if args.root:
                selected = Path(args.root).expanduser().resolve()
                try:
                    root = root_from(selected)
                except RecoveryError:
                    # The standalone workbench is also useful for authenticated
                    # scratch fixtures that intentionally have no project file.
                    root = selected
            else:
                root = root_from()
            return run_match_command(args, root=root)
        root = root_from(args.root)
        if args.command == "probe":
            if args.probe_command == "lookup":
                result = probe_lookup(
                    root,
                    args.owner,
                    args.symbol,
                    args.probe_key,
                    args.input_key,
                    history=args.history,
                )
            else:
                result = probe_record(
                    root,
                    args.owner,
                    args.symbol,
                    args.probe_key,
                    args.input_key,
                    args.profile,
                    args.toolchain_key,
                    args.target_sha256,
                    args.candidate_sha256,
                    args.strict_report,
                    args.status,
                    args.reason,
                    value_report=args.value_report,
                    commit=args.commit,
                    metrics=_probe_metrics(args.metric),
                    history=args.history,
                )
            _print_probe_result(result, as_json=args.json)
            return 0
        data = load(root, validate=False)
        catalog = (
            _catalog(data)
            if args.command in {"queue", "worktree", "catalog"}
            else None
        )
        if args.command == "doctor":
            checks = doctor_checks(data)
            _print_checks(checks, as_json=args.json)
            return (
                1
                if any(
                    item.status == "fail"
                    or (args.strict and item.status == "warn")
                    for item in checks
                )
                else 0
            )
        if args.command == "queue":
            return run_queue_command(
                args,
                root=root,
                owners=catalog["owners"] if catalog else [],
            )
        if args.command == "worktree":
            return run_worktree_command(
                args,
                root=root,
                owners=catalog["owners"] if catalog else [],
            )
        if args.command == "hooks":
            return run_hooks_command(args, root=root)
        if args.command == "integration":
            return run_integration_command(args, root=root)
        if args.command == "pass-report":
            return run_recovery_pass(args, root=root)
        if args.command == "catalog":
            if args.catalog_command == "build":
                destination = root / args.output
                write_catalog(catalog, destination)
                print(
                    f"wrote {destination.relative_to(root)}: "
                    f"{len(catalog['owners'])} owners"
                )
                return 0
            matches = find_owner(catalog, args.value)
            if args.json:
                print(json.dumps(matches, indent=2))
            else:
                for item in matches:
                    print(
                        f"{item['id']}  {item['configured_status']}  "
                        f"{item['source']}  {item['size_bytes']} bytes"
                    )
            return 0 if matches else 1
        if args.command == "context":
            _write_context(data, args)
            return 0
        if args.command == "knowledge":
            return _print_knowledge(data, args)
        if args.command == "check":
            return public_check(data, base=args.base)
        errors = _metadata_errors(data)
        if errors:
            raise RecoveryError(
                "recovery metadata invalid:\n- " + "\n- ".join(errors)
            )
        destination = root / args.output
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(
            recovery_report(data).rstrip()
            + "\n\n"
            + render_freshness_report(data)
            + "\n",
            encoding="utf-8",
        )
        print(f"wrote {destination.relative_to(root)}")
        return 0
    except (
        CatalogError,
        FreshnessError,
        HookError,
        OSError,
        MatchError,
        ProbeError,
        QueueError,
        RecoveryError,
        WorktreeError,
    ) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
