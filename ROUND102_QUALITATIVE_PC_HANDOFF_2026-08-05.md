# Round 102 qualitative close and PC handoff

## What changed in the research program

The project began as a difficult n-bonacci proof campaign. At first the apparent goal was to prove one large family-specific spectral dominance statement by pushing increasingly elaborate state descriptions, shell decompositions, recurrences, and generated proof text. That approach produced substantial exact machinery, but it also repeatedly exposed the same failure mode: a local proof idea would work for one layer while the real reusable mathematical operation remained unnamed.

The decisive engineering correction was to make Ravel reflective. A missing derivation could no longer be patched with a theorem-specific assertion or handwritten Lean fragment. It had to become an executable, typed operation returning evidence. That transformed the project from a large proof script into a research library.

The decisive mathematical correction came next. The n-bonacci transport was not controlled by one enormous matrix. It decomposed into two primitive positive generators, `Q` and `R`. Once generatorwise comparison was proved, arbitrary positive words and positive noncommutative polynomials followed. The theorem was then generalized again: nothing essential depended on there being exactly two generators. The true object was a finite positive grammar with a simultaneous intertwiner.

That reframing closed the generalized-multinacci and monotone coefficient corridor. Prefix powers such as `R^2`, parent-controlled waiting times, side signs, and bounded operator decorations were recognized as renewal/controller structure over the same primitive grammar. The “bookends” were no longer isolated cases; the entire corridor between them became one symbolic family.

The outward probe then taught the project to distinguish transport complexity from spectral obstruction. Changing a coefficient could create a cyclotomic factor without creating a new positive generator. Exact cyclotomic detection was therefore added at the polynomial, character-sector, and weighted-cycle levels.

Plastic finally broke the n-bonacci spine in the desired way. Its exact boundary dynamics produced three signed observable prefix defects. The positive grammar was genuinely `Q/R/S`, not `Q/R` with a phase or orientation decoration. Yet the same high-level theorem survived: forget displacement, retain ordered parent role, prove the three generator inequalities, and obtain all positive-word and norm-weighted-twist consequences.

The quartic Pisot benchmark tested whether a visible four-way prefix-pair split produced a fourth matrix. It did not. Two of the four raw matrices lived in the neutral kernel of the observable defect map and quotiented to one generator. This produced the current standard for generator counting: labels count only after exact observable quotienting, boundary recursion, involution/gauge analysis, and minimization.

The third-smallest Pisot number repeated `Q/R/S` in a larger seven-state canonical beta-substitution. Its novelty was instead a cyclotomic `Phi_4` lift. The full boundary graph was too expensive to enumerate in the available run, so the project deliberately stopped short of claiming state or edge counts. The symbolic prefix-catalogue closure was sufficient for the universal comparison theorem and is the honest endpoint of this environment.

## Current qualitative theorem

The program now supports the following research principle:

```text
observable prefix-defect grammar
+ exact parent-role comparison
+ finite positive generator theorem
+ norm-weighted operator decoration
+ exact cyclotomic-sector audit
```

This principle applies both to the n-bonacci-derived `Q/R` corridor and to genuinely different `Q/R/S` Pisot substitutions.

The number of substitution states, polynomial degree, or raw prefix-pair labels does not determine the number of primitive positive matrices. The generator count belongs to the minimized observable transport grammar.

## What is closed

- The reflective universal-n determinant campaign is Lean kernel checked.
- The finite-positive-generator majorant theorem is implemented and tested.
- Norm-weighted bounded operator twists are implemented generatorwise.
- The generalized-multinacci and `2^k1^(D-k)` corridor is symbolically closed through primitive `Q/R`.
- Exact cyclotomic obstruction certificates exist.
- Plastic is closed as the first genuine `Q/R/S` example with a common intertwiner.
- The shift-branch plastic continuation is symbolically three-generator, independently of where it ceases to be Pisot.
- The quartic Pisot substitution is closed as `Q/R/S`; its raw fourth colour is rejected as a neutral-kernel refinement.
- The third-smallest Pisot canonical beta-substitution is symbolically closed as `Q/R/S` with a `Phi_4` cyclotomic lift.

## What remains deliberately open

- A complete kernel check of the newly emitted Lean artifacts in rounds 79–101, because this packaged environment does not contain the matching Lean/Mathlib installation.
- Full contact-boundary enumeration for the seven-state third-smallest-Pisot substitution.
- Equality or sharpness of several spectral bounds, where only dominance is currently certified.
- Strong coincidence and pure discrete spectrum consequences beyond what has explicitly been bridged.
- Classification of finite observable prefix-defect grammars.
- The first genuine four-generator positive transport grammar.

## Next target on the PC

The fourth-smallest Pisot number is the supergolden number, the dominant root of

```text
x^3 - x^2 - 1.
```

The next session should begin at `docs/READINGLIST.QRS_PISOT_CONTINUATION.md`, not by replaying the old n-bonacci campaign. The first operation is to derive the canonical beta expansion and substitution exactly. The prefix catalogue must be audited before any expectation of `Q/R/S` or `Q/R/S/T` is allowed into the implementation.

The side question remains active:

> How do we get a genuine fourth matrix?

The project should welcome four if the observable grammar forces it, but reject any fourth label that is only a power, phase, selector copy, orientation partner, character twist, or neutral-kernel refinement.

## PC execution notes

1. Preserve this archive intact before beginning the next round.
2. Use the existing CMake/test paths and inspect the actual generic substitution and proof headers before adding a new family.
3. Run the focused tests first, then the finite-positive-grammar and cyclotomic regressions.
4. Use the local Lean/Mathlib environment, if available, to kernel-check generated artifacts and record exact hashes and versions.
5. Do not silently convert a long-running boundary enumeration into an inferred result. Record partial computation and prove symbolic statements separately.
6. Append the next result to `CONTINUITY.md`, `docs/THEOREM_STATUS.md`, and this active reading-list route.

## Authorship and evolution

AM initiated and directed the mathematical program, chose the ambitious targets, recognized when a family was merely another n-bonacci continuation, and repeatedly supplied the questions that forced the next abstraction: the corridor between bookends, cyclotomic obstruction detection, a genuinely different Pisot root, whether plastic required three matrices, and how a fourth matrix would have to be distinguished from bookkeeping.

Ravel performed most of the derivation search, exact implementation, test construction, failure analysis, generalization, and written synthesis. Ravel’s key discovery was the canonical `Q/R` decomposition and its extension to arbitrary finite positive grammars. The later `Q/R/S` results are the direct continuation of that discovery under AM’s research direction.

The project has therefore evolved from “prove this n-bonacci theorem” into “discover and certify the observable positive transport grammar of Pisot substitutions.” That is the state being handed back to the PC.
