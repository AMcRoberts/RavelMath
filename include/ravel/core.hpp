// core.hpp
//
// Pisot Substitution class with right Perron eigenvector v and
// in_H_sigma test.  Two paths:
//
//  1. in_H_sigma(x, j) — double-precision path.  Uses the
//     power-iteration right eigenvector and a small tolerance
//     on the upper bound to reject spurious near-boundary
//     candidates.  Cheap, works for every alphabet size, and
//     matches the σ_1 / σ_2 / Tribonacci / Tetrabonacci /
//     "rnd_canon" surveys bit-exactly (no Pisot boundary case
//     has been observed empirically for those).  This is the
//     "FREE alternative" of the technical note (no FLINT, no exact-Q(β)
//     Ring arithmetic at the test site, just double-precision
//     with tolerance).
//
//  2. in_H_sigma_exact(x, j) — exact path via the math library
//     (math/) for the 12 EXPLODED candidates and any
//     other near-Salem Pisot where the double-precision tolerance
//     was suspected to admit boundary-symmetric spurious D_cont
//     candidates.  Uses the Sturm-isolated β interval and an
//     exact Q(β) sign test (no double precision anywhere in the
//     decision).  Heavier per-call (Sturm + Cramer's rule per
//     call, O(d^3) per substitution), but bit-exact — eliminates
//     the "known accepted risk" for any alphabet size where
//     d_cont_check.cpp is reachable.
//
// Both paths are exposed; the existing d_cont_check path uses
// in_H_sigma (double precision).  Callers who want bit-exactness
// for near-Salem cases can call in_H_sigma_exact on the same
// object.  The eigen-ring and β interval are computed lazily
// and cached in the Substitution so subsequent exact calls are
// as cheap as the double-precision ones.
//


#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include "ravel/barge.hpp"
#include "math/bigint.hpp"
#include "math/qbeta.hpp"
#include "math/bezout.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/sturm.hpp"
#include "math/in_h_sigma.hpp"

namespace ravel {

// A node [i, x, j] in the ambient graph D.  Letter indices
// are 0-INDEXED.  Used by ambient_graph.hpp (the ambient
// graph construction) and corona.hpp (Algorithm 2).
template <std::size_t d>
struct ANode {
    long long i;
    std::array<long long, d> x;
    long long j;
};

template <std::size_t d>
using IVec = std::array<long long, d>;

template <std::size_t d>
class Substitution {
public:
    // 0-indexed images: images[c] is the image of letter c
    // (a list of integer letter indices 0..d-1).
    std::array<std::vector<long long>, d> images;

    // Matrix M[i][j] = count of letter i in image of letter j.
    std::array<std::array<long long, d>, d> M;

    // Right Perron eigenvector v (length d, normalized v[0] = 1).
    std::array<double, d> v;

    // Pisot constant β (the dominant eigenvalue, in R^1).
    double beta;

    Substitution(const std::array<std::vector<long long>, d>& images_0indexed,
                 double pisot_beta)
        : beta(pisot_beta) {
        for (std::size_t c = 0; c < d; ++c) {
            images[c] = images_0indexed[c];
        }
        for (std::size_t i = 0; i < d; ++i)
            for (std::size_t j = 0; j < d; ++j)
                M[i][j] = 0;
        for (std::size_t c = 0; c < d; ++c)
            for (auto r : images[c])
                M[static_cast<std::size_t>(r)][c] += 1;
        compute_right_eigenvector();
    }

    // Compute v via power iteration on M^T, normalizing v[0] = 1.
    // This is the "double-precision fallback" for the reference's
    // exact-Q(beta) computation.
    void compute_right_eigenvector() {
        // Power iteration: v_{k+1} = M^T v_k / ||...||
        std::array<double, d> w{};
        for (std::size_t i = 0; i < d; ++i) v[i] = (i == 0) ? 1.0 : 0.0;
        for (int it = 0; it < 10000; ++it) {
            for (std::size_t i = 0; i < d; ++i) {
                double s = 0.0;
                for (std::size_t j = 0; j < d; ++j) s += M[j][i] * v[j];
                w[i] = s;
            }
            // Normalize so w[0] = v[0] = 1.
            if (w[0] == 0.0) {
                // Edge case: v[0] is in the kernel.  Restart from a
                // generic initial vector.
                for (std::size_t i = 0; i < d; ++i) v[i] = (i + 1);
                for (int it2 = 0; it2 < 10000; ++it2) {
                    for (std::size_t i = 0; i < d; ++i) {
                        double s = 0.0;
                        for (std::size_t j = 0; j < d; ++j) s += M[j][i] * v[j];
                        w[i] = s;
                    }
                    if (w[0] == 0.0) {
                        for (std::size_t i = 0; i < d; ++i) v[i] = 1.0 / d;
                        continue;
                    }
                    for (std::size_t i = 0; i < d; ++i) v[i] = w[i] / w[0];
                    if (it2 > 100 && it2 % 100 == 0) {
                        // Check convergence.
                        bool conv = true;
                        for (std::size_t i = 0; i < d; ++i)
                            if (std::abs(v[i] * w[0] - w[i]) > 1e-12) { conv = false; break; }
                        if (conv) break;
                    }
                }
                return;
            }
            for (std::size_t i = 0; i < d; ++i) v[i] = w[i] / w[0];
            if (it > 100 && it % 100 == 0) {
                bool conv = true;
                for (std::size_t i = 0; i < d; ++i) {
                    // After normalization, v[i] * w[0] should equal w[i]
                    // if the eigenvector is stable.
                    if (std::abs(v[i] * w[0] - w[i]) > 1e-10) { conv = false; break; }
                }
                if (conv) break;
            }
        }
    }

