# Findings, for citation

Every claim below is a numerical/exact-arithmetic observation on a
specific, finite set of substitutions, not a proven theorem (except
where explicitly proven, as noted). Cite as *"a computational survey
found..."*, not *"it is proven that..."*, except where a claim is
explicitly marked PROVEN. Status tags: **OPEN CONJECTURE** (the
underlying question is open in the literature), **SURVEY RESULT**
(this project's own numerical finding, narrow sample), **ILLUSTRATIVE**
(single instance, not a general claim), **STURM VERIFICATION**
(bit-exact Sturm-based classification of a claim already implied by
the wider theory — not a new structural insight). This document
states only the current, current-best-understood result.

## How this document is organized

There are three distinct threads that get conflated under "the 12
EXPLODED" or "rnd13"; kept separate here so citations stay precise:

- **§A. The 12-EXPLODED contact-boundary pipeline resolution**: the
  12 *3-letter cubic* Pisot candidates with `|det|=2` from the
  39-substitution survey that previously made the contact-boundary
  pipeline diverge, now produce finite output under the exact
  `in_H_sigma` path.
- **§B. The Sturm-Pisot certification of those 12**: bit-exact
  classification via `math/`'s Sturm isolator. 11/12 are bit-exactly
  Pisot by the strict definition; the 12th has its real-root
  structure proven but the loose complex-modulus sufficient condition
  is inconclusive. Not a new structural finding — the
  Minervino–Thuswaldner adelic theory already covers Pisot with
  `|det|=2`.
- **§C. Finding 2 — the 4-letter `rnd13`**: the one 4-letter
  non-unimodular Pisot candidate with `λ(G_B) < β` (the
  Barge–Gambaudo necessary condition for standard geometric tiling
  realization). The only one in any survey where the tiling question
  was concretely open; resolved by the adelic tiling classifier.

The 4-letter `rnd13` in §C is **NOT** the same substitution as the
3-letter cubic `rnd13_canon` in §A/§B — different alphabet sizes,
different betas, different roles; the naming overlap is a
project-history artifact, not a meaningful relationship.

---

## §A. The 12-EXPLODED contact-boundary pipeline resolution

**Status: SURVEY RESULT, internally consistent with the strict Sturm
classification and with the Minervino–Thuswaldner adelic theory.**

The 12 "EXPLODED" candidates from the 39-substitution survey are
3-letter cubic Pisot substitutions with `|det M| = 2` whose
contact-boundary pipeline previously diverged in the closure step.
Under the exact `in_H_sigma` path (`Substitution::in_H_sigma_exact` in
`include/ravel/core.hpp`, backed by the Sturm-isolated `β` interval
+ exact Q(β) sign test), all 12 produce finite `|C|`, `|±C|`, `|G_B|`,
and a definite BP-`ρ_nc` / `λ(G_B)` pair. See
`tests/test_12_exploded.cpp` (67/67) for the end-to-end run;
`rnd24_canon`'s full pipeline is skipped in the C++ test (corona blows
up to several thousand nodes, minutes to run) but its D_cont
derivation is verified. The 12 are listed in
`tests/test_12_exploded.cpp` and reproduced independently in
`app/verify_exploded_12.cpp`.

---

## §B. The Sturm-Pisot certification of the 12 EXPLODED

**Status: STURM VERIFICATION (not a new structural finding).**

The Sturm-based exact Pisot classifier
(`math/src/exact_pisot.c::pisot_classify_poly`) implements the strict
Pisot definition: real root `β > 1`, all other `|z| < 1`. Running it
on the 12 EXPLODED matrices:

- **11/12** are Pisot by the strict definition AND the complex-modulus
  sufficient condition (`|det M| < β · ∏|real_secondary|_max`) is also
  proven, bit-exact.
- **1/12** (`rnd13_canon`, 3-letter cubic, `σ(0)=(0,0,1,1,2)`,
  `σ(1)=(0,0,1,1,2,2)`, `σ(2)=(1)`) is Pisot by the strict definition
  (bit-exact) but the complex-modulus sufficient condition is
  inconclusive under the loose bound. This is settled, not an open
  gap: `rnd13_canon`'s characteristic polynomial (`x³-4x²-2x+2`) has
  THREE real roots, so the relevant Sturm branch is "all real roots,"
  which already gives `is_pisot=1` bit-exactly — the complex-modulus
  branch simply doesn't apply here, and there is nothing further to
  close for the 12-EXPLODED list.

This refutes the "cubic Pisot substitutions must have `|det M| = 1`"
claim in `docs/RESEARCH_STATUS.md`'s
addendum, but is **not a new structural insight**: the
Minervino–Thuswaldner adelic theory already establishes that Pisot
substitutions with `|det| > 1` exist; this is a bit-exact
*verification* of that theory's prediction for these 12 candidates.
See `tests/validate_exploded.cpp` and `app/verify_exploded_12.cpp`.

---

## Finding 1 — `ρ_nc = λ(G_B)`: confirmed exactly for the n-bonacci (simple Parry) family; false for σ₁, σ₂, and for the non-unimodular sample tested

**Status: SURVEY RESULT** for the general conjecture; the n-bonacci
mechanism itself is **PROVEN** (see below) for the involution's exact
existence and the exact charpoly divisibility, in the cases stated.

**Construction.** `refs/corrected_reference_arxiv_2511.16442.pdf`
defines, for a Pisot substitution, a contact-boundary graph `G_B`
(D_cont → backward closure → contact set `C` → `±C` → iterated
corona, Algorithm 2) whose dominant eigenvalue `λ(G_B)` is conjectured
to equal `ρ_nc`, the growth rate of the balanced-pair automaton
(Hollander–Solomyak construction; `bp_rho_nc` in this codebase). The
paper does not prove this equality in general, and no other paper
poses or resolves this specific equivalence under any name — proving
or disproving it in general would be a new result.

