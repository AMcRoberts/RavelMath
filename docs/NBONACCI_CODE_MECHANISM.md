# The mechanism in the n-bonacci carry code

## Executive finding

The executable state transition is a shift register with one corrected
coordinate:

\[
 (x_0,\ldots,x_{n-1})\mapsto
 (x_1,\ldots,x_{n-1},x_0-\sum_{j>0}x_j+d),
 \qquad d\in\{-1,0,1\}.
\]

A boundary hit in coordinate `i>0` is transported to `i-1` by the shift. The
only place a new boundary hit can be born is the corrected tail. Long maximal
shell runs are therefore descending boundary witnesses with bounded tail
resets.

The C++ implementation confirms that this is already compressed into a
two-shape symbolic alphabet. `forward_descriptor` in
`include/ravel/nbonacci_margin_invariant.hpp` has exactly four cases:

```text
Root, a>0  : shift (d=0) or append an alternating triple (d=sign)
Root, a=0  : fold to a root (d=-sign)
Triple,a>0 : shift (d=sign)
Triple,a=0 : fold to a root (d=-sign)
```

`predecessor_descriptors` has at most two inverse candidates, independently of
dimension. The large shell word search is therefore observing a projection of
this Root/AlternatingTriple grammar; the missing theorem should be stated on
this descriptor automaton with the tail slack as its weight, rather than on
raw coefficient tuples.

There is already a candidate slack coordinate in the same header:
`predicted_distance_from_previous_shadow` assigns the new descriptor a crossing
distance `1`, `2`, or `3`, while `predicted_distance_to_previous_shadow`
assigns the return distance `1` or `2`. These are not heuristic norms; they are
index-gap formulas for exactly the cases that survive the descriptor grammar.
The concrete next proof task is therefore to prove that the accumulated
crossing distance dominates the shell survival rank. If that comparison is
established, the existing finite shadow induction becomes the sought
dimension-parametric escape certificate.

The existing executable test confirms this interpretation, rather than merely
suggesting it. `make nbonacci_margin_core_graph_test` followed by
`out/nbonacci_margin_core_graph_test` checks the direct descriptor graph through
`n=32` with zero failures. For every `n>=4`, the new alphabet layer has exact
outward distance at most `3` and return distance at most `2`; the canonical hub
has graph radius exactly `3n-1` in both directions. Thus the descriptor slack
is already a verified bounded local rank, and the remaining bridge is to relate
that graph layer to the coefficient shell-return graph.

The companion grammar regression is broader: `make nbonacci_margin_invariant_test`
executes 36,747,061 exact checks through `n=64` with zero failures. It verifies
descriptor round trips, affine-update closure, inverse tables, mirror symmetry,
pair-stratum counts, and transition deltas. The symbolic side of the proposed
lift is therefore extensively verified; only the coefficient-to-descriptor map
remains unproved.

## Where it lives

The exact implementation is in `include/ravel/nbonacci_margin_invariant.hpp`:
`displacement_from_descriptor` gives canonical sparse states,
`forward_descriptor` is the transition table, and
`predecessor_descriptors` is its finite inverse. The coefficient-level version
is `transitions` in `python/nbonacci_max_shell_return_probe.py` and
`python/nbonacci_shell_word_probe.py`. The Lean interface is `carryUpdate` in
`lean/nbonacci_margin_catalogue.lean`.

Lean checks the key interface:

```lean
carryUpdate_take_prefix (x : List ℤ) (d : ℤ) :
  (carryUpdate x d).take (x.length - 1) = x.tail
```

Thus the digit affects only the new tail; boundary transport is deterministic
and all branching is tail creation.

## What the certificates measure

For `[-M,M]^n`, the first-return graph follows all legal interior paths until
the next shell visit. A periodic outer-shell orbit would give a cycle, so a
strict longest-path rank is an exact finite escape certificate.

The rank is temporal survival depth, not a coordinatewise norm. Signed boundary
mask alone is insufficient. The replayable probe
`python/nbonacci_boundary_mask_rank_probe.py` reports this directly:

| dimension/bound | shell masks | varying-mask count | rank height |
|---|---:|---:|---:|
| `n=4,M=4` | 80 | 56 | 6 |
| `n=4,M=10` | 80 | 56 | 5 |
| `n=5,M=6` | 242 | 180 | 7 |

