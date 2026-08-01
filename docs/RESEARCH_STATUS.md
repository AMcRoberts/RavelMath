# Research status

This is the concise entry point for the mathematical and engineering
state of the project. Detailed claim strength is recorded in
`THEOREM_STATUS.md`; the exact active Class-II seam is recorded in
`GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`.

## Verified engineering boundary

The latest complete validation passed `make check` and
`make lean-check`; enrolled Lean files were sorry-free. A fresh checkout
does not include generated build outputs and should rerun those commands.

## Class-II family

The balanced-pair side has an explicit recurrent eight-state family for
every `sigma_{a,1}`, `a>=2`, with exact reductions, strong connectivity,
and characteristic polynomial.

The contact side has an explicit rigid dominant core, a twenty-role
affine interior shell, exact non-base catalogues, universal window and
ranked Red certificates on their stated domains, and a kernel-checked
partition of every legal round for `a>=7` into five phases.

The global occurrence theorem's four-round base seam is now closed
(2026-07-31, night): all four exceptional base transitions (items 1-4
below) are closed for every integer `a>=7` (item 1 for every `a>=3`),
one tier below Lean-formalized. This does not by itself complete the
global occurrence theorem -- recurrent-SCC exhaustion and dominance
remain a separate, later step -- but the base-round seam that has been
this investigation's active target for two days is done. Item 5
documents the shared argument that closes items 2-4; it names no open
work of its own.

