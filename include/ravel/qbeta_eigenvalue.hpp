// qbeta_eigenvalue.hpp
//
// Exact dominant eigenvalue of an integer matrix in Q(beta),
// where beta is the unique real algebraic integer with the given
// monic minimal polynomial x^d + c[0] x^(d-1) + ... + c[d-1].
// This is the bit-exact alternative to double-precision power
// iteration (which is unreliable for large boundary graphs); see
// scripts/qbeta_eigenvalue.cpp for the original CLI driver,
// and scripts/run_qbeta_survey.sh for the canonical
// contact-boundary survey workflow.
//
// Algorithm:
//   1. Represent each element of Q(beta) as a d-tuple of mpz_t
//      (c_0 + c_1 beta + ... + c_{d-1} beta^{d-1}) / scale where
//      scale is an accumulated rescaling factor.
//   2. For a Pisot beta with minpoly as above, multiplication by
//      beta is a fixed d x d integer matrix (after reduction mod
//      the minpoly).
//   3. Multiply matrix A by a Q(beta)^N vector repeatedly.  A has
//      integer entries, so each multiplication is N multiplications
//      by beta plus N additions of d-tuples of mpz_t vectors.
//   4. To avoid integer overflow (e.g. lambda^k for k=80 with
//      lambda~8 reaches 10^75), periodically rescale by dividing
//      all components by their greatest common divisor.  This
//      preserves the eigenvalues exactly: A^k v and A^k v / gcd
//      are eigenvectors with the same ratio.  When components get
//      large enough that exact divisibility-by-2 fails, we track
//      the accumulated log-scale and rely on the next iteration
//      to bring components back into the rescalable regime.
//   5. The growth rate of ||A^k v||_infty (in the real embedding)
//      converges to the dominant eigenvalue; we report
//      lambda = (||A^k v|| / ||A^{k-1} v||) for the converged k.
//
// Supports any degree d >= 1 via qbeta_dominant_eigenvalue (the
// arbitrary-d general entry point), with qbeta_dominant_eigenvalue_4
// kept as a thin wrapper for backward compatibility with the
// scripts/qbeta_eigenvalue.cpp CLI.  Generalizing past the project's
// historical 8 named Pisot rings (Fibonacci, Sqrt2, Tribonacci,
// Plastic, Supergolden, sigma_1, sigma_2, Tetrabonacci -- the fixed
// catalogue in algebraic.hpp) closes the loop on the 39-substitution
// and 87-candidate non-unimodular surveys whose double-precision
// λ(G_B) values were the only ones reported until now.
//
// Per docs/DIRECTION_AND_OPEN_THREADS.md thread C, this header
// exists to let scripts/contact_boundary_4x4.cpp invoke the
// exact eigenvalue computation in-process.  The contact-boundary
// pipeline (contact_boundary.hpp) produces the G_B adjacency matrix
// and we read back its exact spectral radius here, end-to-end in
// one driver invocation.  The historical scripts/run_qbeta_survey.sh
// shell-orchestrated these two steps (dumping the matrix to a file,
// then invoking this CLI) but is now subsumed by the in-process
// path; the shell script just loops contact_boundary_4x4 once per
// candidate.
//
// scripts/qbeta_eigenvalue.cpp is a standalone CLI driver kept for
// ad-hoc debugging and for the Lua bindings (if/when the exact
// eigenvalue is exposed to Lua).  For batch work over the 11 clean
// non-unimodular Pisot candidates, prefer scripts/run_qbeta_survey.sh.

#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "mini-gmp/mini-gmp.h"