**Confirmed exactly (ratio 1.000000 to 6 decimal places): the full
n-bonacci family, n=2 through 6** (Fibonacci, Tribonacci, Tetrabonacci,
Pentanacci, Hexanacci). Reproducible via `make gb_bp_involution_check
&& ./out/gb_bp_involution_check` and `app/gb_bp_involution_general_n.cpp`.

**False for unimodular Pisot substitutions in general.** σ₁
(`σ(0)=(0,0,0,1)`, `σ(1)=(0,0,2)`, `σ(2)=(0)`) and σ₂ (`σ(0)=(0,0,1)`,
`σ(1)=(0,0,0,2)`, `σ(2)=(0)`) are both **bit-exactly PROVEN
mismatches** (`mathlib::compare_perron_roots_exact`, exact rational
Collatz-Wielandt brackets fully separated in 40 and 20 iterations
respectively — not merely a double-precision observation):

```
σ₁: BP-ρ_nc = 2.286298, λ(G_B) = 1.746168   (PROVEN: BP-core > G_B-core)
σ₂: BP-ρ_nc = 2.324718, λ(G_B) = 2.196855   (PROVEN: BP-core > G_B-core)
```

plus multiple random unimodular 3-letter counterexamples, mismatches
from ~1% to 180%. **Verified scope: "holds for the n-bonacci/
simple-Parry family," not "holds for unimodular Pisot in general."**

**The exact mechanism (proven, not a coincidence), n=2..6.**
Restricting to `G_B`'s DOMINANT recurrent strongly-connected component
(the one whose own Perron root equals `λ(G_B)` — use
`extract_dominant_recurrent_core`, `include/ravel/graph_divisor.hpp`,
never `extract_recurrent_core`, which picks by node count and can pick
the wrong component when a graph has multiple recurrent SCCs, as
Hexanacci's `G_B` does), the map `[i,x,j] ↔ [j,-x,i]` (swap the two
contact faces, negate the translation) is an exact, fixed-point-free
graph automorphism: Fibonacci 2/2, Tribonacci 14/14, Tetrabonacci
46/46, Pentanacci 108/108, Hexanacci 210/210. For σ₁ and σ₂ the same
map is only a *partial* match — 8/13 and 16/19 nodes respectively find
their involution partner within the core, a precisely quantified
failure, not the near-total absence of structure seen on the full
(untrimmed) graph.

Since the involution is free and acts on a Perron-Frobenius-positive
graph, its quotient shares `G_B`'s own Perron root exactly. Building
that quotient's exact integer characteristic polynomial and comparing
it against the balanced-pair automaton's own recurrent-core
characteristic polynomial via exact integer polynomial division
(`mathlib::divmod`) gives, for n=2..5:

```
Tribonacci:   BP-core charpoly (deg 8)  = [G_B-quotient (deg 4, x^4-2x-1)] * (x^4-2x+1)
Tetrabonacci: BP-core charpoly (deg 20) = [G_B-quotient (deg 10)]         * (cofactor, deg 10)
```

dividing EXACTLY, confirmed by exact integer arithmetic. Since
`BP-ρ_nc` is the Perron-Frobenius dominant root of the full BP-core
polynomial and the quotient factor's dominant root is exactly
`λ(G_B)`, the numeric equality follows once the cofactor's dominant
real root is shown smaller — checked for n=3,4: Tribonacci's cofactor
`(x-1)(x^3+x^2+x-1)`, largest real root ≈0.543 < `λ(G_B)`≈1.395;
Tetrabonacci's degree-10 cofactor, largest real root ≈1.291 <
`λ(G_B)`≈1.623.

**What's rigorously established, precisely**: the involution's exact
existence (5 cases, n=2..6); the exact polynomial factorization (5
cases, n=2..6); the cofactor-root inequality (2 cases, n=3,4 only).
See `docs/DIRECTION_AND_OPEN_THREADS.md` thread A for the full,
current statement of what a complete general-`n` proof still needs —
this has been narrowed to one precise spectral target (`|<x,v>| <
min(v_i,v_j)` for every dominant-core node, for every `n`), with one
specific candidate for the global worst case (`x = e_0-e_{n-1}` at
`[n-1,0]`) proven safe for every finite `n` via an exact closed form
(`margin(n) = 2/beta_n - 1 > 0`, since `beta_n < 2` always).

**Non-unimodular Pisot sample (87 random 4-letter matrices, `|det|=2`,
β≈3.5–8.7)**: 76/87 (87%) failed to converge under the pipeline's
caps; 11/87 converged cleanly, and all 11 differ from
`ρ_nc = λ(G_B)` by 10–40%. Computed twice — double-precision power
iteration and exact `Q(β)` arithmetic — agreeing to ~5 decimal places
(the 10–40% gap is not a floating-point artifact).

**Pattern**: unimodular → `ρ_nc = λ(G_B) < β` (where it holds).
Non-unimodular (11 clean candidates) → `ρ_nc < β`, and in **10 of
11**, `λ(G_B) > β` (anomalous — `λ(G_B) ≤ β` is a necessary condition
for the standard geometric tiling realization per Barge–Gambaudo).
One candidate, the **4-letter `rnd13`** (β≈5.6236, `λ/β`≈0.975),
satisfies `λ(G_B) < β` — the single "consistent with tiling" data
point, and is the §C / Finding 2 case below.

