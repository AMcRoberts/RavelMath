// spectral.hpp
//
// Spectral invariants for an integer Pisot substitution matrix. Returns
// the Perron root, next-largest-modulus conjugate, determinant, invol
// safety, and the n-1 inequality bound.
// Header-only; Lua bindings in lua_bindings.cpp convert the result
// into a Lua table.

#pragma once

#include <cmath>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace ravel {

struct SpectralInvariants {
    std::size_t n = 0;
    double beta = 0.0;
    double beta_abs = 0.0;
    double beta2 = 0.0;
    long long det_M = 0;
    long long abs_det = 0;
    bool invol_safe = false;
    double bound_rhs = 0.0;
    bool bound_holds = false;
};

inline SpectralInvariants spectral_invariants_2x2(long long a11, long long a12,
                                                  long long a21, long long a22) {
    SpectralInvariants inv;
    inv.n = 2;
    double tr = static_cast<double>(a11) + static_cast<double>(a22);
    double det = static_cast<double>(a11) * static_cast<double>(a22)
               - static_cast<double>(a12) * static_cast<double>(a21);
    double disc = tr * tr - 4.0 * det;
    if (disc < 0.0) {
        inv.beta = std::abs(tr) * 0.5;
        inv.beta_abs = inv.beta;
        inv.beta2 = std::sqrt(-disc) * 0.5;
        inv.det_M = static_cast<long long>(std::llround(det));
        inv.abs_det = std::abs(inv.det_M);
        inv.invol_safe = inv.beta2 < 1.0;
        inv.bound_rhs = std::pow(inv.beta_abs, -1.0);
        inv.bound_holds = inv.beta2 >= inv.bound_rhs - 1e-12;
        return inv;
    }
    double sqd = std::sqrt(disc);
    double l1 = (tr + sqd) * 0.5;
    double l2 = (tr - sqd) * 0.5;
    if (std::abs(l1) > std::abs(l2)) {
        inv.beta = l1;
        inv.beta2 = std::abs(l2);
    } else {
        inv.beta = l2;
        inv.beta2 = std::abs(l1);
    }
    inv.beta_abs = std::abs(inv.beta);
    inv.beta2 = std::abs(inv.beta2);
    inv.det_M = static_cast<long long>(std::llround(det));
    inv.abs_det = std::abs(inv.det_M);
    inv.invol_safe = inv.beta2 < 1.0;
    inv.bound_rhs = std::pow(inv.beta_abs, -1.0 / static_cast<double>(inv.n - 1));
    inv.bound_holds = inv.beta2 >= inv.bound_rhs - 1e-12;
    return inv;
}

