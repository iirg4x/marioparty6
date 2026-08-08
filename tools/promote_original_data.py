#!/usr/bin/env python3
"""Promote an exact, authenticated original-data C owner.

This is deliberately separate from ``promote_recovered_c.py``.  Ordinary
recovered C still rejects raw hexadecimal literals.  This tool accepts one
manifest-bound data initializer, proves its exact shape and hashes, and copies
only that verified Git blob to a fresh branch based directly on main.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path, PurePosixPath
from typing import Any, Mapping, Sequence

sys.path.insert(0, str(Path(__file__).resolve().parent))

from promote_recovered_c import (  # noqa: E402
    PromotionError,
    _blob,
    _branch_exists,
    _changed_paths,
    _git_bytes,
    _normalise_path,
    _queue_verification,
    _run,
    _worktree_list,
    _write_manifest,
    branch_errors,
    git_root,
    message_errors,
    resolve_ref,
    source_ai_markers,
)


DEFAULT_MANIFEST = "config/recovery/original_data.json"
HEX_SHA256 = re.compile(r"[0-9a-f]{64}\Z")
HEX_GIT = re.compile(r"[0-9a-f]{40}\Z")
IDENTIFIER = re.compile(r"[A-Za-z_]\w*\Z")
ARRAY_RE = re.compile(
    r"\bchar\s+(?P<symbol>[A-Za-z_]\w*)\s*\[\s*\]\s*"
    r"ATTRIBUTE_ALIGN\(\s*(?P<alignment>[0-9]+)\s*\)\s*=\s*"
    r"\{(?P<body>.*?)\}\s*;",
    re.DOTALL,
)
LENGTH_RE = re.compile(
    r"\bu16\s+(?P<symbol>[A-Za-z_]\w*)\s*=\s*"
    r"sizeof\(\s*(?P<payload>[A-Za-z_]\w*)\s*\)\s*;"
)
BYTE_TOKEN = re.compile(r"0[xX][0-9A-Fa-f]{1,2}\Z")
INCLUDE_LINE = re.compile(r'#include\s+"[^"\r\n]+"\Z')
IF_LINE = re.compile(r"#if\s+[A-Za-z0-9_ !=<>&|()+\-]+\Z")

TOP_LEVEL_KEYS = {"schema_version", "records"}
RECORD_KEYS = {
    "id",
    "classification",
    "kind",
    "status",
    "owner",
    "path",
    "source_sha256",
    "payload",
    "length_symbol",
    "target",
    "donor",
    "evidence",
}
PAYLOAD_KEYS = {"symbol", "length", "alignment", "sha256"}
LENGTH_KEYS = {"symbol", "value"}
TARGET_KEYS = {
    "section",
    "address",
    "size",
    "length_section",
    "length_address",
    "length_size",
    "relocations",
}
DONOR_KEYS = {"repo", "path", "commit", "blob"}


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _all_changed_paths(root: Path, base: str, head: str) -> list[str]:
    output = _run(
        root,
        "git",
        "diff",
        "--name-only",
        f"{base}...{head}",
    ).stdout
    return sorted({line.strip() for line in output.splitlines() if line.strip()})


def _original_data_branch_errors(branch: str) -> list[str]:
    errors = branch_errors(branch)
    if not branch.startswith("recovery/"):
        errors.append("original-data promotion branch must start with recovery/")
    return errors


def _require_keys(value: Mapping[str, Any], expected: set[str], where: str) -> None:
    actual = set(value)
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        raise PromotionError(f"{where}: keys differ (missing={missing}, extra={extra})")


def _require_mapping(value: Any, where: str) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        raise PromotionError(f"{where}: expected an object")
    return value


def _require_string(value: Any, where: str) -> str:
    if not isinstance(value, str) or not value:
        raise PromotionError(f"{where}: expected a non-empty string")
    return value


def _require_positive_int(value: Any, where: str) -> int:
    if not isinstance(value, int) or isinstance(value, bool) or value <= 0:
        raise PromotionError(f"{where}: expected a positive integer")
    return value


def _require_relative_path(value: Any, where: str) -> str:
    raw = _require_string(value, where).replace("\\", "/")
    path = PurePosixPath(raw)
    if path.is_absolute() or ".." in path.parts or any(marker in raw for marker in "*?["):
        raise PromotionError(f"{where}: expected an exact repository-relative path")
    return path.as_posix()


def validate_record(value: Mapping[str, Any], *, where: str = "record") -> dict[str, Any]:
    """Validate and return one exact original-data manifest record."""

    _require_keys(value, RECORD_KEYS, where)
    if value["classification"] != "authenticated":
        raise PromotionError(f"{where}: classification must be authenticated")
    if value["kind"] != "original_data":
        raise PromotionError(f"{where}: kind must be original_data")
    if value["status"] not in {
        "static_authenticated_pending_native",
        "native_verified",
    }:
        raise PromotionError(f"{where}: invalid status {value['status']!r}")

    record = dict(value)
    for key in ("id", "owner"):
        _require_string(record[key], f"{where}.{key}")

    path = _normalise_path(_require_string(record["path"], f"{where}.path"))
    if any(marker in path for marker in "*?["):
        raise PromotionError(f"{where}.path: wildcards are forbidden")
    record["path"] = path

    source_sha = _require_string(record["source_sha256"], f"{where}.source_sha256")
    if not HEX_SHA256.fullmatch(source_sha):
        raise PromotionError(f"{where}.source_sha256: expected lowercase SHA-256")

    payload = dict(_require_mapping(record["payload"], f"{where}.payload"))
    _require_keys(payload, PAYLOAD_KEYS, f"{where}.payload")
    if not IDENTIFIER.fullmatch(_require_string(payload["symbol"], f"{where}.payload.symbol")):
        raise PromotionError(f"{where}.payload.symbol: invalid C identifier")
    _require_positive_int(payload["length"], f"{where}.payload.length")
    _require_positive_int(payload["alignment"], f"{where}.payload.alignment")
    if not HEX_SHA256.fullmatch(
        _require_string(payload["sha256"], f"{where}.payload.sha256")
    ):
        raise PromotionError(f"{where}.payload.sha256: expected lowercase SHA-256")
    record["payload"] = payload

    length_symbol = dict(
        _require_mapping(record["length_symbol"], f"{where}.length_symbol")
    )
    _require_keys(length_symbol, LENGTH_KEYS, f"{where}.length_symbol")
    if not IDENTIFIER.fullmatch(
        _require_string(length_symbol["symbol"], f"{where}.length_symbol.symbol")
    ):
        raise PromotionError(f"{where}.length_symbol.symbol: invalid C identifier")
    _require_positive_int(length_symbol["value"], f"{where}.length_symbol.value")
    if length_symbol["value"] != payload["length"]:
        raise PromotionError(f"{where}: length symbol value differs from payload length")
    record["length_symbol"] = length_symbol

    target = dict(_require_mapping(record["target"], f"{where}.target"))
    _require_keys(target, TARGET_KEYS, f"{where}.target")
    for key in ("section", "address", "length_section", "length_address"):
        _require_string(target[key], f"{where}.target.{key}")
    for key in ("size", "length_size"):
        _require_positive_int(target[key], f"{where}.target.{key}")
    if target["size"] != payload["length"]:
        raise PromotionError(f"{where}: target size differs from payload length")
    if not isinstance(target["relocations"], list) or target["relocations"]:
        raise PromotionError(f"{where}.target.relocations: expected an empty list")
    for key in ("address", "length_address"):
        try:
            int(target[key], 16)
        except ValueError as error:
            raise PromotionError(f"{where}.target.{key}: invalid hexadecimal address") from error
    record["target"] = target

    donor = dict(_require_mapping(record["donor"], f"{where}.donor"))
    _require_keys(donor, DONOR_KEYS, f"{where}.donor")
    for key in ("repo", "path"):
        donor[key] = _require_relative_path(donor[key], f"{where}.donor.{key}")
    for key in ("commit", "blob"):
        digest = _require_string(donor[key], f"{where}.donor.{key}")
        if not HEX_GIT.fullmatch(digest):
            raise PromotionError(f"{where}.donor.{key}: expected a full lowercase Git hash")
    record["donor"] = donor

    evidence = record["evidence"]
    if not isinstance(evidence, list) or not evidence or not all(
        isinstance(item, str) and item for item in evidence
    ):
        raise PromotionError(f"{where}.evidence: expected non-empty string paths")
    record["evidence"] = [
        _require_relative_path(item, f"{where}.evidence") for item in evidence
    ]
    return record


def load_manifest(root: Path, manifest_path: str = DEFAULT_MANIFEST) -> list[dict[str, Any]]:
    if manifest_path.replace("\\", "/") != DEFAULT_MANIFEST:
        raise PromotionError(
            f"original-data policy must use {DEFAULT_MANIFEST}, not {manifest_path}"
        )
    path = root / manifest_path
    if not path.is_file():
        raise PromotionError(f"original-data manifest not found: {path}")
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise PromotionError(f"invalid original-data manifest {path}: {error}") from error
    top = _require_mapping(payload, str(path))
    _require_keys(top, TOP_LEVEL_KEYS, str(path))
    if (
        not isinstance(top["schema_version"], int)
        or isinstance(top["schema_version"], bool)
        or top["schema_version"] != 1
    ):
        raise PromotionError(f"{path}: unsupported schema_version")
    records = top["records"]
    if not isinstance(records, list) or not records:
        raise PromotionError(f"{path}: records must be a non-empty list")
    validated = [
        validate_record(_require_mapping(item, f"records[{index}]"), where=f"records[{index}]")
        for index, item in enumerate(records)
    ]
    ids = [item["id"] for item in validated]
    paths = [item["path"] for item in validated]
    owners = [item["owner"] for item in validated]
    if len(ids) != len(set(ids)) or len(paths) != len(set(paths)) or len(owners) != len(set(owners)):
        raise PromotionError(f"{path}: record ids, paths, and owners must be unique")
    return validated


def find_record(
    root: Path,
    *,
    owner: str,
    path: str,
    manifest_path: str = DEFAULT_MANIFEST,
) -> dict[str, Any]:
    normalised = _normalise_path(path)
    records = [
        item
        for item in load_manifest(root, manifest_path)
        if item["owner"] == owner and item["path"] == normalised
    ]
    if len(records) != 1:
        raise PromotionError(
            f"expected one original-data record for owner={owner!r} path={normalised!r}; "
            f"found {len(records)}"
        )
    return records[0]


def parse_original_data(text: str, record: Mapping[str, Any]) -> dict[str, Any]:
    """Parse the single allowlisted initializer and reject any executable C."""

    if "\x00" in text:
        raise PromotionError("source contains a NUL byte")
    forbidden = re.search(r"^\s*#\s*(?:define|pragma|include_next|line)\b|\b(?:asm|__asm|volatile|register)\b", text, re.MULTILINE)
    if forbidden:
        raise PromotionError(f"source contains a forbidden construct: {forbidden.group(0).strip()}")

    arrays = list(ARRAY_RE.finditer(text))
    lengths = list(LENGTH_RE.finditer(text))
    if len(arrays) != 1 or len(lengths) != 1:
        raise PromotionError(
            f"source must contain exactly one data array and one length declaration "
            f"(found arrays={len(arrays)}, lengths={len(lengths)})"
        )
    array = arrays[0]
    length = lengths[0]
    if array.end() > length.start():
        raise PromotionError("payload array must precede its length declaration")
    payload = record["payload"]
    length_record = record["length_symbol"]
    if array.group("symbol") != payload["symbol"]:
        raise PromotionError("payload symbol differs from manifest")
    if int(array.group("alignment")) != payload["alignment"]:
        raise PromotionError("payload alignment differs from manifest")
    if length.group("symbol") != length_record["symbol"]:
        raise PromotionError("length symbol differs from manifest")
    if length.group("payload") != payload["symbol"]:
        raise PromotionError("length declaration does not use sizeof(payload symbol)")

    parts = [item.strip() for item in array.group("body").split(",")]
    if parts and parts[-1] == "":
        parts.pop()
    if not parts or any(not BYTE_TOKEN.fullmatch(item) for item in parts):
        raise PromotionError("payload initializer must contain only hexadecimal byte tokens")
    data = bytes(int(item, 16) for item in parts)
    if len(data) != payload["length"]:
        raise PromotionError(
            f"payload length {len(data)} differs from manifest {payload['length']}"
        )
    digest = _sha256(data)
    if digest != payload["sha256"]:
        raise PromotionError(f"payload SHA-256 {digest} differs from manifest")

    remainder = text[: array.start()] + text[array.end() : length.start()] + text[length.end() :]
    for line_number, raw in enumerate(remainder.splitlines(), 1):
        line = raw.strip()
        if not line:
            continue
        if INCLUDE_LINE.fullmatch(line) or IF_LINE.fullmatch(line) or line == "#endif":
            continue
        raise PromotionError(f"line {line_number}: extra declaration or unsupported source text: {line}")
    return {
        "symbol": payload["symbol"],
        "length": len(data),
        "alignment": payload["alignment"],
        "sha256": digest,
        "length_symbol": length_record["symbol"],
    }


def _validate_source(data: bytes, record: Mapping[str, Any]) -> dict[str, Any]:
    digest = _sha256(data)
    if digest != record["source_sha256"]:
        raise PromotionError(f"source SHA-256 {digest} differs from manifest")
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as error:
        raise PromotionError("source is not UTF-8") from error
    markers = source_ai_markers(text)
    if markers:
        raise PromotionError("source contains AI/agent attribution:\n- " + "\n- ".join(markers))
    return parse_original_data(text, record)


def plan_original_data(
    root: Path,
    *,
    base_ref: str,
    source_ref: str,
    owner: str,
    path: str,
    manifest_path: str = DEFAULT_MANIFEST,
    allow_unverified: bool = False,
) -> dict[str, Any]:
    root = root.resolve()
    base = resolve_ref(root, base_ref)
    source = resolve_ref(root, source_ref)
    record = find_record(root, owner=owner, path=path, manifest_path=manifest_path)
    selected = record["path"]
    changed = _changed_paths(root, base, source)
    if selected not in changed:
        raise PromotionError(f"{selected}: not changed between {base} and {source}")
    source_blob = _blob(root, source, selected)
    if source_blob is None:
        raise PromotionError(f"{selected}: missing at source {source}")
    source_data = _git_bytes(root, source, selected)
    parsed = _validate_source(source_data, record)
    queue_proof = None
    if not allow_unverified:
        queue_proof = _queue_verification(root, owner, source)
    return {
        "base_commit": base,
        "source_commit": source,
        "owner": owner,
        "manifest": manifest_path,
        "record": record,
        "queue_proof": queue_proof,
        "files": [
            {
                "path": selected,
                "base_blob": _blob(root, base, selected),
                "source_blob": source_blob,
                "sha256": _sha256(source_data),
                "size": len(source_data),
            }
        ],
        "parsed": parsed,
        "policy": {
            "classification": "authenticated original data; zero clean-C credit",
            "allowed": "one exact manifest-bound src/**/*.c data initializer",
            "ordinary_c": "promote_recovered_c.py remains unchanged and rejects raw hexadecimal literals",
        },
    }


def audit_original_data(
    root: Path,
    *,
    base_ref: str,
    head_ref: str,
    source_ref: str,
    owner: str,
    path: str,
    policy_root: Path | None = None,
    manifest_path: str = DEFAULT_MANIFEST,
) -> dict[str, Any]:
    root = root.resolve()
    policy_root = (policy_root or root).resolve()
    base = resolve_ref(root, base_ref)
    head = resolve_ref(root, head_ref)
    source = resolve_ref(policy_root, source_ref)
    record = find_record(policy_root, owner=owner, path=path, manifest_path=manifest_path)
    selected = record["path"]
    errors: list[str] = []
    changed = _all_changed_paths(root, base, head)
    if changed != [selected]:
        errors.append(f"changed paths {changed} do not equal [{selected!r}]")
    parents = _run(root, "git", "show", "-s", "--format=%P", head).stdout.split()
    if parents != [base]:
        errors.append(f"promotion commit parents {parents} do not equal [{base!r}]")
    head_blob = _blob(root, head, selected)
    source_blob = _blob(policy_root, source, selected)
    if head_blob != source_blob:
        errors.append(f"{selected}: promoted blob {head_blob} differs from source {source_blob}")
    if head_blob is not None:
        try:
            _validate_source(_git_bytes(root, head, selected), record)
        except PromotionError as error:
            errors.append(str(error))
    branch = _run(root, "git", "branch", "--show-current").stdout.strip()
    errors.extend(_original_data_branch_errors(branch))
    message = _run(root, "git", "show", "-s", "--format=%B", head).stdout
    errors.extend(message_errors(message))
    status = _run(root, "git", "status", "--porcelain").stdout.strip()
    if status:
        errors.append("promotion worktree is not clean")
    return {
        "base_commit": base,
        "head_commit": head,
        "source_commit": source,
        "owner": owner,
        "path": selected,
        "errors": sorted(set(errors)),
        "clean_human_promotion": not errors,
    }


def create_original_data(
    root: Path,
    *,
    base_ref: str,
    source_ref: str,
    owner: str,
    path: str,
    branch: str,
    worktree: Path,
    title: str,
    manifest_path: str = DEFAULT_MANIFEST,
    allow_unverified: bool = False,
) -> dict[str, Any]:
    metadata_errors = [*_original_data_branch_errors(branch), *message_errors(title)]
    if metadata_errors:
        raise PromotionError("promotion metadata rejected:\n- " + "\n- ".join(metadata_errors))
    plan = plan_original_data(
        root,
        base_ref=base_ref,
        source_ref=source_ref,
        owner=owner,
        path=path,
        manifest_path=manifest_path,
        allow_unverified=allow_unverified,
    )
    root = root.resolve()
    worktree = worktree.resolve()
    if worktree.exists() or worktree in _worktree_list(root):
        raise PromotionError(f"promotion worktree already exists: {worktree}")
    if _branch_exists(root, branch):
        raise PromotionError(f"promotion branch already exists: {branch}")
    selected = str(plan["files"][0]["path"])
    _run(root, "git", "worktree", "add", "-q", "-b", branch, str(worktree), plan["base_commit"])
    try:
        destination = worktree / selected
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(_git_bytes(root, plan["source_commit"], selected))
        hashed = _run(worktree, "git", "hash-object", "--path", selected, selected).stdout.strip()
        if hashed != plan["files"][0]["source_blob"]:
            raise PromotionError(f"{selected}: working-tree blob differs from verified source")
        _run(worktree, "git", "add", "--", selected)
        staged = [
            line
            for line in _run(worktree, "git", "diff", "--cached", "--name-only").stdout.splitlines()
            if line
        ]
        if staged != [selected]:
            raise PromotionError(f"staged paths {staged} do not equal [{selected!r}]")
        _run(worktree, "git", "commit", "-q", "--no-verify", "-m", title)
        promotion_commit = resolve_ref(worktree, "HEAD")
        audit = audit_original_data(
            worktree,
            base_ref=plan["base_commit"],
            head_ref=promotion_commit,
            source_ref=plan["source_commit"],
            owner=owner,
            path=selected,
            policy_root=root,
            manifest_path=manifest_path,
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
                "run object, relocation, linked-retail, checksum, and progress gates",
                "promote configure/status/progress separately after the owner is proven",
            ],
        }
        result["local_manifest"] = str(_write_manifest(root, branch, result))
        return result
    except Exception:
        _run(root, "git", "worktree", "remove", "--force", str(worktree), allow_failure=True)
        if _branch_exists(root, branch):
            _run(root, "git", "branch", "-D", branch, allow_failure=True)
        raise


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=".")
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("plan", "create", "audit"):
        command = commands.add_parser(name)
        command.add_argument("--base", required=True)
        command.add_argument("--source", required=True)
        command.add_argument("--owner", required=True)
        command.add_argument("--path", required=True)
        command.add_argument("--manifest", default=DEFAULT_MANIFEST)
        if name in {"plan", "create"}:
            command.add_argument("--allow-unverified", action="store_true")
        if name == "create":
            command.add_argument("--branch", required=True)
            command.add_argument("--worktree", required=True)
            command.add_argument("--title", required=True)
        if name == "audit":
            command.add_argument("--head", required=True)
            command.add_argument("--policy-root")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    root = git_root(args.root)
    try:
        if args.command == "plan":
            result = plan_original_data(
                root,
                base_ref=args.base,
                source_ref=args.source,
                owner=args.owner,
                path=args.path,
                manifest_path=args.manifest,
                allow_unverified=args.allow_unverified,
            )
        elif args.command == "create":
            result = create_original_data(
                root,
                base_ref=args.base,
                source_ref=args.source,
                owner=args.owner,
                path=args.path,
                branch=args.branch,
                worktree=Path(args.worktree),
                title=args.title,
                manifest_path=args.manifest,
                allow_unverified=args.allow_unverified,
            )
        else:
            result = audit_original_data(
                root,
                base_ref=args.base,
                head_ref=args.head,
                source_ref=args.source,
                owner=args.owner,
                path=args.path,
                policy_root=Path(args.policy_root) if args.policy_root else None,
                manifest_path=args.manifest,
            )
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0
    except PromotionError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