The missing state is interior slack (the deficits `M-|x_i|`, together with
the signed tail residual). The candidate universal automaton is therefore
boundary type plus slack/tail-reset data.

## Automation target

1. Encode a shell state by signed boundary mask, deficit vector, and tail sign.
2. Apply `carryUpdate` symbolically for each digit `-1,0,1`.
3. Quotient by the smallest equivalence preserving legal successors.
4. Emit a finite transition table and strict rank certificate.
5. Replay edges with exact integers and export the table to Lean, using
   `no_strict_rank_relation_closed`.

The normalized MILP is discovery machinery, not yet a proof certificate. The
shell-return rank is exact for each tested `(n,M)`, but the dimension- and
bound-independent slack closure remains the missing lemma.

The MILP can now optimize the perturbation threshold rather than merely test
one interval. For `n=4`, six transitions, and all 365 canonical digit words,
`--minimize-q` finds no feasible word below
`q=1/7`; the first threshold is attained by the word `-1-11-1-11`.
This is solver evidence (the eventual promotion still needs a replayable
Farkas/SMT certificate), but it supplies the first explicit quantitative
bridge from the homogeneous obstruction to the large-shell cutoff.

That margin has now been independently checked with exact QF_LRA. The
replayable probe `python/nbonacci_word_interval_smt.py` tests the strict
interval `0 <= q < 1/7` for every canonical word and returns
`365/365 UNSAT`, with no unknown results. It can emit a Z3 proof for an
individual branch with `--emit-proof`; the remaining compression is to share
the resulting linear certificate across all word branches.

The same independent scan at `n=5`, seven transitions, tests all 1,094
canonical words and returns `1,094/1,094 UNSAT` for `0 <= q < 1/10`.
At the next coarser endpoint `q < 1/9`, exactly five branches become SAT.
This gives a second exact rational margin and suggests that the cutoff is a
dimension-dependent integer slack invariant, rather than a numerical solver
artifact.

The automated margin-table driver now records the sequence
`k_2=3, k_3=4, k_4=7, k_5=10, k_6=14`, where `q<1/k_n` is uniformly forbidden
for the `n+2`-transition word family. The `n=6` endpoint was checked over all
3,281 canonical words: `q<1/14` is `3,281/3,281` UNSAT, while `q<1/13` has
14 SAT branches. This is a concrete target for discovering the general slack
recurrence.

The shared-selector verifier removes the word-enumeration bottleneck. It
encodes all ternary digits in one exact mixed-integer linear formula:
`python/nbonacci_shared_interval_smt.py`. It extends the endpoint data to
`k_7=20` and `k_8=28`: `q<1/20` is UNSAT for `n=7`, while `q<1/19` is SAT;
`q<1/28` is UNSAT for `n=8`, while `q<1/27` is SAT. This is the preferred
automation path for higher dimensions.

The shared margin scanner (`python/nbonacci_shared_margin_table.py`) confirms
`k_9=40`, `k_10=54`, and `k_11=80` with exact SAT/UNSAT endpoint checks. It
refuses to classify a timeout as a proof, so higher-dimensional runs remain
explicitly inconclusive when the arithmetic solver does not finish.

The homogeneous limit can now be checked independently with
`python/nbonacci_homogeneous_shell_smt.py` (`make
nbonacci_homogeneous_shell_smt`, now an explicit regression target that
fails if the pattern below breaks at any tested `n`). Exact rational SMT
gives the following stable pattern for every tested dimension `2<=n<=14`
(extended from the previously-checked `n<=10`; the Z3 witnesses printed
per `n` are not canonical -- Z3 returns *some* satisfying assignment, not
an extremal or otherwise structurally meaningful one, so do not read
patterns into the specific witness values). A scattershot spot check well
past the enrolled regression range -- `n=18` (0.21s), `n=25` (0.37s),
`n=40` (7.35s), each an isolated single-query run, not a systematic
sweep -- confirms the same `n+1` SAT / `n+2` UNSAT pattern holds with
solve times that stay cheap far beyond where a case-by-case exhaustive
box check (the strong ternary closure route, `docs/
NBONACCI_CONJUGATE_HEIGHT_BOUND.md`) becomes intractable. This is
evidence the pattern is robust, not evidence it is proven for literal
every `n`: SMT confirms finitely many instances exactly, it does not
substitute for the still-missing symbolic induction.

