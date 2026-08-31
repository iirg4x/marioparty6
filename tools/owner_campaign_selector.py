"""Fail-closed, precompile selection for autonomous owner campaigns.

Five Luna workers may propose source cells for one owner, but the compiler must
see at most one proposal at a time.  A proposal is accompanied by a compact
``owner_campaign_selection/v1`` sidecar next to its candidate descriptor.  The
sidecar is evidence, not authority: it binds the current frontier, source,
unit, toolchain, focus/physical artifacts, residual row identities, predicted
remaining counts, and protected-sibling census.  This module never writes,
compiles, consumes, or deletes a candidate.

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
from pathlib import Path
import re
from typing import Any, Mapping, Sequence

from . import owner_campaign


SCHEMA = "owner_campaign_selection/v1"
SELECTION_SCHEMA = SCHEMA
UNKNOWN = "UNKNOWN"
SELECTED = "selected"
MAX_PROPOSALS = 5
OUTCOME_SCHEMA = "owner_campaign_selection_outcome/v1"
SELECTION_OUTCOME_SCHEMA = OUTCOME_SCHEMA
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

    The selector never creates this file.  Owner-campaign/import tooling may
    seed it with sealed outcomes so a fresh batch can suppress already measured
    ``no_gain`` classes without replaying a large historical transcript.
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


def _ledger_path(root: Path, path: Path) -> Path:
    """Bind an internally discovered ledger path without following links."""

    try:
        relative = path.relative_to(root)
    except ValueError as exc:
        raise SelectionError("selection outcome ledger escapes campaign root") from exc
    return _bound_path(root, relative.as_posix(), "selection outcome ledger")


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
        record_class = record.get("source_class")
        record_rows = _record_rows(record)
        record_group = _record_hash(
            record, "predicted_row_group_sha256", "row_group_sha256",
            "predicted_rows_sha256", "predicted_row_group",
        )
        record_key = _record_hash(record, "selection_key_sha256", "suppression_key_sha256")
        if record_key is not None and record_key != key:
            continue
        if record_frontier is not None and record_frontier != frontier_sha256:
            continue
        if not isinstance(record_class, str) or record_class != source_class:
            nested = record.get("selection")
            if isinstance(nested, Mapping):
                record_class = nested.get("source_class")
            if record_class != source_class:
                continue
        if record_rows is not None:
            if len(record_rows) != len(set(record_rows)) or _row_group_digest(record_rows) != row_group:
                continue
        elif record_group != row_group:
            continue
        # A key-only record is accepted only when its key is valid and the
        # frontier/class fields above also match (or are carried by ``selection``).
        if record_key is not None or (
            record_frontier == frontier_sha256 and record_class == source_class
        ):
            return record
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
    fields = getattr(owner_campaign, "CANDIDATE_FIELDS", frozenset())
    if set(value) != set(fields):
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
    return {
        "path": path,
        "path_relative": path.relative_to(root).as_posix(),
        "descriptor": dict(value),
        "descriptor_sha256": digest,
        "source_path": source_path,
        "source_relative": source_path.relative_to(root).as_posix(),
        "source_sha256": source_sha,
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
    # manifests always have a persisted frontier and never take this branch.
    supplied = campaign.get("_selection_frontier")
    if isinstance(supplied, Mapping):
        digest = supplied.get("frontier_sha256")
        body = dict(supplied)
        body.pop("frontier_sha256", None)
        if not _is_sha(digest) or _canonical_digest(body) != digest:
            raise SelectionError("in-memory current frontier digest is invalid")
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
    if not _ownership_complete(value):
        raise SelectionError("selection ownership is incomplete")
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
    return {
        "descriptor_path": proposal["path"],
        "descriptor_relative": proposal["path_relative"],
        "source_path": proposal["source_path"],
        "source_relative": proposal["source_relative"],
        "candidate_sha256": candidate_sha,
        "evidence_path": sidecar,
        "evidence_relative": sidecar.relative_to(root).as_posix(),
        "evidence_sha256": value["evidence_sha256"],
        "frontier_sha256": frontier_ref["sha256"],
        "function": function,
        "unit": expected_unit,
        "source_class": source_class,
        "status": status,
        "rank": rank,
        "residual_rows": residual,
        "predicted_rows": predicted,
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
    a deterministic tie and return ``UNKNOWN``; no source or descriptor is
    touched in either outcome.
    """

    root = Path(os.path.abspath(root))
    paths = list(descriptor_paths)
    if len(paths) > MAX_PROPOSALS:
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

    eligible: list[dict[str, Any]] = []
    rejected: list[dict[str, str]] = []
    for path in paths:
        try:
            raw = _read_json(_bound_path(root, str(path), "candidate descriptor"), "candidate descriptor")
            function = raw.get("function")
            current = current_frontiers.get(function) if isinstance(function, str) else None
            eligible.append(_validate_proposal(root, campaign, path, current))
        except (SelectionError, OSError, owner_campaign.CampaignError) as exc:
            try:
                relative = Path(path).relative_to(root).as_posix()
            except ValueError:
                relative = str(path)
            rejected.append({"descriptor": relative, "reason": str(exc)[:256]})

    ranked = [item for item in eligible if item["rank"] == 1]
    if len(ranked) > 1:
        return _result(
            status=UNKNOWN,
            reason="rank-1 selection tie",
            discovered=len(paths),
            evaluations=[*eligible, *rejected],
        )
    if not ranked:
        reason = "no current-bound rank-1 proposal"
        if rejected:
            reason += ": " + "; ".join(item["reason"] for item in rejected[:2])
        return _result(
            status=UNKNOWN,
            reason=reason,
            discovered=len(paths),
            evaluations=[*eligible, *rejected],
        )
    return _result(
        status=SELECTED,
        reason="one deterministic rank-1 proposal selected",
        discovered=len(paths),
        evaluations=[*eligible, *rejected],
        selected=ranked[0],
    )


# Friendly aliases for callers and future lane adapters.
select_winner = select_winning_candidate
select_batch = select_winning_candidate


__all__ = [
    "MAX_PROPOSALS",
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
]