1. signed-contact reverse inclusion at the neighbor base -- **closed
   for every `a>=3` (2026-07-31)**: the *unsigned* raw 27-state
   self-closure and its Red-pruning to 25 survivors are proved for
   every `a>=3` (window validity abstractly, Red pruning symbolically
   from `tau_a`'s own fixed word forms), not just checked at sampled
   `a`. The full *signed* 50-state contact set is a verified identity
   (`class_ii_neighbor2_signed_contact_set()` equals the unsigned
   survivors' mirror-closure, exactly), and **checked directly against
   the primary source (Loridant-Thuswaldner-Zhang, arXiv:2511.16442,
   2026-07-31) that this identity is the whole obligation**: the
   paper's own `±K := K∪(−K)` (§3.5) is a pure notational
   symmetrization, never an independently-closed object, and its
   corona iteration (Algorithm 2) starts *from* `±C` directly (`A[1] =
   Ĝ_C`) rather than deriving it as a closure target. There is no
   "reverse inclusion for `±C` as its own object" obligation in the
   source material beyond the already-closed unsigned half. Two
   candidate closure-of-a-symmetric-seed hypotheses were tried and
   refuted first (mirror-only seed, and `D_cont ∪ mirror(D_cont)`,
   both giving the same non-self-mirror-closed 32-state subset of
   `±C` at `a` in `{3..8,20,50}`) before the literature check settled
   that neither was actually the missing piece, because there was no
   missing piece of that shape. The Red-pruning argument's arithmetic
   core is Lean-kernel-checked (`lean/class_ii_round1_red_pruning.lean`,
   sorry-free, signed) -- the combinatorial exhaustiveness fact behind
   it is still taken as given C++-checked input, not re-derived in
   Lean, so Round 1 sits at the same "proven, one tier below
   Lean-formalized" strength as items 2-4. See `docs/GLOBAL_CATALOGUE_
   OCCURRENCE_EXHAUSTION.md`'s Round 1 sections for the full account,
   including a caught-and-corrected overclaim about mirror-closure
   preserving correctness and the literature-check section that closed
   this item.
2. round-two raw reverse inclusion and Red exclusion -- **closed for
   every integer `a>=7` (2026-07-31)**: `T_2 = B_2 union E_2` exactly
   (literal identity), and Red exclusion (123 states across three
   ranks) is now provable for every `a>=7`, not just exact-finite-
   checked -- see item 5 below for the argument that closes rounds
   two through four together.
3. round-three raw reverse inclusion and Red exclusion -- **closed for
   every integer `a>=7`**, same argument as round two (`T_3 = B_3
   union E_3` exact, 93 = 68 + 25; 163-state Red exclusion).
4. the round-four reverse/exclusion bridge into the stable theorem --
   its raw-corona occurrence/exclusion property (`T_4 = B_4 union E_4`
   exact, 113 = 88 + 25; 212-state Red exclusion) is **closed for every
   integer `a>=7`** by the same argument. **The actual "connect to the
   round>=5 universal theorem" claim this item names is now closed too
   (2026-07-31, later)**: the already-proven stable raw-corona
   composition certificate (universal for `a>=7`) assumes a specific
   generic-formula source at round 5 (`class_ii_center_layer_candidate
   (a,4)` union `class_ii_neighbor2_layer_extension(a,4)`), and that
   formula is checked, node-for-node, exactly equal to the real
   round-4 post-Red survivor catalogue at 14 widely separated `a`
   (`app/class_ii_round4_stable_bridge_check.cpp`). More than a check:
   the formula side is *provably* `a`-independent by direct code-path
   inspection (its branch conditions are always satisfied once `a>5`),
   and the ground-truth side's `a`-independence for every `a>=7`
   follows from Round 4's own closed-form Red-exclusion proof (a fixed
   node set with a fixed edge set gives a fixed Red-pruning result) --
   so the equality holds for literally every `a>=7`, not just the
   tested values. This closes round four's row in the base-premises
   table.
5. **the argument that closes items 2-4 (2026-07-31, same session):**
   the raw candidates behind Rounds 2/3/4's Red exclusion split into
   exactly three shapes by whether each side's occurrence ranges with
   `a` -- both-range (4 shapes, closed by direct tracing), hybrid (20
   shapes, closed by an exhaustive slope enumeration showing the
   achievable window always grows from one fixed edge), and both-fixed
   (20 shapes, closed by the corrected condition `rhs2 = slope_q -
   slope_p`, found after an earlier draft's `rhs2=0` guess produced
   real counterexamples). All three checked exhaustively (every valid
   edge, not sampled states) at widely separated `a`, zero exceptions.
   Still one tier below Lean-formalized. See `docs/GLOBAL_CATALOGUE_
   OCCURRENCE_EXHAUSTION.md`'s category-closure sections for the full
   account.

Across all four rounds, the "are the pruned-state coordinate sets
literally `a`-independent" question is now resolved: **no**, not
uniformly. Round 1 is a genuine outlier (pruned coordinates affine in
`a`, small 27-state raw target, already closed symbolically and now
Lean-formalized for its arithmetic core). Rounds 2/3/4 share a second
pattern instead: a fixed, `a`-independent pruned-state list with only
the forward-edge weights varying by `a`. This means the still-open
symbolic Red-exclusion proof is one argument applicable to all three
of rounds 2/3/4, not three independent derivations.

**Sharper still (2026-07-31):** the entire forward-edge connectivity
graph over each round's raw pre-Red states -- not just the pruned node
set -- checks out `a`-independent too, at 7-8 sampled `a` values per
round including one far outside the cluster (Round 2 stress-tested at
`a=50`), with a single genuine exception at Round 4's `a=6` (resolved
by `a>=7`).

**The mechanism behind this, closed exactly (2026-07-31, same
session):** the general lemma the paragraph above still needed --
"forward-edge coordinates are affine in `a`" -- turned out to need a
different independent variable than `a` itself. Working directly from
Round 1's own `M(a)` back-substitution shows `x0'`/`x1'` depend only
on the per-letter abelianization counts of the occurrence prefixes
(not their length or `a`), and `x2'` is affine in the occurrence-index
*difference* with slope exactly `1` once those counts are fixed.
Grouping raw candidates accordingly gives **100% clean** agreement at
every tested `a` (135/135 groups, zero exceptions) -- verified against
its own derivation, not curve-fit. The one known exception (Round 4's
`a=6`) is traced to a concrete missing occurrence, not an unexplained
gap.

**Closed for every integer `a>=7`, same session, later.** The raw
candidates split into exactly three shapes of argument (both-range,
hybrid, both-fixed) by whether each side's occurrence ranges with `a`.
All three are now proven, not merely checked: both-range by direct
tracing; hybrid by an exhaustive slope enumeration (every fixed
occurrence in `tau_a`'s images has slope exactly `0` or `1`, forcing
the achievable window to grow monotonically from one fixed edge);
both-fixed by the corrected condition `rhs2 = slope_q - slope_p`
(an earlier draft wrongly required `rhs2=0` and found real
counterexamples -- checked exhaustively, not sampled states, at two
widely separated `a`, then closed in actual closed form: every
slope-nonzero case's required `a` is solved for directly and shown
`<7`, not sampled further). **Rounds 2/3/4's Red-exclusion property is
therefore provable for every integer `a>=7`**, not merely exact-
finite-checked through `a=50`. The shared arithmetic core (why a
solution below a threshold never recurs above it) is now Lean-
kernel-checked (`lean/class_ii_round234_shape_closure.lean`,
sorry-free, signed) -- the specific `pre_red` catalogues themselves
remain C++-checked input, not ported to Lean, so the overall closure
is one tier below fully Lean-formalized. See `docs/GLOBAL_CATALOGUE_
OCCURRENCE_EXHAUSTION.md`'s "Sharper still" section and the three
category-closure sections following it for the full account.

Positive witnesses establish inclusion and survival but do not establish
reverse inclusion or exclusion. Cardinality agreement is discovery
evidence, not set equality.

After layer occurrence is closed, recurrent-SCC exhaustion and dominance
remain separate. Layer equality alone does not prove that a displayed
block is exactly a recurrent SCC.

**Progress on this (2026-07-31/08-01, night):** all five support
statements are now verified together for neighbor 2 at `a` in
`{7,...,20,30}` (fifteen values, `a=30` well outside the cluster), zero exceptions. Items 1
(core/shell/transient partition), 2 (strong connectivity tied to the
real graph), and 5 (recurrent-block distinctness) follow from an exact
match between the real, Tarjan-derived SCC decomposition of the
fully-converged boundary graph and the hand-catalogued recurrent
blocks (also checked separately at `a=30`) -- a Tarjan decomposition
guarantees all three by construction once the match is exact. Items 3
and 4 needed a round/rank ordering on transient states, built directly
from the trusted corona trace (`birth_round(state)` = the first round
it survives Red in, well-defined since Red never re-removes a state
from a later round once it has survived one). That ordering also
answered the scoping question it was built to resolve: rank and round
correspond cleanly (`birth_round = a - rank`, one small bounded tie
near the dominant core), not messily. Item 4 (no return to an earlier
transient stratum): zero violations. Item 3 (escape from every
transient block): held once "escape" was corrected from "strictly
later round" to "does not remain transient forever" (same-round
absorption into a recurrent block counts) -- the strict version found
one apparent gap (the terminal-round transient group) that turned out
to escape into the same-round recurrent core instead, the same kind of
over-strict-then-corrected pattern as item 5's "zero inter-block
edges" earlier. All checked, not yet closed-form -- see
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s "Items 1, 2, and 5"
and "Items 3 and 4" sections for the full account.

## Exact and formal machinery

The native math library provides arbitrary-precision integer and
rational arithmetic, polynomial and matrix arithmetic, `Q(beta)`
arithmetic and linear algebra, Sturm sequencing and root isolation,
exact characteristic polynomials, Perron certificates, rational
intervals, tunable binary big floats, and FFT implementations.

Lean modules formalize the free-involution descent mechanism, return/
contact projection, the conditional determinant reduction, Class-II
affine shells and margins, endpoint catalogues, support identities, and
the global round partition. The explicitly named existence draft is not
an enrolled theorem.

## Adelic and CSY boundary

The adelic classifier includes number-field factorization, ideal
arithmetic cross-checks, local-field arithmetic, prefix automata,
coincidence, and property-(F) components.

The CSY power machinery has exact recurrence and companion-matrix routes
with bounded memo residency. This does not prove globally bounded
memory: exact outputs, arithmetic temporaries, and live polynomial
contexts remain separate resource dimensions.

The current `CSYAutomaton` is a bounded absolute-position prefix object,
not yet the finite local carry quotient of the cited construction.
Depth exhaustion is reported as truncation.

**General number-field machinery, scoped and started (2026-07-31).**
Separate from the Class-II seam: after correcting an earlier bib note
that falsely connected OpenAI/Sawin's unit-distance disproof to this
project's Pisot/Rauzy-fractal machinery (see `refs/references.bib`'s
`Williams2026OpenAIUnitDistance` correction -- that construction needs
a Golod-Shafarevich class-field-tower argument with no substitution or
tile shape anywhere in it), a genuine, tracked initiative to build the
general number-field layer that construction would need was started,
in bounded pieces validated by property/parity tests rather than
assumed correct. First finding: `include/adelic/dedekind_
factorization.hpp` and `ideal_arithmetic.hpp`'s existing Dedekind-
criterion and HNF-ideal machinery, built and tested only against Pisot
substitution characteristic polynomials, turns out to be genuinely
field-generic -- checked against Q(sqrt(-5)), a totally imaginary
field with no real embedding at all, reproducing the classical
`6=2*3=(1+sqrt(-5))(1-sqrt(-5))` non-unique-factorization example with
zero code changes (`tests/number_field_parity_test.cpp`). Second:
`math/poly_discriminant.hpp` extends `maximal_order.hpp`'s
long-long-only discriminant computation to arbitrary precision, and in
doing so found a real, previously undetected bug -- the long-long path
silently returns wrong values (not merely too-large-to-hold ones)
starting at degree 9, from intermediate Bareiss-elimination overflow,
confirmed against an independent closed-form re-derivation
(`tests/poly_discriminant_bigint_test.cpp`, documented directly in
`maximal_order.hpp`). Third: `include/adelic/dedekind_
factorization.hpp`'s `factor_fp` (linear-factor root extraction only,
assuming any degree>=2 residual is a single irreducible) is replaced
with `fp_poly_factor.hpp`'s general, any-degree algorithm (squarefree
+ distinct-degree + Cantor-Zassenhaus equal-degree factorization,
Cohen ch. 3.4) under the same name -- every caller gets the fix with
no call-site changes. Found and fixed a real bug this same way: `x^4+1
mod 5` factors as two irreducible quadratics, which the old code
reported as one wrong quartic, propagating to `factor_prime_in_qbeta`
wrongly reporting one prime above 5 (residue degree 4) instead of two
(residue degree 2 each) -- demonstrated at both levels in
`tests/fp_poly_factor_test.cpp`. `FpPoly`/`FpFactor` and the shared
primitives were extracted to a new `fp_poly.hpp` to avoid a circular
include between the two files. Fourth: `maximal_order.hpp`'s
Pohst-Zassenhaus Round 2 enlargement gets the same arbitrary-precision
treatment (`enlarge_order_round2_bigint`), and the long-long path's
overflow risk turns out to have a concrete, ugly consequence: for
`x^10-2` at `p=2` it doesn't just lose precision, it throws a
*spurious* runtime exception (an int64 overflow masquerading as a
"construction invariant violated" math bug). The bigint version
computes cleanly and passes an independent sanity check --
`disc(O)/disc(O') = 262144 = 512^2`, a perfect square, exactly as the
discriminant-under-enlargement identity requires
(`tests/maximal_order_bigint_test.cpp`). A `needs_another_round`
necessary-condition check is included. Fifth: that general
p-radical-from-structure-constants algorithm (Cohen §6.1.3, Ore's
Frobenius method) is now built (`GeneralOrder`,
`p_radical_from_structure_constants`) and available as a genuine C++
overload of `enlarge_order_round2_bigint` -- the fast, monogenic-
shortcut path stays default for round 1 (Z(β) is always monogenic
there), and the more expensive general path is what round 2+ would
need once the enlarged order may no longer have a single generator.
Cross-checked against the trusted monogenic-shortcut answer on the
same Dedekind cubic, at `p=2, n=3` (the harder `p<=n` regime a
trace-form shortcut wouldn't even cover): exact agreement, `-2012 ->
-503`, computed without ever factoring a defining polynomial
(`tests/general_order_radical_test.cpp`). Sixth: the one remaining gap
in that chain -- deriving a new `GeneralOrder`'s own structure constants
from an enlarged order's HNF basis -- is now closed
(`structure_constants_from_basis_change`), letting a caller chain
rounds to a fixed point rather than compute one round at a time
(validated only for a single-round-then-fixed-point case; a genuine
2+-round worked example was not available to test against and remains
open).

An earlier task dependency wrongly assumed Golod-Shafarevich
verification needed class number/class group computation first;
rereading Sawin's paper directly showed neither Lemma 11 (the
Golod-Shafarevich inequality itself) nor its use in the paper's main
construction needs a computed class group -- only primality testing,
prime enumeration, and the Legendre/Kronecker symbol, all of which were
also confirmed genuinely absent from the codebase (a real first-time
gap, not documentation drift). Seventh: `math/primality.hpp` now
provides `is_prime` (mini-gmp's own unused BPSW + deterministic
Miller-Rabin test), `next_prime`, `sieve_of_eratosthenes`, and
`kronecker_symbol` (Cohen Algorithm 1.4.10, written from scratch, since
mini-gmp has no Jacobi/Kronecker symbol at all), validated against
Euler's criterion for every odd prime <=97
(`math/tests/test_primality.cpp`, 58 checks). Eighth:
`include/adelic/golod_shafarevich.hpp` uses that machinery to check the
Lemma 11 inequality directly, and reproduces Sawin's own published
parameters (`T` of 13 primes, `S_Q` of 22 primes) exactly, including
the equality `36 <= 36` the paper reports and the precise
ramified/split/inert partition of `S_Q` it describes
(`tests/golod_shafarevich_test.cpp`, 16 checks). Class number/class
group computation remains unbuilt but is no longer on the critical
path to this verification; it stays independently interesting as its
own piece. Ninth: while starting task #6 (verifying Sawin's
Proposition 10 exponent formula, which needs logarithms), built
`bigfloat_log` and in doing so found a real, previously-undetected bug
in the pre-existing `bigfloat_exp` -- no argument reduction despite its
own header comment claiming otherwise, so `bigfloat_exp(-20)` (and
anything with `|x| >~ 20`) silently hit a hardcoded 1000-term
Taylor-series cap before converging. First looked like genuine
run-to-run nondeterminism in the shared math-library test binary; an
isolated reproducer showed it was fully deterministic once separated
from the rest of that binary's tiers, at which point the mechanism was
easy to see and fix with the standard `exp(x) = exp(x/2^k)^(2^k)`
range-reduction trick (mirroring `bigfloat_sin`/`bigfloat_cos`'s
existing reduction). `bigfloat_exp` had zero callers anywhere in the
repo before this, so the fix is purely an accuracy improvement, not a
behavior change any caller could depend on. Fifteen repeated runs of
the full test binary after the fix, all clean. Tenth: with
`bigfloat_log` actually working, task #6 itself is now done --
`include/adelic/sawin_exponent.hpp` implements Proposition 10's own
exponent formula (equation 11) directly and reproduces, independently
(not via Sage or any external CAS), the paper's own published
intermediate values for its worked example: numerator `3.8822...`,
denominator `275.055...`, and `delta = 0.014114...`, giving Theorem 1's
published exponent `1.014114` exactly to the paper's own stated
precision (`tests/sawin_exponent_test.cpp`, 8/8 checks, confirmed
deterministic across ten repeated runs given the earlier scare with
`bigfloat_exp`). Ten for ten so far: every property test either
confirmed existing machinery was more general than it had been
exercised as, found a real bug in already-shipped code, or produced a
genuine, independent reproduction of a published result. Both tasks
directly aimed at the OpenAI/Sawin unit-distance construction (#5, #6)
are now complete; only class number/class group computation (#4,
independently interesting, not on any critical path anymore) remains
open from this initiative.

Eleventh: task #4 itself, started. `include/adelic/quadratic_class_group.hpp`
computes class numbers of imaginary quadratic fields via binary
quadratic forms (Cohen Ch. 5), deliberately scoped to that case rather
than general-degree-n class groups (which would need LLL/Buchmann-style
machinery this project has no other use for). Checked against all nine
Heegner numbers (the complete list of negative fundamental
discriminants with class number 1) via an independently-constructed
principal form, several tabulated `h>1` cases, and a cross-check
between two different code paths -- `qform_reduce`'s iterative
transformation against `enumerate_reduced_forms`'s direct search --
which caught a real off-by-one bug in the reduction step's range
formula before it shipped. Eleven for eleven. Class number is done;
class *group* structure (Gauss composition, giving the actual group
law rather than just its order) is the next piece.

## Research directions

The direct next theorem work is the four-transition base seam, followed
by recurrent-SCC exhaustion/dominance. Parallel directions include:

- explaining the sparse Class-II boundary-layer slope structurally;
- extending contact-side family proofs to neighboring substitutions;
- constructing finite local carry quotients and bounded corrections;
- comparing geometric windows, labelled dynamics, twisted operators,
  zeta/L-function packages, and spectral embeddings with an explicit
  preservation/forgetting ledger;
- retiring Python prototypes only after bounded native parity or a
  documented correction.

## Validation commands

```sh
make class_ii_corona_literature_probe
make class_ii_symmetry_probe
make class_ii_bp_family_probe
make lean-check
make check
```

The detailed terminal transport audit is opt-in:

```sh
make class_ii_terminal_transport_probe
```