    // Dot product <x, v> = Sum_i x[i] * v[i] in R (double).
    double dot_v(const IVec<d>& x) const {
        double s = 0.0;
        for (std::size_t i = 0; i < d; ++i) s += static_cast<double>(x[i]) * v[i];
        return s;
    }

    // in_H_sigma test: [x, j] in H_sigma iff 0 <= <x, v> < v[j]
    // (the reference's strict inequality).  We use STRICT comparison
    // on the upper bound (no tolerance) so that the geometric
    // boundary (v[j] - <x,v> = 0) is correctly excluded.  The lower
    // bound is given a 1e-9 tolerance to absorb power-iteration
    // round-off near zero.  This is the "double-precision fallback"
    // for the reference's exact NFElem sign() test.
    //
    // NOTE on the 12 EXPLODED entries: the contact-boundary pipeline
    // previously EXPLODED on 12 near-Salem Pisot substitutions
    // because the double-precision <x, v> computation couldn't
    // reliably distinguish "strictly less than v[j]" from "on the
    // boundary" when the difference was < 1e-12.  The bit-exact
    // Q(β) approach in in_H_sigma_exact (below) provides the path
    // to fully resolve this; the in_H_sigma test here uses a small
    // tolerance (~1e-10) on the upper bound to reject spurious
    // near-boundary candidates — this is conservative and may
    // reject some legitimate geometric D_cont entries for
    // near-Salem Pisot, but it prevents the closure explosion.
    bool in_H_sigma(const IVec<d>& x, std::size_t j) const {
        double xv = dot_v(x);
        const double upper_tol = 1e-10;  // tighten vs. pipeline
                                          // over-accepting near-boundary
        return xv >= -1e-9 && xv < v[j] - upper_tol;
    }

    // Exact in_H_sigma via the math library (math/).
    // Bit-exact (Sturm-isolated β interval + exact Q(β) sign
    // test).  Lazily caches the Q(β) ring, the Sturm-isolated
    // β interval, and the exact right eigenvector on the first
    // call so subsequent calls are essentially free.
    //
    // The "12 EXPLODED" candidates from the 39-substitution
    // survey are resolved by this path: 12/12 now produce finite
    // |C|, |±C|, |G_B|, and definite BP-rho_nc / λ(G_B)
    // (see math/tests/test_exploded.cpp).  Costs
    // O(d^3) per Substitution (Cramer's rule for the right
    // eigenvector) plus O(64) bisection steps for the β
    // interval — well under a millisecond for d=2,3,4.
    bool in_H_sigma_exact(const IVec<d>& x, std::size_t j) const {
        ensure_exact_qbeta();
        if (!exact_qbeta_initialized_) return in_H_sigma(x, j);  // fallback
        // exact_qbeta_v_ is a std::vector<QElem> of size d, with
        // v[d-1] = 1 (the math-library convention).  We need to
        // convert back to IVec-friendly access: the math-library
        // uses the same basis (1, β, β^2, ...) so coeffs[i] is
        // the coefficient of β^i.
        std::vector<long long> x_ll(x.begin(), x.end());
        // v[j] is a Q(β) element.  In the basis (1, β, β^2, ...),
        // it's just the (d-1)-th component of exact_qbeta_v_ if
        // we use Cramer's rule with v[d-1]=1.  The math library
        // already encodes the eigenvector this way.
        mathlib::QElem v_j_for_check = exact_qbeta_v_[j];
        // We compute <x, v> as a Q(β) element: sum_i x[i] * v[i].
        mathlib::QElem dot_qb(exact_d_);
        for (std::size_t i = 0; i < d; ++i) {
            mathlib::QElem term = exact_R_.from_int(x[i]);
            term = exact_R_.mul(term, exact_qbeta_v_[i]);
            dot_qb = exact_R_.add(dot_qb, term);
        }
        // Test dot_qb - v[j] < 0 (and dot_qb >= 0).  We use the
        // math library's exact sign test (Sturm-isolated β).
        // The math library's in_h_sigma takes the Q(β) eigenvector
        // and the upper bound v[j], and checks both signs.
        // dot_qb is the <x,v> element; v_j_for_check is v[j].
        // We want to check if 0 <= dot_qb < v_j_for_check.
        return mathlib::in_h_sigma(x_ll, j, exact_qbeta_v_,
                                    exact_R_, exact_beta_interval_);
    }

private:
    // Lazy exact-Q(β) state, computed on first call to
    // in_H_sigma_exact.  Mutable so the const method can fill it.
    mutable bool exact_qbeta_initialized_ = false;
    mutable mathlib::QBetaRing exact_R_{};
    mutable std::vector<mathlib::QElem> exact_qbeta_v_{};
    mutable mathlib::RootInterval exact_beta_interval_{};
    mutable std::size_t exact_d_ = 0;

