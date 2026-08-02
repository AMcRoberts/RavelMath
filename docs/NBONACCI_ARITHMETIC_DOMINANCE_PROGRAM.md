# N-bonacci arithmetic dominance program

Status: theorem-discovery machinery and exact finite certificates, not yet an
arbitrary-`n` dominance theorem.

## The reversal

The legacy route constructs the paper corona, reduces it, decomposes the full
`G_B` into cyclic SCCs, and then asks which SCC has the largest Perron root.
That route is trustworthy at small dimension but makes the irrelevant
intermediate corona—not the final recurrent graph—the computational
bottleneck.

For n-bonacci, a labelled displacement `(i,x,j)` has an exact arithmetic
description:

- face validity is the exact `Q(beta)` inequality
  `-v_i < <x,v> < v_j`;
- the edge displacement is
  `x' = M^{-1}(x + delta e_0)`, with
  `delta=[j>0]-[i>0]`;
- the parent labels are the finite n-bonacci parent table.

`app/nbonacci_arithmetic_hull.cpp` therefore enumerates a coefficient box,
filters it with Sturm-isolated exact `Q(beta)` signs, installs the symbolic
edges, and computes the cyclic SCCs. This graph deliberately over-approximates
the paper corona. If every corona cycle lies in the arithmetic hull and the
formula-defined core is already dominant in the hull, no corona exhaustion
theorem is needed.

This is stronger than a faster implementation of the paper algorithm: extra
arithmetic states and edges are allowed, so dominance in the hull is an upper
bound for every contained corona graph.

## Exact finite result

With coefficient box `[-1,1]^n`, the formula in
`include/ravel/nbonacci_margin_invariant.hpp` identifies exactly one cyclic
SCC for every `3<=n<=8`. Exact rational Collatz--Wielandt brackets prove that
its Perron root is strictly larger than every other cyclic SCC.

| n | exact-window labelled states | cyclic SCCs | core nodes | core rho (diagnostic) | largest competitor rho (diagnostic) |
|---:|---:|---:|---:|---:|---:|
| 3 | 108 | 1 | 14 | 1.395337 | none |
| 4 | 548 | 3 | 46 | 1.623354 | 1.000000 |
| 5 | 2,390 | 3 | 108 | 1.756228 | 1.290597 |
| 6 | 9,594 | 7 | 210 | 1.836484 | 1.507473 |
| 7 | 36,666 | 7 | 362 | 1.886782 | 1.655611 |
| 8 | 135,416 | 17 | 574 | 1.919376 | 1.753639 |

The arithmetic hull is genuinely larger than `G_B`: for example, at n=6 its
largest competing SCC has 236 nodes, while the legacy corona graph has the
known 221-node component. The enlarged 236-node component is still exactly
dominated. Thus the result is not accidental equality between two
constructions.

Measured on the current host:

- n=4,5,6 together: 1.03 seconds, 14.8 MB peak RSS;
- n=7,8 together: 26.27 seconds, 179 MB peak RSS;
- exact Perron separation for all n=7 competitors: 5.17 seconds, 46 MB;
- exact Perron separation for all n=8 competitors: 28.04 seconds, 179 MB.

The implementation intentionally uses comparison-heavy `std::set` objects.
Packing a ternary translation into a base-3 integer would substantially lower
the n=8 memory figure; optimization is not yet necessary for the theorem
search.

## Graded recurrent structure

Every cyclic SCC found through n=8 lies in one adjacent support pair:

- grade 1: support 2 or 3—the direct core;
- grade 2: support 4 or 5;
- grade 3: support 6 or 7;
- terminal full-support strata: permutation cycles with rho exactly 1.

The earlier “composite hop” observation is therefore a grading, not an
isolated Hexanacci defect. A grade-2 displacement is a sum of elementary core
displacements; the n=7/8 data expose the next grade exactly where the support
budget permits it.

Increasing the coefficient box to `[-2,2]^n` for `3<=n<=5` creates thousands
of additional exact-window states but no new cyclic state at all. Every cyclic
translation returns to the ternary layer. This makes a recurrence/maximum or
finite-carry lemma a realistic replacement for an assumed coefficient bound.

