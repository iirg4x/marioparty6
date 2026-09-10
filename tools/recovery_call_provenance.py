#!/usr/bin/env python3
"""Bounded call-argument provenance diagnostics for aligned objdiff rows.

This module deliberately answers a narrow question: when two aligned
instruction streams call the same contract, do the selected argument
registers have the same *known* call-return origin?  It is diagnostic only;
it does not establish semantic equivalence or advance any recovery gate.
"""
from __future__ import annotations

import argparse
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
import json
import re
import sys
from typing import Any


SCHEMA = "recovery_call_provenance/v1"
MAX_ITEMS = 16

_GPR_RE = re.compile(r"r(?:[0-9]|[12][0-9]|3[01])\Z", re.IGNORECASE)
_OPCODE_RE = re.compile(r"^\s*(?P<opcode>[A-Za-z][A-Za-z0-9_.]*)\b", re.IGNORECASE)
_INTEGER_RE = re.compile(r"[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+)\Z")
_MEMORY_RE = re.compile(
    r"^\s*(?P<displacement>[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+))"
    r"\s*\(\s*(?P<base>r(?:[0-9]|[12][0-9]|3[01]))\s*\)\s*\Z",
    re.IGNORECASE,
)

_DIRECT_CALL_OPCODES = frozenset({"bl", "bla", "bcl", "bcla"})
_INDIRECT_CALL_OPCODES = frozenset({"bctrl", "bcctrl", "blrl", "bclrl"})
_CALL_OPCODES = _DIRECT_CALL_OPCODES | _INDIRECT_CALL_OPCODES
_TERMINATORS = frozenset({"blr", "bclr", "rfi", "rfid", "sc"})
_INDIRECT_BRANCHES = frozenset({"bctr", "bcctr"})
_UNCONDITIONAL_BRANCHES = frozenset({"b", "ba"})
_VOLATILE_REGISTERS = ("r0", *(f"r{index}" for index in range(3, 13)))

_STORE_WIDTHS: dict[str, int] = {
    "stb": 1,
    "stbu": 1,
    "stbx": 1,
    "stbux": 1,
    "sth": 2,
    "sthu": 2,
    "sthx": 2,
    "sthux": 2,
    "stw": 4,
    "stwu": 4,
    "stwx": 4,
    "stwux": 4,
    "stfs": 4,
    "stfsu": 4,
    "stfsx": 4,
    "stfsux": 4,
    "stfd": 8,
    "stfdu": 8,
    "stfdx": 8,
    "stfdux": 8,
    "psq_st": 8,
    "psq_stu": 8,
    "psq_stx": 8,
    "psq_stux": 8,
    "stvx": 16,
    "stvxl": 16,
}

# Instructions in these families do not define a GPR in operand zero.  A
# broad prefix list is safer than accidentally carrying an origin through an
# unsupported operation.  Supported definitions are handled before this set.
_NO_GPR_DEST_PREFIXES = (
    "st",
    "b",
    "cmp",
    "cmpl",
    "mt",
    "mcr",
    "cr",
    "tw",
    "td",
    "sync",
    "isync",
    "dcb",
    "icbi",
    "eieio",
)
_ABI_HELPER_MARKERS = ("savegpr", "restgpr", "savefpr", "restfpr")


@dataclass(frozen=True)
class _Contract:
    name: str
    arguments: tuple[str, ...]
    return_r3: bool


@dataclass(frozen=True)
class _Origin:
    row: int
    callee: str
    depth: int = 0


@dataclass(frozen=True)
class _Fact:
    """Finite possible-origin fact with conservative unknown causes."""

    values: frozenset[_Origin | None]
    causes: frozenset[str]

    @classmethod
    def unknown(cls, *causes: str) -> _Fact:
        return cls(frozenset({None}), frozenset(causes or ("unknown",)))

    @classmethod
    def known(cls, origin: _Origin) -> _Fact:
        return cls(frozenset({origin}), frozenset())

    def join(self, other: _Fact) -> _Fact:
        values = self.values | other.values
        if len(values) > 16:
            return _Fact.unknown("too_many_possible_origins")
        if "too_many_possible_origins" in self.causes | other.causes:
            return _Fact.unknown("too_many_possible_origins")
        return _Fact(values, self.causes | other.causes)

    @property
    def origin(self) -> _Origin | None:
        if len(self.values) == 1:
            value = next(iter(self.values))
            if isinstance(value, _Origin):
                return value
        return None


