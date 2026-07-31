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
   given C++-checked input, not re-derived in Lean. See
   `docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s Round 1 sections
   for the full account, including a caught-and-corrected overclaim
   about mirror-closure preserving correctness.
2. round-two raw reverse inclusion and Red exclusion -- precisely
   scoped (2026-07-31): `T_2 = B_2 union E_2` exactly (checked as a
   literal identity, not sizes); `E_2`'s window validity needs no new
   derivation (its 25 states reduce entirely to bounds already proved
   for the fixed-24 catalogue and the interior tip theorem, plus one
   state that's a one-line corollary of the same bounds). The actual
   open gap is Red exclusion for 123 states across three ranks
   (98, 15, 10) -- bounded (`|x_k|<=3`), not unbounded-in-`a` the way
   Round 1's two pruned states were, so no quick escape-hatch argument
   applies. Red's ranking now has an independent exact finite
   certificate at `a=6,7,8` (all 123 pruned states' edges
   re-derived via `simple_forward_targets_exact` and checked against
   `red_anode`'s own ranks, not just trusted from it) -- a symbolic
   argument for every `a` remains open and is a materially larger
   undertaking than Round 1's (some states have 200+ raw forward-target
   candidates before window filtering).
3. round-three raw reverse inclusion and Red exclusion -- same shape
   as round two (`T_3 = B_3 union E_3` exact, 93 = 68 + 25; Red
   exclusion 256 raw / 163 pruned, independent exact finite
   certificate, zero violations at `a=6,7,8`); its 163 pruned states
   are also confirmed the literal same coordinate set across
   `a=6..30`, matching round two.
4. the round-four reverse/exclusion bridge into the stable theorem --
   its raw-corona occurrence/exclusion shape is also now checked to
   match rounds two/three (`T_4 = B_4 union E_4` exact, 113 = 88 + 25;
   325 raw / 212 pruned, zero violations, pruned states also
   `a`-independent), but the actual "connect to the round>=5 universal
   theorem" claim this item names is a separate, harder bridging
   argument not attempted by that check.

Across all four rounds, the "are the pruned-state coordinate sets
literally `a`-independent" question is now resolved: **no**, not
uniformly. Round 1 is a genuine outlier (pruned coordinates affine in
`a`, small 27-state raw target, already closed symbolically and now
Lean-formalized for its arithmetic core). Rounds 2/3/4 share a second
pattern instead: a fixed, `a`-independent pruned-state list with only
the forward-edge weights varying by `a`. This means the still-open
symbolic Red-exclusion proof is one argument applicable to all three
of rounds 2/3/4, not three independent derivations.

**Sharper still (2026-07-31, background continuation):** the entire
forward-edge connectivity graph over each round's raw pre-Red states
-- not just the pruned node set -- checks out `a`-independent too, at
7-8 sampled `a` values per round including one far outside the
cluster (Round 2 stress-tested at `a=50`), with a single genuine
exception at Round 4's `a=6` (resolved by `a>=7`, this project's
established general stability threshold elsewhere). This is a
substantially cheaper target than a per-state affine-in-`a` argument,
but is not yet a proof: it still needs the general lemma that
forward-edge coordinates are always affine in `a` (established only
for Round 1's two states so far), which combined with the
`a`-independent node sets would make the observed graph agreement
provably universal rather than an exact finite certificate. See
`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s "Sharper still"
section for the full account and precisely what remains.

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
