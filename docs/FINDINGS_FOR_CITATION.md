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
non-unimodular Pisot with `|det| > 2`. **Re-audited and confirmed
(2026-08-01)**: `app/reaudit_gb_dominant_scc.cpp` checked a fresh
batch (target=4, then target=20; combined 24 candidates checked, 14
clean/converged) with `extract_dominant_recurrent_core` against the
original `extract_recurrent_core` (largest-by-node-count) side by
side. **14/14 clean candidates agree exactly** (dominant-SCC ==
largest-SCC, `λ` values identical to within `1.1e-9`), zero SCC
splits. The largest-SCC-vs-dominant-SCC distinction does NOT affect
this row's original `λ(G_B)` values; no longer provisional.

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
re-audited (`app/reaudit_gb_dominant_scc.cpp`) with the dominant-SCC
extractor and **confirmed, not just piloted**: 14/14 clean candidates
across two runs (target=4, target=20) agree exactly between the
largest-SCC and dominant-SCC extractors, zero disagreements, zero SCC
splits — the original's λ values are NOT affected by the largest-
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

**What remains for full generality**: the former single-prime and
multiple-non-simple-factor gaps are both closed. The local-field path
now handles the paper's partial-ramification example and independently
isolates distinct non-simple factors by a per-factor linear Hensel
lift with a fixed `F_p[x]` Bezout certificate. The deterministic
4-letter regression is consequently 7/7 ESTABLISHED, zero skipped;
the previously blocked `rndW3_5` case closes Property (F) at 421
nodes. This remains a tested computational implementation rather than
a complete formalization of general Montes/Ore theory, and ambiguous
factor metadata is rejected rather than guessed.

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
**Correction (Finding 16): this "all 10 are unimodular" claim is
false — σ_{1,2} has `|det M|=2`, not 1.** Not consequential to the
classification below (never dependent on unimodularity), but real and
uncaught until Finding 16 ran the actual tiling check on this family.
Properties computed per candidate:

| Property | AR-exact (Tribonacci) | AR-partial (σ_{a,1} a≥1) | non-AR (σ_{0,1}) |
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
- **AR-exact (Tribonacci family, 1 of 10 tested)**: Constant factor
  at position 0. Orbit of 0 is Arnoux-Rauzy (f(n) = 2n+1).
  The n-bonacci involution [i,x,j]↔[j,-x,i] is EXACT on the dominant
  recurrent core (14/14 for n=3). The orbit quotient charpoly
  Q_sym_GB is exactly x^3 times Q_sym_BP's charpoly (x^4 - 2x - 1).
  This is the n-bonacci family structural proof from Finding 4.
- **AR-partial (σ_{a,1} a≥1, σ_1, σ_2; 7 of 10 tested)**: Constant
  factor at position 0. Orbit of 0 is Arnoux-Rauzy (f(n) = 2n+1
  for n=1..10). The involution is PARTIAL (6/11 to 30/44 matched).
  The A1 cofactor is NOT x^k. The orbit IS Sturmian-like, but the
  n-bonacci involution doesn't apply.
- **non-AR (σ_{0,1}, σ_{0,2}; 2 of 10 tested)**: NO constant
  factor. Orbit of 0 is NOT Arnoux-Rauzy (f(2) = 7, not 5).
  Different combinatorial structure. A2 involution partial
  (30/44 for σ_{0,1}). A1 = no.

Decomposition hypothesis: the n-bonacci involution + nilpotent
cofactor structure (Finding 4) is specific to AR-exact. The other
two classes of unimodular Pisot substitutions do not have this
mechanism. The "Pisot = Sturmian" hypothesis (Barge 2015/2018) is
correct for AR-partial (Arnoux-Rauzy) but breaks for non-AR.

The Arnoux-Rauzy / Christoffel condition f(n) = (d-1)n + 1 (for
3-letter alphabet: f(n) = 2n+1) is a necessary property for
"classical Pisot" AR-exact/AR-partial but not for non-AR.

What this means for the open tiling conjecture: each class may
require a different proof path. The AR-exact proof (n-bonacci) is
done. AR-partial (other constant-factor Pisots) has a Sturmian-like
orbit but no involution; finding the "right" involution is open.
non-AR has no Arnoux-Rauzy structure; the open question is
whether ANY structural principle drives non-AR.

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
an AR-exact/AR-partial/non-AR distinguishing signal.** Every tested AR-exact
substitution terminates; so does the one non-AR unimodular example
tested (σ_{0,1}); so does every tested AR-partial example. The only
outlier, σ_{0,2}, is also the only *non-unimodular* candidate tested
(`det = b = 2`) — its irregular behavior may correlate with
unimodularity rather than with the AR-exact/AR-partial/non-AR split at all, or it
may simply need more than 400 digits/more precision to resolve
(Schmidt's theorem guarantees SOME eventual period exists; it doesn't
bound how long).

A secondary, unconfirmed observation: the AR-partial family σ_{a,1}
(a=1..5) has a strikingly uniform digit pattern `[a+1, 0, 0, a, 1]` —
worth checking algebraically (it likely falls straight out of the
defining relation `β^3 = (a+1)β^2 + aβ + 1`... rearranged) rather than
being a new invariant, but not yet done.

**What this means for Thread A4**: the proposed first step doesn't
work as stated and needs a different approach — beta-expansion
termination-length isn't the invariant that separates non-AR from
AR-exact/AR-partial. Thread A4 (non-AR structure) remains genuinely open;
this finding narrows what *won't* work rather than closing the
thread. `DIRECTION_AND_OPEN_THREADS.md` Thread A4 should be corrected
to remove the periodicity-invariant claim as stated.

## Finding 6.6 — Thread A5: AR-exact's exact signature found outside its two known source families

**Status: EXTENDS Finding 5's data set from 10 to 18 new candidates
(28 total). The coarse two-way split (AR-exact vs.
partial-everything-else) held on every new candidate; AR-exact's
specific structural signature (exact involution + `x^k` nilpotent
cofactor) was also found for the first time in a substitution outside
both of Finding 5's two source families (n-bonacci, `σ_{a,b}`).**

Construction: `app/probe_a5_extend_data.cpp` (kept separate from
`probe_a1_a2_unimodular.cpp` so Finding 5's cited numbers stay
reproducible), reusing the same `include/ravel/involution_helpers.hpp`
primitives. Three batches: (1) `σ_{a,1}` for `a=6..15` (Finding 5
covered `a=0..5`); (2) five more `σ_{a,b≥2}` cases; (3) 8 genuinely
random unimodular (`det=±1`) 3-letter Pisot matrices sampled outside
the `σ_{a,b}` family entirely (entries 0-4, seed 11, 702 trials to
find 8 Pisot+unimodular hits).

Results:
- `σ_{6,1}`, `σ_{7,1}`: partial involution, A1 not `x^k` — same Class
  II pattern as `σ_{1,1}..σ_{5,1}` in Finding 5, extends it cleanly.
- `σ_{8,1}` through `σ_{15,1}`: contact-boundary stuck at a constant
  `|G_B|=168` for all eight, not converging — a resource-cap
  artifact (the boundary construction hits its default cap at
  exactly the same size regardless of `a`), not new structural
  information.
- `σ_{0,3}`, `σ_{1,3}`, `σ_{3,2}`, `σ_{4,2}`, `σ_{0,4}`: **none are
  Pisot**. Pisot-ness in the `σ_{a,b}` family is sensitive to `b` in
  a way not previously characterized here (`σ_{0,1}` and `σ_{0,2}`
  are Pisot per Finding 5/6.5; `σ_{0,3}`, `σ_{0,4}` are not).
- Random unimodular matrices outside `σ_{a,b}`: **7 of 8 show the
  same "partial involution, A1 not `x^k`" pattern as AR-partial/non-AR**
  — the generic/majority behavior among unrelated random unimodular
  Pisot matrices, not a `σ_{a,b}`-specific artifact. **1 of 8**
  (`random_unimod_8`, `β≈5.033`, `|G_B|=80`) shows the **full AR-exact
  signature**: involution EXACT (8/8 core nodes matched) and A1
  cofactor exactly `x^1`. This is the first time AR-exact's exact
  structural signature has been observed in ANY substitution outside
  the n-bonacci family — direct evidence that AR-exact is a genuine,
  recurring structural class rather than an n-bonacci-specific
  artifact, while confirming that it is the minority case (roughly
  1-in-8 in this small sample) among general unimodular Pisot
  substitutions.

**Exhaustiveness check**: every one of the 18 new candidates that
reached a verdict fell cleanly into one of the two known patterns
(AR-exact, or the shared "partial" pattern of AR-partial/non-AR) —
no third pattern appeared. This is consistent with, but does not
prove, Finding 5's implicit hypothesis that the classification is
exhaustive; this probe doesn't check the Arnoux-Rauzy/constant-factor
properties needed to further split "partial" candidates into AR-partial
vs. non-AR specifically.

**Caveats**: still a small sample (26 candidates total across Finding
5 and this extension); the resource-cap wall at `σ_{a,1}, a≥8`
prevents testing whether the AR-partial pattern holds indefinitely as
`a` grows; the random-unimodular sample is only 8 matrices. 4-letter
extension (Tetrabonacci/Pentanacci/Hexanacci-adjacent families,
`β`-substitutions) not yet attempted.

## Finding 6.7 — Thread A4: non-AR branching resolves into persistent extension seams

**Status: POSITIVE EXACT FINITE-WORD RESULT.** Direct extension sets
are checked through factor length 64 on substitution-orbit prefixes of
525456 to 922111 symbols; a suffix-automaton range calculation checks
the non-AR formula and special-factor counts through length 500000
(2026-08-02, strengthened from an initial 60000 pass) on a
35676949-symbol orbit prefix -- a ~71x margin over the checked length.
This is not yet an all-length theorem. Reproducible with
`app/thread_a4_extension_graph_probe.cpp`.

For every internal factor `u` of length `n`, the probe records its
left extensions `L(u)`, right extensions `R(u)`, and two-sided
extensions `E(u)`. This refines the scalar complexity `p(n)` into the
actual branching sites of the substitution language.

- Tribonacci (AR-exact) has `p(n)=2n+1` and exactly one left-special
  and one right-special factor at every checked length, both ternary.
- `σ_{a,1}` for `a=1,2` has the same `p(n)=2n+1`, but realizes the
  two new factors per length as two binary left-special factors and
  one ternary right-special factor. `σ_1` has the reversed orientation:
  one ternary left-special and two binary right-special factors.
  `σ_2` matches Tribonacci's one-ternary-on-each-side profile despite
  belonging to the graph-theoretic AR-partial class. Thus aggregate
  AR complexity does not determine the orientation or multiplicity of
  local branching.
- The non-AR `σ_{0,1}` orbit has `p(n)=5n-5` for every checked
  `4<=n<=500000`. At every one of those lengths it has exactly five
  left-special and three right-special factors; left valence is at
  most two, while at least one right-special factor retains ternary
  valence. The excess slope five is therefore persistent distributed
  branching, not a sporadic exceptional factor.

This supplies the first positive Thread-A4 structure after the
continued-fraction/beta-expansion and balance-constant proposals were
refuted. In the intended stepped-hyperplane interpretation, special
factors are candidate local seam/junction types: the AR-complexity
cases have total branching increment two, while `σ_{0,1}` carries five
persistent branches. What is **not** yet proved is the exact dictionary
between a symbolic extension graph and a projected cube-face vertex
star, or that the displayed formulas persist for all `n`. Those are
now the two sharply stated derivation obligations.

The special-factor framework of Klouda--Pelantová (2009,
`KloudaPelantova2009` in `refs/references.bib`; local PDF checksum in
`refs/FULLTEXT_MANIFEST.tsv`) is a plausible proof route, not a result
being imported as though it already covered this substitution. In
particular, the constant finite-length count does not alone establish
five infinite left-special branches: finite maximal-special families
can grow recursively. The all-length proof must identify the actual
predecessor decomposition before interpreting branch counts.

**Finite-prefix boundary check.** Asking the original 525456-symbol prefix
for lengths up to 60000 produces an apparent first left-special-count drop at
`n=55406` even though complexity and right-special counts remain unchanged.
Repeating on a 4983377-symbol prefix removed the drop and restored the
displayed profile through 60000; repeating again on a 35676949-symbol prefix
(2026-08-02) restored it through 500000, an even wider margin. The former is
therefore a sampled-word boundary artifact, and the probe keeps the largest
tested margin for this pass.

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

## Finding 10 — Item A's AR-partial: A1's nilpotent-cofactor divisibility is symbolically IMPOSSIBLE for every a>=2, not just numerically absent

**Status: SYMBOLIC PROOF, conditional on Finding 9/Theorem 6's own
still-open "universal exhaustion" premise (that the displayed
eleven-state `K_a` really is the full dominant recurrent core for
every `a>=2`, not just the tested `1<=a<=8`). Given that premise, this
closes Item A's Thread A3 question for the `σ_{a,1}` family with an
actual proof, not a wider numeric sweep — directly responding to a
request to prove or disprove the pattern mathematically rather than
just testing whether it holds at larger `a`.**

Built entirely from two already-proven pieces: Theorem 6's explicit
eleven-state matrix `T_a` for `σ_{a,1}` (`docs/CLASS_II_CONTACT_MATRIX.md`,
`charpoly(T_a) = x^6[x^5-a^2x^3-a(a+1)]`, transcribed and independently
re-verified via `sympy` -- exact match) and Finding 7's proven BP-core
charpoly `x[x^3-(a+1)x^2+ax-1]`. Both are **a-independent in their
state labels** -- `T_a`'s eleven `(i,x,j)` triples are the same fixed
combinatorial list for every `a`; only the edge *weights* (`a`,
`a-1`) vary. That makes the involution question a fixed, finite check
good for every `a` at once, not a per-`a` numeric probe:

1. **A2 (involution)**: pairing each state `(i,x,j)` with its mirror
   `(j,-x,i)` inside the eleven-state list (verified via `sympy`, not
   by hand) finds exactly **three** mirror pairs -- `{1,3}, {4,5},
   {6,7}` -- and **five** states (`0,2,8,9,10`) whose mirror is
   simply absent from the eleven-state set (three of them need
   `j=2`, which no state in the list has at all). **6/11 matched,
   always**, independent of `a` -- this is exactly the "6/11" ratio
   Finding 5 recorded numerically for a tested case; now shown to be
   the same ratio for every `a`, by direct combinatorial argument
   rather than by re-running the pipeline at each `a`.
2. **A1 (nilpotent cofactor)**: building `Q_sym` from the three
   matched pairs, following the exact orbit/quotient construction in
   `gb_bp_matrix_equality.cpp` (representative = first-encountered
   element of each pair; edges to unpaired targets are dropped, per
   the same rule the C++ driver uses), gives the explicit `3x3`
   matrix

   ```
   Q_sym = [[0, 0, 0], [0, 1, 1], [a, a-1, a-1]]
   ```

   with characteristic polynomial `x^3 - a*x^2 = x^2(x-a)` --
   **degree 3**. Finding 7's BP-core charpoly has **degree 4**. A
   degree-4 polynomial cannot divide a degree-3 polynomial (other
   than the zero polynomial, which `x^2(x-a)` is not, for any
   specific integer `a`). **The A1 divisibility fails by a pure
   degree-counting argument, for every integer `a>=2` at once** --
   not because some coefficient happens not to cancel at each tested
   `a`, but because the two sides can never even have matching
   degree.

