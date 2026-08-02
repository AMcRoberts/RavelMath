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

The homogeneous recurrence has a sharper exact identity, checked by
`tests/nbonacci_block_identity_test.cpp` for every `2<=n<=40`:

`A^(n+1) = 2*A - I`, where `A=M^(-1)` is the carry map. Over one phase block,
the digit forcing is exactly

`(-e0+2e_last) d0 + sum_{r=1}^{n-1}(e_{r-1}-e_r) d_r + e_last d_n`.

Thus the observed `n+1` phase is an algebraic consequence of the companion
matrix, and the forcing is a bounded discrete derivative of the digit word.
The remaining carry lemma can now be attacked through this block map: outside
the ternary layer, `2A-I` supplies radial growth while the displayed forcing
is uniformly bounded. Run the exact regression with
`make nbonacci_block_identity_test`.

The Lean file now exposes the same implication at the matrix level and
kernel-checks the geometric-sum premise for the explicit inverse-incidence
matrix in dimensions `n=2,...,8` by executable finite instances. The remaining
formal step is consequently index arithmetic for arbitrary `n`, rather than
identifying the matrix or guessing the block law.

The arbitrary-dimension reduction is now explicit in Lean: by
Cayley--Hamilton, it suffices to prove that the inverse-incidence matrix has
characteristic polynomial `X^n + X^(n-1) + ... + X - 1`. The theorem
`nbonacci_geomSum_of_charpoly` turns that polynomial certificate into the
geometric-sum premise without expanding matrix powers. This leaves a standard
companion-characteristic-polynomial calculation as the next formal target.

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
quotient SCC 14). The first higher shell at `n=6,B=5` also passes (1,043,824
chambers; no nontrivial quotient SCC). The `n=7,B=3` run passes too (491,178
chambers; no nontrivial quotient SCC).

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

### Scoped missing lemma: periodic carry escape

The missing lemma that blocks promotion from the finite certificates to the
full theorem is the following precise statement.

> **Periodic carry-bound lemma.** Let `a : Z -> Z` be periodic, and suppose
> `d_t = -a_t + a_(t+1) + ... + a_(t+n)` lies in `{-1,0,1}` for every `t`.
> Then `|a_t| <= 1` for every `t`.

It is enough to prove the weaker fixed-hull version with an explicit bound
`B(n)`, provided the arithmetic-hull core and its dominance certificate are
proved for that same `B(n)`. The ternary statement is the preferred target
because it closes directly onto the generated grade-1 core.

The proof should be split into four independently checkable sublemmas:

1. **Block algebra.** The already-formalized identity `A^(n+1)=2*A-I` and
   the displayed bounded forcing formula.
2. **Forcing bound.** An exact coordinate/L1 bound for every block forcing word
   `d_0,...,d_n`; this is finite digit arithmetic and should be proved in Lean
   and exhaustively regression-tested in C++.
3. **Exterior escape.** Outside the ternary layer, the block map plus bounded
   forcing strictly increases a certified rank (the current exact-gap chamber
   plus `sum_abs` offset is the candidate). This is the genuinely dynamical
   sublemma; it must be stated as a finite chamber transition theorem with a
   parameterized boundary case, not as a floating-point spectral assertion.
4. **Periodic contradiction.** Iterate the strict rank increase around a
   purported periodic orbit to obtain a contradiction, then recover the
   ternary bound.

Automation plan and acceptance gates:

- `nbonacci_block_identity_test` and the Lean matrix/characteristic-polynomial
  bridge certify sublemma 1.
- `python/nbonacci_block_forcing_probe.py` is now the exact digit-word
  enumerator for sublemma 2 (via `make nbonacci_block_forcing_probe`). It checks
  every word through `n=10`, finds maximum coordinate `4`, and observes the
  sharp `L1` bound `2n+2` for even `n` and `2n` for odd `n`. The coarse Lean
  promotion is now complete: it proves `-4 <= F_i <= 4` and
  `sum_i |F_i| <= 4*n`. Every reported extremizer is replayable from the
  closed forcing formula; the sharp parity refinement remains optional.
- Extend `nbonacci_sign_chamber_probe.py` to emit a canonical chamber table,
  exact transition deltas, and a machine-readable difference-constraints
  certificate for sublemma 3. This is now available through
  `--emit-certificate=PATH`; `python/nbonacci_chamber_certificate_check.py`
  replays the JSON and rejects positive difference-constraint cycles. The
  certificate must pass at increasing boxes under the 10 GiB cap and use
  integer arithmetic only.

