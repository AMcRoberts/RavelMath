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
        // All-real-roots branch (disc <= 0): the standard trigonometric
        // Cardano solution, differentiated via dual_cos/dual_acos.
        // For the depressed cubic t^3 + p*t + q = 0 with p < 0 (forced
        // whenever disc <= 0 and not the fully-degenerate p=q=0 case):
        //   t_k = 2*sqrt(-p/3) * cos( (1/3)*acos(arg) - 2*pi*k/3 ),
        //   arg = (3q)/(2p) * sqrt(-3/p),  k = 0, 1, 2.
        // x_k = t_k + tr/3 recovers the original cubic's roots. All
        // three are real here, so "dominant" is just largest by value
        // and "secondary" is second-largest in absolute value, matching
        // the disc>0 branch's beta/beta2 convention (real eigenvalues
        // only, so beta2 there is also effectively a real comparison).
        T p_v = p.val;
        if (p_v == T(0)) {
            // Fully degenerate: p = q = 0 forces a triple root at
            // tr/3. Not differentiable through the trig formula (arg
            // is 0/0); return the exact triple root with zero gradient
            // rather than dividing by zero.
            Dual<T> triple = dual_div(tr, Dual<T>::constant(T(3)));
            inv.beta = Dual<T>(triple.val, T(0));
            inv.beta2 = Dual<T>(triple.val, T(0));
            inv.abs_det = Dual<T>(det.val < T(0) ? -det.val : det.val, T(0));
            inv.bound_holds = true;
            return inv;
        }
        Dual<T> neg_p_third = dual_div(dual_neg(p), Dual<T>::constant(T(3)));
        Dual<T> coeff = dual_mul(Dual<T>::constant(T(2)), dual_sqrt(neg_p_third));
        Dual<T> neg_three_over_p = dual_div(Dual<T>::constant(T(-3)), p);
        Dual<T> sqrt_neg3_p = dual_sqrt(neg_three_over_p);
        Dual<T> three_q = dual_mul(Dual<T>::constant(T(3)), q);
        Dual<T> two_p = dual_mul(Dual<T>::constant(T(2)), p);
        Dual<T> ratio = dual_div(three_q, two_p);
        Dual<T> arg = dual_mul(ratio, sqrt_neg3_p);
        // Clamp the acos argument's PRIMAL value into [-1,1] against
        // roundoff (the algebra guarantees |arg| <= 1 exactly when
        // disc <= 0; floating evaluation can land a hair outside).
        T arg_v = arg.val;
        if (arg_v > T(1)) arg_v = T(1);
        if (arg_v < T(-1)) arg_v = T(-1);
        Dual<T> arg_clamped = Dual<T>(arg_v, arg.eps);
        Dual<T> acos_arg = dual_acos(arg_clamped);
        Dual<T> third_acos = dual_div(acos_arg, Dual<T>::constant(T(3)));
        Dual<T> tr_third = dual_div(tr, Dual<T>::constant(T(3)));

        const double two_pi = 6.283185307179586476925286766559;
        Dual<T> roots[3];
        for (int k = 0; k < 3; ++k) {
            Dual<T> phase = dual_sub(third_acos,
                Dual<T>::constant(T(two_pi * k / 3.0)));
            Dual<T> t_k = dual_mul(coeff, dual_cos(phase));
            roots[k] = dual_add(t_k, tr_third);
        }
        // Pick dominant (largest value) and secondary (second-largest
        // in absolute value), matching the disc>0 branch's convention.
        int dom = 0;
        for (int k = 1; k < 3; ++k) if (roots[k].val > roots[dom].val) dom = k;
        int sec = -1;
        for (int k = 0; k < 3; ++k) {
            if (k == dom) continue;
            if (sec == -1 || std::abs(roots[k].val) > std::abs(roots[sec].val)) sec = k;
        }
        inv.beta = roots[dom];
        Dual<T> sec_root = roots[sec];
        inv.beta2 = sec_root.val < T(0) ? dual_neg(sec_root) : sec_root;
        inv.abs_det = det.val < T(0) ? dual_neg(det) : det;
        inv.bound_holds = true;
    }

    return inv;
}

}  // namespace ravel
