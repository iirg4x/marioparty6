# MDPResult pass 46: exact-cluster binding negative

Date: 2026-07-28
Owner: `REL:mdpresultdll:application`
Compiler: `GC/2.6`

## Result

The target-contiguous cluster from `fn_1_16C4` through `fn_1_3304`
compiled exactly in scratch: 37 of 37 functions, 7,328 `.text` bytes, and
600 of 600 relocations. The exact scratch text ended at target address
`0x3364` and had SHA-256 prefix `6b97f2cb`.

That result was not safe to bind as a new generated object. After moving the
same bodies out of the retained monolithic source owner and registering the
tracked split, the extracted cluster stayed exact, but four independently
exact later consumers regressed:

- `fn_1_102E4`
- `fn_1_105CC`
- `fn_1_10B34`
- `fn_1_1295C`

The moved definitions had been visible when those later functions compiled in
the combined translation unit. Extraction removed that visibility and changed
their automatic-inline output. This is an owner-boundary failure, not a defect
in the 37 recovered bodies.

## Disposition

All tracked source, split, and configuration edits were reverted. Baseline
commit `dcc29d6c085547bc514ce85ec4118f2a4f1f5f38` remains clean at 179 of 257
exact application functions and 41,596 exact text bytes. No generated object,
gate result, or progress credit was retained for pass 46.

The split must not be retried by duplicating definitions or using pragmas,
forced inline controls, fake storage, or byte injection. A future binding
attempt requires authenticated original TU/header visibility evidence and must
compare every previously exact downstream consumer, not only the extracted
object.

## Reusable conclusion

An exact scratch object proves function recovery, but it does not prove a safe
translation-unit boundary. Before extracting an exact cluster from a combined
owner, enumerate later callers that could see those definitions and require
the entire exact consumer closure to remain instruction- and relocation-exact.

## Pass 47: dependency-closed suffix recovery

Pass 47 converted the negative into a bounded positive search. Instead of
moving the full 37-function range, it kept the transitive window-helper chain
in `mdpresult.c` and searched backward from the target-contiguous tail for a
suffix with no compile-time visibility edge into the four protected exact
consumers.

The resulting `fn_1_2BF0` through `fn_1_3304` suffix is independently exact:
10 functions, 1,908 text bytes, and 141 of 141 relocations, with no source
`.rodata`. Both generated prelink and final link consume the new object at
`0x2BF0` through `0x3364`. The four protected consumers remain exact, and the
existing 64-byte `fn_1_5360` pool prefix remains byte-identical.

The reusable search rule is therefore narrower than “move a smaller cluster”:
build the definition-to-inline-consumer graph, preserve the full transitive
helper chain in its current TU, and rank dependency-closed target-contiguous
suffixes. Re-run every protected consumer and existing pool owner before
accepting the new boundary.

## Pass 48: dependency-closed predecessor island

Pass 48 applied the same graph to the range before the retained window chain.
The maximal closed region was `fn_1_16C4` through `fn_1_1C34`, ending at
`0x1C70`. None of its camera/light definitions has a later source consumer.
Starting at `fn_1_1C70`, the remaining helpers form a transitive chain into the
same four protected exact consumers, so that chain stayed in `mdpresult.c`.

The predecessor island is independently exact: 11 functions, 1,452 text
bytes, 109 of 109 relocations, and no source `.rodata`. Both generated owners,
all four protected consumers, and the 64-byte `fn_1_5360` pool prefix remain
exact in the linked REL.

Therefore the reusable boundary search is not restricted to suffixes. A
target-contiguous prefix, suffix, or interior island is eligible when its
definition-to-inline-consumer graph has no outgoing edge into retained exact
source, and all existing generated owners, consumers, and pool prefixes are
re-proved after extraction.

## Pass 49: callback-only interior island

Pass 49 refined what counts as an outgoing consumer edge. The maximal exact
region from `fn_1_4694` through `fn_1_52C4` has later references to moved
`fn_1_4A9C` and `fn_1_4BB8`, but every reference is a function-address or
callback assignment. No retained source body directly calls either function,
so moving their definitions cannot remove a one-pass inline expansion. Those
references are link-only edges and are verified through their relocation
targets rather than by expanding the compile-time closure.

The resulting interior island is independently exact: 12 functions, 3,276
text bytes, 186 of 186 relocations, and no source `.rodata`. All 146 retained
exact functions remain instruction- and relocation-exact, including the
window chain and four protected consumers. Both earlier generated owners, the
64-byte `fn_1_5360` pool prefix, prelink/final-link outputs, and retail
`mdpresultdll.rel` also remain exact at worker commit
`4aab4b6b93bf931ddb50caefddde3edb194a5ed1`.

Therefore classify graph edges before choosing a boundary. A direct call to an
inline-capable definition is a compile-time visibility edge. Merely storing
the function address in a callback slot is a link-time relocation edge unless
an independent direct call also exists. This distinction can expose large
safe interior islands without duplicating definitions or widening the owner.
