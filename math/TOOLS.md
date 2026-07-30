# Math Library — Tools Inventory and Work List

This document tracks the general mathematical tools in the math library
and the application-specific glue.  The goal is to identify what is
generic (reusable) vs what is Pisot-specific (one-off), and to plan
factory/pipeline patterns for future work.

## Tier 0 — Foundational (general, no Pisot assumption)

| Tool | File | Status | Reusability |
|---|---|---|---|
| `BigInt` (mpz_t wrapper) | `bigint.hpp` | ✓ | any math |
| `Rat` (mpq_t wrapper) | `bigint.hpp` | ✓ | any rational math |
| `PolyZ` (Z[x]) | `poly_z.hpp` | ✓ | any polynomial over Z |
| `PolyQ` (Q[x]) | `poly_q.hpp` | ✓ | any polynomial over Q |
| `MatQ` (Q^(n×n)) | `mat_q.hpp` | ✓ | any matrix over Q |
| `poly_at_rational_sign` | `sturm.hpp` | ✓ | any rational eval |
| `sturm_chain(p)` | `sturm.hpp` | ✓ | any polynomial |
| `sturm_root_count(chain, a, b)` | `sturm.hpp` | ✓ | any polynomial |
| `cauchy_bound(p)` | `sturm.hpp` | ✓ | any polynomial |
| `isolate_real_root_rat(chain, a, b, tol)` | `sturm.hpp` | ✓ | any polynomial |
| `qbeta_sign(a, R, β_interval)` | `sturm.hpp` | partial | any Q(β) sign test |
| `Ball` (exact rational interval), `ball_add/sub/mul/div` | `ball.hpp` | ✓ | any exact interval arithmetic |
| `certify_perron_bracket` (Collatz-Wielandt eigenvalue bracket, `long long` vector, overflow-guarded) | `ball.hpp` | ✓ | cheap exact pre-screen for the Perron root of any nonnegative integer matrix (β, λ(G_B), etc.), before committing to full Sturm/Q(β) certification |
| `collatz_step_exact` / `certify_perron_bracket_exact` (Collatz-Wielandt, `BigInt` vector — no overflow guard needed, runs to any `max_iters`/`target_width`) | `ball.hpp` | ✓ | same as above but for matrices/iteration counts where the `long long`-vector version's overflow guard would trip early (fast-growing β, many iterations) |
| `compare_perron_roots_exact(lhs, rhs)` — brackets both matrices' Perron roots and returns `PerronOrder::{less,equal,greater,inconclusive}` from bracket separation alone, zero floating point in the decision | `ball.hpp` | ✓ | rigorously comparing two nonnegative integer matrices' dominant eigenvalues (e.g. BP-core vs. `G_B`-core) without a double-precision tolerance on the ratio |
| `BigFloat` (tunable-precision binary float, `mant * 2^exp`), `bigfloat_add/sub/mul/div/round/cmp` | `bigfloat.hpp` | ✓ | arbitrary-/tunable-precision float arithmetic where a caller-chosen mantissa bit-width (memory cost) matters more than a rigorous enclosure |
| `certify_perron_bracket_bigfloat` / `certify_perron` (with `PerronCertifyMethod::{ExactRationalBracket,TunedBigFloat}`) | `bigfloat.hpp` | ✓ | a switchable Perron-root estimate: bounded, tunable-memory precision (BigFloat) as an alternative to `ball.hpp`'s exact-but-unboundedly-growing rational bracket — NOT a rigorous enclosure, but far tighter than `double` at a chosen, bounded cost |
| `charpoly_faddeev_leverrier(M)` — characteristic polynomial of an integer matrix, exact, with a free Cayley-Hamilton self-check | `charpoly.hpp` | ✓ | any square integer matrix; lifts the Work List item below |
| `is_irreducible(M)`, `is_primitive(M)` — graph-theoretic Perron-Frobenius hypotheses (Seneta 2006, Defs 1.6/1.1), checked via BFS reachability / boolean matrix powers up to Wielandt's bound | `perron_frobenius.hpp` | ✓ | any square nonnegative integer matrix |

## Tier 1 — Quotient ring arithmetic

| Tool | File | Status | Reusability |
|---|---|---|---|
| `QElem` (Q(β) element) | `qbeta.hpp` | ✓ | any quotient ring Q[x]/(c) |
| `QBetaRing` (encapsulates charpoly) | `qbeta.hpp` | ✓ | quotient ring abstraction |
| `add, sub, mul, neg` | `qbeta.hpp` | ✓ | ring arithmetic |
| `beta_inverse` (closed-form) | `qbeta.hpp` | ✓ | any Q(β) where c[d-1] != 0 |
| Modular inverse via Bezout | `bezout.hpp` | ✓ | any quotient ring |

