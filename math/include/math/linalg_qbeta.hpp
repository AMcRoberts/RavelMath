// math/linalg_qbeta.hpp
//
// Linear algebra over Q(β): determinant, linear solver (Gaussian
// elimination with partial pivoting), Cramer's rule, and the right
// eigenvector of a matrix M (M v = λ v for a Pisot eigenvalue λ).
//
// All arithmetic is in the mathlib::Q(β) ring (qpbeta::QBetaRing),
// which is exact.  The eigenvectors are returned as QElem vectors.

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/poly_q.hpp"
#include "math/qbeta.hpp"
#include "math/bezout.hpp"

namespace mathlib {

// A matrix over Q(β).
using QBetaMat = std::vector<std::vector<QElem>>;
using QBetaVec = std::vector<QElem>;

// ===================================================================
// Linear solver: A x = b over Q(β) (Gaussian elimination with partial
// pivoting, since Q(β) is a field)
// ===================================================================
//
// Returns the solution x.  Throws if A is singular.

inline QBetaVec solve_linear(const QBetaMat& A, const QBetaVec& b, const QBetaRing& R) {
    const std::size_t n = A.size();
    if (n == 0) return QBetaVec{};
    if (b.size() != n) {
        throw std::invalid_argument("solve_linear: A and b size mismatch");
    }
    // Augment A with b.
    QBetaMat aug(n, QBetaVec(n + 1));
    for (std::size_t i = 0; i < n; ++i) {
        if (A[i].size() != n) {
            throw std::invalid_argument("solve_linear: A is not square");
        }
        for (std::size_t j = 0; j < n; ++j) aug[i][j] = A[i][j];
        aug[i][n] = b[i];
    }
    // Forward elimination with partial pivoting.
    for (std::size_t col = 0; col < n; ++col) {
        // Find the first non-zero row in [col, n).
        std::size_t pivot = col;
        bool found = false;
        for (std::size_t row = col; row < n; ++row) {
            if (!aug[row][col].is_zero()) { pivot = row; found = true; break; }
        }
        if (!found) {
            throw std::runtime_error("solve_linear: A is singular");
        }
        if (pivot != col) {
            std::swap(aug[pivot], aug[col]);
        }
        // Eliminate below.
        for (std::size_t row = col + 1; row < n; ++row) {
            if (aug[row][col].is_zero()) continue;
            QBetaInverseResult ir = invert_in_qbeta(aug[col][col], R);
            if (!ir.invertible) {
                throw std::runtime_error("solve_linear: pivot not invertible");
            }
            QElem factor = R.mul(aug[row][col], ir.inverse);
            for (std::size_t k = col; k <= n; ++k) {
                QElem prod = R.mul(factor, aug[col][k]);
                aug[row][k] = R.sub(aug[row][k], prod);
            }
        }
    }
    // Back-substitute.
    QBetaVec x(n);
    for (long long row = static_cast<long long>(n) - 1; row >= 0; --row) {
        QElem s = aug[row][n];
        for (std::size_t k = static_cast<std::size_t>(row) + 1; k < n; ++k) {
            QElem prod = R.mul(aug[row][k], x[k]);
            s = R.sub(s, prod);
        }
        QBetaInverseResult ir = invert_in_qbeta(aug[row][row], R);
        if (!ir.invertible) {
            throw std::runtime_error("solve_linear: back-sub pivot not invertible");
        }
        x[row] = R.mul(s, ir.inverse);
    }
    return x;
}

// ===================================================================
// Determinant over Q(β)
// ===================================================================

inline QElem determinant(const QBetaMat& A, const QBetaRing& R) {
    const std::size_t n = A.size();
    if (n == 0) return R.from_int(1);
    QBetaMat aug(n, QBetaVec(n));
    for (std::size_t i = 0; i < n; ++i) {
        if (A[i].size() != n) {
            throw std::invalid_argument("determinant: A is not square");
        }
        for (std::size_t j = 0; j < n; ++j) aug[i][j] = A[i][j];
    }
    QElem det = R.from_int(1);
    for (std::size_t col = 0; col < n; ++col) {
        std::size_t pivot = col;
        bool found = false;
        for (std::size_t row = col; row < n; ++row) {
            if (!aug[row][col].is_zero()) { pivot = row; found = true; break; }
        }
        if (!found) return R.from_int(0);
        if (pivot != col) {
            std::swap(aug[pivot], aug[col]);
            det = R.neg(det);  // row swap flips sign
        }
        det = R.mul(det, aug[col][col]);
        // Eliminate below
        for (std::size_t row = col + 1; row < n; ++row) {
            if (aug[row][col].is_zero()) continue;
            QBetaInverseResult ir = invert_in_qbeta(aug[col][col], R);
            if (!ir.invertible) return R.from_int(0);
            QElem factor = R.mul(aug[row][col], ir.inverse);
            for (std::size_t k = col; k < n; ++k) {
                QElem prod = R.mul(factor, aug[col][k]);
                aug[row][k] = R.sub(aug[row][k], prod);
            }
        }
    }
    return det;
}

// ===================================================================
// Right eigenvector via Cramer's rule
// ===================================================================
//
// For a d×d integer matrix M with dominant eigenvalue β (root of
// charpoly), the right eigenvector v ∈ Q(β)^d satisfies M v = β v.
// We can drop the last equation and the last unknown (v[d-1] = 1) to
// get a (d-1)×(d-1) system:
//
//   For i in [0, d-2): sum_{j=0}^{d-2} M[i,j] v[j] + M[i, d-1] v[d-1] = β v[i]
//   => sum_{j=0}^{d-2} (M[i,j] - β δ_{ij}) v[j] = -M[i, d-1] v[d-1]
//
// Setting v[d-1] = 1, the RHS is -M[i, d-1].  We solve the (d-1)×(d-1)
// system for v[0..d-2] over Q(β), and the eigenvector is v with v[d-1] = 1.

struct EigenvectorResult {
    bool ok;       // true if eigenvector was computed
    QBetaVec v;    // the eigenvector, v[d-1] = 1
};

inline EigenvectorResult right_eigenvector_via_qbeta(
    const std::vector<std::vector<long long>>& M_int,
    const QBetaRing& R) {
    const std::size_t d = R.degree();
    if (M_int.size() != d) {
        throw std::invalid_argument("right_eigenvector_via_qbeta: M size mismatch");
    }
    for (const auto& row : M_int) {
        if (row.size() != d) {
            throw std::invalid_argument("right_eigenvector_via_qbeta: M not square");
        }
    }
    if (d == 1) {
        // 1×1 case: M = [m], β = m, eigenvector is [1].
        QBetaVec v(1);
        v[0] = R.from_int(1);
        return EigenvectorResult{true, v};
    }
    // Build the (d-1)×(d-1) cofactor of (M - β I): A[i][j] = M[i][j] - β δ_{ij}.
    // The RHS is b[i] = -M[i][d-1].
    QBetaMat A(d - 1, QBetaVec(d - 1));
    QBetaVec b(d - 1);
    QElem b_elem = R.beta_k(1);
    for (std::size_t i = 0; i + 1 < d; ++i) {
        for (std::size_t j = 0; j + 1 < d; ++j) {
            QElem mij = R.from_int(M_int[i][j]);
            if (i == j) {
                A[i][j] = R.sub(mij, b_elem);
            } else {
                A[i][j] = mij;
            }
        }
        b[i] = R.from_int(-M_int[i][d - 1]);
    }
    try {
        QBetaVec v = solve_linear(A, b, R);
        v.push_back(R.from_int(1));
        return EigenvectorResult{true, v};
    } catch (...) {
        return EigenvectorResult{false, QBetaVec{}};
    }
}

// ===================================================================
// Left eigenvector via Cramer's rule -- a DEDICATED computation, not
// the right eigenvector's result reinterpreted or transposed.
// ===================================================================
//
// `docs/RESEARCH_STATUS.md` documents a real
// bug that came from conflating these: `core.hpp::ensure_exact_qbeta`
// called `right_eigenvector_via_qbeta(M, R)` directly where the
// height-function test `in_H_sigma` (per the paper's definition of
// H_sigma, `0 <= <x,v> < v[j]`) needs the LEFT Perron eigenvector of
// M instead -- a genuinely different vector in general (not merely a
// relabeling or transpose of the right eigenvector's own entries; for
// sigma_1's M = [[3,2,1],[1,0,0],[0,1,0]] the right eigenvector is
// proportional to (beta^2, beta, 1) while the left one is proportional
// to (1, 1/beta, 1/beta^2)-ish values that are NOT a permutation or
// transpose of the right eigenvector's coordinates).
//
// For a d x d integer matrix M with dominant eigenvalue beta, the left
// eigenvector v satisfies v^T M = beta v^T, equivalently (as a column
// vector) M^T v = beta v.  Rather than building M^T explicitly and
// delegating to right_eigenvector_via_qbeta (which would work, but
// hides the derivation behind an unstated transpose the next reader
// has to already know to trust), this solves the transposed system
// directly: for row i of M^T, M^T[i][j] = M[j][i], so the analogous
// (d-1) x (d-1) Cramer system is
//
//   A[i][j] = M[j][i] - beta * delta_{ij}   (note the SWAPPED indices
//                                             on M, versus M[i][j] in
//                                             the right-eigenvector
//                                             version above)
//   b[i]    = -M[d-1][i]                    (swapped versus -M[i][d-1])
//
// for i, j in [0, d-2], solved for v[0..d-2] with v[d-1] = 1 fixed --
// the same normalization convention right_eigenvector_via_qbeta uses,
// so the two are interchangeable wherever a caller's normalization
// doesn't matter (which is everywhere in this codebase, since the
// in_H_sigma-style predicates are invariant under positive rescaling
// of v, and the Perron eigenvector of a primitive nonnegative matrix
// is positive in every coordinate by Perron-Frobenius).
inline EigenvectorResult left_eigenvector_via_qbeta(
    const std::vector<std::vector<long long>>& M_int,
    const QBetaRing& R) {
    const std::size_t d = R.degree();
    if (M_int.size() != d) {
        throw std::invalid_argument("left_eigenvector_via_qbeta: M size mismatch");
    }
    for (const auto& row : M_int) {
        if (row.size() != d) {
            throw std::invalid_argument("left_eigenvector_via_qbeta: M not square");
        }
    }
    if (d == 1) {
        // 1x1 case: M = [m], beta = m, left eigenvector is [1] (same
        // as the right eigenvector -- every 1x1 matrix is symmetric).
        QBetaVec v(1);
        v[0] = R.from_int(1);
        return EigenvectorResult{true, v};
    }
    // Build the (d-1)x(d-1) cofactor of (M^T - beta I), reading M with
    // SWAPPED indices rather than physically transposing it:
    // A[i][j] = M[j][i] - beta * delta_{ij}, b[i] = -M[d-1][i].
    QBetaMat A(d - 1, QBetaVec(d - 1));
    QBetaVec b(d - 1);
    QElem b_elem = R.beta_k(1);
    for (std::size_t i = 0; i + 1 < d; ++i) {
        for (std::size_t j = 0; j + 1 < d; ++j) {
            QElem mji = R.from_int(M_int[j][i]);  // NOTE: M[j][i], not M[i][j]
            if (i == j) {
                A[i][j] = R.sub(mji, b_elem);
            } else {
                A[i][j] = mji;
            }
        }
        b[i] = R.from_int(-M_int[d - 1][i]);  // NOTE: M[d-1][i], not M[i][d-1]
    }
    try {
        QBetaVec v = solve_linear(A, b, R);
        v.push_back(R.from_int(1));
        return EigenvectorResult{true, v};
    } catch (...) {
        return EigenvectorResult{false, QBetaVec{}};
    }
}

// Compute a left Perron eigenvector when the incidence matrix has a
// cyclotomic (or otherwise neutral) factor and R is the smaller minimal
// Pisot field.  The ordinary helper above intentionally requires
// degree(M)==degree(R); this operation instead solves a nonsingular
// (d-1)-minor of M^T-beta I over R and verifies every remaining row.
// Trying all choices of the normalized coordinate and omitted equation is
// cheap at the dimensions used by the canonical beta-substitution probes,
// and avoids silently choosing a singular cofactor in a cyclotomic lift.
inline EigenvectorResult left_eigenvector_via_qbeta_reduced_factor(
    const std::vector<std::vector<long long>>& M_int,
    const QBetaRing& R) {
    const std::size_t d = M_int.size();
    if (d == 0) return EigenvectorResult{false, QBetaVec{}};
    if (R.degree() >= d) {
        return left_eigenvector_via_qbeta(M_int, R);
    }
    for (const auto& row : M_int) {
        if (row.size() != d) return EigenvectorResult{false, QBetaVec{}};
    }
    const QElem beta = R.beta_k(1);
    for (std::size_t free_col = 0; free_col < d; ++free_col) {
        for (std::size_t omitted_row = 0; omitted_row < d; ++omitted_row) {
            QBetaMat A(d - 1, QBetaVec(d - 1));
            QBetaVec b(d - 1);
            std::size_t ar = 0;
            for (std::size_t row = 0; row < d; ++row) {
                if (row == omitted_row) continue;
                std::size_t ac = 0;
                for (std::size_t col = 0; col < d; ++col) {
                    if (col == free_col) continue;
                    QElem entry = R.from_int(M_int[col][row]);
                    if (row == col) entry = R.sub(entry, beta);
                    A[ar][ac++] = entry;
                }
                b[ar++] = R.from_int(-M_int[free_col][row]);
            }
            try {
                const QBetaVec unknown = solve_linear(A, b, R);
                QBetaVec v(d, R.from_int(0));
                v[free_col] = R.from_int(1);
                std::size_t ac = 0;
                for (std::size_t col = 0; col < d; ++col)
                    if (col != free_col) v[col] = unknown[ac++];

                bool valid = true;
                for (std::size_t row = 0; row < d && valid; ++row) {
                    QElem residual = R.from_int(0);
                    for (std::size_t col = 0; col < d; ++col) {
                        residual = R.add(residual,
                            R.mul(R.from_int(M_int[col][row]), v[col]));
                    }
                    residual = R.sub(residual, R.mul(beta, v[row]));
                    valid = residual.is_zero();
                }
                if (valid) return EigenvectorResult{true, std::move(v)};
            } catch (...) {
                // This minor is singular or has a non-invertible pivot;
                // try another coordinate/equation pair.
            }
        }
    }
    return EigenvectorResult{false, QBetaVec{}};
}

// ===================================================================
// Self-verification: the prevention strategy for eigenvector-sidedness
// bugs.
// ===================================================================
//
// Two real bugs (docs/RESEARCH_STATUS.md) came from the
// same root cause: `right_eigenvector_via_qbeta` and
// `left_eigenvector_via_qbeta` both return the SAME type
// (EigenvectorResult, just a QBetaVec) -- there is nothing in the
// type system stopping a caller who needs a left eigenvector from
// accidentally calling the right-eigenvector function (or vice
// versa), or from receiving a correctly-computed vector and passing
// it to a function that silently assumes the other sidedness.  A
// full fix would give the two a distinct C++ type (e.g. a phantom-
// tagged `SidedEigenvector<Side>`) so a mismatch is a compile error;
// that is a larger, more invasive refactor across every existing
// caller (`core.hpp`, `prefix_automaton.hpp`, several apps and
// tests) and was deliberately NOT done -- see
// `docs/RESEARCH_STATUS.md`'s "Prevention strategy" section
// for why, and for it as a recommended follow-up.
//
// What WAS done: a cheap, exact, always-on RUNTIME check that any
// caller can use to confirm which sidedness a vector actually has,
// before trusting it for anything.  This is strictly weaker than a
// compile-time guarantee, but it converts "silently compute a wrong
// answer that only shows up as a confusing downstream numeric
// mismatch weeks later" into "throw immediately, at the exact call
// site, with an unambiguous message" -- which is what actually
// happened when this was retrofitted onto `core.hpp::
// ensure_exact_qbeta()`, the historical bug site: if that function
// is ever "fixed" back to `right_eigenvector_via_qbeta` by a future
// edit (accidental revert, merge conflict, someone pattern-matching
// off the misleadingly-named `compute_right_eigenvector` in the loose
// path above it), the very next call throws instead of quietly
// corrupting every `in_H_sigma_exact` result downstream.

// True iff v satisfies M v = beta v (a genuine RIGHT eigenvector of M).
inline bool verify_right_eigenvector(
    const QBetaVec& v,
    const std::vector<std::vector<long long>>& M_int,
    const QBetaRing& R) {
    const std::size_t d = M_int.size();
    if (v.size() != d) return false;
    QElem beta = R.beta_k(1);
    for (std::size_t i = 0; i < d; ++i) {
        QElem Mv_i = R.from_int(0);
        for (std::size_t j = 0; j < d; ++j) {
            Mv_i = R.add(Mv_i, R.mul(R.from_int(M_int[i][j]), v[j]));
        }
        QElem beta_vi = R.mul(beta, v[i]);
        if (!(Mv_i == beta_vi)) return false;
    }
    return true;
}

// True iff v satisfies v^T M = beta v^T (a genuine LEFT eigenvector
// of M) -- equivalently, for each column j: sum_i v[i]*M[i][j] ==
// beta*v[j].  NOT the same check as verify_right_eigenvector with the
// loop indices swapped by accident -- this genuinely sums over the
// OTHER index (column-wise, matching v^T M), which is the whole point.
inline bool verify_left_eigenvector(
    const QBetaVec& v,
    const std::vector<std::vector<long long>>& M_int,
    const QBetaRing& R) {
    const std::size_t d = M_int.size();
    if (v.size() != d) return false;
    QElem beta = R.beta_k(1);
    for (std::size_t j = 0; j < d; ++j) {
        QElem vM_j = R.from_int(0);
        for (std::size_t i = 0; i < d; ++i) {
            vM_j = R.add(vM_j, R.mul(v[i], R.from_int(M_int[i][j])));
        }
        QElem beta_vj = R.mul(beta, v[j]);
        if (!(vM_j == beta_vj)) return false;
    }
    return true;
}

}  // namespace mathlib
