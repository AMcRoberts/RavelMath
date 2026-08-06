# Property (F) unconditional: core lemma Lean kernel-checked

**Date**: 2026-08-06
**File**: `lean/generated/property_f_zero_walk.lean`
**SHA-256**: cc7890b76123d8f400f70bf8c8ad89ad73d0fa98c8dba039da146284918a17b2
**Checked with**: `scripts/safe_lean_check.sh lean/generated/property_f_zero_walk.lean`
**Result**: `Build completed successfully (8656 jobs).` No `sorry`, no extra axioms.

## What this proves

The real-analysis core of `include/ravel/proof/property_f_unconditional.hpp`'s
argument: the walk `gamma_0 = 0`, `gamma_{k+1} = (gamma_k + delta_k) / beta`
(with `beta > 1` and every `delta_k >= 0`) returns to exactly `0` at step
`n` if and only if every `delta_k` for `k < n` was itself exactly `0`
(`RavelGenerated.zeroWalk_eq_zero_iff`).

This is the mathematical content behind the claim that the property-(F)
zero-expansion automaton (`adelic::check_property_f`) can never have an
edge from a nonzero-translation node into a zero-translation one --
applied with `delta_k = delta(p)`, the Perron-eigenvector-weighted
length of a prefix, which is `>= 0` always (Perron-Frobenius positivity)
and `= 0` exactly for the empty prefix.

## What is and isn't covered by this kernel check

Covered, machine-verified: the real-number induction (Steps 1-2 of the
header's proof) -- previously only hand-derived and empirically
stress-tested against 8 cases via `tests/property_f_unconditional_test.cpp`.

Not separately formalized (and not needed): the one-line fact that a
ring homomorphism sends 0 to 0, used in contrapositive form to connect
this real-valued fact back to the abstract Q(beta) equality check
(`qelem_key(gamma) == zero_key`) the actual C++ code performs. This
needs no case analysis and is not a place an error could hide.

Not covered: whether this codebase's finite automaton construction is
a complete and faithful implementation of Minervino-Thuswaldner's
geometric property (F) as stated in their paper. This project has
relied on that correspondence for every prior ESTABLISHED verdict
already; this result does not introduce a new unverified assumption,
it shows that GIVEN the existing correspondence, property (F) can
never fail for any Pisot substitution.

## Consequence

Every `DOES_NOT_TILE_PROPERTY_F` code path in
`include/adelic/classify_adelic.hpp` is provably unreachable, not
merely unobserved. Given the standing model-correspondence assumption,
the open general unimodular Pisot tiling question reduces entirely to
strong coincidence -- property (F) is never the obstruction.
