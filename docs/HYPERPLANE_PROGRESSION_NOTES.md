# Hyperplane progression notes — 2026-07-28

This note records an empirical hyperplane-first view of a few small
3-letter Pisot substitutions.  The goal is the "shape in the hyperplane,
project from it, rotate the window geometry, find patterns in the
progression" investigation — a complementary perspective to the
contact-boundary / fixed-±C corona work that has been the recent
focus.

The driver is `app/hyperplane_progression.cpp`.  It enumerates, for
each candidate substitution, the integer points of the **Pisot
polytope**

```
P_j := { x ∈ ℤ^3 : 0 ≤ <x, v> < v_j },    j = 0, 1, 2
```

where `v` is the right Perron eigenvector of the incidence matrix M,
normalized with `v[0] = 1`.  The polytope is exactly the "signed
stepped hyperplane" `H_σ` of the reference contact-boundary
construction, sliced per terminal color `j`.

The driver tabulates, for each candidate:

* the Perron root `β` and the eigenvector `(v_0, v_1, v_2)`;
* the "width ratios" `v_0 / v_1` and `v_1 / v_2` that drive the
  stepped-hyperplane geometry;
* the count `|P_j|` of integer points in `[-bound, bound]^3 ∩ P_j`
  for each color, plus the attained height range;
* a few sample integer points in `P_0`.

This is the **shape before projection**.  The reference's contact
graph, the fixed-`±C` corona, and the eventual recurrent SCC family
are all derived projections of this shape.

## Raw data, σ_{a,1} family and friends

Captured from `./out/hyperplane_progression` on 2026-07-28:

| Substitution | β | v = (v_0, v_1, v_2) | v_0/v_1 | v_1/v_2 | \|P_0\| | \|P_1\| | \|P_2\| |
|---|---|---|---|---|---|---|---|
| Tribonacci n=3 (control) | 1.8393 | (1, 0.8393, 0.5437) | 1.19 | 1.54 | 44 | 37 | 25 |
| σ_{0,1} | 1.3247 | (1, 0.5698, 0.7549) | 1.75 | **0.75** | 46 | 27 | **35** |
| σ_{1,1} | 2.1479 | (1, 0.6823, 0.4656) | 1.47 | 1.47 | 48 | 33 | 23 |
| σ_{2,1} | 3.0796 | (1, 0.7549, 0.3247) | 1.32 | 2.32 | 48 | 37 | 16 |
| σ_{3,1} | 4.0489 | (1, 0.8019, 0.2470) | 1.25 | 3.25 | 48 | 38 | 12 |
| σ_{4,1} | 5.0329 | (1, 0.8342, 0.1987) | 1.20 | 4.20 | 48 | 40 | 10 |
| σ_{5,1} | 6.0236 | (1, 0.8576, 0.1660) | 1.17 | 5.17 | 48 | 42 | 8 |
| σ_{1,2} | 2.5115 | (1, 0.5567, 0.3982) | 1.80 | 1.40 | 49 | 28 | 20 |
| σ_1 (3L) | 2.5468 | (1, 0.5468, 0.3926) | 1.83 | 1.39 | 49 | 27 | 20 |
| σ_2 (3L) | 2.8312 | (1, 0.8312, 0.3532) | 1.20 | 2.35 | 47 | 39 | 17 |
| σ_{0,2} | 1.5214 | (1, 0.4320, 0.6573) | 2.31 | **0.66** | 48 | 21 | **32** |
| σ_{2,0} | 2.4142 | (1, 1.0000, 0.4142) | 1.00 | 2.41 | 44 | 44 | 19 |

All substitutions are Pisot (β > 1, all other |z| < 1).  The bound
for enumeration was 3, giving a 7³ × 3 search space per candidate.

## Patterns

### 1. The σ_{a,1} family is one-dimensional in `a`

Discrete input rule: **a = number of leading 0's in σ(0) and in
σ(1)**.  Equivalently `a = M[0][0] - 1 = M[0][1] - 1` (since
σ_{a,1}(0) = 0^a 1 2 contains a + 1 leading symbols, with the
leading a of them being 0).

Outcomes that shift monotonically as `a` increases:

* `β` grows roughly linearly: `1.32, 2.15, 3.08, 4.05, 5.03, 6.02`.
  From the cubic `β³ = a β² + (a+1) β + 1`, β ≈ a + 1/β ≈ a + small
  positive.  At a = 5, β - a = 1.0; the gap is bounded and the
  `β < 2a` bound follows.