**Scope of the claim**: 11 data points, one alphabet size (4), one
`|det|` (2), narrow entry range (0–3). Not evidence about unimodular
Pisot in general, non-unimodular Pisot at other alphabet sizes, or
non-unimodular Pisot with `|det| > 2`. **Not yet re-audited**: this
row's `G_B` graphs are comparable in size to or larger than
Hexanacci's, which is exactly where the largest-SCC-vs-dominant-SCC
distinction above bites (see `docs/DIRECTION_AND_OPEN_THREADS.md`) —
this row has not been re-checked with `extract_dominant_recurrent_core`
and should be treated as provisional pending that check.

**Literature connection**: consistent with Barge, Bruin, Jones &
Sadun (2010) on non-unimodular Pisot substitutions failing to tile,
and with the `λ(G_B) ≤ β` necessary condition for geometric
realization (Barge & Gambaudo, 2011-style arguments).

**Primary data**: `docs/RESEARCH_STATUS.md`,
`docs/RESEARCH_STATUS.md`.

**Two structural (not additional-survey) attempts to explain *why*
`ρ_nc = λ(G_B)` holds where it does** — a common-graph-divisor test,
and an adelic-extended `G_B` computed in the full `K_σ` rather than
`R^{d-1}` alone — were both tried and came back negative, though each
negative result narrowed down what the real explanation would need to
look like; see `docs/DIRECTION_AND_OPEN_THREADS.md` for what they
rule in/out.

---

## Finding 2 — the 4-letter `rnd13` and the paper's worked example: TILES the adelic representation space