```text
 n+1 transitions: SAT
 n+2 transitions: UNSAT
```

The SAT witnesses are rational boundary chains; the UNSAT query is over real
linear arithmetic with exact equalities, not floating-point sampling.  This
strongly identifies the missing symbolic lemma: the homogeneous boundary
automaton has survival depth exactly `n+1`.  The remaining work is to extract a
generic Farkas/induction proof of that fact and then quantify the perturbation
from `q=0` to `q=1/M`.

**A concrete algebraic lead for that proof, found and verified but not yet
carried through**: the homogeneous block recurrence itself,
`a_{t+n+1} = 2*a_{t+1} - a_t` (see the block-defect identity above with the
forcing term set to zero), has characteristic polynomial `x^(n+1) - 2x + 1`.
This factors as `(x-1)` times the *reciprocal* of the n-bonacci polynomial:
writing `P(x) = x^n - x^(n-1) - ... - x - 1` for the n-bonacci polynomial,
`x^(n+1) - 2x + 1 = (x-1) * (-x^n * P(1/x))`, i.e. its roots are exactly
`{1} union {1/beta_k : beta_k a root of P}`. Verified both symbolically
(direct polynomial division for `n=2,3` by hand, matching for general `n` by
the reciprocal-polynomial identity) and numerically (root-by-root match to
`1e-15` for `n=3,4,5,6`, `python3` + `numpy.roots`). Since the n-bonacci
polynomial is Pisot (one root `beta>1`, every other root `|beta_k|<1`), this
means the homogeneous block map has exactly `n-1` *unstable* eigen-directions
(the reciprocals `1/beta_k` of the small conjugates, now `>1` in modulus),
one marginal direction (eigenvalue exactly `1`, the constant sequences), and
one stable direction (`1/beta<1`). A map with only one contracting direction
out of `n+1` is a plausible mechanism for why a bounded (shell-confined)
orbit cannot survive more than a bounded number of steps -- but this has not
yet been turned into the actual `n+1` survival-depth bound; it explains why
survival should be short, not yet the exact constant `n+1`.

**Two further attempts, one negative result and one real narrowing, neither
yet closing the lemma:**

- **Negative result: the naive dual-eigenbasis norm bound is too loose to
  use.** Writing `x_0 = sum_k c_k v_k` in the eigenbasis of the (separate,
  n-dimensional, not the `n+1`-dimensional block map above) homogeneous
  shift map `T` itself (eigenvalues `1/beta_k` for every root of the
  n-bonacci polynomial, one stable `k=1`, `n-1` unstable), bounding each
  `|c_k|` via the dual (biorthogonal) functionals `w_k` and combining via
  `1 = ||x_0||_inf <= |c_1| ||v_1||_inf + sum_{k>=2} |c_k| ||v_k||_inf`
  gives a survival-depth upper bound that is numerically vacuous (does not
  become infeasible for any `L` up to 199, tested `n=2..11`,
  `python3` script, not yet promoted to a committed file) because the
  stable term `||w_1||_1 ||v_1||_inf` alone already exceeds 1 and does not
  shrink with `L`. The single aggregate inequality throws away too much:
  it does not use that the same `L+1` constraints must hold
  *simultaneously* with a *specific* boundary-touching structure at every
  step, only their disjoint extremes. A tighter argument needs the joint
  structure, not a combined norm bound.
- **Real narrowing: the minimal Z3 unsat core is far smaller than the full
  query, and reveals genuine structure.** `python/
  nbonacci_homogeneous_shell_unsat_core.py` extracts Z3's minimal unsat
  core (via `assert_and_track`/`unsat_core()`) for the `n+2`-transition
  infeasibility query, instead of reading the full, unreadable resolution
  proof. Across every tested `n=2..6`, the minimal core needs only:
  two-sided bounds on `a_0`, two-sided (mostly) bounds on a single *run of
  later, consecutive* scalar indices of the sequence -- "coordinate `n-1`
  at step `t`" is literally `a_{t+n-1}`, so ranging `t` over consecutive
  steps ranges the underlying scalar index consecutively too, not spread
  across the whole `n`-wide window machinery -- and only one or two shell
  (boundary-touch) disjunctions, not one per window. This narrows the
  actual proof obligation from "reason about `n+3` whole `n`-dimensional
  windows" to "reason about `a_0` and one later run of consecutive scalar
  values," a real simplification.
