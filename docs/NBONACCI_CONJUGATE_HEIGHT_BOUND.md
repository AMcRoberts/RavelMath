# The conjugate-height bound: a period-independent carry bound

Status: two claims of different strength, do not conflate them.

- **Weak form (the doc's own stated acceptable target), proved for every
  `n>=2`:** every periodic `a:Z->Z` with n-window defect in `{-1,0,1}`
  satisfies `|a_t| <= B(n)` for an explicit, computable, period-independent
  constant `B(n)` (roughly linear in `n`; see the table below). This holds
  for literal every `n`, not sampled -- the derivation never fixes `n`.
  This is real, previously-absent progress on sublemma 1 of the "arithmetic
  reversal" list in `docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`
  ("Recurrent coefficient/carry bound... It is enough to prove the weaker
  fixed-hull version with an explicit bound `B(n)`"). It does **not** by
  itself close the broader n-bonacci arithmetic-dominance theorem: that
  also needs the core-identification, grade-exhaustion, and
  uniform-dominance sublemmas established at this same growing `B(n)`,
  which `app/nbonacci_arithmetic_hull.cpp` has so far only checked at the
  much smaller fixed box `[-1,1]^n` (`[-2,2]^n` for `n<=5`, no new cyclic
  state found).
- **Strong form (the actual ternary claim, `B(n)=1`), proved for
  `n=2,3,4,5,6,7` only:** by cross-checking the weak-form bound against the
  exact finite carry automaton at a box exceeding `B(n)`, no periodic
  admissible sequence exceeds the ternary layer for these seven values of
  `n`. `n>=8` is not reached by this cross-check (direct box exhaustion
  becomes computationally infeasible there; see "What remains open" below).

Both claims are at high-precision-numeric (60-digit mpmath) plus
exact-finite-check strength, not yet Lean-checked or
interval-arithmetic-certified.

## The claim

> **Periodic carry-bound lemma.** Let `a : Z -> Z` be periodic, and suppose
> `d_t = -a_t + a_(t+1) + ... + a_(t+n)` lies in `{-1,0,1}` for every `t`.
> Then `|a_t| <= 1` for every `t`.

The weaker, still-sufficient target (per the existing doc) is a fixed,
`n`-dependent, period-independent bound `B(n)`, provided the arithmetic-hull
core and its dominance certificate are proved for that same `B(n)`.

## The mechanism: one lemma, two directions

This is the "Algebraic-height route" already named in
`docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`, carried through to an
actual bound rather than left as "numerical estimates, discovery evidence
only."

For `beta` any root of the n-bonacci polynomial `x^n - x^(n-1) - ... - x - 1
= 0`, define `c_j = beta^j - (1 + beta + ... + beta^(j-1))` for
`j = 0, ..., n-1`, and `h(x) = sum_j c_j x_j`. This is exactly
`forward_descriptor`'s underlying linear structure applied at the
coefficient level. Re-deriving the coefficient-matching equations for
`h(x') = ...` against `forward_displacement` (the exact `carryUpdate`
in `include/ravel/nbonacci_margin_invariant.hpp`) shows the identity

```
beta * h(x') = h(x) + delta
```

holds for **every** root `beta` of the characteristic polynomial (the
derivation only uses the characteristic equation, not which root); this is
verified directly against `forward_displacement` for random integer states,
not just derived on paper (`python/nbonacci_conjugate_height_bound.py`,
`identity` column, 500+ trials per `n`, zero failures for `n=2..14`).

The n-bonacci constant is Pisot: its dominant root `beta>1` and every other
root `beta_k` satisfies `|beta_k| < 1` (numerically confirmed for
`n=2..14`; classical for multinacci constants generally). This gives **one
lemma applied in two directions**, not two separate arguments:

- **Dominant root** (`|beta| > 1`, forward direction is a contraction):
  `e_{t+1} = (e_t + d_t) / beta`. If `M = max_t |e_t|` is attained at `t*`
  (finite since `a`, hence `e`, is periodic), then
  `M = |e_{t*}| <= (|e_{t*-1}| + 1)/beta <= (M+1)/beta`, so
  `M <= 1/(beta-1)`. Tight.
- **Conjugate roots** (`|beta_k| < 1`, backward direction is a
  contraction): rewrite as `e_t = beta_k * e_{t+1} - d_t`. The same
  max-attained argument, applied to this direction, gives
  `M <= |beta_k| M + 1`, so `M <= 1/(1 - |beta_k|)`. Tight.

(An earlier draft of this bound used a looser telescoping-sum estimate,
`1/(1-|beta_k|)^2`, derived by summing a geometric series over one full
period instead of using the max-attained trick in the contracting
direction. That looser bound is superseded here — it produced `B(n)`
values 3-4x too large to be useful for a box the exhaustive checker can
run at `n>=5`. Caught by re-deriving the argument symmetrically with the
dominant-root case rather than trusting the first derivation.)

## From conjugate heights back to the raw coefficient

`h_1, ..., h_n` (one per root) is an invertible change of basis from the
window state `x_t = (a_t, ..., a_{t+n-1})`, since the `n` roots are
distinct (standard fact for this irreducible family; not independently
re-derived here). Writing `a_t = x_t[0] = sum_k lambda_k h_k(x_t)` with
`lambda = (C^T)^{-1} e_0` (`C` the matrix of `c_j^{(k)}` coefficients),

```
|a_t| <= B(n) := sum_k |lambda_k| * R_k,   R_1 = 1/(beta-1),  R_k = 1/(1-|beta_k|) for k>=2
```

`python/nbonacci_conjugate_height_bound.py` computes this at 60 decimal
digits of working precision (mpmath), independently re-derives the
Pisot property numerically, and verifies the identity against
`forward_displacement` before trusting the bound.

## Exact values and cross-check against the finite carry automaton

| n | beta | B(n) (tight formula) | box radius checked | exhaustive result |
|---:|---:|---:|---:|---:|
| 2 | 1.618034 | 1.8944 | 2 | `nonternary_cyclic=0` (9 cyclic states) |
| 3 | 1.839287 | 2.1966 | 3 | `nonternary_cyclic=0` (15 cyclic states) |
| 4 | 1.927562 | 2.8685 | 3 | `nonternary_cyclic=0` (37 cyclic states) |
| 5 | 1.965948 | 3.6762 | 4 | `nonternary_cyclic=0` (95 cyclic states) |
| 6 | 1.983583 | 4.5699 | 5 | `nonternary_cyclic=0` (255 cyclic states) |
| 7 | 1.991964 | 5.5296 | 6 | `nonternary_cyclic=0` (699 cyclic states; 62,748,517 states, 5.9 GiB peak RSS, 20s wall) |
| 8 | 1.996031 | 6.5438 | needs 7 (15^8 ≈ 2.56 billion states) | infeasible: extrapolating `n=7`'s measured per-state cost puts `n=8,bound=7` at roughly 240 GiB, far past the 10 GiB probe memory convention |

Each row `n<=6` was produced by running
`./out/nbonacci_carry_cycle_probe --n=N --bound=B --print-witness` at a box
radius `B` strictly larger than the analytic `B(n)`, i.e. the exhaustive
finite check covers every integer state the analytic bound could possibly
allow plus margin. Zero non-ternary cyclic states were found at any of
these radii, and the cyclic-state counts (9, 15, 37, 95, 255) exactly match
the previously-recorded ternary counts from the smaller `bound=2` box in
`docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`'s regression table --
enlarging the box past the analytic bound adds no new cyclic state.

**Combining the two results**: for `n=2,3,4,5,6,7`, every periodic sequence
satisfying the digit condition has `|a_t| <= B(n)` (analytic argument,
holds for *every* period, not sampled), and the exhaustive finite check
independently confirms no periodic orbit inside a box of that radius (or
larger) is non-ternary. This closes the periodic carry-bound lemma for
those five values of `n`, modulo the honest gaps below.

## What remains open

1. **The analytic bound itself is a high-precision numeric certificate
   (60 decimal digits, mpmath), not an exact interval-arithmetic or
   Lean-checked proof.** The Pisot property and the root values are
   confirmed to 60 digits, not certified by Sturm isolation or a rational
   interval. Promoting this is mechanical but not yet done -- for `n=3`
   in particular, there is a clean shortcut: the two conjugate roots'
   common modulus satisfies `|beta_2|^2 = 1/beta` exactly (Vieta,
   product of roots = 1 for this cubic), so only the real root `beta`
   needs a rational interval, not the complex pair individually.
2. **`n>=8` is out of reach for the exhaustive check as currently
   implemented.** `B(n)` continues to grow roughly linearly in `n`
   (`6.54` at `n=8`, `13.44` at `n=14`; see
   `python/nbonacci_conjugate_height_bound.py --n-max=14`), and the box
   volume `(2B(n)+1)^n` grows explosively -- `n=8,bound=7` alone needs
   about `2.56` billion states, versus `n=7,bound=6`'s `62.7` million
   (measured 5.9 GiB peak RSS; the linear extrapolation puts `n=8` at
   roughly 240 GiB). `app/nbonacci_carry_cycle_probe.cpp`'s internal
   safety cap was raised from 3,000,000 to 100,000,000 states after
   confirming `n=7,bound=6` actually fits comfortably (the real safety
   net is the standard 10 GiB probe `ulimit`, not this constant). Two
   independent routes forward from here, neither attempted:
   - **Sharpen `B(n)`.** The current bound sums `|lambda_k| * R_k` by
     worst-case triangle inequality across all `n-1` non-dominant roots,
     which assumes every conjugate height simultaneously attains its
     individual worst case with matching phase. That is almost certainly
     not achievable by a single real digit sequence `d_t`; a sharper joint
     argument (not attempted here) could plausibly bring `B(n)` down
     enough to keep the exhaustive check tractable for a few more `n`.
   - **Reimplement the exhaustive checker with a flat/CSR memory layout.**
     `app/nbonacci_carry_cycle_probe.cpp` currently allocates one
     `std::vector<uint32_t>` per state for both in- and out-edges; a flat
     array of fixed-degree-3 edges (every state has at most 3 outgoing
     transitions, one per digit) would cut the per-state overhead
     substantially and might reach `n=8` or `n=9` without a sharper bound.
3. **Distinctness of the `n` characteristic roots** (needed for the
   `C` matrix to be invertible) is asserted as standard, not
   independently re-derived or checked here.

## Mined diagnosis of the remaining gap (not speculation -- measured)

Where does the slack between `B(n)` and the true value `1` actually live?
Computed directly against the real, already-verified cyclic ternary state
sets for `n=3..7` (`nbonacci_carry_cycle_probe`'s own certified state
lists, re-derived independently in Python and cross-checked to match the
known cyclic counts `15,37,95,255,699` exactly):

| n | `B(n)` (naive combination) | `Σ|lambda_k|·(true per-root max\|h_k\|)` | true `max\|a_t\|` |
|---:|---:|---:|---:|
| 3 | 2.197 | 1.575 | 1 |
| 4 | 2.869 | 2.304 | 1 |
| 5 | 3.676 | 2.458 | 1 |
| 6 | 4.570 | 3.138 | 1 |
| 7 | 5.530 | 3.415 | 1 |

Substituting each root's *actual* achieved maximum (measured on the real
cyclic states, not the a priori `R_k` formula) only partially closes the
gap, and the *remaining* gap after that substitution grows with `n`, not
shrinks. This rules out "the per-root bounds are just loose" as the
explanation. The individual per-root bounds are themselves reasonably
tight (55-100% of true, measured directly) -- the dominant root bound is
tight to `84-99%` and gets tighter as `n` grows.

**The real cause: this is a geometry-of-numbers problem, not a per-coordinate
one.** `a_t = Σ_k lambda_k h_k(x_t)` is the value of a fixed linear
functional. `Σ|lambda_k|·R_k` is the exact maximum of that functional over
the *box* `{h : |h_k| <= R_k for every k}` -- correct as a bound on the
box, achieved at the box corner `h_k = sign(lambda_k)·R_k`. But the actual
achievable set of `h(x_t)` values, as `x_t` ranges over integer states
compatible with *some* periodic admissible sequence, is a proper subset of
that box: each `R_k` is individually achieved by *some* periodic sequence,
generally not the *same* sequence for every `k` simultaneously (extremizing
`h_k` requires an integer digit sequence resonant with `arg(beta_k)`; a
different `k` requires resonance with a different, generically
incommensurate angle). The box bound therefore over-counts a region no
real periodic orbit reaches, and the over-count compounds as `n` grows
because there are more conjugate directions to (independently) worst-case
against.

This means the naive per-eigendirection combination is the wrong tool for
a tight bound at general `n` -- not merely under-optimized. Closing this
gap for literal every `n` needs a genuinely joint argument over the lattice
`{x in Z^n : x_t \text{ is a window of some admissible periodic sequence}}`
intersected with the box (a successive-minima / Minkowski-type statement,
or a direct simultaneous-inequality argument), not a sharper per-root
constant. This is also, plausibly, why the extensive earlier
chamber/gap/residue mining in this same document never found a clean
closed-form potential: the true admissible region is not box-shaped in any
of the coordinate systems tried.

A second, unexplored route with real potential: this whole lemma is very
likely a special case of classical Pisot-numeration literature already
partially adopted in this project (`include/adelic/csy_carry_automaton.hpp`,
Carton-Sudbery-Yassawi, Theorem 3 and Lemma 43; Frougny-Solomyak's
Condition F, which multinacci/n-bonacci numbers are a textbook example of
-- `refs/FullText/arXiv_2606.30496_..._Pisot-numerations-topological-groups.txt`
lines 245-247 and its Frougny [Lot02, Ch. 7] citation). Condition F concerns
boundedness of the carry when *adding* two bounded-digit numbers in a Pisot
base; our lemma concerns a *periodic* bi-infinite sequence with bounded
local defect. The two are closely related (both are about bounded-digit
closure under the same carry automaton) but not verified here to be
literally the same statement -- translating one into the other precisely,
rather than re-deriving the bound from scratch, is likely the fastest real
route to an `n`-independent closed proof and has not been attempted.

## Where it lives

`python/nbonacci_conjugate_height_bound.py` is the reusable, documented
probe: identity verification, Pisot check, and the `B(n)` table, all at
configurable working precision. It has no Makefile target yet (add one,
following this project's convention, before the next session treats it as
enrolled regression rather than a one-off check).
