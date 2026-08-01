// math/in_h_sigma.hpp
//
// Application layer: the geometric test 0 ≤ <x, v> < v[j] for x ∈ Z^d
// and v ∈ Q(β)^d, using exact Q(β) arithmetic + Sturm-based sign test.
//
// This is the replacement for the double-precision in_H_sigma in
// core.hpp::Substitution that fails for the 12 EXPLODED entries
// (near-Salem Pisot substitutions with |b_2| ≥ 0.85).
//
// All arithmetic is exact:
//   * <x, v> is a Q(β) element computed via Tier 1 Q(β) mul.
//   * The sign of <x, v> is determined via Tier 3.5 Sturm isolation
//     (no double precision).
//   * The result is bit-exact, so spurious near-boundary admits are
//     eliminated.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "math/bigint.hpp"
#include "math/poly_z.hpp"
#include "math/qbeta.hpp"
#include "math/sturm.hpp"

namespace mathlib {

// ===================================================================
// Compute <x, v> where x ∈ Z^d and v ∈ Q(β)^d
// ===================================================================

inline QElem dot_qbeta(const std::vector<long long>& x, const QBetaVec& v,
                        const QBetaRing& R) {
    if (x.size() != v.size()) {
        throw std::invalid_argument("dot_qbeta: size mismatch");
    }
    if (x.size() != R.degree()) {
        throw std::invalid_argument("dot_qbeta: size != degree");
    }
    QElem sum = R.zero();
    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i] == 0) continue;
        QElem term = R.from_int(x[i]);
        term = R.mul(term, v[i]);
        sum = R.add(sum, term);
    }
    return sum;
}

// ===================================================================
// in_H_sigma: 0 ≤ <x, v> < v[j]
// ===================================================================
//
// x is an integer vector, v is the Q(β) eigenvector (with v[d-1] = 1
// conventionally, but not required).

inline bool in_h_sigma(const std::vector<long long>& x, std::size_t j,
                        const QBetaVec& v, const QBetaRing& R,
                        const RootInterval& beta_interval) {
    if (j >= v.size()) {
        throw std::invalid_argument("in_h_sigma: j out of range");
    }
    // <x, v>
    QElem x_dot_v = dot_qbeta(x, v, R);
    // sign(<x, v>)
    int sg_xv = qbeta_sign(x_dot_v, R, beta_interval);
    if (sg_xv < 0) return false;  // <x, v> < 0
    // v[j]
    int sg_vj = qbeta_sign(v[j], R, beta_interval);
    if (sg_vj <= 0) return false;  // v[j] <= 0 (shouldn't happen for Pisot v[j])
    // <x, v> - v[j]
    QElem diff = R.sub(x_dot_v, v[j]);
    int sg_diff = qbeta_sign(diff, R, beta_interval);
    if (sg_diff >= 0) return false;  // <x, v> >= v[j]
    return true;
}

// General-tile version of in_h_sigma: 0 <= <x, v> < <bound, v>, for an
// ARBITRARY integer bound vector (Eq 2.2 generalized from <e_j, v> to
// <f_j, v> for a non-unit-cube tile's face vector f_j). Setting
// bound = e_j (the j-th standard basis vector) reproduces in_h_sigma
// above exactly, since dot_qbeta(e_j, v, R) == v[j].
inline bool in_h_sigma_general_bound(const std::vector<long long>& x,
                                      const std::vector<long long>& bound,
                                      const QBetaVec& v, const QBetaRing& R,
                                      const RootInterval& beta_interval) {
    QElem x_dot_v = dot_qbeta(x, v, R);
    int sg_xv = qbeta_sign(x_dot_v, R, beta_interval);
    if (sg_xv < 0) return false;  // <x, v> < 0
    QElem bound_dot_v = dot_qbeta(bound, v, R);
    int sg_bound = qbeta_sign(bound_dot_v, R, beta_interval);
    if (sg_bound <= 0) return false;  // <bound, v> <= 0
    QElem diff = R.sub(x_dot_v, bound_dot_v);
    int sg_diff = qbeta_sign(diff, R, beta_interval);
    if (sg_diff >= 0) return false;  // <x, v> >= <bound, v>
    return true;
}

}  // namespace mathlib