@dataclass(frozen=True)
class _MemCell:
    offset: int
    width: int
    fact: _Fact


@dataclass
class _State:
    registers: dict[str, _Fact]
    memory: dict[int, _MemCell]
    memory_unknown: frozenset[str]

    def clone(self) -> _State:
        return _State(dict(self.registers), dict(self.memory), self.memory_unknown)


@dataclass(frozen=True)
class _CallSite:
    row: int
    callee: str | None
    direct: bool
    helper: bool


@dataclass
class _Analysis:
    before: dict[int, _State]
    calls: dict[int, _CallSite]
    cfg_unknowns: tuple[dict[str, Any], ...]


def _parse_integer(value: Any) -> int | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, str) and _INTEGER_RE.fullmatch(value.strip()):
        text = value.strip()
        sign = -1 if text.startswith("-") else 1
        digits = text[1:] if text[:1] in {"+", "-"} else text
        base = 16 if digits.lower().startswith("0x") else 10
        try:
            return sign * int(digits[2:] if base == 16 else digits, base)
        except ValueError:
            return None
    return None


def _gpr(value: str | None) -> str | None:
    if not isinstance(value, str):
        return None
    value = value.strip().lower()
    return value if _GPR_RE.fullmatch(value) else None


def _parts(row: Any) -> tuple[str, list[str]] | None:
    if not isinstance(row, Mapping):
        return None
    instruction = row.get("instruction")
    if not isinstance(instruction, Mapping):
        return None
    text = instruction.get("formatted")
    if not isinstance(text, str):
        return None
    match = _OPCODE_RE.match(text)
    if match is None:
        return None
    operands = [part.strip().lower() for part in text[match.end():].split(",")]
    return match.group("opcode").lower(), operands


def _placeholder(row: Any) -> bool:
    return not isinstance(row, Mapping) or row.get("instruction") is None


def _base_opcode(opcode: str) -> str:
    return opcode[:-1] if opcode.endswith(".") else opcode


def _stack_operand(value: str | None) -> tuple[int, int] | None:
    if not isinstance(value, str):
        return None
    match = _MEMORY_RE.fullmatch(value)
    if match is None or match.group("base").lower() != "r1":
        return None
    offset = _parse_integer(match.group("displacement"))
    return None if offset is None else (offset, 0)


def _instruction_address(row: Any) -> int | None:
    if not isinstance(row, Mapping) or not isinstance(row.get("instruction"), Mapping):
        return None
    return _parse_integer(row["instruction"].get("address"))


def _normal_symbol(value: str) -> str:
    text = value.strip().lower()
    if text.startswith("<") and text.endswith(">"):
        text = text[1:-1].strip()
    if text.startswith("."):
        text = text[1:]
    return text


def _normal_contracts(contracts: Mapping[str, Any]) -> dict[str, _Contract]:
    if not isinstance(contracts, Mapping):
        raise ValueError("contracts must be a mapping of callee to contract")
    result: dict[str, _Contract] = {}
    for raw_name, raw_contract in contracts.items():
        if not isinstance(raw_name, str) or not raw_name.strip():
            raise ValueError("contract callee names must be nonempty text")
        if not isinstance(raw_contract, Mapping):
            raise ValueError(f"contract {raw_name!r} must be an object")
        raw_arguments = raw_contract.get("arguments")
        if not isinstance(raw_arguments, list):
            raise ValueError(f"contract {raw_name!r}.arguments must be a list")
        arguments: list[str] = []
        for argument in raw_arguments:
            register = _gpr(argument)
            if register is None:
                raise ValueError(f"contract {raw_name!r}.arguments contains invalid GPR")
            if register in arguments:
                raise ValueError(f"contract {raw_name!r}.arguments contains duplicate {register}")
            arguments.append(register)
        return_r3 = raw_contract.get("return_r3")
        if not isinstance(return_r3, bool):
            raise ValueError(f"contract {raw_name!r}.return_r3 must be boolean")
        normalized = _normal_symbol(raw_name)
        if normalized in result:
            raise ValueError(f"duplicate normalized callee contract: {raw_name!r}")
        result[normalized] = _Contract(raw_name, tuple(arguments), return_r3)
    return result


