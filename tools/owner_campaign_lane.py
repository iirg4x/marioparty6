"""Candidate-inbox driver for the autonomous owner-campaign lane.

The driver deliberately stays small.  It does not choose hypotheses, create
Codex tasks, or replace the campaign runtime.  Sol supplies sealed natural-C
candidate descriptors to the campaign inbox; this module discovers a bounded
batch, arbitrates sealed proposals, and delegates measurement/retention to
:mod:`tools.owner_campaign`.

The inbox is a transport boundary, not a second authority boundary.  A
descriptor must pass the same self-digest and campaign binding checks used by
the core loader before it is dispatched.  Proposals carry a selector sidecar
bound to the current frontier and compact proof CAS artifacts.  Terminal
descriptors and their build-root candidate sources are compacted after the
core returns.  Inputs for an infrastructure retry remain intact so the next
invocation can retry them.
"""

from __future__ import annotations

import math
from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
import difflib
import json
import os
from pathlib import Path
import re
import shutil
import tempfile
import threading
import time
from collections import Counter
from contextlib import nullcontext
from typing import Any, Callable, Mapping, Sequence

from . import owner_campaign
from . import owner_campaign_reconstruction
from . import owner_campaign_selector


INBOX_SCHEMA = "owner_campaign_inbox/v1"
LANE_RESULT_SCHEMA = "owner_campaign_lane_result/v1"
SUPERVISOR_RESULT_SCHEMA = "owner_campaign_supervisor_result/v1"
PROPOSAL_RESULT_SCHEMA = "owner_campaign_proposal/v1"
_STATE_LIMIT_FIELDS = frozenset({
    "owner_state_bytes", "scratch_soft_bytes", "scratch_hard_bytes",
    "cell_temporary_bytes", "focus_evidence_bytes", "frontier_bytes",
    "report_bytes", "dedupe_bytes",
})
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
REBASED_STATUS = "rebased"
REBASE_REJECTED_STATUS = "stale_rebase_rejected"
REBASE_TOMBSTONE_SCHEMA = "owner_campaign_rebase_tombstone/v1"
REBASE_TOMBSTONE_FIELDS = {
    "schema", "campaign_id", "function", "status", "old_descriptor_sha256",
    "old_candidate_sha256", "old_frontier_sha256", "new_descriptor",
    "new_descriptor_sha256", "new_candidate_sha256", "rebase_depth",
    "reason", "created_at", "tombstone_sha256",
}

RECONSTRUCTION_RESULT_SCHEMA = "owner_campaign_reconstruction_result/v1"
# The core currently publishes the packet as a content-addressed sidecar.  A
# few transition manifests use a nested reference, so the loader accepts the
# documented names while treating every present-but-malformed reference as a
# hard binding error.  Absence is the only legacy compatibility case.
_RECONSTRUCTION_POINTER_FIELDS = (
    "reconstruction_packet",
    "reconstruction_evidence",
    "reconstruction",
    "reconstruction_packet_sha256",
    "reconstruction_evidence_sha256",
    "reconstruction_sha256",
)


# Windows byte-range locks do not provide a reliable same-process mutex for
# independently opened handles.  Pair the persistent cross-process lock below
# with this process-local guard.  It protects only the final directory link;
# proposal preparation and every compiler/evidence pipeline remain parallel.
_PROPOSAL_PUBLICATION_THREAD_LOCK = threading.Lock()


def inbox_path(root: Path, campaign: Mapping[str, Any]) -> Path:
    """Return the compact per-campaign inbox below ``build/owner-campaign``."""

    return (
        owner_campaign._state_root(Path(root))
        / "inbox"
        / owner_campaign._slug(str(campaign["campaign_id"]))
    )


def _input_path(root: Path, raw: Any, label: str, *, exists: bool = True) -> Path:
    """Resolve a caller-supplied path without accepting indirection."""

    if isinstance(raw, Path):
        value = raw
    elif isinstance(raw, (str, os.PathLike)):
        value = Path(os.fspath(raw))
    else:
        raise owner_campaign.CampaignError(f"{label} is invalid")
    if not str(value) or "\x00" in str(value):
        raise owner_campaign.CampaignError(f"{label} is invalid")
    path = Path(os.path.abspath(value if value.is_absolute() else root / value))
    if not _path_inside(root, path):
        raise owner_campaign.CampaignError(f"{label} escapes the campaign root")
    current = root
    for part in path.relative_to(root).parts:
        current = current / part
        if current.is_symlink():
            raise owner_campaign.CampaignError(
                f"{label} uses symlink indirection: {current}"
            )
    indirection_checker = getattr(owner_campaign, "_path_has_indirection", None)
    if callable(indirection_checker) and indirection_checker(root, path):
        raise owner_campaign.CampaignError(f"{label} uses path indirection")
    if exists and not path.is_file():
        raise owner_campaign.CampaignError(f"{label} is not a file: {path}")
    return path


def _campaign_source_path(root: Path, campaign: Mapping[str, Any]) -> Path:
    source = campaign.get("_source")
    if source is None:
        source = campaign.get("source_relpath")
    return _input_path(root, source, "campaign source")


def _allowed_candidate_path(
    root: Path, campaign: Mapping[str, Any], path: Path
) -> bool:
    allowed: list[Path] = []
    for raw in (
        *campaign.get("allowed_source_paths", []),
        *campaign.get("allowed_build_paths", []),
    ):
        try:
            allowed.append(_input_path(root, raw, "allowed candidate root", exists=False))
        except owner_campaign.CampaignError:
            # A manifest may name a not-yet-created build root.  It is still a
            # valid containment root; path validation above remains strict.
            continue
    return any(path == item or _path_inside(item, path) for item in allowed)


def _stable_file_bytes(
    path: Path, label: str, *, max_bytes: int | None = None,
) -> bytes:
    """Read one stable file snapshot; reject replacement during the read."""

    try:
        before = path.stat()
        if max_bytes is not None and before.st_size > max_bytes:
            raise owner_campaign.CampaignError(
                f"{label} exceeds bounded proposal storage: "
                f"{before.st_size} > {max_bytes} bytes"
            )
        payload = path.read_bytes()
        after = path.stat()
    except OSError as exc:
        raise owner_campaign.CampaignError(f"{label} cannot be read: {path}") from exc
    if (
        before.st_size != after.st_size
        or before.st_mtime_ns != after.st_mtime_ns
        or before.st_ino != after.st_ino
    ):
        raise owner_campaign.CampaignError(f"{label} changed during proposal")
    return payload


def _masked_c_source(text: str) -> str:
    """Blank comments and literals while preserving source offsets/newlines."""

    chars = list(text)
    length = len(chars)
    index = 0
    while index < length:
        if chars[index] == "/" and index + 1 < length and chars[index + 1] == "/":
            chars[index] = chars[index + 1] = " "
            index += 2
            while index < length and chars[index] not in "\r\n":
                chars[index] = " "
                index += 1
            continue
        if chars[index] == "/" and index + 1 < length and chars[index + 1] == "*":
            chars[index] = chars[index + 1] = " "
            index += 2
            while index < length:
                if chars[index] == "*" and index + 1 < length and chars[index + 1] == "/":
                    chars[index] = chars[index + 1] = " "
                    index += 2
                    break
                if chars[index] not in "\r\n":
                    chars[index] = " "
                index += 1
            continue
        if chars[index] in {'"', "'"}:
            quote = chars[index]
            chars[index] = " "
            index += 1
            while index < length:
                token = chars[index]
                escaped = token == "\\"
                if chars[index] not in "\r\n":
                    chars[index] = " "
                index += 1
                if escaped and index < length:
                    if chars[index] not in "\r\n":
                        chars[index] = " "
                    index += 1
                elif token == quote:
                    break
            continue
        index += 1
    return "".join(chars)


def _balanced_close(masked: str, opening: int, left: str, right: str) -> int | None:
    depth = 0
    for index in range(opening, len(masked)):
        token = masked[index]
        if token == left:
            depth += 1
        elif token == right:
            depth -= 1
            if depth == 0:
                return index
            if depth < 0:
                return None
    return None


def _function_span(text: str, function: str, label: str) -> tuple[int, int, bytes]:
    """Find one C function definition and return inclusive source line bounds."""

    masked = _masked_c_source(text)
    pattern = re.compile(r"\b" + re.escape(function) + r"\s*\(")
    matches: list[tuple[int, int]] = []
    for match in pattern.finditer(masked):
        opening = masked.find("(", match.start(), match.end())
        closing = _balanced_close(masked, opening, "(", ")")
        if closing is None:
            continue
        after = closing + 1
        while after < len(masked) and masked[after].isspace():
            after += 1
        if after >= len(masked) or masked[after] != "{":
            continue
        body_close = _balanced_close(masked, after, "{", "}")
        if body_close is None:
            raise owner_campaign.CampaignError(f"{label} has an unterminated body")
        boundary = max(masked.rfind("}", 0, match.start()), masked.rfind(";", 0, match.start()))
        first = match.start() if boundary < 0 else boundary + 1
        while first < match.start() and masked[first].isspace():
            first += 1
        start = text.rfind("\n", 0, first) + 1
        start_line = text.count("\n", 0, start) + 1
        # The closing brace belongs to the function's line even when the line
        # has a trailing newline.  ``splitlines(keepends=True)`` accepts the
        # complete line by this inclusive line number.
        end_line = text.count("\n", 0, body_close) + 1
        matches.append((start_line, end_line))
    if len(matches) != 1:
        if not matches:
            raise owner_campaign.CampaignError(f"{label} definition is not found")
        raise owner_campaign.CampaignError(f"{label} definition is ambiguous")
    start_line, end_line = matches[0]
    lines = text.splitlines(keepends=True)
    span = "".join(lines[start_line - 1:end_line]).encode("utf-8")
    return start_line, end_line, span


def _frontier_for_proposal(
    root: Path, campaign: Mapping[str, Any], function: str
) -> dict[str, Any]:
    """Read and validate the persisted current frontier without measuring."""

    reader = getattr(owner_campaign, "_read_latest_frontier", None)
    if callable(reader) and "_source" in campaign:
        frontier = reader(root, campaign, function)
    else:
        path = (
            owner_campaign._function_root(root, campaign, function)
            / "latest-frontier.json"
        )
        try:
            frontier = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, UnicodeError, json.JSONDecodeError) as exc:
            raise owner_campaign.CampaignError(
                f"current frontier is unavailable for {function}"
            ) from exc
        if not isinstance(frontier, Mapping):
            raise owner_campaign.CampaignError("current frontier is invalid")
        frontier = dict(frontier)
        digest = frontier.pop("frontier_sha256", None)
        if not _is_hex_sha(digest) or _canonical_digest(frontier) != digest:
            raise owner_campaign.CampaignError("current frontier digest is invalid")
        frontier["frontier_sha256"] = digest
    if not isinstance(frontier, Mapping):
        raise owner_campaign.CampaignError("current frontier is unavailable")
    result = dict(frontier)
    if result.get("function") != function or not _is_hex_sha(result.get("frontier_sha256")):
        raise owner_campaign.CampaignError("current frontier function binding is invalid")
    if not _is_hex_sha(result.get("source_sha256")):
        raise owner_campaign.CampaignError("current frontier source binding is invalid")
    return result


def _proposal_descriptor_matches(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    frontier_sha256: str,
    candidate_sha256: str,
) -> bool:
    inbox = inbox_path(root, campaign)
    if not inbox.is_dir():
        return False
    for descriptor_path in inbox.rglob("*.json"):
        if descriptor_path.name.startswith("."):
            continue
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        if sealed is None:
            continue
        descriptor, source_path = sealed
        if (
            descriptor.get("function") == function
            and descriptor.get("base_frontier_sha256") == frontier_sha256
            and descriptor.get("candidate_source", {}).get("sha256") == candidate_sha256
        ):
            return True
    dedupe_reader = getattr(owner_campaign, "_dedupe_records", None)
    dedupe_path = getattr(owner_campaign, "_dedupe_path", None)
    if callable(dedupe_reader) and callable(dedupe_path) and "_source" in campaign:
        ledger = dedupe_path(root, campaign, function)
        if ledger.is_file():
            for record in dedupe_reader(ledger):
                if (
                    record.get("base_frontier_sha256") == frontier_sha256
                    and record.get("candidate_source_sha256") == candidate_sha256
                ):
                    return True
    return False


