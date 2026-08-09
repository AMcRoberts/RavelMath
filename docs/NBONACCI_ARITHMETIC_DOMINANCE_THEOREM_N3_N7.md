# The n-bonacci arithmetic-dominance theorem, closed for n=3,4,5,6,7

> **SUPERSEDED, 2026-08-08.** This describes an abandoned proof route. The theorem this file targets IS closed -- via a different route (the canonical Q/R split), Lean kernel-checked and independently reconfirmed. See `NBONACCI_UNIVERSAL_DOMINANCE_CLOSED_2026-08-08.md` for the current, authoritative status before reading further.

Status: a genuine closure of the theorem this whole program has been
chasing (`docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`, "The arithmetic
reversal reduces the open proof to four explicit lemmas"), for five
concrete values of `n`. Not a universal (all-`n`) closure -- see "Exact
scope" below before citing this as more than it is.

## The theorem

> For `n in {3,4,5,6,7}`, the dominant Perron root of the n-bonacci
> boundary/corona graph `G_B` equals the dominant Perron root of the
> formula-defined grade-1 core (`predicted_core_member` in
> `include/ravel/nbonacci_margin_invariant.hpp`): `rho(G_B) = rho(core)`.

This is precisely what the program's own "arithmetic reversal" section
names as sufficient: "occurrence of the direct core in `G_B` gives the
lower bound `rho(G_B)>=rho(core)`, while containment of every `G_B` cycle
in the core-dominated arithmetic hull gives the reverse inequality."

## Why this now closes (the exact logical chain)

Two independently-developed results, neither sufficient alone, combine
exactly:

1. **Every cyclic orbit of the arithmetic hull is ternary, for `n<=7`.**
   `app/nbonacci_arithmetic_hull.cpp`'s labelled graph, restricted to any
   coefficient bound, is built from the *same* carry update
   (`forward_displacement`) as `app/nbonacci_carry_cycle_probe.cpp`'s raw
   box automaton: a labelled edge `(i,x,j) -> (i',x',j')` uses
   `x' = forward_displacement(x, delta)` with
   `delta = [j>0] - [i>0] in {-1,0,1}`, exactly the digit condition of the
   periodic carry-bound lemma. So any cyclic SCC of the arithmetic hull's
   `x`-coordinates *is* an instance of a periodic sequence satisfying that
   lemma's hypothesis. `docs/NBONACCI_CONJUGATE_HEIGHT_BOUND.md` proves
   (high-precision numeric + exact finite cross-check) that for `n=2..7`
   every such sequence is ternary (`|x_i|<=1` for every coordinate, every
   `t`). Consequently **no cyclic SCC of the arithmetic hull can have any
   coefficient outside `[-1,1]`**, for `n<=7` -- not just "none was found
   at the tested bounds," a proof that none exists at *any* bound.
2. **Within the ternary box, the core exactly dominates every other
   cyclic SCC, for `n=3,4,5,6,7`.** Re-verified directly (not merely
   trusted from prior doc prose) by rerunning
   `./out/nbonacci_arithmetic_hull --exact --bound=1 3 4 5 6 7`: for every
   one of these five `n`, `core_SCC=EXACT` (the found dominant-labelled
   SCC matches the generated core node-for-node: sizes 14, 46, 108, 210,
   362, matching `predicted_core_size(n)` exactly) and every other cyclic
   SCC in the ternary box gets `exact: core>competitor` from
   `mathlib::compare_perron_roots_exact` (exact rational Collatz-Wielandt
   brackets, not a floating-point estimate). `arithmetic-supergraph
   dominance: estimate=PASS exact=PASS` for all five.

Combining: for `n=3,4,5,6,7`, the complete set of cyclic SCCs anywhere in
the arithmetic hull is exactly the set found within the ternary box (by
(1)), and among those the core strictly dominates every competitor (by
(2)). So the core dominates the *entire* arithmetic hull, not just its
ternary restriction.

The arithmetic hull is, by construction, an over-approximation of the real
corona graph `G_B` (`docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`, "The
reversal": "this graph deliberately over-approximates the paper corona...
dominance in the hull is an upper bound for every contained corona
graph"). Since the core itself is a legitimate cycle of `G_B` (it is
exhibited as such independently of the hull construction) and `G_B`'s
cycles are a subset of the hull's cycles, dominance of the core in the
(strictly larger) hull forces dominance of the core in `G_B` too:
`rho(core) <= rho(G_B) <= rho(hull) = rho(core)`, squeezing
`rho(G_B) = rho(core)` exactly.

## A cleaner, more direct route for n=3,4,5,6 -- found by re-checking an
## existing tool, not new computation

`app/nbonacci_dominance_ledger.cpp` (Makefile target
`nbonacci_dominance_ledger`) was already sitting in this project,
apparently never connected to a "the theorem is closed" declaration
anywhere in the canonical status docs (`docs/THEOREM_STATUS.md`,
`docs/RESEARCH_STATUS.md` have no mention of `G_B`, `dominance_ledger`, or
`full-G_B` at all). Unlike the arithmetic hull, it constructs the actual,
literal corona `G_B` via the standard literature corona/reduction
iteration (`c_corona`, `red`, `backward_closure`, `red_anode` --
`Thuswaldner`-style fixed-point induction) and only stops when the
reduced set reaches a genuine fixed point (`corona: ... converged=YES`).
This sidesteps the over-approximation-squeeze argument in the previous
section entirely: it checks core occurrence, recurrent decomposition, and
exact dominance directly on the real `G_B`, not an inflated superset.

Rerun directly (`./out/nbonacci_dominance_ledger --exact 3 4 5 6`):

| n | \|G_B\| | fixed_round | candidate occurrence | cyclic_SCCs | dominance certificate |
|---:|---:|---:|---:|---:|---:|
| 3 | 29 | 3 | 14/14 EXACT | 1 | PASS |
| 4 | 113 | 3 | 46/46 EXACT | 3 | PASS |
| 5 | 382 | 3 | 108/108 EXACT | 3 | PASS |
| 6 | 1221 | 4 | 210/210 EXACT | 7 | PASS |

`rho(G_B) = rho(core)` for `n=3,4,5,6`, directly, with no dependence on
the periodic carry-bound lemma or the hull's over-approximation argument
at all -- these four are closed at the *dominance certificate's* rigor
tier (exact rational Collatz-Wielandt throughout), not gated by the
carry-bound lemma's numeric tier.

**`n=7` does not extend this way**: `nbonacci_dominance_ledger --exact 7`
was run and did not finish within 180 seconds (the direct corona
construction is evidently far more expensive per step at `n=7` than the
arithmetic hull's face-filtered enumeration). `n=7`'s closure therefore
still depends on the hull-over-approximation argument in the previous
section, and is at the carry-bound lemma's weaker (numeric) rigor tier.
Whether `nbonacci_dominance_ledger` could reach `n=7` with more time or a
larger `--exact-iters`/round cap was not investigated further here.

**Revised summary**: `n=3,4,5,6` closed at exact-rational tier via direct
corona construction (this section); `n=7` closed at
high-precision-numeric-plus-exact-finite-check tier via the hull
over-approximation argument (previous section). Both routes agree
node-for-node on the core and on cyclic-SCC counts wherever both were run
(n=3..6), which is itself a useful independent cross-check between two
differently-implemented constructions of related but not identical
graphs.

## Exact scope -- read before citing this

- **Not universal in `n`.** The upper end is limited by (1): the periodic
  carry-bound lemma is only closed for `n<=7` (see
  `docs/NBONACCI_CONJUGATE_HEIGHT_BOUND.md`, "What remains open" --
  `n>=8` needs a fundamentally sharper argument, not a bigger box). `n=2`
  is outside the *lower* end because `nbonacci_arithmetic_hull.cpp`'s
  labelled-graph framework itself requires `n>=3`
  (`label_pairs: n must be >= 3`) -- there is no dominance-theorem
  question posed at `n=2` in this framework to close.
- **Rigor tier is bounded by the weaker of the two pieces.** Result (2)
  (the dominance certificate) is exact rational arithmetic throughout --
  the strongest tier this project uses short of Lean. Result (1) (the
  carry bound) is high-precision numeric (60-digit mpmath root isolation,
  not interval-certified) plus an exact integer finite-box cross-check.
  **The combined theorem is therefore at result (1)'s tier: a
  high-precision numeric argument plus exact finite verification, not
  yet a fully certified (interval-arithmetic or Lean) proof.** Do not
  describe this as Lean-checked or interval-certified until
  `docs/NBONACCI_CONJUGATE_HEIGHT_BOUND.md`'s own open items are closed.
- **`bound=2` spot-checked for `n=3,4,5`** (`./out/nbonacci_arithmetic_hull
  --exact --bound=2 3 4 5`) as an extra sanity cross-check: identical
  cyclic-SCC counts and `exact=PASS` at the larger box, consistent with
  (but not a substitute for) result (1)'s proof that no larger box adds a
  cycle.
- **This closes one specific theorem** (arithmetic dominance,
  `rho(G_B)=rho(core)`) for five values of `n`. It does not by itself
  close the separate, larger "global occurrence theorem" threads recorded
  elsewhere in this project (Class-II recurrent-SCC exhaustion, the
  Tetrabonacci margin work, etc.) -- those are different objects.

## Reproducing this

```sh
make nbonacci_dominance_ledger
./out/nbonacci_dominance_ledger --exact 3 4 5 6   # direct, exact-tier, n=3..6

make nbonacci_conjugate_height_bound   # closes the carry-bound lemma, n=2..7
make nbonacci_arithmetic_hull
./out/nbonacci_arithmetic_hull --exact --bound=1 3 4 5 6 7   # n=7 needs this route
```

Expect `core_SCC=EXACT` and `arithmetic-supergraph dominance: estimate=PASS
exact=PASS` for every one of the five `n`, and (from the first command)
`nonternary_cyclic=0` at a box exceeding the analytic bound for
`n=2,3,4,5,6,7`.

## What remains for a fuller closure

1. **Extend the carry-bound lemma past `n=7`** -- the real bottleneck
   (see `docs/NBONACCI_CONJUGATE_HEIGHT_BOUND.md`'s diagnosed
   geometry-of-numbers gap). Every additional `n` closed there
   immediately extends this dominance theorem too, *provided* the
   dominance certificate (piece 2) also holds at that `n` -- not yet
   checked past `n=7` here.
2. **Promote result (1) to exact/interval-certified or Lean strength.**
   The `n=3` case has a concrete shortcut already noted in
   `docs/NBONACCI_CONJUGATE_HEIGHT_BOUND.md`: the conjugate pair's
   modulus is `1/sqrt(beta)` exactly (Vieta), so only the real cubic root
   needs a rational interval, not the full complex pair.
3. **State and check the dominance certificate for `n=8..whatever` `n`
   the carry-bound lemma eventually reaches**, rather than assuming it
   continues to hold -- `nbonacci_arithmetic_hull.cpp`'s own default
   dimension list stops at `n=6`; the extension to `n=7` used here was
   passed explicitly and should be re-run, not assumed, for any new `n`.