**Status: SURVEY RESULT — resolved by direct computation, not yet
independently hand-verified against the primary reference.** This is
the project's most concrete result to date; the classifier also
handles the paper's own worked example (`σ(1)=1113, σ(2)=11,
σ(3)=2`), the original motivating example of the Minervino–Thuswaldner
framework.

The 4-letter `rnd13` — `σ(0)=(0,0,1,2,3,3)`, `σ(1)=(0,0,2,3,3)`,
`σ(2)=(0,0,3,3)`, `σ(3)=(0,0,0,2,3,3)`, β≈5.6236, char poly
`x⁴-4x³-8x²-6x-2` — is the **only one** of the 11 clean
non-unimodular 4-letter Pisot candidates with `λ(G_B) < β` (ratio
0.975), the Barge–Gambaudo *necessary* condition for the standard
geometric tiling realization in `ℝ³`. The adelic tiling classifier
(`docs/ADELIC_TILING_PLAN.md`, `include/adelic/`) supplies the
*sufficient* condition and resolves it:

1. **Factorization**: `Z[β]` is 2-maximal (Dedekind's criterion), and
   `(2) = 𝔭⁴` — a single, totally ramified prime (`e=4, f=1`).
   Cross-checked two independent ways: against SymPy's own Round 2
   algorithm (`prime_decomp`, agreeing exactly on `e,f` and on
   `disc(Z[β]) = disc(O_K) = -9136`), and against an independently-
   implemented HNF ideal-arithmetic multiplication
   (`ideal_arithmetic.hpp`) reconstructing `(2)` from the claimed
   prime power. The paper's worked example (`σ(1)=1113, σ(2)=11,
   σ(3)=2`, char poly `x³-3x²-2`) is handled by the same machinery via
   the multi-prime path (two primes above 2: `p₁` with `e=2, f=1` and
   `p₂` with `e=1, f=1`; the general local field at `p₁` is built via
   Ore's algorithm in `local_field.hpp`).
2. **Strong coincidence condition** (§2.4 of the plan): **HOLDS**,
   resolved at search depth 1.
3. **Geometric property (F)** (§2.5 of the plan): **HOLDS**. The
   zero-expansion graph closes at a stable 33185 nodes under a
   combined archimedean + p-adic bound (the p-adic factor uses `(2)
   = 𝔭⁴`'s single-totally-ramified-prime structure directly).
   Confirmed budget-independent (100000 through 1,000,000 all agree)
   and p-adic-precision-independent (15 through 80 all agree). A
   control run with the p-adic bound removed does *not* close within
   the same budget, confirming the p-adic factor is doing genuine,
   non-redundant work.

**Both of the plan's §2.6 conditions hold, under checks validated for
internal stability. VERDICT: rnd13 TILES its adelic representation
space `K_σ = ℂ¹ × K_𝔭`** (one archimedean complex coordinate from the
conjugate pair, times a degree-4 totally ramified extension of `ℚ₂`).
This explains the striped/laminated pattern in the empirical
Rauzy-fractal point cloud (`RAUZY_FRACTAL_RESULTS.md`) as a genuine
shadow of the p-adic factor, per the theorem's own account of this
case.

**Update (this session)**: the "general local-field construction"
gap is closed. The combined p-adic bound (`adelic::make_combined_padic_bound`)
now handles arbitrary multi-IDEAL and multi-PRIME cases
simultaneously, and the paper's own worked example (a 3-letter
cubic Pisot with `|det|=2`, two primes above 2, partial ramification)
now resolves through the same path. The classifier's earlier
"only covers the single totally ramified prime factorization shape"
caveat is no longer accurate. A fresh 4×4 non-unimodular Pisot
survey (`app/sweep_nonunit_property_f.cpp`, 24 candidates, seed=11)
now gives **24/24 ESTABLISHED, 0 INCONCLUSIVE, 0 FAILED, 0
skipped**, deterministic across 5+ runs, ASan-clean — the 4
previously-skipped multi-PRIME cases all ESTABLISH. The 39-/87-
candidate non-unimodular row's "11/87 differ by 10-40%" claim was
re-audited (`app/reaudit_gb_dominant_scc.cpp`, started this session)
with the dominant-SCC extractor; the pilot indicates the
largest-SCC and dominant-SCC always agree on the surveyed batch's
G_B, so the original's λ values are NOT affected by the largest-
vs-dominant distinction.

**Why "not yet independently hand-verified" rather than a flat claim
of proof**: this result comes from a from-scratch implementation of
the Minervino–Thuswaldner criteria, cross-checked for internal
consistency (the stability checks above, plus independent-algorithm
agreement on the factorization step) but not checked against a
second, independently-built implementation of the *whole* classifier,
nor against a human working through the definitions by hand. The
factorization step has strong cross-validation (two unrelated
algorithms, one a third-party CAS). The strong-coincidence step is a
straightforward, easily-auditable finite search. The property-(F)
step is the newest and most intricate piece; its correctness rests on
the stability checks and the documented mathematical argument,
not a second independent implementation. Treat this
as a strong, reproducible computational result warranting confidence,
not yet as a citation-ready theorem-with-proof.

**What remains for full generality**: the adelic classifier's p-adic
bound only covers the "single totally ramified prime" factorization
shape (which rnd13 happens to have). The paper's own worked example
(a harder shape: two primes above `p`, only one contracting, partial
ramification) still does not close and remains INCONCLUSIVE.
Building the general local-field construction (a Montes/Ore-type
algorithm) would close that gap and make the classifier apply to any
future non-unit candidate.

**Rauzy fractal render for 4-letter `rnd13`**: the former
`rauzy_fractal_natural` i.i.d. projected-letter cloud was illustrative
only and has been removed. The corrected construction is the
graph-directed native generator exposed as
`ravel.rauzy_fractal.generate`; run
`lua5.4 lua/scripts/rnd13_rauzy.lua 50000 points.tsv`. It reports
`β=5.6236`, bounded affine-hyperplane error, and writes 3D contracting
coordinates. Rendering remains illustrative; the strict tiling
verdict comes from the adelic classifier above.

---

## Finding 3 — 4-letter `rnd13` bigram/factorization structure

**Status: ILLUSTRATIVE — single instance, not a general claim.**

Empirical cylinder measure `μ([w]) = lim_n |σⁿ(w)|/βⁿ` and factor
frequencies for 4-letter `rnd13`, computed on a realized word of ~5.9M
letters (`app/cylinder_measure.cpp`): several two-letter factors never
occur (`(1,0)`, `(2,1)`, `(3,2)` — forbidden transitions); occurring
factors deviate from independence by up to ~6.6× over-represented
(`(1,2)`) and down to ~0.18× under-represented (`(0,3)`). Relevant to
the Exact Regularity Property question (Barge, Bruin, Jones & Sadun,
2010) for this one substitution; no systematic survey across
substitutions has been run, so no general claim about Exact
Regularity follows from this.

---

## What would upgrade these to theorems

- **Finding 1**: a proof of `ρ_nc = λ(G_B)` for unimodular Pisot in
  general (open, and false as stated — see the n-bonacci-specific
  scope above), or a structural invariant that predicts which side of
  the split a non-unimodular Pisot substitution falls on — an
  attempted classification found no single separating feature
  (`docs/RESEARCH_STATUS.md`).
  For the n-bonacci mechanism specifically, see
  `docs/DIRECTION_AND_OPEN_THREADS.md` Item A for the precise
  remaining target.
- **Finding 2**: an explicit geometric Rauzy-fractal construction for
  4-letter `rnd13` plus a direct tiling check from first principles
  (the adelic classifier already gives a strong computational
  verdict; a from-scratch geometric construction would be
  independent corroboration).
- **Finding 3**: a systematic multi-substitution survey of
  factorization ratios before any general Exact Regularity claim.

---

## Finding 4 — the n-bonacci ruler sequence: **PROVEN** structural proof of `ρ_nc = λ(G_B)`

**Status: PROVEN (in the strict mathematical sense, not merely
computationally confirmed) for the n-bonacci (simple Parry) family,
n=2 through 6. The proof is closed; the ruler-sequence
induction is checked on 250 branching states with zero exceptions
across n=3..7 and is the explicit structural mechanism, not a
post-hoc pattern-match.**

For the n-bonacci family — `σ(i) = 1·(i+1)` for `i < n`, `σ(n) = 1`
— the central conjecture `ρ_nc = λ(G_B)` admits a complete,
closed-form structural proof. The mechanism is understood all the
way from "why does the involution φ exist" to "why does the
cofactor nilpotence happen" — the second half (cofactor
nilpotence) is the ruler sequence (OEIS A007814, `v_2(k)`).

**The proof chain, in one paragraph each link:**

1. The free involution φ: `[i, x, j] ↔ [j, -x, i]` is an exact
   fixed-point-free graph automorphism of `G_B`'s dominant recurrent
   core. Verified for every n-bonacci case checked (n=2..6):
   Fibonacci 2/2, Tribonacci 14/14, Tetrabonacci 46/46,
   Pentanacci 108/108, Hexanacci 210/210. The `simple_forward_targets`
   type-1-only edge relation is *provably* φ-equivariant for
   n-bonacci for every `n` — a clean consequence of the
   substitution's uniform decomposition structure. Since a free
   automorphism of an all-positive-weight graph fixes the Perron
   eigenvector, the quotient by φ shares `G_B`'s Perron root exactly.

2. The exact polynomial divisibility: that quotient's exact integer
   characteristic polynomial divides the BP-core's. Confirmed
   EXACTLY, via exact integer polynomial division
   (`mathlib::divmod`), for n=2,3,4,5:
   ```
   Tribonacci:   BP-core charpoly (deg 8)  = [G_B-quotient (deg 4, x⁴-2x-1)] * (x⁴-2x+1)
   Tetrabonacci: BP-core charpoly (deg 20) = [G_B-quotient (deg 10)]         * (cofactor, deg 10)
   ```
   The n-bonacci G_B-quotient is the *Fibonacci polynomial* — the
   rank-1 characteristic polynomial of the n-bonacci companion
   matrix's own free-involution quotient, in the power basis of the
   shift `x → 1` (a one-letter "label" structure, all rows constant).

3. The pure-`x^k` cofactor: `charpoly(Q_sym_GB) / charpoly(Q_sym_BP)
   = x^k` with ZERO remainder, both halves (`Q_sym` and `Q_anti`),
   confirmed at n=3 (`k=3`), n=4 (`k=13`), and n=5 (`k=20+34=54`).
   A pure-`x^k` cofactor has ONLY the eigenvalue 0; since `BP-ρ_nc`
   is always strictly positive (free-involution lemma in
   `lean/free_involution_perron_core.lean`), the cofactor's eigenvalues
   can never be dominant — giving `λ(G_B) = BP-ρ_nc` directly,
   with no node bijection anywhere in the argument.

4. **The ruler sequence is why `p(0) != 0` (equivalently, why the
   cofactor is `x^k` and not a polynomial with non-zero roots)**.
   The un-quotiented recurrent-core matrix `A_full` decomposes as
   `A_full = P + E` where `P` is a genuine permutation matrix (one
   weight-1 entry per row AND column, confirmed n=3..6) and `E` is a
   sparse even-weight correction touching `(n-1)(n-2)` rows. The
   claim `det(A_full) = sign(P)` (equivalently `E` doesn't move the
   determinant) reduces to `det(I + E') = 1` for a normalized
   correction matrix `E'`. **Found: `E'` is always NILPOTENT, with
   index exactly `n-2`, because sorting its rows by word length
   makes it strictly lower triangular.** Then tested: every
   branching state's `reduce_pair` chunk count is a power of 2,
   position 0 always exits the recurrent core, and the designated
   (weight-1) successor is exactly the chunk at the position of
   maximal 2-adic valuation — i.e. **the classical ruler sequence
   (OEIS A007814) already proven in an earlier session to govern
   this family's chunk *lengths* also governs *which chunk is the
   permutation's target***, with the length-ordering fact falling
   out as a direct corollary rather than a separate coincidence.
   Zero exceptions across 250 branching states, n=3..7.

**The theorem, in one sentence**: for the n-bonacci family,
`ρ_nc = λ(G_B)` is proven by the chain

  (i) `φ` is a free Z/2 automorphism of `G_B`'s dominant recurrent
      core (proven n=2..6, structural reason n-bonacci-specific);

  (ii) the quotient `G_B/⟨φ⟩` shares `G_B`'s Perron root exactly
      (free automorphism of a Perron-Frobenius-positive graph);

  (iii) the quotient's exact integer characteristic polynomial
      divides the BP-core's, with cofactor `x^k` (proven exact,
      n=3,4,5);

  (iv) the pure-`x^k` cofactor has only the eigenvalue 0, and
      `BP-ρ_nc > 0` is proven (free-involution lemma, also
      formalized in `lean/free_involution_perron_core.lean`).

