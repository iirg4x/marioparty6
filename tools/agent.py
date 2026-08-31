#!/usr/bin/env python3
"""Unified entry point for recovery context, queueing, worktrees, and checks."""

from __future__ import annotations

import argparse
import importlib
import inspect
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
from tools.crack_harness import CrackHarnessError, add_crack_parser, run_crack_command
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
from tools.recovery_memory import (
    RecoveryMemoryError,
    add_memory_parser,
    run_memory_command,
    startup_check,
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
    "tools/candidate_compile_admission.py",
    "tools/recovery_memory.py",
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
        try:
            startup = startup_check(root, sync_reports=False)
            checks.append(
                Check(
                    "central recovery memory",
                    "pass",
                    f"canonical queue/memory; freshness {startup['knowledge_sha256'][:12]}",
                )
            )
        except (OSError, QueueError, RecoveryMemoryError) as exc:
            checks.append(Check("central recovery memory", "fail", str(exc)))
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


class OwnerCampaignCLIError(RecoveryError):
    """The owner-campaign CLI could not load or dispatch its core API."""


def _owner_campaign_root(args: argparse.Namespace) -> Path:
    """Resolve a campaign root without requiring recovery metadata.

    Campaign fixtures and isolated pilot worktrees intentionally do not need a
    project metadata file.  Keep the normal ``root_from`` behaviour when one
    is available, but use the explicit path as a standalone root otherwise.
    """

    if args.root:
        selected = Path(args.root).expanduser().resolve()
        try:
            return root_from(selected)
        except RecoveryError:
            return selected
    return root_from()


def _add_owner_campaign_command_arguments(
    parser: argparse.ArgumentParser, command: str
) -> None:
    """Add the shared owner-scoped campaign options.

    The workflow implementation owns validation and manifest semantics.  The
    front door accepts paths and scope bindings here, then forwards only the
    options understood by the implementation.  This keeps the CLI usable
    while the workflow evolves without introducing a second authority model.
    """

    parser.add_argument("campaign_path", nargs="?", type=Path)
    parser.add_argument(
        "--campaign",
        "--manifest",
        "--campaign-manifest",
        dest="campaign",
        type=Path,
    )
    parser.add_argument("--output", "--campaign-out", dest="output", type=Path)
    parser.add_argument("--draft", type=Path)
    parser.add_argument("--campaign-id")
    parser.add_argument("--owner")
    parser.add_argument("--unit")
    parser.add_argument("--source", "--source-relpath", dest="source_relpath")
    parser.add_argument("--base-commit")
    parser.add_argument("--target-object", type=Path)
    parser.add_argument("--toolchain", type=Path)
    parser.add_argument("--measurement-producer", type=Path)
    parser.add_argument(
        "--function",
        "--functions",
        dest="functions",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--protected-exact-function",
        "--protected-function",
        dest="protected_exact_functions",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--allowed-source-path",
        dest="allowed_source_paths",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--allowed-build-path",
        dest="allowed_build_paths",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--forbidden-construct",
        dest="forbidden_constructs",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--snapshot-command",
        dest="snapshot_command",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--candidate-command",
        dest="candidate_command",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--candidate",
        "--candidate-path",
        dest="candidate_paths",
        action="append",
        default=[],
    )
    parser.add_argument(
        "--final-owner-command",
        dest="final_owner_command",
        action="append",
        default=[],
    )
    parser.add_argument("--state-root", type=Path)
    parser.add_argument("--workers", "--lanes", dest="workers", type=int)
    parser.add_argument("--max-lanes", dest="max_lanes", type=int)
    parser.add_argument("--timeout", "--watchdog-seconds", dest="timeout", type=int)
    parser.add_argument("--idle-timeout", dest="idle_timeout", type=float)
    parser.add_argument("--poll-interval", dest="poll_interval", type=float)
    parser.add_argument("--cancellation-epoch", type=int)
    parser.add_argument("--max-candidates", type=int)
    parser.add_argument("--resume", action="store_true")
    parser.add_argument("--once", action="store_true")
    parser.add_argument("--json", action="store_true")
    parser.set_defaults(owner_campaign_operation=command)


def _add_owner_campaign_commands(parser: argparse.ArgumentParser) -> None:
    commands = parser.add_subparsers(
        dest="owner_campaign_command", required=True
    )
    for name, aliases in (
        ("initialize", ["init"]),
        ("status", []),
        ("run", []),
    ):
        command = commands.add_parser(
            name,
            aliases=aliases,
            help={
                "initialize": "create an owner-scoped campaign manifest",
                "status": "inspect campaign state and recover pending frontiers",
                "run": "run the autonomous owner campaign loop",
            }[name],
        )
        _add_owner_campaign_command_arguments(command, name)


def _add_owner_campaign_parser(
    subparsers: Any, *, crack_parser: argparse.ArgumentParser | None = None
) -> argparse.ArgumentParser:
    """Register owner-campaign commands and the documented ``crack loop``.

    ``owner-campaign`` is the explicit namespace for initialize/status/run.
    ``crack loop`` remains available as the compact lane entry point used by
    the workflow document.  Both dispatch to the same ``tools.owner_campaign``
    API and neither has a permit or global STOP argument.
    """

    parser = subparsers.add_parser(
        "owner-campaign",
        aliases=["campaign", "owner_campaign"],
        help="initialize, inspect, or run an autonomous owner campaign",
    )
    _add_owner_campaign_commands(parser)

    if crack_parser is not None:
        # ``add_crack_parser`` owns the legacy parser.  Attach the v2 loop to
        # its subparser action without changing crack_harness.py's legacy
        # permit commands or public API.
        for action in crack_parser._actions:
            if isinstance(action, argparse._SubParsersAction):
                loop = action.add_parser(
                    "loop",
                    help="run the autonomous owner campaign loop",
                )
                _add_owner_campaign_command_arguments(loop, "run")
                break
    return parser


def _owner_campaign_module() -> Any:
    try:
        return importlib.import_module("tools.owner_campaign")
    except ModuleNotFoundError as exc:
        if exc.name in {"tools.owner_campaign", "owner_campaign"}:
            raise OwnerCampaignCLIError(
                "owner campaign support is unavailable: tools.owner_campaign "
                "is not installed"
            ) from exc
        raise


def _campaign_path(args: argparse.Namespace) -> Path | None:
    value = getattr(args, "campaign", None) or getattr(args, "campaign_path", None)
    return Path(value) if value is not None else None


def _owner_campaign_values(
    args: argparse.Namespace, *, root: Path
) -> dict[str, Any]:
    campaign = _campaign_path(args)
    output = getattr(args, "output", None)
    values: dict[str, Any] = {
        "args": args,
        "options": args,
        "namespace": args,
        "root": root,
        "project_root": root,
        "repo_root": root,
        "workspace": root,
        "workspace_root": root,
        "base": root,
        "campaign": campaign,
        "campaign_path": campaign,
        "manifest": campaign,
        "manifest_path": campaign,
        "campaign_manifest": campaign,
        "campaign_manifest_path": campaign,
        "output": output,
        "output_path": output,
        "draft": getattr(args, "draft", None),
        "draft_path": getattr(args, "draft", None),
        "campaign_id": getattr(args, "campaign_id", None),
        "owner": getattr(args, "owner", None),
        "unit": getattr(args, "unit", None),
        "source": getattr(args, "source_relpath", None),
        "source_relpath": getattr(args, "source_relpath", None),
        "base_commit": getattr(args, "base_commit", None),
        "target_object": getattr(args, "target_object", None),
        "target_object_path": getattr(args, "target_object", None),
        "toolchain": getattr(args, "toolchain", None),
        "toolchain_path": getattr(args, "toolchain", None),
        "measurement_producer": getattr(args, "measurement_producer", None),
        "measurement_producer_path": getattr(args, "measurement_producer", None),
        "functions": getattr(args, "functions", None) or None,
        "protected_exact_functions": (
            getattr(args, "protected_exact_functions", None) or None
        ),
        "protected_functions": (
            getattr(args, "protected_exact_functions", None) or None
        ),
        "allowed_source_paths": (
            getattr(args, "allowed_source_paths", None) or None
        ),
        "allowed_build_paths": (
            getattr(args, "allowed_build_paths", None) or None
        ),
        "forbidden_constructs": (
            getattr(args, "forbidden_constructs", None) or None
        ),
        "snapshot_command": getattr(args, "snapshot_command", None) or None,
        "candidate_command": getattr(args, "candidate_command", None) or None,
        "candidate_paths": [
            Path(item)
            for item in (getattr(args, "candidate_paths", None) or [])
        ] or None,
        "candidates": [
            Path(item)
            for item in (getattr(args, "candidate_paths", None) or [])
        ] or None,
        "final_owner_command": (
            getattr(args, "final_owner_command", None) or None
        ),
        "state_root": getattr(args, "state_root", None),
        "workers": getattr(args, "workers", None),
        "lanes": getattr(args, "workers", None),
        "max_lanes": getattr(args, "max_lanes", None),
        "timeout": getattr(args, "timeout", None),
        "watchdog_seconds": getattr(args, "timeout", None),
        "idle_timeout": getattr(args, "idle_timeout", None),
        "idle_timeout_seconds": getattr(args, "idle_timeout", None),
        "poll_interval": getattr(args, "poll_interval", None),
        "poll_interval_seconds": getattr(args, "poll_interval", None),
        "cancellation_epoch": getattr(args, "cancellation_epoch", None),
        "max_candidates": getattr(args, "max_candidates", None),
        "resume": getattr(args, "resume", False),
        "once": getattr(args, "once", False),
    }
    return values


def _invoke_owner_campaign_callable(
    function: Any,
    args: argparse.Namespace,
    *,
    root: Path,
    overrides: Mapping[str, Any] | None = None,
) -> Any:
    """Call one core API function using only its declared parameters."""

    values = _owner_campaign_values(args, root=root)
    if overrides:
        values.update(overrides)
    try:
        signature = inspect.signature(function)
    except (TypeError, ValueError):
        return function(args, root=root)

    accepts_kwargs = any(
        parameter.kind is inspect.Parameter.VAR_KEYWORD
        for parameter in signature.parameters.values()
    )
    if accepts_kwargs:
        return function(**{key: value for key, value in values.items() if value is not None})

    kwargs: dict[str, Any] = {}
    for name, parameter in signature.parameters.items():
        if parameter.kind in {
            inspect.Parameter.VAR_POSITIONAL,
            inspect.Parameter.VAR_KEYWORD,
        }:
            continue
        value = values.get(name)
        if value is not None:
            kwargs[name] = value
    return function(**kwargs)


def _owner_campaign_callable(module: Any, operation: str) -> Any:
    candidates = {
        "initialize": (
            "initialize_campaign",
            "initialize",
            "create_campaign",
            "init_campaign",
        ),
        "status": (
            "campaign_status",
            "status_campaign",
            "status",
        ),
        "run": (
            "run_campaign",
            "run_owner_campaign",
            "run_loop",
            "run",
            "loop",
        ),
    }[operation]
    for name in candidates:
        function = getattr(module, name, None)
        if callable(function):
            return function
    raise OwnerCampaignCLIError(
        "tools.owner_campaign does not expose a "
        f"{operation} campaign operation"
    )


def _print_owner_campaign_result(value: Any) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, (Mapping, list, tuple)):
        print(json.dumps(value, indent=2, sort_keys=True, default=str))
        if isinstance(value, Mapping):
            status = str(value.get("status", "")).lower()
            return (
                2
                if status in {
                    "failed",
                    "blocked",
                    "error",
                    "cancelled",
                    "canceled",
                    "infrastructure_terminal",
                }
                else 0
            )
        return 0
    if value is not None:
        print(value)
    return 0


