"""Candidate-inbox driver for the autonomous owner-campaign lane.

The driver deliberately stays small.  It does not choose hypotheses, create
Codex tasks, or replace the campaign runtime.  Sol supplies sealed natural-C
candidate descriptors to the campaign inbox; this module discovers at most
five of them and delegates measurement/retention to :mod:`tools.owner_campaign`.

The inbox is a transport boundary, not a second authority boundary.  A
descriptor must pass the same self-digest and campaign binding checks used by
the core loader before it is dispatched.  Terminal descriptors and their
build-root candidate sources are compacted after the core returns.  Inputs for
an infrastructure retry remain intact so the next invocation can retry them.
"""

from __future__ import annotations

import math
import json
import os
from pathlib import Path
import time
from collections import Counter
from typing import Any, Callable, Mapping, Sequence

from . import owner_campaign


INBOX_SCHEMA = "owner_campaign_inbox/v1"
LANE_RESULT_SCHEMA = "owner_campaign_lane_result/v1"
SUPERVISOR_RESULT_SCHEMA = "owner_campaign_supervisor_result/v1"
DEFAULT_BATCH_SIZE = 5
DEFAULT_WATCHDOG_SECONDS = 1800.0
# Keep an empty-inbox supervisor alive for the full watchdog window.  A short
# default idle cutoff would turn ordinary Luna proposal latency into a false
# terminal state and force the Sol parent to restart the lane.
DEFAULT_IDLE_TIMEOUT_SECONDS = DEFAULT_WATCHDOG_SECONDS
DEFAULT_POLL_INTERVAL_SECONDS = 1.0
MAX_BACKOFF_SECONDS = 8.0
DEFAULT_INFRA_RETRY_LIMIT = 3
TERMINAL_STATUSES = frozenset(
    {"deduplicated", "discarded", "exact", "improved", "no_gain"}
)


def inbox_path(root: Path, campaign: Mapping[str, Any]) -> Path:
    """Return the compact per-campaign inbox below ``build/owner-campaign``."""

    return (
        owner_campaign._state_root(Path(root))
        / "inbox"
        / owner_campaign._slug(str(campaign["campaign_id"]))
    )


def _canonical_digest(value: Any) -> str:
    return owner_campaign._digest_json(value)


def _is_hex_sha(value: Any) -> bool:
    return isinstance(value, str) and owner_campaign.SHA_RE.fullmatch(value) is not None


def _path_inside(root: Path, path: Path) -> bool:
    try:
        path.relative_to(root)
        return True
    except ValueError:
        return False


def _relative_path(root: Path, raw: Any) -> Path | None:
    if not isinstance(raw, str) or not raw or "\x00" in raw:
        return None
    candidate = Path(raw)
    if candidate.is_absolute():
        return None
    path = Path(os.path.abspath(root / candidate))
    if not _path_inside(root, path):
        return None
    current = root
    for part in path.relative_to(root).parts:
        current = current / part
        if current.is_symlink():
            return None
    return path


def _under_allowed_build(root: Path, path: Path, campaign: Mapping[str, Any]) -> bool:
    for raw in campaign.get("allowed_build_paths", []):
        allowed = _relative_path(root, raw)
        if allowed is not None and (path == allowed or _path_inside(allowed, path)):
            return True
    return False


def _sealed_descriptor(
    root: Path, campaign: Mapping[str, Any], path: Path
) -> tuple[dict[str, Any], Path] | None:
    """Read only enough to identify a sealed descriptor without consuming it.

    Full candidate validation, including the current frontier binding and
    natural-C source checks, remains in ``owner_campaign.run_candidate``.  A
    malformed or stale inbox file is ignored rather than making an empty lane
    look successful or poisoning a batch of otherwise valid cells.
    """

    try:
        if path.is_symlink():
            return None
        current = root
        for part in path.relative_to(root).parts:
            current = current / part
            if current.is_symlink():
                return None
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError):
        return None
    if not isinstance(value, Mapping):
        return None
    fields = getattr(owner_campaign, "CANDIDATE_FIELDS", frozenset())
    if set(value) != set(fields):
        return None
    body = dict(value)
    digest = body.pop("candidate_sha256", None)
    if not _is_hex_sha(digest) or _canonical_digest(body) != digest:
        return None
    if (
        value.get("schema") != owner_campaign.CANDIDATE_SCHEMA
        or value.get("campaign_id") != campaign.get("campaign_id")
        or value.get("natural_c") is not True
        or not isinstance(value.get("function"), str)
        or value.get("function") not in campaign.get("functions", [])
        or not isinstance(value.get("hypothesis_family"), str)
        or not value.get("hypothesis_family")
        or not _is_hex_sha(value.get("base_frontier_sha256"))
    ):
        return None
    source = value.get("candidate_source")
    if not isinstance(source, Mapping) or set(source) != {"path", "sha256"}:
        return None
    source_path = _relative_path(root, source.get("path"))
    if source_path is None or not _under_allowed_build(root, source_path, campaign):
        return None
    source_sha = source.get("sha256")
    try:
        if (
            not _is_hex_sha(source_sha)
            or not source_path.is_file()
            or owner_campaign._digest_file(source_path) != source_sha
        ):
            return None
    except OSError:
        return None
    return dict(value), source_path


