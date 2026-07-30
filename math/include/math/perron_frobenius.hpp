// math/perron_frobenius.hpp
//
// A computational Perron-Frobenius certificate: given a square,
// entrywise-nonnegative INTEGER matrix, verify irreducibility, isolate
// the Perron (dominant) eigenvalue exactly, construct its eigenvector
// exactly in Q(beta), and verify -- not assume -- that the eigenvector
// is entrywise strictly positive and genuinely satisfies M v = beta v.
//
// This mirrors, computationally, the theorem this project's research
// line formalized in Lean earlier in this conversation
// (free_involution_perron.lean, `perron_unique_of_irreducible` /
// `perron_exists_of_irreducible`): existence and uniqueness of a
// positive eigenvector for an irreducible nonnegative matrix. See that
// file's citations for the classical sources (Perron 1907, Frobenius
// 1912; Horn & Johnson, "Matrix Analysis", 2nd ed., Thm 8.4.4; Meyer,
// "Matrix Analysis and Applied Linear Algebra", Ch. 8; Seneta,
// "Non-negative Matrices and Markov Chains", 2nd ed., Ch. 1 -- the
// same book Mathlib's own `Matrix.IsIrreducible` cites).
//
// What THIS file computes is not a re-proof of the theorem (that's
// what the Lean file is for) -- it is an executable CERTIFICATE for a
// specific matrix: run it, and either every check below passes (in
// which case, by the cited theorem, the eigenvalue produced genuinely
// is THE Perron root and the eigenvector genuinely is unique up to
// positive scale), or a specific check fails and says which.
//
// Composes existing mathlib tools rather than reimplementing them:
//   - `charpoly_faddeev_leverrier` (charpoly.hpp) for the characteristic
//     polynomial,
//   - `sturm_chain` / `sturm_root_count_rat` / `isolate_real_root_rat`
//     (sturm.hpp) to isolate the dominant real root exactly,
//   - `certify_perron_bracket_exact` (ball.hpp) as a cheap starting
//     bracket (Collatz-Wielandt) before refining with Sturm,
//   - `right_eigenvector_via_qbeta` / `verify_right_eigenvector`
//     (linalg_qbeta.hpp) for the eigenvector itself,
//   - `qbeta_sign` (sturm.hpp) for exact positivity of each coordinate.
// New in this file: `is_irreducible`, `is_primitive` (graph-theoretic,
// Tier 0 -- no existing mathlib tool covered these), and
// `isolate_perron_root` / `certify_perron_frobenius` (Tier 4, the
// composition).

#pragma once

#include <algorithm>
#include <deque>
#include <stdexcept>
#include <vector>

#include "math/ball.hpp"
#include "math/bigint.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "math/qbeta.hpp"
#include "math/sturm.hpp"