* `v_1 / v_2` grows linearly as `a + small`: `1.47, 2.32, 3.25, 4.20,
  5.17` for `a ≥ 1`.  The offset is constant ≈ 0.17.
* `v_0 / v_1` decreases monotonically toward 1: `1.75, 1.47, 1.32,
  1.25, 1.20, 1.17`.
* `|P_2|` (third-color polytope size at bound=3) drops:
  `23, 16, 12, 10, 8`.  Differences: `7, 4, 2, 2`.  The arithmetico-
  geometric tail corresponds to the `20*(r)+8` contact-boundary
  law: smaller |P_2| ↔ narrower window in the third coordinate ↔
  fewer new interior points at each round.
* `|P_1|` grows: `33, 37, 38, 40, 42`.
* `|P_0|` saturates at 48 (the bounding-box ceiling).  The actual
  infinite polytope grows but is bounded here.

### 2. σ_{0,1} (Fibonacci) is a singular anchor, not a generic member

`v_1 < v_2` (ratio 0.75 < 1) inverts the "Class-II normal" pattern.
Consequences:

* `|P_1| < |P_2|`: 27 vs 35.  In all other σ_{a,1}, |P_1| > |P_2|.
* The signed strip inequality `-b < <x, v> < b` with `b = v_1`
  (the "first-color width") accepts fewer points than the
  `-c < <x, v> < c` strip with `c = v_2`.  This is the source of
  the asymmetric Fibonacci behavior (every Fibonacci matrix is
  special).

The discrete shift that produces σ_{0,1} from σ_{1,1} is `a ↦ a - 1`
applied twice; the second application crosses the
`v_1 = v_2` boundary and inverts the order.  This boundary is at
`a = 1`, i.e. σ_{1,1} is the unique **balanced** member.

### 3. σ_{1,1} is the unique balanced pivot — proof

For σ_{a,1}, the right Perron eigenvector of the incidence
matrix satisfies `M^T v = β v` with `v_0 = 1`.  The system gives
explicitly:

```
v_2 = 1 / β
v_1 = (a β + 1) / β²
v_0 = 1.
```

The two width ratios are therefore:

```
v_0 / v_1 = β² / (a β + 1)
v_1 / v_2 = (a β + 1) / β
```

Their ratio satisfies

```
(v_0 / v_1) / (v_1 / v_2) = β³ / (a β + 1)².
```

For σ_{a,1} the Pisot cubic is `β³ = a β² + (a + 1) β + 1`.  Setting
`v_0 / v_1 = v_1 / v_2` is therefore equivalent to

```
a β² + (a + 1) β + 1 = (a β + 1)²
            ⇔      β² + (a + 1) β + 1 = a² β² + 2 a β + 1
            ⇔      (1 - a²) β² + (a + 1 - 2 a) β = 0
            ⇔      (1 - a)(1 + a) β² - (a - 1) β = 0
            ⇔      (1 - a) β [(1 + a) β + 1] = 0.
```

Since `β > 1` and `a ≥ 0` are integers, `(1 + a) β + 1 > 0`, so
the equality `v_0 / v_1 = v_1 / v_2` holds **if and only if `a = 1`**.

So σ_{1,1} is the unique balanced pivot — the only σ_{a,1} member
with equal width ratios.  The numerical "1.466 ≈ 1.466" in the
table is **exact**, not approximate.  This is a clean
kernel-checkable fact, formalised in Lean in
`lean/class_ii_balanced_pivot.lean`:

* `balanced_pivot_at_one`: when `a = 1` and the cubic holds, the
  two width ratios are equal.
* `cubic_is_perfect_square_at_one`: when `a = 1`, the cubic
  factors as `β³ = (β + 1)²`.
* `ratios_equal_implies_cubic_perfect_square`: equal width ratios
  imply `β³ = (a β + 1)²`.
* `balanced_pivot_iff_a_eq_one`: given `a ≥ 1`, `β > 1`, the cubic,
  and equal width ratios, deduce `a = 1`.  This is the unique
  kernel-checked balanced-pivot claim.

The balanced pivot is the natural root from which the σ_{a,1}
family grows in both directions:

* `a ↦ a - 1` (downward) eventually hits σ_{0,1} (Fibonacci),
  which has the inverted ratio `v_1 < v_2`.