def discover_candidates(
    root: Path, campaign: Mapping[str, Any], *, limit: int = DEFAULT_BATCH_SIZE
) -> list[Path]:
    """Return deterministic, sealed candidate descriptors ready for dispatch."""

    if type(limit) is not int or limit < 1:
        raise ValueError("candidate inbox limit must be a positive integer")
    root = Path(os.path.abspath(root))
    inbox = inbox_path(root, campaign)
    if not inbox.is_dir():
        return []
    found: list[tuple[str, str, Path]] = []
    try:
        entries = sorted(
            inbox.rglob("*.json"),
            key=lambda item: item.relative_to(inbox).as_posix(),
        )
    except OSError:
        return []
    for path in entries:
        if path.name.startswith("."):
            continue
        sealed = _sealed_descriptor(root, campaign, path)
        if sealed is None:
            continue
        descriptor, _source = sealed
        created = descriptor.get("created_at")
        # ISO timestamps sort lexically; malformed timestamps are still kept
        # deterministic after well-formed candidates and are fully validated
        # by the core loader before compile.
        created_key = created if isinstance(created, str) else ""
        found.append((created_key, path.relative_to(inbox).as_posix(), path))
    found.sort(key=lambda item: (item[0], item[1]))
    return [item[2] for item in found[:limit]]


def _source_referenced_by_pending(
    root: Path, campaign: Mapping[str, Any], source_path: Path, *, exclude: Path
) -> bool:
    inbox = inbox_path(root, campaign)
    if not inbox.is_dir():
        return False
    for descriptor_path in inbox.rglob("*.json"):
        if descriptor_path == exclude:
            continue
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        if sealed is not None and sealed[1] == source_path:
            return True
    return False


def _compact_terminal_input(
    root: Path, campaign: Mapping[str, Any], descriptor_path: Path
) -> list[str]:
    """Remove a terminal descriptor and an unshared build-root source."""

    removed: list[str] = []
    try:
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        source_path = sealed[1] if sealed is not None else None
        if descriptor_path.exists():
            descriptor_path.unlink()
            removed.append(descriptor_path.relative_to(root).as_posix())
        if (
            source_path is not None
            and source_path.exists()
            and _under_allowed_build(root, source_path, campaign)
            and not _source_referenced_by_pending(
                root, campaign, source_path, exclude=descriptor_path
            )
        ):
            source_path.unlink()
            removed.append(source_path.relative_to(root).as_posix())
    except (OSError, ValueError) as exc:
        # The measurement result is still authoritative.  Surface cleanup
        # trouble to the caller while leaving an infra retry distinguishable.
        removed.append(f"cleanup-error:{descriptor_path}:{exc}")
    return removed


def _result_status(value: Any) -> str:
    return value.get("status", "unknown") if isinstance(value, Mapping) else "unknown"


def run_inbox(
    root: Path,
    campaign: Mapping[str, Any],
    *,
    max_candidates: int = DEFAULT_BATCH_SIZE,
) -> dict[str, Any]:
    """Dispatch one bounded inbox batch through the core campaign loop."""

    if type(max_candidates) is not int or not 1 <= max_candidates <= DEFAULT_BATCH_SIZE:
        raise ValueError(f"max_candidates must be between 1 and {DEFAULT_BATCH_SIZE}")
    root = Path(os.path.abspath(root))
    descriptors = discover_candidates(root, campaign, limit=max_candidates)
    if not descriptors:
        return {
            "schema": LANE_RESULT_SCHEMA,
            "status": "idle",
            "reason": "candidate_inbox_empty",
            "campaign_id": campaign["campaign_id"],
            "discovered": 0,
            "dispatched": 0,
            "results": [],
            "cleaned": [],
            "preserved_infrastructure": [],
            "authority_advanced": False,
        }

    try:
        results = owner_campaign.run_loop(root, campaign, descriptors)
    except owner_campaign.InfrastructureError as exc:
        # A batch-level infrastructure error must not consume its inputs.
        results = [
            {
                "schema": "owner_campaign_result/v1",
                "status": "infra_retry",
                "candidate": str(path),
                "reason": str(exc)[:1000],
                "authority_advanced": False,
            }
            for path in descriptors
        ]

    cleaned: list[str] = []
    preserved: list[str] = []
    for index, descriptor_path in enumerate(descriptors):
        result = results[index] if index < len(results) else {
            "status": "infra_retry",
            "reason": "core returned fewer results than dispatched",
        }
        status = _result_status(result)
        if status in TERMINAL_STATUSES:
            cleaned.extend(_compact_terminal_input(root, campaign, descriptor_path))
        else:
            preserved.append(descriptor_path.relative_to(root).as_posix())

    statuses = [_result_status(item) for item in results]
    if not statuses:
        status = "infra_retry"
    elif all(item == "infra_retry" for item in statuses):
        status = "infra_retry"
    else:
        status = "processed"
    return {
        "schema": LANE_RESULT_SCHEMA,
        "status": status,
        "campaign_id": campaign["campaign_id"],
        "discovered": len(descriptors),
        "dispatched": len(descriptors),
        "results": list(results),
        "cleaned": cleaned,
        "preserved_infrastructure": preserved,
        "authority_advanced": False,
    }