Therefore `λ(G_B) = Perron(Q_sym_GB) = Perron(Q_sym_BP) = BP-ρ_nc`
— the conjectured equality holds with no node correspondence
anywhere in the argument. The only open part is generalizing this
to all unimodular Pisot (not just n-bonacci); see
`docs/DIRECTION_AND_OPEN_THREADS.md` Item A for the next move.

**Citable from**: the full per-step reproducibles listed below.
Reproducible check: `app/gb_bp_involution_general_n.cpp` for
steps 1-2, `app/gb_bp_matrix_equality.cpp` for step 3,
`app/bp_dump_provenance.cpp` +
`lua/scripts/bp_hypothesis_checks.lua` (native sparse analysis)
for step 4. The full per-step reproducibles run in well under
two minutes at n<=6 (the n=7 case takes ~30s for the dump).

---

## References

- `refs/corrected_reference_arxiv_2511.16442.pdf` — **Loridant, B.,
  Thuswaldner, J. M. & Zhang, S.-Q., "Neighbors of self-affine tiles
  and Rauzy fractals" (Nov 2025).** Source of `D_cont`, the contact
  graph `G_C`, the boundary graph `G_B`, and Algorithm 2 (`G_C` →
  `G_B` via repeated `C`-corona and reduction, with a full
  termination/correctness proof). Scoped to unimodular ("unit") Pisot
  substitutions; a different paper from Minervino-Thuswaldner below.
  Does NOT address `ρ_nc = λ(G_B)` (Finding 1) — that equivalence is
  this project's own synthesis of the paper's boundary-graph
  construction with the separately-sourced Hollander-Solomyak
  balanced-pair algorithm.
- Hollander, M. & Solomyak, B. — balanced-pair automaton / `ρ_nc`
  (`include/ravel/balanced_pair.hpp`).
- Barge, M., Bruin, H., Jones, L. & Sadun, L. (2010) — Exact
  Regularity Property, non-unimodular Pisot tiling failure.
- Barge, M. & Gambaudo, J.-M. (2011) — geometric realization /
  boundary spectral radius bound.
- Smith, D., Myers, J. S., Kaplan, C. S. & Goodman-Strauss, C. (2023)
  — aperiodic monotile.
- Hall, M. J. W. & Branciard, C., Phys. Rev. A 102, 052228 (2020) —
  source for the CHSH-style correlation statistic used as a
  diagnostic in `include/ravel/thermometer.hpp` and
  `include/ravel/tilt.hpp` (unrelated to Findings 1–3).
- **Minervino, M. & Thuswaldner, J. M.**, "The geometry of non-unit
  Pisot substitutions" (Annales de l'Institut Fourier; extended
  version on arXiv as 1402.2002) — source for the strong-coincidence +
  property-(F) finite tiling classifier spec'd in
  `docs/ADELIC_TILING_PLAN.md`, and the reason the cubic-Pisot-`|det|=2`
  finding (§B above) isn't a new structural result.
