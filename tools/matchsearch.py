"""Search source forms for a byte-perfect match, scored on the instruction stream.

Two ideas make this work where naive approaches stall.

1. Compile OUTSIDE the build tree.
   The source under test is copied to a scratch file and compiled with the
   exact ninja command line to a scratch object. The repo source and build
   directory are never touched, so candidate testing can run concurrently
   with anything else (including several instances of this tool, via
   --id / SANDBOX_ID).

2. Score the instruction stream, not the match percentage.
   objdiff's match_percent is a fuzzy similarity and is flat across many
   source-form changes -- it converged three separate searches at a false
   optimum during w01Dll recovery. This scores instead:

       lead    leading instructions matching the target exactly, after
               normalising pool-symbol names (ours are "@529"-style, the
               target's are lbl_1_rodata_BC-style; that is naming, not codegen)
       struct  structural insert/delete/replace ops from an opcode-only
               alignment -- i.e. genuinely extra or missing instructions
       diffs   total differing positions

   IMPORTANT: diffs is only meaningful when struct == 0. Past the first
   structural difference the index alignment breaks and diffs becomes noise.
   Use lead and struct until struct reaches 0, then minimise diffs.

   Caveat: the normaliser does not rewrite local branch labels (.L_XXXXXXXX),
   which encode addresses. An edit that changes the emitted instruction count
   shifts every later label and inflates diffs. Cross-check with lead/struct.

Typical workflow

    # where does it currently stand, and where does it first diverge?
    matchsearch.py score  --unit w01Dll/REL/w01Dll/world01 \
                          --src src/REL/w01Dll/world01.c --fn fn_1_13CC

    matchsearch.py show   ... --fn fn_1_13CC --from 500 --to 560

    # try concrete edits: a JSON array of {name, anchor, replacement}
    matchsearch.py try    ... --cands cands.json

    # once struct == 0, sweep declaration order (MWCC assigns float stack
    # slots in source order, so the declaration list is directly observable
    # in the frame layout)
    matchsearch.py decls  ... --fn fn_1_3214 --names valueB,valueA,...

Findings that repeatedly mattered on w01Dll, worth trying first on a new unit:

  - Reusing an existing accumulator instead of introducing a fresh variable.
    A second live range where the original had one shows up as extra spills
    and a spurious fmr after a call returning float.
  - Variable ROLES being swapped between two locals: which one wins a
    callee-saved register is decided by use pattern, not declaration order.
  - Extracting repeated code into static inline helpers. An inlinee's locals
    get their own stack-slot group at the point of expansion, which is often
    the only way to reproduce the target's frame layout. Passing a value as a
    parameter rather than copying an argument into a local matters.
  - Commutative operand order in float expressions: (a*b) and (b*a) allocate
    temporaries differently at -O0.

Things that were measured and do NOT help, so do not spend time on them:
redundant parentheses (AST no-ops), the `register` storage class, `const`
qualifiers, and compound assignment (x += y is identical to x = x + y).
"""
import argparse, difflib, json, os, re, shutil, subprocess, sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SYM = re.compile(r'("?@?[\w.]+"?)(@ha|@l)\b')


def scratch_dir():
    d = os.environ.get("MATCHSEARCH_TMP") or os.path.join(REPO, "build", "matchsearch")
    os.makedirs(d, exist_ok=True)
    return d


def compile_cmd(obj_path, src_path, out_obj):
    """Recover the real compile command from ninja and retarget it."""
    r = subprocess.run(["ninja", "-t", "commands", obj_path],
                       cwd=REPO, capture_output=True, text=True)
    line = [l for l in r.stdout.splitlines() if l.strip()][-1]
    # posix=False keeps Windows backslashes intact; strip the quotes ourselves,
    # since arguments like -pragma "cats off" must survive as ONE token.
    parts = [p[1:-1] if len(p) > 1 and p[0] == p[-1] == '"' else p
             for p in __import__("shlex").split(line, posix=False)]
    out, skip = [], False
    for i, p in enumerate(parts):
        if skip:
            skip = False
            continue
        if p == "-o":
            out += ["-o", out_obj]
            skip = True
        elif p == "-MMD":
            continue
        elif p.endswith(".c") and not p.startswith("-"):
            out.append(src_path)
        else:
            out.append(p)
    return out