def _duration(
    value: float | int | None,
    *,
    default: float,
    label: str,
    minimum: float = 0.0,
) -> float:
    selected = default if value is None else value
    if isinstance(selected, bool):
        raise ValueError(f"{label} must be a finite number")
    try:
        result = float(selected)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{label} must be a finite number") from exc
    if not math.isfinite(result) or result < minimum:
        raise ValueError(f"{label} must be at least {minimum:g} seconds")
    return result


def _terminal_result(
    campaign: Mapping[str, Any],
    *,
    status: str,
    reason: str,
    started: float,
    clock: Callable[[], float],
    batches: int,
    dispatched: int,
    outcomes: Counter[str],
    last_batch_status: str | None,
) -> dict[str, Any]:
    elapsed = max(0.0, clock() - started)
    return {
        "schema": SUPERVISOR_RESULT_SCHEMA,
        "status": status,
        "reason": reason,
        "campaign_id": campaign["campaign_id"],
        "batches": batches,
        "dispatched": dispatched,
        "outcomes": dict(sorted(outcomes.items())),
        "last_batch_status": last_batch_status,
        "elapsed_seconds": round(elapsed, 3),
        "authority_advanced": False,
    }


def _campaign_terminal_state(
    root: Path, campaign: Mapping[str, Any]
) -> tuple[str | None, str | None]:
    """Return a terminal campaign state, if one is observable now.

    Cancellation is checked through the core's signed control record. Other
    core failures are infrastructure-terminal for the supervisor: continuing
    would only rediscover the same invalid state in a tight loop.
    """

    try:
        owner_campaign._check_cancelled(root, campaign)
    except owner_campaign.CampaignError as exc:
        if "campaign is cancelled at the active epoch" in str(exc):
            return "cancelled", str(exc)
        return "infrastructure_terminal", str(exc)[:1000]
    try:
        state = owner_campaign.campaign_status(root, campaign)
    except owner_campaign.CampaignError as exc:
        return "infrastructure_terminal", str(exc)[:1000]
    if (
        isinstance(state, Mapping)
        and state.get("exact_count") == state.get("total")
        and isinstance(state.get("total"), int)
        and state["total"] > 0
    ):
        return "closed", "all campaign functions are exact"
    return None, None


