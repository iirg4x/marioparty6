"""Fail-closed, precompile selection for autonomous owner campaigns.

Five Luna workers may propose source cells for one owner, but the compiler must
see at most one proposal at a time.  A proposal is accompanied by a compact
``owner_campaign_selection/v1`` sidecar next to its candidate descriptor.  The
sidecar is evidence, not authority: it binds the current frontier, source,
unit, toolchain, focus/physical artifacts, residual row identities, predicted
remaining counts, and protected-sibling census.  This module never writes,
compiles, consumes, or deletes a candidate.  It may append a compact terminal
selection outcome after a selected measurement so the lane can enforce bounded
retries after the candidate sidecar is removed.

The companion sidecar convention is intentionally separate from
``owner_campaign_candidate/v1``.  Existing candidate descriptors therefore
remain loadable by the campaign runtime and replay fixtures do not need a
schema migration.  Accepted sidecars use one of two diagnostic classes:
``RANKED_SOURCE_CLASS`` or a matched owner-flow result.  All other inputs are
reported as ``UNKNOWN`` and are left untouched for the Sol lane to pivot.
"""

from __future__ import annotations

import json
import os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import re
from typing import Any, Mapping, Sequence

from . import owner_campaign
from . import owner_campaign_reconstruction


SCHEMA = "owner_campaign_selection/v1"
SELECTION_SCHEMA = SCHEMA
UNKNOWN = "UNKNOWN"
SELECTED = "selected"
MAX_PROPOSALS = 5
# Proposal validation is read-only.  Keep this bound aligned with the five-Luna
# proposal batch so arbitration can overlap I/O and hashing without creating an
# unbounded pool or changing the single-winner dispatch contract.
MAX_VALIDATION_WORKERS = 5
OUTCOME_SCHEMA = "owner_campaign_selection_outcome/v1"
SELECTION_OUTCOME_SCHEMA = OUTCOME_SCHEMA
# A source class is allowed one measured no-gain for a particular row group.
# Two no-gains for the normalized family close that family on the current
# frontier, while six no-gains exhaust the function frontier.  These limits
# are deliberately small: an exhausted function must pivot to another open
# function instead of consuming an unbounded syntax matrix.
MAX_NO_GAIN_PER_FAMILY = 2
MAX_NO_GAIN_PER_FUNCTION = 6
PIVOT_REQUIRED = "pivot_required"
_OUTCOME_LEDGER_NAMES = (
    "selection-outcomes.jsonl",
    "selection-ledger.jsonl",
    "selection-results.jsonl",
    "selection-outcome-ledger.jsonl",
    "candidate-selection-ledger.jsonl",
    "outcome-ledger.jsonl",
)
_MAX_OUTCOME_LEDGER_BYTES = 1024 * 1024
_MAX_OUTCOME_LEDGER_LINES = 4096
_SHA_RE = owner_campaign.SHA_RE
_AMBIGUOUS_CLASS_RE = re.compile(
    r"(?i)(?:ambiguous|unknown|unresolved|\bor\b|[|;,])"
)
_FORBIDDEN_CLASS_RE = re.compile(
    r"(?i)(?:dead|fake|padding|register[-_ ]?shap(?:e|ing)|inline[-_ ]?asm)"
)


class SelectionError(ValueError):
    """A proposal is malformed, stale, ambiguous, or incomplete."""


def _canonical_digest(value: Any) -> str:
    return owner_campaign._digest_json(value)


def _row_group_digest(rows: Sequence[str]) -> str:
    """Digest a stable, order-independent group of predicted row IDs."""

    return _canonical_digest(sorted(set(rows)))


def _selection_key(
    frontier_sha256: str, source_class: str, predicted_rows: Sequence[str],
) -> str:
    """Return the primary retry-suppression identity for one source cell.

    The source class and row group deliberately precede optional compiled
    hashes: a measured ``no_gain`` proves that the causal class is not useful
    on this frontier, so an equivalent restatement must not be dispatched
    again.  Object hashes are retained as optional ledger metadata below for
    duplicate auditing, but never weaken this primary suppression key.
    """

    return _canonical_digest({
        "frontier_sha256": frontier_sha256,
        "source_class": source_class,
        "predicted_row_group_sha256": _row_group_digest(predicted_rows),
    })


def _is_sha(value: Any) -> bool:
    return isinstance(value, str) and _SHA_RE.fullmatch(value) is not None


_COSMETIC_CLASS_SUFFIX_RE = re.compile(
    r"(?:[\s._:/-]+(?:v(?:er(?:sion)?)?|variant|candidate|cell|probe|"
    r"attempt|trial)?[\s._:/-]*\d+)+$",
    re.IGNORECASE,
)


def _normalize_source_class(value: Any) -> str:
    """Return the stable family identity for a proposed source class.

    Sol workers often decorate the same causal class with ``-v2``/``cell-3``
    labels.  Those labels must not evade the no-gain family budget.  Keep the
    semantic words, normalize punctuation/whitespace, and strip only a
    trailing numeric/cosmetic variant suffix.  Invalid values normalize to
    the empty string and are rejected by the normal source-class validator.
    """

    if not isinstance(value, str):
        return ""
    text = value.strip().lower()
    text = re.sub(r"[\s._:/\\-]+", " ", text)
    text = re.sub(r"\s+", " ", text).strip()
    while text:
        reduced = _COSMETIC_CLASS_SUFFIX_RE.sub("", text).strip()
        if reduced == text:
            break
        text = reduced
    return text


normalize_source_class = _normalize_source_class


def _path_inside(root: Path, path: Path) -> bool:
    try:
        path.relative_to(root)
        return True
    except ValueError:
        return False


def _bound_path(root: Path, raw: Any, label: str, *, require_file: bool = True) -> Path:
    if not isinstance(raw, str) or not raw or "\x00" in raw:
        raise SelectionError(f"{label} path is invalid")
    candidate = Path(raw)
    if candidate.is_absolute():
        path = Path(os.path.abspath(candidate))
    else:
        path = Path(os.path.abspath(root / candidate))
    if not _path_inside(root, path):
        raise SelectionError(f"{label} escapes the campaign root")
    current = root
    try:
        parts = path.relative_to(root).parts
    except ValueError as exc:
        raise SelectionError(f"{label} escapes the campaign root") from exc
    for part in parts:
        current = current / part
        if current.is_symlink():
            raise SelectionError(f"{label} uses symlink indirection")
    if require_file and not path.is_file():
        raise SelectionError(f"{label} is not a file")
    return path