- **Correction, found by building proper systematic multi-MUS
  enumeration** (`python/nbonacci_homogeneous_shell_core_enumerate.py` --
  MARCO-style: find a core, permanently retract one of its members from
  the assumption list, re-solve, repeat, rather than a single
  `assert_and_track` call or ad hoc reshuffling). The single-core version
  above reported gaps in the later run for `n=4,5`; those were themselves
  an artifact of which one arbitrary minimal core `assert_and_track`
  happened to return. Solving from the **full** assumption set first (no
  reshuffling) gives a clean, gap-free single later run for
  `n=2,3,4,5,7`. **`n=6` is a genuine, reproducible exception**: its
  natural minimal core has *two* disjoint later runs
  (`indices=[0,1,2,8,9,10,11,12,13]`, i.e. `{1,2}` and `{8,...,13}`), not
  one -- confirmed reproducible (same result from the systematic
  procedure, not a random artifact), not yet explained. Do not assume
  `n=6` fits the single-run pattern; do not assume it doesn't generalize
  to other `n` either -- only `n=2..7` checked. The specific indices are
  still **not shown canonical** (a different, non-blocking exploration
  order could in principle surface an equally-valid single-run core for
  `n=6` too -- not yet checked before this session ended).
- **A third, decisive negative result: box-boundedness alone is not the
  binding constraint.** Fixing `a_0=1` and treating `a_1,...,a_{n-1}` as
  free parameters in `[-1,1]`, every later value `a_n, a_{n+1}, ...` is an
  explicit linear function of those free parameters (direct consequence of
  `a_t = a_{t+1}+...+a_{t+n}`, the same recurrence in its cleanest form).
  Checking, via LP feasibility (no exact-boundary-touch requirement, just
  `|a_j|<=1`), how long a run of later values *can* stay inside the box:
  feasible for at least 24 consecutive later values, tested `n=2..8` --
  nowhere near the tight `n+1` survival bound. **This rules out any
  growth-rate/Diophantine-approximation mechanism** (the kind the
  conjugate-height and dual-eigenbasis attempts both implicitly assumed):
  staying inside the box is cheap for a long time. The entire `n+1`
  tightness comes specifically from requiring **exact** contact with the
  boundary (`|a_j|=1` for some coordinate) at *every* intermediate window,
  a genuinely combinatorial/discrete constraint, not a continuous
  dynamical one. Future attempts on this lemma should look for a covering
  or pigeonhole argument over the discrete choice of which coordinate
  touches the boundary at each step, not a spectral/growth-rate bound.

**The sharpest available statement of the remaining gap**, combining all
four findings above: fix `a_0=1` (WLOG, by the sign symmetry `a -> -a` and
the unsat-core finding that `a_0` is always pinned). The remaining `n-1`
free parameters `a_1,...,a_{n-1} in [-1,1]` linearly determine every later
`a_j` (`j>=n`) via `a_t=a_{t+1}+...+a_{t+n}`. Windows `t=0,...,L` are
covered (in the sense of "some coordinate in the window equals `+-1`") if
every `t` in `{0,...,L}` has some scalar index `j in [t,t+n-1]` with
`a_j=+-1`; each such pinned `j` covers an interval of up to `n` consecutive
`t`s. The `n+1` bound is therefore an interval-covering /
pigeonhole question: how many of the `L+1` required covering intervals can
be realized by *linear equalities compatible with only `n-1` free
parameters* -- not a geometric-growth question (ruled out above). This is
now a concretely stateable combinatorial-linear-algebra claim, not yet
proved; it is the recommended entry point for the next attempt, in
preference to re-deriving the spectral/norm-bound routes already shown not
to work.

For inspection, the probe can emit Z3's proof object for the terminal query:

```sh
python3 python/nbonacci_homogeneous_shell_smt.py \
  --n=4 --emit-proof=/tmp/n4_homogeneous.proof
```