def instrs(path, fn):
    out, on = [], False
    for line in open(path, encoding="utf-8", errors="replace"):
        st = line.strip()
        if re.match(r"^\.fn %s\b" % re.escape(fn), st):
            on = True
            continue
        if on and (".endfn %s" % fn in line or re.match(r"^\.fn \w", st)):
            break
        if not on:
            continue
        s = re.sub(r"/\*.*?\*/", "", line).strip()
        if not s or s.startswith((".", "#")) or s.endswith(":"):
            continue
        out.append(s)
    return out


norm = lambda s: SYM.sub(r"S\2", s)


class Sandbox(object):
    def __init__(self, unit, src, obj, target_s, ident=""):
        d = scratch_dir()
        self.src = os.path.join(REPO, src)
        self.obj = obj
        self.target_s = os.path.join(REPO, target_s)
        self.c = os.path.join(d, "trial%s.c" % ident)
        self.o = os.path.join(d, "trial%s.o" % ident)
        self.s = os.path.join(d, "trial%s.s" % ident)
        self.dtk = os.path.join(REPO, "build", "tools", "dtk.exe")
        if not os.path.exists(self.dtk):
            self.dtk = os.path.join(REPO, "build", "tools", "dtk")
        self.cmd = None

    def build(self, text):
        open(self.c, "w", encoding="utf-8", newline="").write(text)
        if self.cmd is None:
            self.cmd = compile_cmd(self.obj, self.c, self.o)
        if subprocess.run(self.cmd, cwd=REPO, capture_output=True).returncode != 0:
            return False
        subprocess.run([self.dtk, "elf", "disasm", self.o, self.s],
                       cwd=REPO, capture_output=True)
        return True

    def align(self, fn):
        t, o = instrs(self.target_s, fn), instrs(self.s, fn)
        tn, on_ = [norm(x) for x in t], [norm(x) for x in o]
        lead = 0
        for a, b in zip(tn, on_):
            if a != b:
                break
            lead += 1
        diffs = sum(1 for a, b in zip(tn, on_) if a != b) + abs(len(t) - len(o))
        op = lambda s: s.split()[0]
        sm = difflib.SequenceMatcher(None, [op(x) for x in t], [op(x) for x in o],
                                     autojunk=False)
        struct = sum(1 for tag, *_ in sm.get_opcodes() if tag != "equal")
        return {"lead": lead, "struct": struct, "diffs": diffs,
                "n_target": len(t), "n_ours": len(o)}

    def score(self, text, fns):
        if not self.build(text):
            return None
        return dict((f, self.align(f)) for f in fns)


def fmt(s, fns):
    return "  ".join("%s lead=%-5d struct=%-2d diffs=%-5d"
                     % (f.replace("fn_1_", ""), s[f]["lead"], s[f]["struct"],
                        s[f]["diffs"]) for f in fns)


