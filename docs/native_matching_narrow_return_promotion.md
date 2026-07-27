# MWCC narrow-return promotion and normalization

GC/2.6 can preserve a post-call sign extension when an authenticated narrow
return is explicitly promoted before assignment. This is a source-level width
boundary, not permission to alter the shared function declaration.

## Confirmed evidence

- `optionDll:fn_1_A0` assigned the `s16` result of `Hu3DGLightCreate` directly
  to an `s16` lifetime. That natural form omitted the retail post-call `extsh`.
- Restoring the historical form
  `lightId = (int)Hu3DGLightCreate(...)` reproduced the target normalization
  and made the function strict exact without changing the linked symbol or the
  authoritative producer type.
- `fileseldll:ObjectSetup` independently uses the same explicit promotion at
  the same API boundary and is exact, providing a same-game consumer rather
  than a compiler-only guess.

## Recovery rule

Authenticate the producer return width, destination width, and target post-call
extension before testing one explicit-promotion control. Retain it only when an
independent same-game consumer supports the form and the complete instruction
and relocation stream is exact.

Do not change a narrow API declaration to `int`, and do not scatter arbitrary
casts to steer registers. The eventual destination may remain `s16`; the
recovered behavior is the explicit promotion boundary between the call and that
destination.
