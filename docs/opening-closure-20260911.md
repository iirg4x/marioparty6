# Board Opening closure

## Verified reconstruction

Retained c34 closes all 22 retail function bodies in `src/board/opening.c`.
The source-selected diagnostic link matches all 13 allocated ELF sections,
all 22 final function bodies, and all 669 physical relocation sites
(304 REL24, 60 HA, 60 LO, and 245 SDA21). Its 2,416,640-byte DOL is
byte-identical to retail:

- SHA-1: `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`
- SHA-256: `172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`
- Source SHA-256: `cb53e139ee82f532e940972a47b016d4e7c3c0a2580f5d1611c1acebdf0637e6`
- Source object SHA-256: `d91cca123fdba54e8884dcd0d925a9a3ee4fdd24184904a20291548e25e76193`
- Corrected target object SHA-256: `d01857b44f09790999287a226fc11a688a225f34723f5c2b1ed897eb72f43fd8`

Object reports are `build/opening-recovery/c34/final-{strict,data}.json`;
the compact current snapshot is `build/opening-recovery/current.json`.
Linked evidence is under `build/opening-recovery/link-c34/`, including
`inspection.json` and `dol-comparison.json`. These are evidence paths, not
committed binary artifacts. Promotion is not implied by this local proof.

## Source fidelity and linker behavior

Publication c35 source SHA-256 is
`18b4192309acea103f844a08ad0fe2c7835632edfc9ffe679483d1b92d49d716`;
its source object is
`84ed7b252b7a3d43feeac25bcedad74cfce99e7bcc9ac3094c50b0c6726de0a3`.
Fresh canonical-main compilation reproduces that entire object exactly.
`build/opening-recovery/c35/final-{strict,data}.json` remains 22/22 exact.
The source-selected `link-c35/` verification reproduces the same 13 allocated
sections, 22 final function bodies, 669 relocation sites, and retail DOL.
Only ELF symbol/string metadata changes from c34 after the source cleanup.
Final c36 removes the obsolete `_MATH_H` override: the current headers already
respect `dolphin/math.h` coexistence. The entire object remains byte-identical
to c35. Final source SHA-256 is
`b1adc0586d41b6186567346cf3b14f4b85668e2501ae5fd83dec8cab97a8d957`.

This reconstruction uses ordinary C; no inline assembly, fake storage,
padding declarations, forced registers, or code-generation pragmas were added.
Curve integration, Newton iteration, and length helpers are ordinary static
functions in their target-backed definition order. MWCC auto-inlines their
uses; the linker strips their unused out-of-line bodies while retaining the
correct pooled constants.

The former 120-byte `singleGuidePosTbl` target symbol combined three genuine
source objects. Consumer and section-base evidence separates a 48-byte
position table, a 12-byte camera rotation vector, and a 60-byte path-offset
table. The supporting symbol change describes these boundaries without
changing any retail data byte or allocated address.

The compiler emits 220 bytes of `.sdata2` payload and the linker supplies the
four trailing alignment bytes, matching retail's 224-byte extent. Similarly,
the two-byte initialized model ID receives the verified linker alignment.
These exceptions are proven by allocated-section and final-DOL equality, not
by relaxing object checks. ELF symbol/string-table metadata differs only.

The TU retains legacy local API views: `mbGuideModelGet` returns `MBMODELID`
and `mbMasuFind_TypeListGet2` returns `s16` to this caller. Their canonical
owners/public headers are not changed. Target conversion behavior and
same-game caller precedent support this local visibility; this is not a
claim that differing return declarations are a portable modern C interface.
The callback's old-style type/32-bit function-address conversion follows the
already inspected same-game callback boundary. It is retained legacy source
shape, not a generally recommended C idiom. The initial `_MATH_H` prefix was
unnecessary with the current shared-header coexistence checks and is absent
from the final source. No header-guard override exception remains.

The final publication cleanup c35 replaces inherited hexadecimal resource and
message numbers with `DATANUM`/`MESSNUM` domains, names the process and start
flag constants, and uses the existing Mario voice ID plus six. It removes
the explicit coin-work pad member; natural alignment preserves its fields.
The existing three unknown words at offsets 48..59 remain explicitly unknown:
the 60-byte record stride and 40-record BSS extent are target-backed, but their
semantic meaning is not established. The narrowly scoped metadata exception
records this distinction. These changes preserve all 22 strict instruction
matches; they are source-quality cleanup, not additional cracks.

## What actually closed the last mismatch

c28 already had exact saved-register roles, stack homes, and the middle of
`ev_OpeningSingle`, but 54 data instruction rows and two extra moves remained
across distant regions. Operand edits were not the source cause.

The optional native `mwcc_win32_varinfo.py --machine-emit` mode joined observed
pre-color GPR ancestry to actual emitted instructions. On this case it
reproduced all 1,063 emitted words byte-for-byte, closed 2,424 operand joins,
and left 106 unseen operands explicitly UNKNOWN. The existing temporary-pool
analysis located a real pool reset at instruction 835 and bounded an earlier
temporary-count disturbance before vreg 40. It did not invent target vreg IDs
or generate the C repair.

The primary source decision was the declaration/conversion boundary:

```c
/* Before: public int return plus an explicit caller cast. */
masuNum = (s16)mbMasuFind_TypeListGet2(...);

/* After: target-backed local s16 return; the live local remains int. */
masuNum = mbMasuFind_TypeListGet2(...);
```

This removed one hidden frontend temporary and closed all 54 instruction
rows together, bringing Single from 4,252 to the retail 4,244 bytes. Narrowing
the live local instead had added three later sign extensions; the compact
lesson is to distinguish API conversion from local storage, not to try a
matrix of narrow types.

The remaining linked differences were 84 SDA21 sites referencing 14 real
zero-initialized globals. Reordering those real definitions to match MWCC's
reverse small-BSS emission closed the complete linked storage permutation
without changing any function instruction body.

## Tool acceptance and reuse

Only the existing native tracer and its tests changed. The optional GPR mode
reuses the authenticated emission hook, checks the observed PCode/operand
identity before joining ancestry, bounds capture counts, and leaves missing
ancestry UNKNOWN. Default behavior is unchanged. Native acceptance on c28
answered a missing source decision and led to the retained c33/c34 closure;
it was not merely a replay of an already known solution.

The focused native-tracer and temporary-pool suites passed 63 tests. Source
quality reported no findings. Both c33 and final c34 compile byte-identically
with canonical-main headers and generated include context. Clean promotion
and all-container verification are recorded at
their actual completion rather than inferred from the diagnostic DOL.
