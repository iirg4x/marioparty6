# HU_DISP_WIDTH: retail is 576.0f, not 768.0f

Date: 2026-07-28. Target: GP6E01 main.dol
(sha1 `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`). Source commit under test:
`f538fb2c97beab37640483f617dac94434aaa9b3`.

## Question

A report claimed `include/game/disp.h` was wrong: that `HU_DISP_WIDTH 576.0f`
contradicts a retail DOL which allegedly projects sprites and messages through a
768-wide orthographic space, citing `HuSprDispInit` reading `768.0` from sdata2
`0x802C11D0` and `MesDispFunc` reading `768.0` from `0x802C17B4`. If true, seven
`Object(Matching)` TUs would be matching against a wrong constant.

The claim is false. Both cited addresses hold `576.0f`. The header is already
correct and must not be changed.

## Retail byte evidence (checksum-proof container, byte-verified)

`_SDA2_BASE_` is established in `__init_registers` = `.init:0x800032B0`
(symbols.txt), DOL file offset `0x2B0`:

```text
0x8000332C  3C4080 2C  lis r2, 0x802C
0x80003330  604290 C0  ori r2, r2, 0x90C0   # _SDA2_BASE_ = 0x802C90C0
```

This confirms the base the claim used. The disagreement is only in the operand
values.

### HuSprDispInit = `.text:0x800110CC` (file offset `0xDE6C`)

```text
0x8001111C  38610008  addi r3, r1, 8          # &proj
0x80011120  C0228108  lfs  f1, -32504(r2)     # 0x802C11C8
0x80011124  C042810C  lfs  f2, -32500(r2)     # 0x802C11CC
0x80011128  C0628108  lfs  f3, -32504(r2)     # 0x802C11C8
0x8001112C  C0828110  lfs  f4, -32496(r2)     # 0x802C11D0  <- the cited slot
0x80011130  C0A28108  lfs  f5, -32504(r2)     # 0x802C11C8
0x80011134  C0C28114  lfs  f6, -32492(r2)     # 0x802C11D4
0x80011138  48090729  bl   0x800A1860         # C_MTXOrtho
```

Operand values read from the DOL:

```text
0x802C11C8 = 0.0    (00000000)
0x802C11CC = 480.0  (43f00000)
0x802C11D0 = 576.0  (44100000)   <- claim said 768.0
0x802C11D4 = 10.0   (41200000)
0x802C11D8 = 640.0  (44200000)
0x802C11DC = 1.0    (3f800000)
```

So retail is `C_MTXOrtho(proj, 0, 480, 0, 576, 0, 10)`, exactly
`MTXOrtho(proj, 0, HU_DISP_HEIGHT, 0, HU_DISP_WIDTH, 0, 10)` at
`src/game/sprput.c:36` under 576.0f/480.0f.

The same function then calls `GXSetViewportJitter` (`0x800B75E0`) or
`GXSetViewport` (`0x800B7638`) with `0x802C11D8 = 640.0` and
`0x802C11CC = 480.0`, then `GXSetScissor` (`0x800B76A4`) with `li r5,640; li
r6,480`. Retail therefore distinguishes a **576-wide projection space** from a
**640-wide framebuffer**, which is precisely the `HU_DISP_WIDTH` vs `HU_FB_WIDTH`
split the header already encodes.

### MesDispFunc = `.text:0x8004D864`, in `game/window.c` (`.text` split `0x8004C33C-0x80054DB0`)

```text
0x8004D904  C0228720  lfs f1, -30944(r2)      # 0x802C17E0
0x8004D908  C04286FC  lfs f2, -30980(r2)      # 0x802C17BC
0x8004D90C  C0628720  lfs f3, -30944(r2)      # 0x802C17E0
0x8004D910  C08286F4  lfs f4, -30988(r2)      # 0x802C17B4  <- the cited slot
0x8004D914  C0A28720  lfs f5, -30944(r2)      # 0x802C17E0
0x8004D918  C0C28724  lfs f6, -30940(r2)      # 0x802C17E4
0x8004D91C  48053F45  bl  0x800A1860          # C_MTXOrtho
```

```text
0x802C17E0 = 0.0    (00000000)
0x802C17BC = 480.0  (43f00000)
0x802C17B4 = 576.0  (44100000)   <- claim said 768.0
0x802C17E4 = 10.0   (41200000)
```

Matching `src/game/window.c:596`
`C_MTXOrtho(proj, 0.0f, HU_DISP_HEIGHT, 0.0f, HU_DISP_WIDTH, 0.0f, 10.0f)`.

### Whole-image constant census

Scanning every aligned word of all DOL data sections:

- `44100000` (576.0f): **11** occurrences —
  `0x802C1160, 0x802C11D0, 0x802C14BC, 0x802C17B4, 0x802C1E1C, 0x802C3108,
  0x802C3580, 0x802C3CA8, 0x802C493C, 0x802C4B3C, 0x802C4F3C`.
- `44400000` (768.0f): **0** occurrences.

The value 768.0f does not exist anywhere in the retail DOL.

### Retail arithmetic pins the constant to 576

`board/status.c` writes four tables as `HU_DISP_WIDTH ± k`. The compiler folded
those at build time, so retail `.data` contains the *result* of the arithmetic
and can be solved for the operand. From `symbols.txt` addresses:

| retail datum | value | source expression | implied HU_DISP_WIDTH |
| --- | --- | --- | --- |
| `statusPosOn[1].x` `0x802481B8` | 462.0 | `HU_DISP_WIDTH-114` | 576 |
| `statusPosOff[1].x` `0x802481D8` | 674.0 | `HU_DISP_WIDTH+98` | 576 |
| `statusPosOnTeam[1].x` `0x802481F8` | 436.0 | `HU_DISP_WIDTH-140` | 576 |
| `statusPosOffTeam[1].x` `0x80248208` | 700.0 | `HU_DISP_WIDTH+124` | 576 |

