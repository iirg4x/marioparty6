# Single Koopa result recovery — 2026-09-12

The final c95 source makes all 58 Single functions instruction-exact in the
data-relocation view. KoopaEnd is 6116/6116 bytes, with zero canonical relocation
differences and no loss among its 57 siblings. Five functions still have raw
strict symbol-attribution rows; their aggregate initializers require effective
relocation and linked-byte verification, not synthetic symbol substitutions.

## Source cause

The last register cascade was coupled caller context, not local register order.
`GWSingleMgFlagSet` leaves the normalized minigame index in `r3` on every retail
return path. Reconstructing its declaration and implementation consistently as
`int`, explicitly returning that index on both paths, emits the same 18 provider
instructions. All four Single callers discard the value. This is a defined,
machine-supported reconstruction, not a claim to have recovered the original
header spelling or a success/status result.

Together with that interface, restore the live assignment inside the unlock
condition and use uniform truthful file-scope void wipe prototypes. This removes
the prior function-local declaration workaround and preserves MKoopaEnd too.
Changing the wipe prototypes alone had regressed that sibling: the coupled
context is essential. No new assembly was introduced; the previously approved
one-instruction fabs helper with C fallback remains.

## Useful compiler diagnostic

The existing `mwcc_temp_pool_reuse.py --window-alignment` now tests bounded
hypothetical paired boundaries, not only a shifted suffix. On c65, a +1 shift
inside the first temporary-pool epoch reduces 31 conflicting target-role classes
to one. Four displayed bands are [181,245), [181,246), [184,245), [184,246).
There are eight tied fits across all searched epochs. This does not recover
target virtual IDs, establish source authority, or select an ABI automatically.

The useful span runs from the unlock/flag-set boundary toward the case-1 wipe.
That focused an independent callee return-dataflow check, which justified the
consistent C interface. The new implementation replays this known signal; the
source decision still came from compiler and retail semantics. The replay takes
about 0.8 seconds for 52,785 bounded diagnostic cells; 26 focused tests pass.

## Bound artifacts

- Source commit: `745e602` (three source/header paths only).
- Single source SHA-256: `d8fbad86190a8ac052b1d5a07081873d9e0f97b0a4fa39bcb3bfd4cd07ceb375`.
- Single object SHA-256: `1afcbb2bc9a7a705262d3e9ca4eefaa85eda9b90bca4412fa8637e3f12896fa8`.
- Provider source SHA-256: `83803ece29fe210b5cd6570cebd784b5106337348cad5b3b4644b9405f2888f2`.
- Header SHA-256: `8909041b5ac67778af089ada1f196269447fd9c6b6cdf4a0e0b9b381d3598126`.
- Normal, non-override compiler reproduction: `build/single-recovery/c95-normal/`.
- Fresh frontier: `build/single-recovery/current-c95.json`.
- Strict/data proof: `build/single-recovery/c95/.evaluate-nsw8p5a0/`.

## Source-selected retail proof

The isolated full build selects the compiled Single and gamework objects, not
extracted target fallbacks. Single is 58/58 raw instruction and physical exact;
gamework is 54/54. Single's allocated nontext bytes match and its 2261 text plus
21 data relocation entries are canonically exact. The twelve raw strict rows
across five functions are aggregate atom names, not differing effective operands.

DTK checksum and an additional direct byte comparison both pass all 137 retail
containers (DOL plus 136 RELs). DOL SHA-256 is
`172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.
The proof checkout retains the already-verified Masu source; its only subsequent
changes are the three selected Single/provider source/header files and Single's
Matching configuration flag.

- `build/single-recovery/final-link-c95.json`, SHA-256
  `93e5a7568ddf431d48c0be13ae1fc245600d2dfb5ea5a80f5ce365e5e8db7aac`.
- `build/single-recovery/final-direct-c95.json`: all 137 outputs directly compared
  with extracted retail files; comparison census SHA-256
  `958de21f4f570cf504ce8de7389c8027bcc71fe82a85c6dd7a4cf9f708935cfa`.

Clean-main source and Matching/progress promotion remain a separate boundary.

The full-file promotion review additionally required replacing the provider's
inherited numeric literals with the existing SDK `SI_GBA` name and named bitset
operations. The final provider source is
`3bcb10efd37b5bd250604be1734bc4ef78578585e388f892ee1bbb88a555cf70`;
`final-link-c96.json` records its independent no-code-change verification.
The unchanged `GW_COMMON.unk5C8[8]` remains explicitly unresolved save-record
storage, with no invented semantic name or layout change.