def _read_json_artifact(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise owner_campaign.CampaignError(f"{label} is not readable JSON") from exc
    if not isinstance(value, Mapping):
        raise owner_campaign.CampaignError(f"{label} is not a JSON object")
    return dict(value)


def _reconstruction_pointer(
    frontier: Mapping[str, Any],
) -> tuple[Any, str, str] | None:
    """Return a frontier reconstruction pointer in a normalized form.

    The packet itself is stored in the reconstruction CAS; the frontier only
    carries a path/hash pointer.  ``sha256``/``file_sha256`` identify the CAS
    file, while ``packet_sha256`` identifies the packet's internal seal.  The
    latter is accepted for manifests produced during the v2 migration, but a
    file hash is still checked whenever one is supplied.
    """

    for field in _RECONSTRUCTION_POINTER_FIELDS:
        if field not in frontier or frontier[field] is None:
            continue
        raw = frontier[field]
        if isinstance(raw, str):
            if not _is_hex_sha(raw):
                raise owner_campaign.CampaignError(
                    "frontier reconstruction pointer hash is invalid"
                )
            # A scalar frontier field is the packet's content-addressed
            # digest (the core publishes ``reconstruction_evidence_sha256``
            # this way).  The packet self-hash is checked after loading.  A
            # file digest is supported only by the explicit nested pointer
            # form, where its role is unambiguous.
            return None, raw, "packet"
        if not isinstance(raw, Mapping):
            raise owner_campaign.CampaignError(
                "frontier reconstruction pointer is invalid"
            )
        path = raw.get("path", raw.get("artifact_path", raw.get("packet_path")))
        file_sha = raw.get("file_sha256", raw.get("sha256", raw.get("artifact_sha256")))
        packet_sha = raw.get("packet_sha256")
        if path is not None and (
            not isinstance(path, str) or not path or "\x00" in path
        ):
            raise owner_campaign.CampaignError(
                "frontier reconstruction pointer path is invalid"
            )
        if file_sha is not None:
            if not _is_hex_sha(file_sha):
                raise owner_campaign.CampaignError(
                    "frontier reconstruction file hash is invalid"
                )
            digest = str(file_sha)
            hash_kind = "file"
        elif packet_sha is not None:
            if not _is_hex_sha(packet_sha):
                raise owner_campaign.CampaignError(
                    "frontier reconstruction packet hash is invalid"
                )
            digest = str(packet_sha)
            hash_kind = "packet"
        else:
            raise owner_campaign.CampaignError(
                "frontier reconstruction pointer hash is missing"
            )
        if packet_sha is not None and not _is_hex_sha(packet_sha):
            raise owner_campaign.CampaignError(
                "frontier reconstruction packet hash is invalid"
            )
        # Preserve both hashes in a compact tuple by encoding the optional
        # packet hash as the third value.  A file pointer has no second seal.
        return path, digest, hash_kind if packet_sha is None else f"{hash_kind}:{packet_sha}"
    return None


def _reconstruction_cas_path(root: Path, digest: str) -> Path:
    state_root_reader = getattr(owner_campaign, "_state_root", None)
    if not callable(state_root_reader):
        raise owner_campaign.CampaignError("campaign state root is unavailable")
    return (
        state_root_reader(root)
        / "proof-cas"
        / "reconstruction"
        / digest[:2]
        / f"{digest}.json"
    )


def _reconstruction_identity(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    frontier: Mapping[str, Any],
    packet: Mapping[str, Any],
) -> None:
    """Reject a validly sealed packet that belongs to another frontier."""

    expected = {
        "owner": frontier.get("owner", campaign.get("owner")),
        "unit": frontier.get("unit", campaign.get("unit")),
        "function": function,
        "source_path": frontier.get("source_relpath", campaign.get("source_relpath")),
        "source_sha256": frontier.get("source_sha256"),
        "base_commit": campaign.get("base_commit"),
        "target_object_sha256": frontier.get("target_object_sha256"),
        "candidate_object_sha256": frontier.get("candidate_object_sha256"),
        "toolchain_sha256": frontier.get("toolchain_sha256"),
        "frontier_source_sha256": frontier.get("source_sha256"),
    }
    for key, value in expected.items():
        if value is None:
            continue
        if packet.get(key) != value:
            raise owner_campaign.CampaignError(
                f"frontier reconstruction {key} binding drift"
            )
    pointer_frontier = frontier.get("reconstruction_packet_frontier_sha256")
    if pointer_frontier is None:
        pointer_frontier = frontier.get("reconstruction_frontier_sha256")
    if pointer_frontier is not None and pointer_frontier != frontier.get("frontier_sha256"):
        raise owner_campaign.CampaignError(
            "frontier reconstruction frontier binding drift"
        )
    frontier_status = frontier.get("reconstruction_status")
    packet_status = packet.get(
        "status",
        packet.get("target_first_signal", {}).get("status")
        if isinstance(packet.get("target_first_signal"), Mapping) else None,
    )
    if frontier_status is not None and packet_status != frontier_status:
        raise owner_campaign.CampaignError(
            "frontier reconstruction status binding drift"
        )


def _load_reconstruction_for_frontier(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    frontier: Mapping[str, Any],
) -> dict[str, Any] | None:
    """Load and verify the current frontier's target-first packet.

    ``None`` means an old frontier with no packet pointer and is intentionally
    the sole legacy compatibility path.  Once a pointer is present, every
    hash, packet identity, and status gate is strict.
    """

    pointer = _reconstruction_pointer(frontier)
    if pointer is None:
        return None
    path_raw, digest, digest_kind = pointer
    packet_digest: str | None = None
    if ":" in digest_kind:
        _kind, packet_digest = digest_kind.split(":", 1)
        digest_kind = _kind
    if path_raw is None:
        path = _reconstruction_cas_path(root, digest)
    else:
        path = _input_path(root, path_raw, "frontier reconstruction artifact")
    if not path.is_file():
        raise owner_campaign.CampaignError(
            "frontier reconstruction artifact is missing"
        )
    try:
        file_sha = owner_campaign._digest_file(path)
    except OSError as exc:
        raise owner_campaign.CampaignError(
            "frontier reconstruction artifact cannot be hashed"
        ) from exc
    if digest_kind == "file" and file_sha != digest:
        raise owner_campaign.CampaignError(
            "frontier reconstruction artifact hash drift"
        )
    packet = _read_json_artifact(path, "frontier reconstruction artifact")
    try:
        owner_campaign_reconstruction.verify_packet(packet)
    except owner_campaign_reconstruction.ReconstructionPacketError as exc:
        raise owner_campaign.CampaignError(
            f"frontier reconstruction packet is invalid: {exc}"
        ) from exc
    if packet_digest is not None and packet.get("packet_sha256") != packet_digest:
        raise owner_campaign.CampaignError(
            "frontier reconstruction packet hash drift"
        )
    if digest_kind == "packet" and packet.get("packet_sha256") != digest:
        raise owner_campaign.CampaignError(
            "frontier reconstruction packet hash drift"
        )
    signal = packet.get("target_first_signal")
    if not isinstance(signal, Mapping):
        raise owner_campaign.CampaignError(
            "frontier reconstruction target signal is missing"
        )
    status = packet.get("status", signal.get("status"))
    if status not in {"READY", "UNKNOWN"}:
        raise owner_campaign.CampaignError(
            "frontier reconstruction packet status is invalid"
        )
    # The packet builder publishes the status at both the packet and signal
    # levels.  Accepting the signal as the compatibility source keeps older
    # packet-bearing frontiers readable while still rejecting disagreement.
    signal_status = signal.get("status")
    if signal_status != status:
        raise owner_campaign.CampaignError(
            "frontier reconstruction target signal drift"
        )
    if frontier.get("reconstruction_status") is not None:
        packet_status = packet.get("status", status)
        if packet_status != frontier.get("reconstruction_status"):
            raise owner_campaign.CampaignError(
                "frontier reconstruction status binding drift"
            )
    _reconstruction_identity(root, campaign, function, frontier, packet)
    exact_possible = packet.get("exact_terminal_possible")
    if exact_possible is None:
        exact_possible = signal.get("exact_terminal_possible")
    if type(exact_possible) is not bool:
        raise owner_campaign.CampaignError(
            "frontier reconstruction exact-terminal status is invalid"
        )
    clusters = packet.get("causal_clusters")
    if not isinstance(clusters, list):
        raise owner_campaign.CampaignError(
            "frontier reconstruction causal clusters are missing"
        )
    ownership = packet.get("ownership_complete")
    if ownership is None:
        ownership = status == "READY"
    if type(ownership) is not bool:
        raise owner_campaign.CampaignError(
            "frontier reconstruction ownership status is invalid"
        )
    action = packet.get("next_action", signal.get("next_action"))
    if action is None:
        action = (
            "CRACK" if status == "READY"
            else "DECOMPOSE" if any(
                isinstance(packet.get(key), list)
                for key in ("bounded_regions", "target_regions", "decomposition_regions")
            ) else "PIVOT"
        )
    if action not in {"CRACK", "DECOMPOSE", "PIVOT"}:
        raise owner_campaign.CampaignError(
            "frontier reconstruction next action is invalid"
        )
    bounded_regions: list[dict[str, Any]] = []
    for key in (
        "bounded_regions", "target_regions", "decomposition_regions", "regions",
    ):
        raw_regions = packet.get(key)
        if raw_regions is None:
            raw_regions = signal.get(key)
        if raw_regions is None:
            continue
        if not isinstance(raw_regions, list):
            raise owner_campaign.CampaignError(
                "frontier reconstruction bounded regions are invalid"
            )
        for region in raw_regions:
            if not isinstance(region, Mapping):
                raise owner_campaign.CampaignError(
                    "frontier reconstruction bounded region is invalid"
                )
            bounded_regions.append(dict(region))
        break
    if action == "DECOMPOSE" and not bounded_regions:
        raise owner_campaign.CampaignError(
            "frontier reconstruction decomposition has no bounded regions"
        )
    return {
        "path": path,
        "file_sha256": file_sha,
        "packet_sha256": packet["packet_sha256"],
        "packet": packet,
        "status": status,
        "signal": dict(signal),
        "causal_clusters": [dict(cluster) for cluster in clusters],
        "cluster_count": len(clusters),
        "residual_event_count": packet.get("residual_event_count"),
        "exact_terminal_possible": exact_possible,
        "exact_terminal_reason": packet.get("exact_terminal_reason"),
        "ownership_complete": ownership,
        "next_action": action,
        "bounded_regions": bounded_regions,
    }


def _reconstruction_cluster_for_rows(
    reconstruction: Mapping[str, Any], predicted_rows: Sequence[str]
) -> Mapping[str, Any]:
    """Require one closed packet causal cluster for a source proposal.

    Repeated target regions may be marked with the same ``mirror_group`` by
    the reconstruction producer.  Such a group is one atomic source pattern:
    selecting only one occurrence would create a misleading positive result,
    so every row in every member cluster must be predicted together.
    """

    predicted = set(predicted_rows)
    if not predicted:
        raise owner_campaign.CampaignError(
            "reconstruction proposals require predicted rows"
        )

    def cluster_rows(cluster: Mapping[str, Any]) -> set[str]:
        ids: set[str] = set()
        for key in (
            "row_ids",
            "strict_row_ids",
            "data_row_ids",
            "physical_difference_ids",
            "residual_row_ids",
        ):
            values = cluster.get(key)
            if isinstance(values, Mapping):
                for nested in values.values():
                    if isinstance(nested, list):
                        ids.update(item for item in nested if isinstance(item, str))
            elif isinstance(values, list):
                ids.update(item for item in values if isinstance(item, str))
        return ids

    def cluster_group(cluster: Mapping[str, Any]) -> str:
        raw_group = cluster.get("mirror_group", cluster.get("mirror_group_id"))
        if isinstance(raw_group, Mapping):
            raw_group = raw_group.get("id", raw_group.get("group_id"))
        return str(raw_group) if raw_group is not None else ""

    clusters: list[Mapping[str, Any]] = []
    for cluster in reconstruction.get("causal_clusters", []):
        if not isinstance(cluster, Mapping):
            raise owner_campaign.CampaignError(
                "frontier reconstruction causal cluster is invalid"
            )
        clusters.append(cluster)
    matches = [
        (cluster, cluster_rows(cluster))
        for cluster in clusters
        if cluster_rows(cluster) and predicted & cluster_rows(cluster)
    ]
    if not matches:
        raise owner_campaign.CampaignError("predicted rows cross causal clusters")
    groups: dict[str, list[tuple[Mapping[str, Any], set[str]]]] = {}
    for cluster, ids in matches:
        groups.setdefault(cluster_group(cluster), []).append((cluster, ids))
    if len(groups) > 1:
        raise owner_campaign.CampaignError(
            "predicted rows overlap multiple causal clusters"
        )
    group_key, selected = next(iter(groups.items()))
    if not group_key:
        if len(selected) != 1 or not predicted <= selected[0][1]:
            raise owner_campaign.CampaignError("predicted rows cross causal clusters")
        return selected[0][0]

    group_members = [
        (cluster, cluster_rows(cluster))
        for cluster in clusters
        if cluster_group(cluster) == group_key
    ]
    required_rows: set[str] = set()
    for _cluster, ids in group_members:
        required_rows.update(ids)
    if not required_rows <= predicted:
        raise owner_campaign.CampaignError(
            "predicted rows omit a mirrored causal occurrence"
        )
    return {
        "cluster_id": selected[0][0].get("cluster_id"),
        "mirror_group": group_key,
        "cluster_ids": [cluster.get("cluster_id") for cluster, _ids in group_members],
        "strict_row_ids": sorted(
            row for cluster, _ids in group_members
            for row in cluster.get("strict_row_ids", [])
            if isinstance(row, str)
        ),
        "data_row_ids": sorted(
            row for cluster, _ids in group_members
            for row in cluster.get("data_row_ids", [])
            if isinstance(row, str)
        ),
    }


def _reconstruction_region_for_cluster(
    reconstruction: Mapping[str, Any],
    cluster: Mapping[str, Any],
) -> Mapping[str, Any]:
    """Bind a decomposable prediction to one sealed target region.

    ``owner_campaign_reconstruction`` currently emits causal clusters.  Newer
    producers may additionally attach a bounded target region to an UNKNOWN
    packet so the lane can make one improved-only attempt instead of dropping
    a broad function forever.  The region is accepted only when it names the
    selected cluster (or its row IDs); a generic "decompose this function"
    marker is not enough to authorize a proposal.
    """

    cluster_id = cluster.get("cluster_id")
    mirror_group = cluster.get("mirror_group")
    member_cluster_ids = set(
        item for item in cluster.get("cluster_ids", []) if isinstance(item, str)
    )
    cluster_ids: set[str] = set()
    for key in (
        "row_ids", "strict_row_ids", "data_row_ids",
        "physical_difference_ids", "residual_row_ids",
    ):
        values = cluster.get(key)
        if isinstance(values, Mapping):
            for nested in values.values():
                if isinstance(nested, list):
                    cluster_ids.update(item for item in nested if isinstance(item, str))
        elif isinstance(values, list):
            cluster_ids.update(item for item in values if isinstance(item, str))
    for region in reconstruction.get("bounded_regions", []):
        if not isinstance(region, Mapping):
            continue
        if region.get("closed") is False or region.get("complete") is False:
            continue
        named = region.get("cluster_id", region.get("causal_cluster_id"))
        if cluster_id is not None and named == cluster_id:
            return region
        region_group = region.get("mirror_group", region.get("mirror_group_id"))
        if isinstance(region_group, Mapping):
            region_group = region_group.get("id", region_group.get("group_id"))
        if mirror_group is not None and region_group == mirror_group:
            return region
        region_cluster_ids = region.get("cluster_ids")
        if isinstance(region_cluster_ids, list) and member_cluster_ids <= set(
            item for item in region_cluster_ids if isinstance(item, str)
        ):
            return region
        region_ids: set[str] = set()
        for key in (
            "row_ids", "strict_row_ids", "data_row_ids",
            "physical_difference_ids", "residual_row_ids",
        ):
            values = region.get(key)
            if isinstance(values, Mapping):
                for nested in values.values():
                    if isinstance(nested, list):
                        region_ids.update(item for item in nested if isinstance(item, str))
            elif isinstance(values, list):
                region_ids.update(item for item in values if isinstance(item, str))
        if cluster_ids and cluster_ids <= region_ids:
            return region
    raise owner_campaign.CampaignError(
        "reconstruction decomposition region does not bind predicted cluster"
    )


def reconstruct_frontier(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    *,
    snapshotter: Callable[..., Mapping[str, Any]] | None = None,
) -> dict[str, Any]:
    """Read the current frontier and summarize its target-first packet.

    ``snapshot_frontier`` is deliberately called without ``force``.  The core
    returns an existing frontier without running measurement, so this command
    is a cheap context operation even for a live campaign.  A packet pointer is
    strict: once present, its CAS file, self-digest, identity, status, and
    target signal must all verify.  A frontier with no pointer is reported as
    ``LEGACY`` for the old proposal path and never pretends that ownership was
    proven.
    """

    root = Path(os.path.abspath(root))
    if not isinstance(function, str) or not function:
        raise owner_campaign.CampaignError("function is invalid")
    functions = campaign.get("functions", [])
    if function not in functions:
        raise owner_campaign.CampaignError(
            f"function is outside campaign scope: {function}"
        )
    if snapshotter is None:
        snapshotter = getattr(owner_campaign, "snapshot_frontier", None)
    if not callable(snapshotter):
        raise owner_campaign.CampaignError(
            "tools.owner_campaign does not expose snapshot_frontier"
        )
    current = snapshotter(root, campaign, function)
    if not isinstance(current, Mapping):
        raise owner_campaign.CampaignError("current frontier is invalid")
    current = dict(current)
    if current.get("function") != function:
        raise owner_campaign.CampaignError("current frontier function binding is invalid")
    frontier_sha = current.get("frontier_sha256")
    if not _is_hex_sha(frontier_sha):
        raise owner_campaign.CampaignError("current frontier hash is invalid")
    body = dict(current)
    body.pop("frontier_sha256", None)
    if _canonical_digest(body) != frontier_sha:
        raise owner_campaign.CampaignError("current frontier digest is invalid")

    # Prefer the persisted frontier when the snapshotter wrote one.  This
    # catches a writer race and ensures the packet pointer is read from the
    # authoritative CAS-bound record, while still supporting pure fixtures
    # that return an in-memory frontier without creating state files.
    persisted_path = (
        owner_campaign._function_root(root, campaign, function)
        / "latest-frontier.json"
    )
    if persisted_path.is_file():
        persisted = _frontier_for_proposal(root, campaign, function)
        if persisted["frontier_sha256"] != frontier_sha:
            raise owner_campaign.CampaignError("current frontier changed during reconstruction")
        current = persisted

    packet = _load_reconstruction_for_frontier(root, campaign, function, current)
    result: dict[str, Any] = {
        "schema": RECONSTRUCTION_RESULT_SCHEMA,
        "campaign_id": current.get("campaign_id", campaign.get("campaign_id")),
        "owner": current.get("owner", campaign.get("owner")),
        "unit": current.get("unit", campaign.get("unit")),
        "function": function,
        "frontier_sha256": current["frontier_sha256"],
        "frontier_source_sha256": current.get("source_sha256"),
        "authority_advanced": False,
    }
    if packet is None:
        result.update(
            {
                "status": "LEGACY",
                "packet_path": None,
                "artifact_sha256": None,
                "packet_sha256": None,
                "signal": {
                    "status": "LEGACY",
                    "reason": "frontier has no reconstruction CAS pointer",
                },
                "causal_cluster_count": None,
                "residual_event_count": None,
                "exact_terminal_possible": None,
                "exact_terminal_reason": None,
                "next_action": "LEGACY",
                "bounded_regions": [],
                "ownership_complete": None,
            }
        )
        return result
    result.update(
        {
            "status": packet["status"],
            "packet_path": packet["path"].relative_to(root).as_posix(),
            "artifact_sha256": packet["file_sha256"],
            "packet_sha256": packet["packet_sha256"],
            "signal": packet["signal"],
            "causal_cluster_count": packet["cluster_count"],
            "residual_event_count": packet["residual_event_count"],
            "exact_terminal_possible": packet["exact_terminal_possible"],
            "exact_terminal_reason": packet["exact_terminal_reason"],
            "next_action": packet["next_action"],
            "bounded_regions": packet["bounded_regions"],
            "ownership_complete": packet["ownership_complete"],
        }
    )
    return result


def _focus_artifact_for_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    frontier: Mapping[str, Any],
) -> tuple[Path, str, dict[str, Any]]:
    """Load the exact focus CAS object named by the current frontier."""

    digest = frontier.get("focus_evidence_sha256")
    if not _is_hex_sha(digest):
        raise owner_campaign.CampaignError("current frontier focus binding is missing")
    state_root_reader = getattr(owner_campaign, "_state_root", None)
    if not callable(state_root_reader):
        raise owner_campaign.CampaignError("campaign state root is unavailable")
    path = (
        state_root_reader(root) / "proof-cas" / "focus" / digest[:2]
        / f"{digest}.json"
    )
    path = _input_path(root, path, "frontier focus artifact")
    try:
        actual = owner_campaign._digest_file(path)
    except OSError as exc:
        raise owner_campaign.CampaignError("frontier focus artifact cannot be hashed") from exc
    value = _read_json_artifact(path, "frontier focus artifact")
    validator = getattr(owner_campaign, "_validate_focus_evidence", None)
    if callable(validator) and "_source" in campaign and "limits" in campaign:
        try:
            value = dict(validator(value, campaign, function, frontier["source_sha256"]))
        except owner_campaign.CampaignError:
            raise
    else:
        internal = value.get("focus_evidence_sha256")
        if not _is_hex_sha(internal):
            raise owner_campaign.CampaignError("frontier focus self-digest is missing")
        body = dict(value)
        body.pop("focus_evidence_sha256", None)
        if _canonical_digest(body) != internal or internal != digest:
            raise owner_campaign.CampaignError("frontier focus self-digest is invalid")
    internal = value.get("focus_evidence_sha256")
    if internal != digest:
        raise owner_campaign.CampaignError("frontier focus binding drift")
    for key, expected in (
        ("function", function),
        ("source_sha256", frontier.get("source_sha256")),
        ("unit", frontier.get("unit", campaign.get("unit"))),
    ):
        if key in value and value[key] != expected:
            raise owner_campaign.CampaignError(f"frontier focus {key} binding drift")
    return path, actual, value