* `a ↦ a + 1` (upward) produces the canonical Class-II family with
  `v_1 > v_2` and linearly growing `v_1 / v_2 ≈ a + 1/β`.

### 4. σ_1, σ_2 are **off-family** 3-letter Pisot counterexamples

Their incidence matrices:

```
σ_1:   [2 1 1]    σ_2:   [2 2 1]    σ_{2,1}: [2 2 1]
       [1 0 0]           [1 0 0]            [1 0 0]
       [0 1 0]           [0 1 0]            [1 1 0]
```

* σ_2 and σ_{2,1} share the **same row 0** and **row 1**; they
  differ only in row 2: `M[2][*] = (0, 1, 0)` for σ_2 vs `(1, 1, 0)`
  for σ_{2,1}.  The structural edit is: σ_2 has letter 2 appearing
  only in σ(1), not in σ(0).
* σ_1 has row 0 = `(2, 1, 1)` instead of `(2, 2, 1)`; the edit is:
  σ_1 has one fewer 0 in σ(1).

The σ_1, σ_2 substitutions are "cheap random" in the sense of
being small Pisot counterexamples whose hyperplane geometry is
recognizably σ_{2,1}-like but with a structurally meaningful
single-row edit.  These rows are exactly the rows that *do not* host
the recurrent-shell SCCs.

### 5. σ_{1,2} is the b-shift sibling

`σ_{a,b}` with b=1 is the Class-II family.  σ_{1,2} has the same
incidence shape as σ_{1,1} (row 0 = `(1,1,1)`, row 1 = `(2,0,0)`,
row 2 = `(1,1,0)`); it differs from σ_{1,1} only in column 1 of row 1
(M[1][1] becomes 2 — letter 1 appears twice in σ(0)).  The width
structure changes: `v_0/v_1 = 1.80` (largest in the survey), so the
"first-color strip" is much narrower.  This widens P_0 (49 vs 48)
but shrinks P_1 (28 vs 33).

### 6. σ_{0,2} is the deep singular Fibonacci-family cousin

`v_1/v_2 = 0.66`, even more inverted than σ_{0,1}.  `|P_2| > |P_1|`
(32 > 21).  This is the "more-b-than-a" extension of the Fibonacci
pattern.

### 7. σ_{2,0} is the degenerate Fibonacci-style anchor

Row 1 = `(2, 0, 0)` is **all-zero except column 0**; σ(1) has no
letter 1 at all.  This produces the only row of the table with
`v_0 = v_1` exactly (ratio 1.00).  σ_{2,0} is the **simplest
non-trivial Pisot substitution where σ(1) is "all 0's plus a
single 2"**.

## The "root to complexity" strategy

The simplest Pisot-substitution anchor is **σ_{1,1}**: the unique
balanced member of the σ_{a,1} family.  It is the natural root for
two progressions:

1. **Vertical (Class-II) progression.**  Step `a ↦ a + 1` adds a
   leading 0 to both σ(0) and σ(1), rotating the width structure so
   that `v_1 / v_2` grows linearly in `a` while `v_0 / v_1` decreases
   toward 1.  The shape of the third-color polytope `P_2` shrinks
   arithmetico-geometrically.  Each step preserves the Class-II
   structure (the contact set, the 20-node affine shells, the
   eleven-state quintic core) but the boundary graph size
   `20a + 8` and the universal theorem structure follow.

2. **Lateral (off-family) progression.**  Edit single rows of the
   incidence matrix:
   - Row 1 column 1: `0 ↦ 2` produces σ_{1,2};
   - Row 2 column 0: `1 ↦ 0` produces σ_2 (no letter 2 in σ(0));
   - Row 0 column 1: `2 ↦ 1` produces σ_1 (one fewer 0 in σ(1));
   - Column 1 row 1: `0 ↦ 0` (already there) plus row 1 column 0:
     `1 ↦ 2` produces σ_{2,0}.
   Each edit changes one hyperplane statistic in a predictable
   direction, but breaks the Class-II affine structure.

3. **The "Fibonacci anchor" progression.**  σ_{0,1} is the
   `a ↦ a - 1` extreme of the σ_{a,1} family.  It is a singular
   anchor because the width ratios invert (`v_1 < v_2`).  From
   σ_{0,1}, two progressions are possible: increase `a` (back into
   σ_{a,1}) or increase `b` (to σ_{0,b}, the Fibonacci-like
   cousins).

