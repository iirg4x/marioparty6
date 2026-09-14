# Local Qwen support: usable context and measured contribution

2026-09-14. Local support only; Astra remains the source integrator. This is
AI-workspace evidence, not a main progress or source-authenticity claim.

## Runtime and context

The installed launcher at `C:/Users/Anony/.codex/tools/qwen-support` now has
explicit retained-q8, native-q4, native-q8 and custom memory profiles. The live
native-q4 profile uses the existing Qwen3.8-27B Q4_K_M weights, q4_0 K/V cache,
four real inference slots, 262144 per-request tokens and 262144 **shared** KV
tokens. It is not four dedicated 262K caches. Cache quantization is an explicit
precision/memory tradeoff, not a claim of identical quality to q8.

The launcher checks GGUF metadata, available VRAM/RAM, process identity, selected
arguments, listener, slots, startup logs and observed KV allocation. The live
launch used 4608 MiB KV and 66/66 offloaded layers. No global model setting,
duplicate weights, YaRN scaling or autostart was installed. The local
`runtime-release.json` records the final runtime/test file hashes.

The runner renders the actual xhigh chat template and tokenizes that exact text
before admission. Four concurrent jobs explicitly reserve 65536 tokens each;
this is shared-pool admission, **not** a generation cap. Highest reasoning and
uncapped output remain enabled. Generation can still outgrow the reservation.
Status distinguishes queued work from actual streams. No hidden retries or
admission of failed/partial answers occurs.

Opt-in checkpoints retain hash-bound completed structured answers, not hidden
reasoning. Between completed rounds, old findings are compacted to fit the next
exactly tokenized request; the identity ledger and latest result survive.
This does not rescue a model stuck inside an unfinished reasoning stream.

## Correct inputs and actual usefulness

`recovery_causal_groups.decision_packet` accepts bounded `context_fragments`
containing actual header/source/data excerpts with full-file and excerpt hashes.
The model must distinguish real field widths and API signatures from m2c casts.
Do not pass an untyped m2c structure and ask Qwen to authenticate its storage.
Do not ask about a global absent from the selected machine rows.

The first live four-way acceptance batch completed all four jobs without context
exhaustion. Three answers correctly constrained array/sound-record layouts.
Those overlapped reconstruction already performed by Astra and are independent
confirmation, **not** newly discovered source gains.

The fourth answer proposed a used `s16` capture of `GwMgNightF` in m629
`fn_1_53C8`. Packet identity:
`df23d87cb80371c50d9192201b6edc2a9b2a0134dee646cee6ff46aa24762ad9`.
Its first compile changed 1256 to the target 1268 bytes, 97.567825 to 98.5489,
97 to 92 raw differences, with all 15 then-exact siblings preserved. This is a
new measured Qwen-proposed retained gain. Later whole-TU producers improved the
same function further; do not attribute those later gains to that suggestion.
Bound answers/metrics remain under
`build/minigame-recovery-20260913/m629/qwen-bound-context/`.

Validation: 68 Qwen packet/checkpoint/runtime tests passed with the real local
support directory enabled. This includes rendered-token admission, checkpoint
compaction, cached receipt validation, four-slot admission, process capacity and
actual Windows PowerShell profile replacement. It does not prove arbitrary
model answers correct; every proposed source change still needs compile/proof.

## m629 reconstruction boundary

All 24 application functions are reconstructed and compile together;
22 have zero raw objdiff rows in the latest private object comparison (up from
17). Restoring the missing typed data storage aligned literal owners and closed
`fn_1_1F90`, `fn_1_4290`, `fn_1_4368`, and `fn_1_444C` without editing those
bodies or losing an exact sibling. This is Astra's TU-layout reconstruction,
not a new Qwen-discovered gain. The two large bodies are 5092/5092 and
5012/5012 bytes; both now have zero raw differences.

The initialized-data and BSS section extents now match 534 and 2204 bytes.
Twenty-five nonpointer initializers (322 bytes) match their target values and
offsets; all reconstructed BSS symbols have their target offsets. Reversed BSS
definition chronology uses the already verified MWCC emission behavior, not
manual padding. The eight-byte timer owner is provisionally represented by two
typed pointer slots, and the twenty-byte state owner by five s32 elements; only
element zero is observed. The unreferenced short at data A8 and fourteen-byte
table at data 124 have target-backed storage/values, but their semantic role
and original declaration are unknown. Keep those explicit source-quality
limitations; exact data bytes alone do not authenticate the inferred types.

The actual private source-only link now includes reconstructed constructor/
destructor walks and the existing shared EABI runtime. It produces a 40000-byte
REL but is NOT retail-exact. The first full link had 8750 byte differences.
The retained exact-size null-test spelling subsequently removed the controller's
extra 12 bytes and reduced the full REL to 295 differing bytes: 293 inside
`fn_1_336C` and two relocation-stream bytes. Its local score fell from 99.178085
to 97.842470, but the real source-selected REL improved, no exact sibling was
lost, and section/data extents remain correct. Do not throw out this structural
gain on local score alone. `source-link/receipt.json` binds current sources and
both REL hashes. The other setup function's six raw rows are annotations: no
linked bytes differ there. Source-shape review and the final retail-identical
link remain required; no Matching gate or main progress advanced.

Keep explicit source-shape debt: target-observed unused captures, redundant
phase-dependent constants, an irregular nested switch case, and an unaccessed
four-byte player-record interval are not authenticated original spelling.
Do not replace them with padding/assembly or silently label them organic.

Useful measured constraints: m2c's reciprocal multiply can originate in a source
division; its folded 46 threshold was `(group ? 30 : 30) + 16`; its constant
camera weights came from a live named weight. Restoring these producer shapes
reduced the 5012-byte stage function to two string-owner rows. Folding named
Boolean conditions directly into their consumers and preserving chained input
assignments closed saved-register cascades. Standalone scalar aliases and
declaration swaps did not reproduce those source relationships.

The final `fn_1_9E8` arithmetic repair retained ordinary source
`rand8() % (variation * 2 + 1) + ((rand8() % rate != 0 ? 1 : 0)
+ interval) - variation`. The actual compiler still evaluates the conditional
call first as required by the target. m2c's rendered `interval + (bias + random)`
lost MWCC's association/operand-selection behavior: a first reassociation closed
six of seven rows, including two nonlocal register pairs; the composed tree
then closed the final add operand. Do not infer evaluation order from printed
C left-to-right or generalize the rewrite to arbitrary expressions. Acceptance
must preserve the target call order, arithmetic, and siblings. Null-test
cast/operand permutations and an explicit data-pointer cast did not solve the
remaining controller allocator cycle; preserve that constraint without banning
the function or guessing a new type solely for register colors.

The setup/float-pool gains are not whole-module closures. Reuse the latest
`m629-report.json`, `latest-compile.json` and canonical source, and continue from
their current hashes rather than replaying this historical snapshot.

## Closure lessons being implemented in parallel

Two local Qwen jobs under `build/rel-gap-fixes/qwen-closure-lessons/` own concrete
patch proposals: narrow-call conversion boundaries in existing m2c, and opt-in
grouped translation/first compilation in the existing batch runner. No paid
helper was spawned. Their answers are not installed improvements until reviewed,
tested, and exercised on m621's narrow-index/grouped-pool acceptance cases.
Do not report pending responses as tested code or new matching credit.