def _physical_cas_for_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    frontier: Mapping[str, Any],
    focus_path: Path,
    focus_sha256: str,
    focus: Mapping[str, Any],
) -> tuple[Path, str, dict[str, Any]]:
    """Materialize a compact physical CAS projection from the focus CAS.

    The campaign measurement keeps relocation detail inside its authenticated
    focus object.  The selector contract has a separate physical reference, so
    this projection gives that channel a durable, independently hash-bound
    object without retaining the large raw report.
    """

    state_root_reader = getattr(owner_campaign, "_state_root", None)
    if not callable(state_root_reader):
        raise owner_campaign.CampaignError("campaign state root is unavailable")
    physical_ids = focus.get("physical_difference_ids")
    if not isinstance(physical_ids, list):
        raise owner_campaign.CampaignError("frontier focus physical rows are missing")
    report_receipts = frontier.get("report_receipts")
    physical_receipt = (
        report_receipts.get("physical")
        if isinstance(report_receipts, Mapping) else None
    )
    if not _is_hex_sha(physical_receipt):
        raise owner_campaign.CampaignError("frontier physical receipt is missing")
    body: dict[str, Any] = {
        "schema": "owner_campaign_physical_summary/v1",
        "campaign_id": frontier.get("campaign_id", campaign.get("campaign_id")),
        "owner": frontier.get("owner", campaign.get("owner")),
        "unit": frontier.get("unit", campaign.get("unit")),
        "function": frontier.get("function"),
        "source_sha256": frontier.get("source_sha256"),
        "target_object_sha256": frontier.get("target_object_sha256"),
        "focus_artifact_sha256": focus_sha256,
        "physical_receipt_sha256": physical_receipt,
        "physical_target_identity_sha256": focus.get("physical_target_identity_sha256"),
        "physical_candidate_identity_sha256": focus.get("physical_candidate_identity_sha256"),
        "physical_target_count": focus.get("physical_target_count"),
        "physical_candidate_count": focus.get("physical_candidate_count"),
        "physical_difference_count": focus.get("physical_difference_count"),
        "physical_difference_ids": list(physical_ids),
    }
    required = (
        "campaign_id", "owner", "unit", "function", "source_sha256",
        "target_object_sha256", "physical_target_identity_sha256",
        "physical_candidate_identity_sha256",
    )
    if any(not isinstance(body.get(key), str) or not body[key] for key in required):
        raise owner_campaign.CampaignError("frontier physical identity is incomplete")
    for key in (
        "source_sha256", "target_object_sha256", "physical_target_identity_sha256",
        "physical_candidate_identity_sha256",
    ):
        if not _is_hex_sha(body[key]):
            raise owner_campaign.CampaignError(f"frontier physical {key} is invalid")
    for key in (
        "physical_target_count", "physical_candidate_count", "physical_difference_count",
    ):
        if type(body[key]) is not int or body[key] < 0:
            raise owner_campaign.CampaignError(f"frontier physical {key} is invalid")
    if body["physical_difference_count"] != len(physical_ids):
        raise owner_campaign.CampaignError("frontier physical row count is inconsistent")
    body["physical_summary_sha256"] = _canonical_digest(body)
    digest = body["physical_summary_sha256"]
    path = (
        state_root_reader(root) / "proof-cas" / "physical" / digest[:2]
        / f"{digest}.json"
    )
    path = _input_path(root, path, "physical summary artifact", exists=False)
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.is_file():
        if _read_json_artifact(path, "physical summary artifact") != body:
            raise owner_campaign.CampaignError("physical summary CAS publication drift")
    else:
        ensure_peak = getattr(owner_campaign, "_ensure_state_write_peak", None)
        payload = owner_campaign._canonical(body) + b"\n"
        if callable(ensure_peak) and "limits" in campaign:
            ensure_peak(root, campaign, [(path, payload)])
        owner_campaign._atomic_json(
            path,
            body,
            limit=(campaign.get("limits", {}).get("frontier_bytes") if isinstance(campaign.get("limits"), Mapping) else None),
        )
    try:
        file_sha256 = owner_campaign._digest_file(path)
    except OSError as exc:
        raise owner_campaign.CampaignError("physical summary CAS publication failed") from exc
    return path, file_sha256, body


def _selection_evidence_for_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    candidate_path: Path,
    candidate_sha256: str,
    hypothesis_family: str,
    frontier: Mapping[str, Any],
    expected_terminal: str,
    predicted_rows: Sequence[str] | None,
    predicted_remaining_counts: Mapping[str, int] | None,
) -> tuple[dict[str, Any], Path, str, list[str], dict[str, int]]:
    """Build the selector sidecar and its current frontier evidence bindings."""

    if expected_terminal not in {"exact", "improved"}:
        raise owner_campaign.CampaignError("expected terminal is invalid")
    focus_path, focus_sha, focus = _focus_artifact_for_proposal(
        root, campaign, function, frontier
    )
    reconstruction = _load_reconstruction_for_frontier(
        root, campaign, function, frontier
    )
    if reconstruction is not None:
        if reconstruction["status"] == "UNKNOWN":
            if reconstruction["next_action"] != "DECOMPOSE":
                raise owner_campaign.CampaignError(
                    "frontier reconstruction is UNKNOWN; pivot required"
                )
            if expected_terminal != "improved":
                raise owner_campaign.CampaignError(
                    "UNKNOWN reconstruction allows improved-only decomposition"
                )
        elif reconstruction["status"] != "READY" or not reconstruction["ownership_complete"]:
            raise owner_campaign.CampaignError(
                "frontier reconstruction is UNKNOWN/incomplete; pivot required"
            )
        if expected_terminal == "exact" and not reconstruction["exact_terminal_possible"]:
            raise owner_campaign.CampaignError(
                "reconstruction does not support an exact terminal; use improved"
            )
    physical_path, physical_sha, physical = _physical_cas_for_proposal(
        root, campaign, frontier, focus_path, focus_sha, focus
    )
    try:
        strict_rows, data_rows, physical_rows = owner_campaign_selector._artifact_row_groups(
            focus, physical
        )
        residual = owner_campaign_selector._ordered_union(
            strict_rows, data_rows, physical_rows
        )
    except owner_campaign_selector.SelectionError as exc:
        raise owner_campaign.CampaignError(str(exc)) from exc
    if not residual:
        raise owner_campaign.CampaignError("current frontier has no residual rows")
    if predicted_rows is None:
        if expected_terminal != "exact":
            raise owner_campaign.CampaignError(
                "improved proposals require predicted rows and remaining counts"
            )
        predicted = list(residual)
    else:
        predicted = list(predicted_rows)
    if not predicted or len(predicted) != len(set(predicted)):
        raise owner_campaign.CampaignError("predicted rows are invalid")
    if any(not isinstance(row, str) or not row or len(row) > 512 for row in predicted):
        raise owner_campaign.CampaignError("predicted rows are invalid")
    if not set(predicted) <= set(residual):
        raise owner_campaign.CampaignError("predicted rows are outside current residual")
    first_mismatch_rows = owner_campaign_selector._first_mismatch_rows(
        strict_rows, data_rows
    )
    if first_mismatch_rows and not set(first_mismatch_rows) <= set(predicted):
        raise owner_campaign.CampaignError(
            "candidate does not cover the first mismatch"
        )
    current_counts = {
        "strict": len(strict_rows), "data": len(data_rows),
        "physical": len(physical_rows),
    }
    expected_counts = {
        channel: current_counts[channel] - sum(
            row.startswith(f"{channel}:") for row in predicted
        )
        for channel in current_counts
    }
    if expected_terminal == "exact":
        if set(predicted) != set(residual):
            raise owner_campaign.CampaignError(
                "exact proposals must predict every current residual row"
            )
        # Preserve the canonical current-frontier order in the sealed sidecar;
        # callers may supply the complete set in any order.
        predicted = list(residual)
        if predicted_remaining_counts is not None:
            if set(predicted_remaining_counts) != set(current_counts) or any(
                type(predicted_remaining_counts.get(channel)) is not int
                or predicted_remaining_counts[channel] != 0
                for channel in current_counts
            ):
                raise owner_campaign.CampaignError(
                    "exact proposals require zero remaining counts"
                )
        counts = {"strict": 0, "data": 0, "physical": 0}
    else:
        if not isinstance(predicted_remaining_counts, Mapping):
            raise owner_campaign.CampaignError(
                "improved proposals require predicted remaining counts"
            )
        if set(predicted_remaining_counts) != set(current_counts):
            raise owner_campaign.CampaignError(
                "predicted remaining counts must include strict, data, and physical"
            )
        counts = {}
        for channel, current in current_counts.items():
            value = predicted_remaining_counts[channel]
            if type(value) is not int or value < 0 or value > current:
                raise owner_campaign.CampaignError(
                    f"predicted remaining {channel} count is invalid"
                )
            counts[channel] = value
        if counts != expected_counts:
            raise owner_campaign.CampaignError(
                "predicted remaining counts do not match predicted rows"
            )
    reconstruction_cluster: Mapping[str, Any] | None = None
    reconstruction_region: Mapping[str, Any] | None = None
    if reconstruction is not None:
        reconstruction_cluster = _reconstruction_cluster_for_rows(
            reconstruction, predicted
        )
        if reconstruction["status"] == "UNKNOWN":
            reconstruction_region = _reconstruction_region_for_cluster(
                reconstruction, reconstruction_cluster
            )
    protected = focus.get("sibling_digest", focus.get("protected_sibling_digest"))
    if not _is_hex_sha(protected):
        raise owner_campaign.CampaignError("current frontier sibling digest is missing")
    source_relpath = campaign.get("source_relpath", frontier.get("source_relpath"))
    unit = frontier.get("unit", campaign.get("unit"))
    toolchain = frontier.get("toolchain_sha256")
    if not isinstance(source_relpath, str) or not source_relpath:
        raise owner_campaign.CampaignError("current frontier source path is missing")
    if not isinstance(unit, str) or not unit:
        raise owner_campaign.CampaignError("current frontier unit is missing")
    if not _is_hex_sha(toolchain):
        raise owner_campaign.CampaignError("current frontier toolchain binding is missing")
    campaign_id = campaign.get("campaign_id", frontier.get("campaign_id"))
    owner = campaign.get("owner", frontier.get("owner"))
    if not isinstance(campaign_id, str) or not campaign_id:
        raise owner_campaign.CampaignError("current frontier campaign binding is missing")
    if not isinstance(owner, str) or not owner:
        raise owner_campaign.CampaignError("current frontier owner binding is missing")
    if not _is_hex_sha(frontier.get("source_sha256")):
        raise owner_campaign.CampaignError("current frontier source binding is invalid")
    frontier_path = (
        owner_campaign._function_root(root, campaign, function)
        / "latest-frontier.json"
    )
    frontier_path = _input_path(root, frontier_path, "current frontier")
    if _read_json_artifact(frontier_path, "current frontier") != dict(frontier):
        raise owner_campaign.CampaignError("current frontier file drift")
    body: dict[str, Any] = {
        "schema": owner_campaign_selector.SCHEMA,
        "status": "RANKED_SOURCE_CLASS",
        "selection_kind": "RANKED_SOURCE_CLASS",
        "expected_terminal": expected_terminal,
        "campaign_id": campaign_id,
        "owner": owner,
        "unit": unit,
        "function": function,
        "rank": 1,
        "source_class": hypothesis_family,
        "candidate": {
            "path": candidate_path.relative_to(root).as_posix(),
            "sha256": candidate_sha256,
        },
        "frontier": {
            "path": frontier_path.relative_to(root).as_posix(),
            "sha256": frontier["frontier_sha256"],
            "source_sha256": frontier["source_sha256"],
            "function": function,
            "unit": unit,
            "toolchain_sha256": toolchain,
        },
        "base_frontier_sha256": frontier["frontier_sha256"],
        "source_path": source_relpath,
        "source_sha256": frontier["source_sha256"],
        "toolchain_sha256": toolchain,
        "focus_artifact": {
            "path": focus_path.relative_to(root).as_posix(),
            "sha256": focus_sha,
        },
        "physical_artifact": {
            "path": physical_path.relative_to(root).as_posix(),
            "sha256": physical_sha,
        },
        "residual_rows": residual,
        "predicted_rows": predicted,
        "predicted_remaining_counts": counts,
        "protected_sibling_digest": protected,
        "ownership_complete": (
            reconstruction["ownership_complete"]
            if reconstruction is not None else True
        ),
        "ownership_scope": (
            "bounded_decomposition_region"
            if reconstruction is not None
            and reconstruction["status"] == "UNKNOWN"
            and reconstruction["next_action"] == "DECOMPOSE"
            else "complete_function"
        ),
        "candidate_count": 1,
    }
    if reconstruction is not None:
        body["reconstruction"] = {
            "path": reconstruction["path"].relative_to(root).as_posix(),
            "sha256": reconstruction["file_sha256"],
            "packet_sha256": reconstruction["packet_sha256"],
            "status": reconstruction["status"],
            "target_first_signal": reconstruction["signal"],
            "causal_cluster_count": reconstruction["cluster_count"],
            "causal_cluster_id": reconstruction_cluster.get("cluster_id")
            if reconstruction_cluster is not None else None,
            "exact_terminal_possible": reconstruction["exact_terminal_possible"],
            "next_action": reconstruction["next_action"],
            "bounded_region": dict(reconstruction_region)
            if reconstruction_region is not None else None,
        }
    return (
        {**body, "evidence_sha256": _canonical_digest(body)},
        focus_path,
        focus_sha,
        residual,
        counts,
    )


