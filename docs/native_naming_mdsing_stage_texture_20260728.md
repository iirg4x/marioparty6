# mdsing stage-texture naming evidence (2026-07-28)

This record separates stable target identity from proposed semantic names for
the non-minigame `mdsingdll` application owner. The proposals are metadata
only: source still uses `fn_1_*` and `lbl_1_*`, the application object is not a
complete recovered owner, and no retail or main-promotion claim is made.

## Owner and target boundary

Repository catalog, split, symbol, and game-consumer evidence classifies
`mdsingdll` as an eligible non-minigame, non-board application owner. Its target
application text is `0x00000000..0x00035300` with 287 functions and 217,856
bytes. The separate shared runtime at `0x00035300..0x00035D44` is already exact.

The audited stage-texture cluster is target text `0x2CF38..0x2DCD0`. Its source
reconstruction was reverted because normal project headers emit 16 bytes of
unreferenced `sqrtf` constants absent from the target; prohibited header
suppression and pool pragmas were not retained. That source-shape barrier does
not erase the target call, hook-assignment, API-role, symbol-extent, and dataflow
evidence used for these proposed names.

## Proposed function names

- `mdsingdll:0x2D170` / `fn_1_2D170` ->
  `MDSingStageTextureCopyLayerHook`: `fn_1_2D75C`, `fn_1_2DC10`, and
  `fn_1_33FC0` assign it through `Hu3DLayerHookSet`; the body performs the
  authenticated `GXSetTexCopySrc`, `GXSetTexCopyDst`, and `GXCopyTex` sequence.
- `mdsingdll:0x2D1DC` / `fn_1_2D1DC` ->
  `MDSingStageTextureMaterialHook`: `fn_1_2D850` installs it through
  `Hu3DModelMatHookSet`; the body configures capture sampling, indirect warp,
  texture matrices, and the final texture-combine stage.
- `mdsingdll:0x2D850` / `fn_1_2D850` ->
  `MDSingStageTextureObjectInit`: `fn_1_2DAE4` and `fn_1_33264` pass it to
  `omAddObjEx`; it allocates the capture texture and animations, creates and
  configures the model, installs the material hook, and binds model work.

All three are `proposed` with `high` confidence. No exported/debug symbol
authenticates the original spelling, so they are not `accepted`.

## Proposed global names

- `lbl_1_bss_13B8` -> `MDSingStageObjectManager`: owning manager in all
  authenticated `omAddObjEx` stage-texture creation paths.
- `lbl_1_bss_13BC` -> `MDSingStageTextureObject`: stores the created object and
  feeds the model, effect-start, cleanup, and destroy paths.
- `lbl_1_bss_13C0` -> `MDSingStageIndirectTextureAnim`: animation bound to
  `GX_TEXMAP2`, then selected by `GXSetIndTexOrder`.
- `lbl_1_bss_13C4` -> `MDSingStageBlendTextureAnim`: animation bound to
  `GX_TEXMAP3`, then consumed by the final texture-combine stage.
- `lbl_1_bss_13C8` -> `MDSingStageCaptureTextureBuffer`: allocated to the exact
  RGB565 copy-buffer size, written by `GXCopyTex`, sampled by the material hook,
  and freed by cleanup.
- `lbl_1_bss_1428` -> `MDSingStageTextureWork`: target extent `0x50` with two
  complete `0x28` records; consumers cover the indirect matrix, color, offset,
  step, and remaining-time fields.

Each remains `proposed` with `high` confidence. The stable target identity
is authoritative even if later consumer evidence refines the semantic spelling.

## Deferred names

The update callback, effect-start/cleanup wrappers, and four generic
interpolation helpers have plausible roles, but their spelling is less
distinctive. They remain outside the naming ledger until another target caller,
same-game owner, or accepted subsystem vocabulary raises confidence. Binary
similarity and sibling source alone are not naming proof.
