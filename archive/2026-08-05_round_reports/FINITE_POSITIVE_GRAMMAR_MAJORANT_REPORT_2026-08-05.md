# Round 84 — Finite Positive Grammar Majorant

## Scope

This round lifts the hard-coded two-generator Q/R theorem to an arbitrary finite generator alphabet Γ. It does not yet infer Γ from a raw normalization automaton; it proves and executes the universal layer once every channel has a certified generator label.

## Theorem implemented

For generator matrices M_g and norm-weighted matrices M_g^#, every generator word w satisfies the operator-path majorant represented by M_w^#. Every finite nonnegative boundary polynomial, represented by a list of words with repetition encoding multiplicity, inherits the same majorant.

If all certified channel bounds are at most one, then M_g^# <= M_g entrywise for every g, and this inequality propagates through every word and positive polynomial. Expansive generators remain covered by the weighted theorem but do not project to ordinary counts.

## New proof operation

`derive_finite_positive_grammar_majorant`

Inputs:
- finite base vertex set;
- finite generator count;
- channels with source, target, unique generator label, multiplicity, and exact rational norm bound;
- optional positive boundary polynomial as repeated generator words.

Derived evidence:
- total and single-valued generator partition at the channel-record level;
- exact count and norm matrices for every generator;
- triangle-inequality parallel-channel closure;
- submultiplicative path/word closure;
- positive-polynomial closure;
- contractive projection to ordinary count matrices;
- preservation of the legacy Q/R specialization when |Γ| = 2.

## Validation

The executable test includes a genuinely three-generator grammar with balanced, left-defect, and right-defect classes; an expansive third-generator variant; malformed-label rejection; and exact comparison with the legacy Q/R API.

Passing targets:
- `finite_positive_grammar_majorant_test`
- `norm_weighted_qr_majorant_test`
- `twist_dominance_extensions_test`
- `concrete_operator_qr_majorant_test`

The emitted Lean theorem is generic over an arbitrary generator type, not merely a finite type. Kernel checking was not available because the archive lacks the configured Mathlib ridealong at `../LEAN/projects/nbonacci_charmpoly`.

## Remaining discovery layer

The next reusable operation should derive the generator partition from a paired Condition-F normalization automaton. It must certify transition coverage, partition refinement under transport behavior, compatibility with composition, and expression of the boundary recursion as a positive word polynomial. That discovery/minimization problem is separate from—and now cleanly feeds—the completed universal finite-grammar theorem.
