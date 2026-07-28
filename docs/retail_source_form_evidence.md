# Retail source forms that must not be "fixed": coin, hsfmotion/hsfdraw, telop

Date: 2026-07-28. Target: `orig/GP6E01/sys/main.dol`
(sha1 `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`). Source commit under test:
`a34972549072d7e0cd8a0fe01e3d7eecb02cc477`. Addresses from
`config/GP6E01/symbols.txt`.

Three findings, each an authentic retail behaviour that reads as a bug. A future
recovery agent that "corrects" any of them moves away from the target.

> Note on evidence sourcing: a sibling worktree (`marioparty6-pin`) carries an
> `orig/GP6E01/sys/main.dol` that hashes to
> `d9c58f60e5abbab905edc338f86f5c22fca2fd58` and is **not** the retail image.
> Every byte below was read from the `b897e6ad` DOL. Check the hash before
> quoting DOL bytes from any `orig/` tree.

## 1. `board/coin.c`: `coinEffBankTbl[mbRandMod(32)]` overreads by design

`src/board/coin.c` declares a 16-entry table and indexes it with a 0..31 value:

```c
static u8 coinEffBankTbl[] = {          /* line 162, 16 entries */
    2, 2, 2, 2, 2, 3, 3, 3,
    2, 2, 2, 2, 2, 3, 3, 3,
};
static u8 coinEffBankTbl2[] = {         /* line 167, 16 entries */
    2, 2, 2, 2, 2, 3, 3, 3,
    2, 3, 2, 3, 2, 3, 0, 1,
};
...
particleDataP->animBank = coinEffBankTbl[mbRandMod(32)];   /* line 785 */
```

Retail does exactly this. `mbCoinEffCreate` is `.text:0x8017C618` (size `0x4A0`);
`mbRandMod` is `.text:0x8014D348`.

```text
0x8017C688  3c608024  lis   r3,0x8024
0x8017C68C  3b837de0  addi  r28,r3,0x7DE0        # r28 = 0x80247DE0

# in-range control: coinEffColorNoTbl[mbRandMod(16)]
0x8017C7DC  38600010  li    r3,16
0x8017C7E0  4bfd0b69  bl    0x8014D348           # mbRandMod
0x8017C7E4  389c0140  addi  r4,r28,0x140         # 0x80247F20 coinEffColorNoTbl
0x8017C7E8  7f2418ae  lbzx  r25,r4,r3

# the overread: coinEffBankTbl[mbRandMod(32)]
0x8017C878  38600020  li    r3,32
0x8017C87C  4bfd0acd  bl    0x8014D348           # mbRandMod
0x8017C880  389c0150  addi  r4,r28,0x150         # 0x80247F30 coinEffBankTbl
0x8017C884  7c0418ae  lbzx  r0,r4,r3             # index 0..31, table is 16 bytes
0x8017C888  b01f004c  sth   r0,76(r31)
```

The index is used unmasked and unclamped. Symbol sizes make the consequence
exact:

```text
coinEffColorNoTbl = .data:0x80247F20  size 0x10
coinEffBankTbl    = .data:0x80247F30  size 0x10
coinEffBankTbl2   = .data:0x80247F40  size 0x10   <- immediately adjacent
```

Retail bytes at those addresses:

```text
0x80247F30  02020202020303030202020202030303   coinEffBankTbl
0x80247F40  02020202020303030203020302030001   coinEffBankTbl2
```

Both agree byte-for-byte with the source arrays above. So for indices 16..31 —
half of all draws — retail reads `coinEffBankTbl2` and the effect silently uses
that second table's bank numbers, including the `0` and `1` in its last two
entries, which never appear in `coinEffBankTbl`.

The adjacent `li r3,16` site proves this is not the file's general idiom: where
the original wanted an in-range index it wrote the table's real size. The `32` is
specific to this one site.

`board/coin.c` is `NonMatching`, so its object does not authenticate its own
source shape — but the instruction sequence, both symbol sizes, and both tables'
bytes above are read from retail and authenticate this construct independently of
the object's status.

**Do not** change the modulus to 16, mask the index, or merge the two tables.

## 2. `game/hsfmotion.c` + `game/hsfdraw.c`: uninitialized fields survive only by short-circuit

`SetObjAttrMotion` (`src/game/hsfmotion.c:1155`) allocates a fresh
`HU3D_ATTR_ANIM` and initialises only part of it:

```c
attrAnimP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3D_ATTR_ANIM), ...);
attrP->animWorkP = attrAnimP;
attrAnimP->attr = 0;
attrAnimP->trans3D.x = attrAnimP->trans3D.y = attrAnimP->trans3D.z = 0;
attrAnimP->rot.x = attrAnimP->rot.y = attrAnimP->rot.z = 0;
attrAnimP->scale3D.x = attrAnimP->scale3D.y = attrAnimP->scale3D.z = 1;
```

The struct (`include/game/hu3d.h:251`) also contains `animId`, `texScrId`,
`scale`, `trans`, `bitMapPtr` and `unk40`, none of which are written here. The
allocation is not zeroed, so `animId` and `bitMapPtr` hold whatever the heap
block last contained.

`FaceDraw` (`src/game/hsfdraw.c:461`) consumes it:

```c
animWorkP = attrP->animWorkP;
texAnimP = &Hu3DTexAnimData[animWorkP->animId];                    /* line 684 */
if((animWorkP->attr & HU3D_ATTRANIM_ATTR_ANIM2D)
   && !(texAnimP->attr & HU3D_ANIM_ATTR_NOUSE)) {                  /* line 685 */
```

Line 684 computes an **address** from the garbage `animId` — pointer arithmetic
only, never dereferenced. The single dereference, `texAnimP->attr`, sits on the
right of an `&&` whose left operand is the `HU3D_ATTRANIM_ATTR_ANIM2D` bit that
`SetObjAttrMotion` explicitly cleared. The short-circuit is what makes the
uninitialised field harmless; the same protection covers `bitMapPtr`, reached
only under `HU3D_ATTRANIM_ATTR_BMPANIM` at line 690.

Both `game/hsfmotion.c` and `game/hsfdraw.c` are `Object(Matching)`, so both
forms are retail-authenticated.

Two ways to break this while "improving" the code:

- adding the missing initialisers to `SetObjAttrMotion` — changes its emitted
  code and its object;
- reordering line 684 below the flag test, or splitting the `&&` into nested
  `if`s "for clarity" — changes `FaceDraw`'s emitted code.

The order of the two operands of that `&&` is load-bearing retail behaviour.

## 3. `board/telop.c`: `mbTelopCreate` is recovery debt, not a missing file

`src/board/telop.c` exists and defines the telop *time/taunt/language/board-data*
group (`mbTelopCheck`, `mbTelopTimeStarSet`, `mbTelopTimeTPLvlSet`,
`mbTelopTimeDispSet`, `mbTauntInit`, `mbTauntClose`, `mbLanguageGet/Set`,
`mbBoardDataDirRead`, ...). It does **not** define the telop banner itself.

`mbTelopCreate` has no definition anywhere in `src/`. Its only declaration is a
file-local `extern` in a consumer:

```c
src/board/opening.c:46   extern void mbTelopCreate(int playerNo, int telopNo, BOOL statF);
src/board/opening.c:256      mbTelopCreate(-1, boardNo + 16, FALSE);
src/board/opening.c:800      mbTelopCreate(-1, boardNo + 16, FALSE);
```

`board/opening.c` is `Object(Matching)`; `board/telop.c` is `NonMatching`.

Retail facts, verified:

```text
mbTelopCreate     = .text:0x802035D0   size 0x164
TelopInitOMExec   = .text:0x8020399C   size 0xC8   scope:local
TelopOMExec       = .text:0x80203A64   size 0x330  scope:local
telopFileTbl      = .rodata:0x8021AF30 size 0x6C   scope:local
```

`telopFileTbl` is 27 `u32` entries. Read from the retail DOL at `0x8021AF30`:

```text
0x8021AF30  0005004d 0005004e 0005004f 00050050
0x8021AF40  00050051 00050052 00050053 00050054
0x8021AF50  00050055 00050056 00050058 00050057
0x8021AF60  00050057 00050057 0005005a 00050059
0x8021AF70  00050044 00050045 00050046 00050047
0x8021AF80  00050048 00050049 0005004c 0005004b
0x8021AF90  0005004a 00050044 00050044
```

Entry `[16]` is `0x00050044`, matching the `boardNo + 16` call sites in
`opening.c`: board banners start at index 16, and indices 0..15 are the
non-board telops.

The three local functions and the table are the recoverable unit. Their
behaviour — sprite entry and any scale/alpha ramp — has **not** been
disassembled here and is deliberately not recorded as fact; only the addresses,
sizes, scopes and table contents above are verified.

## Consequence

Nothing is promoted to `main` from this investigation. The coin and hsf findings
are recorded as scoped exceptions so that a later agent does not "repair" them;
the telop finding is recorded as owner debt with its retail coordinates so the
next attempt starts from addresses rather than from a search.