namespace mathlib {

// ===================================================================
// Irreducibility / primitivity (Seneta 2006, Definitions 1.6 p.18 and
// 1.1 p.14 -- the same graph-theoretic definitions Mathlib's own
// `Matrix.IsIrreducible` / `Matrix.IsPrimitive` cite from the same
// book, per `Mathlib.LinearAlgebra.Matrix.Irreducible.Defs`).
// ===================================================================

// True iff the digraph i -> j (edge present iff M[i][j] > 0) is
// strongly connected, i.e. M is irreducible. Checked via two BFS
// passes from vertex 0: forward reachability (every vertex reachable
// FROM 0) and backward reachability (every vertex can reach 0) --
// together these are exactly strong connectivity of the WHOLE graph,
// not just a statement about vertex 0 specifically (a standard fact:
// if 0 reaches everyone and everyone reaches 0, then for any i, j,
// i -> 0 -> j gives a path i to j, and symmetrically j to i).
inline bool is_irreducible(const std::vector<std::vector<long long>>& M) {
    const std::size_t n = M.size();
    if (n == 0) return false;
    for (const auto& row : M) {
        if (row.size() != n) {
            throw std::invalid_argument("is_irreducible: matrix is not square");
        }
    }
    if (n == 1) return M[0][0] > 0;  // a single vertex needs a self-loop

    auto bfs_reach = [&](bool reversed) {
        std::vector<bool> seen(n, false);
        std::deque<std::size_t> q;
        seen[0] = true;
        q.push_back(0);
        std::size_t count = 1;
        while (!q.empty()) {
            std::size_t u = q.front();
            q.pop_front();
            for (std::size_t v = 0; v < n; ++v) {
                long long w = reversed ? M[v][u] : M[u][v];
                if (w > 0 && !seen[v]) {
                    seen[v] = true;
                    ++count;
                    q.push_back(v);
                }
            }
        }
        return count == n;
    };
    return bfs_reach(false) && bfs_reach(true);
}

// True iff M is primitive: nonnegative, and some power M^k is
// entrywise strictly positive (Seneta 2006, Def. 1.1, p.14). Checked
// via BOOLEAN matrix powers of the support matrix (M[i][j] > 0), up to
// Wielandt's bound n^2 - 2n + 2 on the smallest such k for a primitive
// matrix (Wielandt, H., "Unzerlegbare, nicht negative Matrizen",
// Math. Z. 52 (1950), 642-648) -- if no power up to that bound is
// fully positive, M is not primitive (this is the theorem's content,
// not a heuristic cutoff).
inline bool is_primitive(const std::vector<std::vector<long long>>& M) {
    const std::size_t n = M.size();
    if (n == 0) return false;
    for (const auto& row : M) {
        if (row.size() != n) {
            throw std::invalid_argument("is_primitive: matrix is not square");
        }
    }
    if (n == 1) return M[0][0] > 0;

    std::vector<std::vector<bool>> support(n, std::vector<bool>(n, false));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            support[i][j] = M[i][j] > 0;

    auto bool_matmul = [&](const std::vector<std::vector<bool>>& A,
                           const std::vector<std::vector<bool>>& B) {
        std::vector<std::vector<bool>> C(n, std::vector<bool>(n, false));
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t k = 0; k < n; ++k) {
                if (!A[i][k]) continue;
                for (std::size_t j = 0; j < n; ++j) {
                    if (B[k][j]) C[i][j] = true;
                }
            }
        }
        return C;
    };
    auto all_positive = [&](const std::vector<std::vector<bool>>& A) {
        for (const auto& row : A)
            for (bool b : row)
                if (!b) return false;
        return true;
    };

    const long long n_ll = static_cast<long long>(n);
    const long long bound = n_ll * n_ll - 2 * n_ll + 2;
    std::vector<std::vector<bool>> power = support;
    if (all_positive(power)) return true;
    for (long long k = 2; k <= bound; ++k) {
        power = bool_matmul(power, support);
        if (all_positive(power)) return true;
    }
    return false;
}

