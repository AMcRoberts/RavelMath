# Competitor-SCC symbolic mining: the core's formula holds, naive rank-matching does not

Status: one confirmed result, one honest negative result. Both from
`python/nbonacci_scc_pattern_miner.py`, a batch driver that runs
`nbonacci_arithmetic_hull --bound=1 n` in parallel across a range of `n`
(memory-budgeted per `n`, since exact-window state count grows ~3^n) and
fits exact-rational polynomials to each structural family, holding out the
largest sampled `n` for validation rather than fitting every available
point.

## Confirmed: the core-size formula

Fitting `nodes(n)` for the grade-1 [CORE] family on `n=3..8` (six points,
degree-5 exact interpolation) and validating against the held-out `n=9`:

```
predicted(9) = 856, actual(9) = 856  -> MATCH
```

This recovers `predicted_core_size(n) = (n-1)(5n^2-10n+6)/3`, already
known and implemented in `include/ravel/nbonacci_margin_invariant.hpp` --
not new mathematics, but a genuine validation that the mining
methodology (parallel batch collection, exact rational fitting,
out-of-sample holdout) works correctly when the target actually is a
polynomial in `n`.

## Negative result: competitor SCCs do not fit by naive rank-matching

The obvious next move -- group competitor SCCs by grade (support-count
bucket, matching `docs/NBONACCI_ARITHMETIC_DOMINANCE_PROGRAM.md`'s
"Graded recurrent structure": grade 1 = support 2/3, grade 2 = support
4/5, grade 3 = support 6/7) and rank (sorted by node count, largest
first within each grade), then fit a polynomial per (grade, rank) across
`n` -- **fails** for every grade-2 and grade-3 family tested, at every
`n=9` holdout:

| family | fit on n | fitted degree | predicted(9) | actual(9) | verdict |
|---|---|---:|---:|---:|---|
| grade=2 rank=0 | 4..8 | 4 | 4254 | 4282 | MISMATCH |
| grade=2 rank=1 | 4..8 | 4 | 1362 | 1372 | MISMATCH |
| grade=3 rank=0 | 6..8 | 2 | 1716 | 2808 | MISMATCH |
| grade=3 rank=1 | 6..8 | 2 | 1146 | 1829 | MISMATCH |
| grade=3 rank=2 | 6..8 | 2 | 1146 | 1829 | MISMATCH |
| grade=3 rank=3 | 6..8 | 2 | 302 | 472 | MISMATCH |

Before this holdout check, the raw finite differences of these same
sequences (computed directly, not through curve-fitting machinery) looked
suggestive: grade-2's fourth difference and grade-3's second difference
each came out constant across the available points. **That apparent
constancy was vacuous, not evidence** -- with only 5 points (grade-2) or
3 points (grade-3), the top finite difference has only one or two values,
which is "constant" by definition regardless of the true underlying
function; it takes at least two *independent* matching values at the top
difference level to mean anything, and the grade-3 sequences never had
more than one. This is a recurring pattern-mining trap, now caught here
mechanically by the holdout check rather than by a human noticing after
the fact.

**What this rules out**: these competitor SCCs are not simple
polynomial-in-`n` families under naive size-rank matching across `n`.
Plausible reasons, none confirmed here: the object matched as "grade-2
rank-0 at n=8" and "grade-2 rank-1 at n=9" may not be the same
*structural* family at all (a new sub-component could appear, merge, or
overtake another in size between consecutive `n`, silently swapping which
raw SCC gets rank 0); or the true invariant is not raw node count but
something else printed alongside it (`quotient` size, `translation_envelope`,
the exact `support=` distribution, or `rho` itself) that a rank-by-size
match obscures.

## Update: n=10 confirms this is not "needs more points"

Re-ran with the grade+period+rank grouping fix (matching families by
`(grade, period, rank)` instead of raw size, closing the n=4
trivial-cycle contamination described above) extended to `n=10`
(`python/nbonacci_scc_pattern_miner.py --n-max=10`, results cached in
`out/nbonacci_hull_cache/`). Every non-core family still mismatches, and
critically the mismatch **grows**, not shrinks, with more data:

| family | fit on n | predicted(10) | actual(10) | relative error |
|---|---|---:|---:|---:|
| grade=2 rank=0 | 5..9 | 8358 | 8386 | 0.3% |
| grade=2 rank=1 | 5..9 | 2678 | 2688 | 0.4% |
| grade=3 rank=0 | 7..9 | 6510 | 8962 | **27%** |
| grade=3 rank=1 | 7..9 | 4193 | 5729 | **27%** |
| grade=3 rank=3 | 7..9 | 1072 | 1452 | **26%** |

Grade-2's error stayed small and roughly stable (0.3-0.4%) across n=9 and
n=10 -- consistent with a genuine but higher-than-quartic-degree
polynomial, still worth pursuing with more points. Grade-3's error is an
order of magnitude larger and growing, which rules out "just needs a
higher-degree fit": a genuinely polynomial family's interpolation error
should shrink as more points anchor the fit, not blow up. The more likely
explanation is a **structural discontinuity** -- a new sub-component
type entering or an existing one splitting/merging -- somewhere in
`n=9..10`, which silently corrupts the rank-by-size matching even with
the period key included. This should be chased by matching on the
`support=` distribution shape directly (item 1 below), not by throwing
more `n` at the same size/period-based matching.

An `--auto-extend` mode was added to
`python/nbonacci_scc_pattern_miner.py` (keeps raising `n-max` and
rerunning, reusing a disk cache keyed by `(n, hull binary mtime)` so
already-computed `n` cost nothing, until a family matches or `--n-max-cap`
is hit) but was not run to its cap here -- the n=10 result already gives
a clear enough signal that raw size/period matching is the wrong
invariant, so blindly extending `n` further would mostly burn compute
confirming what is already visible.

## What a next attempt should do differently

1. **Match families by structural signature, not sorted size.** The hull
   tool already prints the exact `support=k:count,...` distribution per
   SCC (occupied-coordinate histogram) -- match SCCs across `n` by that
   *shape* (suitably generalized/normalized for the growing `n`), not by
   size rank. This is exactly the fix the Class-II 44-shape work needed
   too.
2. **Fit `rho` (or `translation_envelope`) instead of, or alongside,
   `nodes`.** `rho` is the quantity the actual dominance theorem cares
   about; it may have cleaner algebraic structure (it is an algebraic
   number satisfying a low-degree polynomial over `Q(beta)`, unlike raw
   node counts) even where node counts do not fit simply.
3. **Get more data points before trusting any fit.** Grade-3 only starts
   existing at `n=6` (support 6/7 requires it) and this run reached only
   `n=9` -- four points total, one used as holdout, leaving three to fit
   a family that (if genuinely polynomial) could easily be degree 3 or
   higher. Reaching `n=10,11,12` via the same batch pipeline
   (`python/nbonacci_scc_pattern_miner.py`, already memory-budgeted for
   this) is a bounded, mechanical next step before attempting new fits.

## Reproducing this

```sh
make nbonacci_arithmetic_hull
python3 python/nbonacci_scc_pattern_miner.py --n-min=3 --n-max=9 \
    --total-budget-mb=9000 --base-mb=32 --timeout=900
```

Runs in parallel (per-`n` memory cap doubles with `n`, sum of concurrently
running caps stays under `--total-budget-mb`), reports MATCH/MISMATCH
against the held-out largest `n` for every family with enough points to
attempt a fit.
