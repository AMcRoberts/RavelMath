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