def _prepare_candidate_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    candidate_source: Path,
    hypothesis_family: str,
    *,
    expected_terminal: str,
    predicted_rows: Sequence[str] | None,
    predicted_remaining_counts: Mapping[str, int] | None,
    rebase_depth: int = 0,
    required_current_function: bytes | None = None,
) -> dict[str, Any]:
    """Prepare immutable proposal inputs without holding the frontier locks.

    Source decoding, span discovery, diff classification, and focus/physical
    evidence parsing are read-only work.  Keeping them outside the CAS lock
    allows independent Sol proposal workers to make progress concurrently.  A
    later locked publication phase re-reads every bound input before the final
    directory rename, so this optimization does not turn a stale proposal into
    a queued candidate.
    """

    if type(rebase_depth) is not int or not 0 <= rebase_depth <= 5:
        raise owner_campaign.CampaignError(
            "candidate rebase_depth must be between 0 and 5"
        )
    if required_current_function is not None and not isinstance(
        required_current_function, bytes
    ):
        raise owner_campaign.CampaignError("required current function snapshot is invalid")
    source_path = _campaign_source_path(root, campaign)
    limits = campaign.get("limits")
    proposal_limit = (
        limits.get("cell_temporary_bytes")
        if isinstance(limits, Mapping)
        else None
    )
    if type(proposal_limit) is not int or proposal_limit <= 0:
        proposal_limit = None
    source_bytes = _stable_file_bytes(
        source_path, "campaign source", max_bytes=proposal_limit,
    )
    source_sha256 = owner_campaign._digest_bytes(source_bytes)
    frontier = _frontier_for_proposal(root, campaign, function)
    if frontier["source_sha256"] != source_sha256:
        raise owner_campaign.CampaignError("current source has drifted from frontier")
    candidate_bytes = _stable_file_bytes(
        candidate_source, "candidate source", max_bytes=proposal_limit,
    )
    candidate_sha256 = owner_campaign._digest_bytes(candidate_bytes)
    if candidate_sha256 == source_sha256:
        raise owner_campaign.CampaignError("candidate source is byte-identical to frontier")
    try:
        base_text = source_bytes.decode("utf-8")
        candidate_text = candidate_bytes.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise owner_campaign.CampaignError("candidate source is not UTF-8 natural C") from exc
    if "\x00" in candidate_text:
        raise owner_campaign.CampaignError("candidate source contains NUL")
    base_start, base_end, base_span = _function_span(
        base_text, function, "base function"
    )
    if (
        required_current_function is not None
        and base_span != required_current_function
    ):
        raise owner_campaign.CampaignError(
            "current function changed during stale rebase"
        )
    candidate_start, candidate_end, candidate_span = _function_span(
        candidate_text, function, "candidate function"
    )
    base_lines = base_text.splitlines(keepends=True)
    candidate_lines = candidate_text.splitlines(keepends=True)
    if (
        base_lines[: base_start - 1] != candidate_lines[: candidate_start - 1]
        or base_lines[base_end:] != candidate_lines[candidate_end:]
    ):
        raise owner_campaign.CampaignError("candidate edits escape the named function")
    matcher = difflib.SequenceMatcher(
        a=base_lines, b=candidate_lines, autojunk=False
    )
    changed = False
    added: list[str] = []
    for tag, base_a, base_b, candidate_a, candidate_b in matcher.get_opcodes():
        if tag == "equal":
            continue
        changed = True
        if (
            base_a < base_start - 1
            or base_b > base_end
            or candidate_a < candidate_start - 1
            or candidate_b > candidate_end
        ):
            raise owner_campaign.CampaignError("candidate edits cross the named function")
        added.extend(candidate_lines[candidate_a:candidate_b])
    if not changed:
        raise owner_campaign.CampaignError("candidate has no source change")
    added_text = "".join(added)
    for pattern in campaign.get("forbidden_constructs", []):
        try:
            found = re.search(pattern, added_text)
        except re.error as exc:
            raise owner_campaign.CampaignError(
                f"invalid forbidden construct regex: {pattern}"
            ) from exc
        if found:
            raise owner_campaign.CampaignError(
                f"candidate contains forbidden construct: {pattern}"
            )
    if _proposal_descriptor_matches(
        root, campaign, function, frontier["frontier_sha256"], candidate_sha256
    ):
        raise owner_campaign.CampaignError("duplicate candidate is already queued or measured")

    function_slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", function).strip("_") or "function"
    name = f"{function_slug}-{candidate_sha256[:24]}"
    inbox = inbox_path(root, campaign)
    final_dir = inbox / name
    created_at = owner_campaign._now()
    descriptor_body: dict[str, Any] = {
        "schema": owner_campaign.CANDIDATE_SCHEMA,
        "campaign_id": campaign["campaign_id"],
        "function": function,
        "base_frontier_sha256": frontier["frontier_sha256"],
        "base_source": {
            "path": (final_dir / "base.c").relative_to(root).as_posix(),
            "sha256": source_sha256,
        },
        "candidate_source": {
            "path": (final_dir / "candidate.c").relative_to(root).as_posix(),
            "sha256": candidate_sha256,
        },
        "function_span": {
            "base_start_line": base_start,
            "base_end_line": base_end,
            "candidate_start_line": candidate_start,
            "candidate_end_line": candidate_end,
            "base_sha256": owner_campaign._digest_bytes(base_span),
            "candidate_sha256": owner_campaign._digest_bytes(candidate_span),
        },
        "hypothesis_family": hypothesis_family,
        "natural_c": True,
        "rebase_depth": rebase_depth,
        "created_at": created_at,
    }
    descriptor = {
        **descriptor_body,
        "candidate_sha256": _canonical_digest(descriptor_body),
    }
    selection, _focus_path, _focus_sha, residual, counts = _selection_evidence_for_proposal(
        root,
        campaign,
        function,
        final_dir / "candidate.c",
        candidate_sha256,
        hypothesis_family,
        frontier,
        expected_terminal,
        predicted_rows,
        predicted_remaining_counts,
    )
    physical_ref = selection.get("physical_artifact")
    if not isinstance(physical_ref, Mapping):
        raise owner_campaign.CampaignError("selection physical artifact binding is missing")
    _physical_path = _input_path(
        root, physical_ref.get("path"), "physical summary artifact"
    )
    _physical_sha = physical_ref.get("sha256")
    if not _is_hex_sha(_physical_sha):
        raise owner_campaign.CampaignError("selection physical artifact hash is invalid")
    ensure_peak = getattr(owner_campaign, "_ensure_state_write_peak", None)
    if (
        callable(ensure_peak) and isinstance(limits, Mapping)
        and _STATE_LIMIT_FIELDS <= set(limits)
    ):
        ensure_peak(
            root,
            campaign,
            [
                (final_dir / "base.c", source_bytes),
                (final_dir / "candidate.c", candidate_bytes),
                (final_dir / "candidate.json", owner_campaign._canonical(descriptor) + b"\n"),
                (
                    final_dir / "candidate.selection.json",
                    owner_campaign._canonical(selection) + b"\n",
                ),
            ],
        )
    return {
        "function": function,
        "hypothesis_family": hypothesis_family,
        "expected_terminal": expected_terminal,
        "predicted_rows_input": (
            None if predicted_rows is None else list(predicted_rows)
        ),
        "source_path": source_path,
        "source_bytes": source_bytes,
        "source_sha256": source_sha256,
        "candidate_path": candidate_source,
        "candidate_bytes": candidate_bytes,
        "candidate_sha256": candidate_sha256,
        "frontier": frontier,
        "name": name,
        "inbox": inbox,
        "final_dir": final_dir,
        "created_at": created_at,
        "descriptor": descriptor,
        "selection": selection,
        "focus_path": _focus_path,
        "focus_sha256": _focus_sha,
        "physical_path": _physical_path,
        "physical_sha256": _physical_sha,
        "residual": residual,
        "counts": counts,
    }


def _stage_prepared_proposal(prepared: Mapping[str, Any]) -> Path:
    """Write a disposable proposal directory before acquiring CAS locks."""

    inbox = Path(prepared["inbox"])
    inbox.mkdir(parents=True, exist_ok=True)
    stage: Path | None = None
    try:
        stage = Path(tempfile.mkdtemp(prefix=f".{prepared['name']}.", dir=inbox))
        owner_campaign._atomic_bytes(stage / "base.c", prepared["source_bytes"])
        owner_campaign._atomic_bytes(stage / "candidate.c", prepared["candidate_bytes"])
        owner_campaign._atomic_json(stage / "candidate.json", prepared["descriptor"])
        owner_campaign._atomic_json(
            stage / "candidate.selection.json", prepared["selection"]
        )
        return stage
    except BaseException:
        if stage is not None:
            try:
                shutil.rmtree(stage)
            except OSError:
                pass
        raise


def _proposal_directory_matches(root: Path, stage: Path, final_dir: Path) -> bool:
    """Return whether an already-published destination is this exact stage."""

    indirection_checker = getattr(owner_campaign, "_path_has_indirection", None)
    if (
        final_dir.is_symlink()
        or not final_dir.is_dir()
        or (
            callable(indirection_checker)
            and indirection_checker(root, final_dir)
        )
    ):
        return False
    names = {"base.c", "candidate.c", "candidate.json", "candidate.selection.json"}
    try:
        if {item.name for item in final_dir.iterdir()} != names:
            return False
        return all(
            owner_campaign._digest_file(final_dir / name)
            == owner_campaign._digest_file(stage / name)
            for name in names
        )
    except OSError:
        return False


def _commit_proposal_directory(
    root: Path,
    campaign: Mapping[str, Any],
    stage: Path,
    final_dir: Path,
) -> None:
    """Publish one prepared directory through a short process-safe link gate."""

    timeout = (
        owner_campaign._command_timeout_seconds(campaign)
        if "limits" in campaign
        else 30.0
    )
    publication_lock = inbox_path(root, campaign) / ".proposal-publication.lock"
    with _PROPOSAL_PUBLICATION_THREAD_LOCK:
        with owner_campaign._exclusive_lock(publication_lock, timeout):
            deadline = time.monotonic() + min(timeout, 1.0)
            while True:
                if final_dir.exists() or final_dir.is_symlink():
                    if _proposal_directory_matches(root, stage, final_dir):
                        return
                    raise owner_campaign.CampaignError(
                        "duplicate candidate destination has conflicting content"
                    )
                try:
                    # Same-parent rename is atomic and fail-if-present on both
                    # supported platforms.  Windows can transiently return
                    # ACCESS_DENIED/SHARING_VIOLATION while another thread's
                    # directory notification is draining, so retry only those
                    # bounded errors while this narrow publication gate is held.
                    os.rename(stage, final_dir)
                    return
                except OSError as exc:
                    transient = os.name == "nt" and getattr(exc, "winerror", None) in {
                        5, 32, 33,
                    }
                    if not transient or time.monotonic() >= deadline:
                        raise
                    time.sleep(0.02)


def _publish_prepared_proposal_locked(
    root: Path,
    campaign: Mapping[str, Any],
    prepared: Mapping[str, Any],
    stage: Path,
) -> dict[str, Any]:
    """Revalidate a prepared proposal and atomically publish it under CAS locks."""

    function = str(prepared["function"])
    frontier = prepared["frontier"]
    final_dir = Path(prepared["final_dir"])
    source_path = Path(prepared["source_path"])
    candidate_path = Path(prepared["candidate_path"])
    focus_path = Path(prepared["focus_path"])
    physical_path = Path(prepared["physical_path"])
    try:
        current_source = _stable_file_bytes(source_path, "campaign source")
        if owner_campaign._digest_bytes(current_source) != prepared["source_sha256"]:
            raise owner_campaign.CampaignError("campaign source drifted during proposal")
        current_candidate = _stable_file_bytes(candidate_path, "candidate source")
        if owner_campaign._digest_bytes(current_candidate) != prepared["candidate_sha256"]:
            raise owner_campaign.CampaignError("candidate source drifted during proposal")
        staged_base = stage / "base.c"
        if owner_campaign._digest_bytes(
            _stable_file_bytes(staged_base, "staged base source")
        ) != prepared["source_sha256"]:
            raise owner_campaign.CampaignError("staged base source drifted during proposal")
        current_frontier = _frontier_for_proposal(root, campaign, function)
        if current_frontier["frontier_sha256"] != frontier["frontier_sha256"]:
            raise owner_campaign.CampaignError("frontier advanced during proposal")
        current_focus = _stable_file_bytes(focus_path, "frontier focus artifact")
        if owner_campaign._digest_bytes(current_focus) != prepared["focus_sha256"]:
            raise owner_campaign.CampaignError("frontier focus artifact drifted during proposal")
        current_physical = _stable_file_bytes(
            physical_path, "physical summary artifact"
        )
        if owner_campaign._digest_bytes(current_physical) != prepared["physical_sha256"]:
            raise owner_campaign.CampaignError(
                "physical summary artifact drifted during proposal"
            )
        _commit_proposal_directory(root, campaign, stage, final_dir)
    except BaseException:
        if stage.exists():
            try:
                shutil.rmtree(stage)
            except OSError:
                pass
        raise
    descriptor_path = final_dir / "candidate.json"
    return {
        "schema": PROPOSAL_RESULT_SCHEMA,
        "status": "queued",
        "campaign_id": campaign["campaign_id"],
        "function": function,
        "hypothesis_family": prepared["hypothesis_family"],
        "base_frontier_sha256": frontier["frontier_sha256"],
        "candidate_source_sha256": prepared["candidate_sha256"],
        "candidate_source": (final_dir / "candidate.c").relative_to(root).as_posix(),
        "candidate_descriptor": descriptor_path.relative_to(root).as_posix(),
        "descriptor_sha256": owner_campaign._digest_file(descriptor_path),
        "candidate_selection": (final_dir / "candidate.selection.json").relative_to(root).as_posix(),
        "selection_sha256": owner_campaign._digest_file(final_dir / "candidate.selection.json"),
        "selection_evidence": (final_dir / "candidate.selection.json").relative_to(root).as_posix(),
        "selection_evidence_sha256": prepared["selection"]["evidence_sha256"],
        "reconstruction": prepared["selection"].get("reconstruction"),
        "expected_terminal": prepared["expected_terminal"],
        "predicted_rows": (
            prepared["residual"]
            if prepared["predicted_rows_input"] is None
            else list(prepared["predicted_rows_input"])
        ),
        "predicted_remaining_counts": prepared["counts"],
        "created_at": prepared["created_at"],
        "authority_advanced": False,
    }


def propose_candidate(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    candidate_source: Path,
    hypothesis_family: str,
    *,
    expected_terminal: str = "exact",
    predicted_rows: Sequence[str] | None = None,
    predicted_remaining_counts: Mapping[str, int] | None = None,
    rebase_depth: int = 0,
    _required_current_function: bytes | None = None,
) -> dict[str, Any]:
    """Seal one worker source into the inbox against the current frontier.

    This is intentionally a source-only front door.  It does not select a
    winner, compile, retain, or consult legacy approval state.
    """

    root = Path(os.path.abspath(root))
    if not isinstance(function, str) or not function or function not in campaign.get("functions", []):
        raise owner_campaign.CampaignError("function is outside campaign scope")
    if not isinstance(hypothesis_family, str) or not hypothesis_family.strip() or "\x00" in hypothesis_family:
        raise owner_campaign.CampaignError("hypothesis family is invalid")
    hypothesis_family = hypothesis_family.strip()
    if len(hypothesis_family) > 256:
        raise owner_campaign.CampaignError("hypothesis family is too long")
    candidate_path = _input_path(root, candidate_source, "candidate source")
    if candidate_path == _campaign_source_path(root, campaign):
        raise owner_campaign.CampaignError("candidate source must be distinct from campaign source")
    if not _allowed_candidate_path(root, campaign, candidate_path):
        raise owner_campaign.CampaignError("candidate source is outside campaign allowed paths")
    prepared = _prepare_candidate_proposal(
        root, campaign, function, candidate_path, hypothesis_family,
        expected_terminal=expected_terminal,
        predicted_rows=predicted_rows,
        predicted_remaining_counts=predicted_remaining_counts,
        rebase_depth=rebase_depth,
        required_current_function=_required_current_function,
    )
    stage = _stage_prepared_proposal(prepared)
    lock_factory = getattr(owner_campaign, "_frontier_lock_chain", None)
    if callable(lock_factory) and "_source" in campaign and "limits" in campaign:
        context = lock_factory(root, campaign, function)
    else:
        context = nullcontext()
    try:
        # The preflight above accounts for the intended payloads.  Recheck
        # after staging as well so concurrent proposal workers cannot
        # collectively push the inbox beyond the global retained-state cap.
        if (
            "_source" in campaign
            and isinstance(campaign.get("limits"), Mapping)
            and _STATE_LIMIT_FIELDS <= set(campaign["limits"])
        ):
            owner_campaign._check_limits(root, campaign)
        with context:
            return _publish_prepared_proposal_locked(
                root, campaign, prepared, stage
            )
    finally:
        if stage.exists():
            try:
                shutil.rmtree(stage)
            except OSError:
                pass