That proof object is useful for mining a generic contradiction, but it is still
solver output; it is not yet the small independently replayable certificate
needed for a theorem promotion.

## Symbolic proof-mining tool (n=2..8, all dimensions)

`python/nbonacci_shell_covering_proof.py` is a symbolic proof-mining
tool for the same `n+1` SAT / `n+2` UNSAT pattern.  Where the
original `nbonacci_shell_covering_search.py` stops at the *first*
valid `(pin_indices, signs, solution)` triple, this tool enumerates
**all** valid candidates within the `(n-1)`-pin search strategy,
classifies each by structural features, emits a replayable symbolic
proof certificate per candidate, mines the gap pattern for a
closed-form fit across `n`, and attempts a structural comparison
between consecutive `n`'s simplest candidates.

Replay checker: `python/nbonacci_shell_covering_proof_check.py`.
For each JSON sidecar it re-derives every claim (linear system,
solution, sequence, gap pattern, box inequalities, cover property,
UNSAT failure breakdown) from scratch with exact rational arithmetic
and reports per-(n, L) and per-candidate PASS/FAIL.

Run:

```sh
make nbonacci_shell_covering_proof
# equivalent to:
#   python3 python/nbonacci_shell_covering_proof.py --n-min=2 --n-max=8 \
#     --emit-json-dir=out/nbonacci_covering_proof/ \
#     --mine-gap-formula --attempt-promotion
#   python3 python/nbonacci_shell_covering_proof_check.py out/nbonacci_covering_proof/
```

The Python tool applies a 12 GiB virtual-memory fence by default
(per the 2026-08-02 standing directive, "run big stuff with 12gb
memory fence; we've been crashed a bunch by not doing that").  The
fence is the same one the Z3-backed `nbonacci_homogeneous_shell_smt`
and `nbonacci_homogeneous_shell_unsat_core` probes use, with the
default raised from 10 GiB to 12 GiB.  Override via the
`RAVEL_PROBE_MEMORY_MB` environment variable; set to `0` to disable
(not recommended for `n >= 8`).

**C++ regression test** for the same symbolic closure:
`tests/nbonacci_covering_witness_test.cpp` (Makefile target
`nbonacci_covering_witness_test`, enrolled in `make check`).
Encodes the seven n=2..8 simplest candidates as hard-coded integer
data (LCM-of-denominators trick: multiply through by `denom` and
verify in `Z`; no floats, no Z3, no multiprecision), reconstructs
each sequence from the free-parameter vector via the homogeneous
recurrence, and re-verifies the four closure conditions
(pin equalities, free-parameter box, full-sequence box, cover
property) and a fifth sanity check (pin indices strictly
increasing when sorted).  Current run: 280 checks, 0 failures.
This is the same set of facts the Python replay checker
re-derives, in a different arithmetic system, with a different
machine-checkable artifact.  Either side passing is sufficient
evidence the closure is real; both passing is the regression-test
guarantee the symbolic witness stays valid across future changes.

### Simplest candidate per `n` (from this tool, n=2..8)

| n | valid candidates | simplest pins | signs | solution (a_1..a_{n-1}) | gap_pattern | distinct |a_j| | sequence max |a_j| |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 |   3 | (4,)                       | (-1,)                | (1,)                | ()        | 2 | 1 |
| 3 |   4 | (1, 5)                     | (1, -1)              | (1, 0)              | (4,)      | 2 | 1 |
| 4 |   8 | (1, 6, 8)                  | (1, -1, 1)           | (1, 0, -1/3)        | (5, 2)    | 4 | 1 |
| 5 |  33 | (1, 8, 9, 10)              | (1, -1, 1, -1)       | (1, 1/3, -1/3, 1/3) | (7, 1, 1) | 2 | 1 |
| 6 |  66 | (7, 8, 9, 10, 12)          | (1, 1, -1, -1, -1)   | (1, 1, 0, -1/2, 0)  | (1, 1, 1, 2) | 3 | 1 |
| 7 | 212 | (1, 10, 11, 12, 13, 14)    | (1, -1, 1, -1, 1, -1)| (1, 1/3, -1/3, 1/3, -1/3, 1/3) | (9, 1, 1, 1, 1) | 2 | 1 |
| 8 | 424 | (9, 10, 12, 13, 14, 15, 16)| (1, -1, 1, -1, 1, -1, 1) | (1, 0, -1/3, 1/3, -1/3, 1/3, -1/3) | (1, 2, 1, 1, 1, 1) | 4 | 1 |

(Every row: `n+1` SAT confirmed within the `(n-1)`-pin strategy.
Every row's UNSAT (`n+2` transitions) also confirmed by exhaustive
enumeration of `C(n+L-1, n-1) * 2^(n-1)` candidate strategies, all
failing for one of: `singular` linear system, `out_of_box_solution`
on `|a_k|`, `out_of_box_window` on the resulting sequence, or
`cover_incomplete` on the windows.)

### Multi-candidate observation

A non-trivial finding: the simplest candidate at `n = 5` (this
tool) differs from the first-found candidate reported earlier in this
doc and in `docs/READINGLIST.Minimax.TheoremAutomation.md` (which
was `(1, 2, 8, 9)`, gap `(1, 6, 1)`, sequence with three distinct
`|a_j|` values).  The new tool's simplicity sort (fewer distinct
`|a_j|` values, more zeros in the free-parameter solution, smaller
gap-pattern entropy) selects `(1, 8, 9, 10)` (gap `(7, 1, 1)`, only
two distinct `|a_j|` values) as the genuinely simplest.  Both
candidates are valid; the new one is what subsequent mining uses.

