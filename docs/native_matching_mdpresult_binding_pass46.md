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