inline SpectralInvariants spectral_invariants_3x3(
        long long a11, long long a12, long long a13,
        long long a21, long long a22, long long a23,
        long long a31, long long a32, long long a33) {
    SpectralInvariants inv;
    inv.n = 3;
    const double m[3][3] = {
        {static_cast<double>(a11), static_cast<double>(a12), static_cast<double>(a13)},
        {static_cast<double>(a21), static_cast<double>(a22), static_cast<double>(a23)},
        {static_cast<double>(a31), static_cast<double>(a32), static_cast<double>(a33)},
    };
    double tr = m[0][0] + m[1][1] + m[2][2];
    double cf = m[0][0]*m[1][1] + m[0][0]*m[2][2] + m[1][1]*m[2][2]
              - m[0][1]*m[1][0] - m[0][2]*m[2][0] - m[1][2]*m[2][1];
    double det = m[0][0]*(m[1][1]*m[2][2] - m[1][2]*m[2][1])
               - m[0][1]*(m[1][0]*m[2][2] - m[1][2]*m[2][0])
               + m[0][2]*(m[1][0]*m[2][1] - m[1][1]*m[2][0]);
double p = (3.0 * cf - tr * tr) / 3.0;
    double q = (9.0 * tr * cf - 2.0 * tr * tr * tr - 27.0 * det) / 27.0;
    // Cardano discriminant: disc = q²/4 + p³/27.
    //   disc > 0 : casus irreducibilis, one real root + complex
    //              conjugate pair (the Pisot case).
    //   disc = 0 : multiple real roots.
    //   disc < 0 : three distinct real roots.
    double disc = q * q / 4.0 + p * p * p / 27.0;
    double tr3 = tr / 3.0;
    double roots[3] = {0.0, 0.0, 0.0};
    if (disc > 0) {
        // Pisot case: 1 real root + 1 complex conjugate pair.
        // We can't store complex values in a double[3], so report
        // the real root and the modulus of the complex pair (the
        // caller can derive |det|/β for unimodular Pisot if it
        // needs the modulus).
        double sqrt_disc = std::sqrt(disc);
        double u = std::cbrt(-q / 2.0 + sqrt_disc);
        double v = std::cbrt(-q / 2.0 - sqrt_disc);
        double r1 = u + v + tr3;
        double re_part = -(u + v) * 0.5 + tr3;
        double im_part = (std::sqrt(3.0) * 0.5) * (u - v);
        double mod = std::sqrt(re_part * re_part + im_part * im_part);
        roots[0] = r1;
        roots[1] = mod;
        roots[2] = mod;
    } else if (std::abs(disc) < 1e-15) {
        // Multiple real roots.
        double u = std::cbrt(-q / 2.0);
        roots[0] = 2.0 * u + tr3;
        roots[1] = -u + tr3;
        roots[2] = -u + tr3;
    } else {
        double r = std::sqrt(-p * p * p / 27.0);
        double phi = std::acos(std::clamp(-q / (2.0 * r), -1.0, 1.0));
        double m_cbrt = std::cbrt(r);
        roots[0] = 2.0 * m_cbrt * std::cos(phi / 3.0) + tr3;
        roots[1] = 2.0 * m_cbrt * std::cos((phi + 2.0 * M_PI) / 3.0) + tr3;
        roots[2] = 2.0 * m_cbrt * std::cos((phi + 4.0 * M_PI) / 3.0) + tr3;
    }
    int perron_idx = 0;
    for (int i = 1; i < 3; ++i) {
        if (std::abs(roots[i]) > std::abs(roots[perron_idx])) perron_idx = i;
    }
    inv.beta = roots[perron_idx];
    // FIXED (see docs/RESEARCH_STATUS.md and the conversation this came out
    // of): this used to unconditionally overwrite beta2 with
    // sqrt(|det|/|beta|) regardless of which branch above computed
    // roots[]. That formula is ONLY valid when the two secondary
    // roots are a genuine complex-conjugate pair (product = modulus^2
    // exactly there); for the two-distinct-real-roots case it
    // computes their GEOMETRIC MEAN, not their maximum -- silently
    // UNDERESTIMATING beta2 whenever the two real secondary roots
    // have very different magnitudes (e.g. roots 1.28 and 0.34 give
    // a geometric mean of 0.66, wrongly reading as "Pisot" even
    // though the true secondary modulus 1.28 is NOT less than 1).
    // Confirmed via direct root computation: every one of a batch of
    // "random Pisot" 3x3 matrices that later threw
    // "secondary root modulus >= 1" inside check_property_f (wrongly
    // blamed on root-finder precision at the time) turned out to
    // have a genuine secondary root with modulus 1.15-2.55 -- not a
    // precision issue at all, but this exact misclassification bug
    // letting non-Pisot matrices through wide_random_pisot_survey's
    // own filter. The correct value, in every branch (complex pair OR
    // two distinct/multiple real roots), is simply the largest
    // modulus among the two NON-dominant entries of roots[] --
    // which the branches above already computed correctly; the bug
    // was solely in discarding that and recomputing via a
    // narrower-than-it-looked formula afterward.
    double secondary_max = 0.0;
    for (int i = 0; i < 3; ++i) {
        if (i == perron_idx) continue;
        secondary_max = std::max(secondary_max, std::abs(roots[i]));
    }
    inv.beta2 = secondary_max;
    inv.beta_abs = std::abs(inv.beta);
    inv.det_M = static_cast<long long>(std::llround(det));
    inv.abs_det = std::abs(inv.det_M);
    inv.invol_safe = inv.beta2 < 1.0;
    inv.bound_rhs = std::pow(inv.beta_abs, -1.0 / static_cast<double>(inv.n - 1));
    inv.bound_holds = inv.beta2 >= inv.bound_rhs - 1e-12;
    return inv;
}