The candidate count itself is informative:

- `n = 2, 3, 4`: 3, 4, 8 candidates (the search strategy happens to
  be very rigid at small `n`)
- `n = 5, 6, 7, 8`: 33, 66, 212, 424 (rapid growth, but the
  structure of all candidates is similar)

### Structural comparison across `n`

The "movement through candidates" the user asked for: for the
simplest candidate at each `n`, the tool records (a) first pin
index, (b) last pin index, (c) whether the last pin equals
`n + L - 1` (the sequence end), (d) whether the tail pins form a
consecutive-integer cluster, (e) distinct `|a_j|` count, (f) sign
balance, and (g) the gap pattern itself.  Then it prints the
preserved/dropped features between consecutive `n`.

Findings:

- **First pin at index 1** for `n = 3, 4, 5, 7`; first pin at 7
  for `n = 6`; first pin at 9 for `n = 8`.  The "first pin at 1"
  pattern is **not** preserved across all `n`; the simplest
  candidate at `n = 6, 8` starts the pin set deeper into the
  sequence.
- **Last pin at sequence end** (`n + L - 1`) for all `n >= 4`.  This
  is a genuinely preserved structural feature.
- **Tail consecutive-pin cluster** of varying size: 0 at `n = 3, 4,
  6`; 2 at `n = 5`; 4 at `n = 7, 8`.  Not preserved across all `n`,
  but a real structural feature (the cluster size appears to grow
  with `n`, but not monotonically across even/odd).
- **Distinct `|a_j|` count** varies (2, 2, 4, 2, 3, 2, 4); not a
  preserved invariant.
- **Sign balance** varies; not preserved.

### Gap-formula mining

The tool tries three closed-form fits for the gap pattern across
`n`: constant, linear in `k`, bilinear `(A, B, C)` in
`(k, n)`.  All three fail out-of-sample at `n = 5` (holdout) for
the current 4-5 data points: the holdout mismatch is total, not
local.  The honest result: **no simple closed-form fit of the gap
pattern as a function of `n` and `k` is supported by the available
data** (matches the `docs/NBONACCI_SCC_PATTERN_MINING_NEGATIVE_
RESULT.md` discipline: a 3-5-point fit is suggestive, not
evidence).  The remaining work is to either extend to `n = 9, 10`
(via `python/nbonacci_shell_covering_proof.py --n-min=2 --n-max=10`
-- the cost at `n = 10` is `C(18, 9) * 2^9 ≈ 5M` candidates, ~15
minutes) and re-fit, or abandon the closed-form attempt and
proceed directly to the Lean formalization route documented below.

## Lean seam: charpoly of `inverseCarryMatrix n`

