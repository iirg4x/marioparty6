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

Player residual pass 021 measured a fourth ordinary spelling in
`MoveNumOMExec`. Replacing the direct aggregate assignment with block-local
`HuVecF` source and destination pointers followed by
`*posP = *posNormP` still emitted three `lwz`/`stw` copies. The target/source
pair was `1152/1128` bytes at `93.996530%`; the target alone retained the
`psq_l`/`lfs` and `psq_st`/`stfs` transfer. The strict report is
`build/player-residual-closure-021/probe08-movenum-pointer-copy.json` in the
Player 021 recovery worktree, with SHA-256
`795ffcec15119e3e43125b896f5918a9f225101dc50cd51ff313c839b677002c`.
The probe was reverted. The analogous Player collision copy was not tested and
is not evidence for this result.

The SNPC batch also supplies a nearby counterexample to overgeneralization.
`mbObjFadeTexColorSet` becomes exactly `148/148` bytes through an independent
argument-promotion correction. The vector-copy negative therefore applies to a
proven paired-single transfer, not to every material helper or every `HuVecF`
consumer.

## Safe stopping rule

For this signature, run at most one direct aggregate-assignment, block-local
pointer-mediated aggregate-assignment, `HuCopyVecF`, and `memcpy` control under
the exact owner command. Once the applicable ordinary spellings select the
measured non-target mechanisms, preserve the reports and stop. Reopen the
function only when same-game source, a historical SDK/header, or a
compiler-backed helper or intrinsic boundary authenticates why GC/2.6 selected
paired-single operations.

The target opcodes alone do not authenticate a builtin name or source spelling.
Guessed intrinsics, cast or alignment tricks, fake storage, `volatile`, and
inline assembly are not supported solutions.

## Authenticated paired type and indexed-form control

The CodeWarrior Power Architecture Build Tools Reference Manual documents the
compiler type `__vec2x32float__` in section 36.4.32 and the associated
`#pragma vec2x32float_align_4 on | off | reset`. The manual states that the
pragma changes the type's alignment from its default to a four-byte boundary.
The contemporaneous CodeWarrior for Nintendo GameCube product sheet also
states that the compiler supports vectorized paired singles through C
intrinsics. These primary sources authenticate the type and alignment control,
but not a displacement-form load/store helper or a historical SNPC spelling:

- <https://www.nxp.com/docs/en/reference-manual/CWMCUPABTR.pdf>
- <https://www.nxp.com/docs/en/data-sheet/950-00116.pdf>

Pinned GC/2.6 probes then measured the authenticated type directly. Explicit
`__vec2x32float__` memory assignment selected `psq_lx`/`psq_stx`, both at the
owner's `-O0,p` profile and at `-O4,p`. Changing
`vec2x32float_align_4`, `gprfloatcopy`, or peephole optimization did not select
the target `psq_l`/`psq_st` displacement forms.

Staging each SNPC transfer as one `__vec2x32float__` pair plus its remaining
scalar reproduced the target floating-register count but remained nonexact:

| Function | Target/source bytes | Data-value score | Candidate transfer |
| --- | ---: | ---: | --- |
| `mbObjFadeCreate` | `332/328` | `92.277110%` | `psq_lx`, `lfs`, `psq_stx`, `stfs` |
| `mbObjFadeTexRotSet` | `200/192` | `74.140000%` | two indexed paired/scalar transfers |

The data-value report is
`build/snpc-fade-two-exact-104/probe-vec2-staged-zerohex-value.json` with
SHA-256
`19fa4ff48b34f2fddb918f4ee9ff90c2eefb9ccad3b5be3af432e275f00822bd`.
The probe caused no exact-function regression and was reverted. Authentication
of `__vec2x32float__` therefore narrows the search rather than closing it:
after this control selects indexed forms, stop until same-game source, a
historical header, or a compiler-backed helper authenticates the target
displacement-form boundary. Do not retain a cast, pragma, `volatile`, or inline
assembly merely because it exposes paired-single opcodes.