// General n x n spectral invariants, for n >= 4 (n == 2, 3 keep
// using the closed-form solvers above; classify_matrix_spectral in
// survey.hpp decides which path to call).
//
// Previously, any matrix with n != 2, 3 made
// classify_matrix_spectral silently return beta=0/pisot=false —
// see WORK_ITEMS.md W13 and TODO_GENERALIZATION.md. This closes
// that gap with plain numerical linear algebra:
//
//   * beta (dominant eigenvalue): power iteration on M, refined by
//     a Rayleigh quotient. Valid for any primitive nonnegative
//     integer matrix by Perron-Frobenius (beta is real, simple,
//     and strictly the largest in modulus).
//   * beta2 (modulus of the next-largest eigenvalue): Wielandt
//     deflation (M' = M - beta * v u^T, with u, v the right/left
//     Perron eigenvectors normalized so u.v = 1) followed by power
//     iteration on M'. The norm-growth-rate trick used here
//     converges to |lambda_2| whether the second eigenvalue is
//     real or one of a complex-conjugate pair -- the direction can
//     keep rotating in the complex-pair case, but the vector norm
//     still grows at rate |lambda_2| each step once the iterate has
//     settled into that invariant subspace.
//   * det: plain Gaussian elimination with partial pivoting on a
//     double copy, rounded to the nearest integer (fine for the
//     matrix sizes and entry magnitudes this project uses; if that
//     ever stops holding, compute det from the constant term of
//     charpoly_int in barge.hpp instead, which is exact).
//
// This is double-precision, same tradeoff already made throughout
// core.hpp/d_cont_check.hpp for this project (see their own
// "FREE alternative" notes) -- not exact Q(beta) arithmetic. For
// cases that need exact arithmetic at larger boundary-graph sizes,
// see qbeta_eigenvalue.cpp instead.
inline double determinant_general(std::vector<std::vector<double>> A) {
    const std::size_t n = A.size();
    double det = 1.0;
    for (std::size_t col = 0; col < n; ++col) {
        std::size_t piv = col;
        for (std::size_t r = col + 1; r < n; ++r)
            if (std::abs(A[r][col]) > std::abs(A[piv][col])) piv = r;
        if (std::abs(A[piv][col]) < 1e-300) return 0.0;
        if (piv != col) { std::swap(A[piv], A[col]); det = -det; }
        det *= A[col][col];
        for (std::size_t r = col + 1; r < n; ++r) {
            double f = A[r][col] / A[col][col];
            for (std::size_t c = col; c < n; ++c) A[r][c] -= f * A[col][c];
        }
    }
    return det;
}

inline SpectralInvariants spectral_invariants_general(
        const std::vector<std::vector<long long>>& Min) {
    const std::size_t n = Min.size();
    SpectralInvariants inv;
    inv.n = n;
    if (n == 0) return inv;

    std::vector<std::vector<double>> M(n, std::vector<double>(n));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            M[i][j] = static_cast<double>(Min[i][j]);

    inv.det_M = static_cast<long long>(std::llround(determinant_general(M)));
    inv.abs_det = std::abs(inv.det_M);

    auto matvec = [&](const std::vector<std::vector<double>>& A,
                       const std::vector<double>& x, bool transpose) {
        std::vector<double> y(n, 0.0);
        for (std::size_t i = 0; i < n; ++i) {
            double s = 0.0;
            for (std::size_t j = 0; j < n; ++j)
                s += (transpose ? A[j][i] : A[i][j]) * x[j];
            y[i] = s;
        }
        return y;
    };
    auto normalize = [&](std::vector<double>& x) {
        double norm = 0.0;
        for (double c : x) norm += c * c;
        norm = std::sqrt(norm);
        if (norm > 1e-300) for (double& c : x) c /= norm;
        return norm;
    };

    // Right Perron eigenvector v and beta via power iteration.
    std::vector<double> v(n, 1.0);
    normalize(v);
    for (int it = 0; it < 20000; ++it) {
        std::vector<double> w = matvec(M, v, false);
        if (normalize(w) < 1e-300) break;
        v = w;
    }
    {
        std::vector<double> Mv = matvec(M, v, false);
        double num = 0.0, den = 0.0;
        for (std::size_t i = 0; i < n; ++i) { num += v[i] * Mv[i]; den += v[i] * v[i]; }
        inv.beta = den > 0 ? num / den : 0.0;
    }
    inv.beta_abs = std::abs(inv.beta);

    // Left Perron eigenvector u via power iteration on M^T.
    std::vector<double> u(n, 1.0);
    normalize(u);
    for (int it = 0; it < 20000; ++it) {
        std::vector<double> w = matvec(M, u, true);
        if (normalize(w) < 1e-300) break;
        u = w;
    }
    double uv = 0.0;
    for (std::size_t i = 0; i < n; ++i) uv += u[i] * v[i];
    if (std::abs(uv) > 1e-300) for (std::size_t i = 0; i < n; ++i) u[i] /= uv;

    // Wielandt deflation: M' = M - beta * v u^T has the same
    // spectrum as M except beta -> 0, so power iteration on M'
    // converges to (the modulus of) the next-largest eigenvalue.
    std::vector<std::vector<double>> Mp(n, std::vector<double>(n));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            Mp[i][j] = M[i][j] - inv.beta * v[i] * u[j];

    std::vector<double> x(n, 1.0);
    normalize(x);
    double lambda2 = 0.0;
    for (int it = 0; it < 20000; ++it) {
        std::vector<double> w = matvec(Mp, x, false);
        double norm = normalize(w);
        if (norm < 1e-300) { lambda2 = 0.0; break; }
        if (it > 200) lambda2 = norm;  // once settled, ||M'x|| / ||x||=1 -> |lambda2|
        x = w;
    }
    inv.beta2 = lambda2;

    inv.invol_safe = inv.beta2 < 1.0;
    inv.bound_rhs = n > 1 ? std::pow(inv.beta_abs, -1.0 / static_cast<double>(n - 1)) : 0.0;
    inv.bound_holds = inv.beta2 >= inv.bound_rhs - 1e-9;
    return inv;
}

}  // namespace ravel
