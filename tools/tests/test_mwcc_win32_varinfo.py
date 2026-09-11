from __future__ import annotations

import contextlib
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
import struct
from types import SimpleNamespace
from unittest import mock

from tools import mwcc_win32_varinfo as varinfo


class MwccWin32VarInfoTests(unittest.TestCase):
    def test_frontend_snapshot_preserves_shared_object_identity(self):
        debugger, event, context, write, node = self.allocator_fixture()
        write(varinfo.NODE_NAMES, b"".join(struct.pack("<I", i + 1) for i in range(77)))
        debugger.read_string = lambda address: {1: "EASS", 2: "EOBJREF"}.get(address, "ETEMP")
        debugger.read_object_name = lambda address: "angleY"
        stmt = bytearray(0x1A)
        stmt[4] = 4
        struct.pack_into("<I", stmt, 0xA, 0x20000)
        struct.pack_into("<i", stmt, 0x16, 123)
        write(0x19000, stmt)
        expr = bytearray(0x1A)
        struct.pack_into("<II", expr, 0xE, 0x20100, 0x20100)
        write(0x20000, expr)
        obj = bytearray(0x1A)
        obj[0] = 1
        struct.pack_into("<I", obj, 0xE, 0x33333)
        write(0x20100, obj)
        snapshot = debugger.frontend_snapshot(0x19000)
        self.assertFalse(snapshot["incomplete"])
        self.assertEqual(len(snapshot["expressions"]), 2)
        self.assertEqual(snapshot["expressions"][1]["object"], "0x33333")
        self.assertEqual(snapshot["statements"][0]["line"], 123)
        for kind in (13, 14, 15):
            stmt[4] = kind
            write(0x19000, stmt)
            self.assertEqual(len(debugger.frontend_snapshot(0x19000)["expressions"]), 2)
        self.assertTrue(debugger.frontend_snapshot(0)["incomplete"])
        stmt[4] = 255
        write(0x19000, stmt)
        self.assertTrue(debugger.frontend_snapshot(0x19000)["incomplete"])
        stmt[4] = 4
        write(0x19000, stmt)
        # Unknown node layouts remain explicitly incomplete, never invented children.
        obj[0] = 2
        write(0x20100, obj)
        self.assertTrue(debugger.frontend_snapshot(0x19000)["incomplete"])
        # Bad statement links and unreadable children fail rather than truncate.
        struct.pack_into("<I", stmt, 0, 0x19000)
        write(0x19000, stmt)
        with self.assertRaisesRegex(ValueError, "cyclic"):
            debugger.frontend_snapshot(0x19000)
        struct.pack_into("<I", stmt, 0, 0)
        struct.pack_into("<I", stmt, 0xA, 0xDEAD)
        write(0x19000, stmt)
        with self.assertRaisesRegex(ValueError, "truncated"):
            debugger.frontend_snapshot(0x19000)

    def test_frontend_hooks_are_optional_and_authenticated(self):
        debugger = varinfo.Debugger(0, Path("unused.json"), "test",
                                   capture_regalloc=True, capture_frontend=True)
        self.assertTrue(set(varinfo.FRONTEND_HOOK_BYTES) <= set(debugger.observation_hooks))
        self.assertTrue(set(varinfo.TEMP_ORIGIN_HOOK_BYTES) <= set(debugger.observation_hooks))
        self.assertEqual(list(varinfo.FRONTEND_STAGES.values()), ["initial", "optimized", "final"])
        debugger = varinfo.Debugger(0, Path("unused.json"), "test", capture_regalloc=True)
        self.assertTrue(set(varinfo.FRONTEND_HOOK_BYTES).isdisjoint(debugger.observation_hooks))
        self.assertTrue(set(varinfo.TEMP_ORIGIN_HOOK_BYTES).isdisjoint(debugger.observation_hooks))
        hooks = dict(varinfo.EXPECTED_HOOK_BYTES)
        hooks.update(varinfo.REGALLOC_HOOK_BYTES)
        hooks.update(varinfo.FRONTEND_HOOK_BYTES)
        hooks.update(varinfo.TEMP_ORIGIN_HOOK_BYTES)
        varinfo.validate_hook_bytes(lambda address, size: hooks[address][:size], lambda x: x, hooks)
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(varinfo.main(["--frontend"]), 2)

    def test_temporary_origin_uses_object_and_return_stack_fields(self):
        debugger, event, context, write, name_address, return_address = self.temporary_origin_fixture()
        debugger.observe_regalloc(event, varinfo.TEMP_ORIGIN_HOOK)
        origin = debugger.result["temporary_origins"][0]
        self.assertEqual(origin["object"], hex(context.Ebx))
        self.assertEqual(origin["name"], "@1455")
        self.assertEqual(origin["name_hash"], hex(name_address - varinfo.OBJECT_NAME))
        self.assertEqual(origin["type"], "0x24000")
        self.assertEqual(origin["type_raw"], "01020304050607")
        self.assertEqual(origin["varinfo"], "0x25000")
        self.assertEqual(origin["saved_ebx"], "0xdeadbeef")
        self.assertEqual(origin["caller_return_address"], hex(return_address))
        self.assertEqual(origin["caller_preceding5"], "9090909090")
        self.assertIsNone(origin["caller_call_target"])
        self.assertEqual(origin["caller_call_shape"], "unknown")
        self.assertFalse(origin["direct_factory_call"])
        self.assertEqual(origin["prior_frontend_stage"], "pre-initial")
        self.assertEqual(len(origin["object_header"]), varinfo.TEMPORARY_OBJECT_HEADER_SIZE * 2)
        self.assertEqual(len(origin["stack"]), varinfo.TEMPORARY_STACK_SIZE * 2)

    def test_temporary_origin_rebases_direct_e8_factory_validation(self):
        base = 0x00500000
        target = base + (varinfo.TEMP_ORIGIN_FACTORY - varinfo.KNOWN_IMAGE_BASE)
        return_address = target + 0x40
        displacement = target - return_address
        call_bytes = b"\xe8" + struct.pack("<i", displacement)
        debugger, event, context, write, name_address, _ = self.temporary_origin_fixture(
            base=base, return_address=return_address, caller_bytes=call_bytes
        )
        debugger.result["frontend"] = [{"stage": "initial"}]
        debugger.observe_regalloc(event, base + (varinfo.TEMP_ORIGIN_HOOK - varinfo.KNOWN_IMAGE_BASE))
        origin = debugger.result["temporary_origins"][0]
        self.assertEqual(origin["caller_preceding5"], call_bytes.hex())
        self.assertEqual(origin["caller_call_target"], hex(target))
        self.assertEqual(origin["caller_call_shape"], "direct_e8")
        self.assertTrue(origin["direct_factory_call"])
        self.assertEqual(origin["prior_frontend_stage"], "initial")

    def test_temporary_hook_mismatch_fails_before_any_writes(self):
        debugger = varinfo.Debugger(0, Path("unused.json"), "test",
                                   capture_regalloc=True, capture_frontend=True)
        debugger.base = varinfo.KNOWN_IMAGE_BASE
        hooks = {
            **varinfo.EXPECTED_HOOK_BYTES,
            **varinfo.REGALLOC_HOOK_BYTES,
            **varinfo.FRONTEND_HOOK_BYTES,
            **varinfo.TEMP_ORIGIN_HOOK_BYTES,
        }

        def read(address, size):
            absolute = address - debugger.base + varinfo.KNOWN_IMAGE_BASE
            if absolute == varinfo.TEMP_ORIGIN_HOOK:
                return b"\0" * size
            return hooks[absolute][:size]

        writes = []
        debugger.read = read
        debugger.write = lambda address, data: writes.append((address, data))
        with self.assertRaisesRegex(RuntimeError, "hook byte validation failed"):
            debugger.validate_hooks()
        self.assertFalse(debugger.hooks_validated)
        self.assertEqual(writes, [])

    def test_temporary_hook_follows_target_frontend_lifecycle(self):
        debugger = varinfo.Debugger(0, Path("unused.json"), "target",
                                   capture_regalloc=True, capture_frontend=True)
        debugger.base = varinfo.KNOWN_IMAGE_BASE
        debugger.step_over = lambda event, address, rearm: None
        installed = []
        debugger.install_breakpoint = installed.append
        debugger.read_object_name = lambda address: "other"
        event = SimpleNamespace(dwThreadId=1)
        debugger.handle_codegen_breakpoint(event, varinfo.CODEGEN_START)
        self.assertEqual(installed, [])

        debugger.read_object_name = lambda address: "target"
        debugger.handle_codegen_breakpoint(event, varinfo.CODEGEN_START)
        self.assertEqual(set(installed), set(debugger.observation_hooks))
        retired = []
        debugger.remove_breakpoint = retired.append
        debugger.read_object_name = lambda address: "other"
        debugger.regalloc_active = True
        debugger.regalloc_pending = None
        debugger.handle_codegen_breakpoint(event, varinfo.CODEGEN_START)
        self.assertEqual(
            set(retired),
            {debugger.runtime(site) for site in debugger.observation_hooks},
        )

    def test_temporary_origin_rejects_short_reads_names_and_count_overflow(self):
        debugger, event, context, write, name_address, return_address = self.temporary_origin_fixture()
        debugger.read = lambda address, size: b""
        with self.assertRaisesRegex(ValueError, "truncated"):
            debugger.observe_regalloc(event, varinfo.TEMP_ORIGIN_HOOK)

        debugger, event, context, write, name_address, return_address = self.temporary_origin_fixture()
        write(name_address, b"X" * varinfo.TEMPORARY_NAME_LIMIT)
        with self.assertRaisesRegex(ValueError, "NUL-terminated"):
            debugger.observe_regalloc(event, varinfo.TEMP_ORIGIN_HOOK)

        debugger, event, context, write, name_address, return_address = self.temporary_origin_fixture()
        debugger.result["temporary_origins"] = [{}] * varinfo.MAX_TEMPORARY_ORIGINS
        with self.assertRaisesRegex(ValueError, "excessive temporary origin"):
            debugger.observe_regalloc(event, varinfo.TEMP_ORIGIN_HOOK)

    def range_split_fixture(self, base=varinfo.KNOWN_IMAGE_BASE):
        return_address = base + varinfo.RANGE_SPLIT_CALLER_RETURN - varinfo.KNOWN_IMAGE_BASE
        factory = base + varinfo.TEMP_ORIGIN_FACTORY - varinfo.KNOWN_IMAGE_BASE
        debugger, event, context, write, _, _ = self.temporary_origin_fixture(
            base=base, return_address=return_address,
            caller_bytes=b"\xe8" + struct.pack("<i", factory - return_address),
        )
        variable, old_object, old_name = 0x32000, 0x33000, 0x34000
        definition, second_definition, use = 0x37000, 0x37040, 0x38000
        root, unary, leaf, unknown, ast = 0x39000, 0x39040, 0x39080, 0x390C0, 0x3A000
        write(context.Esp, struct.pack("<II", variable, return_address))
        write(debugger.runtime(varinfo.RANGE_SPLIT_CALLER_PREFIX_ADDRESS),
              varinfo.RANGE_SPLIT_CALLER_PREFIX)
        header = bytearray(0x22)
        struct.pack_into("<H", header, 0, 9)
        struct.pack_into("<I", header, 2, old_object)
        struct.pack_into("<I", header, 0x12, definition)
        struct.pack_into("<I", header, 0x16, use)
        write(variable, header)
        header = bytearray(0x2E)
        struct.pack_into("<I", header, varinfo.OBJECT_NAME, old_name)
        struct.pack_into("<I", header, 0x0E, 0x24000)
        struct.pack_into("<I", header, varinfo.OBJECT_VARINFO, 0x35000)
        write(old_object, header)
        write(old_name + varinfo.OBJECT_NAME, b"ratio\0" + b"\0" * 250)
        bitset_pointers = {}
        for number, (label, absolute) in enumerate(varinfo.RANGE_SPLIT_BITSETS.items()):
            pointer = 0x36000 + number * 0x20
            bits = {"remaining_defs": [7], "remaining_uses": [],
                    "selected_defs": [3], "selected_uses": [11]}[label]
            write(pointer, struct.pack("<II", 1, sum(1 << bit for bit in bits)))
            write(debugger.runtime(absolute), struct.pack("<I", pointer))
            bitset_pointers[label] = pointer
        for address, index, expression, following in (
            (definition, 3, root, second_definition), (second_definition, 7, 0, 0)
        ):
            member = bytearray(0x18)
            struct.pack_into("<I", member, 0, index)
            struct.pack_into("<I", member, 8, expression)
            struct.pack_into("<I", member, 0xC, variable)
            struct.pack_into("<I", member, 0x14, following)
            write(address, member)
        reaching = 0x36100
        write(reaching, struct.pack("<II", 1, (1 << 3) | (1 << 7)))
        member = bytearray(0x1C)
        struct.pack_into("<I", member, 0, 11)
        struct.pack_into("<I", member, 8, leaf)
        struct.pack_into("<I", member, 0xC, variable)
        struct.pack_into("<I", member, 0x18, reaching)
        write(use, member)
        for address, kind, children in ((root, 3, [unary, unknown]),
                                         (unary, 2, [leaf]), (leaf, 1, [ast]),
                                         (unknown, 7, [])):
            node = bytearray(0x20 + 4 * len(children))
            node[0] = kind
            for index, child in enumerate(children):
                struct.pack_into("<I", node, 0x20 + index * 4, child)
            write(address, node)
        frontend = bytearray(0x1A)
        frontend[0] = 0x38
        struct.pack_into("<I", frontend, 0xE, old_object)
        write(ast, frontend)
        return debugger, event, context, write, {
            "variable": variable, "old_object": old_object, "definition": definition,
            "second_definition": second_definition, "use": use, "root": root,
            "unary": unary, "leaf": leaf, "unknown": unknown, "ast": ast,
            "bitsets": bitset_pointers,
        }

    def test_range_split_joins_original_object_and_selected_component(self):
        debugger, event, _, _, addresses = self.range_split_fixture()
        debugger.observe_regalloc(event, debugger.runtime(varinfo.TEMP_ORIGIN_HOOK))
        split = debugger.result["temporary_origins"][0]["range_split"]
        self.assertEqual(split["variable"], hex(addresses["variable"]))
        self.assertEqual(split["old_object"]["object"], hex(addresses["old_object"]))
        self.assertEqual(split["old_object"]["name"], "ratio")
        self.assertEqual(split["old_object"]["type_raw"], "01020304050607")
        self.assertEqual(split["bitsets"]["selected_defs"]["set_indices"], [3])
        self.assertEqual(split["bitsets"]["remaining_defs"]["set_indices"], [7])
        self.assertEqual([row["index"] for row in split["definitions"]], [3, 7])
        self.assertIsNone(split["definitions"][1]["expression"])
        self.assertEqual(split["uses"][0]["reaching_defs"]["set_indices"], [3, 7])
        graph = split["definitions"][0]["expression_graph"]
        self.assertTrue(graph["incomplete"])
        nodes = {row["address"]: row for row in graph["nodes"]}
        self.assertTrue(nodes[hex(addresses["unknown"])]["incomplete"])
        self.assertEqual(nodes[hex(addresses["leaf"])]["frontend_ast"]["object_name"], "ratio")

    def test_range_split_rebases_caller_and_bitsets(self):
        debugger, event, _, _, _ = self.range_split_fixture(base=0x500000)
        debugger.observe_regalloc(event, debugger.runtime(varinfo.TEMP_ORIGIN_HOOK))
        split = debugger.result["temporary_origins"][0]["range_split"]
        self.assertEqual(split["caller_prefix_address"],
                         hex(debugger.runtime(varinfo.RANGE_SPLIT_CALLER_PREFIX_ADDRESS)))
        self.assertEqual(split["bitsets"]["selected_uses"]["set_indices"], [11])

    def test_range_split_requires_exact_caller_before_variable_reads(self):
        target = varinfo.TEMP_ORIGIN_FACTORY
        return_address = target + 0x40
        debugger, event, _, _, _, _ = self.temporary_origin_fixture(
            return_address=return_address,
            caller_bytes=b"\xe8" + struct.pack("<i", target - return_address),
        )
        debugger.observe_regalloc(event, debugger.runtime(varinfo.TEMP_ORIGIN_HOOK))
        self.assertNotIn("range_split", debugger.result["temporary_origins"][0])
        debugger, event, _, write, addresses = self.range_split_fixture()
        write(debugger.runtime(varinfo.RANGE_SPLIT_CALLER_PREFIX_ADDRESS), b"\0")
        reads = []
        original_read = debugger.read
        def read(address, size):
            reads.append(address)
            return original_read(address, size)
        debugger.read = read
        with self.assertRaises(ValueError):
            debugger.observe_regalloc(event, debugger.runtime(varinfo.TEMP_ORIGIN_HOOK))
        self.assertNotIn(addresses["variable"], reads)

    def test_range_split_rejects_bad_component_records(self):
        for problem in ("cycle", "parent", "duplicate", "bitset", "truncated"):
            with self.subTest(problem=problem):
                debugger, event, _, write, addresses = self.range_split_fixture()
                if problem == "cycle":
                    write(addresses["definition"] + 0x14,
                          struct.pack("<I", addresses["definition"]))
                elif problem == "parent":
                    write(addresses["definition"] + 0xC, struct.pack("<I", 1))
                elif problem == "duplicate":
                    write(addresses["second_definition"], struct.pack("<I", 3))
                elif problem == "bitset":
                    write(addresses["bitsets"]["selected_defs"],
                          struct.pack("<I", varinfo.RANGE_SPLIT_BITSET_WORD_LIMIT + 1))
                else:
                    original_read = debugger.read
                    debugger.read = lambda address, size: (
                        b"" if address == addresses["variable"] else original_read(address, size)
                    )
                with self.assertRaises(ValueError):
                    debugger.observe_regalloc(event, debugger.runtime(varinfo.TEMP_ORIGIN_HOOK))

    def test_range_split_expression_bounds_count_unfinished_ancestors(self):
        debugger, _, _, _, addresses = self.range_split_fixture()
        with mock.patch.object(varinfo, "RANGE_SPLIT_EXPRESSION_LIMIT", 2):
            with self.assertRaisesRegex(ValueError, "excessive"):
                debugger.range_split_expression_graph(addresses["root"])

    def test_range_split_frontend_leaf_incompleteness_reaches_graph(self):
        for rawkind in (0, 0x37, 0x39, 0xFF):
            with self.subTest(rawkind=rawkind):
                debugger, _, _, write, addresses = self.range_split_fixture()
                write(addresses["ast"], bytes([rawkind]))
                graph = debugger.range_split_expression_graph(addresses["leaf"])
                self.assertTrue(graph["incomplete"])
                node = graph["nodes"][0]
                self.assertTrue(node["incomplete"])
                self.assertTrue(node["frontend_ast"]["incomplete"])
                self.assertEqual(node["frontend_ast"]["rawkind"], rawkind)
                self.assertEqual(bytes.fromhex(node["frontend_ast"]["raw"])[0], rawkind)
        debugger, _, _, write, addresses = self.range_split_fixture()
        self.assertFalse(debugger.range_split_expression_graph(addresses["leaf"])["incomplete"])
        write(addresses["ast"] + 0xE, struct.pack("<I", 0))
        self.assertTrue(debugger.range_split_expression_graph(addresses["leaf"])["incomplete"])

    def test_range_split_expression_cycle_and_shared_leaf(self):
        debugger, _, _, write, addresses = self.range_split_fixture()
        write(addresses["root"] + 0x24, struct.pack("<I", addresses["leaf"]))
        graph = debugger.range_split_expression_graph(addresses["root"])
        self.assertFalse(graph["incomplete"])
        self.assertEqual(len(graph["nodes"]), 3)
        write(addresses["unary"] + 0x20, struct.pack("<I", addresses["root"]))
        with self.assertRaisesRegex(ValueError, "cyclic"):
            debugger.range_split_expression_graph(addresses["root"])

    def allocator_fixture(self, *, regalloc_class=3, graph_count=64,
                          register_limit=32):
        memory = {}

        def write(address, data):
            memory.update({address + i: b for i, b in enumerate(data)})

        def read(address, size):
            if any(address + i not in memory for i in range(size)):
                return b""
            return bytes(memory[address + i] for i in range(size))

        def node(index, color, neighbors=()):
            address = 0x10000 + index * 0x100
            raw = bytearray(0x1A + 2 * len(neighbors))
            struct.pack_into("<hhhh", raw, 0x10, index, len(neighbors), color, 2)
            struct.pack_into("<h", raw, 0x18, len(neighbors))
            for i, adjacent in enumerate(neighbors):
                struct.pack_into("<h", raw, 0x1A + i * 2, adjacent)
            write(address, raw)
            write(0x1000 + index * 4, struct.pack("<I", address))
            return address

        write(varinfo.IG_TABLE, struct.pack("<I", 0x1000))
        write(varinfo.IG_COUNT_BASE + 4 * regalloc_class,
              struct.pack("<I", graph_count))
        write(varinfo.REGISTER_LIMIT_BASE + 4 * regalloc_class,
              struct.pack("<I", register_limit))
        write(varinfo.COLORING_CLASS, bytes([regalloc_class]))
        node(62, 0)
        current = node(36, -1, [62])
        write(0x8000, struct.pack("<II", 3, current))
        debugger = varinfo.Debugger(
            0, Path("unused.json"), "test", capture_regalloc=True,
            regalloc_class=regalloc_class,
        )
        debugger.base = varinfo.KNOWN_IMAGE_BASE
        debugger.read = read
        debugger.regalloc_active = True
        debugger.threads = {1: 1}
        context = SimpleNamespace(Ebx=current, Esp=0x8000, Ecx=1, Edx=2, Eax=1)
        debugger.get_context = lambda thread: context
        return debugger, SimpleNamespace(dwThreadId=1), context, write, node

    def cse_fixture(
        self, *, opcode=0x89, destination=40, mode=1, lower=39, upper=45,
        eligibility=1, operand_count=2, operand_class=4, output_class=4,
    ):
        memory = {}

        def write(address, data):
            memory.update({address + i: byte for i, byte in enumerate(data)})

        def read(address, size):
            if any(address + i not in memory for i in range(size)):
                return b""
            return bytes(memory[address + i] for i in range(size))

        pcode = 0x9000
        header = bytearray(varinfo.CSE_PCODE_HEADER_SIZE)
        struct.pack_into("<hh", header, 0x20, opcode, operand_count)
        operands = bytearray(2 * varinfo.CSE_PCODE_OPERAND_SIZE)
        struct.pack_into("<BBHh", operands, 0, 0, operand_class, 0, destination)
        struct.pack_into("<BBHh", operands, varinfo.CSE_PCODE_OPERAND_SIZE,
                         0, 4, 0, 0)
        raw = bytes(header + operands)
        write(pcode, raw)
        stack = 0x8000
        write(stack, struct.pack("<I", pcode))
        write(stack + 0xC, struct.pack("<I", pcode))
        output = bytearray(varinfo.CSE_PCODE_OPERAND_SIZE)
        struct.pack_into("<BBHh", output, 0, 0, output_class, 0, 40)
        write(stack + 0x14, output)
        write(varinfo.CSE_MODE, struct.pack("<i", mode))
        write(varinfo.CSE_LOWER_BOUND, struct.pack("<h", lower))
        write(varinfo.CSE_UPPER_BOUND, struct.pack("<i", upper))
        debugger = varinfo.Debugger(
            0, Path("unused.json"), "test", capture_regalloc=True,
            regalloc_class=4, capture_cse=True,
        )
        debugger.base = varinfo.KNOWN_IMAGE_BASE
        debugger.read = read
        debugger.regalloc_active = True
        debugger.threads = {1: 1}
        context = SimpleNamespace(Ebx=0, Esp=stack, Ecx=0, Edx=0,
                                  Eax=eligibility, Eip=0)
        debugger.get_context = lambda thread: context
        return debugger, SimpleNamespace(dwThreadId=1), context, write, {
            "pcode": pcode, "raw": raw, "output": bytes(output),
        }

    def temporary_origin_fixture(
        self, *, base=varinfo.KNOWN_IMAGE_BASE, return_address=0x27000,
        caller_bytes=b"\x90" * 5
    ):
        memory = {}

        def write(address, data):
            memory.update({address + i: b for i, b in enumerate(data)})

        def read(address, size):
            if any(address + i not in memory for i in range(size)):
                return b""
            return bytes(memory[address + i] for i in range(size))

        object_address = 0x22000
        name_hash = 0x23000
        name_address = name_hash + varinfo.OBJECT_NAME
        type_address = 0x24000
        varinfo_address = 0x25000
        stack_address = 0x26000
        header = bytearray(varinfo.TEMPORARY_OBJECT_HEADER_SIZE)
        struct.pack_into("<I", header, varinfo.OBJECT_NAME, name_hash)
        struct.pack_into("<I", header, 0x0E, type_address)
        struct.pack_into("<I", header, varinfo.OBJECT_VARINFO, varinfo_address)
        write(object_address, header)
        name = b"@1455\0" + b"\0" * (varinfo.TEMPORARY_NAME_LIMIT - 6)
        write(name_address, name)
        write(type_address, b"\x01\x02\x03\x04\x05\x06\x07")
        write(stack_address, struct.pack("<II", 0xDEADBEEF, return_address))
        self.assertEqual(len(caller_bytes), 5)
        write(return_address - 5, caller_bytes)

        debugger = varinfo.Debugger(
            0, Path("unused.json"), "test", capture_regalloc=True,
            capture_frontend=True,
        )
        debugger.base = base
        debugger.read = read
        debugger.regalloc_active = True
        debugger.threads = {1: 1}
        context = SimpleNamespace(Ebx=object_address, Esp=stack_address, Eax=0,
                                  Ebx_hi=0, Ecx=0, Edx=0, Eip=0)
        debugger.get_context = lambda thread: context
        return (
            debugger,
            SimpleNamespace(dwThreadId=1),
            context,
            write,
            name_address,
            return_address,
        )

    def test_selector_replay_keeps_uncolored_and_alias_neighbors(self):
        self.assertEqual(varinfo.remaining_color_mask(7, [0, -1, 36], 32), 6)
        varinfo.validate_color_selection(7, [0, -1, 36], 32, 6, 1)
        with self.assertRaisesRegex(ValueError, "mask mismatch"):
            varinfo.validate_color_selection(7, [0], 32, 7, 0)
        with self.assertRaisesRegex(ValueError, "first available"):
            varinfo.validate_color_selection(7, [0], 32, 6, 2)

    def test_regalloc_class_defaults_to_fpr_and_rejects_unsupported_ids(self):
        debugger = varinfo.Debugger(0, Path("unused.json"), "test")
        self.assertEqual(debugger.regalloc_class, 3)
        self.assertEqual(debugger.regalloc_class_label, "FPR")
        self.assertEqual(varinfo.FPR_COUNT, varinfo.IG_COUNT_BASE + 4 * 3)
        self.assertEqual(varinfo.FPR_LIMIT, varinfo.REGISTER_LIMIT_BASE + 4 * 3)
        self.assertEqual(varinfo.GPR_COUNT, varinfo.IG_COUNT_BASE + 4 * 4)
        self.assertEqual(varinfo.GPR_LIMIT, varinfo.REGISTER_LIMIT_BASE + 4 * 4)
        for unsupported in (0, 1, 2, 5):
            with self.assertRaisesRegex(ValueError, "regalloc_class"):
                varinfo.Debugger(0, Path("unused.json"), "test",
                                  regalloc_class=unsupported)

    def test_regalloc_class_filter_and_class_specific_count_limit(self):
        debugger, event, _, write, _ = self.allocator_fixture(
            regalloc_class=4, graph_count=68, register_limit=23
        )
        write(varinfo.COLORING_CLASS, b"\x03")
        debugger.observe_regalloc(event, 0x508890)
        self.assertEqual(debugger.regalloc_pass, 0)
        self.assertNotIn("regalloc_passes", debugger.result)

        debugger, event, _, _, _ = self.allocator_fixture(
            regalloc_class=4, graph_count=68, register_limit=23
        )
        debugger.observe_regalloc(event, 0x508890)
        debugger.observe_regalloc(event, 0x5088C6)
        row = debugger.regalloc_pending
        self.assertEqual(row["class"], 4)
        self.assertEqual(row["node"]["graph_count"], 68)
        self.assertEqual(row["register_limit"], 23)

    def test_selector_trace_reports_actual_class_before_filter(self):
        debugger, event, _, write, _ = self.allocator_fixture(regalloc_class=4)
        write(varinfo.COLORING_CLASS, b"\x03")
        debugger.trace = True
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            debugger.observe_regalloc(event, 0x508890)
        self.assertEqual(debugger.regalloc_pass, 0)
        self.assertIn(
            "REGALLOC_SELECTOR target=test COLORING_CLASS=3 requested_class=4",
            stderr.getvalue(),
        )

    def test_gpr_pcode_join_uses_selected_class_and_strict_validation(self):
        debugger, event, context, write, node = self.allocator_fixture(
            regalloc_class=4, graph_count=68
        )
        node(36, 1, [62])
        header = bytearray(0x30)
        struct.pack_into("<hh", header, 0x20, 167, 1)
        struct.pack_into("<BBHh", header, 0x24, 0, 4, 8194, 36)
        write(0x9000, header)
        context.Esi, context.Edx, context.Ecx = 0x9000, 0x9024, context.Ebx
        debugger.observe_regalloc(event, 0x5087A4)
        row = debugger.result["regalloc_pcode"][0]
        self.assertEqual((row["class"], row["opcode"], row["operand_index"], row["color"]),
                         (4, 167, 36, 1))
        context.Eax = 2
        with self.assertRaisesRegex(ValueError, "join mismatch"):
            debugger.observe_regalloc(event, 0x5087A4)

    def test_gpr_error_labels_name_selected_class(self):
        debugger, _, context, write, _ = self.allocator_fixture(regalloc_class=4)
        write(varinfo.GPR_COUNT, struct.pack("<I", 0))
        with self.assertRaisesRegex(ValueError, "invalid GPR graph table or count"):
            debugger.ig_node(context.Ebx)
        debugger, event, _, write, _ = self.allocator_fixture(
            regalloc_class=4, register_limit=0
        )
        with self.assertRaisesRegex(ValueError, "invalid GPR register limit"):
            debugger.observe_regalloc(event, 0x5088C6)

    def test_regalloc_class_cli_requires_regalloc(self):
        self.assertEqual(varinfo.parse_args([]).regalloc_class, "fpr")
        self.assertEqual(
            varinfo.parse_args(["--regalloc", "--regalloc-class", "gpr"]).regalloc_class,
            "gpr",
        )
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            result = varinfo.main(["--regalloc-class", "gpr"])
        self.assertEqual(result, 2)
        self.assertIn("--regalloc-class gpr requires --regalloc", stderr.getvalue())

    def test_cse_is_opt_in_and_requires_gpr_regalloc(self):
        default = varinfo.Debugger(0, Path("unused.json"), "test",
                                   capture_regalloc=True)
        self.assertFalse(default.capture_cse)
        self.assertNotIn("capture_cse", default.result)
        self.assertTrue(set(varinfo.REGALLOC_HOOK_BYTES) <= set(default.observation_hooks))
        self.assertTrue(set(varinfo.CSE_HOOK_BYTES).isdisjoint(default.observation_hooks))

        cse = varinfo.Debugger(0, Path("unused.json"), "test",
                                capture_regalloc=True, regalloc_class=4,
                                capture_cse=True)
        self.assertTrue(set(varinfo.CSE_HOOK_BYTES) <= set(cse.observation_hooks))
        self.assertEqual(cse.result["capture_cse"], True)
        self.assertEqual(cse.result["cse_li_eligibility"], [])
        self.assertEqual(cse.result["cse_li_reuse"], [])

        with self.assertRaisesRegex(ValueError, "capture_regalloc"):
            varinfo.Debugger(0, Path("unused.json"), "test", capture_cse=True)
        with self.assertRaisesRegex(ValueError, "GPR"):
            varinfo.Debugger(0, Path("unused.json"), "test",
                             capture_regalloc=True, capture_cse=True)

    def test_frontend_requires_regalloc_for_imported_api_and_cli(self):
        with self.assertRaisesRegex(ValueError, "capture_frontend requires capture_regalloc"):
            varinfo.Debugger(0, Path("unused.json"), "test", capture_frontend=True)
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            result = varinfo.main(["--frontend"])
        self.assertEqual(result, 2)
        self.assertIn("--frontend requires --regalloc", stderr.getvalue())
        default = varinfo.Debugger(0, Path("unused.json"), "test")
        self.assertNotIn("temporary_origins", default.result)
        self.assertTrue(set(varinfo.FRONTEND_HOOK_BYTES).isdisjoint(default.observation_hooks))
        self.assertTrue(set(varinfo.TEMP_ORIGIN_HOOK_BYTES).isdisjoint(default.observation_hooks))

    def test_target_name_rejects_blank_api_and_cli_before_launch(self):
        for target in ("", " ", "\t\r\n"):
            with self.subTest(target=target):
                with self.assertRaisesRegex(ValueError, "nonempty function name"):
                    varinfo.Debugger(0, Path("unused.json"), target)
                stderr = io.StringIO()
                with mock.patch.object(varinfo, "validate_compiler_fingerprint") as fingerprint:
                    with contextlib.redirect_stderr(stderr):
                        result = varinfo.main(["--target", target])
                fingerprint.assert_not_called()
                self.assertEqual(result, 2)
                self.assertIn("--target must be a nonempty function name", stderr.getvalue())
        for target in ("mbDiceInit", "__ct__7ExampleFv", "fn_1_ABC"):
            debugger = varinfo.Debugger(0, Path("unused.json"), target)
            self.assertEqual(debugger.target_name, target)
            self.assertEqual(varinfo.parse_args(["--target", target]).target, target)

    def test_cse_cli_rejects_missing_regalloc_or_gpr(self):
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            result = varinfo.main(["--cse"])
        self.assertEqual(result, 2)
        self.assertIn("--cse requires --regalloc", stderr.getvalue())

        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            result = varinfo.main(["--regalloc", "--cse"])
        self.assertEqual(result, 2)
        self.assertIn("--cse requires --regalloc-class gpr", stderr.getvalue())

        parsed = varinfo.parse_args(["--regalloc", "--regalloc-class", "gpr", "--cse"])
        self.assertTrue(parsed.cse)

    def test_cse_eligibility_captures_pcode_globals_and_range(self):
        debugger, event, context, write, addresses = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1,
        )
        reads = []
        original_read = debugger.read

        def read(address, size):
            reads.append((address, size))
            return original_read(address, size)

        debugger.read = read
        debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)
        row = debugger.result["cse_li_eligibility"][0]
        self.assertEqual(row["pcode"], hex(addresses["pcode"]))
        self.assertEqual(row["raw"], addresses["raw"].hex())
        self.assertEqual(
            (row["opcode"], row["destination_vreg"], row["mode"],
             row["lower_bound"], row["upper_bound"], row["result_eax"]),
            (0x89, 40, 1, 39, 45, 1),
        )
        self.assertTrue(row["in_generated_interval"])
        self.assertIsNotNone(debugger.cse_pending_eligibility)
        self.assertIn((debugger.runtime(varinfo.CSE_MODE), 4), reads)
        self.assertIn((debugger.runtime(varinfo.CSE_LOWER_BOUND), 2), reads)
        self.assertIn((debugger.runtime(varinfo.CSE_UPPER_BOUND), 4), reads)
        self.assertNotIn((debugger.runtime(varinfo.COLORING_CLASS), 1), reads)
        self.assertEqual(context.Eax, 1)

    def test_cse_lookup_uses_stack_offsets_and_only_reads_output_on_hit(self):
        debugger, event, context, write, addresses = self.cse_fixture(
            opcode=0x8A, destination=45, mode=1, lower=39, upper=45,
            eligibility=1,
        )
        debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)
        context.Eax = 1
        reads = []
        original_read = debugger.read

        def read(address, size):
            reads.append((address, size))
            return original_read(address, size)

        debugger.read = read
        debugger.observe_regalloc(event, varinfo.CSE_LI_REUSE_HOOK)
        row = debugger.result["cse_li_reuse"][0]
        self.assertEqual(
            (row["pcode"], row["raw"], row["opcode"],
             row["destination_vreg"], row["found_eax"],
             row["eligibility_index"]),
            (hex(addresses["pcode"]), addresses["raw"].hex(), 0x8A, 45, 1, 0),
        )
        self.assertEqual(row["output_operand"]["raw"], addresses["output"].hex())
        self.assertEqual(
            (row["output_operand"]["kind"], row["output_operand"]["class"],
             row["output_operand"]["source_vreg"]),
            (0, 4, 40),
        )
        self.assertIn((context.Esp + 0xC, 4), reads)
        self.assertIn((context.Esp + 0x14, 12), reads)
        self.assertIsNone(debugger.cse_pending_eligibility)

        debugger, event, context, write, addresses = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1,
        )
        debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)
        context.Eax = 0
        write(context.Esp + 0xC, struct.pack("<I", addresses["pcode"]))
        reads = []
        original_read = debugger.read

        def read_failed(address, size):
            reads.append((address, size))
            return original_read(address, size)

        debugger.read = read_failed
        debugger.observe_regalloc(event, varinfo.CSE_LI_REUSE_HOOK)
        failed = debugger.result["cse_li_reuse"][0]
        self.assertEqual(failed["found_eax"], 0)
        self.assertNotIn("output_operand", failed)
        self.assertNotIn((context.Esp + 0x14, 12), reads)

    def test_cse_rejects_impossible_success_and_bad_output(self):
        debugger, event, context, write, _ = self.cse_fixture(
            opcode=0x89, destination=38, mode=1, lower=39, upper=45,
            eligibility=1,
        )
        with self.assertRaisesRegex(ValueError, "outside generated interval"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)

        debugger, event, context, write, _ = self.cse_fixture(
            opcode=0x89, destination=40, mode=0, lower=39, upper=45,
            eligibility=1,
        )
        with self.assertRaisesRegex(ValueError, "LI/LIS.*zero CSE mode"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)

        debugger, event, context, write, _ = self.cse_fixture(
            opcode=0x8A, destination=40, mode=0, lower=39, upper=45,
            eligibility=1,
        )
        with self.assertRaisesRegex(ValueError, "LI/LIS.*zero CSE mode"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)

        debugger, event, context, write, addresses = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1, output_class=3,
        )
        debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)
        context.Eax = 1
        with self.assertRaisesRegex(ValueError, "output operand kind/class"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_REUSE_HOOK)

    def test_cse_rejects_invalid_shape_truncation_and_stale_join(self):
        debugger, event, context, write, _ = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1, operand_count=1,
        )
        with self.assertRaisesRegex(ValueError, "exactly two operands"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)

        debugger, event, context, write, _ = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1, operand_class=3,
        )
        with self.assertRaisesRegex(ValueError, "destination operand kind/class"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)

        debugger, event, context, write, _ = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1,
        )
        debugger.read = lambda address, size: b""
        with self.assertRaisesRegex(ValueError, "truncated CSE read"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)

        debugger, event, context, write, addresses = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1,
        )
        debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)
        other = addresses["pcode"] + 0x100
        write(other, addresses["raw"])
        write(context.Esp + 0xC, struct.pack("<I", other))
        context.Eax = 1
        with self.assertRaisesRegex(ValueError, "matching successful eligibility"):
            debugger.observe_regalloc(event, varinfo.CSE_LI_REUSE_HOOK)

    def test_cse_non_li_gate_clears_pending_and_lifecycle_targets_hooks(self):
        debugger, event, context, write, addresses = self.cse_fixture(
            opcode=0x89, destination=40, mode=1, lower=39, upper=45,
            eligibility=1,
        )
        debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)
        non_li = addresses["pcode"] + 0x100
        raw = bytearray(addresses["raw"])
        struct.pack_into("<h", raw, 0x20, 0x90)
        write(non_li, raw)
        write(context.Esp, struct.pack("<I", non_li))
        debugger.observe_regalloc(event, varinfo.CSE_LI_ELIGIBILITY_HOOK)
        self.assertIsNone(debugger.cse_pending_eligibility)

        debugger = varinfo.Debugger(0, Path("unused.json"), "target",
                                    capture_regalloc=True, regalloc_class=4,
                                    capture_cse=True)
        debugger.base = varinfo.KNOWN_IMAGE_BASE
        debugger.step_over = lambda event, address, rearm: None
        installed = []
        debugger.install_breakpoint = installed.append
        debugger.read_object_name = lambda address: "other"
        debugger.handle_codegen_breakpoint(event, varinfo.CODEGEN_START)
        self.assertEqual(installed, [])
        debugger.read_object_name = lambda address: "target"
        debugger.handle_codegen_breakpoint(event, varinfo.CODEGEN_START)
        self.assertTrue(set(varinfo.CSE_HOOK_BYTES) <= set(installed))
        retired = []
        debugger.remove_breakpoint = retired.append
        debugger.read_object_name = lambda address: "other"
        debugger.regalloc_active = True
        debugger.regalloc_pending = None
        debugger.handle_codegen_breakpoint(event, varinfo.CODEGEN_START)
        self.assertTrue(
            {debugger.runtime(site) for site in varinfo.CSE_HOOK_BYTES} <= set(retired)
        )

    def test_optional_hooks_are_authenticated_too(self):
        hooks = {**varinfo.EXPECTED_HOOK_BYTES, **varinfo.REGALLOC_HOOK_BYTES}
        varinfo.validate_hook_bytes(lambda a, n: hooks[a], lambda a: a, hooks)
        with self.assertRaisesRegex(RuntimeError, "0050892e"):
            varinfo.validate_hook_bytes(
                lambda a, n: b"\0" * n if a == 0x50892E else hooks[a], lambda a: a, hooks
            )

        hooks.update(varinfo.CSE_HOOK_BYTES)
        varinfo.validate_hook_bytes(lambda a, n: hooks[a], lambda a: a, hooks)
        with self.assertRaisesRegex(RuntimeError, "00509356"):
            varinfo.validate_hook_bytes(
                lambda a, n: b"\0" * n if a == varinfo.CSE_LI_ELIGIBILITY_HOOK else hooks[a],
                lambda a: a,
                hooks,
            )

    def test_native_selection_requires_matching_mask_and_verified_store(self):
        debugger, event, context, write, node = self.allocator_fixture()
        debugger.observe_regalloc(event, 0x508890)
        debugger.observe_regalloc(event, 0x5088C6)
        row = debugger.regalloc_pending
        self.assertEqual(row["initial_mask"], 3)  # EDX is deliberately different.
        self.assertEqual(row["node"]["neighbors"][0]["vreg"], 62)
        debugger.observe_regalloc(event, 0x50892E)
        with self.assertRaisesRegex(ValueError, "stored color mismatch"):
            debugger.observe_regalloc(event, 0x508932)
        write(context.Ebx + 0x14, struct.pack("<h", 1))
        debugger.observe_regalloc(event, 0x508932)
        debugger.observe_regalloc(event, 0x508994)
        self.assertIsNone(debugger.regalloc_pending)
        self.assertEqual(row["status"], "observed")

    def test_node_table_and_neighbor_reads_are_complete(self):
        debugger, event, context, write, node = self.allocator_fixture()
        write(0x1000 + 36 * 4, struct.pack("<I", 0x1234))
        with self.assertRaisesRegex(ValueError, "identity mismatch"):
            debugger.ig_node(context.Ebx, neighbors=True)
        node(36, -1, [63])  # No memory for neighbor 63.
        with self.assertRaisesRegex(ValueError, "truncated allocator read"):
            debugger.ig_node(context.Ebx, neighbors=True)
        node(36, -1, [64])
        with self.assertRaisesRegex(ValueError, "outside graph"):
            debugger.ig_node(context.Ebx, neighbors=True)

    def test_pcode_join_uses_same_node_identity(self):
        debugger, event, context, write, node = self.allocator_fixture()
        node(36, 1, [62])
        header = bytearray(0x30)
        struct.pack_into("<hh", header, 0x20, 167, 1)
        struct.pack_into("<BBHh", header, 0x24, 0, 3, 8194, 36)
        write(0x9000, header)
        context.Esi, context.Edx, context.Ecx = 0x9000, 0x9024, context.Ebx
        debugger.observe_regalloc(event, 0x5087A4)
        row = debugger.result["regalloc_pcode"][0]
        self.assertEqual((row["opcode"], row["operand_index"], row["color"]), (167, 36, 1))
        context.Eax = 2
        with self.assertRaisesRegex(ValueError, "join mismatch"):
            debugger.observe_regalloc(event, 0x5087A4)

    def test_spill_and_new_register_paths_are_distinct(self):
        for site, post, outcome in [(0x508954, 0x508958, "new_register"), (0x508987, None, "spill")]:
            debugger, event, context, write, node = self.allocator_fixture()
            write(0x8000, struct.pack("<I", 1))
            debugger.observe_regalloc(event, 0x5088C6)
            context.Eax = 31
            debugger.observe_regalloc(event, site)
            if post:
                write(context.Ebx + 0x14, struct.pack("<h", 31))
                debugger.observe_regalloc(event, post)
            else:
                write(context.Ebx + 0x16, struct.pack("<H", 3))
            debugger.observe_regalloc(event, 0x508994)
            self.assertEqual(debugger.result["regalloc_selections"][0]["outcome"], outcome)

    def test_target_boundary_retires_optional_hooks_without_o0_dump(self):
        debugger, event, context, write, node = self.allocator_fixture()
        debugger.read_object_name = lambda obj: "next_function"
        retired, stepped = [], []
        debugger.remove_breakpoint = retired.append
        debugger.step_over = lambda event, address, rearm: stepped.append(rearm)
        self.assertFalse(debugger.dumped)
        debugger.handle_codegen_breakpoint(event, varinfo.CODEGEN_START)
        self.assertEqual(set(retired), set(varinfo.REGALLOC_HOOK_BYTES))
        self.assertEqual(stepped, [False])
        self.assertFalse(debugger.regalloc_active)

    def test_default_command_and_arguments_are_stable(self) -> None:
        parsed = varinfo.parse_args([])
        self.assertEqual(Path(parsed.compiler), varinfo.DEFAULT_COMPILER)
        self.assertEqual(Path(parsed.output), varinfo.DEFAULT_OUTPUT)
        self.assertEqual(parsed.timeout, varinfo.DEFAULT_TIMEOUT_SECONDS)
        self.assertFalse(hasattr(parsed, "compiler_sha256"))

        command = varinfo.default_command(varinfo.REPO_ROOT, varinfo.DEFAULT_OUTPUT.parent)
        self.assertIn("src/board/telop.c", command)
        self.assertEqual(command[-1], str(varinfo.DEFAULT_OUTPUT.parent))

        explicit = varinfo.parse_args(["--target", "sample", "--", "-O0,p", "-c", "sample.c"])
        self.assertEqual(explicit.target, "sample")
        self.assertEqual(explicit.compiler_args, ["--", "-O0,p", "-c", "sample.c"])

    def test_atomic_output_is_schema_valid_and_leaves_no_temp(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "nested" / "report.json"
            payload = {
                "schema_version": 1,
                "tool": "mwcc_win32_varinfo",
                "target": "sample",
                "capture_assignments": False,
                "known_image_base": varinfo.KNOWN_IMAGE_BASE,
                "breakpoints": {},
            }
            varinfo.atomic_write_json(output, payload)
            loaded = json.loads(output.read_text(encoding="utf-8"))
            varinfo.validate_result_schema(loaded)
            self.assertEqual(loaded, payload)
            self.assertEqual(list(output.parent.glob("*.tmp")), [])

    def test_hook_bytes_must_match_before_writes(self) -> None:
        memory = {
            absolute + 0x1000: expected
            for absolute, expected in varinfo.EXPECTED_HOOK_BYTES.items()
        }

        def read(address: int, size: int) -> bytes:
            return memory[address][:size]

        varinfo.validate_hook_bytes(read, lambda absolute: absolute + 0x1000)

        with self.assertRaisesRegex(RuntimeError, "hook byte validation failed"):
            varinfo.validate_hook_bytes(
                lambda address, size: b"\0" * size,
                lambda absolute: absolute,
            )

    def test_compiler_name_and_fingerprint_are_checked_without_launching(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            compiler = Path(directory) / "mwcceppc.exe"
            compiler.write_bytes(b"test compiler")
            expected = hashlib.sha256(compiler.read_bytes()).hexdigest()
            self.assertEqual(varinfo.validate_compiler_fingerprint(compiler, expected), expected)
            with self.assertRaisesRegex(ValueError, "SHA-256 mismatch"):
                varinfo.validate_compiler_fingerprint(compiler, "0" * 64)
            with self.assertRaisesRegex(ValueError, "must be named"):
                varinfo.validate_compiler_path(compiler.with_name("wrapper.exe"))

    def test_non_windows_main_exits_before_compiler_launch(self) -> None:
        stderr = io.StringIO()
        with mock.patch.object(varinfo.os, "name", "posix"), mock.patch.object(
            varinfo, "kernel32"
        ) as kernel32, contextlib.redirect_stderr(stderr):
            result = varinfo.main([])
        self.assertEqual(result, 2)
        self.assertIn("requires Windows", stderr.getvalue())
        kernel32.CreateProcessW.assert_not_called()

    def test_missing_compiler_is_clear_and_does_not_launch(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            missing = Path(directory) / "mwcceppc.exe"
            cwd = Path(directory)
            stderr = io.StringIO()
            with mock.patch.object(varinfo.os, "name", "nt"), mock.patch.object(
                varinfo, "kernel32"
            ) as kernel32, contextlib.redirect_stderr(stderr):
                result = varinfo.main(
                    ["--compiler", str(missing), "--cwd", str(cwd)]
                )
            self.assertEqual(result, 2)
            self.assertIn("compiler not found", stderr.getvalue())
            kernel32.CreateProcessW.assert_not_called()

    def test_invalid_timeout_is_rejected_before_launch(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            compiler = Path(directory) / "mwcceppc.exe"
            compiler.write_bytes(b"test compiler")
            cwd = Path(directory)
            stderr = io.StringIO()
            with mock.patch.object(varinfo.os, "name", "nt"), mock.patch.object(
                varinfo, "kernel32"
            ) as kernel32, contextlib.redirect_stderr(stderr):
                result = varinfo.main(
                    [
                        "--compiler",
                        str(compiler),
                        "--cwd",
                        str(cwd),
                        "--timeout",
                        "0",
                    ]
                )
            self.assertEqual(result, 2)
            self.assertIn("timeout must be between", stderr.getvalue())
            kernel32.CreateProcessW.assert_not_called()


if __name__ == "__main__":
    unittest.main()
