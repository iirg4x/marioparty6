# Blind retail evidence: `ExitWatch`

## Trial contract

- Recover one natural C definition for `static void ExitWatch(void)`.
- Do not use pragmas, forced registers, inline assembly, dead branches, fake storage, arbitrary `volatile`, cast ladders, or source-shape tricks.
- Change one evidenced source-shape dimension per compile attempt.
- Maximum eight compiler attempts.
- Do not browse outside the packet directory or inspect repository/history/retained source.
- Return `candidate.c`, candidate assembly, and an attempt log.

## Retail identity

- Game/revision: Mario Party 6, GP6E01, VERSION=0.
- Target virtual bounds: `0x8014A9D0..0x8014AAFC`.
- Target object bounds: `.text+0x64..+0x190`.
- Size: 300 bytes / 75 instructions.
- Target owner: `main:board/exit`, target object `.text=0x1B8`, `.sbss=0x10`.
- Fresh retained-object proof before sealing: `ExitWatch` exact; all 6 owner functions exact; retained relocations `39/39` exact.

## ABI and public declarations

Use 32-bit PowerPC ABI and these exact evidence-backed declarations in the compile harness. Names beginning `unk` are intentionally not semantic claims.

```c
typedef signed short s16;
typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;

typedef struct ExitObjEntry {
    u16 stat;
    u8 unk02[0x5E];
} ExitObjEntry; /* size 0x60 */

typedef struct ExitObjList {
    s16 count;
    u8 unk02[0x0A];
    ExitObjEntry *entries; /* +0x0C */
} ExitObjList;

typedef struct ExitObjManager {
    u8 unk00[0x12C];
    ExitObjList *list; /* +0x12C */
} ExitObjManager;

extern s16 omSysExitReq;
extern ExitObjManager *mbObjMan;

extern s32 mbPauseStartCheck(void);
extern void mbPauseCreate(s32 pauseNo);
extern void HuPrcVSleep(void);
extern u8 WipeCheck(void);
extern s32 HuARDMACheck(void);
extern void HuPrcResetStat(ExitObjManager *manager, s32 stat);
extern void omResetStatBit(ExitObjEntry *entry, u16 stat);
extern void mbPauseEnableReset(void);
extern void omSysPauseCtrl(s32 pause);
extern void HuPrcAllPause(s32 pause);
extern void Hu3DPauseSet(s32 pause);
extern void HuSprPauseSet(s32 pause);
extern void HuPrcKill(ExitObjManager *manager);
extern void HuPrcEnd(void);
```

The harness owns these target-local `.sbss` declarations in order. Only `exitReq` and the first word of `exitFlag` are accessed by the target function; `exitWatchProc` is included to reproduce same-owner storage chronology.

```c
static void *exitWatchProc;
static s32 exitReq;
static s32 exitFlag[2];
```

The harness forward-declares `ExitWatch` and retains its address in a file-scope function pointer so MWCC emits the local function. This is harness-only reachability, not candidate source:

```c
static void ExitWatch(void);
static void (*blindExitWatchReference)(void) = ExitWatch;
```

## Target control/data evidence

- Entry saves `r29`, `r30`, `r31`; there are three persistent locals.
- Outer wait condition reads 32-bit `exitReq`; when zero it reads signed 16-bit `omSysExitReq`.
- While both exit requests are zero, call `mbPauseStartCheck`; a nonnegative result is passed to `mbPauseCreate`; every iteration then calls `HuPrcVSleep`.
- On exit request, store integer `1` to the first word of `exitFlag`.
- Sleep while the unsigned-byte return of `WipeCheck` is nonzero.
- Then sleep while the signed-int return of `HuARDMACheck` is nonzero.
- Call `HuPrcResetStat(mbObjMan, 2)`.
- Load `mbObjMan->list`. Iterate signed index `0` through `list->count - 1`.
- Entries are stride `0x60`; read 16-bit `entry.stat`. If `(stat & 0x21) == 0`, call `omResetStatBit(&entry, 0x10)`.
- After the loop, call `mbPauseEnableReset`, sleep once, then pass `0` to `omSysPauseCtrl`, `HuPrcAllPause`, `Hu3DPauseSet`, and `HuSprPauseSet` in that order.
- Call `HuPrcKill(mbObjMan)`, then `HuPrcEnd`.
- The retail binary retains an ordinary epilogue after `HuPrcEnd`; do not infer `noreturn`.

## Relocations

The function has 23 relocation-bearing entries:

- `R_PPC_EMB_SDA21`: `exitReq`, `omSysExitReq`, `exitFlag`, `mbObjMan` (three accesses).
- `R_PPC_REL24`: `mbPauseStartCheck`, `mbPauseCreate`, `HuPrcVSleep` (four call sites), `WipeCheck`, `HuARDMACheck`, `HuPrcResetStat`, `omResetStatBit`, `mbPauseEnableReset`, `omSysPauseCtrl`, `HuPrcAllPause`, `Hu3DPauseSet`, `HuSprPauseSet`, `HuPrcKill`, `HuPrcEnd`.

The direct same-owner creator materializes `ExitWatch` as a process callback through exact HA/LO relocations. No other direct caller exists.

## Compiler/toolchain

- Configured compiler: `GC/2.6/mwcceppc.exe`.
- Compiler reports: Metrowerks Embedded PowerPC C/C++ 2.4.7 build 107, runtime July 14 2003.
- Target `.comment`: compiler `2.4.7.1`.
- Compiler SHA-256: `316e2a98236c23f3fc902243b157eaebf8ef2ad6edb88cfd632a15b6676fa9a8`.
- DTK: 0.9.2 commit `4d039140f2d2ed80572b1949b76a5ff9b3094e06`.
- Effective flags:

```text
-nodefaults -proc gekko -align powerpc -enum int -fp hardware
-Cpp_exceptions off -O4,p -inline auto -pragma "cats off"
-pragma "warn_notinlined off" -maxerrors 1 -nosyspath -RTTI off
-fp_contract on -str reuse -multibyte -DMUSY_TARGET=MUSY_TARGET_DOLPHIN
-DVERSION=0 -DNDEBUG=1 -O0,p -char unsigned -fp_contract off
```

Later flags win: effective optimization is `-O0,p`, `char` is unsigned, and FP contraction is off.

No compiler-wide knowledge card was selected: the current cards are scoped to `GC/1.3.2`, not this `GC/2.6` owner.

## Acceptance

- Candidate is frozen and SHA-256 recorded before retained-source reveal.
- Candidate function instruction bytes and all function relocations are exact under the pinned retail compiler.
- Candidate remains natural and organic with score at least 85/100.
- At most eight compile attempts.
- No repository/source/history leakage.