def run_supervisor(
    root: Path,
    campaign: Mapping[str, Any],
    *,
    max_candidates: int = DEFAULT_BATCH_SIZE,
    idle_timeout_seconds: float | int | None = DEFAULT_IDLE_TIMEOUT_SECONDS,
    watchdog_seconds: float | int | None = DEFAULT_WATCHDOG_SECONDS,
    poll_interval_seconds: float | int | None = DEFAULT_POLL_INTERVAL_SECONDS,
    infra_retry_limit: int = DEFAULT_INFRA_RETRY_LIMIT,
    clock: Callable[[], float] | None = None,
    sleeper: Callable[[float], None] | None = None,
) -> dict[str, Any]:
    """Keep a Sol-owned campaign live until a bounded terminal state.

    Candidate selection remains outside this module. The supervisor only
    discovers sealed inbox descriptors, dispatches them through the core, and
    waits with bounded exponential backoff when the inbox is empty or an
    infrastructure retry is pending. ``--once`` uses :func:`run_inbox` instead
    for deterministic snapshots and tests.
    """

    if type(max_candidates) is not int or not 1 <= max_candidates <= DEFAULT_BATCH_SIZE:
        raise ValueError(f"max_candidates must be between 1 and {DEFAULT_BATCH_SIZE}")
    if type(infra_retry_limit) is not int or infra_retry_limit < 1:
        raise ValueError("infra_retry_limit must be a positive integer")
    idle_timeout = _duration(
        idle_timeout_seconds,
        default=DEFAULT_IDLE_TIMEOUT_SECONDS,
        label="idle_timeout_seconds",
    )
    watchdog = _duration(
        watchdog_seconds,
        default=DEFAULT_WATCHDOG_SECONDS,
        label="watchdog_seconds",
    )
    poll_interval = _duration(
        poll_interval_seconds,
        default=DEFAULT_POLL_INTERVAL_SECONDS,
        label="poll_interval_seconds",
        minimum=0.01,
    )
    clock = clock or time.monotonic
    sleeper = sleeper or time.sleep
    root = Path(os.path.abspath(root))
    started = clock()
    idle_started = started
    delay = poll_interval
    infra_retries = 0
    batches = 0
    dispatched = 0
    outcomes: Counter[str] = Counter()
    last_batch_status: str | None = None

    while True:
        now = clock()
        if now - started >= watchdog:
            return _terminal_result(
                campaign,
                status="watchdog_timeout",
                reason="supervisor watchdog expired",
                started=started,
                clock=clock,
                batches=batches,
                dispatched=dispatched,
                outcomes=outcomes,
                last_batch_status=last_batch_status,
            )

        terminal, reason = _campaign_terminal_state(root, campaign)
        if terminal is not None:
            return _terminal_result(
                campaign,
                status=terminal,
                reason=reason or terminal,
                started=started,
                clock=clock,
                batches=batches,
                dispatched=dispatched,
                outcomes=outcomes,
                last_batch_status=last_batch_status,
            )

        descriptors = discover_candidates(root, campaign, limit=max_candidates)
        if descriptors:
            try:
                batch = run_inbox(
                    root, campaign, max_candidates=max_candidates
                )
            except owner_campaign.CampaignError as exc:
                return _terminal_result(
                    campaign,
                    status="infrastructure_terminal",
                    reason=str(exc)[:1000],
                    started=started,
                    clock=clock,
                    batches=batches,
                    dispatched=dispatched,
                    outcomes=outcomes,
                    last_batch_status=last_batch_status,
                )
            batches += 1
            dispatched += int(batch.get("dispatched", len(descriptors)))
            batch_results = batch.get("results", [])
            if not isinstance(batch_results, list):
                batch_results = []
            batch_statuses = [_result_status(item) for item in batch_results]
            outcomes.update(batch_statuses)
            last_batch_status = str(batch.get("status", "unknown"))
            infra_only = bool(batch_statuses) and all(
                status in {"infra_retry", "stale_rebase", "stale"}
                for status in batch_statuses
            )
            if last_batch_status == "infra_retry" or infra_only:
                infra_retries += 1
                if infra_retries >= infra_retry_limit:
                    return _terminal_result(
                        campaign,
                        status="infrastructure_terminal",
                        reason="infrastructure retry limit reached",
                        started=started,
                        clock=clock,
                        batches=batches,
                        dispatched=dispatched,
                        outcomes=outcomes,
                        last_batch_status=last_batch_status,
                    )
                # Retryable inputs remain in the inbox. Sleep before looking
                # again so stale/retry descriptors cannot spin the supervisor.
                now = clock()
                remaining = max(0.0, watchdog - (now - started))
                sleeper(min(delay, remaining))
                delay = min(MAX_BACKOFF_SECONDS, max(poll_interval, delay * 2))
            else:
                infra_retries = 0
                idle_started = clock()
                delay = poll_interval
            continue

        now = clock()
        if now - idle_started >= idle_timeout:
            return _terminal_result(
                campaign,
                status="idle_timeout",
                reason="candidate inbox stayed empty until idle timeout",
                started=started,
                clock=clock,
                batches=batches,
                dispatched=dispatched,
                outcomes=outcomes,
                last_batch_status=last_batch_status,
            )
        remaining_idle = max(0.0, idle_timeout - (now - idle_started))
        remaining_watchdog = max(0.0, watchdog - (now - started))
        sleeper(min(delay, remaining_idle, remaining_watchdog))
        delay = min(MAX_BACKOFF_SECONDS, max(poll_interval, delay * 2))


def run_owner_campaign_inbox(
    root: Path, campaign: Mapping[str, Any], *, max_candidates: int = DEFAULT_BATCH_SIZE
) -> dict[str, Any]:
    """Compatibility name for callers that treat the inbox as a lane."""

    return run_inbox(root, campaign, max_candidates=max_candidates)


__all__ = [
    "DEFAULT_BATCH_SIZE",
    "DEFAULT_IDLE_TIMEOUT_SECONDS",
    "DEFAULT_POLL_INTERVAL_SECONDS",
    "DEFAULT_WATCHDOG_SECONDS",
    "INBOX_SCHEMA",
    "LANE_RESULT_SCHEMA",
    "SUPERVISOR_RESULT_SCHEMA",
    "TERMINAL_STATUSES",
    "discover_candidates",
    "inbox_path",
    "run_inbox",
    "run_owner_campaign_inbox",
    "run_supervisor",
]