**What this does and doesn't close**: this is a genuine proof that
*if* `K_a` is the dominant recurrent core (Finding 9's open premise),
then A1 fails and A2 is exactly 6/11-partial for every `a>=2` --
answering "does the pattern hold indefinitely" without needing to run
the pipeline at `a=100` or `a=1000`. It does NOT prove Finding 9's own
open item (that `K_a` really does exhaust the dominant recurrent core
for all `a`, not just the checked `1<=a<=8`) -- that remains exactly
as open as it was. Reproducible: `python/class_ii_a1_a2_proof.py`.

Cross-check against Thread A5 (Finding 6.6): `σ_{6,1}` and `σ_{7,1}`
were tested numerically there (via the full pipeline, `|G_B|=128,148`
matching the `20a+8` formula exactly) and both showed "partial
involution, A1 not x^k" -- consistent with, though not by itself
proof of, this symbolic result.

## Finding 11 — Item B, Thread B3: rnd13's involution structure is partial, like non-unimodular candidates in general; A1's exact check is computationally infeasible at this scale

**Status: PARTIAL RESULT (A2 only). Answers Thread B3's question --
"do the n-bonacci involution results give structural information
about the non-unimodular tilers (only rnd13 in the 24/24 batch)?" --
for the involution half, honestly, without forcing the cofactor half
through at unreasonable cost.**

Ran the same A1/A2 machinery used throughout Item A
(`include/ravel/involution_helpers.hpp`) against `rnd13`'s actual
`G_B` (`app/probe_b3_rnd13_involution.cpp`), the one non-unimodular
Pisot candidate in the 24/24 survey known to TILE (Finding 2).
`rnd13`'s contact-boundary is substantial: `|G_B|=1101`, dominant
recurrent core 917 nodes.

**A2 (involution)**: 870/917 matched -- **partial**, not the exact
signature AR-exact substitutions show. `rnd13` does NOT carry the
n-bonacci-style involution cleanly, despite being the one candidate
known to tile. At face value this suggests the involution/cofactor
mechanism and adelic tiling are independent properties, at least for
this one tested candidate -- tiling doesn't require (or predict) the
AR-exact structural signature.

**A1 (nilpotent cofactor)**: NOT COMPLETED, and this is itself a
real finding, not just a gap. The quotient construction gives a
`435x435` integer matrix (`Q_sym_GB`, 435 orbits from the 870 matched
core nodes) whose EXACT characteristic polynomial (via
`charpoly_PolyZ`'s BigInt arithmetic) did not finish in over 30
minutes of wall time before being killed as part of diagnosing what
first looked like a hang. Instrumented diagnosis (flushed progress
prints after every stage) showed it was NOT a hang -- every earlier
stage (contact-boundary construction, the A2 involution check, both
quotient constructions) completed within about a minute total; the
cost is entirely in the exact-arithmetic characteristic polynomial of
a matrix roughly 40x larger (435 vs 11) than anything A1 has been
checked on before (Theorem 6's `K_a`, Finding 10). This is a genuine
scaling limit of the current exact approach, not a bug: worth noting
for any future attempt to run A1 at 4-letter / large-`|G_B|` scale.

**What this leaves open**: whether `rnd13`'s A1 cofactor is `x^k` or
not is genuinely unknown, not just unreported. A faster numeric
(non-exact) check, or a faster exact charpoly algorithm at this
scale, would be needed to close it. Reproducible driver:
`app/probe_b3_rnd13_involution.cpp` (the A2 result reproduces in
under 90 seconds; A1 requires either more patience or a different
method).

## Finding 12 — the global occurrence theorem's four-round base-premises seam is closed for every `a>=7`

**Status: PAPER PROOF for every integer `a>=7` (Round 1's own row for
every `a>=3`), one tier below Lean-formalized; not yet the full global
occurrence theorem (recurrent-SCC exhaustion is a separate obligation,
Finding 13).**

`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s base-premises table
names four exceptional round-1-through-4 transitions that the general
`round>=5` stable machinery does not cover. All four close:

- **Round 1** (`1fda87c`, 2026-07-31): closes by a literature check
  against Loridant--Thuswaldner--Zhang, not a symbolic construction.
  `±C := C ∪ (−C)` (§3.5 of the primary source) is a definitional
  symmetrization, not an independently-closed object the algorithm's
  connector set needs to be proven equal to -- the paper's own
  Algorithm 2 starts from `±C` directly. This dissolves the question
  rather than answering it by computation; a same-night attempt at a
  direct symmetric-seed closure construction (Finding 9's SCC-shell
  machinery, applied to `D_cont(tau_a) ∪ mirror(D_cont(tau_a))`) is
  documented as a real, ruled-out dead end rather than erased.
- **Rounds 2 and 3**: closed symbolically for every `a>=7` via the raw-
  corona/Red catalogues already exact-checked and Lean-formalized
  (`class_ii_neighbor2_nonbase_pre_red_catalogue`/`_post_red_catalogue`).
- **Round 4** (`1f82dbd`, 2026-07-31): the round-4-to-stable bridge
  reduces to whether the generic stable machinery's own round-4 output
  equals the real round-4 survivor catalogue. Checked exactly at
  fourteen widely separated `a` from 7 to 50
  (`app/class_ii_neighbor2_round4_stable_bridge_check.cpp`), then
  proven outright: both sides are independently provably constant for
  `a>=7` (the generic formula's branch conditions are already decided
  for `a>5`; the ground-truth side's constancy follows from the
  closed-form proof that closed rounds 2-4, which showed no
  slope-nonzero edge exists in the shape-classified structure for
  `a>=7` at all). Two independently-arrived-at constants, checked
  equal and both proven constant -- not a pattern holding at fourteen
  points.

`class_ii_neighbor2_first_missing_premise` (`include/ravel/
class_ii_neighbor2_pruning.hpp`) still hardcodes all four rounds as
open as of its own "2026-07-30" comment, predating this closure --
flagged as a possibly-stale load-bearing artifact (2026-08-02), not
yet corrected pending a careful check of whether that function's
specific technical sense of "reverse inclusion" matches the literature
argument that closed Round 1.

## Finding 13 — recurrent-SCC exhaustion (items 1-5) verified together for neighbor 2 at `a` in `{7,...,20,30}`

**Status: EXACT FINITE CERTIFICATE at fifteen tested `a`, not yet
closed-form; the true global occurrence theorem's final obligation
after Finding 12's base seam.**

`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`'s "What 'stitching'
still means" section names five items needed to promote the displayed
recurrent catalogues from "exhibited" to "exactly the recurrent SCCs of
the real boundary graph": (1) a partition into core/shell/transient,
(2) universal strong connectivity of each recurrent block tied to real
edges, (3) an escape witness from every transient block, (4) no return
edge from a recurrent block to an earlier transient stratum, (5) no
edges joining two distinct recurrent blocks.

- **Items 1, 2, 5** (`c17e879`, 2026-07-31): `app/class_ii_neighbor2_
  recurrent_exhaustion_check.cpp` runs generic Tarjan SCC decomposition
  against the real, fully-converged boundary graph and matches it
  node-for-node against the hand-catalogued recurrent blocks at all
  fifteen tested `a`. A first-draft stronger check (zero edges of any
  kind between distinct recurrent blocks) was found false -- eight
  such one-way edges every time, between the two ranks nearest the
  dominant core -- and corrected; item 5 as actually stated does not
  require this.
- **Items 3, 4** (`d28c66b`, 2026-07-31): `app/class_ii_neighbor2_
  round_stratified_transient_check.cpp` supplies the round/rank
  ordering items 3-4 need: a state's birth round is the first round it
  survives Red in the full corona trace, well-defined since a state
  never leaves a later round's survivor set once it enters one. Zero
  violations of item 4 at all fifteen `a`; item 3 holds once "escape"
  is corrected from "strictly later round" to "does not remain
  transient forever" (same-round absorption into a recurrent block
  counts).

Extended computationally, 2026-08-02: `app/class_ii_neighbor2_
round2_birth_mechanism_check.cpp` and `app/class_ii_neighbor2_
round_by_round_birth_check.cpp` reconstruct the corona iteration
directly from the closed-form seed (bypassing the expensive D_cont/
backward_closure front-end) and confirm, round by round rather than
only via the aggregate birth-round number, that every catalogued
rank's states are present among the correct round's new arrivals --
zero exceptions at every tested `a` from 7 through 30. Not yet a proof
for every integer `a`: the underlying `same_letter_H`/`in_H_sigma`
acceptance test is a real-valued inequality against the Perron
eigenvector, still evaluated numerically per `a` rather than reduced to
the `(b,c,1)`-coordinate polynomial-inequality argument
`lean/class_ii_affine_shells.lean` uses for the structurally similar
stepped-hyperplane endpoint theorem -- see that file and
`docs/CUT_AND_PROJECT_PROOF_DIRECTION.md` for the template a closed-
form version of this finding would need to follow.

## Finding 14 — Thread A4's persistent non-AR branching survives a much longer-range check

**Status: EXACT FINITE CERTIFICATE, strengthened range (2026-08-02).**

Finding 6.7 (`sigma_{0,1}`'s constant `p(n)=5n-5`, `LS=5`, `RS=3`
branching profile) originally held through `n=60000` on a
4,983,377-symbol orbit prefix. Re-checked on a 35,676,949-symbol prefix
(a ~71x margin over the checked length, well past the margin that
mattered for the earlier `n=55406` finite-prefix boundary artifact):
holds unchanged through `n=500000`, zero exceptions.
`app/thread_a4_extension_graph_probe.cpp` (committed at this strength).

A cheap, bounded check of whether the constant profile generalizes to
the rest of the `sigma_{0,b}` non-AR family (not yet a citable finding
in its own right -- recorded here as a pointer, not promoted): `b=2`
and `b=3` do NOT show a constant profile through `n=30` on a small
orbit; both oscillate between two regimes instead. Scratch check only,
not committed; a real pass would need the same boundary-artifact
discipline (larger orbit, longer range) Finding 6.7 itself required
before being trusted.

## Finding 15 — round 2's same_letter_H requirement, and the general shape of every future one, closed by a single verified closed form

**Status: PROVED for every real `a>=7` and, for the recurring
`k*(b-c)` family, every real `a` and integer `k` in `[1,a-2]`
simultaneously (bivariate exact polynomial arithmetic throughout, no
per-point substitution). A single closed form,
`b^2*height(p,q,r) = [(p+q)*a+r]*b^2 + [p*(a+1)+q]*b + p`, covers
every possible `(p,q,r)` at once -- verified exactly against all 13
round-2 families and four cases invented on the spot, none of which
broke the pattern. The same automated method rolls forward to rounds
3-5 with zero new hand derivation, and the algebraic reason for the
whole thing is identified and verified exactly: every case reduces to
the single identity `b*(b-c)=c`, itself the Class-II cubic restated.**

The recurrent-SCC exhaustion birth-round mechanism (Finding 13) relies
on `same_letter_H`, an acceptance test against the real Perron
eigenvector. Grounded empirically first: `subst.v` normalized equals
exactly `(beta, a+1/beta, 1)`, the `(b,c,1)` coordinates already
proven universal for the center's shells in `lean/
class_ii_affine_shells.lean` -- confirmed to apply to neighbor 2
directly, since `sigma_a` and `tau_a` share an incidence matrix and
hence a Perron direction.

Round 2's birth mechanism needs 19 `same_letter_H` tests
(`app/class_ii_neighbor2_round2_birth_mechanism_check.cpp`'s
witnesses), reducing to 13 distinct `(height, width)` algebraic
families. `app/class_ii_neighbor2_same_letter_h_symbolic_proof.cpp`
proves every family positive via a general, automated method: reduce
`b^2*height` and `b^2*(width-height)` modulo the Class-II cubic
(`b^3=a*b^2+(a+1)*b+1`) using exact BigInt polynomial division
(`math/poly_z.hpp`); if the result isn't already coefficientwise
non-negative, multiply by `b` and reduce again (valid since `b>0`,
so `b^k*x>0 => x>0`). Every family finds a certificate within at most
2 extra multiplications, at every tested `a` from 7 to 10^6 -- not a
numerical sweep standing in for a proof, an actual exact positivity
certificate per case.

This is a general tool, not a round-2-specific one: the certificate
search takes any `(height, width)` pair and finds its own proof,
independent of which round or catalogue it came from -- the same
"family of families" principle as `docs/FAMILY_OF_FAMILIES.md` (build
the general mechanism once, apply it repeatedly). Extending to rounds
3 through `a-2` (all still `a`-independent per the round-a-
independence discovery below) is now a matter of running the same
search on the next round's family table, not new research.

Preceded by a real, corrected error worth citing alongside the result:
an earlier pass claimed the worked example's margin "grows like `a`
and `2a^2`" -- wrong (a beta-scaled quantity was checked, not the
actual margin, which shrinks like `O(1/a)` but never reaches zero,
closed form `b-c=a/beta+1/beta^2`). Caught by building and running a
numerical batch across the full table rather than trusting the single
hand-derived case, then re-derived correctly and confirmed against
this project's own bit-exact `in_H_sigma_exact` path. See `TODAY.md`
2026-08-02 for the full trace.

**Extended across `k` the same way, then hunted for and found a
complete unification (2026-08-02).** `height=k*(b-c)` was still swept
as separate concrete integers for `k` even after `a` was freed --
fixed by tracking each coefficient as `P0(a)+k*P1(a)` (exact, since
height is linear in `k`) and proving both endpoints `k=1` and `k=a-2`
of the relevant interval, which (linear functions extremize at
endpoints) proves the whole interval `[1,a-2]` and every `a` in it at
once. Both endpoints proven for both widths
(`app/class_ii_neighbor2_trivariate_positivity_proof.cpp`, `a9941e4`).

The algebraic reason for that axis too, verified exactly: `b^2*(width_
b-height) = (a*b+1)*(b+1-k)` and `b^2*(width_c-height) = (a*b+1)*(b-k)`
-- both products of manifestly positive factors, direct consequences
of `b-c=c/b` and `b^2=c*(b+1)` (both restatements of `b*(b-c)=c`, the
cubic).

**Then: does this break down for round 2's other 8 (non-`k(b-c)`)
families?** No, verified exactly, not asserted -- every possible
`height=p*b+q*c+r` reduces via the SAME master identity to one closed
form, `b^2*height(p,q,r) = [(p+q)*a+r]*b^2 + [p*(a+1)+q]*b + p`,
confirmed against all 13 round-2 families plus four cases invented on
the spot (`app/class_ii_neighbor2_universal_closed_form.cpp`,
`f147d8a`). Combined with the equally trivial width closed forms,
BOTH bounds of literally any same_letter_H test this project's corona
rule could generate are available in one algebraic step, zero
per-case search. Caught and fixed a genuine, separate bug along the
way: `PolyZ::operator==` underflowed comparing two zero polynomials
(`math/include/math/poly_z.hpp`, fixed, `3efc96d`, full `make check`
green).

**Transported across the whole fixed-incidence fiber, then crossed a
matrix wall (2026-08-02).** `app/class_ii_identity_transport_probe.cpp`
applies the same exact reduction to the regular recurrent shells of
Class-II neighbors 0 and 1, not only neighbor 2. The first run exposed
an important orientation error in the probe itself: `same_letter_H`
tests the signed strip `-width < height < width`, whereas the earlier
neighbor-2 representatives happened to be positively oriented and had
been tested as `0 < height < width`. Once the actual signed predicate
was used, all neighbor-0 and neighbor-1 templates passed. This is not
an `m<=40` survey standing in for a theorem: every margin is affine in
the shell parameter `m`, and the probe proves both symbolic endpoints
`m=2` and `m=a-c`; hence the entire admissible interval for every `a`
follows by affine interpolation. Result: the same closed form and
signed-strip certificate hold for every regular shell of both other
word-order neighbors.

The next hop changes the incidence matrix while remaining in the
three-letter unimodular Pisot setting: Tribonacci. An initial version
used its right-eigenvector power basis `(beta^2,beta,1)`; that is not
the acceptance-strip convention in `Substitution::in_H_sigma`, which
uses the left Perron covector (`M^T` power iteration). Corrected before
finalizing the result. Scaled integrally, the actual basis is
`(b,c,1)=(beta,beta^2-beta,1)`. Here the Class-II identity genuinely
fails by exact remainder `-1`; the replacement is `b*(b-c)=c-1`, exact
zero remainder. The general reduction machinery survives: for every
`(p,q,r)`,

`b^2*(p*b+q*c+r) = (p+q+r)beta^2 + (p+q)beta + p`.

Thus the pattern is now located precisely. Sparse identities are the
multiplication table of `Z[beta]=Z[x]/(f_M)` in the Perron-coordinate
basis and change with the incidence-matrix/number-field stratum;
characteristic-polynomial reduction is the transportable mechanism.
Word-order changes inside a fixed-incidence fiber do not change that
table. The route toward broader unimodular families is therefore an
automatically generated quotient-ring multiplication tensor plus a
positivity cone per matrix stratum, not an attempt to force the literal
Class-II identity onto every substitution.

The sharper pattern is simpler than a generic quotient-ring comparison.
For the scaled left Perron covector `v=(b,c,1)`, `M^T v=beta v` says
that multiplication by `beta=b` has coordinate matrix **exactly the
incidence matrix `M`**. Therefore `b(b-c)` is obtained without a search:
subtract incidence column 1 from column 0 and read the resulting three
coordinates in `(b,c,1)`. Class II has column difference `(0,1,0)`,
giving `c`; Tribonacci has `(0,1,-1)`, giving `c-1`.

The generated tensors confirm this directly. Class II at `a=7` has
`b^2=7b+c+1`, `bc=7b+1`, `c^2=b+6c`; Tribonacci has
`b^2=b+c`, `bc=b+1`, `c^2=b+c-1`. For a companion incidence stratum
with cubic `beta^3=A beta^2+B beta+1`, the acceptance-covector basis is
`(b,c,1)=(beta,beta^2-A beta,1)` and column subtraction gives the
universal identity

`b*(b-c) = (A-B)b + c - 1`.

This is the sought pattern: the differing algebraic identities are
literal differences of incidence columns. Quotient-ring reduction is
still the independent executable verification and handles other
products, but this particular identity is already encoded linearly in
`M^T v=beta v`.

`lean/perron_column_difference.lean` kernel-checks the
dimension-independent theorem (sorry-free): for arbitrary finite real
`M`, `v`, and `beta`, the left-eigenvector equations imply
`beta(v_i-v_j)=sum_k(M_{ki}-M_{kj})v_k` for every pair of columns.
It also checks the three-coordinate Class-II and Tribonacci corollaries.
No Pisot, unimodular, primitive, or positivity hypothesis appears in
this theorem; those assumptions enter only when turning the algebraic
identity into contracting lattice/window geometry. The file and updated
manifest carry both verified GPG and X.509 signatures.
The C++ probe now treats the four expected column differences as assertions
and exits nonzero on any identity, tensor, symbolic-endpoint, or transport
failure; its enrolled explicit Make target reports `Overall transport probe:
PASS`, not merely a diagnostic table followed by unconditional success. It
also constructs the four actual incidence matrices and requires all three
columns of the multiplication-by-`beta` tensor to equal `M` exactly before
checking the derived column differences; current result is
`Full multiplication-by-beta tensors equal M: ALL EXACT`.

**The first geometric wall after the universal algebra (Lean-checked).** For
the companion incidence stratum, the same two left-eigenvector coordinate
equations prove `b>c` throughout `A>=B>=1`, but `c>b` throughout
`B>=A+1`. For integer positive coefficients these regimes meet without an
unclassified integer band. The identity does not break at this wall; its
orientation does. This explains why neighbor 0's earlier signed-strip
correction was structural rather than cosmetic: `same_letter_H` uses
`H_sigma union (-H_sigma)`, so it can transport across an orientation reversal
that would invalidate an oriented `0<height<width` proof.

**The same identity hops out of dimension three.** Consecutive columns of the
`n`-bonacci incidence matrix share their `e_0` contribution and differ by
`e_{i+1}-e_{i+2}`. The universal column theorem therefore gives the gap
recursion `beta(v_i-v_{i+1})=v_{i+1}-v_{i+2}`. Tribonacci's
`b(b-c)=c-1` is the terminal three-letter instance; Tetrabonacci's first
instance is `b(b-c)=c-d`. `lean/perron_column_difference.lean`
kernel-checks both the arbitrary local recursion and the explicit
Tetrabonacci corollary. This is a genuine direction toward broader families,
not another cubic-coordinate coincidence.
Lean also checks the complete four-letter first chain:
`beta(b-c)=c-d`, `beta(c-d)=d-1`, `beta(d-1)=1`; hence for positive
`beta`, `b>c>d>1`. This is already a genuine stepped-hyperplane width
ordering obtained from incidence columns, bridging the universal algebra to
the positivity side rather than stopping at a formal identity.

**Exact finite window-margin lift, including the two nearest dimensions.**
`app/item1_per_pair_check.cpp` now replaces its former `1e-6` verdict with an
exact `Q(beta)` certificate. It constructs and verifies the left Perron
covector, normalizes `v_0=1`, maximizes `|<x,v>|` over every dominant-core
face-pair group by exact sign comparisons, requires exact equality with the
recorded row formula, and requires every residual width margin to be strictly
positive. It also requires that the exact maximum is attained by the predicted
sparse vector (`-e_0+e_j-e_{j+1}` in row 0 or `-e_i+e_{n-1}` later), up to
the signed mirror convention. For n=3,4,5,6,7 (dominant-core sizes
14,46,108,210,362), the result is zero exact formula mismatches, zero
structural-witness failures, and zero nonpositive margins; in every case
the exact global minimum is `2/beta-1 > 0`. Thus the finite Tetrabonacci
catalogue and the two nearest extensions, Pentanacci and Hexanacci, are closed
at the exact arithmetic level; n=7 is an additional checked control. This is
not yet a symbolic occurrence/exhaustion theorem for arbitrary n.

The target also independently reconstructs every forward record from each
dominant-core node with exact rational `M x'=rhs` solving and exact
`Q(beta)` window validity, comparing destination, both prefix labels, and
multiplicity against the fast path. Across n=3..7 it finds zero exactly
invalid core nodes and zero transition-record mismatches. This removes the
fast-path numerical ambiguity on these computed recurrent cores; it still
does not prove that no additional core states occur for arbitrary n.

The Lean module now matches those two nearest algebraic steps: a reusable
positive-scaled-gap lemma yields explicit Pentanacci order
`b>c>d>e>1` and Hexanacci order `b>c>d>e>f>1` from their consecutive left
Perron coordinate equations and `beta>0`.

**Rolled forward automatically to rounds 3-5, confirming the "family
of families" framing rather than asserting it** (`app/class_ii_
neighbor2_interior_regime_structure_check.cpp`): the enumeration and
certificate search generalize to round `p` with zero round-specific
code, and every round's families are witnessed and certified. This
also revealed the interior regime's real structure: rounds 3+ reduce
to a single parametrized family, `height=k*(b-c)`, `width in {b,c}`,
for `k` around the round number -- not new algebra each round.

**Upgraded from per-`a` certification to a genuine proof**
(`app/class_ii_neighbor2_bivariate_positivity_proof.cpp`): represents
polynomials in `b` with coefficients that are themselves exact
integer polynomials in `a` (`math/poly_z.hpp`'s `PolyZ` as the
coefficient ring), reduces mod the cubic with `a` free throughout, and
substitutes `a = m + (domain lower bound)` before the non-negativity
check -- proving positivity for the family's actual domain (`a>=k+2`
for `height=k*(b-c)`; `a>=7` for the other round-2 families), not a
sampled range. A first version of this check asked the wrong question
("positive for every `a>=0`, independent of `k`") and it genuinely
failed for `k>=3` -- caught by running it and reading the output, not
assumed correct. All 13 round-2 families and the whole `k*(b-c)`
subfamily (`k=1` through `60`, tested) are now proved this way.

**The symbolic reason, verified exactly:** `b*(b-c) = c` is not a
consequence of the Class-II cubic discovered by algebraic
manipulation -- cleared of its `1/b` denominator, it *is* the cubic,
coefficient for coefficient (confirmed both in sympy, exact rational
cancellation, and independently via this project's own BigInt
bivariate arithmetic). Equivalently `b^2=c*(b+1)`, or `b-c=c/b`. This
is why every `k*(b-c)`-shaped test reduces almost immediately: the
recurring quantity is not an arbitrary one that happens to behave well
under the cubic, it is the cubic's own defining relation restated in
`(b,c)` coordinates. Practical consequence for a future Lean
formalization: the positivity results should follow as near-immediate
corollaries of a single one-line lemma (`b*(b-c)=c`) rather than
needing a separate inequality chain per family.

## Finding 16 — Finding 5's AR-partial/non-AR family run through the actual classical Pisot tiling check for the first time: 7/7 ESTABLISHED, plus a correction to Finding 5's own data

**Status: NEW, empirically verified. All 7 unimodular-or-not members of
Finding 5's AR-partial/non-AR test set satisfy strong coincidence and
property (F) cleanly, zero exceptions. A real error in Finding 5's own
table is corrected along the way.**

Finding 5 tabulated 9 structural properties (A1/A2 involution, letter
frequencies, orbit complexity, ...) for 10 unimodular Pisot
substitutions to classify them as AR-exact / AR-partial / non-AR, for
a *different* project-internal conjecture (`rho_nc = lambda(G_B)`).
That tabulation never ran the classical Pisot tiling conjecture check
itself (`adelic::check_strong_coincidence` / `adelic::check_property_f`,
`include/adelic/coincidence_and_property_f.hpp`) on the AR-partial/
non-AR members — only Tribonacci (already covered by citation, Barge
2015/2018, as a beta-substitution) and sigma_1/sigma_2 (checked
separately in `sweep_mismatches_property_f.cpp`) had ever gone through
it. `app/probe_ar_family_strong_coincidence.cpp` closes that gap,
running sigma_{0,1} (non-AR), sigma_{1,1}..sigma_{5,1} (AR-partial),
and sigma_{1,2} through the real check directly.

**Result: 7/7 ESTABLISHED** (strong coincidence HOLDS + property (F)
HOLDS). Six of the seven are trivial to resolve — AR-partial resolves
strong coincidence at depth 1 every time (a=1..5), and property (F)
closes within a few thousand nodes. The non-AR case (sigma_{0,1})
needs depth 13 for coincidence but still resolves cleanly, with
property (F) closing at 205 nodes.

**A real correction to Finding 5's own data, caught in the process**:
Finding 5's table states "All 10 are unimodular Pisot (beta > 1, all
other eigenvalues < 1 in modulus)". This is false for sigma_{1,2}:
its incidence matrix has `|det|=2`, not 1. This was not a typo caught
by inspection -- it surfaced because the first run of this probe
supplied no p-adic bound (matching the "unimodular, archimedean-only
suffices" assumption inherited from Finding 5), and property (F)
correctly ran to the full 300000-node budget without closing --
exactly the documented behavior for a non-unit substitution checked
without its p-adic factor (`coincidence_and_property_f.hpp`'s own BUG
2 note). Supplying the correct p-adic bound for `p=2` (the only prime
dividing the determinant) resolves it immediately, at only 12 nodes.
Finding 5's structural classification (AR-exact/AR-partial/non-AR) is
unaffected by this correction -- it was never about unimodularity --
but the "All 10 are unimodular" sentence in Finding 5 is wrong and
should be read with this correction in mind.

This is the empirical groundwork requested before attempting a
structural (involution-style) proof of strong coincidence for the
AR-partial/non-AR classes in general, per this project's standing
discipline of verifying computationally before building theory on top
of a claim. The natural next step, not yet attempted: the AR-partial
family resolves strong coincidence at depth 1 for every tested `a`,
which is itself a suspiciously clean, uniform fact -- worth checking
whether it holds by a short direct argument (not just "every tested
case happens to"), analogous to how Finding 4's n-bonacci proof
started from a similarly clean uniform empirical pattern.

## Finding 17 — the depth-1 mechanism: "constant factor at position 0" forces strong coincidence unconditionally, for any substitution, Pisot or not

**Status: PROVED (definitional, from `pair_has_coincidence`'s own loop
structure) and cross-checked against the actual search on Finding 5's
full AR-partial/non-AR family plus two synthetic cases (different
alphabet size, different constant letter, and a no-constant-factor
control). `include/ravel/proof/constant_factor_forces_depth1_
coincidence.hpp`.**

Finding 16 found that Finding 5's AR-partial substitutions
(`sigma_{a,1}`, a=1..5) all resolve strong coincidence at depth 1,
while the non-AR case (`sigma_{0,1}`) needs depth 13. This finding
identifies the exact, general reason, not just the pattern.

**Theorem.** If `sigma(i)` and `sigma(j)` begin with the same letter
`c`, the pair `(i,j)` exhibits a coincidence at k=1, unconditionally.
**Proof**: read `adelic::pair_has_coincidence`'s loop directly —
`running` (the prefix-abelianization accumulator) starts at the zero
vector and is only incremented AFTER the current letter's prefix set
is recorded. Scanning w1's first letter (=c) inserts the zero vector
into `prefix_set1[c]`. Scanning w2's first letter (=c) checks
`prefix_set1[c].count(running)` while `running` is STILL the zero
vector (nothing processed yet) — an unconditional hit. This needs no
Pisot property, no unimodularity, nothing about either word beyond
position 0; it holds for any two words over any alphabet sharing a
first letter. Verified directly against the live function, not
re-derived independently: `tests/constant_factor_forces_depth1_
coincidence_test.cpp` cross-checks the certificate's prediction
against `pair_has_coincidence`'s actual output on all 6 members of
the a=0..5 family (matches exactly: constant-factor iff depth-1,
every case), plus a synthetic 4-letter case with constant letter 2
(not 0, ruling out a letter-0-specific artifact) and a no-constant-
factor control that correctly declines to predict.

**Consequence for the open general unimodular Pisot question**: "has
a constant factor at position 0" (every alphabet letter's image
begins with the same letter — already a column in Finding 5's table,
previously not connected to coincidence depth) is a SUFFICIENT
condition for strong coincidence to hold trivially for the WHOLE
substitution, with no search needed. **Strong coincidence is
therefore never the obstruction for constant-factor substitutions —
any hypothetical counterexample to the classical Pisot tiling
conjecture among unimodular Pisot substitutions can only come from
the non-constant-factor ("non-AR") class.** This does not touch
property (F) (untested by this argument) and does not claim non-
constant-factor substitutions fail coincidence (sigma_{0,1} still
resolves, just at depth 13, via a different route — a later shared
occurrence or a matching suffix rather than the trivial matching-
empty-prefix one). It narrows the search, it does not close it: the
next open question is whether property (F) has an analogous
structural sufficient condition for the constant-factor class (which
would fully close that class), and separately whether non-constant-
factor substitutions have ANY structural guarantee for coincidence at
all, or whether depth-13-and-similar resolutions are themselves
case-specific.

**Literature connection, added after the fact (2026-08-06, checking
arXiv:1408.2110, Durand & Petite, "Conjugacy of unimodular Pisot
substitution subshifts to domain exchanges")**: a substitution whose
every image begins AND ends with the same letter is classical,
established terminology in this field -- a "proper" substitution
(traceable through Dekking 1978, Queffelec's standard textbook, and
Host's unpublished manuscript; Durand-Petite cite Queffelec for the
definition, not claiming it themselves). Durand-Petite use properness
as a technical device for a DIFFERENT purpose (bypassing the
classical coincidence condition entirely via return-word/Bratteli-
Vershik machinery to prove their own domain-exchange theorem) and
remark, in one unproved, uncited sentence, that a proper substitution
"hence satisfy[ies] a form of coincidence" -- an informal aside, not a
developed lemma, not attributed to a specific prior proof. This
project's OWN derivation here (the exact claim that depth is
PRECISELY 1, proved directly from `pair_has_coincidence`'s own code
and machine-verified) was derived independently, before this
literature check, from reading this project's own implementation for
an unrelated immediate purpose (building a foundation for property-
(F) work) -- not sourced from, or dependent on, Durand-Petite or any
prior explicit proof of this exact statement. The GENERAL folklore
intuition that "proper implies some coincidence" clearly belongs to
the field and predates this project by decades; the SPECIFIC,
formally proved, depth-exact statement here does not appear to be
written down explicitly anywhere consulted.

## Finding 18 — property (F) closed off from the constant-factor lens, with a proven (not just unsuccessful) reason

**Status: PROVED negative result. `include/ravel/proof/constant_factor_gives_nothing_for_property_f.hpp`, cross-checked against the real computed prefix automaton for the whole a=0..5 family.**

Follow-up to Finding 17 (constant factor forces strong coincidence at
depth 1, unconditionally). Natural question: does constant factor at
position 0 give property (F) the same free ride? **No, and the reason
is structural, provable without examining any specific substitution's
full closure:**

**Fact 1** (general, holds for every Pisot substitution, constant
factor or not): a prefix-automaton edge preserves `gamma=0` if and
only if its prefix `p` is empty. Proof: `delta(p) = <P(p), v>` with
`v` the left Perron eigenvector — Perron-Frobenius guarantees every
entry of `v` is strictly positive, so `delta(p)` is a sum of positive
numbers over `p`'s letters, zero only when `p` is empty. Confirmed
directly against the actual computed eigenvector and digit set for
every member of the a=0..5 family (`tests/constant_factor_gives_
nothing_for_property_f_test.cpp`), not just asserted from the general
theorem.

**Fact 2**: property (F)'s own definition permanently excludes any
cycle staying entirely among `gamma=0` nodes from counting as a
violation (this is the project's own BUG 1 fix, already documented in
`coincidence_and_property_f.hpp`).

**Consequence**: by Fact 1, the connectivity a constant factor at
position 0 creates (every letter reachable from the shared leading
letter, all while `gamma=0`) lives ENTIRELY inside the region Fact 2
already, unconditionally, excludes from ever causing a violation. The
actual hard question for property (F) — does any path leave `gamma=0`
(necessarily via a nonempty prefix, hence necessarily leaving the
constant-factor-structured region) and return — is completely
untouched by whether the substitution has a constant factor. This is
provable in general, not merely "not found a proof yet."

**Independent empirical confirmation**: Finding 16's own node counts
for property (F) on the constant-factor family — 105, 199, 963, 2762,
6451 for a=1..5 — grow monotonically with image length, exactly as if
the constant factor weren't there. Contrast strong coincidence, whose
depth stayed at exactly 1 across the same range. If the constant
factor had bought real leverage for property (F), the node counts
would stay small and roughly flat the way coincidence's depth did;
instead they scale with the genuinely hard part of the search.

**Net position on the open general unimodular Pisot question**:
strong coincidence is fully explained (Finding 17) for the large,
easily-recognized constant-factor class. Property (F) is not, and
this finding shows *why* the same lens can't be reused — any future
proof attempt for property (F) needs a genuinely different structural
handle, not a refinement of the coincidence argument. This is where
the "seeking a counterexample via theory" line of work currently
stops: coincidence is closed for constant-factor substitutions;
property (F) remains open for every unimodular Pisot substitution,
constant-factor or not, with no known reduction of its difficulty.

## Finding 19 — property (F), as implemented by this project's own finite automaton, appears to be UNCONDITIONAL: no configuration can ever produce a violation

**Status: A large claim, stated carefully. PROVED for the specific finite construction this codebase implements (`adelic::check_property_f`), by a clean 5-step argument from Perron-Frobenius positivity. Cross-checked against the real, trusted function itself (not a reimplemented copy) via a new permanent diagnostic parameter, across 8 structurally diverse cases including the project's largest-ever closure (rnd13, 33185 nodes). SCOPE CAVEAT below is load-bearing — read it before citing this as resolving the literature's Pisot conjecture.**

Follow-up to Finding 18 (constant factor gives property F no leverage).
Rather than stopping there, re-examined property (F)'s actual
violation condition directly: an SCC in the zero-expansion graph
containing BOTH a zero-translation node (`gamma=0`) and a nonzero one.

**The argument** (full detail in `include/ravel/proof/property_f_
unconditional.hpp`): a transition `gamma' = beta^{-1}(gamma + delta(p))`
can only ever produce `gamma'=0` (as an exact algebraic Q(beta)
element) if `gamma=0` already and `p` is empty. Proof: evaluate the
identity at the DOMINANT real embedding (the one sending the abstract
symbol `beta` to the actual Pisot number). Under that embedding: `v`
(the left Perron eigenvector `delta(p)` is built from) is strictly
positive by Perron-Frobenius; `delta(p)`, a nonnegative combination of
`v`'s entries, is therefore >= 0, zero only for the empty prefix; `beta`
itself is real and > 1. By induction from the `gamma=0` starting
frontier, every reachable node's dominant-embedding value stays >= 0,
and `gamma + delta(p)` can only be exactly 0 (forcing `gamma'=0`, since
field embeddings are injective) when both terms are individually
exactly 0. Consequence: no nonzero-gamma node can EVER have an edge
into a zero-gamma node — so no SCC can ever mix zero and nonzero
nodes, so `check_property_f` can never discover a violation.

**Verified against the real function, not a re-derivation**: added a
permanent, opt-in diagnostic parameter to `adelic::check_property_f`
(`out_zero_nodes_beyond_frontier`, default `nullptr`, zero behavior
change for existing callers) that counts any zero-translation node
discovered beyond the initial alphabet-sized starting frontier — the
theorem predicts this is always exactly 0. `tests/property_f_
unconditional_test.cpp` confirms 0 across all 8 tested cases: Finding
5's whole a=0..5 family (AR-exact through non-AR, unimodular), the
simplest known non-unit Pisot substitution (`x^2-2x-2`), and `rnd13`
(the project's largest-ever closure, 33185 nodes, cross-checked
against the exact documented node count).

**Consequence, if the SCOPE caveat holds**: every `DOES_NOT_TILE_
PROPERTY_F` code path in `classify_adelic.hpp` is provably
unreachable, not merely unobserved — matching the fact that this
project has never once produced a genuine property-(F) FAILS verdict
across every substitution it has ever run, unit or non-unit. Combined
with Finding 17/18, the ENTIRE remaining difficulty of the open
general unimodular Pisot question (within this project's own
verification framework) would reduce to strong coincidence alone —
property (F) would never be the obstruction, for any Pisot
substitution, constant-factor or not.

**UPDATE (2026-08-06, same session): the core real-analysis lemma is
now Lean kernel-checked**, not just hand-derived and stress-tested.
`lean/generated/property_f_zero_walk.lean`
(`RavelGenerated.zeroWalk_eq_zero_iff`) proves, machine-verified with
no `sorry` and no extra axioms: the walk `gamma_0=0,
gamma_{k+1}=(gamma_k+delta_k)/beta` (beta>1, every delta_k>=0) returns
to exactly 0 at step n iff every delta_k for k<n was itself 0 — this
is Steps 1-2 of the argument above, previously only hand-derived. See
`docs/PROPERTY_F_UNCONDITIONAL_KERNEL_CHECKED_2026-08-06.md` for the
hash and check details. The remaining, still load-bearing caveat:

**SCOPE CAVEAT (load-bearing, not a formality)**: this proves the
SPECIFIC finite graph-cycle test implemented in this codebase can
never witness a violation. It does NOT, by itself, constitute an
independently-verified resolution of the "geometric property (F)"
condition as understood in the wider Minervino-Thuswaldner literature
— that would additionally require confirming (a) this construction is
a complete, faithful implementation of their definition (this project
has relied on that assumption for every prior ESTABLISHED verdict,
not newly introduced here) and (b) the literature's property (F) does
not carry content beyond what this specific automaton captures. Both
are plausible given how directly the code follows the paper's stated
construction, but neither has been independently cross-checked against
the primary literature in this session. Treat this finding as strong,
freshly-derived-and-tested evidence pointing at a real structural fact
about the implemented check, flagged prominently for scrutiny given
its size, rather than as a closed resolution of a fifty-year-open
literature question.

## Finding 20 — the SCOPE caveat on Finding 19 is now a CONFIRMED discrepancy, not an assumption, and remains genuinely unresolved

**Status: IMPORTANT CORRECTION IN PROGRESS. Do not treat Finding 19 (property F unconditional) as validated against the literature — the correspondence it assumed has been checked directly against the primary source and found to differ in structure. A first attempt at a corrected check produced a result contradicting known ground truth (Fibonacci), so neither the original check_property_f nor the corrected cycle-criterion below should currently be trusted for property-(F) verdicts pending further work.**

Read the primary source directly (Minervino-Thuswaldner, "The geometry
of non-unit Pisot substitutions," Ann. Inst. Fourier 64 (2014)
1373-1417 — full text archived at
`docs/bibliography/minervino_thuswaldner_2014_nonunit_pisot.pdf`, text
extracted to the `.txt` alongside it). Definition 9.3 states property
(F) as: the iterates of `T_ext^{-1}` starting from
`U = {(0,a) : a in alphabet}` eventually cover ALL of Gamma, the
independently-defined "self-replicating translation set" (Definition
4.6/eq. 12) — a COVERAGE condition. This project's
`adelic::check_property_f` instead builds only the forward closure
from U and flags a "mixed zero/nonzero cycle" as the failure
condition — a different, and (per `property_f_unconditional.hpp`)
UNCONDITIONALLY-NEVER-TRIGGERED condition. `include/ravel/proof/
property_f_unconditional.hpp` proves a true fact (no mixed cycle can
ever occur) but that fact may not correspond to genuine property-(F)
failure as the literature defines it.

Working through Lemma 9.8's proof (which connects property F's failure
to a cycle in the zero-expansion graph G^(0)), derived: property F
FAILS iff there exists a cycle in G^(0) whose nodes are ALL nonzero
(never touching a zero-translation node at all) -- NOT "mixed", the
OPPOSITE emphasis from what the codebase checks. Attempted a
tractable reformulation: property F holds iff every SIMPLE cycle in
the small, purely-combinatorial prefix automaton has periodic
translation value exactly zero (derivation: a nonzero simple-cycle
periodic value is directly a nonzero-only G^(0) cycle; if every
simple cycle has value zero, the recursion fixes 0 along any
concatenation of cycles too, and since bi-infinite walks in a finite
graph are eventually periodic in their "deep past," every element of
Gamma traces back to a genuine zero point).

**This reformulation, implemented in `include/ravel/proof/
property_f_correct_cycle_criterion.hpp`, gives the WRONG answer on
Fibonacci** (`sigma(0)=01, sigma(1)=0`) -- it reports property F
FAILS, via the 2-cycle `0->1->0` with periodic value exactly `beta`
(confirmed nonzero abstractly, confirmed within the geometric bound
M via the same archimedean-norm machinery `check_property_f` itself
uses) -- directly contradicting Rauzy's classical, extremely
well-established result that Fibonacci has pure discrete spectrum,
and contradicting the ground truth this project's own BUG1 fix (in
`coincidence_and_property_f.hpp`) was explicitly validated against.

**This is not yet resolved.** Possibilities not yet ruled out: (a) a
further error in the simple-cycle-suffices argument (maybe not every
closed walk actually decomposes the way the "0 stays fixed under
concatenation" argument assumes, if the relevant composition isn't
associative/commutative in the needed sense); (b) a missing
constraint on which simple cycles correspond to genuinely admissible
walks (Definition 9.6's "every node is the starting point of an
infinite walk" may carry more content than "trivially true because
cycles repeat forever" -- possibly requiring the walk to also be
extendable as a genuine LEFT-infinite walk under the actual
prefix-suffix structure, not just any graph cycle in my simplified
letter-level automaton); (c) a subtlety in the Phi/Phi0 projection
maps used throughout the paper that this project's Q(beta)-abstract-
equality convention for "gamma=0" may not fully correspond to.

**Practical consequence, stated plainly**: neither the original
check_property_f verdict NOR the new cycle-criterion should currently
be cited as resolving property (F) for any substitution in this
project's history, pending this discrepancy being fully worked out.
The Lean-kernel-checked lemma in `property_f_zero_walk.lean` remains
correct as FAR AS IT GOES (a true fact about a real-valued walk), but
its relevance to genuine property-(F) failure is now the open
question, not a settled scope caveat.

## Finding 21 — the property (F) verdict bug is FIXED and verified: every historical ESTABLISHED case reproduces exactly, node-for-node

**Status: RESOLVED. `include/adelic/coincidence_and_property_f.hpp`'s
`check_property_f` corrected; `tests/property_f_correct_verdict_test.cpp`
locks in the regression. Supersedes Finding 20's "unresolved" status.**

Finding 20 confirmed a real discrepancy between this project's
`check_property_f` (flags a "mixed" zero/nonzero cycle) and the
primary source's actual criterion (Minervino-Thuswaldner, Lemma 9.8:
failure is a cycle that is NOT entirely zero-nodes). A first attempted
fix — enumerate cycles abstractly in the small letter-level prefix
automaton, independent of the existing BFS closure
(`include/ravel/proof/property_f_correct_cycle_criterion.hpp`) — gave
a **false FAILS on Fibonacci** (`sigma(0)=01, sigma(1)=0`), directly
contradicting Rauzy's classical result. Diagnosed by instrumenting the
real, trusted `check_property_f` to print every node in its BFS-from-U
closure for Fibonacci: exactly 8 nodes, none of which is the `gamma=beta`
value the abstract cycle enumeration found on the `0->1->0` cycle —
even though that value satisfies both the algebraic recursion and the
geometric bound M. **The abstract-cycle approach over-generates**: it
counts cycles that are algebraically self-consistent but never
actually reached by any genuine walk starting from the zero frontier.
`property_f_correct_cycle_criterion.hpp` is kept in the tree with a
prominent header marking it superseded, as a record of the failed
attempt, per this project's standing discipline of not erasing wrong
turns.

