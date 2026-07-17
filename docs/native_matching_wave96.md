# Native Matching Wave 96 -- `mdparty.c` shared flow recovery

Wave 96 recovers six real shared helper boundaries, the target-matching
direct-truth condition shape, two assignment-condition expressions, a float
result lifetime, two local chronologies, and one target-proven control-flow
scope. Seventeen application functions and `0xE4A0` target text bytes become
exact. The application owner remains `NonMatching`: 248 of 258 functions and
`0x3A600 / 0x3F424` target text bytes are now exact, leaving 10 functions and
`0x4E24` bytes as compiler-shape work.

## Recovered shared helper

`fn_1_2EF24` was already an exact, otherwise unused `0x308` function:

```c
s16 fn_1_2EF24(void)
{
    fn_1_52B0();
    fn_1_4020(2, 0xD0008, 1);
    return fn_1_3F28(2);
}
```

Thirteen caller sites manually repeated that exact sequence. A late
`inline s16 fn_1_2EF24(void);` declaration restores the shared source boundary
without removing the standalone target symbol. Fully Matching
`src/REL/mdseldll/mdsel.c` independently authenticates the source idiom: its
analogous exact `fn_1_B804` helper has the same plain-definition/late-inline
shape and its caller tests the result through direct truthiness.

That condition spelling is byte-relevant. `if (fn_1_2EF24())` and
`if (!fn_1_2EF24())` reproduce the target. Explicit `!= 0` or `== 0` around
the inline call creates an additional halfword result object. Changing the
helper return to `s32` creates a word result object and also removes the
standalone target's final `extsh`. Both alternatives were rejected.

The retained helper calls preserve the exact operation order and branch sense:

- nonzero continues the surrounding retry loop;
- zero selects the existing `result = -1`/break path.

## Recovered insert-message helper

The exact `0x70` `fn_1_39AC` function was also present but unused. Its two
calls clear a window home position and insert a message. `fn_1_4258` manually
duplicated that body after staging `activeWin` and `insertedMess` locals.
Restoring a late inline declaration and the natural helper call removes those
duplicate locals:

```c
fn_1_39AC(lbl_1_data_DA2[0], lbl_1_data_DA8[1], insertPos);
```

The helper and exact `fn_1_4258` standalone bodies remain byte-identical. The
additional inline depth reproduces the target result-object chronology in four
callers and closes all four without a stack-slot force or manual body.
`fn_1_38314`, another already non-exact nested caller, changes from
`97.368996%` to `96.174675%`; its target/source sizes remain equal at `0x728`
and its aligned difference count falls from 113 to 111. No exact function
regresses. The helper boundary is retained because the target contains the
exact otherwise-unused helper, its body is the exact duplicated sequence, and
four independent callers become byte-identical.

## Recovered nested-inline boundaries

`fn_1_34D10` and `fn_1_36264` each contained four manually expanded copies of
the exact `fn_1_D6B0` object-reset helper. Restoring the calls while preserving
the helpers' plain definitions and late inline declarations keeps both
standalone functions exact at `0x2B4` each. It also restores MWCC's nested
inline-function data in `fn_1_3EAC8`, improving that state machine from
`44.786312%` to `99.791320%`. Its target/source sizes are now `0x95C/0x958`,
with six tail-only instruction differences remaining.

The alternative definition-time-inline form removed required standalone
symbols. Changing `fn_1_3AD84` to return `s32` regressed both its exact
standalone body and the state machine. Casts, folded assignment conditions,
and inverted tests were neutral or worse. All of those probes are absent.

Three other exact, already late-inline helpers replace duplicated bodies:

- `fn_1_884C` restores the sprite-selection boundary in `fn_1_156B4`,
  `fn_1_160A0`, and `fn_1_18384`;
- `fn_1_8C28` restores the paired animation setup in `fn_1_18AA4`;
- `fn_1_9804` restores the window/sprite hide sequence in `fn_1_38314`.

Every helper and its formerly exact caller remain exact. These boundaries
close `fn_1_18384` and `fn_1_18AA4`, while materially reducing the remaining
register-allocation residues in the other three callers.

## Recovered float lifetime and state scope

`fn_1_22A8` now gives the result of `fn_1_1868` a real `float value` lifetime
before passing it to `fn_1_1404`. The target's extra floating-point move and
the associated vector spill occur in all three inline callers; the recovered
lifetime closes `fn_1_5324` and `fn_1_6324` and improves `fn_1_22A8` itself.