def _read_json(path: Path, label: str) -> Mapping[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise SelectionError(f"{label} is unreadable") from exc
    if not isinstance(value, Mapping):
        raise SelectionError(f"{label} is not an object")
    return value


def _file_ref(value: Any, label: str, root: Path) -> tuple[Path, str]:
    if not isinstance(value, Mapping):
        raise SelectionError(f"{label} binding is missing")
    path_raw = value.get("path")
    digest = value.get("sha256", value.get("file_sha256"))
    if not _is_sha(digest):
        raise SelectionError(f"{label} hash is invalid")
    path = _bound_path(root, path_raw, label)
    try:
        actual = owner_campaign._digest_file(path)
    except OSError as exc:
        raise SelectionError(f"{label} cannot be hashed") from exc
    if actual != digest:
        raise SelectionError(f"{label} hash drift")
    return path, digest


def _verify_self_digest(value: Mapping[str, Any], label: str) -> None:
    digest = value.get("evidence_sha256")
    if not _is_sha(digest):
        raise SelectionError(f"{label} evidence_sha256 is invalid")
    body = dict(value)
    body.pop("evidence_sha256", None)
    if _canonical_digest(body) != digest:
        raise SelectionError(f"{label} evidence digest is invalid")


def selection_evidence_paths(descriptor_path: Path) -> tuple[Path, ...]:
    """Return supported sidecar names in deterministic priority order."""

    descriptor_path = Path(descriptor_path)
    return (
        descriptor_path.with_name(f"{descriptor_path.stem}.selection.json"),
        Path(f"{descriptor_path}.selection.json"),
        descriptor_path.parent / "selection" / f"{descriptor_path.stem}.json",
    )


def selection_evidence_path(descriptor_path: Path) -> Path:
    """Return the canonical sidecar path used for new proposals."""

    return selection_evidence_paths(descriptor_path)[0]


def selection_outcome_ledger_path(
    root: Path, campaign: Mapping[str, Any], function: str,
) -> Path:
    """Return the compact per-function outcome ledger location.

    The selector may create this file when the lane records a terminal
    measurement.  Owner-campaign/import tooling may also seed it with sealed
    outcomes so a fresh batch can suppress already measured ``no_gain`` classes
    without replaying a large historical transcript.
    """

    return owner_campaign._function_root(Path(root), campaign, function) / _OUTCOME_LEDGER_NAMES[0]


def selection_ledger_path(
    root: Path, campaign: Mapping[str, Any], function: str,
) -> Path:
    """Compatibility alias for the canonical compact outcome ledger."""

    return selection_outcome_ledger_path(root, campaign, function)


def _outcome_ledger_paths(
    root: Path, campaign: Mapping[str, Any], function: str,
) -> tuple[Path, ...]:
    """Return known compact-ledger paths in deterministic priority order."""

    root = Path(os.path.abspath(root))
    paths: list[Path] = []
    try:
        directories = [
            owner_campaign._function_root(root, campaign, function),
            owner_campaign._owner_root(root, campaign),
            owner_campaign._state_root(root),
        ]
    except (KeyError, TypeError):
        directories = [root / "build" / "owner-campaign"]
    # The first directory is function-scoped; owner/state paths are accepted
    # for migration importers that centralize compact records.
    for directory in directories:
        for name in _OUTCOME_LEDGER_NAMES:
            path = directory / name
            if path not in paths:
                paths.append(path)
    # The old dedupe ledger may contain extended records from an importer.  Its
    # native records are still digest-validated by owner_campaign; records that
    # lack the new key fields simply cannot suppress anything.
    try:
        candidate_results = owner_campaign._function_root(root, campaign, function) / "candidate-results.jsonl"
        if candidate_results not in paths:
            paths.append(candidate_results)
    except (KeyError, TypeError):
        pass
    return tuple(paths)


def _ledger_path(
    root: Path, path: Path, *, require_file: bool = True,
) -> Path:
    """Bind an internally discovered ledger path without following links."""

    try:
        relative = path.relative_to(root)
    except ValueError as exc:
        raise SelectionError("selection outcome ledger escapes campaign root") from exc
    return _bound_path(
        root, relative.as_posix(), "selection outcome ledger",
        require_file=require_file,
    )


def _ledger_records(path: Path) -> list[dict[str, Any]]:
    """Read a bounded JSONL ledger and fail closed on malformed sealed data."""

    try:
        size = path.stat().st_size
        if size > _MAX_OUTCOME_LEDGER_BYTES:
            raise SelectionError("selection outcome ledger exceeds compact limit")
        lines = path.read_text(encoding="utf-8").splitlines()
    except SelectionError:
        raise
    except (OSError, UnicodeError) as exc:
        raise SelectionError("selection outcome ledger is unreadable") from exc
    if len(lines) > _MAX_OUTCOME_LEDGER_LINES:
        raise SelectionError("selection outcome ledger exceeds line limit")
    records: list[dict[str, Any]] = []
    for line in lines:
        if not line.strip():
            continue
        try:
            value = json.loads(line)
        except json.JSONDecodeError as exc:
            raise SelectionError("selection outcome ledger is corrupt") from exc
        if not isinstance(value, Mapping):
            raise SelectionError("selection outcome ledger record is not an object")
        record = dict(value)
        # New compact records must be sealed.  The legacy candidate-results
        # schema is validated by its own canonical validator, while an
        # unsealed ad-hoc record is ignored rather than becoming suppression
        # authority.
        digest_field = next(
            (field for field in ("outcome_sha256", "selection_outcome_sha256", "result_sha256") if field in record),
            None,
        )
        if digest_field is not None:
            digest = record.pop(digest_field)
            if not _is_sha(digest) or _canonical_digest(record) != digest:
                raise SelectionError("selection outcome ledger record digest is invalid")
            record[digest_field] = digest
        records.append(record)
    return records


def _read_selection_outcomes(
    root: Path, campaign: Mapping[str, Any], function: str,
) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    seen_paths: set[Path] = set()
    for raw_path in _outcome_ledger_paths(root, campaign, function):
        try:
            path = _ledger_path(root, raw_path)
        except SelectionError:
            continue
        if path in seen_paths or not path.is_file():
            continue
        seen_paths.add(path)
        # Native candidate-results are strictly validated, including duplicate
        # candidate keys.  Extended records can still be consumed by the
        # generic parser when they carry the optional source-class fields.
        if path.name == "candidate-results.jsonl":
            try:
                legacy = owner_campaign._dedupe_records(path)
            except owner_campaign.CampaignError as exc:
                raise SelectionError(f"selection outcome ledger is invalid: {path}") from exc
            records.extend(dict(item) for item in legacy)
        else:
            records.extend(_ledger_records(path))
    return records


def _record_source_class(record: Mapping[str, Any]) -> str:
    """Read and normalize a source class from a compact outcome record."""

    for value in (
        record.get("source_class"),
        record.get("hypothesis_family"),
        record.get("source_class_normalized"),
    ):
        normalized = _normalize_source_class(value)
        if normalized:
            return normalized
    nested = record.get("selection")
    if isinstance(nested, Mapping):
        return _record_source_class(nested)
    return ""


def _record_frontier(record: Mapping[str, Any]) -> str | None:
    return _record_hash(
        record, "frontier_sha256", "base_frontier_sha256", "current_frontier_sha256",
    )


def _record_selection_key(record: Mapping[str, Any]) -> str | None:
    key = _record_hash(record, "selection_key_sha256", "suppression_key_sha256")
    if key is not None:
        return key
    frontier = _record_frontier(record)
    source_class = _record_source_class(record)
    rows = _record_rows(record)
    if frontier is None or not source_class or rows is None:
        return None
    return _selection_key(frontier, source_class, rows)


def _record_identity(record: Mapping[str, Any]) -> tuple[str, ...]:
    """Return a compact identity used to de-duplicate migrated ledgers."""

    frontier = _record_frontier(record) or ""
    function = record.get("function")
    status = record.get("status")
    candidate = record.get("candidate_source_sha256", record.get("candidate_sha256"))
    if _is_sha(candidate):
        return (str(function or ""), frontier, str(status or ""), str(candidate))
    key = _record_selection_key(record)
    if key is not None:
        return (str(function or ""), frontier, str(status or ""), key)
    result = _record_hash(record, "result_sha256", "outcome_sha256", "selection_outcome_sha256")
    return (str(function or ""), frontier, str(status or ""), result or _canonical_digest(dict(record)))


def _no_gain_records(
    root: Path, campaign: Mapping[str, Any], function: str, frontier_sha256: str,
) -> list[dict[str, Any]]:
    """Return distinct measured no-gains on one function frontier."""

    records: list[dict[str, Any]] = []
    seen: set[tuple[str, ...]] = set()
    for record in _read_selection_outcomes(root, campaign, function):
        if str(record.get("status", "")).lower() != "no_gain":
            continue
        record_function = record.get("function")
        if record_function not in {None, function}:
            continue
        if record.get("campaign_id") not in {None, campaign.get("campaign_id")}:
            continue
        if record.get("owner") not in {None, campaign.get("owner")}:
            continue
        if _record_frontier(record) != frontier_sha256:
            continue
        identity = _record_identity(record)
        if identity in seen:
            continue
        seen.add(identity)
        records.append(record)
    return records


def _family_no_gain_count(
    root: Path, campaign: Mapping[str, Any], function: str,
    frontier_sha256: str, source_class: str,
) -> int:
    family = _normalize_source_class(source_class)
    if not family:
        return 0
    return sum(
        1 for record in _no_gain_records(root, campaign, function, frontier_sha256)
        if _record_source_class(record) == family
    )


def _function_no_gain_count(
    root: Path, campaign: Mapping[str, Any], function: str,
    frontier_sha256: str,
) -> int:
    return len(_no_gain_records(root, campaign, function, frontier_sha256))


def _result_hash(result: Mapping[str, Any], selection: Mapping[str, Any]) -> str:
    """Use the core result seal, with a compact deterministic fallback for tests."""

    supplied = result.get("result_sha256")
    if supplied is not None:
        if not _is_sha(supplied):
            raise SelectionError("selection outcome result hash is invalid")
        return str(supplied)
    # Do not persist the result payload.  The fallback only identifies the
    # terminal event using fields that are already compact and non-source data.
    body = {
        "status": result.get("status"),
        "candidate_key": result.get("candidate_key"),
        "frontier_sha256": result.get("frontier_sha256", selection.get("frontier_sha256")),
        "metrics": result.get("metrics"),
    }
    return _canonical_digest(body)


def append_selection_outcome(
    root: Path,
    campaign: Mapping[str, Any],
    selection: Mapping[str, Any],
    result: Mapping[str, Any],
) -> dict[str, Any]:
    """Append one sealed terminal selection outcome under an exclusive lock.

    This ledger is intentionally separate from the core candidate-results
    ledger: it preserves the source class and predicted row group after the
    selector sidecar is compacted.  The selection key is idempotent, so a
    crash after publication and before cleanup cannot duplicate a record on
    recovery.  Only hashes/status/row IDs are retained; source and object
    bytes (and their paths) never enter the compact record.
    """

    root = Path(os.path.abspath(root))
    required = (
        "function", "frontier_sha256",
        "source_class", "predicted_rows", "selection_key_sha256",
        "candidate_sha256",
    )
    if any(key not in selection for key in required):
        raise SelectionError("selection outcome binding is incomplete")
    function = selection["function"]
    source_class = selection["source_class"]
    normalized_class = _normalize_source_class(source_class)
    rows = _rows(selection["predicted_rows"], "selection outcome predicted rows")
    frontier = selection["frontier_sha256"]
    selection_key = selection["selection_key_sha256"]
    candidate_sha = selection["candidate_sha256"]
    campaign_id = selection.get("campaign_id", campaign.get("campaign_id"))
    owner = selection.get("owner", campaign.get("owner"))
    unit = selection.get("unit", campaign.get("unit"))
    if (
        campaign_id != campaign.get("campaign_id")
        or not isinstance(owner, str)
        or owner != campaign.get("owner")
        or not isinstance(unit, str)
        or unit != campaign.get("unit")
        or not isinstance(function, str)
        or not _is_sha(frontier)
        or not normalized_class
        or not _is_sha(selection_key)
        or not _is_sha(candidate_sha)
    ):
        raise SelectionError("selection outcome identity is invalid")
    expected_key = _selection_key(frontier, source_class, rows)
    normalized_key = _selection_key(frontier, normalized_class, rows)
    if selection_key not in {expected_key, normalized_key}:
        raise SelectionError("selection outcome key is not bound to the selection")
    status = result.get("status")
    if not isinstance(status, str) or status not in {
        "deduplicated", "discarded", "exact", "improved", "no_gain",
    }:
        raise SelectionError("selection outcome status is not terminal")
    result_sha = _result_hash(result, selection)
    body: dict[str, Any] = {
        "schema": OUTCOME_SCHEMA,
        "campaign_id": campaign_id,
        "owner": owner,
        "unit": unit,
        "function": function,
        "frontier_sha256": frontier,
        "source_class": source_class,
        "source_class_normalized": normalized_class,
        "predicted_rows": rows,
        "predicted_row_group_sha256": _row_group_digest(rows),
        "selection_key_sha256": selection_key,
        "candidate_source_sha256": candidate_sha,
        "candidate_identity_sha256": selection.get("candidate_identity_sha256"),
        "status": status,
        "result_sha256": result_sha,
        "recorded_at": owner_campaign._now(),
    }
    if not _is_sha(body["candidate_identity_sha256"]):
        body.pop("candidate_identity_sha256")
    record = {**body, "outcome_sha256": _canonical_digest(body)}
    ledger = _ledger_path(
        root,
        selection_outcome_ledger_path(root, campaign, function),
        require_file=False,
    )
    lock_path = ledger.with_name(f"{ledger.name}.lock")
    timeout = owner_campaign._command_timeout_seconds(campaign)
    with owner_campaign._exclusive_lock(lock_path, timeout):
        existing = _ledger_records(ledger) if ledger.is_file() else []
        # The selection key is the idempotency token.  Return an identical
        # existing record after a recovery; conflicting bindings fail closed.
        for prior in existing:
            if _record_selection_key(prior) != selection_key:
                continue
            comparable = {
                key: prior.get(key)
                for key in (
                    "campaign_id", "owner", "unit", "function", "frontier_sha256",
                    "source_class_normalized", "predicted_row_group_sha256",
                    "selection_key_sha256", "candidate_source_sha256", "status",
                    "result_sha256",
                )
            }
            expected = {
                key: record.get(key)
                for key in comparable
            }
            if comparable != expected:
                raise SelectionError("selection outcome idempotency conflict")
            return prior
        if len(existing) >= _MAX_OUTCOME_LEDGER_LINES:
            raise SelectionError("selection outcome ledger exceeds line limit")
        try:
            current_bytes = ledger.read_bytes() if ledger.is_file() else b""
        except OSError as exc:
            raise SelectionError("selection outcome ledger is unreadable") from exc
        if current_bytes and not current_bytes.endswith(b"\n"):
            raise SelectionError("selection outcome ledger has an unterminated record")
        payload = _canonical_digest(record)  # force canonicalization before I/O
        del payload
        line = owner_campaign._canonical(record) + b"\n"
        if len(current_bytes) + len(line) > _MAX_OUTCOME_LEDGER_BYTES:
            raise SelectionError("selection outcome ledger exceeds compact limit")
        ledger.parent.mkdir(parents=True, exist_ok=True)
        try:
            with ledger.open("ab") as stream:
                stream.write(line)
                stream.flush()
                os.fsync(stream.fileno())
        except OSError as exc:
            raise SelectionError("selection outcome ledger publication failed") from exc
    return record


def _record_rows(record: Mapping[str, Any]) -> list[str] | None:
    for key in ("predicted_rows", "predicted_row_ids", "row_group", "predicted_row_group"):
        value = record.get(key)
        if isinstance(value, list):
            if all(isinstance(item, str) and item for item in value):
                return list(value)
            return None
        if isinstance(value, Mapping):
            nested_rows = value.get("rows", value.get("ids"))
            if isinstance(nested_rows, list) and all(
                isinstance(item, str) and item for item in nested_rows
            ):
                return list(nested_rows)
    nested = record.get("selection")
    if isinstance(nested, Mapping):
        return _record_rows(nested)
    return None


def _record_hash(record: Mapping[str, Any], *keys: str) -> str | None:
    for key in keys:
        value = record.get(key)
        if value is not None:
            return str(value) if _is_sha(value) else None
    return None


def _prior_no_gain(
    root: Path, campaign: Mapping[str, Any], function: str,
    frontier_sha256: str, source_class: str, predicted_rows: Sequence[str],
) -> dict[str, Any] | None:
    """Find a sealed prior no-gain for the same causal selection identity."""

    key = _selection_key(frontier_sha256, source_class, predicted_rows)
    normalized_class = _normalize_source_class(source_class)
    row_group = _row_group_digest(predicted_rows)
    for record in _read_selection_outcomes(root, campaign, function):
        status = record.get("status")
        if not isinstance(status, str) or status.lower() != "no_gain":
            # Infrastructure failures and successful improvements remain
            # retryable/eligible; only a measured no-gain suppresses a class.
            continue
        if record.get("function") not in {None, function}:
            continue
        if record.get("campaign_id") not in {None, campaign.get("campaign_id")}:
            continue
        if record.get("owner") not in {None, campaign.get("owner")}:
            continue
        record_frontier = _record_hash(
            record, "frontier_sha256", "base_frontier_sha256", "current_frontier_sha256",
        )
        record_class = _record_source_class(record)
        record_rows = _record_rows(record)
        record_group = _record_hash(
            record, "predicted_row_group_sha256", "row_group_sha256",
            "predicted_rows_sha256", "predicted_row_group",
        )
        record_key = _record_hash(record, "selection_key_sha256", "suppression_key_sha256")
        if record_frontier is not None and record_frontier != frontier_sha256:
            continue
        if record_class != normalized_class:
            continue
        if record_rows is not None:
            if len(record_rows) != len(set(record_rows)) or _row_group_digest(record_rows) != row_group:
                continue
        elif record_group != row_group:
            continue
        # The normalized class/row-group identity is authoritative.  A raw
        # selection key may differ only because a worker used a cosmetic
        # ``-v2`` suffix; that must still be suppressed.
        if record_key is None or record_key == key or record_class == normalized_class:
            return record
    return None


def _budget_reason(
    root: Path, campaign: Mapping[str, Any], function: str,
    frontier_sha256: str, source_class: str,
) -> str | None:
    """Return a deterministic pivot reason when a frontier budget is closed."""

    function_count = _function_no_gain_count(root, campaign, function, frontier_sha256)
    if function_count >= MAX_NO_GAIN_PER_FUNCTION:
        return (
            f"function no_gain budget exhausted on frontier "
            f"({function_count}/{MAX_NO_GAIN_PER_FUNCTION} compiled candidates)"
        )
    family_count = _family_no_gain_count(
        root, campaign, function, frontier_sha256, source_class
    )
    if family_count >= MAX_NO_GAIN_PER_FAMILY:
        return (
            f"hypothesis family no_gain budget exhausted on frontier "
            f"({family_count}/{MAX_NO_GAIN_PER_FAMILY}; "
            f"family={_normalize_source_class(source_class)})"
        )
    return None


def _find_selection_evidence(root: Path, descriptor_path: Path) -> Path:
    for path in selection_evidence_paths(descriptor_path):
        if not _path_inside(root, Path(os.path.abspath(path))):
            continue
        if path.is_file():
            return path
    raise SelectionError("selection evidence sidecar is missing")


def _descriptor(root: Path, path: Path, campaign: Mapping[str, Any]) -> dict[str, Any]:
    path = _bound_path(root, str(path), "candidate descriptor")
    value = _read_json(path, "candidate descriptor")
    required_fields = set(
        getattr(owner_campaign, "CANDIDATE_FIELDS", frozenset())
    )
    optional_fields = set(
        getattr(owner_campaign, "CANDIDATE_OPTIONAL_FIELDS", frozenset())
    )
    observed_fields = set(value)
    if (
        not required_fields <= observed_fields
        or not observed_fields <= required_fields | optional_fields
    ):
        raise SelectionError("candidate descriptor is not a closed object")
    body = dict(value)
    digest = body.pop("candidate_sha256", None)
    if not _is_sha(digest) or _canonical_digest(body) != digest:
        raise SelectionError("candidate descriptor digest is invalid")
    if (
        value.get("schema") != owner_campaign.CANDIDATE_SCHEMA
        or value.get("campaign_id") != campaign.get("campaign_id")
        or value.get("natural_c") is not True
        or not isinstance(value.get("function"), str)
    ):
        raise SelectionError("candidate descriptor identity is invalid")
    if value["function"] not in campaign.get("functions", [value["function"]]):
        raise SelectionError("candidate function is outside campaign scope")
    source = value.get("candidate_source")
    if not isinstance(source, Mapping) or set(source) != {"path", "sha256"}:
        raise SelectionError("candidate source binding is invalid")
    source_path = _bound_path(root, source.get("path"), "candidate source")
    source_sha = source.get("sha256")
    if not _is_sha(source_sha) or owner_campaign._digest_file(source_path) != source_sha:
        raise SelectionError("candidate source hash drift")
    rebase_depth = value.get("rebase_depth")
    if type(rebase_depth) is not int or not 0 <= rebase_depth <= 5:
        raise SelectionError("candidate rebase_depth must be between 0 and 5")
    base = value.get("base_source")
    if not isinstance(base, Mapping) or set(base) != {"path", "sha256"}:
        raise SelectionError("candidate base source binding is invalid")
    base_path = _bound_path(root, base.get("path"), "candidate base source")
    base_sha = base.get("sha256")
    if not _is_sha(base_sha):
        raise SelectionError("candidate base source sha256 is invalid")
    try:
        base_actual = owner_campaign._digest_file(base_path)
    except OSError as exc:
        raise SelectionError("candidate base source cannot be hashed") from exc
    if base_actual != base_sha:
        raise SelectionError("candidate base source hash drift")
    allowed_build_paths = campaign.get("allowed_build_paths")
    if not isinstance(allowed_build_paths, (list, tuple)):
        raise SelectionError("candidate base source allowed paths are missing")
    allowed_build_roots = [
        _bound_path(root, item, "allowed candidate base path", require_file=False)
        for item in allowed_build_paths
    ]
    if not any(
        base_path == allowed or _path_inside(allowed, base_path)
        for allowed in allowed_build_roots
    ):
        raise SelectionError("candidate base source is outside campaign allowed build paths")
    return {
        "path": path,
        "path_relative": path.relative_to(root).as_posix(),
        "descriptor": dict(value),
        "descriptor_sha256": digest,
        "source_path": source_path,
        "source_relative": source_path.relative_to(root).as_posix(),
        "source_sha256": source_sha,
        "base_source_path": base_path,
        "base_source_relative": base_path.relative_to(root).as_posix(),
        "base_source_sha256": base_sha,
        "rebase_depth": rebase_depth,
    }


def _frontier_file(root: Path, campaign: Mapping[str, Any], function: str) -> Mapping[str, Any] | None:
    """Read an existing frontier without invoking snapshot/measurement."""

    try:
        path = owner_campaign._function_root(root, campaign, function) / "latest-frontier.json"
    except (KeyError, TypeError):
        path = Path()
    if path and path.is_file():
        value = _read_json(path, "latest frontier")
        try:
            return owner_campaign._validate_frontier(value, campaign, function)
        except owner_campaign.CampaignError as exc:
            raise SelectionError(f"current frontier is invalid: {exc}") from exc
    # Pure unit callers can provide a sealed in-memory frontier.  Production
    # Manifests always have a persisted frontier and never take this branch.
    # Unit/replay callers may provide one sealed frontier per function.  Keep
    # the legacy single-frontier form as a compatibility fallback, but never
    # reuse it for a different function when a per-function map is available.
    supplied = None
    supplied_by_function = campaign.get("_selection_frontiers")
    if isinstance(supplied_by_function, Mapping):
        candidate = supplied_by_function.get(function)
        if isinstance(candidate, Mapping):
            supplied = candidate
    if supplied is None:
        candidate = campaign.get("_selection_frontier")
        if isinstance(candidate, Mapping):
            supplied = candidate
    if isinstance(supplied, Mapping):
        digest = supplied.get("frontier_sha256")
        body = dict(supplied)
        body.pop("frontier_sha256", None)
        if not _is_sha(digest) or _canonical_digest(body) != digest:
            raise SelectionError("in-memory current frontier digest is invalid")
        if supplied.get("function") != function:
            raise SelectionError("in-memory current frontier function mismatch")
        return dict(supplied)
    return None


def _frontier_ref(value: Mapping[str, Any]) -> dict[str, Any]:
    nested = value.get("frontier")
    if isinstance(nested, Mapping):
        result = dict(nested)
    else:
        result = {}
    for key, aliases in (
        ("sha256", ("frontier_sha256",)),
        ("source_sha256", ("current_source_sha256",)),
        ("function", ()),
        ("unit", ()),
        ("toolchain_sha256", ("toolchain",)),
    ):
        if key not in result:
            for alias in aliases:
                if alias in value:
                    result[key] = value[alias]
                    break
    if not _is_sha(result.get("sha256")):
        raise SelectionError("selection frontier hash is missing")
    for key in ("source_sha256", "toolchain_sha256"):
        if not _is_sha(result.get(key)):
            raise SelectionError(f"selection frontier {key} is missing")
    if not isinstance(result.get("function"), str) or not result["function"]:
        raise SelectionError("selection frontier function is missing")
    if not isinstance(result.get("unit"), str) or not result["unit"]:
        raise SelectionError("selection frontier unit is missing")
    return result


def _load_artifact(root: Path, ref: Any, label: str) -> tuple[Path, str, dict[str, Any]]:
    path, digest = _file_ref(ref, label, root)
    value = dict(_read_json(path, label))
    # Both the campaign focus CAS and the current-residual focus artifacts use
    # a self digest.  If present, it must be valid; simple test/owner physical
    # summaries may only have the file binding and are still hash-bound here.
    for field in ("focus_evidence_sha256", "artifact_sha256", "physical_summary_sha256"):
        if field in value:
            internal = value[field]
            body = dict(value)
            body.pop(field, None)
            if not _is_sha(internal) or _canonical_digest(body) != internal:
                raise SelectionError(f"{label} self-digest is invalid")
            break
    return path, digest, value


def _rows(value: Any, label: str) -> list[str]:
    if not isinstance(value, list) or len(value) != len(set(value)):
        raise SelectionError(f"{label} must be a unique row-id list")
    if any(not isinstance(item, str) or not item or len(item) > 512 for item in value):
        raise SelectionError(f"{label} contains an invalid row id")
    return list(value)


def _artifact_row_groups(focus: Mapping[str, Any], physical: Mapping[str, Any]) -> tuple[list[str], list[str], list[str]]:
    strict = focus.get("strict_row_ids", focus.get("strict_rows"))
    data = focus.get("data_row_ids", focus.get("data_rows"))
    physical_rows = focus.get("physical_difference_ids", focus.get("physical_differences"))
    strict_rows = _rows(strict, "focus strict rows") if isinstance(strict, list) else []
    data_rows = _rows(data, "focus data rows") if isinstance(data, list) else []
    physical_ids = _rows(physical_rows, "focus physical rows") if isinstance(physical_rows, list) else []

    if not physical_ids:
        for key in ("physical_difference_ids", "residual_rows"):
            candidate = physical.get(key)
            if isinstance(candidate, list):
                physical_ids = _rows(candidate, f"physical {key}")
                break
        if not physical_ids and isinstance(physical.get("differences"), list):
            # A physical summary that already emits stable IDs may call the
            # field ``differences``.  Do not manufacture IDs from raw details.
            differences = physical["differences"]
            if all(isinstance(item, str) for item in differences):
                physical_ids = _rows(differences, "physical differences")

    direct = focus.get("residual_rows")
    if isinstance(direct, list):
        all_rows = _rows(direct, "focus residual rows")
        known = set(strict_rows) | set(data_rows) | set(physical_ids)
        if known and set(all_rows) != known:
            raise SelectionError("focus residual rows do not match channel row IDs")
        if not known:
            # Keep a direct compact residual usable when channels are omitted.
            strict_rows = list(all_rows)
    if not strict_rows and not data_rows and not physical_ids:
        raise SelectionError("focus has no stable residual row IDs")
    return strict_rows, data_rows, physical_ids


def _ordered_union(*groups: Sequence[str]) -> list[str]:
    result: list[str] = []
    seen: set[str] = set()
    for group in groups:
        for row in group:
            if row not in seen:
                seen.add(row)
                result.append(row)
    return result


def _first_mismatch_rows(
    strict_rows: Sequence[str],
    data_rows: Sequence[str],
) -> list[str]:
    """Return the mandatory first residual in each instruction channel.

    Stable focus row IDs preserve target instruction order.  Strict and data
    are separate proof views of the same function, so a candidate must own the
    first row in both channels when both remain.  Physical-only differences
    have no comparable instruction chronology and are handled separately.
    """

    return [rows[0] for rows in (strict_rows, data_rows) if rows]


def _candidate_reference(value: Mapping[str, Any], proposal: Mapping[str, Any]) -> tuple[str | None, str]:
    nested = value.get("candidate")
    candidate = dict(nested) if isinstance(nested, Mapping) else {}
    path = candidate.get("path", value.get("candidate_path"))
    digest = candidate.get("sha256", value.get("candidate_sha256"))
    if path is not None and not isinstance(path, str):
        raise SelectionError("selection candidate path is invalid")
    if not _is_sha(digest):
        raise SelectionError("selection candidate hash is missing")
    expected = {proposal["source_sha256"], proposal["descriptor_sha256"]}
    if digest not in expected:
        raise SelectionError("selection candidate hash does not bind candidate")
    return path, digest


def _optional_candidate_object_sha(value: Mapping[str, Any]) -> str | None:
    """Read an optional compiler-object identity for duplicate auditing."""

    nested = value.get("candidate")
    candidate = nested if isinstance(nested, Mapping) else {}
    present: list[Any] = []
    for key in (
        "candidate_object_sha256", "compiled_candidate_sha256", "object_sha256",
    ):
        if key in value:
            present.append(value[key])
        if key in candidate:
            present.append(candidate[key])
    if not present:
        return None
    if any(not _is_sha(item) for item in present) or len(set(present)) != 1:
        raise SelectionError("selection candidate object hash is invalid")
    return str(present[0])


def _candidate_identity_digest(candidate_sha256: str, object_sha256: str | None) -> str:
    """Bind source and, when present, the compiled object for audit trails."""

    return _canonical_digest({
        "candidate_source_sha256": candidate_sha256,
        "candidate_object_sha256": object_sha256,
    })


def _selection_class(value: Mapping[str, Any]) -> tuple[str, str]:
    diagnosis = value.get("diagnosis")
    if not isinstance(diagnosis, Mapping):
        diagnosis = value.get("owner_flow") if isinstance(value.get("owner_flow"), Mapping) else {}
    status = value.get("status")
    if not isinstance(status, str):
        status = diagnosis.get("status")
    kind = value.get("selection_kind", value.get("evidence_kind"))
    if not isinstance(kind, str):
        kind = ""
    source_class = value.get("source_class")
    if not isinstance(source_class, str):
        ranked = diagnosis.get("ranked_source_class")
        if isinstance(ranked, Mapping):
            source_class = ranked.get("source_class")
    if not isinstance(source_class, str) or not source_class.strip():
        raise SelectionError("selection source_class is missing")
    source_class = source_class.strip()
    if _AMBIGUOUS_CLASS_RE.search(source_class) or _FORBIDDEN_CLASS_RE.search(source_class):
        raise SelectionError("selection source_class is ambiguous or prohibited")
    if status == "RANKED_SOURCE_CLASS":
        return status, source_class
    owner_flow = (
        status in {"OWNER_FLOW", "OWNER_FLOW_MATCHED", "matched"}
        or kind.lower() in {"owner_flow", "owner-flow", "ownerflow"}
        or value.get("owner_flow") is not None
    )
    if owner_flow:
        if status not in {"OWNER_FLOW", "OWNER_FLOW_MATCHED", "matched", None}:
            raise SelectionError("owner-flow status is invalid")
        return "OWNER_FLOW", source_class
    raise SelectionError("selection evidence is not ranked source-class or owner-flow")


def _rank(value: Mapping[str, Any]) -> int:
    raw = value.get("rank")
    if raw is None and isinstance(value.get("ranked_source_class"), Mapping):
        raw = value["ranked_source_class"].get("rank")
    if type(raw) is not int or raw != 1:
        raise SelectionError("selection rank is not exactly 1")
    return raw


def _predicted_counts(value: Mapping[str, Any], current: Mapping[str, int]) -> dict[str, int]:
    raw = value.get("predicted_remaining_counts", value.get("predicted_remaining"))
    if not isinstance(raw, Mapping) or set(raw) != {"strict", "data", "physical"}:
        raise SelectionError("predicted remaining counts are incomplete")
    result: dict[str, int] = {}
    for key in ("strict", "data", "physical"):
        item = raw[key]
        if type(item) is not int or item < 0 or item > current[key]:
            raise SelectionError(f"predicted remaining {key} count is invalid")
        result[key] = item
    return result


def _protected_digest(value: Mapping[str, Any]) -> str:
    digest = value.get("protected_sibling_digest", value.get("sibling_digest"))
    if not _is_sha(digest):
        raise SelectionError("protected sibling digest is missing")
    return digest


def _ownership_complete(value: Mapping[str, Any]) -> bool:
    if value.get("ownership_complete") is True:
        return True
    ownership = value.get("ownership")
    if isinstance(ownership, Mapping) and ownership.get("complete") is True:
        return True
    diagnosis = value.get("diagnosis")
    if isinstance(diagnosis, Mapping):
        permutation = diagnosis.get("maximal_closed_permutation")
        if isinstance(permutation, Mapping) and permutation.get("complete") is True:
            return True
        facts = diagnosis.get("facts")
        if isinstance(facts, Mapping) and facts.get("all_strict_rows_accounted") is True:
            return True
    owner_flow = value.get("owner_flow")
    if isinstance(owner_flow, Mapping):
        facts = owner_flow.get("facts")
        if isinstance(facts, Mapping) and facts.get("all_strict_rows_accounted") is True:
            return True
    return False


def _reconstruction_row_ids(value: Mapping[str, Any]) -> set[str]:
    """Return the row identities sealed by one cluster or bounded region."""

    result: set[str] = set()
    for key in (
        "row_ids", "strict_row_ids", "data_row_ids",
        "physical_difference_ids", "residual_row_ids",
    ):
        rows = value.get(key)
        if isinstance(rows, Mapping):
            for nested in rows.values():
                if isinstance(nested, list):
                    result.update(row for row in nested if isinstance(row, str))
        elif isinstance(rows, list):
            result.update(row for row in rows if isinstance(row, str))
    return result


def _bounded_decomposition_ownership(
    root: Path,
    value: Mapping[str, Any],
    *,
    function: str,
    unit: Any,
    source_sha256: str,
    toolchain_sha256: str,
    frontier_sha256: str,
    predicted_rows: Sequence[str],
) -> bool:
    """Validate one improved-only, region-scoped UNKNOWN reconstruction."""

    if value.get("ownership_complete") is not False:
        return False
    if value.get("ownership_scope") != "bounded_decomposition_region":
        return False
    if value.get("expected_terminal") != "improved":
        return False
    reconstruction = value.get("reconstruction")
    if not isinstance(reconstruction, Mapping):
        return False
    if (
        reconstruction.get("status") != "UNKNOWN"
        or reconstruction.get("next_action") != "DECOMPOSE"
    ):
        return False
    region = reconstruction.get("bounded_region")
    cluster_id = reconstruction.get("causal_cluster_id")
    if not isinstance(region, Mapping) or not isinstance(cluster_id, str) or not cluster_id:
        return False
    # Production broad packets predate an explicit ``closed`` marker.  An
    # explicit false remains fatal; absence is accepted only when the region
    # is exactly cross-linked to one sealed causal cluster below.
    if region.get("closed") is False or region.get("complete") is False:
        return False
    region_cluster_id = region.get("cluster_id")
    region_cluster_ids = region.get("cluster_ids")
    if (
        isinstance(region_cluster_id, str)
        and region_cluster_id != cluster_id
    ):
        return False
    if (
        isinstance(region_cluster_ids, list)
        and cluster_id not in region_cluster_ids
    ):
        return False

    try:
        packet_path, packet_file_sha = _file_ref(
            reconstruction, "selection reconstruction", root
        )
        packet = _read_json(packet_path, "selection reconstruction")
        owner_campaign_reconstruction.verify_packet(packet)
    except (
        SelectionError,
        owner_campaign_reconstruction.ReconstructionPacketError,
    ):
        return False
    packet_sha = reconstruction.get("packet_sha256")
    parent_frontier = packet.get("parent_frontier_sha256")
    if (
        packet_file_sha != reconstruction.get("sha256")
        or packet.get("packet_sha256") != packet_sha
        or packet.get("status") != "UNKNOWN"
        or packet.get("function") != function
        or packet.get("unit") != unit
        or packet.get("source_sha256") != source_sha256
        or packet.get("toolchain_sha256") != toolchain_sha256
        or (
            parent_frontier is not None
            and parent_frontier != frontier_sha256
        )
    ):
        return False
    signal = packet.get("target_first_signal")
    if (
        not isinstance(signal, Mapping)
        or signal.get("status") != "UNKNOWN"
        or signal.get("next_action") != "DECOMPOSE"
        or signal.get("exact_terminal_possible") is not False
    ):
        return False

    packet_regions = packet.get("decomposition_regions")
    if not isinstance(packet_regions, list) or not any(
        isinstance(item, Mapping) and dict(item) == dict(region)
        for item in packet_regions
    ):
        return False
    packet_clusters = packet.get("causal_clusters")
    if not isinstance(packet_clusters, list):
        return False
    selected = [
        item for item in packet_clusters
        if isinstance(item, Mapping) and item.get("cluster_id") == cluster_id
    ]
    if len(selected) != 1:
        return False

    permitted_clusters: list[Mapping[str, Any]] = list(selected)
    region_ids = region.get("cluster_ids")
    if isinstance(region_ids, list):
        wanted = {item for item in region_ids if isinstance(item, str)}
        permitted_clusters = [
            item for item in packet_clusters
            if isinstance(item, Mapping) and item.get("cluster_id") in wanted
        ]
        if {item.get("cluster_id") for item in permitted_clusters} != wanted:
            return False
    else:
        region_group = region.get("mirror_group", region.get("mirror_group_id"))
        if isinstance(region_group, Mapping):
            region_group = region_group.get("id", region_group.get("group_id"))
        if isinstance(region_group, str) and region_group:
            permitted_clusters = [
                item for item in packet_clusters
                if isinstance(item, Mapping)
                and item.get("mirror_group", item.get("mirror_group_id")) == region_group
            ]
            if not permitted_clusters:
                return False
    permitted_rows: set[str] = set()
    for cluster in permitted_clusters:
        permitted_rows.update(_reconstruction_row_ids(cluster))
    if not permitted_rows:
        permitted_rows.update(_reconstruction_row_ids(region))
    return bool(permitted_rows) and set(predicted_rows) <= permitted_rows


def _validate_proposal(
    root: Path,
    campaign: Mapping[str, Any],
    descriptor_path: Path,
    current_frontier: Mapping[str, Any] | None,
) -> dict[str, Any]:
    proposal = _descriptor(root, descriptor_path, campaign)
    sidecar = _find_selection_evidence(root, proposal["path"])
    value = _read_json(sidecar, "selection evidence")
    if value.get("schema") != SCHEMA:
        raise SelectionError("selection evidence schema is invalid")
    _verify_self_digest(value, "selection evidence")
    function = proposal["descriptor"]["function"]
    if (
        value.get("campaign_id") != campaign.get("campaign_id")
        or value.get("function") != function
        or value.get("owner") not in {None, campaign.get("owner")}
    ):
        raise SelectionError("selection evidence identity is invalid")
    status, source_class = _selection_class(value)
    rank = _rank(value)
    candidate_path, candidate_sha = _candidate_reference(value, proposal)
    if candidate_path is not None:
        normalized = Path(candidate_path)
        candidate_rel = normalized.as_posix().lstrip("./")
        if candidate_rel not in {proposal["path_relative"], proposal["source_relative"]}:
            raise SelectionError("selection candidate path does not bind descriptor")

    frontier_ref = _frontier_ref(value)
    if current_frontier is None:
        # Only schema-less unit callers may use the sealed frontier in the
        # sidecar itself.  A loaded campaign must have a persisted frontier.
        if "base_commit" in campaign:
            raise SelectionError("current frontier is unavailable")
        current = dict(frontier_ref)
    else:
        current = dict(current_frontier)
        current_hash = current.get("frontier_sha256")
        if not _is_sha(current_hash):
            raise SelectionError("current frontier hash is invalid")
        if frontier_ref["sha256"] != current_hash:
            raise SelectionError("selection frontier is stale")
    if frontier_ref["function"] != function or current.get("function", function) != function:
        raise SelectionError("selection frontier function mismatch")

    expected_source = current.get("source_sha256", frontier_ref["source_sha256"])
    expected_toolchain = current.get("toolchain_sha256", frontier_ref["toolchain_sha256"])
    if not _is_sha(expected_source) or not _is_sha(expected_toolchain):
        raise SelectionError("current frontier identity is incomplete")
    if frontier_ref["source_sha256"] != expected_source or frontier_ref["toolchain_sha256"] != expected_toolchain:
        raise SelectionError("selection frontier source/toolchain drift")
    if proposal["base_source_sha256"] != expected_source:
        raise SelectionError("candidate base source does not match the current frontier")
    try:
        base_actual = owner_campaign._digest_file(proposal["base_source_path"])
    except OSError as exc:
        raise SelectionError("candidate base source cannot be hashed") from exc
    if base_actual != proposal["base_source_sha256"]:
        raise SelectionError("candidate base source hash drift")
    evidence_source = value.get("source_sha256")
    evidence_toolchain = value.get("toolchain_sha256")
    if evidence_source != expected_source or evidence_toolchain != expected_toolchain:
        raise SelectionError("selection source/toolchain binding drift")
    expected_unit = campaign.get("unit", current.get("unit", frontier_ref["unit"]))
    if value.get("unit") != expected_unit or frontier_ref["unit"] != expected_unit:
        raise SelectionError("selection unit binding drift")

    source_relpath = campaign.get("source_relpath")
    if source_relpath:
        live_source = _bound_path(root, source_relpath, "campaign source")
        if owner_campaign._digest_file(live_source) != expected_source:
            raise SelectionError("current source hash drift")

    focus_ref = value.get("focus_artifact", value.get("focus_report"))
    physical_ref = value.get("physical_artifact", value.get("physical_summary"))
    focus_path, focus_file_sha, focus = _load_artifact(root, focus_ref, "focus artifact")
    physical_path, physical_file_sha, physical = _load_artifact(root, physical_ref, "physical artifact")
    internal_focus_sha = focus.get("focus_evidence_sha256", focus.get("artifact_sha256"))
    current_focus_sha = current.get("focus_evidence_sha256")
    if current_focus_sha and internal_focus_sha and internal_focus_sha != current_focus_sha:
        raise SelectionError("focus artifact is not the current frontier focus")
    if current_focus_sha and not internal_focus_sha and focus_file_sha != current_focus_sha:
        raise SelectionError("focus artifact does not bind current frontier focus")
    for artifact, label in ((focus, "focus"), (physical, "physical")):
        for key, expected in (("function", function), ("unit", expected_unit), ("source_sha256", expected_source)):
            if key in artifact and artifact[key] != expected:
                raise SelectionError(f"{label} artifact {key} binding drift")

    strict_rows, data_rows, physical_rows = _artifact_row_groups(focus, physical)
    residual = _ordered_union(strict_rows, data_rows, physical_rows)
    claimed_residual = _rows(value.get("residual_rows"), "selection residual rows")
    if claimed_residual != residual:
        raise SelectionError("selection residual rows do not match current focus/physical artifacts")
    predicted = _rows(value.get("predicted_rows"), "selection predicted rows")
    if not predicted or not set(predicted) <= set(residual):
        raise SelectionError("selection predicted rows are outside current residual")
    first_mismatch_rows = _first_mismatch_rows(strict_rows, data_rows)
    if first_mismatch_rows and not set(first_mismatch_rows) <= set(predicted):
        raise SelectionError(
            "selection does not cover the first mismatch"
        )
    current_counts = {
        "strict": len(strict_rows),
        "data": len(data_rows),
        "physical": len(physical_rows),
    }
    counts = _predicted_counts(value, current_counts)
    if all(
        row.startswith(("strict:", "data:", "physical:"))
        for row in residual
    ):
        predicted_by_channel = {
            "strict": sum(row.startswith("strict:") for row in predicted),
            "data": sum(row.startswith("data:") for row in predicted),
            "physical": sum(row.startswith("physical:") for row in predicted),
        }
        expected_counts = {
            channel: current_counts[channel] - predicted_by_channel[channel]
            for channel in current_counts
        }
        if counts != expected_counts:
            raise SelectionError("predicted remaining counts do not match predicted rows")
    if value.get("expected_terminal") == "exact" and any(counts.values()):
        raise SelectionError("exact selection does not predict zero remaining rows")
    if not _ownership_complete(value) and not _bounded_decomposition_ownership(
        root,
        value,
        function=function,
        unit=expected_unit,
        source_sha256=expected_source,
        toolchain_sha256=expected_toolchain,
        frontier_sha256=frontier_ref["sha256"],
        predicted_rows=predicted,
    ):
        raise SelectionError("selection ownership is incomplete")
    protected = _protected_digest(value)
    focus_protected = focus.get("sibling_digest", focus.get("protected_sibling_digest"))
    if focus_protected is not None and protected != focus_protected:
        raise SelectionError("protected sibling digest drift")
    if value.get("candidate_count") not in (None, 1):
        raise SelectionError("selection evidence does not contain exactly one candidate")
    candidates = value.get("candidates")
    if candidates is not None and (not isinstance(candidates, list) or len(candidates) != 1):
        raise SelectionError("selection evidence does not contain exactly one candidate")
    candidate_object_sha = _optional_candidate_object_sha(value)
    predicted_row_group_sha = _row_group_digest(predicted)
    selection_key_sha = _selection_key(
        frontier_ref["sha256"], source_class, predicted,
    )
    candidate_identity_sha = _candidate_identity_digest(candidate_sha, candidate_object_sha)
    prior_no_gain = _prior_no_gain(
        root, campaign, function, frontier_ref["sha256"], source_class, predicted,
    )
    if prior_no_gain is not None:
        prior_identity = (
            prior_no_gain.get("result_sha256")
            or prior_no_gain.get("outcome_sha256")
            or prior_no_gain.get("selection_key_sha256")
            or "recorded"
        )
        raise SelectionError(
            "selection suppressed by prior no_gain for the same frontier/source class/row group "
            f"({prior_identity})"
        )
    budget_reason = _budget_reason(
        root, campaign, function, frontier_ref["sha256"], source_class
    )
    if budget_reason is not None:
        raise SelectionError(budget_reason)
    return {
        "descriptor_path": proposal["path"],
        "descriptor_relative": proposal["path_relative"],
        "source_path": proposal["source_path"],
        "source_relative": proposal["source_relative"],
        "candidate_sha256": candidate_sha,
        "base_source_path": proposal["base_source_path"],
        "base_source_relative": proposal["base_source_relative"],
        "base_source_sha256": proposal["base_source_sha256"],
        "rebase_depth": proposal["rebase_depth"],
        "evidence_path": sidecar,
        "evidence_relative": sidecar.relative_to(root).as_posix(),
        "evidence_sha256": value["evidence_sha256"],
        "frontier_sha256": frontier_ref["sha256"],
        "function": function,
        "unit": expected_unit,
        "source_class": source_class,
        "source_class_normalized": _normalize_source_class(source_class),
        "status": status,
        "rank": rank,
        "expected_terminal": value.get("expected_terminal"),
        "residual_rows": residual,
        "predicted_rows": predicted,
        "first_mismatch_rows": first_mismatch_rows,
        "predicted_remaining_counts": counts,
        "protected_sibling_digest": protected,
        "focus_artifact_sha256": focus_file_sha,
        "physical_artifact_sha256": physical_file_sha,
        "predicted_row_group_sha256": predicted_row_group_sha,
        "selection_key_sha256": selection_key_sha,
        "candidate_identity_sha256": candidate_identity_sha,
        **(
            {"candidate_object_sha256": candidate_object_sha}
            if candidate_object_sha is not None else {}
        ),
    }


def _validate_proposal_for_selection(
    index: int,
    root: Path,
    campaign: Mapping[str, Any],
    descriptor_path: Path,
    current_frontiers: Mapping[str, Mapping[str, Any] | None],
) -> tuple[int, dict[str, Any] | None, dict[str, str] | None]:
    """Validate one proposal in a read-only worker.

    The selector historically treated malformed/stale proposals as individual
    rejections, so those expected validation errors are converted to the same
    compact rejection record here.  Unexpected exceptions deliberately escape
    the worker and are re-raised by ``future.result()``; silently converting a
    worker/runtime failure into a rejected proposal would make a partial batch
    look authoritative.
    """

    try:
        raw = _read_json(
            _bound_path(root, str(descriptor_path), "candidate descriptor"),
            "candidate descriptor",
        )
        function = raw.get("function")
        current = current_frontiers.get(function) if isinstance(function, str) else None
        return index, _validate_proposal(root, campaign, descriptor_path, current), None
    except (SelectionError, OSError, owner_campaign.CampaignError) as exc:
        try:
            relative = Path(descriptor_path).relative_to(root).as_posix()
        except ValueError:
            relative = str(descriptor_path)
        return index, None, {"descriptor": relative, "reason": str(exc)[:256]}


def _result(
    *,
    status: str,
    reason: str,
    discovered: int,
    evaluations: Sequence[Mapping[str, Any]],
    selected: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    def json_ready(value: Any) -> Any:
        if isinstance(value, Path):
            return value.as_posix()
        if isinstance(value, Mapping):
            return {str(key): json_ready(item) for key, item in value.items()}
        if isinstance(value, list):
            return [json_ready(item) for item in value]
        return value

    body: dict[str, Any] = {
        "schema": "owner_campaign_selection_result/v1",
        "status": status,
        "selection_status": selected["status"] if selected is not None else UNKNOWN,
        "reason": reason,
        "discovered": discovered,
        "eligible": [json_ready(dict(item)) for item in evaluations],
        "dispatched": 1 if selected is not None else 0,
        "authority_advanced": False,
    }
    if selected is not None:
        body["selected"] = json_ready(dict(selected))
    body["selection_sha256"] = _canonical_digest(body)
    return body


def select_winning_candidate(
    root: Path,
    campaign: Mapping[str, Any],
    descriptor_paths: Sequence[Path],
) -> dict[str, Any]:
    """Select at most one current-bound rank-1 proposal without compiling.

    Invalid proposals are suppressed individually so one malformed Luna
    proposal cannot block a valid winner.  Multiple valid rank-1 proposals are
    ordered deterministically from their sealed evidence: exact predictions
    first, then the fewest predicted remaining rows, then the smallest current
    residual.  This lets parallel workers queue proposals without turning a
    rank-1 tie into an administrative stop.
    """

    root = Path(os.path.abspath(root))
    paths = list(descriptor_paths)
    # Five is the normal single-function Sol batch.  A lane may pass a wider
    # read-only arbitration pool when it must pivot across functions; the
    # selector still returns one winner and never dispatches the pool itself.
    # Retain the old malformed same-function guard for callers that accidentally
    # hand us an unbounded worker batch.
    if len(paths) > MAX_PROPOSALS:
        candidate_functions: set[str] = set()
        for path in paths:
            try:
                raw = _read_json(
                    _bound_path(root, str(path), "candidate descriptor"),
                    "candidate descriptor",
                )
            except SelectionError:
                continue
            function = raw.get("function")
            if isinstance(function, str):
                candidate_functions.add(function)
        if len(candidate_functions) <= 1:
            return _result(
                status=UNKNOWN,
                reason=f"proposal batch exceeds {MAX_PROPOSALS}",
                discovered=len(paths),
                evaluations=[],
            )
    if not paths:
        return _result(status=UNKNOWN, reason="proposal batch is empty", discovered=0, evaluations=[])

    # Every descriptor in one batch must compare against the same current
    # frontier.  A missing frontier is UNKNOWN for production manifests and
    # cannot trigger a compiler run.
    functions: list[str] = []
    for path in paths:
        try:
            raw = _read_json(_bound_path(root, str(path), "candidate descriptor"), "candidate descriptor")
            function = raw.get("function")
            if isinstance(function, str):
                functions.append(function)
        except SelectionError:
            continue
    current_frontiers: dict[str, Mapping[str, Any] | None] = {}
    try:
        for function in sorted(set(functions)):
            current_frontiers[function] = _frontier_file(root, campaign, function)
    except SelectionError as exc:
        return _result(status=UNKNOWN, reason=str(exc), discovered=len(paths), evaluations=[])

    # Validation is read-only, so the five-proposal Sol batch can overlap file
    # reads/hashing.  Futures are consumed in submission order rather than
    # completion order: eligible/rejected lists, diagnostics, and downstream
    # arbitration remain byte-for-byte deterministic for a given input order.
    # The worker only catches the same expected per-proposal errors as the old
    # loop; an unexpected worker exception propagates through ``result()`` and
    # fails closed instead of being misreported as a malformed proposal.
    validation_results: list[
        tuple[int, dict[str, Any] | None, dict[str, str] | None]
    ] = []
    worker_count = min(MAX_VALIDATION_WORKERS, len(paths))
    with ThreadPoolExecutor(
        max_workers=worker_count,
        thread_name_prefix="owner-campaign-select",
    ) as executor:
        futures = [
            executor.submit(
                _validate_proposal_for_selection,
                index,
                root,
                campaign,
                path,
                current_frontiers,
            )
            for index, path in enumerate(paths)
        ]
        for future in futures:
            validation_results.append(future.result())

    # Preserve the explicit input order even if a future implementation or
    # executor changes completion order.  The index is part of the worker
    # contract so an accidental result mix-up fails closed rather than silently
    # reordering proposal diagnostics.
    if [item[0] for item in validation_results] != list(range(len(paths))):
        raise SelectionError("proposal validation results are out of order")
    eligible = [
        item[1] for item in validation_results
        if item[1] is not None
    ]
    rejected = [
        item[2] for item in validation_results
        if item[2] is not None
    ]

    ranked = [item for item in eligible if item["rank"] == 1]
    if not ranked:
        reason = "no current-bound rank-1 proposal"
        if rejected:
            reason += ": " + "; ".join(item["reason"] for item in rejected[:2])
        budget_rejections = [
            item for item in rejected
            if "no_gain budget exhausted" in item.get("reason", "")
        ]
        if budget_rejections and len(budget_rejections) == len(rejected) and not eligible:
            reason = "all current-bound proposals exhausted; pivot required: " + "; ".join(
                item["reason"] for item in budget_rejections[:2]
            )
            return _result(
                status=PIVOT_REQUIRED,
                reason=reason,
                discovered=len(paths),
                evaluations=[*eligible, *rejected],
            )
        return _result(
            status=UNKNOWN,
            reason=reason,
            discovered=len(paths),
            evaluations=[*eligible, *rejected],
        )
    function_order = {
        name: index
        for index, name in enumerate(campaign.get("functions", []))
        if isinstance(name, str)
    }
    ranked.sort(
        key=lambda item: (
            0 if item.get("expected_terminal") == "exact" else 1,
            sum(item["predicted_remaining_counts"].values()),
            len(item["residual_rows"]),
            function_order.get(item["function"], len(function_order)),
            item["selection_key_sha256"],
            item["candidate_identity_sha256"],
        )
    )
    reason = (
        "one deterministic rank-1 proposal selected"
        if len(ranked) == 1
        else "parallel rank-1 proposals deterministically arbitrated"
    )
    return _result(
        status=SELECTED,
        reason=reason,
        discovered=len(paths),
        evaluations=[*eligible, *rejected],
        selected=ranked[0],
    )


# Friendly aliases for callers and future lane adapters.
select_winner = select_winning_candidate
select_batch = select_winning_candidate


__all__ = [
    "MAX_NO_GAIN_PER_FAMILY",
    "MAX_NO_GAIN_PER_FUNCTION",
    "MAX_PROPOSALS",
    "MAX_VALIDATION_WORKERS",
    "PIVOT_REQUIRED",
    "SCHEMA",
    "SELECTION_SCHEMA",
    "SELECTED",
    "UNKNOWN",
    "SelectionError",
    "select_batch",
    "select_winner",
    "select_winning_candidate",
    "selection_evidence_path",
    "selection_evidence_paths",
    "selection_ledger_path",
    "selection_outcome_ledger_path",
    "append_selection_outcome",
    "normalize_source_class",
]
