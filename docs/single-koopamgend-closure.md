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

Object proof is complete. Source-selected whole-game link and clean-main
promotion are separate gates and are recorded in the final closure receipt.
