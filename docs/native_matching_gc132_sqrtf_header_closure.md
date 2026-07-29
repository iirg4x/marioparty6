# GC/1.3.2 sqrtf header-closure ownership

## Reusable result

With the repository's pinned GC/1.3.2 profile, an unused extern-inline `sqrtf` body exposed by a broad public header can emit the weak local-static constants `_half$localstatic3$sqrtf__Ff` and `_three$localstatic4$sqrtf__Ff`. The recovered function's `.text` and relocations can remain exact while the linked REL gains 16 bytes of `.rodata`.

Treat that signature as an include-closure problem before changing already exact function C.

## Confirmed evidence

### mdsing pass20

Eight natural donor-derived functions reached exact text and relocation shapes but were not retained because their broad public-header closure emitted the 16-byte weak `sqrtf` block. The accepted three-function subset did not require that closure. No suppression macro, pragma, fake prototype, padding, or fabricated literal was used.

### mdbank pass3

The first linked `fn_1_2CD8` owner used broad Dolphin/game headers. Its source object had exact 152-byte text and exact relocations, but the full REL was 16 bytes too large in `.rodata`. The PLF exposed the two weak local-static `sqrtf` symbols immediately before the target runtime constants.

Replacing the umbrella includes with the authenticated narrow type header `<dolphin/mtx/GeoTypes.h>` and canonical `Vec`/`s16` declarations removed all source-object `.rodata` without changing the function text or relocations. The resulting `mdbankdll.rel` was 138,528 bytes and matched retail exactly, with SHA-1 `a1f9c9f9f8dfb62ffbfbb75f7155c3ff38b4df79`.

## Recovery procedure

1. Prove the function text and normalized relocations independently.
2. If the REL is exactly 16 bytes too large, inspect the PLF/object symbols for `_half$localstatic3$sqrtf__Ff` and `_three$localstatic4$sqrtf__Ff`.
3. Compare the preprocessed include closure and remove umbrella headers that are not required by the owner.
4. Prefer the narrowest authenticated repository header that supplies the real types. Preserve the repository's existing implicit extern-call behavior when that is the historical C shape.
5. Rebuild the object, confirm the unwanted `.rodata` is gone, then run the serialized full checksum and direct retail REL comparison.

## Boundaries

- Do not suppress the symbols with macros, pragmas, local fake prototypes, or compiler controls.
- Do not rewrite exact function bodies merely to compensate for a section owned by an unused inline header.
- Do not accept object-only proof. The mdbank case demonstrates why linked-section and retail-byte checks remain mandatory.
- A synthetic zero-text emitter establishes compiler capability, not historical ownership.
