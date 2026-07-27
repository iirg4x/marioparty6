# MWCC commutative `fmuls` operand-order negative evidence

Algebraically reversing a floating multiplication is not a reliable way to
change MWCC register allocation. Two compiler profiles produced neutral source
swap controls in otherwise exact-sized functions.

## Bounded observations

- Under GC/2.6, `s01Dll:fn_1_1E70` and `fn_1_1EF8` retained the same emitted
  `fmuls` operand order after reversing the commutative source operands.
- Under GC/1.3.2, `staffdll:fn_1_44`, `fn_1_118`, and `fn_1_200` did the same.
  An explicit capture control added `0x10` bytes of frame/instruction debt
  instead of correcting the original mismatch family.

## Recovery rule

When function size, control flow, producers, and every non-`fmuls` instruction
already agree, permit one direct operand-reversal control. If the compiler
canonicalizes it identically, stop algebraic spelling probes. Investigate the
producer lifetime, expression tree, or definition/evaluation chronology that
assigned the registers.

Do not retain a temporary whose only purpose is register-pressure shaping. This
card records a useful stopping condition; it is not a recipe for forcing an
operand order.