submit_candidate = propose_candidate
enqueue_candidate = propose_candidate
queue_candidate = propose_candidate
propose = propose_candidate


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
    base_source = value.get("base_source")
    if not isinstance(base_source, Mapping) or set(base_source) != {"path", "sha256"}:
        return None
    base_path = _relative_path(root, base_source.get("path"))
    if (
        base_path is None
        or not _under_allowed_build(root, base_path, campaign)
        or not _is_hex_sha(base_source.get("sha256"))
    ):
        return None
    try:
        if (
            not base_path.is_file()
            or owner_campaign._digest_file(base_path) != base_source["sha256"]
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
    found: list[tuple[str, str, Path, str]] = []
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
        found.append((created_key, path.relative_to(inbox).as_posix(), path, descriptor["function"]))
    found.sort(key=lambda item: (item[0], item[1]))
    # A global first-N slice can hide an eligible function behind a stalled
    # function's proposal run.  Interleave candidates by manifest function
    # order while preserving created/path order within each function.  The
    # Arbitration later preserves deterministic proposal order while allowing
    # the streaming scheduler to fill every worker slot when a function has
    # multiple current-bound candidates.
    scoped_functions = [
        function for function in campaign.get("functions", [])
        if isinstance(function, str)
    ]
    if len(scoped_functions) > 1:
        buckets: dict[str, list[tuple[str, str, Path, str]]] = {
            function: [] for function in scoped_functions
        }
        for entry in found:
            buckets.setdefault(entry[3], []).append(entry)
        ordered: list[tuple[str, str, Path, str]] = []
        offset = 0
        while len(ordered) < len(found):
            added = False
            for function in scoped_functions:
                bucket = buckets.get(function, [])
                if offset < len(bucket):
                    ordered.append(bucket[offset])
                    added = True
            if not added:
                break
            offset += 1
        found = ordered
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
    """Remove terminal inbox inputs, retrying transient filesystem errors.

    The descriptor is deliberately removed last.  If a source, sidecar, or
    base snapshot cannot be removed, the descriptor remains as a durable
    retry token and the lane reports ``infra_retry`` instead of silently
    claiming that terminal history was compacted.
    """

    removed: list[str] = []
    errors: list[str] = []
    descriptor_path = Path(os.path.abspath(descriptor_path))
    source_path: Path | None = None
    try:
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        if sealed is not None:
            source_path = sealed[1]
        else:
            # A prior cleanup attempt may already have removed the candidate
            # source, making the full sealed-descriptor check impossible.  A
            # strictly contained raw binding is still enough to finish
            # cleanup; never use an unbound path.
            try:
                raw = json.loads(descriptor_path.read_text(encoding="utf-8"))
                binding = raw.get("candidate_source") if isinstance(raw, Mapping) else None
                raw_path = binding.get("path") if isinstance(binding, Mapping) else None
                candidate = _relative_path(root, raw_path)
                if (
                    candidate is not None
                    and _under_allowed_build(root, candidate, campaign)
                ):
                    source_path = candidate
            except (OSError, UnicodeError, json.JSONDecodeError, AttributeError):
                source_path = None
    except (OSError, ValueError):
        source_path = None

    base_snapshot = descriptor_path.parent / "base.c"
    sidecars: list[Path] = []
    try:
        sidecars = [
            Path(path)
            for path in owner_campaign_selector.selection_evidence_paths(
                descriptor_path
            )
            if _path_inside(inbox_path(root, campaign), Path(os.path.abspath(path)))
        ]
    except (OSError, ValueError):
        errors.append(f"cleanup-error:{descriptor_path}:selection sidecar discovery failed")

    def relative(path: Path) -> str:
        try:
            return path.relative_to(root).as_posix()
        except ValueError:
            return str(path)

    def unlink_retry(path: Path, *, required: bool) -> None:
        if not required and not (path.exists() or path.is_symlink()):
            return
        last: OSError | None = None
        for attempt in range(3):
            try:
                if path.exists() or path.is_symlink():
                    path.unlink()
                    removed.append(relative(path))
                return
            except OSError as exc:
                last = exc
                if attempt < 2:
                    time.sleep(0.02 * (attempt + 1))
        if last is not None:
            errors.append(f"cleanup-error:{path}:{last}")

    # Remove source and base only when they are campaign-contained and not
    # shared by another still-pending proposal.  Sidecars are always inbox
    # contained; an absent sidecar is already compacted.
    if (
        source_path is not None
        and _under_allowed_build(root, source_path, campaign)
        and not _source_referenced_by_pending(
            root, campaign, source_path, exclude=descriptor_path
        )
    ):
        unlink_retry(source_path, required=False)
    if (
        base_snapshot != source_path
        and _under_allowed_build(root, base_snapshot, campaign)
    ):
        unlink_retry(base_snapshot, required=False)
    for sidecar in sidecars:
        if sidecar.is_symlink():
            errors.append(f"cleanup-error:{sidecar}:selection sidecar is indirect")
        else:
            unlink_retry(sidecar, required=False)

    # Keep the descriptor as the retry token until every companion is gone.
    if not errors:
        unlink_retry(descriptor_path, required=False)
    return [*removed, *errors]


def _rebase_tombstone_path(
    root: Path, campaign: Mapping[str, Any], descriptor_sha256: str
) -> Path:
    return inbox_path(root, campaign) / ".rebases" / f"{descriptor_sha256}.receipt"


def _read_rebase_tombstone(path: Path) -> dict[str, Any] | None:
    if not path.is_file() or path.is_symlink():
        return None
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError):
        return None
    if not isinstance(value, Mapping) or set(value) != REBASE_TOMBSTONE_FIELDS:
        return None
    body = dict(value)
    digest = body.pop("tombstone_sha256", None)
    status = value.get("status")
    if (
        not _is_hex_sha(digest)
        or _canonical_digest(body) != digest
        or value.get("schema") != REBASE_TOMBSTONE_SCHEMA
        or status not in {REBASED_STATUS, REBASE_REJECTED_STATUS}
        or not isinstance(value.get("campaign_id"), str)
        or not value["campaign_id"]
        or not isinstance(value.get("function"), str)
        or not value["function"]
        or not _is_hex_sha(value.get("old_descriptor_sha256"))
        or not _is_hex_sha(value.get("old_candidate_sha256"))
        or not _is_hex_sha(value.get("old_frontier_sha256"))
        or type(value.get("rebase_depth")) is not int
        or not 0 <= value["rebase_depth"] <= 5
        or not isinstance(value.get("reason"), str)
        or not isinstance(value.get("created_at"), str)
    ):
        return None
    if status == REBASED_STATUS:
        if (
            not isinstance(value.get("new_descriptor"), str)
            or not value["new_descriptor"]
            or Path(value["new_descriptor"]).is_absolute()
            or not _is_hex_sha(value.get("new_descriptor_sha256"))
            or not _is_hex_sha(value.get("new_candidate_sha256"))
            or value["rebase_depth"] < 1
        ):
            return None
    elif any(
        value.get(field) is not None
        for field in (
            "new_descriptor", "new_descriptor_sha256", "new_candidate_sha256"
        )
    ):
        return None
    return dict(value)


def _publish_rebase_tombstone(
    root: Path,
    campaign: Mapping[str, Any],
    *,
    function: str,
    status: str,
    old_descriptor_sha256: str,
    old_candidate_sha256: str,
    old_frontier_sha256: str,
    new_descriptor: str | None,
    new_descriptor_sha256: str | None,
    new_candidate_sha256: str | None,
    rebase_depth: int,
    reason: str,
) -> dict[str, Any]:
    body = {
        "schema": REBASE_TOMBSTONE_SCHEMA,
        "campaign_id": campaign["campaign_id"],
        "function": function,
        "status": status,
        "old_descriptor_sha256": old_descriptor_sha256,
        "old_candidate_sha256": old_candidate_sha256,
        "old_frontier_sha256": old_frontier_sha256,
        "new_descriptor": new_descriptor,
        "new_descriptor_sha256": new_descriptor_sha256,
        "new_candidate_sha256": new_candidate_sha256,
        "rebase_depth": rebase_depth,
        "reason": reason[:1000],
        "created_at": owner_campaign._now(),
    }
    value = {**body, "tombstone_sha256": _canonical_digest(body)}
    path = _rebase_tombstone_path(root, campaign, old_descriptor_sha256)
    existing = _read_rebase_tombstone(path)
    if existing is not None:
        # The old descriptor identity may reach this boundary again after a
        # process interruption.  Its first durable disposition is final.
        return existing
    ensure_peak = getattr(owner_campaign, "_ensure_state_write_peak", None)
    payload = owner_campaign._canonical(value) + b"\n"
    if callable(ensure_peak) and isinstance(campaign.get("limits"), Mapping):
        ensure_peak(root, campaign, [(path, payload)])
    owner_campaign._atomic_json(path, value, limit=16 << 10)
    observed = _read_rebase_tombstone(path)
    if observed != value:
        raise owner_campaign.CampaignError("stale rebase tombstone publication failed")
    return value


def _cleanup_selection_input(
    root: Path, campaign: Mapping[str, Any], selected: Mapping[str, Any]
) -> list[str]:
    removed: list[str] = []
    raw = selected.get("evidence_path")
    if not isinstance(raw, (str, os.PathLike)):
        return removed
    path = Path(raw)
    if not path.is_absolute():
        path = root / path
    path = Path(os.path.abspath(path))
    try:
        if (
            path.is_file()
            and _path_inside(inbox_path(root, campaign), path)
            and not path.is_symlink()
        ):
            path.unlink()
            removed.append(path.relative_to(root).as_posix())
    except (OSError, ValueError) as exc:
        removed.append(f"cleanup-error:{path}:{exc}")
    return removed


def _row_rebase_key(row: str) -> tuple[str, int, str] | None:
    match = re.match(
        r"^(strict|data):[^:]+:row:(\d+):kind=([^:]+)(?::|$)", row
    )
    if match is None:
        return None
    return match.group(1), int(match.group(2)), match.group(3)


def _remap_predicted_rows(
    predicted_rows: Sequence[str], current_rows: Sequence[str]
) -> list[str]:
    indexed: dict[tuple[str, int, str], list[str]] = {}
    current_set = set(current_rows)
    for row in current_rows:
        key = _row_rebase_key(row)
        if key is not None:
            indexed.setdefault(key, []).append(row)
    remapped: list[str] = []
    for row in predicted_rows:
        if row in current_set:
            selected = row
        else:
            key = _row_rebase_key(row)
            matches = indexed.get(key, []) if key is not None else []
            # Physical-difference identities intentionally include the
            # candidate relocation payload.  A disjoint TU gain can change
            # that hash even though this source cell still targets the same
            # strict/data rows.  Do not invent a physical mapping: drop only
            # that unmatched prediction and downgrade the refreshed proposal
            # to ``improved`` below.  The candidate measurement must preserve
            # the current physical channel before it can be retained.
            if row.startswith("physical:") and not matches:
                continue
            if len(matches) != 1:
                raise owner_campaign.CampaignError(
                    "stale candidate prediction no longer maps uniquely to the current residual"
                )
            selected = matches[0]
        if selected in remapped:
            raise owner_campaign.CampaignError(
                "stale candidate prediction maps to duplicate current rows"
            )
        remapped.append(selected)
    if not remapped:
        raise owner_campaign.CampaignError("stale candidate has no current predicted rows")
    return remapped


def _current_residual_for_rebase(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    *,
    worker: int,
) -> tuple[dict[str, Any], list[str], dict[str, int]]:
    # The first retained function advances the shared TU source.  Refresh
    # this function's baseline independently on its assigned worker before
    # rebuilding the proposal against that source.
    if "_source" in campaign and "limits" in campaign:
        frontier = owner_campaign.snapshot_frontier(
            root,
            campaign,
            function,
            worker=worker,
            _defer_maintenance=True,
        )
    else:
        # Descriptor-only fixtures and authorized replay callers predate the
        # loaded production manifest.  Their frontier is already advanced by
        # the caller and remains fully checked by proposal publication.
        frontier = _frontier_for_proposal(root, campaign, function)
    focus_path, focus_sha, focus = _focus_artifact_for_proposal(
        root, campaign, function, frontier
    )
    _physical_path, _physical_sha, physical = _physical_cas_for_proposal(
        root, campaign, frontier, focus_path, focus_sha, focus
    )
    try:
        strict_rows, data_rows, physical_rows = (
            owner_campaign_selector._artifact_row_groups(focus, physical)
        )
        residual = owner_campaign_selector._ordered_union(
            strict_rows, data_rows, physical_rows
        )
    except owner_campaign_selector.SelectionError as exc:
        raise owner_campaign.CampaignError(str(exc)) from exc
    if not residual:
        raise owner_campaign.CampaignError(
            "stale candidate function is already exact on the current source"
        )
    return dict(frontier), residual, {
        "strict": len(strict_rows),
        "data": len(data_rows),
        "physical": len(physical_rows),
    }


def _find_rebased_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    function: str,
    frontier_sha256: str,
    candidate_sha256: str,
) -> Path | None:
    inbox = inbox_path(root, campaign)
    if not inbox.is_dir():
        return None
    for path in inbox.rglob("candidate.json"):
        sealed = _sealed_descriptor(root, campaign, path)
        if sealed is None:
            continue
        descriptor = sealed[0]
        if (
            descriptor.get("function") == function
            and descriptor.get("base_frontier_sha256") == frontier_sha256
            and descriptor.get("candidate_source", {}).get("sha256")
            == candidate_sha256
        ):
            return path
    return None


def _rebase_result(
    *,
    status: str,
    function: str,
    reason: str,
    old_descriptor: Path,
    old_descriptor_sha256: str,
    rebase_depth: int,
    cleaned: Sequence[str],
    new_descriptor: Path | None = None,
    new_candidate: Path | None = None,
    new_candidate_sha256: str | None = None,
) -> dict[str, Any]:
    body: dict[str, Any] = {
        "schema": "owner_campaign_result/v1",
        "status": status,
        "function": function,
        "reason": reason[:1000],
        "old_descriptor": str(old_descriptor),
        "old_descriptor_sha256": old_descriptor_sha256,
        "rebase_depth": rebase_depth,
        "cleaned": list(cleaned),
        "new_descriptor": str(new_descriptor) if new_descriptor is not None else None,
        "new_candidate": str(new_candidate) if new_candidate is not None else None,
        "new_candidate_sha256": new_candidate_sha256,
        "cleanup_status": (
            "cleanup_incomplete"
            if any(item.startswith("cleanup-error:") for item in cleaned)
            else "complete"
        ),
        "authority_advanced": False,
    }
    return {**body, "result_sha256": _canonical_digest(body)}