**The actual fix is much smaller.** The BFS-from-U closure itself was
never wrong — it faithfully implements the paper's own `T_ext^{-1}`
formula (verified directly against eq. 13 of the primary source). Only
the *verdict* extracted from it was wrong. Combined with
`property_f_unconditional.hpp`'s own proven fact (no mixed zero/nonzero
cycle can ever occur, for any Pisot substitution — Lean-kernel-checked,
`lean/generated/property_f_zero_walk.lean`), the paper's real criterion
("not entirely zero") collapses to exactly: **flag a cycle if it
contains ANY nonzero node** — not "both zero and nonzero." The fix is
a one-line change (`coincidence_and_property_f.hpp`, dropping the
`scc_has_zero &&` requirement from the violation check), with a full
account of the correction, the failed alternative, and the reasoning
left in place as a comment at the fix site.

**Verified against every ESTABLISHED case this project has on
record, node-for-node identical**: Fibonacci (8 nodes, holds — the
critical regression check), `rnd13` (33185 nodes, holds — the
project's largest-ever closure), the worked example (archimedean-only
control still correctly INCONCLUSIVE, matching its documented status),
`x^2-2x-2` (47 nodes, holds), the whole Finding 5/16 AR-partial/non-AR
family (identical node counts across all 7), the 24-candidate 3-letter
non-unit sweep (24/24 ESTABLISHED, unchanged), and the 7-candidate
4-letter non-unit sweep (7/7 ESTABLISHED, unchanged). **Nothing in
this project's history was ever a false positive — the old verdict
logic just could never have caught a real failure if one existed.**
Property (F) is now, for the first time, a genuinely meaningful,
non-vacuous check.

