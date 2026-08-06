# Lean playground check — joint Q/R universal dominance interface

Paste `lean/generated/condition_f_joint_qr_playground.lean` into a Mathlib-enabled Lean 4 playground.

Expected result: no errors and no unsolved goals.

The file kernel-checks four reusable logical steps:

1. simultaneous Q/R inequalities propagate through every noncommutative word;
2. they propagate through every finite nonnegative word polynomial, including the generated terminal-parent boundary substitution;
3. the base comparison propagates by dimension induction;
4. the quotient/core/literal inequalities compose by antisymmetric sandwich.

This file deliberately does not claim that the local machine has run Lean. The user performs the actual playground/kernel check.

## Result

AM reported on 2026-08-05 that the standalone playground file passed the Lean
kernel check with no messages.  The four listed generic steps are therefore
kernel-checked.