The first reduction of that lemma is now automated in
`python/nbonacci_periodic_carry_probe.py`. Writing a state as the periodic word
`x_t=(a_t,...,a_{t+n-1})` reduces the carry update to
`delta_t=-a_t+a_{t+1}+...+a_{t+n}`, with `delta_t` a signed digit. The probe
uses integer UNSAT queries to rule out `|a_t|>=2` on an explicit rectangle of
dimensions and periods. This is a bounded certificate generator, not yet the
uniform maximum principle; a future promotion must prove the periodic-word
claim for arbitrary period (or supply a finite carry-state quotient).

The finite carry-state quotient is now implemented in
`app/nbonacci_carry_cycle_probe.cpp`. It enumerates the whole box
`[-B,B]^n`, permits all three signed digits at every state, and removes
vertices that cannot lie on a directed cycle. It reports no non-ternary cyclic
state for `n=3..8, B=2`; the cyclic set is unchanged for `n=3..6, B=3` and
`n=3..5, B=4`. Since this probe forgets the beta window and face labels, these
are stronger finite carry certificates than the arithmetic-hull experiments.
The remaining promotion target is precise: prove that every periodic orbit
enters one of these finite boxes (or prove a direct maximum principle), after
which the recurrent coefficient bound is no longer empirical.

Sign-chamber mining is available through
`python/nbonacci_sign_chamber_probe.py`. Sign alone is too coarse: at
`n=3,B=6` it leaves a 12-chamber quotient SCC and two self-loops. Refining by
the weak ordering of absolute magnitudes removes neither phenomenon entirely
(46-chamber quotient SCC, two self-loops). The next chamber refinement should
therefore encode magnitude gaps or residues together with signs; this negative
result prevents a false “sign-only” escape lemma.

The Python probes install a 10 GiB virtual-memory cap by default
(`RAVEL_PROBE_MEMORY_MB=10240`); set that variable to `0` only for a deliberate
larger run. The corresponding Make targets cap the native carry probe at the
same limit with `ulimit`, so failed synthesis attempts terminate as bounded
experiments rather than destabilizing the workshop process.

The combined refinement is now testable as `--mode=gaps-mod --modulus=auto`
(which selects modulus `n+1`):
the chamber records signs, exact coordinate-wise gaps from the smallest
absolute value, and that smallest value modulo `n+1`. The transient quotient
is not stable as the box grows: at `n=3,B=8` it has small cycles again. The
stronger affine check assigns an integer offset to each chamber and verifies
that a radial statistic plus that offset increases on every transient edge.
The `--rank-min` statistic `min_i |x_i| + offset(chamber(x))` passes several
small boxes but fails at `n=4,B=8`. The `--rank-base=sum` statistic
`sum_i |x_i| + offset(chamber(x))` repairs that failure and passes under the
memory cap for `n=3,B=30`, `n=4,B=12`, `n=5,B=6`, and `n=6,B=4`. The next
`n=4,B=15` expansion passes under the requested 10 GiB cap (503,460 chambers;
peak RSS about 545 MiB). Extending to `n=4,B=20` also passes (1,332,130
chambers; peak RSS about 3.3 GiB). The earlier 1 GiB `MEMORY_LIMIT` was
therefore only a Python representation boundary, not a rank failure.
The next-dimensional run `n=5,B=8` passes as well (855,564 chambers; largest
quotient SCC 14).

The dimension-matched residue (`modulus=auto`, meaning `n+1`) remains part of
the chamber phase. Modulus four fails at `n=4,B=6`, while modulus five repairs
it, supporting the `n+1` prefix-period signal. This is still finite-box
evidence, but it has now produced a concrete two-layer target: a bounded
sign/gap/residue phase plus an `L1` radial drift certificate. A flat affine
formula in the gap coordinates alone is infeasible already at `n=3,B=8`, so
the chamber offset is genuinely piecewise rather than a hidden three-parameter
linear norm.

Reducing the gaps themselves modulo `n+1` is too aggressive: the resulting
finite phase fails at `n=3,B=30` and `n=4,B=10`, even after adding scalar
quotient-weighted radial terms. Exact relative gaps, or an equivalent richer
carry state, are therefore required by the current evidence.

