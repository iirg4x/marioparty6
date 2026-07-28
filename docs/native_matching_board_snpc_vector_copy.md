# Board paired-single vector-copy negative evidence

This note records a bounded negative result for 12-byte `HuVecF` transfers under
the pinned GC/2.6 board profile. It does not recover or name the original source
construct. The target instructions are authoritative; no retained sibling or
historical header currently authenticates the missing paired-single boundary.

## Repeated target signature

The target copies one vector in `mbObjFadeCreate` and two vectors in
`mbObjFadeTexRotSet` with `psq_l`/`psq_st` for the first two floats and
`lfs`/`stfs` for the third. The completed SNPC sweep measured three ordinary C
spellings from source commit `e34582844685c6d8349a9cc4a75100a80c9cf80c`:

| Function | Probe | Target/source bytes | Strict score | Emitted copy shape |
| --- | --- | ---: | ---: | --- |
| `mbObjFadeCreate` | aggregate assignment | `332/300` | `82.289154%` | three `lwz`/`stw` word copies |
| `mbObjFadeCreate` | `HuCopyVecF` | `332/300` | `77.156624%` | three `lfs`/`stfs` scalar copies |
| `mbObjFadeCreate` | `memcpy` | `332/292` | `84.409640%` | target-absent `memcpy` call |
| `mbObjFadeTexRotSet` | aggregate assignment | `200/136` | `52.340000%` | two three-word integer copies |
| `mbObjFadeTexRotSet` | `HuCopyVecF` | `200/136` | `54.420000%` | two scalar-float copy groups |
| `mbObjFadeTexRotSet` | `memcpy` | `200/128` | `36.820000%` | two target-absent `memcpy` calls |

The retained aggregate report is
`build/snpc-bulk-exact-001/final-source-strict.json` with SHA-256
`84f1363d66f2a7aa82da73f7ee8117175197ea3844a23f78582e901a02f71123`.
The `HuCopyVecF` report is
`build/snpc-bulk-exact-001/batch-hucopyvec-strict.json` with SHA-256
`fdd978578ad59612e344de9404c9c03d844476b0c23ef4e716ef5e7dc7def185`.
The `memcpy` report is
`build/snpc-bulk-exact-001/probe-memcpy-vec-strict.json` with SHA-256
`5aabd766b994cf36cd6009ac507da2f70017b18eacc6f01d9f1faa7fc21ca2b9`.
All three reports pair 24 functions and retain 15 exact functions totaling
`0x7B0` target text bytes. The probes were reverted; the aggregate assignment
remains the natural unresolved source.

The percentage movement is a counterexample to score-driven retention.
`memcpy` raises `mbObjFadeCreate` from `82.289154%` to `84.409640%`, but it
shrinks the source by eight more bytes and introduces a call absent from the
target. `HuCopyVecF` modestly raises `mbObjFadeTexRotSet`, yet its scalar loads
and stores still do not resemble either target paired-single transfer.

## Independent repetitions

[`native_matching_wave65.md`](native_matching_wave65.md) preserves the same
negative result for `mbDiceSNpcNumPosSet`, `mbDiceSNpcNumOfsSet`, and
`mbDiceSNpcNumOfsGet`. Each target/source pair is `0x14/0x1C` at `0%`:
aggregate assignment emits integer copies, while `HuCopyVecF` emits scalar
float copies instead of the target paired-single sequence.

[`native_matching_wave93.md`](native_matching_wave93.md) records a second owner
family: three then-missing `config.c` panel helpers rejected aggregate
assignment, `HuCopyVecF`, and `memcpy` for the same paired-single reason. That
historical run did not retain per-function scores, so this note does not invent
them or elevate those temporary probes to recovered source.

The SNPC batch also supplies a nearby counterexample to overgeneralization.
`mbObjFadeTexColorSet` becomes exactly `148/148` bytes through an independent
argument-promotion correction. The vector-copy negative therefore applies to a
proven paired-single transfer, not to every material helper or every `HuVecF`
consumer.

## Safe stopping rule

For this signature, run at most one aggregate-assignment, `HuCopyVecF`, and
`memcpy` control under the exact owner command. If all three select the measured
non-target mechanisms, preserve the reports and stop. Reopen the function only
when same-game source, a historical SDK/header, or a compiler-backed helper or
intrinsic boundary authenticates why GC/2.6 selected paired-single operations.

The target opcodes alone do not authenticate a builtin name or source spelling.
Guessed intrinsics, cast or alignment tricks, fake storage, `volatile`, and
inline assembly are not supported solutions.