def _run_owner_campaign_command(
    args: argparse.Namespace, *, root: Path
) -> int:
    module = _owner_campaign_module()
    operation = getattr(args, "owner_campaign_operation", None) or (
        "run" if getattr(args, "crack_command", None) == "loop" else None
    )
    if operation not in {"initialize", "status", "run"}:
        raise OwnerCampaignCLIError("owner campaign operation is missing")

    if operation == "initialize":
        from tools.owner_campaign_manifest import initialize_campaign

        value = _invoke_owner_campaign_callable(
            initialize_campaign,
            args,
            root=root,
        )
        return _print_owner_campaign_result(value)

    # A normal Sol lane does not pass cells through the manager CLI.  When no
    # explicit descriptor is supplied, consume the compact per-campaign inbox
    # through the lane adapter.  Explicit paths remain useful for tests and
    # migration, but the documented ``crack loop`` path is inbox-driven.
    if operation == "run" and not (
        getattr(args, "candidate_paths", None) or []
    ):
        campaign_path = _campaign_path(args)
        if campaign_path is None:
            raise OwnerCampaignCLIError(
                "owner campaign run requires --campaign"
            )
        loader = getattr(module, "load_campaign", None)
        if not callable(loader):
            raise OwnerCampaignCLIError(
                "tools.owner_campaign does not expose load_campaign"
            )
        campaign = loader(root, campaign_path)
        from tools.owner_campaign_lane import run_inbox
        max_candidates = (
            getattr(args, "max_candidates", None)
            if getattr(args, "max_candidates", None) is not None
            else 5
        )
        if getattr(args, "once", False):
            value = run_inbox(root, campaign, max_candidates=max_candidates)
        else:
            from tools.owner_campaign_lane import run_supervisor

            value = run_supervisor(
                root,
                campaign,
                max_candidates=max_candidates,
                idle_timeout_seconds=getattr(args, "idle_timeout", None),
                watchdog_seconds=getattr(args, "timeout", None),
                poll_interval_seconds=getattr(args, "poll_interval", None),
            )
        return _print_owner_campaign_result(value)

    # Prefer a module-level command adapter when supplied by the workflow.  It
    # can apply richer validation while keeping this front door stable.
    command_runner = getattr(module, "run_owner_campaign_command", None)
    try:
        if callable(command_runner):
            setattr(args, "campaign_command", operation)
            value = _invoke_owner_campaign_callable(command_runner, args, root=root)
        else:
            campaign_path = _campaign_path(args)
            if operation in {"status", "run"}:
                if campaign_path is None:
                    raise OwnerCampaignCLIError(
                        f"owner campaign {operation} requires --campaign"
                    )
                loader = getattr(module, "load_campaign", None)
                if not callable(loader):
                    raise OwnerCampaignCLIError(
                        "tools.owner_campaign does not expose load_campaign"
                    )
                campaign = loader(root, campaign_path)
                function = _owner_campaign_callable(module, operation)
                value = _invoke_owner_campaign_callable(
                    function,
                    args,
                    root=root,
                    overrides={
                        "campaign": campaign,
                        "candidate_paths": [
                            Path(item)
                            for item in (
                                getattr(args, "candidate_paths", None) or []
                            )
                        ],
                    },
                )
            else:
                try:
                    function = _owner_campaign_callable(module, operation)
                except OwnerCampaignCLIError:
                    # A manifest is already the authority boundary.  When the
                    # runtime has no separate constructor, initialize means
                    # validate/load that manifest and return its identity.
                    if campaign_path := _campaign_path(args):
                        loader = getattr(module, "load_campaign", None)
                        if callable(loader):
                            campaign = loader(root, campaign_path)
                            value = {
                                "schema": "owner_campaign_init/v1",
                                "status": "initialized",
                                "campaign_id": campaign["campaign_id"],
                                "manifest_sha256": campaign["manifest_sha256"],
                                "owner": campaign["owner"],
                                "unit": campaign["unit"],
                                "manifest_path": str(campaign_path),
                                "authority_advanced": False,
                            }
                        else:
                            raise
                    else:
                        raise
                else:
                    value = _invoke_owner_campaign_callable(
                        function, args, root=root
                    )
    except (OSError, ValueError, RuntimeError) as exc:
        print(f"error: {exc}")
        return 2
    except TypeError as exc:
        raise OwnerCampaignCLIError(
            f"cannot dispatch {operation} through tools.owner_campaign: {exc}"
        ) from exc
    return _print_owner_campaign_result(value)


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
    add_memory_parser(sub)
    crack_parser = add_crack_parser(sub)
    _add_owner_campaign_parser(sub, crack_parser=crack_parser)

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
        if args.command in {"owner-campaign", "campaign", "owner_campaign"} or (
            args.command == "crack"
            and getattr(args, "crack_command", None) == "loop"
        ):
            return _run_owner_campaign_command(
                args,
                root=_owner_campaign_root(args),
            )
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
        if args.command == "memory":
            return run_memory_command(args, root=root)
        if args.command == "crack":
            return run_crack_command(args, root=root)
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
            startup_check(root, sync_reports=True, strict_reports=True)
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
        CrackHarnessError,
        FreshnessError,
        HookError,
        OSError,
        MatchError,
        ProbeError,
        QueueError,
        RecoveryMemoryError,
        RecoveryError,
        WorktreeError,
    ) as exc:
        print(f"error: {exc}")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