def better(s, b, fns):
    out = []
    for f in fns:
        n = f.replace("fn_1_", "")
        if s[f]["lead"] > b[f]["lead"]:
            out.append("%s lead +%d" % (n, s[f]["lead"] - b[f]["lead"]))
        if s[f]["struct"] < b[f]["struct"]:
            out.append("%s struct -%d" % (n, b[f]["struct"] - s[f]["struct"]))
        if s[f]["struct"] == 0 and b[f]["struct"] == 0 and s[f]["diffs"] < b[f]["diffs"]:
            out.append("%s diffs -%d" % (n, b[f]["diffs"] - s[f]["diffs"]))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", choices=["score", "show", "try", "decls"])
    ap.add_argument("--unit", required=True, help="objdiff unit name")
    ap.add_argument("--src", required=True, help="source path, relative to repo root")
    ap.add_argument("--obj", help="object path (default derived from --src)")
    ap.add_argument("--target", help="canonical target .s (default derived from --unit)")
    ap.add_argument("--fn", action="append", default=[], help="function(s) to score")
    ap.add_argument("--id", default=os.environ.get("SANDBOX_ID", ""),
                    help="unique suffix so concurrent runs do not collide")
    ap.add_argument("--cands", help="JSON array of {name,anchor,replacement}")
    ap.add_argument("--names", help="comma-separated declaration names for decls mode")
    ap.add_argument("--from", dest="lo", type=int, default=0)
    ap.add_argument("--to", dest="hi", type=int, default=60)
    a = ap.parse_args()

    obj = a.obj or ("build/GP6E01/%s.o" % a.src[:-2])
    # unit "w01Dll/REL/w01Dll/world01" -> build/GP6E01/w01Dll/asm/REL/w01Dll/world01.s
    head, _, rest = a.unit.partition("/")
    target = a.target or ("build/GP6E01/%s/asm/%s.s" % (head, rest))
    fns = a.fn or []
    sb = Sandbox(a.unit, a.src, obj, target, a.id)
    base_src = open(sb.src, encoding="utf-8").read()

    if a.mode == "show":
        sb.build(base_src)
        t, o = instrs(sb.target_s, fns[0]), instrs(sb.s, fns[0])
        print("idx  | TARGET                                 | OURS")
        for i in range(a.lo, min(a.hi, max(len(t), len(o)))):
            x = t[i] if i < len(t) else ""
            y = o[i] if i < len(o) else ""
            print("%4d | %-38s | %-30s%s"
                  % (i, x, y, "" if norm(x) == norm(y) else "  <-- DIFF"))
        return

    base = sb.score(base_src, fns)
    if base is None:
        print("baseline build FAILED")
        return 1
    print("BASELINE  " + fmt(base, fns))

    if a.mode == "score":
        print(json.dumps(base, indent=1))

    elif a.mode == "try":
        wins = []
        for c in json.load(open(a.cands, encoding="utf-8")):
            n = base_src.count(c["anchor"])
            if n != 1:
                print("%-30s ANCHOR %s (%d)"
                      % (c["name"], "MISS" if n == 0 else "AMBIGUOUS", n))
                continue
            s = sb.score(base_src.replace(c["anchor"], c["replacement"], 1), fns)
            if s is None:
                print("%-30s BUILD FAIL" % c["name"])
                continue
            b = better(s, base, fns)
            print("%-30s %s%s" % (c["name"], fmt(s, fns),
                                  ("   <<< " + ", ".join(b)) if b else ""))
            if b:
                wins.append(c["name"])
        print("\n%d improving candidate(s): %s" % (len(wins), ", ".join(wins) or "-"))

    elif a.mode == "decls":
        names = a.names.split(",")
        block = lambda o: "".join("    float %s;\n" % x for x in o).rstrip("\n")
        cur = list(names)
        assert base_src.count(block(cur)) == 1, "declaration block not found/unique"
        tmpl = base_src.replace(block(cur), "\x00D\x00", 1)
        key = lambda s: (-s[fns[0]]["lead"], s[fns[0]]["struct"], s[fns[0]]["diffs"])
        best, seen = base, {tuple(cur)}
        while True:
            found = None
            for i, nm in enumerate(names):
                rest = [x for x in cur if x != nm]
                for j in range(len(names)):
                    order = rest[:j] + [nm] + rest[j:]
                    if tuple(order) in seen:
                        continue
                    seen.add(tuple(order))
                    s = sb.score(tmpl.replace("\x00D\x00", block(order), 1), fns)
                    if s and key(s) < key(best) and (found is None or key(s) < key(found[0])):
                        found = (s, order, "%s->%d" % (nm, j))
            if not found:
                print("converged")
                break
            best, cur, mv = found
            print("ACCEPT %-22s %s" % (mv, fmt(best, fns)))
        print("\nfinal order: %s" % ", ".join(cur))

    print("\n(repo source never modified)")


if __name__ == "__main__":
    sys.exit(main() or 0)
