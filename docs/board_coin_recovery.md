# Board Coin recovery — 2026-09-11

Owner: `main:board/coin`; canonical source: `src/board/coin.c`.
This records completed source-selected retail proof. Clean-main promotion is
tracked separately from the object and link results.

## Retained proof

The retained `c49` candidate improves strict/data exactness from **37/52 to
52/52**, recovering 15 functions without losing the exact siblings. Independent
physical comparison reports zero differing bytes in all 52 functions.

- Source SHA-256: `b0a21fadb1fde90bc0bc7cb2c2d82096cf0f302d53c7a949f3bacd0a71cc1e72`.
- Object SHA-256: `eea59832411a0bd5314c1b9e11251474775f3dc569ba27eb07181257ca58d2dc`.
- Candidate, evaluation and compiler receipt: `build/coin-recovery/retained-c49/`.

Final source-selected linking passed in a detached current-main proof tree:
52/52 strict/data functions, all 655 allocated relocations, and byte-identical
main.dol plus all 136 configured RELs. The link explicitly selects compiled
Coin and Math objects. Main DOL SHA-1 is
`b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.
`build/coin-recovery/final-proof.json` has SHA-256
`3d3c8e3ab00c3d51d5f6b4410ac374c1f8f2d8e0ac5f09b6d9c264bae9d21689`.
The seven final SDA21 differences were corrected by restoring the three
static definitions' target emission order. Short .sdata/.sbss/.sdata2 tails
are zero linker-alignment bytes, not source padding objects.
The native boundaries below are disclosed exceptions, not pure-C-only
matching or proof of original source tokens.

## Native operation boundary

The user authorized three tiny math kernels with readable C fallbacks:

- `CoinVecCopy`: paired-single XY plus scalar Z copy.
- `CoinAbsFloat`: one `fabs` operation.
- `CoinMtxTranslationSet`: three position loads and three matrix translation
  stores. This macro explicitly uses fixed `fp4`, `fp5`, and `fp6`, and binds
  each real input once in matrix-then-position order.

Only these contracts and their reviewed consumers are covered by
`coin-target-native-operation-contracts` in `config/recovery/exceptions.json`.
No gameplay assembly, unrelated register control, or blanket native permission
is admitted. Dice is supporting precedent for copy/absolute-value operations;
its no-fixed-physical-register wording is not a claim about the translation
kernel here. Further native changes require renewed review.

## Causal reconstruction lessons

`CoinDisp` closed through source structure: separate the allocation core from
the public motion wrapper, recover the inline-depth groups at `4c`/`48`/`44`,
retain the direct ternary, and do not introduce a parent-return alias. The
useful structural step moved size from 2,172 to 2,188 bytes while mismatch rows
temporarily increased from 169 to 176; it was retained as evidence toward the
target constraint, not mislabeled an instruction-count improvement. The final
candidate has zero mismatch rows.

The existing evaluator received a narrowly scoped, tested retention correction
so this structural gain was not discarded solely because rows increased. Its
acceptance exercise replayed the already identified gain; tooling did not
discover the source change. Keep size/structure progress distinct from strict
exactness, and retain exact siblings throughout.

An unused zero scalar in the effect path is retail-proven. Its semantic name
remains unknown: the zero value and storage evidence do not justify inventing
a gameplay meaning. The existing `coin-effect-bank-table-retail-overread`
constraint remains unchanged with `rules: []`; it suppresses no quality rule.

## Math consumer correction and promotion boundary

The associated `src/board/math.c` change consistently corrects the
`mbMtxScaleRotXDeg` argument order in its definition and its `ObjectCullHook`
call. Relative to current clean main these are two lines; the whole Math
object remains identical. Its pre-existing native bodies are not a new Coin
exception.

The exact `board-math-sdk-native-kernels` policy record is carried from Math
commit `ad55acbd918bda3f6c01c4b4debb83cb3081b18b`. Its historical Math link
claim belongs to that earlier recovery; the Coin link is independently proved. No new
native Math permission is added here.

Policy and this note stay on the AI workspace branch. The policy must be in
the exact verified source commit before a fresh source-only `recovery/*`
promotion; only explicitly selected canonical source/header blobs cross to
the clean branch. Re-run final source/consumer/link proof after any source
change, and do not substitute a policy exception for binary verification.