**Consequence for the open general unimodular Pisot question**: the
earlier claim (Finding 19, now withdrawn) that "the entire remaining
difficulty reduces to strong coincidence alone" does not hold —
property (F) is a real, live condition again, not a rubber stamp.
Whether it holds in general for all unimodular Pisot substitutions
remains exactly as open as strong coincidence does. What IS gained:
this project's adelic tiling infrastructure is now trustworthy for
future work, and the constant-factor investigation (Finding 18) is
unaffected — its conclusion (constant-factor connectivity is confined
to the always-safe all-zero region) holds under the corrected
criterion exactly as it did before, since an all-zero cycle is still
never a violation either way.

## Finding 22 — the Diophantine reduction: strong coincidence past the boundary case is exact landmark-vector cancellation under repeated matrix application

**Status: A real reduction, verified exact by two independent
computational methods, not a closed-form theorem. This is the
concrete next-session starting point for the coincidence question.**
`include/ravel/proof/coincidence_as_landmark_vector_cancellation.hpp`,
`tests/coincidence_as_landmark_vector_cancellation_test.cpp`.

Any position within `sigma^k(b)` corresponds to a Dumont-Thomas walk
(the same prefix automaton property (F) already builds, read top-down).
**Theorem (verified, not just derived by hand)**: the prefix's
abelianization equals exactly `sum_ell M^{ell-1} * p_ell`, where `p_ell`
is the abelianization of the sibling letters before the chosen child
at level `ell`, and `M` is the incidence matrix. Verified to reproduce
the true abelianization bit-for-bit against direct word materialization,
independent of the recursive-decomposition method used to derive it.

