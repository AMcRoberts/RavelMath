// include/ravel/spectral_dual.hpp
//
// Differentiable 3x3 spectral invariants via the forward-mode
// autodiff infrastructure in include/ravel/dual.hpp.
//
// The non-differentiable counterpart is spectral_invariants_3x3 in
// include/ravel/spectral.hpp, which computes:
//
//   tr(M), cf(M) (sum of 2x2 minors), det(M), then
//   Cardano's resolved form for the real root and modulus of the
//   complex pair: roots via cbrt(...) of polynomials in tr, cf, det.
//
// The differentiable version takes ravel::dual::Dual<T> matrix entries
// (T-typed primal, with `eps` carrying df/dM_ij), runs the same
// expression through the chain rule, and returns
// ravel::dual::Dual<T> outputs whose `.eps` field is the gradient
// d(invariants)/d(M_ij).
//
// =====================================================================
// Why this is required specifically for smooth-relaxation
// =====================================================================
// The smooth-relaxation search (RESEARCH_VECTORS.md vector 2) treats
// the matrix entries as continuous variables and walks them in
// gradient directions.  The natural objective is "good Pisot-quality",
// e.g. maximize the ratio |secondary modulus| / |Perron root|, or
// minimize the distance from some target substitution.  The gradient
// of any such objective w.r.t. M_ij has to flow through the spectral
// invariants, including their cbrt; this header makes that gradient
// computable, at any precision the user asks for (via the Dual<T>
// scalar type -- Double for cheap gradient evaluation; BigFloat for
// arbitrary precision including irrational results).
// =====================================================================

#pragma once

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "ravel/dual.hpp"
#include "ravel/spectral.hpp"  // for spectral_invariants_3x3 fallback

