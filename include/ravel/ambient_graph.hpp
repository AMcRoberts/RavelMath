// ambient_graph.hpp
//
// Definition 3.1 (ambient graph), from arXiv:2511.16442 (November
// 2025 paper).  Nodes are the elements of
//   D = { [i, x, j] in A x H_sigma : x = 0 => i < j }
// with edges
//   [i, x, j] -> [i', x', j']
// when there exist
//   (p_1, i, s_1) in P (i' is a PARENT of i: sigma(i') = p_1 i s_1)
//   (q_1, j, t_1) in P (j' is a PARENT of j: sigma(j') = q_1 j t_1)
// with EITHER
//   type 1: M x' = x + l(q_1) - l(p_1)
//   type 2: M x' = -x + l(p_1) - l(q_1)   (i' and j' swapped)
//
// Both type-1 and type-2 are needed for the full ambient graph.
// Type-2 is the same as type-1 with the (i', j') and (p_1, q_1)
// roles swapped AND a sign flip on the x term.
//
// For σ_1, G_B (boundary graph) has exactly 26 nodes;
// tested defensively in the test binary.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <optional>
#include <utility>

#include "ravel/core.hpp"
#include "ravel/faces.hpp"
#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"

namespace ravel {

// Find all (c, p, s) decompositions such that
// sigma(c) = p ++ [inner] ++ s, i.e. c is a parent of `inner` and
// sigma(c) splits into a prefix p, the inner letter, and a suffix s.
//
// `parent_letter`  = c  (the OUTER letter whose image we decomposed;
//                      this is the letter that becomes `next.i` on the
//                      edge according to Def 3.1 / 3.5).
// `inner`          = the letter we searched for (== the original
//                      parameter); recorded for callers that want it.
template <std::size_t d>
struct ParentDecomposition {
    long long parent_letter;
    long long inner;
    std::vector<long long> p;
    std::vector<long long> s;
};

template <std::size_t d>
std::vector<ParentDecomposition<d>> parent_decompositions(
        const std::array<std::vector<long long>, d>& images,
        long long inner) {
    std::vector<ParentDecomposition<d>> result;
    for (std::size_t c = 0; c < d; ++c) {
        const auto& img = images[c];
        for (std::size_t k = 0; k < img.size(); ++k) {
            if (img[k] == inner) {
                ParentDecomposition<d> pd;
                pd.parent_letter = static_cast<long long>(c);
                pd.inner = inner;
                pd.p.assign(img.begin(), img.begin() + k);
                pd.s.assign(img.begin() + k + 1, img.end());
                result.push_back(pd);
            }
        }
    }
    return result;
}

// Abelianization (letter-count vector) of a word w.
template <std::size_t d>
std::array<long long, d> abelianization(const std::vector<long long>& w) {
    std::array<long long, d> l{};
    for (auto c : w) l[static_cast<std::size_t>(c)] += 1;
    return l;
}

// Solve M @ x' = rhs in R^d, where M is the integer incidence
// matrix subst.M.  For Pisot matrices M is singular (rank d-1
// over R), so the system has a solution iff rhs is in the image
// of M.  Returns std::nullopt if no integral solution exists or
// M is singular along the rhs direction (so callers can
// distinguish "no edge here" from "edge exists with x'=0").
//
// **"fast and messy"** version: double-precision arithmetic
// throughout, 1e-12 / 1e-9 tolerance checks on singularity and
// on the final `llround()` snap to integer.  Cheap, no allocation
// beyond the local stack array, and good enough for every
// non-near-Salem Pisot substitution the project has actually
// exercised at production scale.  If you see a contact-boundary
// "EXPLODED" candidate or a non-converging closure, swap to
// `solve_Mx_eq_rhs_exact` (below) for the same call site and
// diff the two outputs -- the disagreement, if any, is the
// tolerance window where the messy version is silently lying.
template <std::size_t d>
std::optional<std::array<long long, d>> solve_Mx_eq_rhs(
        const std::array<std::array<long long, d>, d>& M,
        const std::array<double, d>& rhs) {
    // Augmented matrix [A | rhs].
    std::array<std::array<double, d + 1>, d> aug;
    for (std::size_t i = 0; i < d; ++i) {
        for (std::size_t j = 0; j < d; ++j) aug[i][j] = static_cast<double>(M[i][j]);
        aug[i][d] = rhs[i];
    }
    // Gaussian elimination with partial pivoting.
    for (std::size_t k = 0; k < d; ++k) {
        std::size_t piv = k;
        double max_abs = std::abs(aug[k][k]);
        for (std::size_t i = k + 1; i < d; ++i) {
            if (std::abs(aug[i][k]) > max_abs) {
                max_abs = std::abs(aug[i][k]);
                piv = i;
            }
        }
        if (max_abs < 1e-12) continue;  // singular row
        if (piv != k) std::swap(aug[k], aug[piv]);
        for (std::size_t i = k + 1; i < d; ++i) {
            double factor = aug[i][k] / aug[k][k];
            for (std::size_t j = k; j <= d; ++j) {
                aug[i][j] -= factor * aug[k][j];
            }
        }
    }
    // Back-substitution.  Track consistency on every row, including
    // the singular ones (rhs must vanish there too).
    std::array<long long, d> xprime{};
    for (int i = d - 1; i >= 0; --i) {
        if (std::abs(aug[i][i]) < 1e-12) {
            // Singular row: rhs must be ~0 for solvability.
            if (std::abs(aug[i][d]) > 1e-9) return std::nullopt;
            xprime[i] = 0;  // arbitrary kernel choice
        } else {
            double sum = aug[i][d];
            for (std::size_t j = i + 1; j < d; ++j)
                sum -= aug[i][j] * xprime[j];
            xprime[i] = static_cast<long long>(std::llround(sum / aug[i][i]));
        }
    }
    return xprime;
}

// **"checking"** version: bit-exact rational Gaussian elimination
// via mini-gmp mpq_t.  All division is exact in Q, so no
// accumulated rounding error ever occurs.  The final solvability
// and integrality checks are done as exact equality tests (denom
// == 1, not a rounding tolerance).  Use this as the slow, correct
// reference against which `solve_Mx_eq_rhs` is diffed when a
// result is suspect; or use it as the default if the substitution
// is one of the known near-Salem / "EXPLODED" candidates where
// the messy version's tolerance window is the actual bug.
//
// Takes integer rhs (the data is always built from abelianization
// counts and lattice coordinates, both integer; the double version
// cast rhs to double only because its own arithmetic was in
// double, not because the underlying data needed it).
//
// Cost: each row operation in Gaussian elimination does rational
// multiplies and subtracts on mpq_t with numerator/denominator
// bit-lengths that grow by O(1) per pivot; in practice this is
// ~10-50x slower than the double version for d=3,4 and well
// within the contact-boundary pipeline's per-iteration budget.
// The same Pisot-singular-M convention applies: returns
// std::nullopt if no integral solution exists or M is singular
// along the rhs direction.
template <std::size_t d>
std::optional<std::array<long long, d>> solve_Mx_eq_rhs_exact(
        const std::array<std::array<long long, d>, d>& M,
        const std::array<long long, d>& rhs) {
    auto mpq_new = []() {
        mpq_ptr q = (mpq_ptr)std::malloc(sizeof(__mpq_struct));
        mpq_init(q);
        return q;
    };
    auto mpq_drop = [](mpq_ptr q) { mpq_clear(q); std::free(q); };

    // Augmented matrix [A | rhs], exact rationals, flat d x (d+1).
    std::vector<mpq_ptr> aug(d * (d + 1));
    auto at = [&](std::size_t i, std::size_t j) -> mpq_ptr& {
        return aug[i * (d + 1) + j];
    };
    for (std::size_t i = 0; i < d; ++i) {
        for (std::size_t j = 0; j < d; ++j) {
            at(i, j) = mpq_new();
            mpq_set_si(at(i, j), M[i][j], 1);
        }
        at(i, d) = mpq_new();
        mpq_set_si(at(i, d), rhs[i], 1);
    }
    auto swap_rows = [&](std::size_t r1, std::size_t r2) {
        for (std::size_t j = 0; j <= d; ++j) std::swap(at(r1, j), at(r2, j));
    };

    mpq_ptr factor = mpq_new(), tmp = mpq_new();

    // Gaussian elimination with partial pivoting (exact: pivot
    // choice only affects numerical stability, irrelevant in exact
    // arithmetic, but keeps the row search simple/consistent).
    for (std::size_t k = 0; k < d; ++k) {
        std::size_t piv = k;
        while (piv < d && mpq_sgn(at(piv, k)) == 0) ++piv;
        if (piv == d) continue;  // entire column below is zero: singular
        if (piv != k) swap_rows(k, piv);
        for (std::size_t i = k + 1; i < d; ++i) {
            if (mpq_sgn(at(i, k)) == 0) continue;
            mpq_div(factor, at(i, k), at(k, k));
            for (std::size_t j = k; j <= d; ++j) {
                mpq_mul(tmp, factor, at(k, j));
                mpq_sub(at(i, j), at(i, j), tmp);
            }
        }
    }

    // Back-substitution.  Every solution component must come out
    // an exact integer (denom == 1, not rounding); singular rows
    // require an exactly-zero RHS.
    std::vector<mpq_ptr> xprime(d);
    for (std::size_t i = 0; i < d; ++i) xprime[i] = mpq_new();
    bool ok = true;
    for (std::size_t ii = 0; ii < d; ++ii) {
        std::size_t i = d - 1 - ii;
        if (mpq_sgn(at(i, i)) == 0) {
            if (mpq_sgn(at(i, d)) != 0) { ok = false; break; }
            mpq_set_si(xprime[i], 0, 1);
        } else {
            mpq_set(tmp, at(i, d));
            for (std::size_t j = i + 1; j < d; ++j) {
                mpq_ptr t2 = mpq_new();
                mpq_mul(t2, at(i, j), xprime[j]);
                mpq_sub(tmp, tmp, t2);
                mpq_drop(t2);
            }
            mpq_div(xprime[i], tmp, at(i, i));
        }
    }

    std::optional<std::array<long long, d>> result;
    if (ok) {
        std::array<long long, d> out{};
        bool all_integer = true;
        for (std::size_t i = 0; i < d; ++i) {
            mpz_t den;
            mpz_init(den);
            mpq_get_den(den, xprime[i]);
            if (mpz_cmp_ui(den, 1) != 0) { all_integer = false; mpz_clear(den); break; }
            mpz_clear(den);
            mpz_t num;
            mpz_init(num);
            mpq_get_num(num, xprime[i]);
            out[i] = mpz_get_si(num);
            mpz_clear(num);
        }
        if (all_integer) result = out;
    }

    for (std::size_t i = 0; i < d; ++i) mpq_drop(xprime[i]);
    mpq_drop(factor); mpq_drop(tmp);
    for (std::size_t i = 0; i < d * (d + 1); ++i) mpq_drop(aug[i]);
    return result;
}

// Forward type-1 edges.  M x' = x + l(q_1) - l(p_1).
template <std::size_t d>
std::vector<ANode<d>> forward_edges_type1(
        const Substitution<d>& subst, const ANode<d>& node) {
    std::vector<ANode<d>> result;
    auto p_decomps = parent_decompositions<d>(subst.images, node.i);
    auto q_decomps = parent_decompositions<d>(subst.images, node.j);
    for (const auto& pd : p_decomps) {
        auto l_p = abelianization<d>(pd.p);
        for (const auto& qd : q_decomps) {
            auto l_q = abelianization<d>(qd.p);
            std::array<double, d> rhs;
            for (std::size_t i = 0; i < d; ++i) {
                rhs[i] = static_cast<double>(node.x[i])
                       + static_cast<double>(l_q[i])
                       - static_cast<double>(l_p[i]);
            }
            auto xprime_opt = solve_Mx_eq_rhs<d>(subst.M, rhs);
            if (!xprime_opt.has_value()) continue;  // no integral solution, no edge
            auto xprime = *xprime_opt;
            // Check x' is in D: in_H_sigma(x', j') and (if x' = 0 then i' < j').
            ANode<d> next;
            next.i = pd.parent_letter;
            next.x = xprime;
            next.j = qd.parent_letter;
            if (!subst.in_H_sigma(next.x, next.j)) continue;
            if (next.i >= next.j) {
                bool all_zero = true;
                for (std::size_t i = 0; i < d; ++i)
                    if (next.x[i] != 0) { all_zero = false; break; }
                if (all_zero) continue;
            }
            result.push_back(next);
        }
    }
    return result;
}

// Forward type-2 edges.  M x' = -x + l(p_1) - l(q_1).
// NB: in the type-2 edge, the parent of j is the new i', and the
// parent of i is the new j'.
template <std::size_t d>
std::vector<ANode<d>> forward_edges_type2(
        const Substitution<d>& subst, const ANode<d>& node) {
    std::vector<ANode<d>> result;
    auto p_decomps = parent_decompositions<d>(subst.images, node.i);
    auto q_decomps = parent_decompositions<d>(subst.images, node.j);
    for (const auto& pd : p_decomps) {
        auto l_p = abelianization<d>(pd.p);
        for (const auto& qd : q_decomps) {
            auto l_q = abelianization<d>(qd.p);
            std::array<double, d> rhs;
            for (std::size_t i = 0; i < d; ++i) {
                rhs[i] = -static_cast<double>(node.x[i])
                       + static_cast<double>(l_p[i])
                       - static_cast<double>(l_q[i]);
            }
            auto xprime_opt = solve_Mx_eq_rhs<d>(subst.M, rhs);
            if (!xprime_opt.has_value()) continue;
            auto xprime = *xprime_opt;
            ANode<d> next;
            next.i = qd.parent_letter;
            next.x = xprime;
            next.j = pd.parent_letter;
            if (!subst.in_H_sigma(next.x, next.j)) continue;
            if (next.i >= next.j) {
                bool all_zero = true;
                for (std::size_t i = 0; i < d; ++i)
                    if (next.x[i] != 0) { all_zero = false; break; }
                if (all_zero) continue;
            }
            result.push_back(next);
        }
    }
    return result;
}

// Combined forward edges (type-1 + type-2), matching the
// full Def 3.1 ambient graph.
template <std::size_t d>
std::vector<ANode<d>> forward_edges(
        const Substitution<d>& subst, const ANode<d>& node) {
    auto edges1 = forward_edges_type1<d>(subst, node);
    auto edges2 = forward_edges_type2<d>(subst, node);
    edges1.insert(edges1.end(), edges2.begin(), edges2.end());
    return edges1;
}

}  // namespace ravel