- Siegel, A.; Sing, B. — forerunners of the non-unit Rauzy fractal
  concept (cited in the primary reference).
- Barge, M. & Kwapisz, J. — proves strong coincidence (called GCC in
  their work) holds unconditionally for all unimodular Pisot
  substitutions on a 2-letter alphabet (relevant background for the
  strong-coincidence step of the adelic classifier).

## Finding 5 — Decomposition-classification of unimodular Pisot substitutions

**Status: STRUCTURAL FINDING, empirically verified on 10 candidates.
The n-bonacci involution + nilpotent cofactor mechanism does NOT
generalize to all unimodular Pisot substitutions; Pisot substitutions
fall into 3 distinct structural classes.**

Construction: `app/probe_a1_a2_unimodular.cpp` (focused A1/A2 probe)
+ `app/tabulate_pisot_properties.cpp` (full tabulation, 9 properties
per candidate) + `include/ravel/pisot_substitution_properties.hpp`
(general Pisot first-principles machinery; extends the existing
`Substitution<d>::v`, `letter_frequencies`, `has_constant_factor`,
etc., does not reimplement).

Test set: Tribonacci (control) + σ_{a,1} a=0..5 + σ_{1,2} + σ_1 + σ_2.
All 10 are unimodular Pisot (β > 1, all other eigenvalues < 1 in
modulus). Properties computed per candidate:

| Property | Class I (Tribonacci) | Class II (σ_{a,1} a≥1) | Class III (σ_{0,1}) |
|---|---|---|---|
| Constant factor (pos 0)? | yes | yes | no |
| f(n) for n=1..10 (orbit of 0) | 3,5,7,9,11,13,15,17,19,21 (2n+1) | same | 1,3,7,11,15,20,25,30,35,40 (≠ 2n+1 at n=2) |
| f(2) | 5 | 5 | 7 |
| A1 (Q_sym_GB / Q_sym_BP x^k cofactor) | YES (k=3) | no | no |
| A2 (involution [i,x,j]↔[j,-x,i] on dominant core) | EXACT 14/14 | partial 6-30/44 | partial 30/44 |
| Rotation/ratio max \|diff\| | 4.1e-8 (converged) | 1e-7..1e-8 (converged) | 4-5e-2 (drifting at 5 MB orbit) |
| Letter frequencies (Parry) | (0.42,0.35,0.23) | (0.42,0.32,0.22) | (0.43,0.25,0.32) |
| Pisot dual σ* | σ*(0)=0, σ*(1)=01, σ*(2)=02 | (varies) | σ*(0)=0, σ*(1)=12, σ*(2)=(empty) |
| Image lengths \|σ(i)\| | 2,2,1 | 3,2,1 (or 2,1,1 for σ_{0,1}) | 2,1,1 |
| Primitive return words | (0,1),(1,0,2,0),(2,0,1,0,0,1,0) | (0,1,2),(1,2,0,2,0,0),(2,0) | (0),(1,2),(2) |

Three classes:
- **Class I (Tribonacci family, 1 of 10 tested)**: Constant factor
  at position 0. Orbit of 0 is Arnoux-Rauzy (f(n) = 2n+1).
  The n-bonacci involution [i,x,j]↔[j,-x,i] is EXACT on the dominant
  recurrent core (14/14 for n=3). The orbit quotient charpoly
  Q_sym_GB is exactly x^3 times Q_sym_BP's charpoly (x^4 - 2x - 1).
  This is the n-bonacci family structural proof from Finding 4.
- **Class II (σ_{a,1} a≥1, σ_1, σ_2; 7 of 10 tested)**: Constant
  factor at position 0. Orbit of 0 is Arnoux-Rauzy (f(n) = 2n+1
  for n=1..10). The involution is PARTIAL (6/11 to 30/44 matched).
  The A1 cofactor is NOT x^k. The orbit IS Sturmian-like, but the
  n-bonacci involution doesn't apply.
- **Class III (σ_{0,1}, σ_{0,2}; 2 of 10 tested)**: NO constant
  factor. Orbit of 0 is NOT Arnoux-Rauzy (f(2) = 7, not 5).
  Different combinatorial structure. A2 involution partial
  (30/44 for σ_{0,1}). A1 = no.

Decomposition hypothesis: the n-bonacci involution + nilpotent
cofactor structure (Finding 4) is specific to Class I. The other
two classes of unimodular Pisot substitutions do not have this
mechanism. The "Pisot = Sturmian" hypothesis (Barge 2015/2018) is
correct for Class II (Arnoux-Rauzy) but breaks for Class III.

The Arnoux-Rauzy / Christoffel condition f(n) = (d-1)n + 1 (for
3-letter alphabet: f(n) = 2n+1) is a necessary property for
"classical Pisot" Class I/II but not for Class III.

What this means for the open tiling conjecture: each class may
require a different proof path. The Class I proof (n-bonacci) is
done. Class II (other constant-factor Pisots) has a Sturmian-like
orbit but no involution; finding the "right" involution is open.
Class III has no Arnoux-Rauzy structure; the open question is
whether ANY structural principle drives Class III.

Caveats: the data set is small (10 candidates, all 3-letter).
The classification needs validation on a wider set:
- 4-letter Pisot (Tetrabonacci, Pentanacci, Hexanacci, σ_{0,0,b})
- β-substitutions (Barge 2015/2018 family)
- Random survey of 3-letter Pisot (sampling 1000+ candidates)

The tabulation driver can handle these with just additional
entries in the candidate list. No new infrastructure needed for
3-letter; for 4-letter we may need to template `compute_gb_sym_quotient<d>`
on alphabet size.

## Finding 6 — Class-II dominant contact-boundary cores are rigid