## Tier 2 — GCD and inversion

| Tool | File | Status | Reusability |
|---|---|---|---|
| `divmod` (Z[x], pseudo-remainder) | `poly_z.hpp` | ✓ | any Z[x] |
| `gcd` (Z[x], via pseudo-remainder) | `poly_z.hpp` | ✓ | any Z[x] |
| `divmod` (Q[x], exact) | `poly_q.hpp` | ✓ | any Q[x] |
| `gcd` (Q[x], exact) | `poly_q.hpp` | ✓ | any Q[x] |
| `ext_gcd` (Z[x], Bezout) | `bezout.hpp` | ✓ | any Z[x] |
| `ext_gcd` (Q[x], Bezout) | `bezout.hpp` | ✓ | any Q[x] |
| `reduce_mod_charpoly_z` | `bezout.hpp` | ✓ | any Z[x]/(c) |
| `invert_in_qbeta` | `bezout.hpp` | ✓ | any quotient ring |

## Tier 3 — Linear algebra

| Tool | File | Status | Reusability |
|---|---|---|---|
| `solve_linear` (Q(β)) | `linalg_qbeta.hpp` | ✓ | any linear solve |
| `determinant` (Q(β)) | `linalg_qbeta.hpp` | ✓ | any determinant |
| `right_eigenvector_via_qbeta` | `linalg_qbeta.hpp` | ✓ | generalized eigenvalue (M v = β v) |
| `left_eigenvector_via_qbeta` | `linalg_qbeta.hpp` | ✓ | generalized eigenvalue (v^T M = β v^T) -- a DEDICATED derivation (index-swapped Cramer system), not a wrapper around the right eigenvector; see `docs/RESEARCH_STATUS.md` for why conflating the two was a real, shipped bug (`core.hpp::ensure_exact_qbeta` was silently computing the wrong one) |

## Tier 4 — Application: Pisot / Spectre

| Tool | File | Status | Notes |
|---|---|---|---|
| `dot_qbeta` | `in_h_sigma.hpp` | ✓ | Pisot-specific (Z^d × Q(β)^d → Q(β)) |
| `in_h_sigma` | `in_h_sigma.hpp` | ✓ | the contact-boundary test |
| `isolate_beta` (β > 1 for Pisot) | `sturm.hpp` | ✓ | Pisot-specific (uses general tools) |
| `qbeta_right_eigenvector` (compat) | `qbasis.hpp` (shim) | ✓ | compat shim over Cramer |
| `qbeta_via_cramer` (compat) | `qbasis.hpp` (shim) | ✓ | compat shim |
| `isolate_perron_root(charpoly, upper_bound)` — isolates the LARGEST real root of an arbitrary charpoly, not assuming β > 1 (generalizes `isolate_beta` to any irreducible nonnegative integer matrix, not just Pisot) | `perron_frobenius.hpp` | ✓ | general (Tier 0/4 boundary): any square nonnegative integer matrix |
| `certify_perron_frobenius(M)` — the full computational certificate: irreducibility, exact eigenvalue isolation, exact eigenvector via Q(β), verified entrywise-positive (`qbeta_sign`) and verified `M v = β v` (`verify_right_eigenvector`) | `perron_frobenius.hpp` | ✓ | ties together `charpoly.hpp` + `sturm.hpp` + `ball.hpp` + `linalg_qbeta.hpp`; mirrors, computationally, `perron_unique_of_irreducible` / `perron_exists_of_irreducible` in this project's Lean formalization (same conversation) |

## Identified One-Off Code (Pisot-specific glue)

These are compositions of general tools that solve Pisot-specific
problems.  Each one should be replaceable with a factory call.

| One-off | What it does | General pattern |
|---|---|---|
| `isolate_beta(R)` | Isolate β > 1 of Pisot charpoly | "find positive real root of monic poly with one root > 1" |
| `dot_qbeta(x, v)` | Linear functional in Q(β)^d | "inner product in Q(β)^d" |
| `in_h_sigma(x, j, v)` | Geometric test 0 ≤ <x,v> < v[j] | "lattice point in H_σ" |
| `qbeta_right_eigenvector(M)` | v such that Mv = βv | "generalized eigenvector" |

## Work List: Tools to Build (NOT yet in mathlib)

These are general techniques that would compose into future Pisot and
non-Pisot work.  Each is a factory-friendly tool, not a one-off.