def _auto_rebase_stale_candidate(
    root: Path,
    campaign: Mapping[str, Any],
    descriptor_path: Path,
    selected: Mapping[str, Any],
    *,
    worker: int,
) -> dict[str, Any]:
    """Requeue one disjoint stale function edit against the current TU source.

    The immutable base snapshot proves what the candidate changed.  Rebasing
    is allowed only when the named function is still byte-identical in the
    live source; all other source changes are inherited.  Current focus and
    physical residuals are refreshed before a new selector packet is sealed.
    """

    descriptor_path = Path(os.path.abspath(descriptor_path))
    sealed = _sealed_descriptor(root, campaign, descriptor_path)
    if sealed is None:
        raise owner_campaign.CampaignError("stale candidate descriptor is not sealed")
    descriptor, candidate_path = sealed
    descriptor_file_sha = owner_campaign._digest_file(descriptor_path)
    function = descriptor["function"]
    old_candidate_sha = descriptor["candidate_source"]["sha256"]
    old_frontier_sha = descriptor["base_frontier_sha256"]
    old_depth = descriptor["rebase_depth"]
    if (
        selected.get("function") != function
        or selected.get("candidate_sha256") != old_candidate_sha
        or selected.get("frontier_sha256") != old_frontier_sha
        or selected.get("rebase_depth") != old_depth
        or selected.get("base_source_sha256")
        != descriptor["base_source"]["sha256"]
    ):
        raise owner_campaign.CampaignError("stale selection binding drift")
    tombstone_path = _rebase_tombstone_path(root, campaign, descriptor_file_sha)
    prior = _read_rebase_tombstone(tombstone_path)
    if prior is not None:
        if (
            prior["campaign_id"] != campaign["campaign_id"]
            or prior["function"] != function
            or prior["old_descriptor_sha256"] != descriptor_file_sha
            or prior["old_candidate_sha256"] != old_candidate_sha
            or prior["old_frontier_sha256"] != old_frontier_sha
        ):
            raise owner_campaign.CampaignError("stale rebase tombstone binding drift")
        next_path = (
            _relative_path(root, prior["new_descriptor"])
            if isinstance(prior.get("new_descriptor"), str)
            else None
        )
        next_candidate: Path | None = None
        if next_path is not None:
            next_sealed = _sealed_descriptor(root, campaign, next_path)
            if (
                next_sealed is None
                or owner_campaign._digest_file(next_path)
                != prior["new_descriptor_sha256"]
                or next_sealed[0]["candidate_source"]["sha256"]
                != prior["new_candidate_sha256"]
            ):
                raise owner_campaign.CampaignError(
                    "stale rebase tombstone destination drift"
                )
            next_candidate = next_sealed[1]
        cleaned = _compact_terminal_input(root, campaign, descriptor_path)
        cleaned.extend(_cleanup_selection_input(root, campaign, selected))
        return _rebase_result(
            status=prior["status"],
            function=function,
            reason="recovered durable stale rebase disposition",
            old_descriptor=descriptor_path,
            old_descriptor_sha256=descriptor_file_sha,
            rebase_depth=prior["rebase_depth"],
            cleaned=cleaned,
            new_descriptor=next_path,
            new_candidate=next_candidate,
            new_candidate_sha256=prior.get("new_candidate_sha256"),
        )
    if old_depth >= 5:
        raise owner_campaign.CampaignError("stale candidate reached maximum rebase depth")

    base_path = _relative_path(root, descriptor["base_source"]["path"])
    if base_path is None:
        raise owner_campaign.CampaignError("stale candidate base snapshot is invalid")
    base_bytes = _stable_file_bytes(base_path, "stale candidate base source")
    candidate_bytes = _stable_file_bytes(candidate_path, "stale candidate source")
    current_path = _campaign_source_path(root, campaign)
    current_bytes = _stable_file_bytes(current_path, "current campaign source")
    if owner_campaign._digest_bytes(base_bytes) != descriptor["base_source"]["sha256"]:
        raise owner_campaign.CampaignError("stale candidate base snapshot drift")
    if owner_campaign._digest_bytes(candidate_bytes) != old_candidate_sha:
        raise owner_campaign.CampaignError("stale candidate source drift")
    try:
        base_text = base_bytes.decode("utf-8")
        candidate_text = candidate_bytes.decode("utf-8")
        current_text = current_bytes.decode("utf-8")
    except UnicodeError as exc:
        raise owner_campaign.CampaignError("stale rebase source is not UTF-8") from exc
    base_start, base_end, base_span = _function_span(base_text, function, "stale base function")
    candidate_start, candidate_end, candidate_span = _function_span(
        candidate_text, function, "stale candidate function"
    )
    current_start, current_end, current_span = _function_span(
        current_text, function, "current stale function"
    )
    span = descriptor["function_span"]
    if (
        (base_start, base_end, candidate_start, candidate_end)
        != (
            span["base_start_line"], span["base_end_line"],
            span["candidate_start_line"], span["candidate_end_line"],
        )
        or owner_campaign._digest_bytes(base_span) != span["base_sha256"]
        or owner_campaign._digest_bytes(candidate_span) != span["candidate_sha256"]
    ):
        raise owner_campaign.CampaignError("stale candidate function span drift")
    base_lines = base_text.splitlines(keepends=True)
    candidate_lines = candidate_text.splitlines(keepends=True)
    if (
        base_lines[: base_start - 1] != candidate_lines[: candidate_start - 1]
        or base_lines[base_end:] != candidate_lines[candidate_end:]
    ):
        raise owner_campaign.CampaignError("stale candidate edits escape its function")
    if current_span != base_span:
        raise owner_campaign.CampaignError(
            "current function changed; stale candidate cannot be rebased safely"
        )
    current_lines = current_text.splitlines(keepends=True)
    rebased_text = "".join(
        [
            *current_lines[: current_start - 1],
            *candidate_lines[candidate_start - 1:candidate_end],
            *current_lines[current_end:],
        ]
    )
    rebased_bytes = rebased_text.encode("utf-8")
    rebased_sha = owner_campaign._digest_bytes(rebased_bytes)
    if rebased_sha == owner_campaign._digest_bytes(current_bytes):
        raise owner_campaign.CampaignError("stale candidate is already present in current source")

    frontier, residual, current_counts = _current_residual_for_rebase(
        root, campaign, function, worker=worker
    )
    predicted_raw = selected.get("predicted_rows")
    if not isinstance(predicted_raw, list):
        raise owner_campaign.CampaignError("stale selection predicted rows are invalid")
    predicted = _remap_predicted_rows(predicted_raw, residual)
    old_expected = selected.get("expected_terminal")
    expected_terminal = (
        "exact"
        if old_expected == "exact" and set(predicted) == set(residual)
        else "improved"
    )
    remaining = {
        channel: current_counts[channel]
        - sum(row.startswith(f"{channel}:") for row in predicted)
        for channel in current_counts
    }

    existing = _find_rebased_proposal(
        root, campaign, function, frontier["frontier_sha256"], rebased_sha
    )
    temp_path: Path | None = None
    if existing is None:
        inbox = inbox_path(root, campaign)
        inbox.mkdir(parents=True, exist_ok=True)
        if not _allowed_candidate_path(root, campaign, inbox):
            raise owner_campaign.CampaignError(
                "campaign inbox is outside allowed build paths"
            )
        fd, raw = tempfile.mkstemp(prefix=".stale-rebase-", suffix=".c", dir=inbox)
        temp_path = Path(raw)
        try:
            with os.fdopen(fd, "wb") as stream:
                stream.write(rebased_bytes)
                stream.flush()
                os.fsync(stream.fileno())
            proposal = propose_candidate(
                root,
                campaign,
                function,
                temp_path,
                str(selected.get("source_class") or descriptor["hypothesis_family"]),
                expected_terminal=expected_terminal,
                predicted_rows=predicted,
                predicted_remaining_counts=(
                    {"strict": 0, "data": 0, "physical": 0}
                    if expected_terminal == "exact" else remaining
                ),
                rebase_depth=old_depth + 1,
                _required_current_function=base_span,
            )
        finally:
            temp_path.unlink(missing_ok=True)
        new_descriptor = _relative_path(root, proposal["candidate_descriptor"])
        if new_descriptor is None:
            raise owner_campaign.CampaignError("rebased proposal path is invalid")
    else:
        new_descriptor = existing
    new_descriptor_sha = owner_campaign._digest_file(new_descriptor)
    new_sealed = _sealed_descriptor(root, campaign, new_descriptor)
    if new_sealed is None:
        raise owner_campaign.CampaignError("rebased proposal did not seal")
    new_candidate = new_sealed[1]
    _publish_rebase_tombstone(
        root,
        campaign,
        function=function,
        status=REBASED_STATUS,
        old_descriptor_sha256=descriptor_file_sha,
        old_candidate_sha256=old_candidate_sha,
        old_frontier_sha256=old_frontier_sha,
        new_descriptor=new_descriptor.relative_to(root).as_posix(),
        new_descriptor_sha256=new_descriptor_sha,
        new_candidate_sha256=rebased_sha,
        rebase_depth=old_depth + 1,
        reason="disjoint current-source gain inherited and proposal evidence refreshed",
    )
    cleaned = _compact_terminal_input(root, campaign, descriptor_path)
    cleaned.extend(_cleanup_selection_input(root, campaign, selected))
    try:
        descriptor_path.parent.rmdir()
    except OSError:
        pass
    return _rebase_result(
        status=REBASED_STATUS,
        function=function,
        reason="stale candidate automatically rebased onto the current source",
        old_descriptor=descriptor_path,
        old_descriptor_sha256=descriptor_file_sha,
        rebase_depth=old_depth + 1,
        cleaned=cleaned,
        new_descriptor=new_descriptor,
        new_candidate=new_candidate,
        new_candidate_sha256=rebased_sha,
    )


def _reject_stale_rebase(
    root: Path,
    campaign: Mapping[str, Any],
    descriptor_path: Path,
    selected: Mapping[str, Any],
    reason: str,
) -> dict[str, Any]:
    sealed = _sealed_descriptor(root, campaign, descriptor_path)
    if sealed is None:
        return {
            "schema": "owner_campaign_result/v1",
            "status": "infra_retry",
            "reason": "stale descriptor vanished before deterministic rejection",
            "authority_advanced": False,
        }
    descriptor = sealed[0]
    descriptor_sha = owner_campaign._digest_file(descriptor_path)
    depth = descriptor["rebase_depth"]
    prior = _read_rebase_tombstone(
        _rebase_tombstone_path(root, campaign, descriptor_sha)
    )
    if prior is not None and prior["status"] != REBASE_REJECTED_STATUS:
        raise owner_campaign.CampaignError(
            "cannot replace a durable successful stale rebase disposition"
        )
    _publish_rebase_tombstone(
        root,
        campaign,
        function=descriptor["function"],
        status=REBASE_REJECTED_STATUS,
        old_descriptor_sha256=descriptor_sha,
        old_candidate_sha256=descriptor["candidate_source"]["sha256"],
        old_frontier_sha256=descriptor["base_frontier_sha256"],
        new_descriptor=None,
        new_descriptor_sha256=None,
        new_candidate_sha256=None,
        rebase_depth=depth,
        reason=reason,
    )
    cleaned = _compact_terminal_input(root, campaign, descriptor_path)
    cleaned.extend(_cleanup_selection_input(root, campaign, selected))
    try:
        descriptor_path.parent.rmdir()
    except OSError:
        pass
    return _rebase_result(
        status=REBASE_REJECTED_STATUS,
        function=descriptor["function"],
        reason=reason,
        old_descriptor=descriptor_path,
        old_descriptor_sha256=descriptor_sha,
        rebase_depth=depth,
        cleaned=cleaned,
    )


def _result_status(value: Any) -> str:
    return value.get("status", "unknown") if isinstance(value, Mapping) else "unknown"


def _group_descriptors_by_function(
    root: Path,
    campaign: Mapping[str, Any],
    descriptors: Sequence[Path],
) -> dict[str, list[Path]]:
    """Group a discovered v2 batch without weakening descriptor validation.

    ``discover_candidates`` has already performed the inexpensive sealed-input
    check.  Re-read the descriptor here because the selector is a second
    boundary and must never dispatch a path whose identity changed between the
    scan and arbitration.  A changed or deleted descriptor is an infrastructure
    error, rather than an opportunity to guess its function.
    """

    grouped: dict[str, list[Path]] = {}
    for descriptor_path in descriptors:
        sealed = _sealed_descriptor(root, campaign, descriptor_path)
        if sealed is None:
            raise owner_campaign.CampaignError(
                f"candidate descriptor changed during arbitration: {descriptor_path}"
            )
        descriptor = sealed[0]
        function = descriptor.get("function")
        if not isinstance(function, str) or not function:
            raise owner_campaign.CampaignError(
                f"candidate descriptor function is invalid: {descriptor_path}"
            )
        grouped.setdefault(function, []).append(descriptor_path)
    return grouped


def _pre_discovered_descriptor_batch(
    root: Path,
    campaign: Mapping[str, Any],
    descriptors: Sequence[Path],
) -> list[Path]:
    """Normalize a supervisor-owned discovery batch without rescanning it.

    The supervisor has already performed the sealed descriptor scan.  This
    boundary therefore checks only path shape/containment and preserves the
    scan order; selector arbitration performs the required identity reread
    immediately before selecting a proposal.  Keeping those responsibilities
    separate avoids a second full inbox scan while still preventing a caller
    from injecting an arbitrary path into the legacy dispatch path.
    """

    if isinstance(descriptors, (str, bytes)) or not isinstance(descriptors, Sequence):
        raise owner_campaign.CampaignError(
            "pre-discovered descriptor batch is invalid"
        )
    inbox = inbox_path(root, campaign)
    normalized: list[Path] = []
    seen: set[Path] = set()
    for raw in descriptors:
        if not isinstance(raw, (str, os.PathLike)):
            raise owner_campaign.CampaignError(
                "pre-discovered descriptor path is invalid"
            )
        try:
            path = _input_path(root, raw, "pre-discovered descriptor")
        except owner_campaign.CampaignError:
            raise
        if not _path_inside(inbox, path) or path.suffix.lower() != ".json":
            raise owner_campaign.CampaignError(
                "pre-discovered descriptor is outside campaign inbox"
            )
        if path in seen:
            raise owner_campaign.CampaignError(
                "pre-discovered descriptor batch contains duplicates"
            )
        seen.add(path)
        normalized.append(path)
    return normalized


def _relative_paths(root: Path, paths: Sequence[Path]) -> list[str]:
    """Render paths for lane output while preserving deterministic order."""

    return [path.relative_to(root).as_posix() for path in paths]


def _dispatch_selected_candidate(
    root: Path,
    campaign: Mapping[str, Any],
    descriptor_path: Path,
    *,
    worker: int,
) -> dict[str, Any]:
    """Measure one selected function without running per-cell maintenance.

    V2 selection and measurement form one function-local pipeline.  The core
    candidate runner still owns scratch isolation, source/frontier CAS, proof,
    and retention; only its storage-maintenance pass is deferred until every
    function pipeline in this bounded batch has finished.
    """

    try:
        return owner_campaign.run_candidate(
            root,
            campaign,
            descriptor_path,
            worker=worker,
            _defer_maintenance=True,
        )
    except owner_campaign.InfrastructureError as exc:
        return {
            "schema": "owner_campaign_result/v1",
            "status": "infra_retry",
            "candidate": str(descriptor_path),
            "reason": str(exc)[:1000],
            "authority_advanced": False,
        }


def _post_pipeline_maintenance(
    root: Path,
    campaign: Mapping[str, Any],
    results: Sequence[Mapping[str, Any]],
) -> None:
    """Run the one serialized maintenance pass after all v2 pipelines."""

    # Descriptor-only selector/replay fixtures intentionally omit the loaded
    # campaign's canonical limits block.  They have no production scratch or
    # retention tree to maintain.  Every load_campaign result has ``limits``.
    if "limits" not in campaign:
        return
    try:
        owner_campaign._check_limits(root, campaign)
    except BaseException as exc:
        error = owner_campaign.CampaignError(
            "batch maintenance failed after candidate results were finalized: "
            f"{exc}"
        )
        error.candidate_results = tuple(results)  # type: ignore[attr-defined]
        raise error from exc