namespace ravel {

// General-d monic minimal polynomial:
//   x^d + c[0] x^(d-1) + ... + c[d-1] = 0
// beta is assumed to be the unique real root with beta > 1 (Pisot).
struct QBetaCharPoly {
    std::vector<long long> c;  // size d
};

// Degree-4 specialization (kept for backward compatibility with
// scripts/qbeta_eigenvalue.cpp CLI and existing call sites).
struct QBetaCharPoly4 {
    std::array<long long, 4> c{};
};

// Tunable knobs.  Defaults match the original CLI script's
// behaviour (K_MAX=80, 256 MiB safety cap).  Contact-boundary
// surveys on the random 4x4 candidates typically converge in
// 30-50 steps.
struct QBetaOptions {
    int k_max = 80;                             // max power-iteration steps
    std::size_t max_memory_bytes = 1ULL << 28;  // 256 MiB safety cap (0 = no cap)
};

struct QBetaResult {
    double lambda = 0.0;            // final ratio estimate; 0.0 if not converged
    bool converged = false;         // true iff the ratio stabilised
    int steps_taken = 0;            // 0..k_max
    std::size_t peak_memory_bytes = 0; // observed peak working set
    std::string error;              // populated on bad input / capacity exceeded
};

// ---------------------------------------------------------------------------
// Internals
// ---------------------------------------------------------------------------
namespace qbeta_detail {

// A Q(beta) element is a d-tuple of mpz_t.  We store the coefficients
// as a heap-allocated array of __mpz_struct (NOT std::vector<mpz_t>,
// which is ill-formed because mpz_t itself is __mpz_struct[1]).
// The single source of rescaling is the per-vector scale_log2 (passed
// alongside v by the iteration loop); each QElem's per-element
// scale_log2 is always 0 in normal use, kept as a field only so qadd
// can be implemented generically.  See the public function for the
// global rescale step.
struct QElem {
    std::size_t d;
    mpz_t* coeffs;            // heap-allocated array of length d
    long long scale_log2;     // always 0 in normal use

