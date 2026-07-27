# frandmod signedness: DOL contract and mixed TU visibility

Date: 2026-07-27. Target: GP6E01 main.dol
(sha1 `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`). Compiler profile for the
cited DOL TUs: GC/2.6, `-O0,p -char unsigned` (game overlay profile from
`configure.py`).

## Question

`include/game/frand.h` declared `u32 frandmod(u32)`, `src/game/frand.c` defined
it as u32, both `game/frand.c` and `game/charman.c` are `Object(Matching)`, and
the full build verifies 137/137 checksums. Yet the retail DOL converts
`frandmod` results to float through the signed `xoris 0x8000` sequence inside
`CharModelLandDustCreate`. Both could not hold if charman compiled against the
u32 prototype.

## Retail byte evidence (checksum-proof container, byte-verified)

`CharModelLandDustCreate` = `.text:0x8005F968` (symbols.txt), DOL file offset
`0x5C708`. At `0x8005FAA8`:

```text
0x8005FAA8  38600004  li r3, 4
0x8005FAAC  4BFE0229  bl 0x8003FCD4        # frandmod
0x8005FAB0  38030008  addi r0, r3, 8
0x8005FAB4  7C0000D0  neg r0, r0
0x8005FAB8  C8228880  lfd f1, -0x7780(r2)  # 4330000080000000
0x8005FABC  6C008000  xoris r0, r0, 0x8000 # signed int-to-float marker
0x8005FAC0  90010034  stw r0, 0x34(r1)
0x8005FAC4  3C004330  lis r0, 0x4330
```

The earlier `frandmod(6)` site at `0x8005F9C8-0x8005F9CC` likewise uses
`xoris r0, r3, 0x8000`. The signed pattern (xoris + `4330000080000000` magic)
is distinct from the unsigned pattern (no xoris, `4330000000000000` magic), so
these sites prove charman's TU treated the `frandmod` result as signed.

## Resolution: charman.c never saw the prototype

Preprocessing `src/game/charman.c` with the exact build flags
(`mwcceppc GC/2.6 ... -i include -i build/GP6E01/include -E`) shows the first
occurrence of `frandmod` is the call site itself: no declaration is in scope.
Only `include/game/board/effect.h` includes `game/frand.h`, and charman.c
includes neither. Under C89 the call is implicitly declared as returning `int`,
which is exactly the signed conversion the DOL shows. The u32 prototype in the
shared header never participated in charman's codegen, so both observations
were simultaneously true.

The retail evidence therefore shows mixed visibility in the original program:

- TUs without the declaration (implicit signed int): `game/charman.c`,
  `game/hsfanim.c`, `game/kerent.c`, `game/mggamemes.c`, `game/mic.c`,
  `REL/fileseldll/filename.c`, plus `REL/mdpartydll/stage.c` and
  `REL/mdseldll/mdsel.c` which carry explicit local `s32 frandmod(s32)`
  declarations recorded in `config/GP6E01/tu_declarations.json`
  (authority: dol).
- TUs that resolved `frandmod` through the shared header and match with
  unsigned codegen at the result's use sites: `game/actman.c` (via
  `SafeNormalize`, inlined into `MgPlayerVecChase` `0x8007E9F8`,
  `MgActorVecChase` `0x8007EC44`, `GetStickMtx` `0x8007F114`, `PlayerColHook`
  `0x8007F7D4/0x8007FC44/0x8007FDDC/0x80080044`, `MgActorExec` `0x80085938`)
  and `game/colman.c`.

## Source-shape consequence (byte-verified both directions)

Flipping the header to `s32 frandmod(s32 modulus)` (the tu_declarations.json
contract) keeps `frand.o` byte-identical because the body computes
`(frand_seed & 0x7FFFFFFF) % (u32)modulus` and unsigned usual-arithmetic
conversion already emits `divwu` (`0x8003FD58 7C041B96`). Consumers that
previously compiled against the u32 prototype need their unsigned codegen
preserved explicitly:

- float mixes: `(u32)frandmod(n)` before the float context restores the
  no-xoris `4330000000000000` conversion (`actman.c` SafeNormalize x/z,
  `PlayerColHook` rotY; `colman.c` colDelta x/z, bounce scale);
- zero compares: `(u32)frandmod(1) != 0` restores `cmplwi` (`0x28xxxxxx`);
  the signed form emits `cmpwi` (`0x2Cxxxxxx`), observed as exactly eight
  single-byte DOL regressions at the addresses listed above before the cast
  was added.

All other Matching consumers of the header (`game/saveload.c`,
`REL/fileseldll/filesel.c`, `REL/selmenuDll/selmenu.c`,
`REL/bootDll/opening.c`, and the frand.h-including board TUs with no frandmod
calls) use the result in width-preserving integer contexts or already carry
`(s32)`/`(s16)` casts, and were re-verified by the full checksum gate.

Full-container proof after the change: `dtk shasum -c config/GP6E01/build.sha1`
reports 137 files OK (main.dol byte-identical to retail), from the workspace
tree at the commit recorded in the queue verification for `main:game/frand`.

## Why this matters beyond matching

Under the old u32 prototype, any consumer added later with the header in scope
would compute `-(frandmod(4)+8)` as a wrapped unsigned value near 2^32; a
float conversion then yields 4.29e9 instead of a small negative number. The
retail-proven s32 contract removes that trap while keeping every retail byte
identical.
