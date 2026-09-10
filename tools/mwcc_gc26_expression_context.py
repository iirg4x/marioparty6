"""Pinned GC2.6 recursive expression contexts. No process engine or event emission.

Pointers stay internal; integrations must tokenize before publishing. Entry/RET
hooks must run before their original instruction. Stack checks authenticate the
conditional static map during execution; unknown states fail closed.
"""
import json

COMPILER_SHA256 = "316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8"
_DATA = json.loads(r'''{"0x444370":{"kinds":[75],"prefix":"53565583ec088b54","returns":["0x4443ed","0x444436","0x44444e"]},"0x44bda0":{"kinds":[59],"prefix":"5356578b7424108b","returns":["0x44bdd3"]},"0x44bde0":{"kinds":[58],"prefix":"5356575583ec408b","returns":["0x44bf83"]},"0x44bf90":{"kinds":[7,28,29],"prefix":"5356575583ec108b","returns":["0x44c046","0x44c189","0x44c29b"]},"0x44c2a0":{"kinds":[19,20,21,22,23,24],"prefix":"53568b44240c8b5c","returns":["0x44c2fd","0x44c33e","0x44c351"]},"0x44d0c0":{"kinds":[2,3,10,13,14,31,32,33,34,35,36,37,38,39,40,42,43,44,45,46,47,57,60,61,62,63,64,65,66,67,68,69,70,71,74,77],"prefix":"6834110000688094","returns":["0x44d0d1"]},"0x44d0e0":{"kinds":[56],"prefix":"5356558b6c24108b","returns":["0x44d12c"]},"0x44d130":{"kinds":[54,55],"prefix":"53558b6c240c8b5c","returns":["0x44d161"]},"0x44d170":{"kinds":[76],"prefix":"5356575581ecd000","returns":["0x44d6e0","0x44d95b","0x44db74","0x44dbba"]},"0x44dcd0":{"kinds":[53],"prefix":"5356575581ecd800","returns":["0x44e1be","0x44e399","0x44e405","0x44e671","0x44e7b1","0x44e909","0x44ea65","0x44ebb1","0x44f3d0"]},"0x44f910":{"kinds":[52],"prefix":"68f9090000688094","returns":["0x44f921"]},"0x44f930":{"kinds":[51],"prefix":"803db5b05e000074","returns":["0x44f968"]},"0x44f970":{"kinds":[50],"prefix":"535657558b6c2414","returns":["0x44f9ee","0x44f9fd"]},"0x44fa00":{"kinds":[49],"prefix":"68b3090000688094","returns":["0x44fa11"]},"0x44fa20":{"kinds":[48],"prefix":"5356575583ec488b","returns":["0x44fa95","0x44fadc","0x450438"]},"0x450440":{"kinds":[41],"prefix":"535657558b442414","returns":["0x450501"]},"0x4505f0":{"kinds":[30],"prefix":"5356575583ec708b","returns":["0x4506bd","0x4506f8","0x451239"]},"0x451240":{"kinds":[27],"prefix":"535657558b5c2414","returns":["0x451289","0x4512bb","0x45133a"]},"0x451340":{"kinds":[26],"prefix":"535657558b5c2414","returns":["0x451389","0x4513bb","0x45143a"]},"0x451440":{"kinds":[25],"prefix":"5356575581ece000","returns":["0x45148e","0x4514ca","0x451d06"]},"0x451d10":{"kinds":[18],"prefix":"5356575583ec188b","returns":["0x451d55","0x451d81","0x451e90"]},"0x451ea0":{"kinds":[17],"prefix":"535657558b542414","returns":["0x451ee7","0x451f11","0x451f44"]},"0x451f50":{"kinds":[16],"prefix":"535657558b542414","returns":["0x451f98","0x451fc6","0x452134"]},"0x452140":{"kinds":[15],"prefix":"5356575583ec388b","returns":["0x4521a4","0x4521d0","0x45248c"]},"0x452490":{"kinds":[12],"prefix":"5356575581ec8800","returns":["0x45251c","0x452562","0x452678","0x452c77"]},"0x452c80":{"kinds":[11],"prefix":"5356575583ec708b","returns":["0x452cd9","0x452d0f","0x4532f6"]},"0x4534d0":{"kinds":[9],"prefix":"535657558b4c2414","returns":["0x453518","0x453546","0x4536f4"]},"0x453700":{"kinds":[8],"prefix":"535657558b4c2414","returns":["0x45386c"]},"0x453870":{"kinds":[6],"prefix":"5356575583ec308b","returns":["0x4538b8","0x453982","0x453a63"]},"0x453a70":{"kinds":[5],"prefix":"5356575583ec488b","returns":["0x453ab8","0x453b83","0x453d8d","0x453e0c"]},"0x453e10":{"kinds":[4],"prefix":"5356575583ec208b","returns":["0x453e76","0x453ead","0x453edd","0x4541a3"]},"0x4541b0":{"kinds":[0,1],"prefix":"5356575583ec608b","returns":["0x454242","0x45427c","0x4547be"]},"0x4547c0":{"kinds":[73],"prefix":"5356578b4c24108b","returns":["0x454827"]},"0x454830":{"kinds":[72],"prefix":"5356578b7c24108b","returns":["0x45487e"]}}''')
HANDLERS = {int(k, 16): {"kinds": tuple(v["kinds"]), "prefix": bytes.fromhex(v["prefix"]),
                       "returns": tuple(int(r, 16) for r in v["returns"])}
            for k, v in _DATA.items()}