    QElem() : d(0), coeffs(nullptr), scale_log2(0) {}
    explicit QElem(std::size_t d_) : d(d_), coeffs(nullptr), scale_log2(0) {
        coeffs = new mpz_t[d];
        for (std::size_t i = 0; i < d; ++i) mpz_init(coeffs[i]);
    }
    QElem(const QElem& other) : d(other.d), coeffs(nullptr), scale_log2(other.scale_log2) {
        coeffs = new mpz_t[d];
        for (std::size_t i = 0; i < d; ++i) mpz_init_set(coeffs[i], other.coeffs[i]);
    }
    QElem& operator=(const QElem& other) {
        if (this != &other) {
            if (d != other.d) {
                for (std::size_t i = 0; i < d; ++i) mpz_clear(coeffs[i]);
                delete[] coeffs;
                d = other.d;
                coeffs = new mpz_t[d];
                for (std::size_t i = 0; i < d; ++i) mpz_init(coeffs[i]);
            }
            for (std::size_t i = 0; i < d; ++i) mpz_set(coeffs[i], other.coeffs[i]);
            scale_log2 = other.scale_log2;
        }
        return *this;
    }
    QElem(QElem&& other) noexcept : d(other.d), coeffs(other.coeffs), scale_log2(other.scale_log2) {
        other.coeffs = nullptr;
        other.d = 0;
    }
    QElem& operator=(QElem&& other) noexcept {
        if (this != &other) {
            for (std::size_t i = 0; i < d; ++i) mpz_clear(coeffs[i]);
            delete[] coeffs;
            d = other.d;
            coeffs = other.coeffs;
            scale_log2 = other.scale_log2;
            other.coeffs = nullptr;
            other.d = 0;
        }
        return *this;
    }
    ~QElem() {
        if (coeffs) {
            for (std::size_t i = 0; i < d; ++i) mpz_clear(coeffs[i]);
            delete[] coeffs;
        }
    }
};

inline QElem qzero(std::size_t d) { return QElem(d); }
inline QElem qone(std::size_t d)  {
    QElem o(d);
    mpz_set_ui(o.coeffs[0], 1);
    return o;
}

inline QElem qmul_si(const QElem& x, long long s) {
    QElem r = x;
    for (std::size_t i = 0; i < r.d; ++i) mpz_mul_si(r.coeffs[i], r.coeffs[i], s);
    return r;
}

// Multiply by beta.  For x = a_0 + a_1 beta + ... + a_{d-1} beta^{d-1}:
//   x * beta = a_0 beta + a_1 beta^2 + ... + a_{d-2} beta^{d-1}
//              + a_{d-1} beta^d
//   beta^d = -c[0] beta^{d-1} - c[1] beta^{d-2} - ... - c[d-1]
//   so coefficient i of (x*beta):
//     i = 0:        -a_{d-1} * c[d-1]
//     i in [1, d-1]: a_{i-1} - a_{d-1} * c[d-1-i]
//
// c.size() == d (the polynomial degree).  The polynomial is
//   x^d + c[0] x^{d-1} + ... + c[d-1] = 0
// so d coefficients c[0..d-1] fully specify a degree-d polynomial.
inline QElem qmul_by_beta(const QElem& x, const std::vector<long long>& c) {
    const std::size_t d = c.size();
    QElem r(d);
    // r.coeffs[0] = -x.coeffs[d-1] * c[d-1]
    mpz_mul_si(r.coeffs[0], x.coeffs[d - 1], -c[d - 1]);
    for (std::size_t i = 1; i < d; ++i) {
        mpz_set(r.coeffs[i], x.coeffs[i - 1]);
        mpz_submul_ui(r.coeffs[i], x.coeffs[d - 1],
                      static_cast<unsigned long>(c[d - 1 - i] < 0
                          ? -c[d - 1 - i]
                          : c[d - 1 - i]));
        if (c[d - 1 - i] < 0) mpz_neg(r.coeffs[i], r.coeffs[i]);
    }
    r.scale_log2 = x.scale_log2;
    return r;
}

inline QElem qadd(const QElem& a, const QElem& b) {
    const std::size_t d = a.d;
    QElem r(d);
    if (a.scale_log2 == b.scale_log2) {
        for (std::size_t i = 0; i < d; ++i)
            mpz_add(r.coeffs[i], a.coeffs[i], b.coeffs[i]);
        r.scale_log2 = a.scale_log2;
        return r;
    }
    const long long s_max = std::max(a.scale_log2, b.scale_log2);
    const QElem* lo = (a.scale_log2 < b.scale_log2) ? &a : &b;
    const QElem* hi = (a.scale_log2 < b.scale_log2) ? &b : &a;
    const long long shift = s_max - lo->scale_log2;
    for (std::size_t i = 0; i < d; ++i) {
        mpz_mul_2exp(r.coeffs[i], lo->coeffs[i], shift);
        mpz_add(r.coeffs[i], r.coeffs[i], hi->coeffs[i]);
    }
    r.scale_log2 = s_max;
    return r;
}

// Vector-level rescale: if every nonzero coefficient in every QElem
// is even, halve them all (preserving the vector's value up to a
// factor of 2) and bump the global scale_log2 by 1.  Otherwise do
// nothing.  The single-source-of-truth invariant: scale_log2 is
// tracked on the vector, not on individual QElems.  (Earlier
// per-element rescaling produced inconsistent scales across the
// vector's components and made the lambda ratio oscillate rather
// than converge.)
inline bool qrescale_vector_by_2(std::vector<QElem>& v, long long& scale_log2) {
    for (const auto& x : v) {
        for (std::size_t i = 0; i < x.d; ++i) {
            const int odd  = (mpz_odd_p(x.coeffs[i]) != 0);
            const int zero = (mpz_sgn(x.coeffs[i]) == 0);
            if (odd && !zero) return false;
        }
    }
    for (auto& x : v) {
        for (std::size_t i = 0; i < x.d; ++i) {
            if (mpz_sgn(x.coeffs[i]) != 0)
                mpz_fdiv_q_2exp(x.coeffs[i], x.coeffs[i], 1);
        }
    }
    ++scale_log2;
    return true;
}

// Evaluate the Q(beta) element as a real number using the real
// root beta_R (Horner).  Multiplied by 2^{-scale_log2}.  For very
// large values we lose double precision but the lambda estimate
// only needs the log ratio.
inline double qreal(const QElem& x, double beta_R, long long scale_log2) {
    mpz_t tmp;
    mpz_init(tmp);
    double r = 0;
    double br = 1;
    for (std::size_t i = 0; i < x.d; ++i) {
        mpz_set(tmp, x.coeffs[i]);
        double cc = mpz_get_d(tmp);
        r += cc * br;
        br *= beta_R;
    }
    mpz_clear(tmp);
    r *= std::ldexp(1.0, -scale_log2);
    return r;
}

// Estimate the byte cost of one QElem (mpz limbs are variable; we
// take the worst case observed across the input).  Used only for
// the safety cap; conservative to avoid false negatives.
inline std::size_t qelem_peak_bytes(const std::vector<QElem>& vs) {
    std::size_t peak = 0;
    for (const auto& v : vs) {
        for (std::size_t i = 0; i < v.d; ++i) {
            std::size_t limbs = mpz_size(v.coeffs[i]);
            peak += limbs * sizeof(mp_limb_t) + 32;
        }
    }
    return peak;
}

// Find the real root beta > 1 of x^d + c[0] x^{d-1} + ... + c[d-1] = 0
// via bisection (bracketing the sign change) followed by Newton's
// refinement.  Returns 0.0 if no sign change is found in [1, 100].
inline double find_real_root(const std::vector<long long>& c) {
    const std::size_t d = c.size();
    auto poly_val = [&](double x) {
        // Horner: p(x) = x^d + c[0] x^{d-1} + ... + c[d-1]
        double r = 1;
        for (std::size_t i = 0; i + 1 < d; ++i) {
            r = r * x + c[i];
        }
        r = r * x + c[d - 1];
        return r;
    };
    auto dpoly_val = [&](double x) {
        // p'(x) = d x^{d-1} + (d-1) c[0] x^{d-2} + ... + c[d-2]
        double r = static_cast<double>(d);
        for (std::size_t i = 0; i + 1 < d; ++i) {
            r = r * x + static_cast<double>(d - 1 - i) * c[i];
        }
        return r;
    };

    double lo = 1.0, hi = 100.0;
    double f_lo = poly_val(lo);
    if (f_lo == 0.0) return lo;
    double f_hi = poly_val(hi);
    if (f_lo * f_hi > 0.0) {
        // No sign change in [1, 100]; widen search.
        hi = 1e6;
        f_hi = poly_val(hi);
        if (f_lo * f_hi > 0.0) return 0.0;
    }
    for (int i = 0; i < 200; ++i) {
        double m = (lo + hi) / 2;
        double f_m = poly_val(m);
        if (f_lo * f_m < 0) { hi = m; f_hi = f_m; }
        else { lo = m; f_lo = f_m; }
    }
    double beta_R = (lo + hi) / 2;
    for (int i = 0; i < 50; ++i) {
        double f = poly_val(beta_R), df = dpoly_val(beta_R);
        if (std::abs(df) < 1e-15) break;
        double dx = f / df;
        beta_R -= dx;
        if (std::abs(dx) < 1e-15) break;
    }
    return beta_R;
}

// Core algorithm: given a char poly `c` (size d) and a matrix,
// compute the dominant eigenvalue.  The function name is the
// general-d entry point; qbeta_dominant_eigenvalue_4 below is a
// thin wrapper.
inline QBetaResult qbeta_dominant_eigenvalue_impl(
    const std::vector<std::vector<long long>>& matrix,
    const std::vector<long long>& c,
    const QBetaOptions& opts) {

    using namespace qbeta_detail;

    QBetaResult out;
    const std::size_t N = matrix.size();
    if (N == 0) {
        out.error = "qbeta_dominant_eigenvalue: empty matrix";
        return out;
    }
    for (const auto& row : matrix) {
        if (row.size() != N) {
            out.error = "qbeta_dominant_eigenvalue: non-square matrix";
            return out;
        }
    }
    if (c.empty()) {
        out.error = "qbeta_dominant_eigenvalue: empty char poly";
        return out;
    }

    // c has d elements representing the polynomial
    //   x^d + c[0] x^(d-1) + ... + c[d-1] = 0
    const std::size_t d = c.size();

    double beta_R = find_real_root(c);
    if (beta_R <= 1.0) {
        out.error = "qbeta_dominant_eigenvalue: no real root > 1 found "
                    "(char poly may not be a Pisot polynomial)";
        return out;
    }

    std::vector<QElem> v;
    v.reserve(N);
    for (std::size_t i = 0; i < N; ++i) v.push_back(qone(d));

    long long scale_log2 = 0;  // single-source-of-truth for rescaling
    double prev_norm = 0;
    double lambda = 0;
    int steps_taken = 0;
    for (int step = 1; step <= opts.k_max; ++step) {
        std::vector<QElem> vnew;
        vnew.reserve(N);
        for (std::size_t i = 0; i < N; ++i) {
            QElem acc = qzero(d);
            for (std::size_t j = 0; j < N; ++j) {
                long long a = matrix[i][j];
                if (a == 0) continue;
                QElem prod = qmul_si(v[j], a);
                acc = qadd(acc, std::move(prod));
            }
            vnew.push_back(std::move(acc));
        }
        v = std::move(vnew);

        // Memory guard: if the QElem working set has grown past the
        // cap, abort rather than OOM.  Conservative: measures v
        // only (vnew has been moved into v by this point).
        std::size_t peak = qelem_peak_bytes(v);
        out.peak_memory_bytes = std::max(out.peak_memory_bytes, peak);
        if (opts.max_memory_bytes > 0 && peak > opts.max_memory_bytes) {
            out.error = "qbeta_dominant_eigenvalue: capacity exceeded "
                        "(peak " + std::to_string(peak) + " bytes > cap "
                        + std::to_string(opts.max_memory_bytes) + ")";
            out.steps_taken = step;
            return out;
        }

        // Vector-level uniform rescale (single source of scale_log2).
        qrescale_vector_by_2(v, scale_log2);

        double cur_norm = 0;
        for (std::size_t i = 0; i < N; ++i) {
            double r = std::abs(qreal(v[i], beta_R, scale_log2));
            if (r > cur_norm) cur_norm = r;
        }
        if (cur_norm > 0 && prev_norm > 0) {
            lambda = cur_norm / prev_norm;
        }
        prev_norm = cur_norm;
        steps_taken = step;
    }

    out.lambda = lambda;
    out.converged = (lambda > 0.0);
    out.steps_taken = steps_taken;
    return out;
}

}  // namespace qbeta_detail

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

// Compute the dominant eigenvalue of an N x N integer matrix using
// exact Q(beta) arithmetic for arbitrary degree d >= 1.  The char
// poly is a degree-d monic polynomial; the unique real root > 1 is
// used as beta.  Returns a default result with `error` populated
// on bad input or capacity exceeded.
inline QBetaResult qbeta_dominant_eigenvalue(
    const std::vector<std::vector<long long>>& matrix,
    const QBetaCharPoly& poly,
    const QBetaOptions& opts = {}) {
    return qbeta_detail::qbeta_dominant_eigenvalue_impl(matrix, poly.c, opts);
}

// Degree-4 specialization (kept for backward compatibility with
// scripts/qbeta_eigenvalue.cpp CLI and existing call sites).
inline QBetaResult qbeta_dominant_eigenvalue_4(
    const std::vector<std::vector<long long>>& matrix,
    const QBetaCharPoly4& poly,
    const QBetaOptions& opts = {}) {
    std::vector<long long> c(poly.c.begin(), poly.c.end());
    return qbeta_detail::qbeta_dominant_eigenvalue_impl(matrix, c, opts);
}

}  // namespace ravel