def _instruction_branch_dest(row: Any, operands: list[str]) -> int | None:
    instruction = row.get("instruction") if isinstance(row, Mapping) else None
    if isinstance(instruction, Mapping) and "branch_dest" in instruction:
        destination = _parse_integer(instruction.get("branch_dest"))
        if destination is not None:
            return destination
    if operands:
        return _parse_integer(operands[-1])
    return None


def _call_site(
    row: int,
    parsed: tuple[str, list[str]] | None,
    contracts: Mapping[str, _Contract],
) -> _CallSite | None:
    if parsed is None:
        return None
    opcode, operands = parsed
    opcode = _base_opcode(opcode)
    if opcode not in _CALL_OPCODES:
        return None
    target = operands[-1] if opcode in _DIRECT_CALL_OPCODES and operands else None
    helper = bool(re.fullmatch(r"_+(?:save|rest)(?:gpr|fpr)_?\d+", target or ""))
    callee = contracts.get(_normal_symbol(target)).name if target is not None and _normal_symbol(target) in contracts else None
    return _CallSite(row, callee, target is not None, helper)


def _branch_kind(
    row: Any,
    parsed: tuple[str, list[str]] | None,
) -> tuple[str, int | None] | None:
    if parsed is None:
        return None
    opcode, operands = parsed
    opcode = _base_opcode(opcode)
    if opcode in _CALL_OPCODES:
        return None
    if opcode in _TERMINATORS:
        return ("terminate", None)
    if opcode in _INDIRECT_BRANCHES:
        return ("indirect", None)
    if not opcode.startswith("b"):
        return None
    destination = _instruction_branch_dest(row, operands)
    if opcode in _UNCONDITIONAL_BRANCHES:
        return ("unconditional", destination)
    return ("conditional", destination)


def _initial_state(entry_arguments: tuple[str, ...] = ()) -> _State:
    state = _State(
        {f"r{index}": _Fact.unknown("entry_undefined") for index in range(32)},
        {},
        frozenset(),
    )
    for register in entry_arguments:
        state.registers[register] = _Fact.known(_Origin(-1, "entry:" + register))
    return state


def _derived(state: _State, base_register: str, operation: str) -> _Fact:
    origin = state.registers[base_register].origin
    if origin is None or origin.depth >= 4:
        return _Fact.unknown("derived_origin_unresolved")
    return _Fact.known(_Origin(origin.row, operation + "(" + origin.callee + ")", origin.depth + 1))


def _join_states(left: _State, right: _State) -> _State:
    registers = {
        register: left.registers[register].join(right.registers[register])
        for register in left.registers
    }
    memory: dict[int, _MemCell] = {}
    for offset in set(left.memory) | set(right.memory):
        left_cell = left.memory.get(offset)
        right_cell = right.memory.get(offset)
        if left_cell is None:
            assert right_cell is not None
            memory[offset] = _MemCell(
                offset,
                right_cell.width,
                right_cell.fact.join(_Fact.unknown("stack_path_join")),
            )
        elif right_cell is None:
            memory[offset] = _MemCell(
                offset,
                left_cell.width,
                left_cell.fact.join(_Fact.unknown("stack_path_join")),
            )
        else:
            memory[offset] = _MemCell(
                offset,
                max(left_cell.width, right_cell.width),
                (left_cell.fact.join(right_cell.fact) if left_cell.width == right_cell.width
                 else _Fact.unknown("stack_width_join")),
            )
    return _State(registers, memory, left.memory_unknown | right.memory_unknown)


def _overlap(left_offset: int, left_width: int, right_offset: int, right_width: int) -> bool:
    return left_offset < right_offset + right_width and right_offset < left_offset + left_width


def _store_stack(state: _State, offset: int, width: int, fact: _Fact) -> None:
    state.memory = {
        start: cell
        for start, cell in state.memory.items()
        if not _overlap(start, cell.width, offset, width)
    }
    state.memory[offset] = _MemCell(offset, width, fact)


def _invalidate_stack(state: _State, offset: int | None, width: int | None, cause: str) -> None:
    if offset is None or width is None:
        state.memory.clear()
    else:
        state.memory = {
            start: cell
            for start, cell in state.memory.items()
            if not _overlap(start, cell.width, offset, width)
        }
    state.memory_unknown = state.memory_unknown | frozenset({cause})