DISPATCH = {kind: address for address, row in HANDLERS.items() for kind in row["kinds"]}
RETURN_OWNERS = {ret: address for address, row in HANDLERS.items() for ret in row["returns"]}


class ContextError(ValueError):
    pass


def allocation_destination_request(*, allocation_site, esp, ebx, ebp, counter_before,
                                   base, active, read, compiler_sha256=COMPILER_SHA256):
    """Decode only the sealed kind10 materializer increment, using its own frame.

    No hooks, publication or inferred source semantics. Addresses in the result
    are RVAs; active expression identity remains the integrator's token binding.
    Other sites return None, never reuse outer-handler arguments as this ABI.
    """
    if compiler_sha256 != COMPILER_SHA256:
        raise ContextError('unsupported compiler')
    if allocation_site != 0xE2E9E:
        return None
    def exact(address, size):
        data = read(address, size)
        if len(data) != size:
            raise ContextError('short materializer frame/code read')
        return data
    def u32(address):
        return int.from_bytes(exact(address, 4), 'little')
    if exact(base + 0xE2C10, 15) != bytes.fromhex('5356575583ec088b7424208b5c2424'):
        raise ContextError('materializer prologue drift')
    if exact(base + 0xE2E8E, 10) != bytes.fromhex('6685db74050fbfebeb0c'):
        raise ContextError('materializer destination branch drift')
    requested = u32(esp + 0x24)
    if requested & 0xffff or ebx & 0xffff or ebp != counter_before:
        raise ContextError('materializer allocation contradicts zero destination branch')
    return_pc = u32(esp + 0x18)
    instruction = exact(return_pc - 5, 5)
    if instruction[0] != 0xE8 or return_pc + int.from_bytes(instruction[1:], 'little', signed=True) != base + 0xE2C10:
        raise ContextError('materializer caller is not authenticated direct call')
    caller = return_pc - base - 5
    joined = bool(active and active['handler'] in HANDLERS and esp < active['esp']
                  and active['handler'] - 0x400000 <= caller
                  < max(HANDLERS[active['handler']]['returns']) - 0x400000)
    return dict(status='CAPTURED_ACTUAL_HELPER_DESTINATION', helper_rva=0xE2C10,
                allocation_site=allocation_site, caller_rva=caller,
                requested_destination_low16=requested & 0xffff,
                actual_destination_zero=True, branch='zero_destination_allocates_kind10_materialization',
                active_expression_join='direct_caller_inside_active_handler' if joined else 'not_established',
                expression_kind=active['expression_kind'] if joined else None,
                source_repair_supported=False)