In `fn_1_737C`, the target branch for `work->state != 1` jumps directly to the
loop increment. That proves the model-position/effect update belongs inside
the `work->state == 1` block. Restoring that scope closes the final instruction
difference and makes the `0x584` function exact.

## Exact closures

The shared helper closes seven callers:

- `fn_1_30B4C` -- `0x9BC`
- `fn_1_31AC0` -- `0xE44`
- `fn_1_329A4` -- `0xC4C`
- `fn_1_36518` -- `0xD48`
- `fn_1_3736C` -- `0xFA8`
- `fn_1_38A3C` -- `0x1224`
- `fn_1_3CFE4` -- `0x1AE4`

`fn_1_3BEA8` contributes the eighth closure (`0xBC8`). Its two target result
lifetimes are assignment conditions, not separate assignment statements:

```c
if ((result = fn_1_3F28(2)) == 0) {
```

The recovered expression removes the three-way saved-register cycle while
leaving the emitted operations, control flow, and relocations unchanged.

The recovered `fn_1_39AC` boundary closes four more callers:

- `fn_1_33724` -- `0x15EC`
- `fn_1_350F8` -- `0x116C`
- `fn_1_39C60` -- `0x1124`
- `fn_1_3AD84` -- `0x1124`

The later helper, lifetime, and control-flow recoveries close five more:

- `fn_1_5324` -- `0x57C`
- `fn_1_6324` -- `0x580`
- `fn_1_737C` -- `0x584`
- `fn_1_18384` -- `0x68C`
- `fn_1_18AA4` -- `0x9E8`

## Retained improvements

The remaining non-exact retry-helper caller improves and no caller regresses:

- `fn_1_2F22C`: `96.559700%` to `96.641790%`;

`fn_1_33724` and `fn_1_350F8` also recover two source chronology facts directly
visible in the target. Their initialized locals are emitted in
`result, activeNo, firstNo, maxNo, count` order, and the equality loads
`firstNo` before `activeNo`. Reordering those declarations and writing
`firstNo == activeNo` accounts for the target prologue and comparison operand
order without a fake lifetime, cast, pragma, or register force.

The retained non-exact improvements are:

- `fn_1_22A8`: `99.548740%` to `99.693140%`;
- `fn_1_156B4`: `99.952760%` to `99.968506%`;
- `fn_1_38314`: `96.174675%` to `98.930130%` after the complete helper pass;
- `fn_1_3EAC8`: `44.786312%` to `99.791320%`.

`fn_1_160A0` remains at `99.968506%`; its exact duplicated body was still
replaced by the authenticated `fn_1_884C` helper. No exact function regresses.

## Name provenance and tool use

This wave retains address names because no retail symbol, string, map, DWARF,
or authenticated exact-code corpus proves game-local identifier spelling.
Sibling semantic names are not treated as extracted originals.

The full MP5 history makes that distinction concrete. Commit
`071ec3185c349608bfbf2b5a5064493abf618658` adds the `mdpartyDll` sources and,
in the same commit, replaces 155 address labels in its REL symbol file with
semantic labels. The parent revision still has `fn_1_*` entries and contains
no cited map or debug-symbol import. Those labels are useful behavioral
reconstructions, but the available repository evidence does not authenticate
them as original Hudson spelling.

The local recovery toolchain now also includes `m2c` for cold PowerPC/MWCC
structure and the GC/1.3.2 build of `mwcc-inspector` for candidate-source AST
inspection. Neither tool recovers stripped original identifiers: names may be
authenticated only by surviving metadata or transferred from a
provenance-bearing exact-code corpus. The pinned compiler used for this wave is
the project `20240706` GC/1.3.2 binary, SHA-256
`7cbae0a5bd81e07d7fa8975bbc4e969b5dea265cc29c5ee6bae0453a6e25f225`.

## Verification

The final relocation-aware object report is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave96_final_object.json`

It is generated with:

```powershell
rtk ninja build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave96_final_object.json `
  --format json -c functionRelocDiffs=data_value
```

The report proves 248 exact functions and `0x3A600` exact target text bytes.
The 10 remaining functions account for `0x4E24` target bytes. The final
serialized `rtk ninja -j1` build and the explicit DTK checksum each report
`137 files OK`. Separate `cmp` gates prove both `main.dol` and
`mdpartydll.rel` byte-identical, and the final build leaves
`config/GP6E01/symbols.txt` unchanged.