def _load_stack(state: _State, offset: int) -> _Fact:
    overlaps = [
        cell for cell in state.memory.values()
        if _overlap(cell.offset, cell.width, offset, 4)
    ]
    exact = [cell for cell in overlaps if cell.offset == offset]
    if len(exact) == 1 and exact[0].width == 4 and len(overlaps) == 1:
        return exact[0].fact
    if overlaps:
        return _Fact.unknown("stack_overlap_unknown")
    if state.memory_unknown:
        return _Fact.unknown(*sorted(state.memory_unknown))
    return _Fact.unknown("stack_reload_unknown")


def _set_unknown(state: _State, register: str, cause: str) -> None:
    state.registers[register] = _Fact.unknown(cause)


def _is_store(opcode: str) -> bool:
    return opcode.startswith("st") or opcode == "stmw"


def _memory_effect(state: _State, opcode: str, operands: list[str]) -> None:
    if not _is_store(opcode):
        return
    stack_operands: list[tuple[int, int]] = []
    for operand in operands:
        parsed = _stack_operand(operand)
        if parsed is not None:
            stack_operands.append((parsed[0], _STORE_WIDTHS.get(opcode, 0)))
    if len(stack_operands) == 1 and stack_operands[0][1] > 0:
        offset, width = stack_operands[0]
        _invalidate_stack(state, offset, width, "stack_store_invalidation")
    else:
        # An indexed/symbolic store may alias every tracked frame slot.
        _invalidate_stack(state, None, None, "unknown_memory_store")


def _transfer(
    state: _State,
    row: int,
    raw_row: Any,
    parsed: tuple[str, list[str]] | None,
    call: _CallSite | None,
    origin_rows: Mapping[int, _Origin],
    contracts: Mapping[str, _Contract],
) -> _State:
    result = state.clone()
    if parsed is None:
        if _placeholder(raw_row):
            return result
        for register in result.registers:
            _set_unknown(result, register, "unsupported_instruction")
        _invalidate_stack(result, None, None, "unsupported_instruction_memory")
        return result

    opcode, operands = parsed
    opcode = _base_opcode(opcode)
    if call is not None:
        if call.helper:
            return result
        for register in _VOLATILE_REGISTERS:
            _set_unknown(result, register, "call_clobber")
        result.memory.clear()
        result.memory_unknown = result.memory_unknown | frozenset({"call_memory_side_effect"})
        origin = origin_rows.get(row)
        if origin is not None:
            result.registers["r3"] = _Fact.known(origin)
        elif call.callee is not None:
            contract = contracts[_normal_symbol(call.callee)]
            _set_unknown(result, "r3", "unpaired_call_return" if contract.return_r3 else "call_return_not_declared")
        else:
            result.registers["r3"] = _Fact.unknown("call_clobber", "unknown_call_return")
        return result

    if opcode == "mr" and len(operands) == 2:
        destination, source = _gpr(operands[0]), _gpr(operands[1])
        if destination is not None and source is not None:
            result.registers[destination] = result.registers[source]
            return result
    if opcode in {"extsh", "extsb"} and len(operands) == 2:
        destination, source = _gpr(operands[0]), _gpr(operands[1])
        if destination is not None and source is not None:
            result.registers[destination] = result.registers[source]
            return result

    if opcode in {"li", "lis"} and len(operands) == 2 and _gpr(operands[0]):
        immediate = _parse_integer(operands[1])
        if immediate is not None:
            value = (immediate << 16 if opcode == "lis" else immediate) & 0xffffffff
            result.registers[operands[0]] = _Fact.known(_Origin(-1, f"constant:0x{value:x}"))
            return result
    if opcode == "addi" and len(operands) == 3 and _gpr(operands[0]) and _gpr(operands[1]):
        immediate = _parse_integer(operands[2])
        if immediate is not None and operands[1] != "r0":
            result.registers[operands[0]] = _derived(state, operands[1], f"address+{immediate}")
            return result

    if opcode in {"lwz", "lha", "lhz", "lbz"} and len(operands) == 2 and _gpr(operands[0]):
        memory = _MEMORY_RE.fullmatch(operands[1])
        if memory and memory.group("base") not in {"r0", "r1"}:
            offset = _parse_integer(memory.group("displacement"))
            result.registers[operands[0]] = _derived(state, memory.group("base"), f"{opcode}@{offset}")
            return result

    if opcode == "stw" and len(operands) == 2:
        source = _gpr(operands[0])
        stack = _stack_operand(operands[1])
        if source is not None and stack is not None:
            _store_stack(result, stack[0], 4, result.registers[source])
            return result
    if opcode == "lwz" and len(operands) == 2:
        destination = _gpr(operands[0])
        stack = _stack_operand(operands[1])
        if destination is not None:
            result.registers[destination] = (
                _load_stack(result, stack[0]) if stack is not None else _Fact.unknown("unsupported_load")
            )
            return result

    _memory_effect(result, opcode, operands)
    if opcode == "lmw" and operands and _gpr(operands[0]):
        for number in range(int(operands[0][1:]), 32):
            _set_unknown(result, f"r{number}", "unsupported_multiple_load")
    if opcode.endswith(("u", "ux")) and opcode.startswith(("l", "st")):
        for operand in operands[1:]:
            memory = _MEMORY_RE.fullmatch(operand)
            if memory:
                _set_unknown(result, memory.group("base").lower(), "address_update")
            elif _gpr(operand):
                _set_unknown(result, operand, "indexed_address_update")
    if operands:
        destination = _gpr(operands[0])
        if destination is not None and not opcode.startswith(_NO_GPR_DEST_PREFIXES):
            _set_unknown(result, destination, "unsupported_definition")
    return result