def hook_descriptors(compiler_sha256=COMPILER_SHA256):
    if compiler_sha256 != COMPILER_SHA256:
        raise ContextError("unsupported compiler")
    rows = [{"id": "gc26_expr_enter_%08x" % a, "address": a,
             "expected_bytes": h["prefix"].hex(), "phase": "entry"}
            for a, h in sorted(HANDLERS.items())]
    rows += [{"id": "gc26_expr_exit_%08x" % a, "address": a,
              "expected_bytes": "c3", "phase": "exit"}
             for a in sorted(RETURN_OWNERS)]
    return tuple(sorted(rows, key=lambda row: row["address"]))


def _uint(value, label, nonzero=False):
    if type(value) is not int or not (int(nonzero) <= value <= 0xffffffff):
        raise ContextError("invalid " + label)
    return value


class ExpressionTracker:
    def __init__(self, compiler_sha256, max_depth=256):
        if compiler_sha256 != COMPILER_SHA256:
            raise ContextError("unsupported compiler")
        if type(max_depth) is not int or not 1 <= max_depth <= 4096:
            raise ContextError("invalid depth bound")
        self.max_depth = max_depth
        self._frames = {}
        self._failed = False

    def _check(self):
        if self._failed:
            raise ContextError("expression context invalidated")

    def enter(self, thread, handler, expression_pointer, expression_kind, esp, return_pc,
              result_descriptor):
        self._check()
        try:
            _uint(thread, "thread", True)
            for label, value in (("expression", expression_pointer), ("ESP", esp),
                                 ("return PC", return_pc)):
                _uint(value, label, True)
            _uint(result_descriptor, "result descriptor")
            if type(expression_kind) is not int or DISPATCH.get(expression_kind) != handler:
                raise ContextError("expression kind/handler disagreement")
            stack = self._frames.setdefault(thread, [])
            if len(stack) >= self.max_depth:
                raise ContextError("expression depth exceeded")
            if stack and esp >= stack[-1]["esp"]:
                raise ContextError("nested expression stack did not descend")
            frame = dict(handler=handler, expression_pointer=expression_pointer,
                         expression_kind=expression_kind, esp=esp, return_pc=return_pc,
                         result_descriptor=result_descriptor)
            stack.append(frame)
            return dict(frame)
        except (ValueError, TypeError):
            self._failed = True
            raise

    def exit(self, thread, return_site, esp, return_pc):
        self._check()
        stack = self._frames.get(thread, [])
        if (not stack or RETURN_OWNERS.get(return_site) != stack[-1]["handler"]
                or esp != stack[-1]["esp"] or return_pc != stack[-1]["return_pc"]):
            self._failed = True
            raise ContextError("expression exit frame mismatch")
        frame = stack.pop()
        return dict(frame)

    def active(self, thread):
        self._check()
        stack = self._frames.get(thread, [])
        return dict(stack[-1]) if stack else None

    def assert_empty(self):
        self._check()
        if any(self._frames.values()):
            self._failed = True
            raise ContextError("unclosed expression frames")

    def read_entry(self, thread, handler, esp, read_u32, read_u8):
        """Callbacks read target memory; no OS or relocation policy here."""
        self._check()
        try:
            expr = read_u32(esp + 4)
            return self.enter(thread, handler, expr, read_u8(expr), esp,
                              read_u32(esp), read_u32(esp + 16))
        except Exception:
            self._failed = True
            raise

    def read_exit(self, thread, return_site, esp, read_u32):
        self._check()
        try:
            return self.exit(thread, return_site, esp, read_u32(esp))
        except Exception:
            self._failed = True
            raise