def run_inbox(
    root: Path,
    campaign: Mapping[str, Any],
    *,
    max_candidates: int = DEFAULT_BATCH_SIZE,
    _pre_discovered: Sequence[Path] | None = None,
    _defer_maintenance: bool = False,
    _worker: int | None = None,
    _preselected_selection: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Dispatch one bounded inbox batch through the core campaign loop.

    ``_pre_discovered`` is an internal supervisor handoff.  When supplied,
    the batch is consumed as-is and is not rediscovered; v2 selector mode
    revalidates each descriptor at its grouping boundary before arbitration.
    """

    if type(max_candidates) is not int or not 1 <= max_candidates <= DEFAULT_BATCH_SIZE:
        raise ValueError(f"max_candidates must be between 1 and {DEFAULT_BATCH_SIZE}")
    if _worker is not None and (type(_worker) is not int or not 0 <= _worker < DEFAULT_BATCH_SIZE):
        raise ValueError(f"_worker must be between 0 and {DEFAULT_BATCH_SIZE - 1}")
    if _preselected_selection is not None and _pre_discovered is None:
        raise ValueError("_preselected_selection requires pre-discovered descriptors")
    root = Path(os.path.abspath(root))
    if _pre_discovered is None:
        # Scan a bounded slice per campaign function.  A single global slice
        # can contain only exhausted proposals from the first function and
        # falsely terminate a lane while another function has an eligible
        # winner.  The selector arbitrates the widened read-only pool; the
        # streaming supervisor may preselect one descriptor at a time from a
        # same-function group to keep every worker occupied.
        function_count = sum(
            isinstance(function, str) for function in campaign.get("functions", [])
        )
        scan_limit = max_candidates * max(1, function_count)
        descriptors = discover_candidates(root, campaign, limit=scan_limit)
    else:
        descriptors = _pre_discovered_descriptor_batch(
            root, campaign, _pre_discovered
        )
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
            "recorded_outcomes": [],
            "authority_advanced": False,
        }

    # A real v2 manifest always uses the selector.  The small descriptor-only
    # fallback is retained for old unit/replay callers that predate the
    # selection sidecar; it is unreachable for a loaded v2 campaign because
    # ``base_commit`` is mandatory there.  This keeps replay compatibility
    # without allowing a production campaign to compile an unranked batch.
    selection: dict[str, Any] | None = None
    selections: list[dict[str, Any]] = []
    selection_by_descriptor: dict[Path, Mapping[str, Any]] = {}
    selection_required = "base_commit" in campaign or any(
        any(path.is_file() for path in owner_campaign_selector.selection_evidence_paths(descriptor))
        for descriptor in descriptors
    )
    dispatch_descriptors = descriptors
    pipeline_results: list[dict[str, Any]] | None = None
    if selection_required and _preselected_selection is not None:
        current_selection = dict(_preselected_selection)
        if current_selection.get("status") != owner_campaign_selector.SELECTED:
            raise owner_campaign.CampaignError(
                "preselected streaming result is not selected"
            )
        selected = current_selection.get("selected")
        if not isinstance(selected, Mapping):
            raise owner_campaign.CampaignError(
                "preselected streaming result has no selection binding"
            )
        selected_raw = selected.get("descriptor_path")
        if not isinstance(selected_raw, str):
            raise owner_campaign.CampaignError(
                "preselected streaming result has no descriptor path"
            )
        selected_path = Path(selected_raw)
        if not selected_path.is_absolute():
            selected_path = root / selected_path
        selected_path = Path(os.path.abspath(selected_path))
        function_paths = {
            Path(os.path.abspath(path)) for path in descriptors
        }
        if selected_path not in function_paths:
            raise owner_campaign.CampaignError(
                "preselected descriptor is outside its streaming batch"
            )
        selection_required = True
        selections.append(current_selection)
        selection_by_descriptor[selected_path] = dict(selected)
        dispatch_descriptors = [selected_path]
        streaming_dispatch = "_source" in campaign and "limits" in campaign
        if streaming_dispatch:
            result = _dispatch_selected_candidate(
                root, campaign, selected_path, worker=_worker or 0,
            )
            pipeline_results = [result]
        else:
            pipeline_results = None
    elif selection_required:
        streaming_dispatch = "_source" in campaign and "limits" in campaign
        grouped = _group_descriptors_by_function(root, campaign, descriptors)
        # Each function group is independently arbitrated, and one selected
        # descriptor enters that function's measure pipeline.  The streaming
        # supervisor uses the same path repeatedly for eligible siblings; this
        # compatibility entry point still dispatches one winner per group.
        manifest_order = {
            function: index
            for index, function in enumerate(campaign.get("functions", []))
            if isinstance(function, str)
        }
        selected_groups = sorted(
            grouped.items(),
            key=lambda item: (manifest_order.get(item[0], len(manifest_order)), item[0]),
        )[:max_candidates]

        def select_and_dispatch(
            indexed_item: tuple[int, tuple[str, list[Path]]],
        ) -> dict[str, Any]:
            worker, item = indexed_item
            _function, function_descriptors = item

            def selection_failure(reason: str) -> dict[str, Any]:
                return {
                    "function": _function,
                    "selection": {
                        "status": owner_campaign_selector.UNKNOWN,
                        "reason": reason[:1000],
                    },
                    "descriptor": None,
                    "selected": None,
                    "result": None,
                }

            try:
                current_selection = owner_campaign_selector.select_winning_candidate(
                    root, campaign, function_descriptors
                )
            except Exception as exc:
                return selection_failure(
                    f"selector arbitration failed for {_function}: {exc}"
                )
            if not isinstance(current_selection, Mapping):
                return selection_failure(
                    f"selector returned an invalid result for {_function}"
                )
            current_selection = dict(current_selection)
            if current_selection.get("status") != owner_campaign_selector.SELECTED:
                return {
                    "function": _function,
                    "selection": current_selection,
                    "descriptor": None,
                    "selected": None,
                    "result": None,
                }
            selected = current_selection.get("selected")
            if not isinstance(selected, Mapping):
                return selection_failure(
                    "selector returned selected status without a selection binding"
                )
            if selected.get("function") != _function:
                return selection_failure(
                    "selector selected a descriptor with the wrong function binding"
                )
            selected_raw = selected.get("descriptor_path")
            selected_path = Path(selected_raw) if isinstance(selected_raw, str) else None
            if selected_path is None:
                return selection_failure(
                    "selector returned selected status without a descriptor path"
                )
            if not selected_path.is_absolute():
                selected_path = root / selected_path
            selected_path = Path(os.path.abspath(selected_path))
            function_paths = {
                Path(os.path.abspath(path)) for path in function_descriptors
            }
            if selected_path not in function_paths:
                return selection_failure(
                    "selector selected a descriptor outside its function group"
                )
            result: Mapping[str, Any] | None = None
            if streaming_dispatch:
                try:
                    result = _dispatch_selected_candidate(
                        root,
                        campaign,
                        selected_path,
                        worker=worker,
                    )
                except Exception as exc:
                    result = {
                        "schema": "owner_campaign_result/v1",
                        "status": "infra_retry",
                        "candidate": str(selected_path),
                        "function": _function,
                        "reason": f"candidate pipeline failed: {exc}"[:1000],
                        "authority_advanced": False,
                    }
                if not isinstance(result, Mapping):
                    result = {
                        "schema": "owner_campaign_result/v1",
                        "status": "infra_retry",
                        "candidate": str(selected_path),
                        "function": _function,
                        "reason": "candidate pipeline returned an invalid result",
                        "authority_advanced": False,
                    }
            return {
                "function": _function,
                "selection": current_selection,
                "descriptor": selected_path,
                "selected": dict(selected),
                "result": None if result is None else dict(result),
            }

        if _worker is None:
            indexed_groups = list(enumerate(selected_groups))
        else:
            # The streaming supervisor owns one persistent slot per pipeline.
            # Pass its stable slot through to the core scratch namespace rather
            # than creating a second worker assignment inside this single-cell
            # compatibility entry point.
            indexed_groups = [(_worker, selected_groups[0])] if selected_groups else []
        worker_count = min(DEFAULT_BATCH_SIZE, len(indexed_groups))
        with ThreadPoolExecutor(
            max_workers=worker_count,
            thread_name_prefix="owner-campaign-pipeline",
        ) as executor:
            completed_pipelines = list(
                executor.map(select_and_dispatch, indexed_groups)
            )

        dispatch_descriptors = []
        pipeline_results = [] if streaming_dispatch else None
        for pipeline in completed_pipelines:
            _function = pipeline["function"]
            current_selection = pipeline["selection"]
            selections.append(current_selection)
            selected_path = pipeline["descriptor"]
            selected = pipeline["selected"]
            result = pipeline["result"]
            if selected_path is None:
                continue
            if selected_path in selection_by_descriptor:
                raise owner_campaign.CampaignError(
                    "selector selected one descriptor more than once"
                )
            selection_by_descriptor[selected_path] = selected
            dispatch_descriptors.append(selected_path)
            if pipeline_results is not None:
                pipeline_results.append(result)
        if not dispatch_descriptors:
            selection_statuses = [item.get("status") for item in selections]
            lane_status = (
                "pivot_required"
                if owner_campaign_selector.PIVOT_REQUIRED in selection_statuses
                else "selection_unknown"
            )
            reasons = [
                str(item.get("reason", "no deterministic winner"))
                for item in selections
                if item.get("reason")
            ]
            return {
                "schema": LANE_RESULT_SCHEMA,
                "status": lane_status,
                "reason": "; ".join(reasons[:3]) or "no deterministic winner",
                "campaign_id": campaign["campaign_id"],
                "discovered": len(descriptors),
                "dispatched": 0,
                "results": [],
                "cleaned": [],
                "preserved_infrastructure": _relative_paths(root, descriptors),
                "selection": selections[0] if len(selections) == 1 else None,
                "selections": selections,
                "recorded_outcomes": [],
                "authority_advanced": False,
            }
        # Preserve the old scalar field when this was a one-function selection;
        # callers that understand the widened scheduler consume ``selections``.
        selection = selections[0] if len(selections) == 1 else None
    else:
        # The descriptor-only compatibility path has no selector to arbitrate
        # a widened cross-function scan, so retain its historical dispatch cap.
        dispatch_descriptors = descriptors[:max_candidates]

    if pipeline_results is not None:
        results = pipeline_results
    else:
        try:
            results = owner_campaign.run_loop(root, campaign, dispatch_descriptors)
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
                for path in dispatch_descriptors
            ]

    # A gain retained by one worker advances the shared translation-unit
    # source.  Other functions may have completed useful measurements against
    # the prior source in parallel; convert those stale results into refreshed
    # proposals concurrently instead of throwing the work away or forcing a
    # manager-controlled retry.  The descriptor-only legacy path intentionally
    # keeps its old preservation behavior because it has no selector evidence
    # with which to revalidate the row prediction.
    if selection_required:
        mutable_results = list(results)
        stale_jobs: list[tuple[int, Path, Mapping[str, Any]]] = []
        for index, descriptor_path in enumerate(dispatch_descriptors):
            if index >= len(mutable_results):
                continue
            if _result_status(mutable_results[index]) not in {"stale_rebase", "stale"}:
                continue
            selected = selection_by_descriptor.get(
                Path(os.path.abspath(descriptor_path))
            )
            if selected is None:
                raise owner_campaign.CampaignError(
                    "stale selected result has no selection binding"
                )
            stale_jobs.append((index, descriptor_path, selected))

        def rebase_job(
            item: tuple[int, Path, Mapping[str, Any]],
        ) -> tuple[int, dict[str, Any]]:
            index, descriptor_path, selected = item
            try:
                try:
                    value = _auto_rebase_stale_candidate(
                        root,
                        campaign,
                        descriptor_path,
                        selected,
                        worker=index % DEFAULT_BATCH_SIZE,
                    )
                except owner_campaign.InfrastructureError as exc:
                    value = {
                        "schema": "owner_campaign_result/v1",
                        "status": "infra_retry",
                        "function": selected.get("function"),
                        "candidate": str(descriptor_path),
                        "reason": f"automatic stale rebase infrastructure failure: {exc}"[:1000],
                        "authority_advanced": False,
                    }
                except owner_campaign.CampaignError as exc:
                    value = _reject_stale_rebase(
                        root, campaign, descriptor_path, selected, str(exc)
                    )
            except Exception as exc:
                # One failed rebase publication must not abort sibling jobs or
                # skip the batch's sole maintenance pass.  Preserve the input
                # for recovery and surface the failure in its function slot.
                value = {
                    "schema": "owner_campaign_result/v1",
                    "status": "infra_retry",
                    "function": selected.get("function"),
                    "candidate": str(descriptor_path),
                    "reason": f"automatic stale rebase failed: {exc}"[:1000],
                    "authority_advanced": False,
                }
            return index, value

        if stale_jobs:
            with ThreadPoolExecutor(
                max_workers=min(DEFAULT_BATCH_SIZE, len(stale_jobs)),
                thread_name_prefix="owner-campaign-rebase",
            ) as executor:
                for index, value in executor.map(rebase_job, stale_jobs):
                    mutable_results[index] = value
            results = mutable_results

    if pipeline_results is not None and not _defer_maintenance:
        # Candidate measurement and every stale-frontier refresh defer their
        # individual maintenance.  Run one serialized pass only after all
        # function pipelines and rebase jobs have reached inspectable results.
        _post_pipeline_maintenance(root, campaign, results)

    cleaned: list[str] = []
    cleanup_failures: list[str] = []
    dispatched_paths = {Path(os.path.abspath(path)) for path in dispatch_descriptors}
    preserved: list[str] = [
        path.relative_to(root).as_posix()
        for path in descriptors
        if Path(os.path.abspath(path)) not in dispatched_paths
    ]
    recorded_outcomes: list[dict[str, Any]] = []
    for index, descriptor_path in enumerate(dispatch_descriptors):
        result = results[index] if index < len(results) else {
            "status": "infra_retry",
            "reason": "core returned fewer results than dispatched",
        }
        status = _result_status(result)
        if status == REBASED_STATUS:
            if isinstance(result, Mapping):
                result_cleaned = result.get("cleaned", [])
                if isinstance(result_cleaned, list):
                    cleaned.extend(str(item) for item in result_cleaned)
                new_raw = result.get("new_descriptor")
                if isinstance(new_raw, str):
                    new_path = Path(new_raw)
                    if not new_path.is_absolute():
                        new_path = root / new_path
                    try:
                        preserved.append(
                            Path(os.path.abspath(new_path)).relative_to(root).as_posix()
                        )
                    except ValueError:
                        cleaned.append(f"cleanup-error:{new_path}:rebased path escapes root")
            continue
        if status == REBASE_REJECTED_STATUS:
            if isinstance(result, Mapping):
                result_cleaned = result.get("cleaned", [])
                if isinstance(result_cleaned, list):
                    cleaned.extend(str(item) for item in result_cleaned)
            continue
        if status in TERMINAL_STATUSES:
            selected = selection_by_descriptor.get(
                Path(os.path.abspath(descriptor_path))
            )
            if selection_required:
                if selected is None:
                    raise owner_campaign.CampaignError(
                        "terminal selected result has no selection binding"
                    )
                result_function = (
                    result.get("function")
                    if isinstance(result, Mapping)
                    else None
                )
                if (
                    result_function is not None
                    and result_function != selected.get("function")
                ):
                    raise owner_campaign.CampaignError(
                        "terminal result function does not bind to its selection"
                    )
                try:
                    outcome = owner_campaign_selector.append_selection_outcome(
                        root, campaign, selected, result
                    )
                except (owner_campaign_selector.SelectionError, OSError) as exc:
                    # Do not delete a descriptor/sidecar unless the compact
                    # source-class outcome is durable.  A missing outcome
                    # would make a measured no-gain invisible on recovery.
                    raise owner_campaign.CampaignError(
                        f"selection outcome publication failed: {exc}"
                    ) from exc
                recorded_outcomes.append(outcome)
            compacted = _compact_terminal_input(root, campaign, descriptor_path)
            cleaned.extend(compacted)
            cleanup_failures.extend(
                item for item in compacted
                if str(item).startswith("cleanup-error:")
            )
            if selected is not None:
                evidence_raw = selected.get("evidence_path")
                if not isinstance(evidence_raw, (str, os.PathLike)):
                    raise owner_campaign.CampaignError(
                        "terminal selected result has no evidence binding"
                    )
                evidence_path = Path(evidence_raw)
                if not evidence_path.is_absolute():
                    evidence_path = root / evidence_path
                evidence_path = Path(os.path.abspath(evidence_path))
                try:
                    if evidence_path.is_file():
                        evidence_path.unlink()
                        cleaned.append(evidence_path.relative_to(root).as_posix())
                except (OSError, ValueError) as exc:
                    failure = f"cleanup-error:{evidence_path}:{exc}"
                    cleaned.append(failure)
                    cleanup_failures.append(failure)
            if any(
                str(item).startswith("cleanup-error:") for item in compacted
            ) or any(
                str(item).startswith("cleanup-error:")
                and descriptor_path.name in str(item)
                for item in cleanup_failures
            ):
                preserved.append(descriptor_path.relative_to(root).as_posix())
        else:
            preserved.append(descriptor_path.relative_to(root).as_posix())

    statuses = [_result_status(item) for item in results]
    if cleanup_failures:
        status = "infra_retry"
    elif not statuses:
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
        "dispatched": len(dispatch_descriptors),
        "results": list(results),
        "cleaned": cleaned,
        "preserved_infrastructure": preserved,
        "selection": selection,
        "selections": selections,
        "recorded_outcomes": recorded_outcomes,
        "cleanup_failures": cleanup_failures,
        "authority_advanced": False,
    }


def _streaming_pipeline(
    root: Path,
    campaign: Mapping[str, Any],
    descriptor_paths: Sequence[Path] | Path,
    *,
    worker: int,
    preselection: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Run one selected descriptor in one persistent supervisor slot.

    ``run_inbox`` remains the compatibility/``--once`` entry point.  The
    streaming supervisor calls it with a one-descriptor group after doing
    read-only selector arbitration.  The complete ranked same-function group
    is selected before this call; passing that sealed selection through keeps
    rank selection, source/frontier CAS, and stale-rebase invariants in one
    implementation while allowing several descriptors for one function to
    occupy independent slots.
    """

    if isinstance(descriptor_paths, (str, os.PathLike)):
        paths = [Path(descriptor_paths)]
    else:
        paths = list(descriptor_paths)
    if not paths:
        raise owner_campaign.CampaignError(
            "streaming candidate pipeline received no descriptors"
        )
    run_kwargs: dict[str, Any] = {
        "max_candidates": 1,
        "_pre_discovered": paths,
        "_defer_maintenance": True,
        "_worker": worker,
    }
    if preselection is not None:
        run_kwargs["_preselected_selection"] = dict(preselection)
    value = run_inbox(root, campaign, **run_kwargs)
    if not isinstance(value, Mapping):
        raise owner_campaign.CampaignError(
            "streaming candidate pipeline returned an invalid lane result"
        )
    return dict(value)


def _streaming_failure_result(
    campaign: Mapping[str, Any],
    descriptor_path: Path,
    function: str,
    reason: str,
    *,
    root: Path | None = None,
) -> dict[str, Any]:
    """Represent one failed slot without masking unrelated slot outcomes."""

    display_path = str(descriptor_path)
    if root is not None:
        try:
            display_path = descriptor_path.relative_to(root).as_posix()
        except ValueError:
            display_path = str(descriptor_path)
    result = {
        "schema": "owner_campaign_result/v1",
        "status": "infra_retry",
        "candidate": display_path,
        "function": function,
        "reason": reason[:1000],
        "authority_advanced": False,
    }
    return {
        "schema": LANE_RESULT_SCHEMA,
        "status": "infra_retry",
        "campaign_id": campaign["campaign_id"],
        "discovered": 1,
        "dispatched": 1,
        "results": [result],
        "cleaned": [],
        "preserved_infrastructure": [display_path],
        "selection": None,
        "selections": [],
        "recorded_outcomes": [],
        "authority_advanced": False,
    }


def _is_source_advance_retryable(value: Any) -> bool:
    """Return whether a lane result is retryable after a source/frontier race.

    These messages are emitted by the core snapshot CAS when another worker
    retains a shared-TU gain while this worker is measuring.  Compiler,
    selector, and cleanup failures remain terminal for the descriptor; only
    the explicit snapshot publication races are put back into the scheduler's
    bounded inbox.
    """

    if not isinstance(value, Mapping):
        return False
    messages: list[str] = []
    for key in ("reason", "terminal_reason"):
        raw = value.get(key)
        if isinstance(raw, str):
            messages.append(raw)
    raw_results = value.get("results")
    if isinstance(raw_results, Sequence) and not isinstance(raw_results, (str, bytes)):
        for item in raw_results:
            if isinstance(item, Mapping):
                reason = item.get("reason")
                if isinstance(reason, str):
                    messages.append(reason)
    retryable_markers = (
        "frontier snapshot became stale before publication",
        "frontier advanced without matching the live source",
        "latest frontier disappeared during snapshot",
    )
    return any(marker in message for message in messages for marker in retryable_markers)


def _run_streaming_inbox(
    root: Path,
    campaign: Mapping[str, Any],
    *,
    max_candidates: int,
    initial_descriptors: Sequence[Path],
    poll_interval: float,
    clock: Callable[[], float],
    watchdog_deadline: float,
) -> dict[str, Any]:
    """Drain a v2 inbox with persistent, continuously refilled worker slots.

    The old supervisor waited for a complete five-function wave and claimed a
    function for the whole drain.  This scheduler refills every free slot with
    the next sealed descriptor, even when several descriptors belong to the
    same function.  A same-function group is still passed through the selector
    in deterministic rank order before each descriptor is submitted; the core
    runner remains the authority for source/frontier CAS and stale rebases.
    Only the batch-tail maintenance is deferred until this drain reaches a
    terminal boundary.
    """

    root = Path(os.path.abspath(root))
    if type(max_candidates) is not int or not 1 <= max_candidates <= DEFAULT_BATCH_SIZE:
        raise ValueError(f"max_candidates must be between 1 and {DEFAULT_BATCH_SIZE}")
    if not isinstance(initial_descriptors, Sequence):
        raise owner_campaign.CampaignError("streaming descriptor batch is invalid")

    function_count = sum(
        isinstance(function, str) for function in campaign.get("functions", [])
    )
    scan_limit = max_candidates * max(1, function_count)
    def normalize_path(raw_path: Path | str | os.PathLike[str]) -> Path:
        path = Path(raw_path)
        if not path.is_absolute():
            path = root / path
        return Path(os.path.abspath(path))

    initial = [normalize_path(path) for path in initial_descriptors]
    discovered: list[Path] = []
    discovered_set: set[Path] = set()
    attempted: set[Path] = set()
    source_race_retries: dict[Path, int] = {}
    initial_index = 0
    slot_ids = set(range(max_candidates))
    completed: dict[int, dict[str, Any]] = {}
    future_meta: dict[Any, tuple[int, Path, str, int, Mapping[str, Any] | None]] = {}
    next_sequence = 0
    boundary_status: str | None = None
    boundary_reason: str | None = None
    primary_error: BaseException | None = None

    def note_discovered(paths: Sequence[Path]) -> None:
        for raw_path in paths:
            path = normalize_path(raw_path)
            if path not in discovered_set:
                discovered_set.add(path)
                discovered.append(path)

    note_discovered(initial)

    def descriptor_function(path: Path) -> str | None:
        sealed = _sealed_descriptor(root, campaign, path)
        if sealed is None:
            return None
        function = sealed[0].get("function")
        return function if isinstance(function, str) and function else None

    def refill_source() -> list[Path]:
        nonlocal initial_index
        if initial_index < len(initial):
            paths = initial[initial_index:]
            initial_index = len(initial)
            return paths
        paths = discover_candidates(root, campaign, limit=scan_limit)
        note_discovered(paths)
        return paths

    def next_eligible() -> tuple[list[Path], str, Mapping[str, Any] | None] | None:
        # Re-scan after every completed slot.  ``discover_candidates`` is
        # bounded and deterministic; attempted paths prevent duplicate
        # dispatch while allowing same-function siblings to fill free slots.
        # The selector sees all currently eligible siblings for the chosen
        # function and returns the next deterministic winner.  Removing only
        # that winner from ``attempted`` leaves the remaining ranked siblings
        # available for later slots.
        for _ in range(2):
            groups: dict[str, list[Path]] = {}
            for path in refill_source():
                if path in attempted:
                    continue
                function = descriptor_function(path)
                if function is None:
                    continue
                groups.setdefault(function, []).append(path)
            if not groups:
                return None
            manifest_order = {
                function: index
                for index, function in enumerate(campaign.get("functions", []))
                if isinstance(function, str)
            }
            function = min(
                groups,
                key=lambda item: (manifest_order.get(item, len(manifest_order)), item),
            )
            group = groups[function]
            selection_required = "base_commit" in campaign or any(
                any(
                    path.is_file()
                    for path in owner_campaign_selector.selection_evidence_paths(path)
                )
                for path in group
            )
            if not selection_required:
                # Legacy descriptor-only callers do not carry selection
                # evidence.  Keep their historical one-slot dispatch shape;
                # production v2 campaigns always take the branch below.
                attempted.update(group)
                return group, function, None

            try:
                current_selection = owner_campaign_selector.select_winning_candidate(
                    root, campaign, group
                )
            except Exception:
                # Let run_inbox produce the canonical selector diagnostic for
                # this group.  Mark all inputs consumed for this drain so an
                # invalid/unknown group cannot spin forever.
                attempted.update(group)
                return group, function, None
            if not isinstance(current_selection, Mapping):
                attempted.update(group)
                return group, function, None
            current_selection = dict(current_selection)
            if current_selection.get("status") != owner_campaign_selector.SELECTED:
                attempted.update(group)
                return group, function, None
            selected = current_selection.get("selected")
            selected_raw = (
                selected.get("descriptor_path")
                if isinstance(selected, Mapping)
                else None
            )
            selected_path = Path(selected_raw) if isinstance(selected_raw, str) else None
            if selected_path is None:
                attempted.update(group)
                return group, function, None
            selected_path = normalize_path(selected_path)
            group_paths = {normalize_path(path) for path in group}
            if selected_path not in group_paths:
                attempted.update(group)
                return group, function, None
            attempted.add(selected_path)
            return [selected_path], function, current_selection
        return None

    def submit_available(executor: ThreadPoolExecutor) -> None:
        nonlocal next_sequence
        planned: list[
            tuple[int, list[Path], str, int, Mapping[str, Any] | None]
        ] = []
        while (
            boundary_status is None
            and len(future_meta) + len(planned) < max_candidates
            and slot_ids
        ):
            candidate = next_eligible()
            if candidate is None:
                break
            paths, function, preselection = candidate
            slot = min(slot_ids)
            slot_ids.remove(slot)
            sequence = next_sequence
            next_sequence += 1
            planned.append((sequence, list(paths), function, slot, preselection))

        # Select the entire initial wave before starting any pipeline.  This
        # avoids a fast same-function winner advancing the frontier before its
        # sibling has even been ranked, while still leaving stale-rebase/CAS to
        # the core runner once the workers begin.
        for sequence, paths, function, slot, preselection in planned:
            submit_kwargs: dict[str, Any] = {"worker": slot}
            if preselection is not None:
                submit_kwargs["preselection"] = preselection
            future = executor.submit(
                _streaming_pipeline,
                root,
                campaign,
                paths,
                **submit_kwargs,
            )
            future_meta[future] = (
                sequence, paths[0], function, slot, preselection
            )

    def consume_done(done: Sequence[Any]) -> None:
        for future in sorted(
            done,
            key=lambda item: future_meta.get(
                item, (10**9, Path("."), "", 0, None)
            )[0],
        ):
            meta = future_meta.pop(future, None)
            if meta is None:
                continue
            sequence, path, function, slot, _preselection = meta
            slot_ids.add(slot)
            value: Mapping[str, Any]
            try:
                raw_value = future.result()
                if not isinstance(raw_value, Mapping):
                    raise owner_campaign.CampaignError(
                        "streaming candidate pipeline returned an invalid lane result"
                    )
                value = dict(raw_value)
                completed[sequence] = value
            except BaseException as exc:
                value = _streaming_failure_result(
                    campaign,
                    path,
                    function,
                    f"streaming candidate pipeline failed: {exc}",
                    root=root,
                )
                completed[sequence] = value

            if _is_source_advance_retryable(value):
                retries = source_race_retries.get(path, 0)
                if retries < 1 and path.exists():
                    source_race_retries[path] = retries + 1
                    attempted.discard(path)

    executor = ThreadPoolExecutor(
        max_workers=max_candidates,
        thread_name_prefix="owner-campaign-stream",
    )
    try:
        submit_available(executor)
        while future_meta and boundary_status is None:
            try:
                owner_campaign._check_cancelled(root, campaign)
            except owner_campaign.CampaignError as exc:
                if "campaign is cancelled at the active epoch" in str(exc):
                    boundary_status = "cancelled"
                    boundary_reason = str(exc)
                    break
                primary_error = exc
                break
            remaining = max(0.0, watchdog_deadline - clock())
            if remaining <= 0.0:
                boundary_status = "watchdog_timeout"
                boundary_reason = "supervisor watchdog expired during streaming drain"
                break
            done, _pending = wait(
                tuple(future_meta),
                timeout=min(poll_interval, remaining),
                return_when=FIRST_COMPLETED,
            )
            if not done:
                continue
            consume_done(tuple(done))
            submit_available(executor)
    finally:
        # No new slot is submitted after cancellation/watchdog.  Running
        # compiler/evidence pipelines are allowed to finish so their existing
        # cleanup/CAS contracts can complete; queued futures are cancelled.
        if boundary_status is not None or primary_error is not None:
            for future in tuple(future_meta):
                future.cancel()
        executor.shutdown(wait=True, cancel_futures=True)
        remaining_done = [future for future in tuple(future_meta) if future.done()]
        consume_done(remaining_done)

    if primary_error is not None:
        raise primary_error

    ordered_lane_results = [completed[index] for index in sorted(completed)]
    candidate_results: list[Mapping[str, Any]] = []
    selections: list[Mapping[str, Any]] = []
    cleaned: list[str] = []
    preserved: list[str] = []
    recorded_outcomes: list[Mapping[str, Any]] = []
    dispatched = 0
    for lane_result in ordered_lane_results:
        dispatched += int(lane_result.get("dispatched", 0))
        raw_results = lane_result.get("results", [])
        if isinstance(raw_results, list):
            candidate_results.extend(
                item for item in raw_results if isinstance(item, Mapping)
            )
        raw_selections = lane_result.get("selections", [])
        if isinstance(raw_selections, list):
            selections.extend(
                item for item in raw_selections if isinstance(item, Mapping)
            )
        raw_cleaned = lane_result.get("cleaned", [])
        if isinstance(raw_cleaned, list):
            cleaned.extend(str(item) for item in raw_cleaned)
        raw_preserved = lane_result.get("preserved_infrastructure", [])
        if isinstance(raw_preserved, list):
            preserved.extend(str(item) for item in raw_preserved)
        raw_outcomes = lane_result.get("recorded_outcomes", [])
        if isinstance(raw_outcomes, list):
            recorded_outcomes.extend(
                item for item in raw_outcomes if isinstance(item, Mapping)
            )

    attempted_set = set(attempted)
    for path in discovered:
        if path in attempted_set:
            continue
        try:
            preserved_path = path.relative_to(root).as_posix()
        except ValueError:
            continue
        if path.exists() and preserved_path not in preserved:
            preserved.append(preserved_path)
    # Each pipeline's maintenance is deferred.  Even a selection-unknown
    # drain gets exactly one bounded maintenance pass at its terminal boundary.
    _post_pipeline_maintenance(root, campaign, candidate_results)

    if boundary_status is not None:
        status = boundary_status
    else:
        result_statuses = [_result_status(item) for item in candidate_results]
        selection_statuses = [item.get("status") for item in selections]
        if not result_statuses:
            if owner_campaign_selector.PIVOT_REQUIRED in selection_statuses:
                status = "pivot_required"
            elif selection_statuses:
                status = "selection_unknown"
            else:
                status = "idle"
        elif all(item == "infra_retry" for item in result_statuses):
            status = "infra_retry"
        else:
            status = "processed"

    result: dict[str, Any] = {
        "schema": LANE_RESULT_SCHEMA,
        "status": status,
        "campaign_id": campaign["campaign_id"],
        "discovered": len(discovered),
        "dispatched": dispatched,
        "results": candidate_results,
        "cleaned": cleaned,
        "preserved_infrastructure": preserved,
        "selection": selections[0] if len(selections) == 1 else None,
        "selections": selections,
        "recorded_outcomes": recorded_outcomes,
        "authority_advanced": False,
    }
    if boundary_status is not None:
        result["terminal_reason"] = boundary_reason or boundary_status
        result["_terminal_boundary"] = True
    return result


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
        # Supervisor polling only needs the exact-manifest count.  Avoid the
        # full per-function frontier/proof-CAS walk on every loop; explicit
        # status commands continue to use campaign_status for diagnostics.
        progress_reader = getattr(owner_campaign, "campaign_terminal_progress", None)
        if callable(progress_reader) and "_source" in campaign and "limits" in campaign:
            state = progress_reader(root, campaign)
        else:
            # Legacy in-memory fixtures and pre-v2 callers do not carry the
            # loaded campaign fields required by campaign_terminal_progress.
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

    The supervisor resolves the lane's sealed evidence through the deterministic
    winning-cell selector, dispatches at most one rank-1 candidate per function
    (up to five isolated functions), and waits with bounded exponential backoff
    when no candidate is supportable or an infrastructure retry is pending.
    Portfolio scope and cross-owner priorities remain outside the lane.
    ``--once`` uses :func:`run_inbox` instead for deterministic snapshots and
    tests.
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
    # A selector UNKNOWN/PIVOT is a function-local disposition, not a lane
    # terminal state.  Keep the sealed descriptors in the inbox, but suppress
    # their paths for this supervisor invocation so an unrankable function is
    # not selected repeatedly while other functions wait for a slot.
    deferred_selection_paths: set[Path] = set()

    def normalize_deferred_path(raw_path: Path | str | os.PathLike[str]) -> Path:
        path = Path(raw_path)
        if not path.is_absolute():
            path = root / path
        return Path(os.path.abspath(path))

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

        function_count = sum(
            isinstance(function, str) for function in campaign.get("functions", [])
        )
        scan_limit = max_candidates * max(1, function_count)
        # ``discover_candidates`` applies its limit before the deferred-path
        # filter below.  Include the deferred count in the read-only scan so a
        # fresh proposal is not hidden behind old UNKNOWN descriptors for the
        # same function.  The inbox remains bounded by the campaign limits;
        # this only widens discovery enough to skip already-seen paths.
        scan_limit += len(deferred_selection_paths)
        discovered_descriptors = discover_candidates(root, campaign, limit=scan_limit)
        descriptors = [
            path
            for path in discovered_descriptors
            if normalize_deferred_path(path) not in deferred_selection_paths
        ]
        if descriptors:
            try:
                if (
                    "base_commit" in campaign
                    and "_source" in campaign
                    and "limits" in campaign
                ):
                    # Loaded v2 campaigns use a continuously refilled pool of
                    # persistent slots.  The legacy branch below is retained
                    # for descriptor-only/replay callers and therefore keeps
                    # its historical whole-batch ``run_inbox`` semantics.
                    batch = _run_streaming_inbox(
                        root,
                        campaign,
                        max_candidates=max_candidates,
                        initial_descriptors=descriptors,
                        poll_interval=poll_interval,
                        clock=clock,
                        watchdog_deadline=started + watchdog,
                    )
                else:
                    batch = run_inbox(
                        root,
                        campaign,
                        max_candidates=max_candidates,
                        _pre_discovered=descriptors,
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
            if batch.get("_terminal_boundary"):
                return _terminal_result(
                    campaign,
                    status=last_batch_status,
                    reason=str(
                        batch.get("terminal_reason", last_batch_status)
                    )[:1000],
                    started=started,
                    clock=clock,
                    batches=batches,
                    dispatched=dispatched,
                    outcomes=outcomes,
                    last_batch_status=last_batch_status,
                )
            if last_batch_status in {"selection_unknown", "pivot_required"}:
                # No candidate was dispatched for this batch, so retain every
                # descriptor in the inbox but defer those exact identities for
                # this supervisor run.  A later, newly sealed descriptor for
                # the same function remains eligible; unrelated functions are
                # immediately discoverable and can use the freed slots.
                deferred_selection_paths.update(
                    normalize_deferred_path(path) for path in descriptors
                )
                infra_retries = 0
                now = clock()
                remaining = max(0.0, watchdog - (now - started))
                if remaining <= 0.0:
                    continue
                sleeper(min(delay, remaining))
                delay = min(MAX_BACKOFF_SECONDS, max(poll_interval, delay * 2))
                continue
            infra_statuses = (
                {"infra_retry"}
                if "base_commit" in campaign
                else {"infra_retry", "stale_rebase", "stale"}
            )
            infra_only = bool(batch_statuses) and all(
                status in infra_statuses for status in batch_statuses
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
    "PROPOSAL_RESULT_SCHEMA",
    "RECONSTRUCTION_RESULT_SCHEMA",
    "SUPERVISOR_RESULT_SCHEMA",
    "TERMINAL_STATUSES",
    "discover_candidates",
    "enqueue_candidate",
    "inbox_path",
    "propose",
    "propose_candidate",
    "queue_candidate",
    "reconstruct_frontier",
    "run_inbox",
    "run_owner_campaign_inbox",
    "run_supervisor",
    "submit_candidate",
]