`docs/READINGLIST.Minimax.TheoremAutomation.md` and the comment in
`lean/nbonacci_margin_catalogue.lean` both record an open seam:
proving `(inverseCarryMatrix n).charpoly = nbonacciCharpoly n` for
literal every `n` (not just `n <= 8` via `native_decide`) would
remove the dimension-parametric index-arithmetic gap and, via the
existing `nbonacci_block_identity_of_charpoly`, give a fully
general-n Lean proof of `A^(n+1) = 2A - 1` for the carry map.

**Mathlib audit (this session, 2026-08-02).** Searched the local
Mathlib (`free_involution_perron/.lake/packages/mathlib/Mathlib/`)
for a companion-matrix charpoly lemma.  None exists.  Available
infrastructure is:
- `Matrix.charmatrix M = X*I - C(M)`, `Matrix.charpoly M = det (charmatrix M)`;
- `Matrix.det_of_upperTriangular` / `det_of_lowerTriangular` (det = product of diagonal for triangular matrices);
- `Matrix.det_succ_column` (Laplacian expansion along a column);
- `Matrix.charpoly_fromBlocks_zero₁₂` (block triangular charpoly factorization).

**The route**, recorded in `lean/nbonacci_margin_catalogue.lean`
as a roadmap comment (with no `sorry` holes, to keep the file
kernel-checked):

1. Cofactor-expand `M_n = charmatrix (inverseCarryMatrix n)` along column 0:
   `det M_n = X * det(qMatrix n) + (-1)^n * det(rMatrix n)`.
2. `det rMatrix n = (-1)^(n-1)` by `Matrix.det_of_lowerTriangular`.
3. `det qMatrix n = 1 + X + ... + X^(n-1)` by induction on n with
   the induction step using cofactor expansion of `qMatrix (n+1)`
   along its last row `[1, 1, ..., 1, X+1]`.
4. Combine: `det M_n = X * (1 + X + ... + X^(n-1)) + (-1)^n * (-1)^(n-1) = X + X^2 + ... + X^n - 1 = nbonacciCharpoly n`.

Estimated ~80 lines of Lean for steps 1-4; the two sub-lemmas are
the meat (each ~30 lines, one triangular-product argument, one
cofactor-induction argument).  Left as work-in-progress for a
later session with focused Lean time; the gap-formula route in
`docs/NBONACCI_CODE_MECHANISM.md` is mathematically the same fact
and is already documented and machine-checked for `n = 2..8`.

The algebra explains why this is the right limit.  Writing
`S_t = a_t+...+a_{t+n-1}`, the carry identity gives
`S_{t+1}=a_t+d_t` and hence
`a_{t+n}=2a_t-S_t+d_t`.  Eliminating `S_t` one step later yields the exact
block defect

```text
a_{t+n+1} - 2*a_{t+1} + a_t = d_{t+1} - d_t,
```

For normalized states the digit is `q d_t`, so the right side is
`q(d_{t+1}-d_t)`, with magnitude at most `2q`.  At `q=0` it vanishes; finite
`q=1/M` shells are therefore a bounded perturbation of the homogeneous
`(n+1)`-block recurrence.  This is the algebraic interface the next
certificate generator should use instead of expanding all coefficient paths.

The identity itself is independently replayed by
`python/nbonacci_block_defect_smt.py`; the current scan proves the negation
UNSAT for every block position in dimensions `2` through `10`.
The elimination step is now also formalized as
`blockDefect_of_sum_update` in `lean/nbonacci_margin_catalogue.lean`, and the
catalogue compiles cleanly.

There is a useful asymptotic reduction already exposed by the same code. After
normalizing by `M`, the digit contribution is `q=1/M`. At `q=0` the machine is
the homogeneous shift/tail map. Exhaustive canonical-word search for
`n=4`, six transitions tests all 365 words and finds none feasible. By
compactness, an exact proof that every homogeneous six-window shell schedule
is impossible would imply a positive margin for each finite schedule and
therefore exclude all sufficiently large integer shells. The remaining finite
range can then be discharged by the exact shell-return certificates. This is
the most promising automation seam for turning numerical discovery into the
universal lemma: replace the MILP result by replayable rational Farkas/SMT
certificates, then compute the resulting margin explicitly.

```text
carry update -> boundary shift -> tail-reset branches
             -> finite slack automaton -> strict shell rank
             -> no periodic exterior orbit
```