def _analyze(
    rows: list[Any],
    contracts: Mapping[str, _Contract],
    origin_rows: Mapping[int, _Origin],
    entry_arguments: tuple[str, ...] = (),
) -> _Analysis:
    parsed = [_parts(row) for row in rows]
    calls = {
        index: call
        for index, (row, parts) in enumerate(zip(rows, parsed))
        if (call := _call_site(index, parts, contracts)) is not None
    }

    addresses: dict[int, list[int]] = {}
    for index, row in enumerate(rows):
        address = _instruction_address(row)
        if address is not None:
            addresses.setdefault(address, []).append(index)

    successors: dict[int, list[int]] = {}
    cfg_unknowns: list[dict[str, Any]] = []

    def cfg_unknown(row: int, reason: str) -> None:
        if len(cfg_unknowns) < MAX_ITEMS:
            cfg_unknowns.append({"row": row, "reason": reason})

    for index, row in enumerate(rows):
        parts = parsed[index]
        if parts is None and not _placeholder(row):
            cfg_unknown(index, "malformed_instruction")
        call = calls.get(index)
        if call is not None:
            successors[index] = [index + 1] if index + 1 < len(rows) else []
            continue
        branch = _branch_kind(row, parts)
        if branch is None:
            successors[index] = [index + 1] if index + 1 < len(rows) else []
            continue
        kind, destination = branch
        if kind == "terminate":
            successors[index] = []
        elif kind == "indirect":
            cfg_unknown(index, "indirect_control_flow")
            successors[index] = []
        elif destination is None:
            cfg_unknown(index, "branch_destination_unresolved")
            successors[index] = [index + 1] if index + 1 < len(rows) else []
        else:
            destination_rows = addresses.get(destination, [])
            if len(destination_rows) != 1:
                cfg_unknown(index, "branch_destination_ambiguous")
                successors[index] = [index + 1] if index + 1 < len(rows) else []
                continue
            next_rows = [destination_rows[0]]
            if kind == "conditional" and index + 1 < len(rows):
                next_rows.append(index + 1)
            successors[index] = sorted(set(next_rows))

    before: dict[int, _State] = {}
    if rows:
        before[0] = _initial_state(entry_arguments)
    pending = [0] if rows else []
    steps = 0
    while pending:
        steps += 1
        if steps > max(1, len(rows)) * 256:
            cfg_unknown(0, "dataflow_iteration_limit")
            break
        index = pending.pop()
        after = _transfer(
            before[index], index, rows[index], parsed[index], calls.get(index), origin_rows, contracts
        )
        for successor in successors.get(index, []):
            candidate = after.clone()
            previous = before.get(successor)
            merged = candidate if previous is None else _join_states(previous, candidate)
            if previous is None or merged != previous:
                before[successor] = merged
                pending.append(successor)
    return _Analysis(before, calls, tuple(cfg_unknowns))


def _origin_json(origin: _Origin | None) -> dict[str, Any] | None:
    return None if origin is None else {"row": origin.row, "callee": origin.callee}


def _fact_reason(fact: _Fact) -> str:
    if len(fact.values) > 1:
        return "producer_path_ambiguous"
    return sorted(fact.causes)[0] if fact.causes else "producer_unknown"