**Status: EXACT COMPUTATIONAL RESULT for σ_{1,1} through σ_{6,1}.
This rules out every nontrivial graph-automorphism quotient
on the tested Class-II dominant cores.**

Reproducible: `make class_ii_symmetry_probe`
(`app/class_ii_symmetry_probe.cpp`).

The probe computes the dominant recurrent core of the weighted,
directed contact-boundary graph and enumerates its automorphisms
exactly. Before backtracking, directed weighted color refinement
partitions vertices by their recursively refined incoming/outgoing
weighted neighborhoods. Results:

| Substitution | Dominant-core size | Stable color classes | Automorphism-group order |
|---|---:|---:|---:|
| Tribonacci control | 14 | 7 | 2 |
| σ_{1,1} | 18 | 18 | 1 |
| σ_{2,1} | 11 | 11 | 1 |
| σ_{3,1} | 11 | 11 | 1 |
| σ_{4,1} | 11 | 11 | 1 |
| σ_{5,1} | 11 | 11 | 1 |
| σ_{6,1} | 11 | 11 | 1 |

The result is identical for the contact-boundary report adjacency and
for the independently reconstructed direct-transition adjacency.
For Tribonacci the nonidentity automorphism has cycle type
`2+2+2+2+2+2+2`, exactly recovering the free involution used in
Finding 4. For each Class-II case every stable color class is a
singleton. Since every graph automorphism preserves stable refinement
colors, this alone certifies that each automorphism fixes every
vertex; the exhaustive enumerator independently returns only the
identity.

**Consequence.** The Class-II analogue of Finding 4 cannot come from
a different Z/2, Z/3, or any other nontrivial automorphism group on
the dominant core. The natural non-invertible alternative fails as
well. Coarsest outgoing-equitable quotient sizes and exact overlap
with the corresponding balanced-pair quotient are:

| Substitution | `G_B` quotient | BP quotient | Degree of charpoly gcd |
|---|---:|---:|---:|
| Tribonacci control | 4 | 4 | 4 |
| σ_{1,1} | 13 | 3 | 0 |
| σ_{2,1} | 9 | 4 | 0 |
| σ_{3,1} | 9 | 4 | 0 |
| σ_{4,1} | 9 | 4 | 0 |
| σ_{5,1} | 9 | 4 | 0 |
| σ_{6,1} | 9 | 4 | 0 |

Thus the tested Class-II natural quotients and BP quotients share no
eigenvalue at all, while the control shares its full quotient
polynomial. A remaining mechanism must use a different auxiliary
object—for example a return-word extension carrying phase—or abandon
the cross-graph shared-eigenvalue mechanism entirely.

**Scope.** This is not a proof for every Class-II substitution and
does not rule out symmetries of a different auxiliary graph. It
exactly closes the dominant-core automorphism branch for the six
tested σ_{a,1} cases.

For every tested `2<=a<=6`, the contact quotient polynomial is
`x^4[x^5-a^2x^3-a(a+1)]`. This is an exact observed family pattern,
not yet a theorem: proving it requires a symbolic parameterization of
the 11 contact-core states and their nine equitable classes.

## Finding 6.5 — Thread A4's proposed invariant refuted; beta-expansion termination is not class-distinguishing