For regression, the unrestricted carry automaton at `B=2` leaves the following
numbers of cyclic states (all ternary):

| n | box states | cyclic states | non-ternary cyclic |
|---:|---:|---:|---:|
| 3 | 125 | 15 | 0 |
| 4 | 625 | 37 | 0 |
| 5 | 3,125 | 95 | 0 |
| 6 | 15,625 | 255 | 0 |
| 7 | 78,125 | 699 | 0 |
| 8 | 390,625 | 1,947 | 0 |

## Intertwiner search

For a shell block `S` and core block `C`, a nonnegative matrix `P` satisfying

`S P <= P C`

pulls the positive core Perron vector back to a positive shell subeigenvector.
If the inequality is strict somewhere and `S` is irreducible, classical
strict Perron--Frobenius gives `rho(S)<rho(C)`.

`python/nbonacci_intertwiner_search.py` finds unconstrained quotient-level
intertwiners. These are useful diagnostics but not explanations: a sufficiently
general cone map can merely repackage an already true spectral inequality.

`python/nbonacci_restricted_intertwiner_search.py` instead permits mass only
where the core translation is an elementary summand of the shell translation
and the remainder has the required lower grade. Results so far:

- both n=4 competitors: strict decomposition-supported intertwiners;
- both n=5 competitors: strict intertwiners;
- n=6 smaller grade-2 competitor: strict intertwiner;
- n=6 largest grade-2 competitor: minimal-decomposition relation is
  infeasible, so cancellation or a two-stage phase transport is genuinely
required;
- terminal components can be discharged independently by their permutation
  matrices and rho=1.

A relaxed relation allowing nonminimal cancellation is also infeasible for
the largest n=6 shell. Direct positive grade-lowering maps into the core or
the one-particle carry graph are therefore ruled out for that component under
the tested elementary-decomposition supports. A finite phase cover or a
symbolic Collatz weight remains viable; the failed maps should not be cited as
the dominance mechanism.

An exterior-power probe likewise rejects the most literal version of the
particle analogy: the raw core adjacency has several subdominant eigenvalue
moduli greater than one, so its ordinary compound-matrix products do not match
the shell Perron roots. The observed support grading may still arise from a
normalized carry/transfer operator, but that is a new target, not a result.

The failed deterministic and equal-label projections are retained as useful
negative results: the grade-lowering map must transport face phase and, at the
first large n=6 shell, cannot use only minimal decompositions.

## Universal theorem obligations

The arithmetic reversal reduces the open proof to four explicit lemmas.

1. **Recurrent coefficient/carry bound.** Every cyclic exact-window orbit of
   the n-bonacci displacement recurrence is ternary, or at least lies in a
   fixed arithmetic hull for which the following lemmas hold. The existing
   finite carry-automaton machinery is the natural generator/checker.
2. **Core identification.** The grade-1 cyclic block is exactly the direct
   predicate in `nbonacci_margin_invariant.hpp`. Its edge grammar, counts, and
   strong connectivity are already generated independently.
3. **Grade exhaustion.** Every other cyclic orbit lies in one higher-support
   grade, with the full-support remainder a permutation component.
4. **Uniform dominance certificate.** Produce either a parametric
   grade-lowering intertwiner or a symbolic positive subeigenvector for each
   nonterminal grade. The current LP searches are support miners for this
   certificate, not final proofs.

Once these are proved, occurrence of the direct core in `G_B` gives the lower
bound `rho(G_B)>=rho(core)`, while containment of every `G_B` cycle in the
core-dominated arithmetic hull gives the reverse inequality. No formula for
the full transient corona and no equality between the corona and arithmetic
hull is required.

## Commands

```sh
make nbonacci_dominance_ledger
make nbonacci_arithmetic_hull
make nbonacci_intertwiner_search
make nbonacci_restricted_intertwiner_search

./out/nbonacci_arithmetic_hull --exact --bound=1 7 8
./out/nbonacci_arithmetic_hull --bound=2 3 4 5
make nbonacci_periodic_carry_probe
make nbonacci_carry_cycle_probe
make nbonacci_sign_chamber_probe
```

The first command is the independent legacy-corona regression through n=6.
The second and later commands are the active theorem-discovery route.