Four independent equations, one consistent solution: 576. Under 768 these would
read 654, 866, 628 and 892 — none of which appear. `HU_DISP_HEIGHT` is pinned the
same way by `statusPosOn[2].y = 400.0 = HU_DISP_HEIGHT-80`, giving 480.

This is the strongest form of the evidence: it does not depend on reading any
float pool slot correctly, only on retail's folded results.

## Falsification experiment (byte-verified both directions)

Baseline at `f538fb2`: `ninja` then
`build/tools/dtk shasum -q -c config/GP6E01/build.sha1` reports **137 files OK**,
and `build/GP6E01/main.dol` is sha1-identical to `orig/GP6E01/sys/main.dol`
(`b897e6ade6b3a0cd2f9907689f38a3b19c327e70`).

With the single edit `HU_DISP_WIDTH 576.0f -> 768.0f`:

```text
136 files OK
build/GP6E01/main.dol: FAILED
```

Comparing the produced DOL against retail, the two slots the claim cited are
among the regressions, and they change in the direction the macro dictates:

```text
0x802C11D0 retail=576.0 (44100000)  built=768.0 (44400000)  DIFF
0x802C17B4 retail=576.0 (44100000)  built=768.0 (44400000)  DIFF
```

Nine differing regions in total, including `.data 0x802481B8-0x8024820A` (82
bytes), which spans `statusPosOn` (`0x802481B0`), `statusPosOff` (`0x802481D0`),
`statusPosOnTeam` (`0x802481F0`) and `statusPosOffTeam` (`0x80248200`) — the
`board/status.c` tables built from `HU_DISP_WIDTH`.

This closes the causal loop in both directions: the header macro is the sole
source of those retail bytes, and only 576.0f reproduces them. Reverting
restores 137/137 OK.

## Contradiction resolution

The prompt offered three candidate explanations for "header says 576 yet the
build matches": (a) no Matching TU uses the macro, (b) Matching consumers use it
only where 576 and 768 coincide, (c) the DOL sites read a runtime variable and
the macro is a decomp invention.

None applies. The premise itself was wrong: **the DOL contains 576.0f at both
cited addresses**, so there was never a contradiction to resolve. The macro is
used by seven Matching TUs, it does reach codegen as a pooled float literal, and
retail agrees with it.

The likely origin of the error is a misread of the constant `44100000` (576.0f)
as 768.0f; the SDA2 base and both operand addresses in the report were correct,
which is why the report looked well-formed.

## Consumers of `include/game/disp.h`

`disp.h` is included only by `.c` files — no header includes it — so the consumer
set is closed and exact:

| TU | HU_DISP_* uses | status |
| --- | --- | --- |
| `game/sprput.c` | 2 | Matching |
| `game/window.c` | 5 | Matching |
| `game/printfunc.c` | 7 | Matching |
| `game/hsfman.c` | 5 | Matching |
| `game/hsfex.c` | 6 | Matching |
| `board/status.c` | 8 | Matching |
| `board/window.c` | 3 | Matching |
| `board/math.c` | 10 | NonMatching |
| `board/audio.c` | 1 | NonMatching |

`board/config.c`, `board/effect.c`, `board/wipe.c`, `game/hsfanim.c`,
`game/hsfdraw.c` and `game/wipe.c` include `disp.h` but use only `HU_FB_*`
(or nothing), so they are unaffected by the width constant.

Seven Matching consumers is the re-verification obligation for any future edit to
this header.

## The `status.c` x=700 corroboration inverts

The report cited `src/board/status.c:404`,
`HuSprGrpPosSet(statusMasuWork.gid, 700.0f, 72.0f)`, as evidence for a 768-wide
space because 700 would be off-canvas at 576.

That site is the `else` branch — the hide path. Parking a sprite beyond the right
edge is the established idiom in this very file: `statusPosOff` places players at
`HU_DISP_WIDTH+98` and `-98`, and `statusPosOffTeam` at `HU_DISP_WIDTH+124`.

Under 576 that last expression is **exactly 700**, and retail
`statusPosOffTeam[1].x` at `0x80248208` does hold `700.0` (`442f0000`). The
literal `700.0f` at line 404 is the same off-screen park X written out longhand.
It is not an anomaly needing a wider canvas to explain — it is `HU_DISP_WIDTH+124`
evaluated at 576, and retail agrees to the byte.

Under 576 the element is correctly hidden. Under 768 it would remain **on**
screen, which would be the actual bug. The corroboration argues for 576.

## Consequence

No change to `include/game/disp.h` is warranted. `HU_DISP_WIDTH 576.0f`,
`HU_DISP_HEIGHT 480.0f`, the derived `HU_DISP_CENTERX/Y`, and the separate
`HU_FB_WIDTH 640` / `HU_FB_HEIGHT 480` all agree with retail. Nothing is
promoted to `main` from this investigation.

## Note for the port (decomp truth only)

A port built on `HU_DISP_WIDTH = 576` for its 2D layer is resting on the correct
retail constant; no decomp-side adaptation is owed. Retail composes 2D by
projecting a 576x480 orthographic space onto a 640x480 framebuffer — an
anisotropic map, not a 1:1 blit — so 2D X coordinates are scaled by 640/576 =
10/9 on the way to the framebuffer while Y is 1:1. Any widescreen or
resolution-independent work should adapt that mapping rather than redefine the
projection width. Adapting the port is deliberately out of scope here; this note
records only what the retail bytes prove.