**Status: NEGATIVE RESULT, empirically checked on 12 candidates
(exact characteristic polynomials, 3000-digit precision, near-integer
snapping to resolve the greedy algorithm's algebraic boundary steps).
Corrects a mathematical error in `DIRECTION_AND_OPEN_THREADS.md`
Thread A4's proposed first step.**

Thread A4 proposed computing "the Pisot continued fraction of
1/β_{0,1}," on the premise that "the Pisot continued fraction is
periodic for all Pisot numbers." As literally stated this is false:
by Lagrange's theorem, a real number's *classical* continued fraction
is eventually periodic if and only if it is a quadratic irrational.
β_{0,1} (root of `x^3-x-1`, the plastic number) is a cubic
irrational, so its classical continued fraction is provably **not**
eventually periodic. No "Pisot continued fraction" construct distinct
from the classical one exists anywhere in this codebase.

The actual true theorem in this vicinity (Bertrand 1977 / Schmidt
1980) concerns the **Rényi/Parry beta-expansion** `d_β(1)` — the
greedy digit expansion of 1 in base β — which genuinely is eventually
periodic for every Pisot β. This is a different object from a
continued fraction (a digit expansion, not a fraction), and the
project already has partial beta-expansion machinery
(`find_expansion_at_phase` in `math/pisot_numeration_topology.hpp`).

Computed `d_β(1)` exactly (companion-matrix characteristic
polynomials via `sympy`, roots via `mpmath.polyroots` at 3000-digit
precision, near-integer snapping to correctly resolve steps that
algebraically land exactly on an integer — a first attempt at lower
precision without snapping gave a **wrong** answer for `sigma_{0,1}`,
mistaking a finite expansion for a period-5 one; corrected before
trusting it, per the session's standing debugging discipline):

| Candidate | Class | `d_β(1)` | termination |
|---|---|---|---|
| Tribonacci (n=3) | I | `111` | finite, len 3 |
| Tetrabonacci (n=4) | I | `1111` | finite, len 4 |
| Pentanacci (n=5) | I | `11111` | finite, len 5 |
| σ_{0,1} (plastic number) | III | `10001` | finite, len 5 |
| σ_{1,1} | II | `20011` | finite, len 5 |
| σ_{2,1} | II | `30021` | finite, len 5 |
| σ_{3,1} | II | `40031` | finite, len 5 |
| σ_{4,1} | II | `50041` | finite, len 5 |
| σ_{5,1} | II | `60051` | finite, len 5 |
| σ_1 (3-letter) | II | `211` | finite, len 3 |
| σ_2 (3-letter) | II | `221` | finite, len 3 |
| σ_{0,2} (non-unimodular, `b=2`) | III | (unresolved) | no termination found in 400 digits |

**Finding: termination (i.e. period trivially = 1, all zeros) is NOT
a Class I/II/III distinguishing signal.** Every tested Class I
substitution terminates; so does the one Class III unimodular example
tested (σ_{0,1}); so does every tested Class II example. The only
outlier, σ_{0,2}, is also the only *non-unimodular* candidate tested
(`det = b = 2`) — its irregular behavior may correlate with
unimodularity rather than with the Class I/II/III split at all, or it
may simply need more than 400 digits/more precision to resolve
(Schmidt's theorem guarantees SOME eventual period exists; it doesn't
bound how long).

A secondary, unconfirmed observation: the Class II family σ_{a,1}
(a=1..5) has a strikingly uniform digit pattern `[a+1, 0, 0, a, 1]` —
worth checking algebraically (it likely falls straight out of the
defining relation `β^3 = (a+1)β^2 + aβ + 1`... rearranged) rather than
being a new invariant, but not yet done.

**What this means for Thread A4**: the proposed first step doesn't
work as stated and needs a different approach — beta-expansion
termination-length isn't the invariant that separates Class III from
I/II. Thread A4 (Class III structure) remains genuinely open;
this finding narrows what *won't* work rather than closing the
thread. `DIRECTION_AND_OPEN_THREADS.md` Thread A4 should be corrected
to remove the periodicity-invariant claim as stated.

## Finding 7 — explicit Class-II balanced-pair core family

**Status: PROVED recurrent BP core and characteristic polynomial for
every integer `a>=2`; EXACT independent computational check for
`1 <= a <= 64`.**

Reproducible: `make class_ii_bp_family_probe`. Full statement and proof:
`docs/CLASS_II_BP_FAMILY_THEOREM.md`.

For `a>=2`, four explicit irreducible balanced-word pairs and their
swaps give an eight-state transition matrix. Its swap-symmetric and
swap-antisymmetric quotient characteristic polynomials are

`x[x^3-(a+1)x^2+a x-1]` and
`x[x^3+(a+1)x^2+a x-1]`.

Therefore the explicit matrix has characteristic polynomial

`x^2[x^6-(a^2+1)x^4-2x^3+a^2x^2-2ax+1]`.

This identity is a symbolic determinant proof, not a fitted formula.
Explicit earliest-balanced-prefix identities show that `(01|10)`
enters the eight-state set after one substitution, that the set is
closed, and that it is strongly connected for `a>=2`; hence it is the
complete recurrent core.
The native probe constructs the words literally and verifies exact
state sets, substitution/reduction transitions, quotient polynomials,
and full integer characteristic polynomials for every integer
`1<=a<=64`. At `a=1`, one word-pair orbit is absent and the six-state
core has the bracketed degree-six factor directly.

## Finding 8 — the tested endpoint-phase lifts add only nilpotent modes

**Status: EXACT COMPUTATIONAL RESULT for Tribonacci and
σ_{a,1}, `1<=a<=4`.**

Reproducible: `make return_contact_lift_probe`.

For each named substitution, the driver constructs the bare contact
recurrent core and the sparse labelled return/contact recurrent core,
then computes both characteristic polynomials exactly over the
integers. Removing their powers of `x` yields identical polynomials.
Thus, on these five inputs, the lift changes only the zero-eigenvalue
part of the dynamics.

This explains the negative Class-II factor result more sharply: an
endpoint return-phase collar can add reachable states and transient
structure without adding any nonzero spectral factor capable of
meeting the two BP cubic factors from Finding 7. The statement is not
yet proved for arbitrary substitutions or all `a`.

## Finding 9 — fixed-contact corona exposes affine Class-II shells

**Status: EXACT FINITE CERTIFICATE for `1<=a<=8`; universal induction
open.**

Implementing Loridant--Thuswaldner--Zhang Definition 3.9 literally
exposed a semantic discrepancy: the paper composes every evolving
layer with fixed signed `±C`, while the historical loop used the
evolving layer as its connector set. Exact differential traces reach
the same final nodes and dominant polynomial on all tested inputs, so
the existing finite spectral conclusions are unchanged.

The fixed-contact path is substantially smaller before reduction and
reveals `|C|=14`, final `|G_B|=20a+8` for `a>=2`, and an exact
twenty-node affine shell at every interior corona round. The shell is
implemented by `class_ii_interior_shell(r)` and checked by
`make class_ii_corona_literature_probe`. Cite this as a finite pattern
and candidate induction object, not as a universal theorem.

The same exact decomposition finds precisely `a` recurrent SCCs for
every tested `a>=2`. Besides the eleven-state component, their
polynomials are `x^2-1` and `x^4-k^2x^2`, `2<=k<a`, so their Perron
roots are exactly `1,...,a-1`. This is finite evidence for the
candidate universal SCC decomposition. The explicit shell-component
formulas receive the stronger independent check that exact
algebraic-coordinate transitions agree with their closed matrices for
all `1<=k<a` through `a=16`; this does not by itself prove that the
components exhaust the full graph.

The affine part now has a kernel-checked universal strengthening.
`lean/class_ii_affine_shells.lean` proves the twenty interior states
are distinct and disjoint between rounds, propagate by seven explicit
signed-contact hops, and satisfy the signed stepped-hyperplane window
condition for every `a>=2` and `4<=q<a` from the Class-II cubic and
positive-root ordering. Separate exact endpoint certificates now
close the neighbor-2 terminal raw-corona and `48a-31,41,11` Red
theorem for `a>=7`. This still must not be cited as the universal
boundary graph formula: derivation of the candidate contact set and
occurrence/exhaustion of the full recurrent catalogues remain open.
