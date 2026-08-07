// ravel/proof/depressed_cubic_not_pisot_certificate.hpp
//
// GENERALIZED replacement for the earlier sigma_0_2-specific
// certificate: works on ANY depressed cubic x^3+c*x+d, not one
// hardcoded polynomial. This file does NOT assert truth itself -- it
// stages exact integer data (c, d, an exact sign-change bracket) into
// a reflection trace node. The actual proof is whatever Lean text the
// renderer emits from that node and `lake env lean` kernel-checks;
// nothing here should be read as a claim on its own. No boolean this
// file returns is a substitute for the kernel check.
//
// Instantiates RavelGenerated.depressed_cubic_q_gt_one_iff_beta_lt_neg_d
// (lean/depressed_cubic_complex_pair_modulus.lean): for a positive
// real root beta of x^3+c*x+d, the complex pair's modulus^2 exceeds 1
// iff beta < -d. Given an exact bracket (lo,hi) with cubic(lo)<0<cubic(hi)
// and hi <= -d (checked exactly, integers only), the root is forced
// below -d, so the iff's right side holds for ANY beta in (lo,hi) --
// the renderer states this as a hypothesis-carrying corollary, not as
// an unconditional fact about a specific numeric beta (this file never
// constructs beta).

#pragma once

#include "math/proof_reflection.hpp"

namespace ravel::proof {

// `c`, `d`: depressed cubic coefficients. `lo`, `hi`: an exact integer
// bracket the caller supplies. Records a reflection node ONLY if the
// bracket is verified exactly (integer arithmetic: cubic(lo)<0<cubic(hi),
// and hi<=-d so the discriminant/modulus conclusion is available to
// the renderer without further search). Returns nothing -- there is
// no "did this succeed" boolean to trust; check the recorded trace
// and the rendered Lean's kernel-check result instead.
inline void stage_depressed_cubic_not_pisot(long long c, long long d, long long lo, long long hi) {
    auto cubic = [&](long long x) { return x * x * x + c * x + d; };
    if (!(cubic(lo) < 0 && cubic(hi) > 0)) return;   // bracket not verified; stage nothing
    if (!(hi <= -d)) return;                          // insufficient to force beta < -d; stage nothing
    if (!mathlib::reflection::enabled()) return;

    mathlib::reflection::DepressedCubicNotPisotCertificate node;
    node.c = c;
    node.d = d;
    node.lo = lo;
    node.hi = hi;
    node.description = "x^3" + (c >= 0 ? "+" + std::to_string(c) : std::to_string(c)) + "x"
        + (d >= 0 ? "+" + std::to_string(d) : std::to_string(d))
        + ", bracket (" + std::to_string(lo) + "," + std::to_string(hi) + ")";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}  // namespace ravel::proof