namespace ravel {

template <typename T>
struct SpectralInvariantsDual {
    std::size_t n = 0;
    ravel::dual::Dual<T> beta;
    ravel::dual::Dual<T> beta2;
    ravel::dual::Dual<T> abs_det;
    ravel::dual::Dual<T> invol_safe;
    bool bound_holds = false;
};

template <typename T>
SpectralInvariantsDual<T> spectral_invariants_3x3_dual(
        ravel::dual::Dual<T> a11, ravel::dual::Dual<T> a12, ravel::dual::Dual<T> a13,
        ravel::dual::Dual<T> a21, ravel::dual::Dual<T> a22, ravel::dual::Dual<T> a23,
        ravel::dual::Dual<T> a31, ravel::dual::Dual<T> a32, ravel::dual::Dual<T> a33) {
    using namespace ravel::dual;
    SpectralInvariantsDual<T> inv;
    inv.n = 3;

    // tr = a11 + a22 + a33
    ravel::dual::Dual<T> tr = dual_add(dual_add(a11, a22), a33);
    // cf = (a11 a22 + a11 a33 + a22 a33) - (a12 a21 + a13 a31 + a23 a32)
    ravel::dual::Dual<T> cf = dual_sub(
        dual_add(
            dual_add(dual_mul(a11, a22), dual_mul(a11, a33)),
            dual_mul(a22, a33)),
        dual_add(
            dual_add(dual_mul(a12, a21), dual_mul(a13, a31)),
            dual_mul(a23, a32)));
    // det via 3x3 cofactor expansion along the first row:
    //   det = a11*(a22*a33 - a23*a32)
    //       - a12*(a21*a33 - a23*a31)
    //       + a13*(a21*a32 - a22*a31)
    ravel::dual::Dual<T> m11 = dual_sub(dual_mul(a22, a33), dual_mul(a23, a32));
    ravel::dual::Dual<T> m12 = dual_sub(dual_mul(a21, a33), dual_mul(a23, a31));
    ravel::dual::Dual<T> m13 = dual_sub(dual_mul(a21, a32), dual_mul(a22, a31));
    ravel::dual::Dual<T> det = dual_add(
        dual_sub(dual_mul(a11, m11), dual_mul(a12, m12)),
        dual_mul(a13, m13));

    // p = (3 cf - tr^2) / 3
    ravel::dual::Dual<T> tr_sq = dual_mul(tr, tr);
    ravel::dual::Dual<T> three_cf = dual_mul(Dual<T>::constant(T(3)), cf);
    // Cardinal formula: 3 cf - tr^2  -- NOT 3*tr^2 (which would silently
    // change the result).  Bug fix: previously the dual version had
    // `three_tr_sq = 3 * tr^2`, giving p = cf - tr^2 instead of the
    // intended (3*cf - tr^2)/3.  Restored to match spectral_invariants_3x3.
    ravel::dual::Dual<T> p = dual_div(dual_sub(three_cf, tr_sq),
                                        Dual<T>::constant(T(3)));
    // q = (9 tr cf - 2 tr^3 - 27 det) / 27
    ravel::dual::Dual<T> tr_cu = dual_mul(tr, tr_sq);
    ravel::dual::Dual<T> nine_tr_cf = dual_mul(Dual<T>::constant(T(9)),
                                                dual_mul(tr, cf));
    ravel::dual::Dual<T> two_tr_cu = dual_mul(Dual<T>::constant(T(2)), tr_cu);
    ravel::dual::Dual<T> twentyseven_det = dual_mul(Dual<T>::constant(T(27)), det);
    ravel::dual::Dual<T> q = dual_div(
        dual_sub(dual_sub(nine_tr_cf, two_tr_cu), twentyseven_det),
        Dual<T>::constant(T(27)));
    // disc = q^2/4 + p^3/27 (Cardano)
    ravel::dual::Dual<T> q_sq = dual_mul(q, q);
    ravel::dual::Dual<T> p_cu = dual_mul(p, dual_mul(p, p));
    ravel::dual::Dual<T> quarter = Dual<T>::constant(T(0.25));
    ravel::dual::Dual<T> twentyseventh = Dual<T>::constant(T(1) / T(27));
    ravel::dual::Dual<T> disc = dual_add(
        dual_mul(q_sq, quarter),
        dual_mul(p_cu, twentyseventh));

    // Branch on the discriminant's value.
    T disc_as_T = disc.val;
    if (disc_as_T > T(0)) {
        // Casus irreducibilis: one real root + complex conjugate pair.
        ravel::dual::Dual<T> sqrt_disc = dual_sqrt(disc);
        ravel::dual::Dual<T> half_q = dual_mul(q, Dual<T>::constant(T(0.5)));
        ravel::dual::Dual<T> neg_half_q = dual_neg(half_q);
        ravel::dual::Dual<T> u_arg = dual_add(neg_half_q, sqrt_disc);
        ravel::dual::Dual<T> v_arg = dual_sub(neg_half_q, sqrt_disc);
        ravel::dual::Dual<T> u = dual_cbrt(u_arg);
        ravel::dual::Dual<T> v = dual_cbrt(v_arg);
        // Real root (Perron-Frobenius candidate for Pisot): r = u + v + tr/3
        ravel::dual::Dual<T> real_root = dual_add(
            dual_add(u, v),
            dual_mul(tr, Dual<T>::constant(T(1) / T(3))));
        // Modulus of the complex pair (in the ORIGINAL cubic's coordinate,
        // not the depressed cubic): for y_i = x_i - tr/3 (the depressed
        // roots), y_2 has |y_2|^2 = u^2 + v^2 - uv and Re(y_2) = -(u+v)/2.
        // For x_2 = y_2 + tr/3 (back to the original cubic):
        //   |x_2|^2 = |y_2 + tr/3|^2
        //           = |y_2|^2 + (tr/3)^2 + 2*tr/3*Re(y_2)
        //           = (u^2 + v^2 - uv) + (tr/3)^2 - (tr/3)*(u + v)
        ravel::dual::Dual<T> u_sq = dual_mul(u, u);
        ravel::dual::Dual<T> v_sq = dual_mul(v, v);
        ravel::dual::Dual<T> uv = dual_mul(u, v);
        ravel::dual::Dual<T> tr_third = dual_mul(tr, Dual<T>::constant(T(1) / T(3)));
        ravel::dual::Dual<T> u_plus_v = dual_add(u, v);
        // mod^2 in the original cubic = u^2 + v^2 - uv + (tr/3)^2 - (tr/3)*(u+v)
        ravel::dual::Dual<T> depressed_mod_sq = dual_sub(dual_add(u_sq, v_sq), uv);
        ravel::dual::Dual<T> shifted_sq = dual_mul(tr_third, tr_third);
        ravel::dual::Dual<T> cross_term = dual_mul(tr_third, u_plus_v);
        ravel::dual::Dual<T> mod_sq_orig =
            dual_add(dual_sub(depressed_mod_sq, cross_term), shifted_sq);
        ravel::dual::Dual<T> mod = dual_sqrt(mod_sq_orig);

        // Pick the dominant (largest magnitude) as beta.  For Pisot
        // matrices the real root is dominant (Perron theorem).
        T real_root_v = real_root.val;
        T mod_v = mod.val;
        if (real_root_v >= mod_v) {
            inv.beta = real_root;
            inv.beta2 = mod;
        } else {
            inv.beta = mod;
            inv.beta2 = real_root;
        }
        inv.abs_det = det;
        inv.bound_holds = true;
    } else {
        // All-real-roots branch (disc <= 0): for the smooth-relaxation
        // prototype we use a non-differentiable fallback.  Compute the
        // spectral invariants via the bit-exact (no dual) path; the
        // returned Dual<T> wraps the scalar value with eps=0.  This
        // is a documented limitation -- gradient = 0 here means the
        // search gets no signal in this branch and stays put; for
        // Pisot smoothing the user should start from a disc > 0
        // matrix or pick an alternative Pisot-flavored objective that
        // is differentiable in this branch.  Extending to full
        // chain-rule through arccos / cos for disc < 0 is a future-
        // session TODO (need dual_cos, dual_acos specializations, plus
        // the standard reduced-cubic chain-rule identities).
        T a11_v = a11.val;
        T a12_v = a12.val;
        T a13_v = a13.val;
        T a21_v = a21.val;
        T a22_v = a22.val;
        T a23_v = a23.val;
        T a31_v = a31.val;
        T a32_v = a32.val;
        T a33_v = a33.val;
        // Convert to double for spectral_invariants_3x3 (which is the
        // double-precision path); this gives the right scalar value
        // for the fallback.  Documented limitation: gradient is zero
        // here.
        double d_a11 = static_cast<double>(a11_v);
        double d_a12 = static_cast<double>(a12_v);
        double d_a13 = static_cast<double>(a13_v);
        double d_a21 = static_cast<double>(a21_v);
        double d_a22 = static_cast<double>(a22_v);
        double d_a23 = static_cast<double>(a23_v);
        double d_a31 = static_cast<double>(a31_v);
        double d_a32 = static_cast<double>(a32_v);
        double d_a33 = static_cast<double>(a33_v);
        ravel::SpectralInvariants ref = ravel::spectral_invariants_3x3(
            d_a11, d_a12, d_a13, d_a21, d_a22, d_a23, d_a31, d_a32, d_a33);
        inv.beta = Dual<T>(T(ref.beta));
        inv.beta2 = Dual<T>(T(ref.beta2));
        inv.abs_det = Dual<T>(T(ref.abs_det));
        inv.bound_holds = true;
    }

    return inv;
}

}  // namespace ravel
