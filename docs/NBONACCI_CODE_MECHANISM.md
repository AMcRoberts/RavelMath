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

```text
carry update -> boundary shift -> tail-reset branches
             -> finite slack automaton -> strict shell rank
             -> no periodic exterior orbit
```
