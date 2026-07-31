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

The global occurrence theorem is not closed. Its remaining state-level
seam consists of four exceptional base transitions:

1. signed-contact reverse inclusion at the neighbor base -- **partially
   closed (2026-07-31)**: the *unsigned* raw 27-state self-closure and
   its Red-pruning to 25 survivors are now proved for every `a>=3`
   (window validity abstractly, Red pruning symbolically from `tau_a`'s
   own fixed word forms), not just checked at sampled `a`. The exact
   relationship to the full *signed* 50-state contact set is now a
   verified identity (`class_ii_neighbor2_signed_contact_set()` equals
   the unsigned survivors' mirror-closure, exactly) rather than an
   assumption, though whether that identity is itself the complete
   answer the base-premises table means remains open. The Red-pruning
   argument's arithmetic core is now Lean-kernel-checked
   (`lean/class_ii_round1_red_pruning.lean`, sorry-free, signed) --
   the combinatorial exhaustiveness fact behind it is still taken as
   given C++-checked input, not re-derived in Lean. The natural
   remaining candidate for "does `±C` equal the closure of some
   symmetric seed" -- seeding the same pipeline from `D_cont ∪
   mirror(D_cont)` -- was tried and refuted (2026-07-31): it reproduces
   the same 32-state result as the already-refuted mirror-only seed, a
   strict 32-of-50 subset of `±C` that is not even itself self-mirror-
   closed, checked at `a` in `{3..8,20,50}`. `±C`'s correctness rests
   on the `c_corona`/`same_letter_H` construction argument, not on any
   closure-of-a-symmetric-seed characterization. See
   `docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s Round 1 sections
   for the full account, including a caught-and-corrected overclaim
   about mirror-closure preserving correctness.
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
   integer `a>=7`** by the same argument. The actual "connect to the
   round>=5 universal theorem" claim this item names is a separate,
   harder bridging argument, still not attempted.
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