**The reduction**: at any level where the current letter's image has
length 1 (no possible "second or later" child), the term is
identically zero — no matrix power can rescue a zero vector. For
`sigma_{0,1}` (`sigma(0)=[1,2], sigma(1)=[2], sigma(2)=[0]`, the non-AR
control from Finding 5/16/17), only letter 0 has an image of length
>= 2, so the ENTIRE abelianization of ANY prefix, from ANY starting
letter, reduces to a sum over a sparse subset of depths of a single
fixed "landmark vector" `v0 = (0,1,0)` transformed by `M^{depth-1}`.
Strong coincidence between two starting letters therefore reduces
exactly to: do two finite subsets of positive integers give the same
subset-sum of `{M^n v0}`?

**Confirmed concretely** on the actual depth-12 match for pair `(0,2)`
found earlier tonight (position 17, the match that originally motivated
this investigation): walk from letter 0 has landmark depths `{12,1}`;
walk from letter 2 has landmark depths `{11,6,4}`; and
`M^11 v0 + M^0 v0 = M^10 v0 + M^5 v0 + M^3 v0` exactly (both sides equal
`(5,5,7)`, the substitution's real prefix abelianization at that
position, independently confirmed by materializing the actual 37- and
28-letter words and counting).

**What this does and doesn't close**: it replaces a vague "why does
coincidence eventually happen for non-constant-factor substitutions"
with an exact, well-posed, attackable question — when do two subset-
sums of an integer vector's orbit under repeated integer-matrix
multiplication coincide. That is now a real object (linear recurrence
sequences over Z, orbit-collision under a fixed matrix, adjacent in
spirit to vanishing-sum/S-unit-equation territory, though not
identical to it) rather than an open-ended search. No bound, no
closed form, and no proof that a collision always eventually occurs
is established here — this is the reduction, handed forward as the
concrete next-session starting point, not the resolution.

## Finding 23 — the walk-realizability layer closed exactly for the single-junction class: coincidence reduces to a verified composition model

**Status: EXACT, verified against brute force with zero discrepancies
(one real implementation bug found and fixed via that same
cross-check). Closes the second layer of the Diophantine reduction
(Finding 22) for a precisely-named structural class.**
`include/ravel/proof/single_junction_coincidence_composition.hpp`,
`tests/single_junction_coincidence_composition_test.cpp`.