// ===================================================================
// Isolating the Perron root exactly
// ===================================================================
//
// Unlike `isolate_beta` (sturm.hpp), which assumes a Pisot-style
// beta > 1 and searches (1, cauchy_bound), this isolates the LARGEST
// real root of an arbitrary charpoly in [0, upper_bound) -- correct
// for any irreducible nonnegative integer matrix, where the Perron
// root is guaranteed positive (Seneta 2006, the existence half of the
// theorem cited above) but not a priori known to exceed 1 (e.g. the
// 2x2 permutation-like matrix [[0,1],[1,0]] has Perron root exactly 1).
//
// `upper_bound` should be a value known to exceed every eigenvalue in
// modulus; the maximum row sum is a standard, cheap, EXACT such bound
// for a nonnegative matrix (Horn & Johnson, Thm 8.1.22, "Perron root
// is bounded by the maximum row sum").
inline RootInterval isolate_perron_root(const PolyZ& charpoly, const Rat& upper_bound) {
    std::vector<PolyZ> chain = sturm_chain(charpoly);
    if (chain.empty()) {
        throw std::invalid_argument("isolate_perron_root: empty Sturm chain");
    }
    Rat low;
    set_si(low, 0, 1);
    Rat high = upper_bound;
    long long total = sturm_root_count_rat(chain, low, high);
    if (total <= 0) {
        throw std::runtime_error(
            "isolate_perron_root: no positive real root found below upper_bound "
            "(upper_bound too small, or matrix has no positive eigenvalue -- "
            "should be impossible for an irreducible nonnegative matrix)");
    }
    // Narrow (low, high) down to contain exactly the TOPMOST root: if
    // (mid, high) still contains a root, the top root is above mid, so
    // raise low to mid; otherwise the top root is at or below mid, so
    // lower high to mid. This targets the LARGEST root specifically,
    // not merely "a" root.
    for (int iter = 0; iter < 200 && sturm_root_count_rat(chain, low, high) > 1; ++iter) {
        Rat sum;
        add(sum, low, high);
        Rat two;
        set_si(two, 2, 1);
        Rat mid;
        div(mid, sum, two);
        if (cmp(mid, low) == 0 || cmp(mid, high) == 0) break;  // out of precision
        if (sturm_root_count_rat(chain, mid, high) >= 1) {
            low = mid;
        } else {
            high = mid;
        }
    }
    return isolate_real_root_rat(chain, low, high);
}

// ===================================================================
// The full certificate
// ===================================================================

struct PerronFrobeniusCertificate {
    bool irreducible = false;
    RootInterval beta_interval;   // isolating interval for the Perron root
    QBetaVec eigenvector;          // the right Perron eigenvector, in Q(beta)
    bool eigen_equation_holds = false;  // M v == beta v, verified exactly
    bool eigenvector_all_positive = false;  // every coordinate's sign, via qbeta_sign
    bool ok = false;  // true iff every check above passed
};

// Runs the full certificate on M. Does NOT throw for a reducible or
// otherwise ill-posed matrix -- returns a certificate with `ok = false`
// and the specific flags set to show which check failed, since a
// negative result (this matrix is NOT covered by the theorem, or the
// computation found a contradiction) is exactly as informative as a
// positive one for a caller auditing a specific matrix.
inline PerronFrobeniusCertificate certify_perron_frobenius(
    const std::vector<std::vector<long long>>& M) {
    PerronFrobeniusCertificate cert;
    const std::size_t n = M.size();
    if (n == 0) return cert;
    for (const auto& row : M) {
        if (row.size() != n) {
            throw std::invalid_argument("certify_perron_frobenius: matrix is not square");
        }
        for (long long x : row) {
            if (x < 0) {
                throw std::invalid_argument(
                    "certify_perron_frobenius: matrix is not entrywise nonnegative");
            }
        }
    }

    cert.irreducible = is_irreducible(M);
    if (!cert.irreducible) return cert;

    PolyZ cp = charpoly_faddeev_leverrier(M);

    long long max_row_sum = 0;
    for (const auto& row : M) {
        long long s = 0;
        for (long long x : row) s += x;
        max_row_sum = std::max(max_row_sum, s);
    }
    Rat upper_bound;
    set_si(upper_bound, max_row_sum + 1, 1);

    cert.beta_interval = isolate_perron_root(cp, upper_bound);

    QBetaRing R(cp);
    EigenvectorResult eig = right_eigenvector_via_qbeta(M, R);
    if (!eig.ok) return cert;
    cert.eigenvector = eig.v;

    cert.eigen_equation_holds = verify_right_eigenvector(cert.eigenvector, M, R);

    cert.eigenvector_all_positive = true;
    for (const auto& coord : cert.eigenvector) {
        if (qbeta_sign(coord, R, cert.beta_interval) <= 0) {
            cert.eigenvector_all_positive = false;
            break;
        }
    }

    cert.ok = cert.irreducible && cert.eigen_equation_holds && cert.eigenvector_all_positive;
    return cert;
}

}  // namespace mathlib