### Algebra
- [ ] **Resultant** `res(p, q)` for polynomials — discriminant, common factors
- [ ] **Discriminant** of a polynomial — algebraic number field discriminant
- [ ] **Subresultant PRS** (preferred over pseudo-remainder for GCD) — keeps coefficients bounded
- [ ] **Polynomial composition** `p(q(x))` — useful for substitution iterates

### Lattice / Number theory
- [ ] **LLL lattice basis reduction** — Pisot number recognition, integer relations
- [ ] **Continued fraction expansion** of algebraic numbers — Pisot/Salem classification
- [ ] **CF for quadratic irrationals** — simpler than general CF
- [ ] **Algebraic number recognition** — given a decimal approximation, find the minimal polynomial

### Linear algebra
- [x] **MatQ** (Q^(n×n) with full operations) — for general linear algebra.
  **DONE**: `mat_q.hpp` (constructor, add/sub/neg, scalar mul, matrix mul,
  transpose, determinant, inverse; `math/tests/test_mat_q.cpp`, 17 tests).
- [ ] **Smith normal form** of integer matrices — for divisibility lattices
- [ ] **Hermite normal form** — for integer linear systems
- [x] **Characteristic polynomial** via Faddeev-LeVerrier (in `barge.hpp` — lift to mathlib).
  **DONE**: `charpoly.hpp`'s `charpoly_faddeev_leverrier` (exact over Q
  internally, verified integral output, free Cayley-Hamilton self-check;
  `math/tests/test_charpoly.cpp`, 24 tests). `barge.hpp`'s existing
  ravel::-level copy is untouched.
- [ ] **Matrix inverse** over Q (general)

### Real approximation
- [x] **MPFR-style high-precision float** — for tight numerical bounds.
  **DONE**: `bigfloat.hpp`'s `BigFloat` (tunable-precision `mant * 2^exp`,
  round-to-nearest, explicit `precision_bits` parameter per operation) plus
  `certify_perron_bracket_bigfloat` / `certify_perron` (the switchable
  `PerronCertifyMethod::{ExactRationalBracket,TunedBigFloat}` entry point).
- [ ] **Mignotte-style root bounds** — for tighter Cauchy-style bounds
- [x] **Interval arithmetic** over Q — for safe numerical computation.
  **DONE**: `ball.hpp`'s `Ball` (exact `Rat` endpoints) predates this
  work-list entry; this line was stale (the tool already existed one
  section above, in Tier 0).

### Pisot-specific
- [ ] **Pisot polynomial classifier** (already in `exact_pisot.c`) — lift to C++
- [ ] **Salem polynomial classifier** — complement of Pisot
- [ ] **Substitution → matrix → charpoly** pipeline — already in `barge.hpp`
- [ ] **Exact eigenvalue** via Q(β) + Cramer (already done)

## Factory / Pipeline Pattern

The user wants a factory that takes (problem, technique) → (algorithm),
and caches the result for reuse.  Sketch:

```lua
-- lua/mathlib_factory.lua
local factory = {}

-- Map (problem_class, technique) -> cached algorithm
factory.cache = {}

function factory.apply(problem, technique)
    local key = problem:id() .. "/" .. technique:name()
    if factory.cache[key] then
        return factory.cache[key]
    end
    local algo = technique:solve(problem)
    factory.cache[key] = algo
    factory:persist(key, algo)  -- write to .lua or .cpp file
    return algo
end

-- Example technique: sturm_isolation
factory.techniques.sturm_isolation = {
    name = "sturm_isolation",
    solve = function(problem)
        local chain = sturm_chain(problem:poly())
        return function(lo, hi, tol)
            return isolate_real_root_rat(chain, lo, hi, tol)
        end
    end,
}

-- Example problem: pisot_charpoly
factory.problems.pisot_charpoly = {
    id = function(self) return "pisot_charpoly_d" .. self:degree() end,
    poly = function(self) return self.cpoly end,
    degree = function(self) return #self.c end,
}

return factory
```

**Concrete instance**: instead of writing `isolate_beta(R)` as a one-off,
define a "find positive real root" technique, parameterize by the
problem, and cache.

## Status Summary

- 366 tests pass in the math library (Tier 0-4, 12 tiers total).
- 12 EXPLODED entries resolve cleanly with exact in_H_sigma.
- `test_qbasis.cpp` (legacy shim) passes cleanly ("all qbasis cases
  passed"); see `docs/RESEARCH_STATUS.md` for the history of issues this
  shim once had and how they were resolved.
- All general tools are in place.