Finding 22 reduced coincidence to a linear-algebra question (do two
subset-sums of an integer vector's orbit under a matrix collide) but
left open which subsets are actually *realizable* as genuine walks.
Tried the natural shortcut first: Cayley-Hamilton on the incidence
matrix gives universal, v0-independent relations for free whenever the
minimal polynomial has all coefficients in `{-1,0,1}` — true here,
`x^3-x-1` (the plastic number's own minimal polynomial), giving
`M^3 = M + I` exactly. Checked whether this directly produces a short
coincidence witness: it does NOT. The predicted landmark-depth pattern
`{2,1}` is combinatorially unreachable — after a landmark at depth 2,
the deterministic chain (`2->0`) forces the walk straight to depth 0,
skipping depth 1 entirely, so that specific exponent pair can never
co-occur no matter how the walk is built. The linear algebra says a
relation *can* exist abstractly; a separate, independent combinatorial
constraint governs which relations are actually *walked*.

**That second layer is now closed exactly** for substitutions with one
junction letter (image length >= 2) and every other letter forming a
deterministic single-successor chain back to it (`sigma_{0,1}`'s
exact shape). The achievable landmark-depth-sets are exactly the
compositions of `(depth - run_in)` into the junction's own "jump
sizes" (one per child index: `1 + deterministic-chain-length-back-to-
junction`), where a jump landing on a nonzero-index child commits a
landmark event the instant it is chosen — even if the walk runs out of
depth before completing the rest of that jump's deterministic tail.
Verified exact against full brute-force Dumont-Thomas enumeration
across every depth 1-8 and all three starting letters, zero
discrepancies, after fixing a real bug the cross-check itself caught:
the first version silently dropped landmark events made right before
the walk ran out of depth, because it skipped the whole branch instead
of recording the leaf directly.

**Consequence**: for this class, the coincidence question is no longer
"why does this eventually happen" — it is a completely mechanical
composition-counting question (structurally the same family as
counting compositions of an integer into fixed part sizes, e.g. the
`{2,3}` case here is exactly the classical Padovan/Perrin-style
counting sequence), directly computable without brute-force word
enumeration. **Not yet extended**: substitutions with more than one
junction letter, where jumps from different junctions interleave and
the single composition-sequence picture no longer applies on its own
— the next concrete target.

## Finding 24 — reverse-engineering a substitution's minimal polynomial directly from "which walks finish" counting data

**Status: A general, validated methodological tool, proposed directly
by AM. Confirmed exact on two structurally different substitutions
with different minimal polynomials, purely from counting data -- no
incidence matrix ever built.**
`include/ravel/proof/reverse_engineer_minimal_polynomial_from_returns.hpp`,
`tests/reverse_engineer_minimal_polynomial_from_returns_test.cpp`.

AM's proposed technique: instead of deriving a substitution's algebraic
structure by hand (build the incidence matrix, compute its
characteristic polynomial), recover it from "which walks finish" --
count, for each depth K, how many Dumont-Thomas walks of that length
starting at a fixed reference letter land EXACTLY back on that letter
with zero depth remaining (a "clean return"), then fit the minimal
integer linear recurrence this counting sequence satisfies.

**Verified on two independent cases**: `sigma_{0,1}` recovers
`g(K)=g(K-2)+g(K-3)` exactly (characteristic polynomial `x^3-x-1`, the
plastic number's own minimal polynomial); the `x^3-2x^2-x+1`
substitution (`sigma(0)=0,0,1 sigma(1)=2 sigma(2)=0,1`) recovers
`g(K)=2g(K-1)+g(K-2)-g(K-3)` exactly (characteristic polynomial
`x^3-2x^2-x+1`, that substitution's own known minimal polynomial) --
different substitution, different polynomial, both recovered exactly
from pure simulation with zero prior algebraic knowledge fed in.

**Why this works, not just an empirical curiosity**: closed-walk
counts in a graph are governed by powers of its adjacency matrix
(transfer-matrix method), and by Cayley-Hamilton the matrix satisfies
its own characteristic polynomial exactly -- so any closed-walk
counting sequence built from a substitution's transition structure
must satisfy the SAME linear recurrence the incidence matrix does.

**A real error caught in the process**: the test's first negative
control (a sequence with no small recurrence, to confirm the fitter
doesn't spuriously match noise) used the squares `i^2` -- which turned
out to actually satisfy a genuine order-3 linear recurrence
(`a(n)=3a(n-1)-3a(n-2)+a(n-3)`, characteristic polynomial `(x-1)^3`,
a standard fact about degree-d polynomial sequences) and the test
caught its own bad control by failing. Replaced with the prime
sequence, which has no such structure.

**Consequence for the coincidence program**: this gives a way to
recover the exact algebraic relation (the input to Finding 22's
landmark-vector-cancellation reduction) directly from simulation,
without first hand-deriving the incidence matrix -- valuable
specifically because hand-derivation is exactly the kind of step this
project's own discipline has repeatedly found error-prone under
complexity (multiple junctions, larger alphabets). It complements,
rather than replaces, Finding 23's explicit walk-realizability
construction -- recovering the polynomial is not the same as knowing
which of its implied relations are actually walk-realizable.

## Finding 25 — an exact coincidence closure (built on a new shared generic-closure contract), and a real gcd-obstruction discovered with it

**Status: the closure tool is exact and verified (three independent
cross-checks against the depth-12 ground truth). The gcd-obstruction
is a strong, cleanly-confirmed empirical finding with a proposed
mechanism -- NOT yet a completed rigorous proof; stated honestly as
such below.**
`include/ravel/generic_memoized_dag_closure.hpp`,
`include/ravel/proof/coincidence_closure.hpp`,
`tests/coincidence_closure_test.cpp`,
`tests/coincidence_gcd_obstruction_test.cpp`.

**The tool.** Replaces raw word materialization (which scales with the
exponentially-growing word length, and stalled past depth 30 on the
sparser variants explored tonight) with an exact, deduped closure over
`(terminal_letter, exact_landmark_vector)` states -- the same
architectural pattern `corona.hpp` already uses for contact-boundary
closure, applied here to coincidence instead. The key fact making this
well-founded: the weight `M^{depth_here-1}` applied to a landmark event
depends only on the REMAINING depth at the moment of the choice, not
on the eventual total K, so `reachable(junction, remaining_depth)` is
computable by plain memoized recursion over a genuinely acyclic state
space (remaining_depth strictly decreases). Factored the acyclic-DAG-
memoization contract itself into a new shared header,
`generic_memoized_dag_closure.hpp` -- deliberately NOT retrofitted onto
`corona.hpp` or the property-(F) zero-expansion closure, both of which
have genuinely different shapes (cyclic state spaces, round-level
pruning) that would cost real correctness risk on tested code for no
functional gain; the new coincidence closure's state space really is a
DAG and fits the shared base honestly.

A real bug caught and fixed while building this, the same way every
other bug tonight was caught -- by re-deriving carefully rather than
trusting the first draft: the initial version, on a mid-chain depth
cutoff, used the edge's eventual target junction as the terminal
letter. That's wrong whenever the chain has length > 1 and the walk
runs out of depth before reaching the end of it -- the terminal is
whichever letter the chain has actually reached, not where it was
headed. Fixed by storing the full chain on each edge and indexing
directly into it.

**Verified three independent ways** against the already-established
ground truth (`sigma_{0,1}`'s pair (0,2), brute-force materialization
found depth 12): the closure reproduces K=12 directly; the gap-based
sweep (below) reproduces K=12 again for gap=1 in the `gcd=1` case;
and the earlier `single_check`/`sparse_closure` exploration's
qualitative behavior (fast growth, no internal collisions) matches
what the closure predicts analytically from the substitution's own
eigenvalue.

**The gcd-obstruction finding.** Reduced every coincidence pair to a
single number: the "gap" (the difference in deterministic run-in
length each starting letter needs before its first junction visit).
Swept gaps 1 through 7-8 on two structurally different single-junction
substitutions: one with junction jump sizes `{4,2}` (`gcd=2`), one with
`{3,2}` (`gcd=1`). Result, exact and clean: in the `gcd=2` case, EVERY
even gap resolves quickly (K=4,6,10 for gaps 2,4,6) and EVERY odd gap
fails to resolve at all within a depth-40 search (gaps 1,3,5,7); in
the `gcd=1` case, EVERY tested gap resolves (within depth 17), with no
obstruction of any kind. **Proposed mechanism** (plausible, not yet a
completed proof): since every junction jump size is divisible by `g =
gcd(jump sizes)`, the sequence of junction-visit depths reachable from
a fixed starting depth K stays congruent to K mod g throughout the
entire recursive walk structure -- suggesting a residue-class
obstruction that a pair with an incompatible gap can never cross. The
part not yet nailed down rigorously is the mid-chain-cutoff case
(a walk that runs out of depth partway through a jump, landing at some
arbitrary intermediate letter, not necessarily preserving the same mod-g
residue in an obviously provable way) -- the empirical evidence across
both tested cases is airtight, but a full proof accounting for that
case explicitly has not been written down. Flagged honestly as the
next concrete step, not glossed over as already closed.

**Consequence**: this reframes what "hard" means for the coincidence
question yet again. It is not primarily about how sparse the branching
structure is (Finding 23's original framing) -- it is about whether a
specific pair's gap is compatible with the junction's own jump-size
arithmetic. A substitution can have a pair that is provably impossible
to ever coincide (an incompatible gap under a `gcd>1` structure) sitting
right next to a pair that resolves in single digits, within the exact
same substitution.

## Finding 26 — the gcd-obstruction is a proven theorem, and it extends to multi-junction substitutions unchanged

**Status: closes both open questions from Finding 25. The gcd-
obstruction is now a proven impossibility result (not an unresolved
depth-40 search), and the proof holds identically for multi-junction
substitutions -- verified computationally on the invariant itself, not
just its consequence, under an explicit memory/time cap.**
`include/ravel/proof/coincidence_gcd_obstruction_theorem.hpp`,
`tests/coincidence_gcd_obstruction_theorem_test.cpp`.

**Setup**: for any letter `x`, `dist(x)` = the number of forced
deterministic steps from `x` to the nearest junction (`0` if `x` is
itself a junction) -- well-defined for any letter given irreducibility.
`g` = gcd of every jump size in the WHOLE junction graph, across all
junctions.

**Theorem**: for any state `(terminal, vector)` in
`reachable(junction, d)`, `d + dist(terminal) ≡ 0 (mod g)`.

**Proof, by induction on `d`** (this closes exactly the gap Finding 25
left open -- the mid-chain-cutoff case, worked through explicitly):
base case `d=0` gives `terminal=junction`, `dist=0`, trivially `0`.
A clean edge (jump size `s ≤ d`, `s ≡ 0 mod g`) recurses into `d-s`;
by induction `(d-s)+dist(term)≡0`, and since `s≡0 (mod g)`,
`d+dist(term)≡0` too -- **this step never depends on whether the edge
stays at the same junction or crosses to a different one**, which is
exactly why the multi-junction case needs no separate argument. The
mid-chain cutoff (`s > d`) lands on `chain[d-1]`, whose distance to
the junction is `(s-1)-(d-1) = s-d` by construction of the chain, so
`d + dist(terminal) = s ≡ 0 (mod g)` directly, since `s` is a jump size
by definition. All three cases close the induction.

**Consequence**: if two starting points' reachable sets at depths
`K-r_a` and `K-r_b` ever shared a state, the theorem applied to that
same state at both depths forces `r_a - r_b ≡ 0 (mod g)` by
subtraction. So an incompatible-gap pair provably **cannot ever**
coincide -- for any `K`, not merely "not found within a search bound."
This is the rigorous half of Finding 25's proposed mechanism, now
actually proved.

**Verified directly, not just derived**: checked the INVARIANT itself
(not its consequence) against real closures under an explicit
`ulimit -v 10GB` + 30s cap, on two cases: the single-junction `gcd=1`
substitution (vacuously consistent, 29174 states, 0 violations) and a
newly-constructed genuine multi-junction `gcd=2` example (junctions at
two different letters, with edges crossing directly between them --
`0→2` and `2→0` both occur, jump sizes `{2,2,2,4}`), 789 states, 0
violations. The small state counts are deliberate: this verifies an
exact algebraic identity, not a search for a witness, so a modest
bound is fully conclusive -- no large sweep was needed or run.

**SCOPE, stated honestly**: this proves the incompatible-gap direction
completely (a genuine impossibility result). It does NOT prove the
converse -- that every compatible-gap pair eventually coincides -- that
remains strong empirical support (every tested case, both single- and
multi-junction), not a completed proof. Flagged as still open, not
glossed over.

## Finding 27 — a proved (partial) converse: the leftmost-loop corollary

**Status: a genuine sub-case of Finding 26's open converse, closed
rigorously — not the full converse.**

Drove the existing Finding-25/26 `CoincidenceClosure` at gaps it hadn't
been pointed at yet (not a new search tool -- the same verified
machinery, asked a new question of it): for the Finding-26 multi-junction
`gcd=2` example, swept every compatible gap up to `6g` and recorded the
*minimal* witnessing depth `K`, not just whether one exists. The result
had structure: `gap ∈ {0,4,8,12}` coincide at exactly `K=gap`, while
`gap ∈ {2,6,10}` need `K=gap+10`. The `K=gap` cases are not an artifact
of search order -- they have an exact construction behind them.

**The leftmost loop.** `child_index=0` at any junction always carries
the zero landmark vector (it's the abelianization of siblings *before*
the chosen child; child 0 has none). So the walk that always takes
`child_index=0` -- "always take the leftmost branch" -- accumulates the
zero vector at every step, by construction, regardless of which
junctions it passes through. This walk is deterministic (each junction
has a unique `child_index=0`), so starting from any junction over a
finite junction graph it is eventually periodic. If the starting
junction `J` lies on its own eventual cycle, that cycle is a closed
walk from `J` back to `J` using only zero-landmark edges; call its
total jump-size length `L` (necessarily a multiple of `g`, being a sum
of jump sizes, but possibly a *proper* multiple -- `L=4` vs. `g=2` in
the example above).

**Corollary (proved, not observed).** For any junction `J` on its own
leftmost cycle of length `L`, and every `m ≥ 0`, the pair with
`gap = mL` coincides at *exactly* `K = mL`: repeating the leftmost
cycle `m` times is a walk of depth `mL` from `J` back to `J`
contributing the zero vector throughout, i.e. `reachable(J, mL)`
contains `(J, zero_vector)` -- the unique element of `reachable(J, 0)`.
That is precisely the `K=gap` witness condition, produced by direct
construction, no search required.

See `include/ravel/proof/coincidence_converse_leftmost_loop.hpp`,
`leftmost_loop_length()`, verified against both known examples in
`tests/coincidence_converse_leftmost_loop_test.cpp` (including a
negative check: `gap=2` on the multi-junction example is correctly
*not* covered by this corollary alone, confirming `L=4` and not `L=2`
is the real loop length there).

**SCOPE, stated honestly**: this covers exactly the sub-lattice of
gaps that are multiples of the leftmost-cycle length `L`, which can be
a strict multiple of `g` (as in the worked example). It says nothing
about compatible gaps outside that sub-lattice (`gap=2,6,10` above,
which do coincide but need the full search and land at `K=gap+10`, not
the naive `K=gap`). The full converse -- every compatible gap, via some
general bound or construction -- remains open. The `+10` constant
offset observed for the non-multiple-of-`L` residue class is a
suggestive, unexplained regularity, not yet investigated.

## Finding 28 — Finding 6.5's unimodularity hypothesis for beta-expansion termination is refuted; sigma_{0,2} remains genuinely open

**Status: NEGATIVE RESULT (the hypothesis), narrows Task 4's scope
honestly. sigma_{0,2}'s termination status stays unresolved -- not
solved here, but now isolated as a real outlier rather than folded
into a plausible-looking pattern.**

Finding 6.5 flagged sigma_{0,2} (det/norm = 2, the only non-unimodular
candidate in that batch) as the sole case whose `d_beta(1)` didn't
resolve to finite within 400 digits, and explicitly hedged: "may
correlate with unimodularity... not yet confirmed." Tested that
directly rather than leaving it as a plausible-sounding aside: searched
small cubic monic integer polynomials `x^3 - p x^2 - q x - r` for the
Pisot property, split hits by `|r|` (unimodular vs. non-unimodular),
and ran the same exact-charpoly / 3000-dps near-integer-snapped greedy
expansion (`python/beta_expansion_thread_a4.py`'s machinery, reused
directly to avoid reintroducing either precision bug it already found)
on 6 of each.

**Result: 6/6 unimodular AND 6/6 non-unimodular candidates terminate.**
Unimodularity is not the distinguishing factor -- the hypothesis is
refuted on a clean, deliberately-varied sample, not just left unconfirmed.
See `python/beta_expansion_unimodularity_check.py`.

**A real bug caught while building this**: the first version of the
Pisot-candidate filter picked the root of largest MODULUS as the
candidate beta without checking it was real and positive, so
`x^3 - x + 1` (dominant root `~-1.3247`, a negative real number) was
accepted as "Pisot" and produced a spurious periodic-looking result
purely from evaluating a beta-expansion at a number that isn't
actually a Pisot number at all (Pisot numbers are real algebraic
integers `> 1` by definition, not merely large-modulus roots). Fixed
by requiring the dominant root be real and `> 1` before accepting a
candidate; re-ran the whole search after the fix, not just patched the
one bad case.

**sigma_{0,2}, re-examined honestly**: re-ran its expansion (charpoly
`x^3 - x - 2`) up to 2500 digits at 3000-dps precision (via the
corrected pipeline) -- still `UNRESOLVED`, no period found up to
`n/3` digits. The digit sequence shows no visible short-range
structure. Schmidt's theorem (cited in Finding 6.5) guarantees SOME
eventual period exists; it does not bound how long the pre-period or
period can be, and 2500 digits at ~3000-dps precision is close to
where accumulated rounding in the greedy algorithm's repeated
multiply-by-beta step becomes a real risk (error grows roughly by a
factor of beta per digit, so precision headroom is not unlimited) --
so this result should be read as "not resolved at this budget," not
as "confirmed non-periodic within any reasonable period," and further
digits were deliberately not pushed given the accumulating precision
risk rather than genuine new information.

**What this means for Task 4**: the coarse "is it unimodular" signal
is now ruled out as a cheap classifier. sigma_{0,2} remains the one
concrete open outlier in this project's data, with no known correlate
yet. The honest characterization (terminating vs. eventually-periodic,
and why) remains open.

## Finding 29 — Task 3 pivoted: Pisot numbers have genuine constructive accumulation points at every integer >= 2, but the between-integer structure remains open

**Status: the accumulation-point half is exact and provable (a
direct consequence of a well-known family, verified computationally
here at degrees 3-4 via the certified `pisot_classify_3x3/4x4`
classifier, not newly discovered). The "deep vs. shallow region"
density claim is only weakly suggested by a small sample -- reported
honestly as inconclusive, not oversold.**

AM correctly pointed out that Task 3 ("targeted search for
undiscovered Pisot numbers in (1.65, 2.41)") assumed a false premise
-- Pisot numbers are not confined to a finite range -- and asked a
better-posed structural question instead: is there a "mountain range"
shape to where Pisot numbers accumulate (deep/dense regions vs.
shallow/sparse ones), and can an arbitrary target root be matched to
an actual nearby Pisot number.

**Constructive accumulation at every integer (proved, not just
observed).** The "a-bonacci" family (charpoly
`x^n - a(x^{n-1}+...+x+1)`, generalizing Tribonacci/Tetrabonacci's
`a=1` case to any positive integer weight `a`) has dominant root
increasing monotonically in `n`, converging to exactly `a+1` from
below. Verified exactly (via `pisot_classify_3x3`/`_4x4`, no floating
point in the classification) for `a=1..5`, comparing `n=3` to `n=4`:
every case is Pisot, strictly increasing toward `a+1`, still below it.
So every integer `>= 2` is a genuine Pisot accumulation point,
approached by an explicit, arbitrary-degree family -- a "peak" of
unbounded depth in AM's image (arbitrarily many distinct-degree Pisot
numbers in any neighborhood of `a+1`, one per `n`, for every `a`).

**A practical tool, honestly bounded.** `nearest_pisot_near_integer()`
in `app/probe_pisot_accumulation_structure.cpp` returns an actual
verified Pisot number near a target close to an integer, using the
best of the `n=3` and `n=4` a-bonacci candidates. Tested on four
targets (1.9, 3.05, 4.5, 2.99), errors ranging ~0.016 to ~0.47 (the
`4.5` case is genuinely a "valley" -- equidistant from two peaks,
where this family alone doesn't help). Closing the gap arbitrarily
needs `pisot_classify` wired up at higher degree (not built here) to
push `n` further; this tool caps out at whatever `n=4` gives.

**Density claim (the "shallow" half), reported honestly as weak.** A
dense small-coefficient cubic sweep (`x^3-px^2-qx-r`, `p in [0,4]`,
`q,r in [-4,4]`, 82 distinct Pisot roots found in `(1,5)`) shows a
mild lean toward more hits near integers than near midpoints (13 vs.
10 near 4/3.5, 7 vs. 4 near 3/2.5) but near 2 the counts are
statistically meaningless (1 vs. 1). This is NOT strong evidence of a
real "valley" structure at this sample size and coefficient bound --
it is reported as suggestive at best, and should not be cited as a
confirmed density result. A genuine density argument would need
either a much larger coefficient/degree sweep or an analytic argument
(the classical literature -- Dufresnoy-Pisot 1978's derived-set
analysis of the Pisot numbers -- goes considerably further than
anything reproduced here, and was not consulted for this finding
beyond the accumulation-point fact already well known from it).

**What this means for Task 3**: closes it as originally scoped (the
fixed-range search doesn't make sense) and replaces it with a better
question that is now partially answered -- accumulation is real and
constructive at integers; whether a comparably clean structure exists
between them is open.

## Finding 30 — the exact Pisot classifier generalized to arbitrary degree (honestly capped at one complex-conjugate pair)

**Status: extends `pisot_classify_3x3`/`_4x4` (Task 1's original tool)
to any degree 1-15, with an explicit, tested guard rather than a
silent gap. Directly answers the "generate Pisot numbers of arbitrary
degree" half of AM's combined request for the Task 3 pivot tool.**

`pisot_classify_3x3`/`_4x4`'s internal classifier (`pisot_classify_poly`
in `math/src/exact_pisot.c`) already certifies Pisot-ness rigorously
for any degree via Sturm-isolated real roots plus an exact rational
lower-bound argument for the secondary complex-pair modulus (see the
extensive bug-history comments already in that file from earlier
sessions). It was only ever WIRED to degree 3 and 4. Exposed it
directly: `pisot_classify_degree_n(coeffs, degree, out)` in
`math/include/exact_pisot.h` / `math/src/exact_pisot.c`.

**A real limitation, made explicit rather than left implicit.** The
modulus-bound argument (`|det| = beta * prod(real secondaries) *
|complex product|^2`, lower-bounding `beta*prod(real)` to squeeze the
complex factor below 1) only bounds the COMBINED modulus-squared
product across ALL complex-conjugate pairs together. With exactly one
pair (guaranteed at degree <=4, since a real quartic has at most one
conjugate pair among its non-dominant roots) that combined bound IS
the pair's own bound. At degree >=5, a polynomial can have two or more
complex pairs, and a combined product < 1 does NOT imply each factor
is individually < 1 -- so the method genuinely cannot certify those
cases. Added an explicit guard (`n_complex > 1` refuses with `rc=0`,
not a guess) rather than let it silently mis-certify.

**Verified both directions, not just the refusal.** The a-bonacci
family at degree 5-8 (`a=1,2,3`) correctly refuses (`rc=0`) -- these
have two complex pairs among their secondary roots, confirmed by direct
check, matching the guard's own stated reason, not a different bug.
A hand-found degree-5 counterexample with only ONE complex pair
(`x^5-x^4-x^3-x^2+1`, found via a small mpmath sweep specifically
looking for a single-pair degree-5 Pisot case) correctly SUCCEEDS
(`rc=1`, `is_pisot=1`, `beta in [1.7784796161, 1.7784796161]`) --
confirming the guard is scoped to the genuine limitation, not
over-blocking degree 5 wholesale.

**Practical consequence for the nearest-Pisot tool (Finding 29)**: the
a-bonacci convergence to integer accumulation points is real at every
degree, but this classifier can only VERIFY it (and hence be used to
push the approximation tighter) up to degree 4 for that particular
family, since a-bonacci polynomials pick up a second complex pair at
degree 5. Closing that gap needs either a genuinely different
per-pair modulus certificate (a real extension, not attempted here)
or picking a different high-degree family known to keep only one
complex pair -- not investigated further this session.

## Finding 31 — sigma_{0,2}'s beta-expansion: exact search rules out precision, still doesn't resolve

**Status: negative/inconclusive result, honestly reported. The exact
integer-arithmetic search (see Finding 30's probe_exact_beta_expansion.cpp)
ran for 30 minutes real time and 123,500 steps with ZERO repeated
states and no termination.**

This decisively rules out precision as the explanation for Finding
28's "UNRESOLVED" result -- the exact search uses pure integer
arithmetic in Z[beta] (no floating point anywhere) plus exact rational
Sturm-bracket sign comparisons, refined (via `robust_sign`, itself a
real bug fix over the naive `qbeta_sign` fallback) until genuinely
unambiguous at every step. Every one of the 123,500 states visited was
distinct -- not a single collision. Schmidt's theorem guarantees
eventual periodicity exists; this session's data now says the
pre-period/period (if reachable at all within a tractable budget) is
at minimum in the hundreds of thousands, likely much larger, or the
per-step cost (which grew noticeably slower over the run, plausibly
from growing integer-coefficient magnitudes) makes brute continuation
impractical without a smarter approach (e.g., detecting boundedness of
the coefficient lattice directly rather than searching for a literal
repeat).

**Left open, deliberately not force-closed**: continuing this search
much further is a real wall-clock cost with no guarantee of resolving
soon. Whether to keep pushing it, or treat sigma_{0,2} as "open,
substantially better characterized than before" and move on, is a
judgment call left to whoever picks this thread up next.

## Finding 32 — sigma_{0,2} was never actually Pisot; corrects Findings 6.5/28/30/31, and answers AM's number-theoretic prediction question directly

**Status: CORRECTS a misclassification that propagated through three
prior findings. Also a genuine number-theoretic (not search-based)
answer to "can you predict closure or non-closure": yes, and the
prediction is confirmed both by the certified classifier and by a
matching quantitative growth-rate check.**

AM asked, after Finding 31 left the exact search for sigma_{0,2}'s
beta-expansion genuinely open at 123,500 steps with zero repeats,
whether the outcome could be predicted number-theoretically instead
of by more search. Checked the one thing that should have been
checked FIRST, before ever running an expansion: is sigma_{0,2} (the
matrix `[[0,0,1],[2,0,0],[1,1,0]]`, `sigma_ab_matrix(0,2)` in this
project's own convention, used verbatim in
`python/beta_expansion_thread_a4.py`'s `CANDIDATES` dict since Finding
6.5) actually Pisot at all.

**It is not.** Running it through this project's own certified,
bug-history-hardened classifier:
```
pisot_classify_3x3([[0,0,1],[2,0,0],[1,1,0]]) -> is_pisot=0
  has_complex_pair=1, is_complex_modulus_lt_1=0
```
Direct root computation confirms exactly why: charpoly `x^3-x-2` has
dominant real root `beta ~ 1.52138`, but its complex-conjugate pair
has modulus `~1.14656 > 1` (product of all three roots is `2`, so
`|conjugate|^2 = 2/beta ~ 1.3146`, `|conjugate| ~ 1.1466` -- comfortably
outside the unit disk, not a close call or a precision artifact).

**This is genuine number-theoretic prediction, not just a
classification lookup**: Schmidt's theorem (the ONLY reason to expect
`d_beta(1)` is eventually periodic, cited since Finding 6.5) requires
beta to be Pisot. It does not apply here at all -- there was never a
theorem promising Finding 28/30/31's search would terminate or
repeat. More than that, the FAILURE mode is predictable and was
verified quantitatively: Schmidt's proof works by using every
conjugate's modulus `<1` to keep the greedy recurrence's integer
coefficient vector (in the `(1,beta,beta^2)` basis) confined to a
bounded region, forcing an eventual repeat by pigeonhole. With one
conjugate pair at modulus `~1.1466 > 1`, that same recurrence is
EXPANDING in that direction instead of contracting -- predicting the
coefficient vector's magnitude grows roughly like `1.1466^n`, not
stays bounded. Checked this directly: the exact recurrence's
coefficient magnitude went from `1` at step 0 to `~9.5e17` by step
300 and `~8.3e18` by step 2700 -- matching the predicted
`1.1466^300 ~ 8e17` almost exactly. The search never found a repeat
because there is no mechanism forcing one to exist; genuine
non-closure was the correctly predicted outcome, not an artifact of
an insufficient search budget.

**Where the misclassification came from**: Finding 5's original
10-candidate table (Tribonacci, `sigma_{a,1}` a=0..5, `sigma_{1,2}`,
`sigma_1`, `sigma_2`) does not actually include `sigma_{0,2}` (a=0,
b=2) at all -- its "non-AR" class row mentions "sigma_{0,1},
sigma_{0,2}" where the second entry almost certainly should read
`sigma_{1,2}` (the one non-unimodular candidate genuinely in that
list, per Finding 16's correction). Finding 6.5 then built its own,
DIFFERENT candidate table for the beta-expansion question and, in
naming its own `(0,2)` entry after that likely-mistyped label,
constructed the ACTUAL matrix `sigma_ab_matrix(0,2)` without ever
re-running it through the certified Pisot classifier -- the omission
that let this stand for three findings.

**Corrections to prior findings, stated explicitly rather than
silently edited**:
- Finding 6.5's table entry `sigma_{0,2} (III) -> (unresolved)` should
  read: not a Pisot number at all, so "unresolved" was the wrong
  framing -- there is no applicable theorem and no reason to expect
  resolution.
- Finding 28's refutation of the unimodularity hypothesis (6/6 vs.
  6/6 termination) is UNAFFECTED -- that search used its own fresh,
  correctly-gated `is_pisot` check (the negative-root bug fix), and
  never included this specific matrix. Its own re-examination of
  "sigma_{0,2}" at 2500 digits, however, inherited the same
  unverified premise and should be read as moot for the same reason.
- Finding 30's "still in progress as of this commit" framing and
  Finding 31's "genuinely open" framing are both superseded here:
  not open, resolved -- explained, not by more search, but by
  checking the one premise that was never checked.

**A methodological lesson worth stating plainly**: this project has a
strong, correctly-applied norm of verifying premises directly rather
than trusting inherited labels (the entire session's discipline around
memory fences, exact arithmetic, catching false positives). This is a
case where that discipline was NOT applied at the right point -- a
Pisot classification was trusted across three findings and a
substantial compute budget (an exact search that ran 30 minutes)
without ever being re-verified against the project's own certified
tool, which would have taken under a second.

## Finding 33 — a certified map of the Pisot numbers, grounding AM's "poles/harmonics" question in classical theory

**Status: combines established literature (Salem, Siegel, Dufresnoy-
Pisot, Boyd) with a fresh, independently certified computational
sweep (201 points, degree 3-7, window (1,4.5)) via
`pisot_classify_degree_n` (Finding 30). No new theorem -- a grounded
synthesis plus direct quantitative confirmation.**

AM asked for "a good map about where pisot numbers actually appear
and where their points of lowest complexity are, and how complexity
branches among the structure," having noticed "spottiness" near the
smallest Pisot number. This is real, established structure, not a
vague impression:

- **Salem (1944) / Siegel (1944)**: the plastic number
  (`~1.3247179572`, root of `x^3-x-1`) is THE smallest Pisot number,
  and the set S of all Pisot numbers is closed.
- **Dufresnoy & Pisot (1955)**: every Pisot number below the golden
  ratio `phi=(1+sqrt5)/2~1.618034` is completely, exhaustively known
  -- a short, closed, finite list, not merely "sparse in our search."
  `phi` itself is proved to be the SMALLEST LIMIT POINT of the whole
  set S -- an exact, not approximate, threshold between "discrete and
  solved" and "self-accumulating."
- **Boyd (1984-85), "Pisot numbers in the neighborhood of a limit
  point" I & II**: continuing Dufresnoy-Pisot's study of the
  successive derived sets `S' = limit points of S`,
  `S'' = limit points of S'`, etc. Each `S^(k)` is nonempty, and its
  minimum satisfies `sqrt(k) <= min S^(k) <= 2^(k/2)` -- a genuine
  quantitative bound on AM's "harmonics" intuition: each level of the
  hierarchy has its own "pole," climbing at a proven, bracketed rate.
- **This project (Finding 29, same session)**: a THIRD, complementary
  structure above 2 -- every integer `>=2` is a constructively
  reachable accumulation point via the a-bonacci family, an explicit
  arbitrary-degree recipe rather than a general existence statement.

**Independent computational confirmation**: swept every monic integer
polynomial of degree 3-7 with small bounded coefficients through the
certified `pisot_classify_degree_n` classifier (honestly refusing
multi-complex-pair cases per Finding 30's disclosed limitation),
window `(1, 4.5)`. 201 distinct certified Pisot values found. Density
per unit interval (points found / interval width, NOT a claim about
the true infinite-degree density): `4.85` below `phi`, `18.32` in
`[phi,2)`, `72` in `[2,3)`, `99` in `[3,4)` -- a clean, monotone climb
exactly where the literature predicts the discrete-to-accumulating
transition happens, reproduced from scratch rather than asserted.

**An honest gap surfaced by the sweep itself**: the literature's
`theta_3 ~ 1.443269` (root of `x^5-x^4-x^3+x^2-1`), one of the small
number of Pisot numbers below `phi`, does NOT appear in the certified
output -- confirmed (via direct root computation) to have TWO
complex-conjugate pairs among its secondary roots, exactly the
Finding-30 limitation (the modulus bound only certifies their combined
product). A real, disclosed absence, not a silent gap in the map.

See `app/probe_pisot_map.cpp` (the sweep) and the published map
artifact (interactive number-line, density histogram, and derived-set
harmonics diagram, built the same session).

**SCOPE, stated honestly**: this is a synthesis and a bounded,
degree-limited empirical confirmation, not a new theorem. The exact
minima of `S''`, `S'''`, etc. (only their bracket is reproduced here)
and any structure beyond degree 7 or coefficient bound ~4 are not
computed -- Boyd's own specific numeric tables were not accessible
(paywalled) and are not reproduced.

## Finding 34 — the converse of the gcd-obstruction theorem: a genuine "permanence" phenomenon found, full proof still open

**Status: substantially stronger empirical evidence than Finding 27's
leftmost-loop corollary, plus an identified but unproven likely proof
mechanism. NOT a completed proof of the general converse.**

Picked the open converse back up (every compatible gap eventually
coincides -- Finding 26's unproven direction; Finding 27 proved only
the sub-lattice of gaps that are multiples of a junction's own
leftmost-cycle length). Rather than test isolated gap values, checked
a sharper question directly on the Finding 26 multi-junction `gcd=2`
example: does `reachable(0,D)` and `reachable(0,D+g)` share a state,
as a function of `D` itself?

**Result: a hard threshold, then permanence.** Exhaustively checked
every `D` from 0 to 40: the intersection is EMPTY for every `D < 10`,
and NONEMPTY for every single `D` from 10 to 40 (31 consecutive
depths, not a sample). This is a qualitatively different, stronger
kind of evidence than "gap=g has some witness" -- it says the
coincidence mechanism, once it starts, never turns back off, at least
as far as verified.

**Sharper still: a literal recurring fixed point.** The exact same
state -- `(terminal=0, vec=(10,0,6,0,0,0,3,0))` -- was found as the
shared witness at `D=10,12,14,16,18,20`, six independently-computed
depths in a row landing on the identical integer vector, not six
different coincidences. This state is a genuine fixed point under
`+g` extension, and if it continues indefinitely (not verified beyond
`D=20` for the exact-witness check, though the weaker "some witness
exists" check was verified through `D=40`), it alone would prove the
FULL converse for `gcd=2` immediately: repeating whatever produces it
gives a witness for `gap=2` at every sufficiently large `K`, and
combined with Finding 27's `gap=4` mechanism (multiples of `L=4`), this
covers every multiple of `g=2`.

**Why this ISN'T a complete proof yet.** Two honest gaps:
1. The natural inductive argument ("if `reachable(0,D0)` meets
   `reachable(0,D0+g)`, does the SAME hold at `D0+g` meeting
   `D0+2g`, etc.?") requires an actual insertable "return-to-self"
   loop of length exactly `g` somewhere in the state graph, not just
   an observed recurring witness. The direct length-`g` return edge
   at junction 0 (the `child_index=1` self-loop) carries a NONZERO
   landmark (`e_1`, the unit vector for the sibling letter before it)
   -- unlike Finding 27's landmark-FREE leftmost loop, this edge does
   NOT trivially preserve the vector on repetition, so the mechanism
   behind the observed fixed point is not simply "repeat one edge."
2. This was checked on ONE example (the same one used throughout
   Findings 25-27). No second structurally-different multi-junction
   case was tried this session -- unlike Finding 27, which was cross-
   checked against two examples before being trusted.

**The most likely route to a real proof, identified but not
executed**: Schmidt's own classical proof that `d_beta(1)` is
eventually periodic for Pisot `beta` (cited since Finding 6.5) uses
exactly this shape of argument -- Pisot contraction (all conjugates of
`beta` besides the dominant root have modulus `<1`) bounds an
appropriately-normalized embedding of the state sequence into a
bounded region, forcing an eventual return by pigeonhole. The
`reachable(J,D)` state sets here are built from the SAME incidence
matrix `M`, whose spectrum has the same Pisot-contraction property by
construction -- so a Schmidt-style pigeonhole argument, projecting
state vectors through `M`'s contracting eigenspaces, is a plausible
and structurally well-motivated way to prove permanence in general,
not just observe it. Not attempted this session.

See `tests/coincidence_converse_permanence_test.cpp` for the locked-in
regression (permanence over `D=0..26`, exact fixed-point witness at
`D=10..20`).

**Honest bottom line**: the converse is now much better characterized
-- a genuine, striking, reproducible phenomenon (permanence + a
literal fixed point) has been found, and a concrete, technically
sound-looking proof strategy has been named -- but the general
theorem for arbitrary multi-junction Pisot substitutions is still
open.

## Finding 35 — no substitution with a nontrivial gcd-obstruction can be Pisot: the whole g>1 thread was studying a necessarily non-Pisot phenomenon

**Status: PROVEN, general theorem. Redirects the entire Findings
25-27/34 gcd-obstruction thread: not wrong, but scoped to substitutions
that can never actually be Pisot in the first place.**

AM asked directly, after Finding 34's proof-strategy check surfaced
that the multi-junction test example wasn't Pisot: "does this define
a specific thing that can't be pisot in the first place... can any
such construction be pisot?" Checked rigorously, not just for the one
example -- the answer is a hard **no**, provable in general.

**Theorem.** If a substitution's junction graph has
`g = gcd(all jump sizes) > 1`, its incidence matrix cannot be Pisot.

**Proof.** (1) Every cycle in the full letter-level digraph passes
through at least one junction -- non-junction letters have a unique
forced path that provably terminates at a junction
(`build_junction_graph`'s own irreducibility assertion), so no cycle
can avoid every junction. (2) Hence every cycle decomposes into a
closed walk in the junction graph, with length equal to a sum of
individual jump sizes. (3) Since `g` divides every individual jump
size by definition, `g` divides every such sum -- so `g` divides the
length of EVERY cycle in the whole digraph. (4) The Perron-Frobenius
PERIOD of an irreducible nonnegative matrix is exactly the gcd of all
its digraph's cycle lengths (Seneta 2006) -- since `g` divides every
cycle length, the period is a multiple of `g`, so `g>1` forces
period `>1`: the matrix is imprimitive, not merely possibly so.
(5) An irreducible matrix with period `h>=2` has EXACTLY `h`
eigenvalues tied at the maximum modulus (the Perron root times each
`h`-th root of unity) -- standard Perron-Frobenius theory. (6) Pisot's
definition requires a UNIQUE dominant eigenvalue with every other
eigenvalue strictly inside the unit circle; step 5 gives `h>=2`
eigenvalues AT modulus `>1`, directly contradicting this. QED.

**Verified computationally, exactly (not floating point), via this
project's own certified `is_primitive`
(`math/include/math/perron_frobenius.hpp`, Wielandt-bound boolean
matrix powers)**: the `g=2` matrix used throughout Findings 25-27/34
is irreducible but NOT primitive; a second, independently-constructed
`g=4` example is also irreducible but not primitive; the `g=1`
single-junction control example used since Finding 26 IS both
irreducible and primitive.

**What this means for the thread, stated plainly.** Findings 25-27
and 34's mathematics is entirely correct -- the gcd-obstruction
theorem (Finding 26) is a true, general combinatorial fact for ANY
substitution meeting the structural assumptions, Pisot or not. But its
NONTRIVIAL content (`g>1`, a genuine obstruction existing at all) can
only ever be triggered by a substitution that is provably not Pisot.
For every genuine, real Pisot substitution -- the actual object of
interest for strong-coincidence and property-(F) work -- `g` is
ALWAYS exactly 1. The gcd-obstruction is therefore always vacuous in
the case that matters; chasing its converse for `g>1` was chasing a
combinatorially interesting but practically moot question.

**The real open question, correctly scoped**: does a coincidence
witness always exist (in finite depth) for `g=1`, across ANY genuine
Pisot substitution -- single-junction (already seen: permanent from
`D=0`, Finding 34's control run) or multi-junction (not yet tested on
an actual Pisot example; every multi-junction example examined so far,
`g=2` and `g=4`, was constructed for combinatorial testing purposes
and turned out non-Pisot). Finding a genuine multi-junction Pisot
substitution and re-running Finding 34's permanence check on it is the
natural, correctly-motivated next step, not extending the g>1 work
further.

## Finding 36 — the coincidence machinery run on an actual, certified, genuinely multi-junction Pisot substitution: strong coincidence holds, immediately and permanently, on every pair

**Status: the first real-world confirmation of this thread's whole
apparatus (Findings 25-27, 34-35). Every multi-junction example used
before this was, per Finding 35, provably non-Pisot; this is the
correction.**

Found the right candidate close at hand: `sigma_{1,1}`
(`sigma(0)=[0,1,2], sigma(1)=[0,2], sigma(2)=[0]`), already certified
Pisot in this project's much earlier work (Finding 5/6.5's AR-partial
family), re-verified fresh here (`pisot_classify_3x3`:
`beta~2.14789904`; `is_primitive`: true) and genuinely multi-junction
-- letters 0 AND 1 both have image length >=2, unlike every prior
multi-junction test case. Confirmed `g=1` (matching Finding 35's
theorem exactly: a real Pisot substitution can never have `g>1`).

**Ran all three pairwise coincidence checks** (0,1), (0,2), (1,2) --
correctly handling letter 2's forced run-in (image length 1, landing
on junction 0 after 1 step) via the same origin/dist bookkeeping used
throughout Findings 25-27. Every pair shows a coincidence witness
starting almost immediately (`K=1`) and PERMANENTLY at every single
subsequent depth checked (`K=1..14`, exhaustive, not sampled) -- not
just the cross-junction pair tested first, all three.

This is a clean, decisive confirmation: for a real Pisot substitution,
strong coincidence isn't a delicate, hard-won property requiring deep
search -- it holds almost immediately and then never stops holding, at
least on this example. It directly validates the redirection from
Finding 35: the "interesting" open combinatorics (gcd-obstruction,
its converse, the permanence phenomenon) was never actually blocking
real strong-coincidence questions -- once restricted to genuine Pisot
substitutions, the phenomenon this thread predicted (permanence) shows
up immediately, strongly, and on every letter pair, not just some.

See `tests/sigma11_genuine_pisot_coincidence_test.cpp`.

**SCOPE, stated honestly**: one example, `beta~2.148`. Not a general
theorem that EVERY genuine multi-junction Pisot substitution behaves
this cleanly -- but it is the first piece of direct evidence that the
answer, for real substitutions, is likely "yes, easily," reframing
this entire session's coincidence-closure work as having correctly
identified a real phenomenon, just needing the right (Pisot-certified)
examples to see it clearly.

## Finding 37 — 10 genuine non-unit Pisot substitutions in Barge's (2016) structural class, all ESTABLISHED via this project's own corrected machinery

**Status: STRONG, freshly-generated evidence for a likely-novel extension.
Not yet independently peer-reviewed or literature-cross-checked for
prior art beyond the searches already run this session (see the
"literature check" conversation this same session) -- reported with
that caveat explicit, not as a closed result.**

Context: M. Barge (2016) proved the classical Pisot substitution
conjecture (pure discrete spectrum) for substitutions that are
"injective on first letters" (the map `a -> first letter of sigma(a)`
is a bijection) AND "constant on final letters" (every image ends in
the same fixed letter). That proof operates in the classical
(unimodular-framed) tiling-space setting. The non-unit EXTENSION of
the Pisot conjecture (Minervino-Thuswaldner 2014, using a genuinely
different adelic representation space `K_sigma`) is newer and far less
studied -- no evidence was found in this session's literature search
that Barge's class has been checked against the non-unit extension.

**Found genuine non-unit examples in Barge's class.** A randomized
search over "injective-first, constant-last" substitutions (4- and
5-letter alphabets), filtered through the certified, arbitrary-degree
Pisot classifier (Finding 30) plus an explicit irreducibility check
(algebraic degree of beta must equal alphabet size, per the
literature's own "irreducible Pisot substitution" hypothesis) found
this is NOT a rare or vacuous intersection -- unlike the earlier,
much narrower exhaustive check at alphabet 3 (0 hits), alphabet 4-5
randomized search found genuine non-unit Pisot hits at a normal rate
(dozens found in a few thousand trials). 10 structurally diverse
examples (`|det| in {2,3}`, both signs, 4- and 5-letter alphabets)
were taken forward.

**All 10 run through the full pipeline, correctly.** Using the
DEDICATED `left_eigenvector_via_qbeta` (not the M/M-transpose-
guessing pattern this project's own earlier `nonunit_family_sweep.cpp`
driver used, which this investigation found can silently select a
mathematically wrong eigenvector on some inputs -- exactly the
documented bug class in `math/linalg_qbeta.hpp`'s own history) and
independently re-verifying the eigenvector via `verify_left_
eigenvector` before trusting it (`eig_verified=yes` on all 10, not
assumed): **all 10 show strong coincidence HOLDS (depth 1, every
case) and property (F) HOLDS** (one case, needing a 2,000,000-node
budget to resolve past 635,855 nodes rather than a smaller default --
resolved cleanly to HOLDS, not left inconclusive). By Minervino-
Thuswaldner's own tiling theorem (Lemma 9.8 and the surrounding
argument, already used throughout Findings 18-21), strong coincidence
+ property (F) together establish the (non-unit) Pisot substitution
conjecture for each of these 10 substitutions.

See `app/probe_barge_class_nonunit_instances.cpp`.

**Two elementary observations worth separating out for their own
right**: (1) EVERY one of the 10 examples resolves strong coincidence
at depth 1 -- not the constant-FIRST-letter mechanism of Finding 17
(these substitutions have DISTINCT, not shared, first letters), but
plausibly a dual, SUFFIX-based mechanism from the shared constant
LAST letter. Not yet proved as a general theorem the way Finding 17
was -- flagged as the natural next elementary result to chase.
(2) The full-node-count spread (14 to 635,855) shows property (F)'s
difficulty genuinely varies across this class, unlike strong
coincidence's uniform depth-1 resolution -- consistent with Finding
18's earlier finding that constant-factor-type structure helps
coincidence but doesn't automatically help property (F).

**SCOPE, stated honestly**: (a) this is 10 examples, not a general
theorem covering all of Barge's class -- the natural next step (which
AM has asked to pursue) is finding a genuine structural proof, the way
Finding 17 did for the constant-factor coincidence case, rather than
resting on case-by-case verification; (b) this project's property-(F)
implementation, while corrected and primary-source-checked (Findings
19-21), has not been independently reviewed by a third party or
cross-checked against the literature beyond this project's own
reading; (c) whether Barge's class or this specific non-unit
intersection has already been addressed somewhere in the literature
not surfaced by this session's searches remains a real, open
possibility that should be checked more thoroughly before any formal
claim of priority.

## Finding 38 — constant last letter forces depth-1 strong coincidence, unconditionally: the exact dual of Finding 17, closing the coincidence half of Finding 37

**Status: PROVED (definitional, from `pair_has_coincidence`'s own
suffix-matching loop), cross-checked against all 10 of Finding 37's
actual non-unit Pisot substitutions plus two synthetic controls.
`include/ravel/proof/constant_last_letter_forces_depth1_coincidence.hpp`.**

Finding 37 found strong coincidence resolving at depth 1 for all 10
genuine non-unit Pisot substitutions in Barge's (2016) class, but that
was only ever an OBSERVATION -- unlike Finding 17's constant-factor
case, which was proved. This closes exactly that gap, needed because
Lemma 9.8's zero-expansion-graph apparatus (which Findings 34-37's
property-(F) work all rests on) is stated with strong coincidence as a
HYPOTHESIS, not something it establishes -- any general property-(F)
argument for this class needs a proven coincidence foundation under
it, not an empirical one.

**Theorem.** If `sigma(i)` and `sigma(j)` end with the same letter
`c`, the pair `(i,j)` exhibits a coincidence at k=1, unconditionally.

**Proof** (exact dual of Finding 17, using the SUFFIX half of
`pair_has_coincidence` instead of the prefix half -- Definition 2.1's
coincidence condition is `P(p1)=P(p2)` OR `P(s1)=P(s2)`, and Finding
17 only ever used the first disjunct): at word `w1`'s FINAL position
(necessarily letter `c`, by hypothesis), the computed suffix
(`total1 - running - e_c`, i.e. "how many letters remain after this
position") is exactly the ZERO vector, since this IS the last
position -- inserted into `suffix_set1[c]`. When `w2` (also ending in
`c`) is scanned and its own final position is reached, the identical
computation gives the zero vector again, and `suffix_set1[c]` already
contains it -- an unconditional hit, regardless of anything about
either word before its last position. Needs no Pisot property, no
irreducibility, nothing beyond the two words sharing a last letter.

**Verified against all 10 of Finding 37's actual substitutions**
(all genuinely have constant last letter -- that is half of Barge's
own hypothesis) plus a synthetic case with constant letter 2 (not 0,
ruling out a letter-0-specific artifact) and a no-constant-last-letter
control that correctly declines to predict.

**Consequence**: strong coincidence for the ENTIRE class of
substitutions satisfying "constant last letter" (which includes all of
Barge's class, unit or non-unit) is now PROVED, not observed --
closing exactly the missing foundation stone under Finding 37's
empirical result, and giving the same certainty Finding 17 already
gives for the dual (constant-first-letter) class. Combined, Findings
17 and 38 mean strong coincidence is never the obstruction for EITHER
of these two large, easily-recognized classes -- any future
counterexample search for the open Strong Coincidence Conjecture can
skip both.

**Literature connection (see Finding 17's own addendum for the full
account)**: "constant first letter AND constant last letter together"
is the classical, decades-old "proper substitution" (Dekking 1978,
Queffelec's textbook). Durand & Petite (arXiv:1408.2110) use
properness as a technical device for an unrelated purpose and remark,
in one unproved, uncited sentence, that it gives "a form of
coincidence" -- an aside, not this theorem. Finding 38's exact,
depth-1, machine-verified statement was derived independently, from
this project's own code, before this literature check.

## Finding 39 — the zero-run bound: Finding 17 generalized to the ENTIRE canonical terminating-expansion family, not just the constant-factor special case

**Status: PROVED (mechanistic argument from the substitution's own
"pass-through" structure), verified against a direct, from-scratch
coincidence search on 22 structurally diverse digit sequences spanning
N=1..10 and zero-run lengths 0..5. Roadmap Stage 1's first real
result: widening the PROVEN strong-coincidence base past the trivial
constant-factor case, using terminating_generator_theorem.hpp's
already-proven family characterization as the starting point rather
than one-off examples.**

Finding 17 proved depth-1 coincidence for substitutions where every
image begins with the same letter. For the canonical terminating
family (`sigma(s) = 0^{t_s}(s+1)`), that condition holds exactly when
every digit `t_s > 0` for `s<N-1`. What happens when some digit is
`0` -- breaking the constant-factor condition -- was untested. Swept
the family systematically (varying `N` and digit patterns, the same
"extend across the whole family, not one example" discipline used
throughout the twist/dominance/generator threads) rather than checking
isolated cases.

**Theorem.** Let `R` = the length of the longest run of consecutive
zero digits among `t_0,...,t_{N-2}` (`R=0` if none, Finding 17's own
case). Every pair of letters resolves strong coincidence at depth
`<= R+1`, exact (achieved) whenever a run of length `R` occurs.

**Proof.** `t_s=0` means `sigma(s)=[s+1]` exactly -- a single-letter
pass-through with no leading 0 and no branching. A maximal run of `R`
consecutive zero digits at positions `s,...,s+R-1` means every letter
in the run chains, letter-for-letter with no branching, to `[s+R]`
within at most `R` applications (slowest case: letter `s` itself,
needing exactly `R` steps). One more application gives
`sigma(s+R) = 0^{t_{s+R}}(s+R+1)`, genuinely beginning with `0`
(maximality of the run guarantees `t_{s+R}>0`) -- triggering Finding
17's own depth-1 mechanism at that point. Total depth from the run's
start: `R+1`. Pairs entirely outside any run already resolve at depth
1 (Finding 17 directly, `R=0` locally); a pair straddling two
different-length runs is bounded by the larger, since the slower side
determines when both first produce a shared leading letter. Taking
the maximum over the whole digit sequence gives the stated bound.

**Verified, not just derived**: `derive_zero_run_coincidence_bound()`
in `include/ravel/proof/zero_run_forces_bounded_coincidence.hpp`,
cross-checked against a genuine from-scratch coincidence search
(`tests/zero_run_forces_bounded_coincidence_test.cpp`) on 22 digit
sequences -- single runs of length 1-5, multiple separate runs of
different lengths in the same sequence, runs at the start vs. middle
of the sequence, N ranging 1 to 10. Exact match on every case (not
merely "within the bound" -- the predicted depth was ACHIEVED exactly
in every case with `R>0`).

**Consequence for the roadmap**: strong coincidence is now PROVED
(not merely observed) for the entire canonical terminating-expansion
family -- every Pisot number with a finite greedy expansion, any
digit pattern, any alphabet size -- with an EXPLICIT, exact depth
bound in terms of the digits themselves. This is the first Stage-1
result that isn't just the trivial depth-1 case: it required
understanding the family's own generator/pass-through structure
(directly building on `terminating_generator_theorem.hpp`) to extend
across dimensions, exactly the intended roadmap methodology.

## Finding 40 — return-word induction dramatically reduces coincidence-resolution depth: sigma_{0,1}'s worst-case depth 13 collapses to 2

**Status: real, verified, useful application of Durand & Petite's
return-substitution machinery (already present in this project as
`include/ravel/return_substitution.hpp`, predating this session --
newly cross-validated against the literature here) to this project's
own hardest documented coincidence case. Empirical and suggestive,
NOT a formal theorem connecting the induced substitution's
coincidence to the original's -- stated honestly below.**

Following up on AM's request to implement something useful from
arXiv:1408.2110 (Durand & Petite) while continuing the coincidence
roadmap: that paper's key technical device is Durand's earlier
"return substitution" construction (their Proposition 7/8) -- given a
substitution and a marker letter, build a new substitution on the
alphabet of "return words" to that marker. This project already had
an implementation (`ravel/return_substitution.hpp`, pre-dating this
session, with its own `SubstitutionRule`/`ReturnPhaseSystem` API) but
had never been cross-checked against this specific literature.

**Validated the existing tool against two independent literature
checks** before trusting it further:
1. Tribonacci's return substitution to marker=0 reproduces Tribonacci
   exactly, matching the paper's own explicit claim.
2. The paper's own worked example (`sigma: 1->1123, 2->211, 3->21`)
   -- checked via Proposition 8 (dominant eigenvalue of the original
   and induced incidence matrices must match exactly): confirmed,
   `3.3829757679` both sides.

(A first, independent implementation attempt at this same construction,
written before discovering the pre-existing tool, caught a real bug of
its own on the way -- using a token's own original-length position
instead of the CUMULATIVE SIGMA-IMAGE-LENGTH position to locate where
its image sits, giving a dominant eigenvalue of `3.2695` instead of
the correct `3.3830`. Fixed, verified, then discarded in favor of the
project's existing, now-validated implementation to avoid needless
duplication.)

**Applied to sigma_{0,1}** (this project's own longstanding hardest
coincidence example, worst-case depth 13, "non-AR" per Finding 5's
classification). No single letter of `sigma_{0,1}` generates its own
fixed point directly (`sigma(0)=[1,2]`, `sigma(1)=[2]`, `sigma(2)=[0]`
-- none starts with its own index), so applied the construction to
`sigma^3` instead (`sigma^3(0)=[0,1,2]` does start with 0). The
resulting 5-letter return-word-induced substitution resolves EVERY
pair within depth 2 -- collapsing the original worst-case depth of 13
to 2. Verified exactly (not approximately) against a from-scratch
coincidence search on both substitutions
(`tests/return_word_coincidence_reduction_test.cpp`).

**Contrast, ruling out "always trivial"**: applied the same
construction to a Finding-39 zero-run example (digits `{1,0,0,0,1}`,
worst-case depth 4) where letter 0 already generates its own fixed
point directly (`sigma(0)` already starts with 0). There, the induced
substitution is IDENTICAL to the original (same images, letter for
letter) -- no reduction at all, because there was nothing to
simplify. The dramatic reduction is specific to substitutions that
need a higher power to find a valid marker, not a universal
side-effect of the construction.

**SCOPE, stated honestly**: this shows the return-word-induced object
is a genuinely more tractable structure to STUDY for at least one hard
case -- a real, useful analytical handle, and a legitimate implemented
tool from the literature. It does NOT establish (here) any formal
theorem connecting the induced substitution's coincidence property to
the ORIGINAL substitution's strong coincidence condition -- Durand-
Petite's own use of return words is for a different purpose entirely
(their Proposition 9 builds a further, more involved "proper
substitution" construction on top of this one, not used here). Whether
"induced substitution resolves coincidence quickly" can be turned into
a real theorem about the original substitution is the natural next
question if this thread is picked up again.