The first emitted higher-dimensional certificate passes at `n=4,B=8`:
82,194 canonical chambers, 149,778 weighted edges, and a 9.2 MiB JSON
artifact replayed successfully by `nbonacci_chamber_certificate_check.py`.
The next-dimensional controlled run also passes at `n=5,B=4`: 58,954
canonical chambers, 96,824 weighted edges, and a 6.3 MiB replayable artifact;
the quotient had no nontrivial SCCs at this bound.
The next controlled point passes at `n=6,B=3`: 117,394 canonical chambers,
178,978 weighted edges, and a 13 MiB replayable certificate, again with no
nontrivial quotient SCCs.
The larger known passing shell `n=5,B=6` also emits and replays successfully:
371,198 chambers, 611,658 weighted edges, and a 41 MiB certificate.
The next shell `n=6,B=4` also passes: 531,186 chambers, 812,926 weighted
edges, and a 57 MiB replayable certificate, with no nontrivial quotient SCCs.
The `n=7,B=3` shell also emits and replays successfully: 822,844 chambers,
1,181,154 weighted edges, and an 87 MiB certificate, again with no
nontrivial quotient SCCs.
The `n=8,B=2` shell also passes: 388,678 chambers, 523,824 weighted edges,
and a 41 MiB replayable certificate, with no nontrivial quotient SCCs.

An offset-stability check at `n=3` is a useful negative result: comparing the
`B=8` and `B=12` certificates, 3,798 of 4,388 shared chamber signatures have
different canonical offsets. Thus the universal potential cannot depend only
on signs, gap residues, or a bounded phase label; absolute gap scale remains
part of the parameterized rank data.
An attempted refinement that also records `floor(min_abs/(n+1))` does not
stabilize the potential: at `n=3`, bounds 8 and 12 still differ on 4,042 of
4,898 shared signatures. Scale enters through the exact gap geometry, not a
single radial quotient.

The capped vectorized raw-block test (`make nbonacci_block_l1_growth_probe`)
confirms why the chamber potential is needed: minimum raw `L1` changes are
`-15` at `n=3,B=8` and `-13` at `n=4,B=6`. Thus the exact block identity and
bounded forcing do not by themselves yield radial `L1` escape.
- Add a checker that verifies every directed cycle in the certified exterior
  graph has positive total rank change (equivalently, no zero/nonpositive
  cycle survives); this is the finite obstruction audit for sublemma 4.
- The periodic-contradiction endpoint is now kernel-checked as
  `no_strict_rank_finite` in `lean/nbonacci_margin_catalogue.lean`: a strict
  integer rank increase on a finite exterior state set is impossible. The
  remaining work is only to instantiate its finite state set and rank from a
  parameterized chamber certificate.
- Only after these gates pass should the result be promoted from bounded
  evidence to a universal carry theorem. The four broader obligations below
  then consume that theorem as their recurrent coefficient bound.

### Maximum-shell return reduction

The chamber rank search exposed a cleaner finite reduction. For a fixed box
`[-M,M]^n`, form the outer shell `max_i |a_i| = M` and compute the exact
first-return relation: paths may move through the interior, but stop at their
first subsequent shell visit. A periodic orbit whose global maximum is `M`
would induce a directed cycle in this shell-return graph. Therefore an
acyclic graph, together with its longest-path integer rank, is already a
finite exterior-escape certificate; it does not require a guessed affine
formula on all chambers.

This is implemented by `python/nbonacci_max_shell_return_probe.py` and
replayed by `python/nbonacci_max_shell_return_check.py`. Under the 10 GiB cap,
the emitted n=3, M=8 certificate has 1,538 shell states and 4,638 return
edges; n=4, M=8 has 32,896 states and 143,538 edges. Both replay with strict
rank increase. The same relation passes n=4 at M=3,4,6,8,10 and n=5 at M=3.
The stability driver reports n=4 as PASS for every M=2,...,12; its longest
shell-window rank drops to 5 and then remains 5 from M=8 through M=12.
Backtracked maximal chains show that this rank is a bounded survival depth:
the shell can persist for only finitely many consecutive windows before the
carry update leaves the shell. The remaining universal step is therefore to
prove a parameterized bound on that survival depth from the block identity,
rather than stabilize a single global chamber potential.

The logical endpoint is kernel-checked in
`lean/nbonacci_margin_catalogue.lean` as
`no_strict_rank_relation_closed`: a nonempty finite closed return relation
cannot have a strictly increasing integer rank on every edge. The shell JSON
checker supplies exactly those finite edge inequalities; only the uniform
shell-survival theorem is still missing.

### Algebraic-height route

The exact carry update also supplies a dimension-parametric scalar identity.
If `β` is the n-bonacci root and
`c_j = β^j - (1 + β + ... + β^(j-1))`, then
`h(x) = Σ c_j x_j` satisfies `β h(x') = h(x) + d`. The same identity holds
in every conjugate embedding. On a periodic word, closure bounds each
conjugate height by a geometric digit sum; Vandermonde inversion then gives a
finite coefficient bound `B(n)`. This is a second route to the missing lemma:
automate certified root-modulus intervals and the inverse-embedding bound,
then feed the resulting finite hull into the exact arithmetic-hull machinery.
The current numerical estimates are only discovery evidence, not yet a proof.

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
