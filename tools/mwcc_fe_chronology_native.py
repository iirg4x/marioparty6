#!/usr/bin/env python3
"""Fail-closed GC/2.6 front-end local chronology capture.

This is the producer side of the small front-end chronology contract.  It is
intentionally separate from :mod:`mwcc_fe_chronology`: the older validator is
still a blocked consumer contract, while this module is the authenticated
capture adapter for the repaired GC/2.6 hook plan.

The native transport is inherited from ``capsule_stack_home_native``.  This
module only supplies the front-end hook plan and the pointer-to-generation
join.  Raw compiler pointers are accepted by the private adapter while a
capture is paused, but they never cross the packet boundary.  A packet is
diagnostic evidence only; it does not name source locals or advance matching
authority.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys
from collections.abc import Iterable, Mapping, Sequence
from typing import Any

try:  # Package import for ``python -m tools.mwcc_fe_chronology_native``.
    from . import capsule_stack_home_native as _stack_home
except ImportError:  # Direct ``python tools/mwcc_fe_chronology_native.py``.
    import capsule_stack_home_native as _stack_home


SCHEMA = "mwcc_fe_chronology_native/v1"
EVENT_SCHEMA = "mwcc_fe_chronology_native_event/v1"
TOOL_VERSION = "mwcc-fe-chronology-native-1"
SCHEMA_VERSION = 1
TARGET_FUNCTION = "CapSelectMasuPlayer"
DIAGNOSTIC_ONLY = True
BOARD_ADMISSION = False
EXACTNESS_CLAIM = False

SHA256_RE = re.compile(r"[0-9a-f]{64}\Z")
SESSION_RE = re.compile(r"session-[0-9a-f]{16}\Z")
GENERATION_RE = re.compile(r"(?:object|varinfo)-generation-[0-9]{6}\Z")
HOOK_ID_RE = re.compile(r"[a-z][a-z0-9_]{2,63}\Z")


# These prefixes are read from the authenticated GC/2.6 build107 image.  The
# code addresses are private transport anchors; only hook IDs are serialized.
# Keep this table closed and ordered: installing a different image or silently
# replacing a site with a nearby instruction would make all following links
# unauthenticated.
HOOKS: tuple[dict[str, Any], ...] = (
    {
        "id": "reset",
        "address": 0x004F0A4A,
        "prefix": "c705d4a85e0000000000",
        "role": "frontend_reset",
    },
    {
        "id": "generic_insert_0",
        "address": 0x004E91DC,
        "prefix": "a3d4a85e0089f05e5bc3",
        "role": "generic_completed_insertion",
    },
    {
        "id": "generic_insert_1",
        "address": 0x004F113B,
        "prefix": "a3d4a85e006a1ee8d9fb",
        "role": "generic_completed_insertion",
    },
    {
        "id": "generic_insert_2",
        "address": 0x004F4DC9,
        "prefix": "a3d4a85e008b450050e889",
        "role": "generic_completed_insertion",
    },
    {
        "id": "bulk_object_link",
        "address": 0x0055CBE2,
        "prefix": "895d04c7450000000000c60305c6430100",
        "role": "bulk_object_link",
    },
    {
        "id": "target_boundary",
        "address": 0x00433492,
        "prefix": "8b400e8b5006eb08",
        "role": "target_boundary",
    },
)
HOOK_BY_ID = {str(row["id"]): row for row in HOOKS}
HOOK_BY_ADDRESS = {int(row["address"]): row for row in HOOKS}
HOOK_ORDER = tuple(str(row["id"]) for row in HOOKS)
GENERIC_HOOK_IDS = tuple(row for row in HOOK_ORDER if row.startswith("generic_insert_"))
# The generic insertion sites expose the newly created list node in EAX, but
# retain the compiler Object argument in a different register at each site.
# The hook is sampled after its first instruction, while EAX still names the
# node, so using EAX here silently joins the node instead of the Object.
GENERIC_OBJECT_REGISTERS = {
    "generic_insert_0": "esi",
    "generic_insert_1": "ebp",
    "generic_insert_2": "ebx",
}

PROVENANCE_FIELDS = frozenset(
    {"source_sha256", "compiler_sha256", "trace_sha256", "session_id"}
)
PACKET_FIELDS = frozenset(
    {
        "schema",
        "schema_version",
        "tool_version",
        "status",
        "diagnostic_only",
        "board_admission",
        "exactness_claim",
        "authority_advanced",
        "function",
        "provenance",
        "hook_plan",
        "hook_coverage",
        "events",
        "generations",
        "limitations",
        "packet_sha256",
    }
)
GENERATION_FIELDS = frozenset(
    {
        "generation_id",
        "inserted_by",
        "insert_event_sequence",
        "link_event_sequence",
        "linked",
        "varinfo_generation_id",
        "home_value",
        "evidence",
    }
)
EVENT_FIELDS: dict[str, frozenset[str]] = {
    "reset": frozenset({"schema", "sequence", "event_kind", "hook_id"}),
    "generic_insertion": frozenset(
        {"schema", "sequence", "event_kind", "hook_id", "generation_id"}
    ),
    "bulk_object_link": frozenset(
        {"schema", "sequence", "event_kind", "hook_id", "generation_ids"}
    ),
    "target_boundary": frozenset(
        {"schema", "sequence", "event_kind", "hook_id", "phase"}
    ),
    "post_allocation_snapshot": frozenset(
        {"schema", "sequence", "event_kind", "bindings"}
    ),
}
LIMITATIONS = [
    "Object and VarInfo pointers are private capture identities and are replaced by generation IDs.",
    "Source declaration, inline ownership, and semantic local names remain UNKNOWN.",
    "No source, object, build, queue, authority, or retail state is advanced.",
]


class Rejected(ValueError):
    """Raised for an unauthenticated, ambiguous, or incomplete capture."""


ChronologyError = Rejected


def _strict_keys(value: Any, expected: Iterable[str], label: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise Rejected(f"{label} must be an object")
    actual = set(value)
    wanted = set(expected)
    if actual != wanted:
        missing = sorted(wanted - actual)
        extra = sorted(actual - wanted)
        raise Rejected(f"{label} field allowlist mismatch (missing={missing}, extra={extra})")
    return value


def _text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value or value != value.strip():
        raise Rejected(f"{label} must be non-empty canonical text")
    return value


def _integer(value: Any, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise Rejected(f"{label} must be an integer")
    return value


def _pointer(value: Any, label: str) -> int:
    result = _integer(value, label)
    if result <= 0 or result > 0xFFFFFFFF:
        raise Rejected(f"{label} is not a valid transient compiler pointer")
    return result


def _sha256(value: Any, label: str) -> str:
    result = _text(value, label)
    if SHA256_RE.fullmatch(result) is None:
        raise Rejected(f"{label} must be lowercase SHA-256")
    return result


def _generation(kind: str, ordinal: int, label: str) -> str:
    if kind not in {"object", "varinfo"}:
        raise Rejected(f"{label} has an unsupported generation kind")
    if isinstance(ordinal, bool) or not isinstance(ordinal, int) or ordinal < 0:
        raise Rejected(f"{label} has an invalid generation ordinal")
    token = f"{kind}-generation-{ordinal:06d}"
    if GENERATION_RE.fullmatch(token) is None:
        raise Rejected(f"{label} is not pointer-free")
    return token


def _validate_generation_token(value: Any, kind: str, label: str) -> str:
    token = _text(value, label)
    expected = re.compile(rf"{re.escape(kind)}-generation-[0-9]{{6}}\Z")
    if expected.fullmatch(token) is None:
        raise Rejected(f"{label} is not a canonical pointer-free generation ID")
    return token


def _normalize_opcode(value: Any, label: str) -> bytes:
    if isinstance(value, bytes):
        result = value
    elif isinstance(value, bytearray):
        result = bytes(value)
    elif isinstance(value, str) and re.fullmatch(r"[0-9a-fA-F]+", value) and len(value) % 2 == 0:
        try:
            result = bytes.fromhex(value)
        except ValueError as exc:
            raise Rejected(f"{label} is not valid opcode bytes") from exc
    elif isinstance(value, Sequence) and not isinstance(value, (str, bytes, bytearray)):
        values = []
        for index, item in enumerate(value):
            number = _integer(item, f"{label}[{index}]")
            if number < 0 or number > 0xFF:
                raise Rejected(f"{label}[{index}] is not a byte")
            values.append(number)
        result = bytes(values)
    else:
        raise Rejected(f"{label} must contain opcode bytes")
    if not result:
        raise Rejected(f"{label} must not be empty")
    return result


def _validate_provenance(value: Any) -> dict[str, str]:
    if isinstance(value, Mapping) and "provenance" in value:
        value = value["provenance"]
    row = _strict_keys(value, PROVENANCE_FIELDS, "provenance")
    session_id = _text(row["session_id"], "provenance.session_id")
    if SESSION_RE.fullmatch(session_id) is None:
        raise Rejected("provenance.session_id is not a canonical capture session")
    return {
        "compiler_sha256": _sha256(row["compiler_sha256"], "provenance.compiler_sha256"),
        "session_id": session_id,
        "source_sha256": _sha256(row["source_sha256"], "provenance.source_sha256"),
        "trace_sha256": _sha256(row["trace_sha256"], "provenance.trace_sha256"),
    }


def _pointer_free(value: Any, location: str = "packet") -> None:
    forbidden = {
        "address",
        "addresses",
        "pointer",
        "pointers",
        "ptr",
        "raw_pointer",
        "raw_address",
        "object_pointer",
        "varinfo_pointer",
        "thread_id",
        "process_id",
        "handle",
    }
    if isinstance(value, Mapping):
        for key, child in value.items():
            normalized = str(key).strip().lower()
            if normalized in forbidden or normalized.endswith("_pointer") or normalized.endswith("_address"):
                raise Rejected(f"{location}.{key} exposes a raw pointer/address")
            _pointer_free(child, f"{location}.{key}")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            _pointer_free(child, f"{location}[{index}]")


def canonical_hash(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(value, ensure_ascii=True, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def seal(value: Mapping[str, Any]) -> dict[str, Any]:
    result = dict(value)
    result.pop("packet_sha256", None)
    result["packet_sha256"] = canonical_hash(result)
    return result


def validate_hook_image(read_image: Any) -> dict[str, str]:
    """Validate all authenticated hook prefixes before any breakpoint write."""

    if not callable(read_image):
        raise Rejected("hook-image reader is unavailable")
    observed: dict[str, str] = {}
    for row in HOOKS:
        hook_id = str(row["id"])
        expected = bytes.fromhex(str(row["prefix"]))
        try:
            actual = read_image(int(row["address"]), len(expected))
        except Exception as exc:
            raise Rejected(f"hook-image read failed for {hook_id}: {exc}") from exc
        if not isinstance(actual, bytes) or actual != expected:
            actual_hex = actual.hex() if isinstance(actual, bytes) else repr(actual)
            raise Rejected(f"wrong opcode at {hook_id}: {actual_hex} != {expected.hex()}")
        observed[hook_id] = actual.hex()
    return observed


def _hook_id(address: Any) -> str:
    value = _integer(address, "hook address")
    row = HOOK_BY_ADDRESS.get(value)
    if row is None:
        raise Rejected(f"unsupported front-end hook address: 0x{value:08x}")
    return str(row["id"])


class FrontendChronologySession:
    """Consume authenticated hook observations and emit a pointer-free packet."""

    def __init__(
        self,
        provenance: Mapping[str, Any],
        backend: Any | None = None,
        *,
        function: str = TARGET_FUNCTION,
    ) -> None:
        if not isinstance(function, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", function):
            raise Rejected("unsupported target function")
        self.provenance = _validate_provenance(provenance)
        self.backend = backend
        self.function = function
        self.started = False
        self.exited = False
        self.target_active = False
        self.target_entry_seen = False
        self.target_exit_seen = False
        self.reset_seen = False
        self.bulk_seen = False
        self.snapshot_seen = False
        self.hook_counts = {hook_id: 0 for hook_id in HOOK_ORDER}
        self.events: list[dict[str, Any]] = []
        self.generations: list[dict[str, Any]] = []
        self.by_pointer: dict[int, dict[str, Any]] = {}
        self.by_generation: dict[str, dict[str, Any]] = {}
        self.linked_generations: list[str] = []
        self.snapshot_bindings: list[dict[str, Any]] = []
        # MWCC reuses the same compiler globals for each function.  The reset
        # hook therefore starts a new private epoch; only the epoch whose
        # function boundary names the requested target is eligible for output.
        self.epochs: list[dict[str, Any]] = []
        self.current_epoch: dict[str, Any] | None = None
        self.selected_epoch: dict[str, Any] | None = None
        self.hooks_authenticated = False

    def _start_epoch(self) -> dict[str, Any]:
        epoch: dict[str, Any] = {
            "epoch_id": f"epoch-{len(self.epochs):06d}",
            "hook_counts": {hook_id: 0 for hook_id in HOOK_ORDER},
            "events": [],
            "generations": [],
            "by_pointer": {},
            "by_generation": {},
            "linked_generations": [],
            "snapshot_bindings": [],
            "bulk_seen": False,
            "snapshot_seen": False,
            "target_entry_seen": False,
            "target_exit_seen": False,
            "target_active": False,
            "closed_by_reset": False,
        }
        self.epochs.append(epoch)
        self.current_epoch = epoch
        self.hook_counts = epoch["hook_counts"]
        self.events = epoch["events"]
        self.generations = epoch["generations"]
        self.by_pointer = epoch["by_pointer"]
        self.by_generation = epoch["by_generation"]
        self.linked_generations = epoch["linked_generations"]
        self.snapshot_bindings = epoch["snapshot_bindings"]
        self.bulk_seen = False
        self.snapshot_seen = False
        self.target_active = False
        self.reset_seen = True
        return epoch

    def _use_selected_epoch(self) -> dict[str, Any]:
        epoch = self.selected_epoch
        if epoch is None:
            raise Rejected("target epoch is unbound")
        self.hook_counts = epoch["hook_counts"]
        self.events = epoch["events"]
        self.generations = epoch["generations"]
        self.by_pointer = epoch["by_pointer"]
        self.by_generation = epoch["by_generation"]
        self.linked_generations = epoch["linked_generations"]
        self.snapshot_bindings = epoch["snapshot_bindings"]
        self.bulk_seen = bool(epoch["bulk_seen"])
        self.snapshot_seen = bool(epoch["snapshot_seen"])
        self.target_active = bool(epoch["target_active"])
        self.reset_seen = True
        return epoch

    def _append(self, event_kind: str, payload: Mapping[str, Any]) -> dict[str, Any]:
        fields = EVENT_FIELDS.get(event_kind)
        if fields is None:
            raise Rejected(f"unsupported chronology event kind: {event_kind}")
        event: dict[str, Any] = {
            "schema": EVENT_SCHEMA,
            "sequence": len(self.events),
            "event_kind": event_kind,
        }
        event.update(dict(payload))
        _strict_keys(event, fields, f"event[{len(self.events)}]")
        _pointer_free(event, f"event[{len(self.events)}]")
        self.events.append(event)
        return event

    def _validate_event_opcode(self, hook_id: str, opcode: Any | None) -> None:
        if opcode is None:
            if not self.hooks_authenticated:
                raise Rejected(f"missing opcode evidence for {hook_id}")
            return
        observed = _normalize_opcode(opcode, f"{hook_id}.opcode")
        expected = bytes.fromhex(str(HOOK_BY_ID[hook_id]["prefix"]))
        if observed != expected:
            raise Rejected(f"wrong opcode at {hook_id}: {observed.hex()} != {expected.hex()}")

    def on_process_started(self, *, hook_bytes: Mapping[str, Any] | None = None) -> None:
        if self.started:
            raise Rejected("duplicate process-start event")
        if self.exited:
            raise Rejected("process-start arrived after process exit")
        if hook_bytes is not None:
            rows = _strict_keys(hook_bytes, HOOK_ORDER, "hook_bytes")
            for hook_id in HOOK_ORDER:
                expected = bytes.fromhex(str(HOOK_BY_ID[hook_id]["prefix"]))
                actual = _normalize_opcode(rows[hook_id], f"hook_bytes.{hook_id}")
                if actual != expected:
                    raise Rejected(f"wrong opcode at {hook_id}: {actual.hex()} != {expected.hex()}")
            self.hooks_authenticated = True
        elif self.backend is not None:
            self.hooks_authenticated = bool(validate_hook_image(getattr(self.backend, "read_image", None)))
        self.started = True
        if self.backend is not None:
            mark = getattr(self.backend, "mark_live_prefixes_validated", None)
            if callable(mark):
                mark()
            install = getattr(self.backend, "install_breakpoint", None)
            if callable(install):
                for row in HOOKS:
                    install(int(row["address"]))

    def _require_started(self) -> None:
        if not self.started or self.exited:
            raise Rejected("hook event arrived outside process lifetime")

    def on_hook(
        self,
        hook_id: str,
        *,
        opcode: Any | None = None,
        pointer: Any | None = None,
        object_pointers: Sequence[Any] | None = None,
        phase: str | None = None,
        function: str | None = None,
    ) -> None:
        self._require_started()
        if hook_id not in HOOK_BY_ID:
            raise Rejected(f"unsupported front-end hook ID: {hook_id}")
        self._validate_event_opcode(hook_id, opcode)

        if hook_id == "reset":
            # A reset is the compiler's function-local-list delimiter.  The
            # reset which follows CapSelectMasuPlayer therefore closes the
            # selected epoch; it is not evidence that that epoch was
            # duplicated or malformed.  Keep the selected epoch immutable and
            # start a fresh list for the next function.
            if self.current_epoch is not None and self.current_epoch["target_active"]:
                self.current_epoch["target_active"] = False
                self.current_epoch["closed_by_reset"] = True
            epoch = self._start_epoch()
            epoch["hook_counts"]["reset"] += 1
            self._append("reset", {"hook_id": hook_id})
            return

        epoch = self.current_epoch
        if epoch is None or not self.reset_seen:
            raise Rejected(f"{hook_id} arrived before authenticated reset")

        # Once the selected target has closed, later codegen in the same
        # reset-delimited compiler epoch belongs to another function.  Keep
        # the transport running, but never contaminate the selected packet.
        if (
            self.selected_epoch is epoch
            and epoch["target_exit_seen"]
            and hook_id in GENERIC_HOOK_IDS + ("bulk_object_link",)
        ):
            return

        epoch["hook_counts"][hook_id] += 1

        if hook_id in GENERIC_HOOK_IDS:
            if pointer is None:
                raise Rejected(f"{hook_id} completed insertion has no Object identity")
            raw_pointer = _pointer(pointer, f"{hook_id}.pointer")
            if raw_pointer in self.by_pointer:
                raise Rejected(f"duplicate/ambiguous Object insertion pointer at {hook_id}")
            generation_id = _generation("object", len(epoch["generations"]), "Object generation")
            row = {
                "generation_id": generation_id,
                "inserted_by": hook_id,
                "insert_event_sequence": len(self.events),
                "link_event_sequence": None,
                "linked": False,
                "varinfo_generation_id": None,
                "home_value": None,
                "evidence": dict(self.provenance),
            }
            epoch["by_pointer"][raw_pointer] = row
            epoch["by_generation"][generation_id] = row
            epoch["generations"].append(row)
            self._append(
                "generic_insertion",
                {"hook_id": hook_id, "generation_id": generation_id},
            )
            return

        if hook_id == "bulk_object_link":
            if self.bulk_seen:
                raise Rejected("duplicate bulk Object-link hook")
            if not self.generations:
                raise Rejected("bulk Object-link arrived before any generic insertion")
            if object_pointers is None or isinstance(object_pointers, (str, bytes)):
                raise Rejected("bulk Object-link payload is missing")
            raw_pointers = [_pointer(value, f"{hook_id}.object_pointers[{index}]") for index, value in enumerate(object_pointers)]
            if not raw_pointers:
                raise Rejected("bulk Object-link payload is empty")
            if len(set(raw_pointers)) != len(raw_pointers):
                raise Rejected("bulk Object-link payload contains duplicate Object identities")
            if len(raw_pointers) != len(self.generations):
                raise Rejected("bulk Object-link coverage is incomplete or ambiguous")
            generation_ids: list[str] = []
            for raw_pointer in raw_pointers:
                row = self.by_pointer.get(raw_pointer)
                if row is None:
                    raise Rejected("bulk Object-link references a missing insertion")
                if row["linked"]:
                    raise Rejected("bulk Object-link repeats an Object generation")
                row["linked"] = True
                row["link_event_sequence"] = len(self.events)
                generation_ids.append(str(row["generation_id"]))
            epoch["linked_generations"] = generation_ids
            epoch["bulk_seen"] = True
            self.linked_generations = generation_ids
            self.bulk_seen = True
            self._append(
                "bulk_object_link",
                {"hook_id": hook_id, "generation_ids": generation_ids},
            )
            return

        if hook_id == "target_boundary":
            if phase not in {"entry", "exit"}:
                raise Rejected("target boundary phase is missing or ambiguous")
            if phase == "entry":
                if function != self.function:
                    raise Rejected("target-boundary entry selected the wrong function")
                if self.selected_epoch is not None:
                    raise Rejected("ambiguous multiple target epochs")
                if epoch["target_entry_seen"] or self.target_entry_seen:
                    raise Rejected("duplicate target-boundary entry")
                epoch["target_active"] = True
                epoch["target_entry_seen"] = True
                self.selected_epoch = epoch
                self.target_active = True
                self.target_entry_seen = True
            else:
                if self.selected_epoch is not epoch:
                    return
                if not epoch["target_active"] or epoch["target_exit_seen"]:
                    raise Rejected("duplicate or unmatched target-boundary exit")
                if not epoch["bulk_seen"]:
                    raise Rejected("target-boundary exit arrived before bulk Object-link coverage")
                epoch["target_active"] = False
                epoch["target_exit_seen"] = True
                self.target_active = False
                self.target_exit_seen = True
            self._append("target_boundary", {"hook_id": hook_id, "phase": phase})
            return

        raise Rejected(f"hook {hook_id} has no chronology handler")

    def on_hook_complete(self, hook_id: str, *, pointer: Any | None = None, object_pointers: Sequence[Any] | None = None) -> None:
        """Capture a hook after its single-step has executed.

        Native transport calls this method after the inherited Win32
        single-step primitive.  Opcode identity was authenticated before any
        breakpoint was installed, so the completed event does not repeat raw
        instruction bytes.
        """

        self.on_hook(
            hook_id,
            pointer=pointer,
            object_pointers=object_pointers,
        )

    def on_target_boundary(self, *, phase: str, function: str, opcode: Any | None = None) -> None:
        self.on_hook("target_boundary", phase=phase, function=function, opcode=opcode)

    def on_post_allocation_snapshot(self, rows: Sequence[Mapping[str, Any]]) -> None:
        self._require_started()
        if self.current_epoch is not self.selected_epoch:
            raise Rejected("post-allocation snapshot is unbound to the target epoch")
        if not self.target_entry_seen:
            raise Rejected("post-allocation snapshot arrived before target entry")
        if not self.bulk_seen:
            raise Rejected("post-allocation snapshot has no bulk Object-link binding")
        if self.snapshot_seen:
            raise Rejected("duplicate post-allocation snapshot")
        if not isinstance(rows, Sequence) or isinstance(rows, (str, bytes)) or not rows:
            raise Rejected("post-allocation Object list snapshot is empty")

        object_rows: dict[int, Mapping[str, Any]] = {}
        varinfo_rows: dict[int, Mapping[str, Any]] = {}
        for index, raw in enumerate(rows):
            if not isinstance(raw, Mapping):
                raise Rejected(f"post-allocation snapshot row {index} is not an object")
            keys = set(raw)
            allowed = {"pointer", "varinfo_pointer", "home_value"}
            if not keys.issubset(allowed) or not {"pointer", "varinfo_pointer"}.issubset(keys):
                raise Rejected(f"post-allocation snapshot row {index} field allowlist mismatch")
            object_pointer = _pointer(raw["pointer"], f"snapshot[{index}].pointer")
            varinfo_pointer = _pointer(raw["varinfo_pointer"], f"snapshot[{index}].varinfo_pointer")
            if object_pointer in object_rows:
                raise Rejected("post-allocation snapshot contains duplicate Object identity")
            if varinfo_pointer in varinfo_rows:
                raise Rejected("post-allocation snapshot contains duplicate VarInfo identity")
            object_rows[object_pointer] = raw
            varinfo_rows[varinfo_pointer] = raw

        bindings: list[dict[str, Any]] = []
        seen_varinfo: set[int] = set()
        for generation_id in self.linked_generations:
            generation = self.by_generation[generation_id]
            raw_pointer = next(pointer for pointer, row in self.by_pointer.items() if row is generation)
            raw = object_rows.get(raw_pointer)
            if raw is None:
                raise Rejected("post-allocation snapshot is missing a linked Object generation")
            varinfo_pointer = _pointer(raw["varinfo_pointer"], "snapshot.varinfo_pointer")
            if varinfo_pointer in seen_varinfo:
                raise Rejected("post-allocation snapshot creates an ambiguous VarInfo join")
            seen_varinfo.add(varinfo_pointer)
            varinfo_id = _generation("varinfo", len(seen_varinfo) - 1, "VarInfo generation")
            home_value = raw.get("home_value")
            if home_value is None and self.backend is not None:
                snapshot_varinfo = getattr(self.backend, "snapshot_varinfo", None)
                if callable(snapshot_varinfo):
                    details = snapshot_varinfo(varinfo_pointer)
                    if not isinstance(details, Mapping) or "home_value" not in details:
                        raise Rejected("post-allocation VarInfo home snapshot is incomplete")
                    home_value = details["home_value"]
            if home_value is not None:
                home_value = _integer(home_value, "snapshot.home_value")
            generation["varinfo_generation_id"] = varinfo_id
            generation["home_value"] = home_value
            bindings.append(
                {
                    "object_generation_id": generation_id,
                    "varinfo_generation_id": varinfo_id,
                    "home_value": home_value,
                }
            )
        if len(bindings) != len(self.generations) or not all(row["linked"] for row in self.generations):
            raise Rejected("post-allocation snapshot coverage is incomplete")
        self.current_epoch["snapshot_bindings"] = bindings
        self.current_epoch["snapshot_seen"] = True
        self.snapshot_bindings = bindings
        self.snapshot_seen = True
        self._append("post_allocation_snapshot", {"bindings": bindings})

    def on_process_exit(self, exit_code: int = 0) -> dict[str, Any]:
        if self.exited:
            raise Rejected("duplicate process-exit event")
        if not self.started:
            raise Rejected("process exited before capture start")
        if exit_code != 0:
            raise Rejected(f"compiler process exited with status {exit_code}")
        self.exited = True
        selected = self.selected_epoch
        if selected is None:
            raise Rejected("target function entry was not bound to an epoch")
        if not selected["target_entry_seen"]:
            raise Rejected("target boundary chronology is incomplete")
        if not selected["generations"]:
            raise Rejected("selected target epoch has no Object generations")
        if not selected["bulk_seen"] or not selected["snapshot_seen"]:
            raise Rejected("bulk-link or post-allocation chronology is incomplete")
        if len(selected["linked_generations"]) != len(selected["generations"]):
            raise Rejected("selected target epoch has missing Object links")
        self._use_selected_epoch()
        return self.packet()

    def run(self, events: Iterable[Any]) -> dict[str, Any]:
        """Consume fake/native event records and close the capture."""

        for raw in events:
            if isinstance(raw, Mapping):
                event_kind = raw.get("event_kind")
                address = raw.get("address")
                payload = raw.get("payload")
            else:
                event_kind = getattr(raw, "event_kind", None)
                address = getattr(raw, "address", None)
                payload = getattr(raw, "payload", None)
            if not isinstance(payload, Mapping):
                payload = {}
            if event_kind in {"process_started", "start"}:
                self.on_process_started(hook_bytes=payload.get("hook_bytes"))
            elif event_kind in {"breakpoint", "hook"}:
                hook_id = payload.get("hook_id") if "hook_id" in payload else _hook_id(address)
                if not isinstance(hook_id, str):
                    raise Rejected("hook ID is missing")
                if hook_id == "target_boundary":
                    self.on_target_boundary(
                        phase=payload.get("phase"),
                        function=payload.get("function"),
                        opcode=payload.get("opcode"),
                    )
                elif hook_id == "bulk_object_link":
                    self.on_hook(
                        hook_id,
                        opcode=payload.get("opcode"),
                        object_pointers=payload.get("object_pointers"),
                    )
                else:
                    self.on_hook(
                        hook_id,
                        opcode=payload.get("opcode"),
                        pointer=payload.get("pointer", payload.get("object_pointer")),
                    )
            elif event_kind in {"post_allocation_snapshot", "compiler_list_snapshot"}:
                rows = payload.get("rows", payload.get("objects"))
                self.on_post_allocation_snapshot(rows)
            elif event_kind in {"process_exit", "exit"}:
                return self.on_process_exit(_integer(payload.get("exit_code", 0), "exit_code"))
            elif event_kind == "disconnect":
                raise Rejected(f"native transport disconnected: {payload.get('reason', 'unknown')}")
            else:
                raise Rejected(f"unsupported frontend transport event: {event_kind!r}")
        if not self.exited:
            raise Rejected("frontend transport ended without process exit")
        return self.packet()

    def packet(self) -> dict[str, Any]:
        if not self.exited:
            raise Rejected("cannot seal a live chronology session")
        if self.selected_epoch is None:
            raise Rejected("cannot seal without a selected target epoch")
        if self.current_epoch is not self.selected_epoch:
            self._use_selected_epoch()
        packet = {
            "schema": SCHEMA,
            "schema_version": SCHEMA_VERSION,
            "tool_version": TOOL_VERSION,
            "status": "CAPTURED_UNKNOWN_OWNERSHIP",
            "diagnostic_only": True,
            "board_admission": False,
            "exactness_claim": False,
            "authority_advanced": False,
            "function": self.function,
            "provenance": dict(self.provenance),
            "hook_plan": list(HOOK_ORDER),
            "hook_coverage": dict(self.hook_counts),
            "events": [dict(event) for event in self.events],
            "generations": [dict(row) for row in self.generations],
            "limitations": list(LIMITATIONS),
        }
        _pointer_free(packet)
        return seal(packet)


def _validate_event(event: Any, index: int, generation_ids: set[str]) -> None:
    if not isinstance(event, Mapping):
        raise Rejected(f"events[{index}] is not an object")
    event_kind = _text(event.get("event_kind"), f"events[{index}].event_kind")
    expected = EVENT_FIELDS.get(event_kind)
    if expected is None:
        raise Rejected(f"events[{index}] has an unsupported event kind")
    _strict_keys(event, expected, f"events[{index}]")
    if event.get("schema") != EVENT_SCHEMA or event.get("sequence") != index:
        raise Rejected(f"events[{index}] schema/sequence mismatch")
    hook_id = event.get("hook_id")
    if event_kind == "reset" and hook_id != "reset":
        raise Rejected("reset event hook mismatch")
    if event_kind == "generic_insertion":
        if hook_id not in GENERIC_HOOK_IDS:
            raise Rejected("generic insertion hook mismatch")
        generation_ids.add(_validate_generation_token(event.get("generation_id"), "object", f"events[{index}].generation_id"))
    elif event_kind == "bulk_object_link":
        if hook_id != "bulk_object_link":
            raise Rejected("bulk-link event hook mismatch")
        raw_ids = event.get("generation_ids")
        if not isinstance(raw_ids, list) or not raw_ids:
            raise Rejected("bulk-link generation coverage is missing")
        if len(set(raw_ids)) != len(raw_ids):
            raise Rejected("bulk-link generation coverage is ambiguous")
        for value in raw_ids:
            token = _validate_generation_token(value, "object", f"events[{index}].generation_ids")
            if token not in generation_ids:
                raise Rejected("bulk-link references an unknown generation")
    elif event_kind == "target_boundary":
        if hook_id != "target_boundary" or event.get("phase") not in {"entry", "exit"}:
            raise Rejected("target-boundary event is invalid")
    elif event_kind == "post_allocation_snapshot":
        raw_bindings = event.get("bindings")
        if not isinstance(raw_bindings, list) or not raw_bindings:
            raise Rejected("post-allocation binding coverage is missing")
        seen: set[str] = set()
        seen_varinfos: set[str] = set()
        for bind_index, raw in enumerate(raw_bindings):
            bind = _strict_keys(
                raw,
                {"object_generation_id", "varinfo_generation_id", "home_value"},
                f"events[{index}].bindings[{bind_index}]",
            )
            object_id = _validate_generation_token(bind["object_generation_id"], "object", f"events[{index}].bindings[{bind_index}].object_generation_id")
            varinfo_id = _validate_generation_token(bind["varinfo_generation_id"], "varinfo", f"events[{index}].bindings[{bind_index}].varinfo_generation_id")
            if object_id in seen or object_id not in generation_ids:
                raise Rejected("post-allocation binding is duplicate or unbound")
            if varinfo_id in seen_varinfos:
                raise Rejected("post-allocation binding creates an ambiguous VarInfo join")
            seen.add(object_id)
            seen_varinfos.add(varinfo_id)
            if bind["home_value"] is not None:
                _integer(bind["home_value"], f"events[{index}].bindings[{bind_index}].home_value")


def validate_packet(value: Mapping[str, Any]) -> dict[str, Any]:
    """Validate a sealed native chronology packet without native access."""

    packet = dict(_strict_keys(value, PACKET_FIELDS, "packet"))
    if packet.get("schema") != SCHEMA or packet.get("schema_version") != SCHEMA_VERSION:
        raise Rejected("packet schema/version mismatch")
    if packet.get("tool_version") != TOOL_VERSION:
        raise Rejected("packet tool version mismatch")
    if packet.get("status") != "CAPTURED_UNKNOWN_OWNERSHIP":
        raise Rejected("packet is not an authenticated diagnostic capture")
    if packet.get("diagnostic_only") is not True or packet.get("board_admission") is not False or packet.get("exactness_claim") is not False or packet.get("authority_advanced") is not False:
        raise Rejected("packet policy mismatch")
    function = packet.get("function")
    if not isinstance(function, str) or re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", function) is None:
        raise Rejected("packet target function is not canonical")
    provenance = _validate_provenance(packet.get("provenance"))
    if packet.get("hook_plan") != list(HOOK_ORDER):
        raise Rejected("packet hook plan mismatch")
    coverage = packet.get("hook_coverage")
    _strict_keys(coverage, HOOK_ORDER, "packet.hook_coverage")
    for hook_id in HOOK_ORDER:
        _integer(coverage[hook_id], f"packet.hook_coverage.{hook_id}")
    if coverage["reset"] != 1:
        raise Rejected("packet selected epoch must contain exactly one reset")
    if coverage["bulk_object_link"] < 1 or coverage["target_boundary"] < 1:
        raise Rejected("packet selected epoch link/boundary coverage is incomplete")
    if sum(coverage[generic_id] for generic_id in GENERIC_HOOK_IDS) < 1:
        raise Rejected("packet selected epoch has no generic Object insertion")
    events = packet.get("events")
    if not isinstance(events, list) or not events:
        raise Rejected("packet events are missing")
    generation_ids: set[str] = set()
    seen_bulk = False
    reset_count = 0
    target_phases: list[str] = []
    snapshot_index: int | None = None
    for index, event in enumerate(events):
        _validate_event(event, index, generation_ids)
        if event["event_kind"] == "reset":
            reset_count += 1
            if reset_count > 1:
                raise Rejected("packet contains multiple reset epochs")
        elif event["event_kind"] == "bulk_object_link":
            if seen_bulk:
                raise Rejected("packet has duplicate bulk-link event")
            seen_bulk = True
            if set(event["generation_ids"]) != generation_ids:
                raise Rejected("packet bulk-link coverage is incomplete")
        elif event["event_kind"] == "target_boundary":
            target_phases.append(event["phase"])
        elif event["event_kind"] == "post_allocation_snapshot":
            if snapshot_index is not None:
                raise Rejected("packet has duplicate post-allocation snapshot")
            snapshot_index = index
    if target_phases not in (["entry"], ["entry", "exit"]):
        raise Rejected("packet target-boundary chronology is incomplete")
    if reset_count != 1:
        raise Rejected("packet selected epoch reset coverage is incomplete")
    if not seen_bulk or snapshot_index is None:
        raise Rejected("packet chronology is incomplete")
    entry_index = events.index(next(event for event in events if event["event_kind"] == "target_boundary" and event["phase"] == "entry"))
    bulk_index = events.index(next(event for event in events if event["event_kind"] == "bulk_object_link"))
    if len(target_phases) == 2:
        exit_index = events.index(next(event for event in events if event["event_kind"] == "target_boundary" and event["phase"] == "exit"))
        if bulk_index > exit_index:
            raise Rejected("packet bulk-link event is after target exit")
        if snapshot_index <= exit_index:
            raise Rejected("packet snapshot is before target exit")
    elif snapshot_index <= entry_index:
        raise Rejected("packet snapshot is before target entry")
    generations = packet.get("generations")
    if not isinstance(generations, list) or not generations:
        raise Rejected("packet generation inventory is missing")
    seen_generations: set[str] = set()
    seen_varinfos: set[str] = set()
    for index, raw in enumerate(generations):
        row = _strict_keys(raw, GENERATION_FIELDS, f"packet.generations[{index}]")
        generation_id = _validate_generation_token(row["generation_id"], "object", f"packet.generations[{index}].generation_id")
        varinfo_id = _validate_generation_token(row["varinfo_generation_id"], "varinfo", f"packet.generations[{index}].varinfo_generation_id")
        if generation_id in seen_generations or generation_id not in generation_ids:
            raise Rejected("packet generation identity is duplicate or unbound")
        if varinfo_id in seen_varinfos:
            raise Rejected("packet VarInfo generation identity is ambiguous")
        seen_generations.add(generation_id)
        seen_varinfos.add(varinfo_id)
        if row["inserted_by"] not in GENERIC_HOOK_IDS or row["linked"] is not True:
            raise Rejected("packet generation insertion/link identity is invalid")
        _integer(row["insert_event_sequence"], f"packet.generations[{index}].insert_event_sequence")
        _integer(row["link_event_sequence"], f"packet.generations[{index}].link_event_sequence")
        if row["home_value"] is not None:
            _integer(row["home_value"], f"packet.generations[{index}].home_value")
        evidence = _strict_keys(row["evidence"], PROVENANCE_FIELDS, f"packet.generations[{index}].evidence")
        if {key: evidence[key] for key in sorted(PROVENANCE_FIELDS)} != provenance:
            raise Rejected("packet generation provenance is not bound")
    if seen_generations != generation_ids:
        raise Rejected("packet generation coverage is incomplete")
    limitations = packet.get("limitations")
    if not isinstance(limitations, list) or not limitations or not all(isinstance(item, str) and item for item in limitations):
        raise Rejected("packet limitations are missing")
    _pointer_free(packet)
    digest = packet.get("packet_sha256")
    if _sha256(digest, "packet.packet_sha256") != canonical_hash({key: val for key, val in packet.items() if key != "packet_sha256"}):
        raise Rejected("packet self-digest mismatch")
    return packet


def capture_with_backend(
    provenance: Mapping[str, Any],
    backend: Any,
    *,
    function: str = TARGET_FUNCTION,
) -> dict[str, Any]:
    """Run a backend-provided event stream and always close its transport."""

    session = FrontendChronologySession(provenance, backend, function=function)
    try:
        result = backend.run(session)
        if result is not None:
            if isinstance(result, Mapping):
                packet = dict(result)
                return validate_packet(packet)
            return session.run(result)
        if not session.exited:
            raise Rejected("native backend ended without process exit")
        return session.packet()
    finally:
        close = getattr(backend, "close", None)
        if callable(close):
            close()


class FrontendWow64Backend(_stack_home.NativeWow64Backend):
    """GC/2.6 frontend hook adapter reusing stack-home Win32 primitives."""

    def _frontend_hook_id(self, normalized: int) -> str | None:
        row = HOOK_BY_ADDRESS.get(normalized)
        return str(row["id"]) if row is not None else None

    def _frontend_pointer(self, hook_id: str, thread_id: int) -> int | None:
        if hook_id in GENERIC_HOOK_IDS:
            register = GENERIC_OBJECT_REGISTERS.get(hook_id)
            if register is None:
                raise Rejected(f"generic insertion hook has no Object register mapping: {hook_id}")
            return self.read_register(thread_id, register)
        if hook_id == "bulk_object_link":
            return self.read_register(thread_id, "ebx")
        return None

    def run(self, session: FrontendChronologySession) -> None:  # pragma: no cover - native-only transport
        import ctypes

        event = self.native.DEBUG_EVENT()
        pending: dict[int, tuple[str, int]] = {}
        while True:
            if not self.native.kernel32.WaitForDebugEvent(ctypes.byref(event), 1000):
                error = ctypes.get_last_error()
                if error == self.native.ERROR_SEM_TIMEOUT:
                    continue
                session.on_disconnect(f"WaitForDebugEvent error {error}")
            code = int(event.dwDebugEventCode)
            pid, tid = int(event.dwProcessId), int(event.dwThreadId)
            if code == self.native.CREATE_PROCESS_DEBUG_EVENT:
                self.base = int(event.u.CreateProcessInfo.lpBaseOfImage or 0)
                if self.base != _stack_home.KNOWN_IMAGE_BASE:
                    raise Rejected(f"live image base mismatch: 0x{self.base:08x}")
                handle = int(event.u.CreateProcessInfo.hThread or 0)
                if not handle:
                    handle = self.threads.pop(0, 0)
                self._register_thread(tid, handle)
                session.on_process_started()
                file_handle = int(event.u.CreateProcessInfo.hFile or 0)
                if file_handle:
                    self.native.kernel32.CloseHandle(file_handle)
            elif code == self.native.CREATE_THREAD_DEBUG_EVENT:
                self._register_thread(tid, int(event.u.CreateThread.hThread or 0))
            elif code == self.native.EXIT_THREAD_DEBUG_EVENT:
                handle = self.threads.get(tid)
                if handle:
                    self._close_thread(tid, handle)
            elif code == self.native.EXIT_PROCESS_DEBUG_EVENT:
                self.exited = True
                session.on_process_exit(int(event.u.ExitProcess.dwExitCode))
            elif code == self.native.EXCEPTION_DEBUG_EVENT:
                exception = event.u.Exception.ExceptionRecord
                exception_code = int(exception.ExceptionCode)
                address = int(exception.ExceptionAddress or 0)
                is_step = exception_code in (self.native.EXCEPTION_SINGLE_STEP, self.native.EXCEPTION_WX86_SINGLE_STEP)
                is_break = exception_code in (self.native.EXCEPTION_BREAKPOINT, self.native.EXCEPTION_WX86_BREAKPOINT)
                if is_step:
                    step = self.pending_steps.pop(tid, None)
                    if step is None or tid not in pending:
                        session.on_disconnect("frontend single-step had no pending hook")
                    context = self._get_context(self.threads[tid])
                    context.EFlags &= ~self.native.WOW64_CONTEXT_TF
                    self._set_context(self.threads[tid], context)
                    hook_id, normalized = pending.pop(tid)
                    if hook_id not in {"ignore_boundary", "target_boundary"}:
                        if hook_id == "bulk_object_link":
                            # The completed bulk linker has updated the same
                            # compiler list consumed by the stack-home lane.
                            # Reuse that audited reader for the transient
                            # pointer set; the session converts it to IDs.
                            rows = list(self.snapshot_objects())
                            object_pointers = [row.get("pointer") for row in rows]
                            session.on_hook_complete(
                                hook_id,
                                object_pointers=object_pointers,
                            )
                            # The target boundary is a function-entry
                            # observation, not a paired exit event.  If the
                            # bulk linker ran first, take the authenticated
                            # list snapshot as soon as the selected entry is
                            # known.  The inverse order is handled at the
                            # boundary below.
                            if (
                                session.selected_epoch is session.current_epoch
                                and session.target_entry_seen
                                and not session.snapshot_seen
                            ):
                                session.on_post_allocation_snapshot(self.snapshot_objects())
                        else:
                            session.on_hook_complete(
                                hook_id,
                                pointer=self._frontend_pointer(hook_id, tid),
                            )
                    self.install_breakpoint(normalized)
                elif is_break:
                    normalized = self._normalise(address)
                    hook_id = self._frontend_hook_id(normalized)
                    if hook_id is None:
                        if session.target_active:
                            session.on_disconnect(f"unexpected frontend breakpoint 0x{normalized:08x}")
                    elif hook_id == "target_boundary":
                        function = self.current_function()
                        if function == session.function:
                            # Any target entry after one has already been
                            # selected is an ambiguity, including one in a
                            # later reset-delimited epoch.
                            session.on_target_boundary(phase="entry", function=function)
                            if session.bulk_seen and not session.snapshot_seen:
                                session.on_post_allocation_snapshot(self.snapshot_objects())
                            self.single_step(normalized, tid, rearm=True)
                            pending[tid] = ("target_boundary", normalized)
                        else:
                            # The site is a general function-entry hook.  It
                            # is intentionally ignored for non-target
                            # functions, but the epoch reset and its Object
                            # hooks are still processed independently.
                            self.single_step(normalized, tid, rearm=True)
                            pending[tid] = ("ignore_boundary", normalized)
                    else:
                        self.single_step(normalized, tid, rearm=True)
                        pending[tid] = (hook_id, normalized)
            if not self.native.kernel32.ContinueDebugEvent(pid, tid, self.native.DBG_CONTINUE):
                session.on_disconnect("ContinueDebugEvent failed")
            if self.exited:
                break


def load_packet(path: str | Path) -> dict[str, Any]:
    packet_path = Path(path)
    try:
        value = json.loads(packet_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise Rejected(f"invalid packet {packet_path}: {exc}") from exc
    return validate_packet(value)


def _fixture() -> dict[str, Any]:
    hashes = {
        "source_sha256": "a" * 64,
        "compiler_sha256": "b" * 64,
        "trace_sha256": "c" * 64,
        "session_id": "session-0123456789abcdef",
    }
    hook_bytes = {str(row["id"]): str(row["prefix"]) for row in HOOKS}
    session = FrontendChronologySession(hashes)
    session.on_process_started(hook_bytes=hook_bytes)
    session.on_hook("reset")
    session.on_hook("target_boundary", phase="entry", function=TARGET_FUNCTION)
    session.on_hook("generic_insert_0", pointer=0x1000)
    session.on_hook("generic_insert_1", pointer=0x1100)
    session.on_hook("generic_insert_2", pointer=0x1200)
    session.on_hook("bulk_object_link", object_pointers=[0x1000, 0x1100, 0x1200])
    session.on_hook("target_boundary", phase="exit", function=TARGET_FUNCTION)
    session.on_post_allocation_snapshot(
        [
            {"pointer": 0x1000, "varinfo_pointer": 0x2000, "home_value": -32},
            {"pointer": 0x1100, "varinfo_pointer": 0x2100, "home_value": -36},
            {"pointer": 0x1200, "varinfo_pointer": 0x2200, "home_value": -40},
        ]
    )
    return session.on_process_exit()


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("self-test")
    validate = sub.add_parser("validate")
    validate.add_argument("packet", type=Path)
    args = parser.parse_args(argv)
    try:
        if args.command == "self-test":
            packet = _fixture()
            validate_packet(packet)
            result = {"schema": SCHEMA, "status": "OK", "generations": len(packet["generations"])}
        else:
            packet = load_packet(args.packet)
            result = {"schema": SCHEMA, "status": "valid", "generations": len(packet["generations"])}
        print(json.dumps(result, sort_keys=True))
        return 0
    except Exception as exc:
        print(json.dumps({"schema": SCHEMA, "status": "UNKNOWN", "reason": str(exc)}, sort_keys=True), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