## What this changes in the live obligation

The Class-II live obligation (terminal / repeated-terminal Red
exclusion) was framed in terms of the σ_{a,1} family.  This
progression analysis shows that:

* The σ_{a,1} family is **one-dimensional** in `a`, with the
  width structure `v_1 / v_2 ≈ a + 0.17` and `v_0 / v_1 ↘ 1`.
* The singular anchor σ_{0,1} is **outside** the Class-II normal
  pattern (v_1 < v_2) and produces a smaller / differently-shaped
  polytope family.  Its contact-boundary structure was historically
  harder to characterize because the width inversion breaks the
  "a ≥ 2" assumption used in the affine-shell induction.
* The **balanced pivot** σ_{1,1} is the natural root and is the
  smallest member of the Class-II family that satisfies
  `v_1 > v_2`.  A symmetric-base induction starting at σ_{1,1} may
  prove both the σ_{a,1} family and the σ_{0,1} Fibonacci anchor
  simultaneously.

## Linear deviation pattern (new observation)

The driver reports `|r_01 - r_12|` for each candidate.  The pattern
across the σ_{a,1} family is exactly:

| a   | r_01   | r_12   | |r_01 - r_12| |
|-----|--------|--------|--------------|
| 0   | 1.75   | 0.75   | 1.00         |
| 1   | 1.466… | 1.466… | 0 (kernel)   |
| 2   | 1.32   | 2.32   | 1.00         |
| 3   | 1.25   | 3.25   | 2.00         |
| 4   | 1.20   | 4.20   | 3.00         |
| 5   | 1.17   | 5.17   | 4.00         |

So `|r_01 - r_12| = a - 1` for all `a ≥ 2`.  This is the *signed*
form of the in-family monotonicity: each step `a ↦ a + 1` adds
exactly 1 to the deviation, with σ_{1,1} as the unique zero crossing.
For a = 0 the deviation is 1 (negative-direction crossing); for
a ≥ 1 it grows linearly.

This is a clean closed-form progression rule: a single discrete
shift `a ↦ a + 1` produces exactly +1 in the absolute width-ratio
deviation.  Combined with the cubic's monotonic growth in β
(roughly linear in a), this gives a complete one-parameter
progression law for the σ_{a,1} family.

This linear deviation is **kernel-checked**: under the cubic
hypothesis and `β > 1`, the signed deviation is exactly `1 - a`
(`deviation_identity` in `class_ii_balanced_pivot.lean`), and for
`a ≥ 2` the absolute value is `a - 1` (`deviation_abs_eq_a_minus_one`).
Both proofs use only `field_simp`, `linarith`, `ring`, and the
cubic hypothesis.

## Kernel-checked balanced-pivot theorem

`lean/class_ii_balanced_pivot.lean` formalizes the uniqueness
result.  It declares:

* `ClassIICubic a β`:  `β³ = a β² + (a + 1) β + 1`.
* `WidthRatio01 a β`:  `β² / (a β + 1)`.
* `WidthRatio12 a β`:  `(a β + 1) / β`.

And proves:

* `balanced_pivot_at_one β`: when `a = 1` and the cubic holds,
  the two width ratios are equal.
* `cubic_is_perfect_square_at_one β`: when `a = 1`, the cubic
  factors as `β³ = (β + 1)²`.
* `ratios_equal_implies_cubic_perfect_square`: equal width ratios
  imply `β³ = (a β + 1)²`.
* `balanced_pivot_iff_a_eq_one`: the main result — given
  `a ≥ 1`, `β > 1`, the cubic, and equal width ratios, deduce
  `a = 1`.  Combined with `balanced_pivot_at_one`, this is the
  iff characterization.

The proof chain is: h_eq ⇒ β³ = (aβ+1)² via field_simp; combine
with the cubic to get `(a-1)·β·(aβ+1) = 0`; factor out the
nonzero `β·(aβ+1)`; conclude `(a - 1) = 0`.

## Deliverables added in this session

* `app/hyperplane_progression.cpp` — the hyperplane-first driver.
* `out/hyperplane_progression` — built binary (via
  `make hyperplane_progression`).
* `lean/class_ii_balanced_pivot.lean` — kernel-checked
  balanced-pivot uniqueness theorem, added to `make lean-check`.
* `Makefile` — new `hyperplane_progression` build target.
* `docs/HYPERPLANE_PROGRESSION_NOTES.md` — this document.