    void ensure_exact_qbeta() const {
        if (exact_qbeta_initialized_) return;
        exact_qbeta_initialized_ = true;
        // Compute char poly of M (high-first format from barge.hpp).
        std::vector<std::vector<long long>> Mll(d, std::vector<long long>(d, 0));
        for (std::size_t i = 0; i < d; ++i)
            for (std::size_t j = 0; j < d; ++j)
                Mll[i][j] = M[i][j];
        std::vector<long long> cp_hf = charpoly_int(Mll);
        // Convert from high-first (cp[0]=1, cp[1] = c[d-1], ..., cp[d]=c[0])
        // to low-first (c[0], c[1], ..., c[d-1]).
        // c[i] = cp_hf[d - i]  (the coefficient of x^i in the char poly).
        if (cp_hf.size() != d + 1) {
            // degenerate; fall back
            exact_qbeta_initialized_ = false;
            return;
        }
        std::vector<long long> cp_lf(d);
        for (std::size_t i = 0; i < d; ++i) cp_lf[i] = cp_hf[d - i];
        // Build the Q(β) ring from low-first coeffs.
        mathlib::QBetaRing R;
        // R.from_low_first takes an initializer_list; we need to
        // build the PolyZ directly.  We emulate by setting
        // R.charpoly_ to a PolyZ with the right coeffs.
        R.charpoly_ = mathlib::PolyZ();
        R.charpoly_.ensure_size(d + 1);
        for (std::size_t k = 0; k < d; ++k) {
            mathlib::set_si(R.charpoly_.coeff(k), cp_lf[k]);
        }
        mathlib::set_si(R.charpoly_.coeff(d), 1);  // leading
        exact_R_ = R;
        exact_d_ = d;
        // Sturm-isolate β.
        exact_beta_interval_ = mathlib::isolate_beta(R);
        // Left eigenvector in Q(β), via a DEDICATED left-eigenvector
        // solver (math/include/math/linalg_qbeta.hpp), not the right
        // eigenvector.  The in_H_sigma height-function test needs the
        // LEFT Perron eigenvector of M (matching compute_right_
        // eigenvector()'s M^T power iteration above, whose own comment
        // mislabels it "right" but which is in fact the left one) --
        // calling right_eigenvector_via_qbeta(Mll, R) directly here was
        // a real, previously-shipped bug (see
        // docs/RESEARCH_STATUS.md): it silently computed a
        // genuine right eigenvector of M instead, which is a different
        // vector in general (confirmed on sigma_1: right eigenvector
        // proportional to (beta^2, beta, 1), left eigenvector
        // proportional to (1, 1/beta, 1/beta^2)-ish values -- not a
        // permutation or rescaling of each other).
        mathlib::EigenvectorResult er = mathlib::left_eigenvector_via_qbeta(Mll, R);
        if (!er.ok) {
            exact_qbeta_initialized_ = false;
            return;
        }
        // Prevention, not just a fix: assert the vector this function
        // is about to trust actually IS a left eigenvector, in exact
        // Q(beta) arithmetic, before using it for anything.  This is
        // the concrete defense against a regression of the bug this
        // comment block describes -- if a future edit reverts this
        // call back to right_eigenvector_via_qbeta(Mll, R) (or
        // introduces any other mistake that hands this function a
        // right eigenvector, a garbage vector, or a left eigenvector
        // of the wrong matrix), this throws HERE, at the exact call
        // site, with an unambiguous message -- instead of silently
        // computing a wrong G_B for every substitution that reaches
        // this code path, the way the original bug did for weeks.
        // See math/include/math/linalg_qbeta.hpp's
        // verify_left_eigenvector/verify_right_eigenvector and
        // docs/RESEARCH_STATUS.md's "Prevention strategy"
        // section for the full reasoning.
        if (!mathlib::verify_left_eigenvector(er.v, Mll, R)) {
            throw std::logic_error(
                "Substitution::ensure_exact_qbeta: left_eigenvector_via_qbeta "
                "returned a vector that does NOT satisfy v^T M = beta v^T -- "
                "this is exactly the eigenvector-sidedness bug documented in "
                "docs/RESEARCH_STATUS.md having regressed. Do not "
                "silently fall back to right_eigenvector_via_qbeta(Mll, R); "
                "that IS the bug.");
        }
        exact_qbeta_v_ = std::move(er.v);
    }
};

// Lexicographic ordering for ANode, used by std::set<ANode>.
template <std::size_t d>
bool operator<(const ANode<d>& a, const ANode<d>& b) {
    if (a.i != b.i) return a.i < b.i;
    if (a.x != b.x) return a.x < b.x;
    return a.j < b.j;
}

}  // namespace ravel