def _rows_list(rows: Sequence[Mapping[str, Any]] | Any, side: str) -> list[Any]:
    if isinstance(rows, (str, bytes, bytearray)) or not isinstance(rows, Sequence):
        raise ValueError(f"{side}_rows must be a sequence of aligned row objects")
    if len(rows) > 10000:
        raise ValueError(f"{side}_rows exceeds bounded diagnostic scope")
    return list(rows)


def compare(
    target_rows: Sequence[Mapping[str, Any]],
    candidate_rows: Sequence[Mapping[str, Any]],
    contracts: Mapping[str, Any],
    *, entry_arguments: Sequence[str] = (),
) -> dict[str, Any]:
    """Compare call argument producer identities for two aligned row streams.

    ``contracts`` is intentionally explicit.  A callee is inspected only for
    the listed argument registers, and its return is tracked only when
    ``return_r3`` is true.  Unknown or non-unique producers are reported as
    uncertainty rather than as mismatches.
    """
    target = _rows_list(target_rows, "target")
    candidate = _rows_list(candidate_rows, "candidate")
    normalized_contracts = _normal_contracts(contracts)
    if (isinstance(entry_arguments, (str, bytes)) or len(set(entry_arguments)) != len(entry_arguments)
            or any(register not in {f"r{i}" for i in range(3, 11)} for register in entry_arguments)):
        raise ValueError("entry_arguments must name distinct declared GPR parameters r3-r10")
    entry_arguments = tuple(entry_arguments)

    # Pairing is the only authority for a call-return origin.  A return from
    # an unpaired call cannot be compared safely across object identities.
    preliminary_target = {
        index: call
        for index, (row, parts) in enumerate(zip(target, [_parts(item) for item in target]))
        if (call := _call_site(index, parts, normalized_contracts)) is not None
    }
    preliminary_candidate = {
        index: call
        for index, (row, parts) in enumerate(zip(candidate, [_parts(item) for item in candidate]))
        if (call := _call_site(index, parts, normalized_contracts)) is not None
    }
    pair_rows: dict[int, str] = {}
    paired_calls = 0
    for index in sorted(set(preliminary_target) | set(preliminary_candidate)):
        left = preliminary_target.get(index)
        right = preliminary_candidate.get(index)
        if left is None or right is None or left.helper or right.helper:
            continue
        if left.callee is None or left.callee != right.callee:
            continue
        contract = normalized_contracts[_normal_symbol(left.callee)]
        paired_calls += 1
        if contract.return_r3:
            pair_rows[index] = contract.name

    target_origins = {
        row: _Origin(row, callee) for row, callee in pair_rows.items()
    }
    candidate_origins = {
        row: _Origin(row, callee) for row, callee in pair_rows.items()
    }
    target_analysis = _analyze(target, normalized_contracts, target_origins, entry_arguments)
    candidate_analysis = _analyze(candidate, normalized_contracts, candidate_origins, entry_arguments)

    findings: list[dict[str, Any]] = []
    unknowns: list[dict[str, Any]] = []
    finding_count = 0
    unknown_count = 0
    checked_argument_count = 0
    equal_argument_count = 0

    def add_unknown(item: dict[str, Any], amount: int = 1) -> None:
        nonlocal unknown_count
        unknown_count += amount
        if len(unknowns) < MAX_ITEMS:
            unknowns.append(item)

    def add_finding(item: dict[str, Any]) -> None:
        nonlocal finding_count
        finding_count += 1
        if len(findings) < MAX_ITEMS:
            findings.append(item)

    for index in sorted(set(target_analysis.calls) | set(candidate_analysis.calls)):
        target_call = target_analysis.calls.get(index)
        candidate_call = candidate_analysis.calls.get(index)
        if target_call is None or candidate_call is None:
            call = target_call or candidate_call
            contract = normalized_contracts.get(_normal_symbol(call.callee)) if call and call.callee else None
            if contract is None:
                continue
            amount = max(1, len(contract.arguments))
            add_unknown({
                "row": index,
                "callee": contract.name,
                "argument": None,
                "reason": "unpaired_call",
                "side": "target" if target_call is not None else "candidate",
            }, amount)
            continue
        if target_call.helper or candidate_call.helper or target_call.callee is None:
            continue
        if target_call.callee != candidate_call.callee:
            contract = normalized_contracts.get(_normal_symbol(target_call.callee))
            if contract is None:
                contract = normalized_contracts.get(_normal_symbol(candidate_call.callee or ""))
            if contract is not None:
                add_unknown({
                    "row": index,
                    "callee": contract.name,
                    "argument": None,
                    "reason": "call_callee_unpaired",
                    "target_callee": target_call.callee,
                    "candidate_callee": candidate_call.callee,
                }, max(1, len(contract.arguments)))
            continue
        contract = normalized_contracts[_normal_symbol(target_call.callee)]
        if not contract.arguments:
            continue
        for register in contract.arguments:
            checked_argument_count += 1
            target_state = target_analysis.before.get(index)
            candidate_state = candidate_analysis.before.get(index)
            if target_analysis.cfg_unknowns or candidate_analysis.cfg_unknowns:
                add_unknown({
                    "row": index,
                    "callee": contract.name,
                    "argument": register,
                    "reason": "control_flow_unresolved",
                })
                continue
            if target_state is None or candidate_state is None:
                add_unknown({
                    "row": index,
                    "callee": contract.name,
                    "argument": register,
                    "reason": "unreachable_call",
                })
                continue
            target_fact = target_state.registers[register]
            candidate_fact = candidate_state.registers[register]
            target_origin = target_fact.origin
            candidate_origin = candidate_fact.origin
            if target_origin is None or candidate_origin is None:
                side = "both" if target_origin is None and candidate_origin is None else (
                    "target" if target_origin is None else "candidate"
                )
                add_unknown({
                    "row": index,
                    "callee": contract.name,
                    "argument": register,
                    "side": side,
                    "reason": _fact_reason(target_fact if target_origin is None else candidate_fact),
                    "target_causes": sorted(target_fact.causes)[:4],
                    "candidate_causes": sorted(candidate_fact.causes)[:4],
                })
                continue
            if target_origin == candidate_origin:
                equal_argument_count += 1
                continue
            add_finding({
                "row": index,
                "callee": contract.name,
                "argument": register,
                "classification": "different_call_return_origin" if target_origin.depth == candidate_origin.depth == 0
                    and target_origin.row >= 0 and candidate_origin.row >= 0 else "different_value_origin",
                "target_origin": _origin_json(target_origin),
                "candidate_origin": _origin_json(candidate_origin),
            })

    truncated = finding_count > len(findings) or unknown_count > len(unknowns)
    status = "mismatch" if finding_count else ("unknown" if unknown_count else ("exact" if checked_argument_count else "none"))
    counts = {
        "target_calls": len(target_analysis.calls),
        "candidate_calls": len(candidate_analysis.calls),
        "paired_calls": paired_calls,
        "checked_arguments": checked_argument_count,
        "equal_arguments": equal_argument_count,
        "mismatched_arguments": finding_count,
        "unknown_arguments": unknown_count,
    }
    return {
        "schema": SCHEMA,
        "schema_version": 1,
        "status": status,
        "authority_advanced": False,
        "diagnostic_only": True,
        "comparison_scope": "producer/entry/field/constant provenance, not runtime value equivalence or alias proof",
        "target_row_count": len(target),
        "candidate_row_count": len(candidate),
        "finding_count": finding_count,
        "unknown_count": unknown_count,
        "paired_call_count": paired_calls,
        "checked_argument_count": checked_argument_count,
        "equal_argument_count": equal_argument_count,
        "mismatch_count": finding_count,
        "counts": counts,
        "findings": findings,
        "unknowns": unknowns,
        "cfg_unknowns": list(target_analysis.cfg_unknowns[:MAX_ITEMS]) + list(candidate_analysis.cfg_unknowns[:MAX_ITEMS]),
        "truncated": truncated,
        "returned_finding_count": len(findings),
        "returned_unknown_count": len(unknowns),
    }


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", help="JSON object with target_rows, candidate_rows, and contracts")
    args = parser.parse_args(argv)
    if args.input is None:
        parser.print_help()
        return 0
    with open(args.input, encoding="utf-8") as stream:
        document = json.load(stream)
    if not isinstance(document, Mapping):
        raise ValueError("input JSON must be an object")
    result = compare(document.get("target_rows", []), document.get("candidate_rows", []), document.get("contracts", {}))
    print(json.dumps(result, sort_keys=True, separators=(",", ":")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
