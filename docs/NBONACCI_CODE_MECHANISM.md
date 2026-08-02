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

The homogeneous limit can now be checked independently with
`python/nbonacci_homogeneous_shell_smt.py`.  Exact rational SMT gives the
following stable pattern for every tested dimension `2<=n<=10`:

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

For inspection, the probe can emit Z3's proof object for the terminal query:

```sh
python3 python/nbonacci_homogeneous_shell_smt.py \
  --n=4 --emit-proof=/tmp/n4_homogeneous.proof
```

That proof object is useful for mining a generic contradiction, but it is still
solver output; it is not yet the small independently replayable certificate
needed for a theorem promotion.

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
