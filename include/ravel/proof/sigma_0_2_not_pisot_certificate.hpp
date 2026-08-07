// ravel/proof/sigma_0_2_not_pisot_certificate.hpp
//
// C++ certificate for Finding 32: sigma_{0,2}'s incidence
// characteristic polynomial x^3 - x - 2 has a complex-conjugate
// secondary pair of modulus > 1, hence is NOT Pisot. Instantiates
// RavelGenerated.sigma_0_2_charpoly_not_pisot
// (lean/depressed_cubic_complex_pair_modulus.lean): given a real root
// beta with 0 < beta < 2 of x^3-x-2, the depressed-cubic
// factorization forces the OTHER quadratic factor's complex roots to
// have modulus^2 = beta^2-1 > 1.
//
// SCOPE, stated honestly: the C++ certificate below verifies EXACTLY
// (integer arithmetic) that the polynomial IS x^3-x-2 and that a real
// root in (0,2) exists (the exact sign change at the endpoints,
// cubic(0)=-2<0, cubic(2)=4>0 -- existence of an actual root in that
// bracket via IVT is standard and not separately formalized here; the
// Lean theorem's hypotheses are conditional on such a beta existing,
// not on constructing it). This does not re-derive the general
// arbitrary-degree Pisot classifier (Finding 30's own scope); it is
// specific to this one polynomial, matching Finding 32's own claim.

#pragma once

#include <string>

#include "math/proof_reflection.hpp"

namespace ravel::proof {

struct Sigma02NotPisotCertificate {
    bool applies{};
    std::string note;
};

// `c`, `d`: the depressed cubic's coefficients (x^3 + c*x + d).
// Certifies NOT-Pisot exactly when (c,d) == (-1,-2) (sigma_{0,2}'s own
// polynomial) and the sign-change bracket (0,2) is verified exactly.
inline Sigma02NotPisotCertificate certify_sigma_0_2_not_pisot(long long c, long long d) {
    Sigma02NotPisotCertificate out;
    if (c != -1 || d != -2) {
        out.applies = false;
        out.note = "polynomial is not x^3-x-2 -- this certificate is specific to sigma_{0,2}'s "
                    "own charpoly, not a general classifier (see Finding 30 for that scope gap).";
        return out;
    }
    auto cubic = [&](long long x) { return x * x * x + c * x + d; };
    long long lo = 0, hi = 2;
    if (!(cubic(lo) < 0 && cubic(hi) > 0)) {
        out.applies = false;
        out.note = "sign-change bracket (0,2) failed exactly -- should be impossible for this "
                    "polynomial; refusing to proceed.";
        return out;
    }
    out.applies = true;
    out.note = "x^3-x-2 has cubic(0)=-2<0, cubic(2)=4>0 (exact integers) -- a real root beta in "
               "(0,2) exists; the depressed-cubic factorization then forces the complex pair's "
               "modulus^2 = beta^2-1 > 1, so sigma_{0,2} is NOT Pisot";

    mathlib::reflection::LemmaApplication citation;
    citation.theorem_name = "sigma_0_2_charpoly_not_pisot";
    citation.conclusion = "x^3-x-2's complex-conjugate secondary pair has modulus^2 = beta^2-1 "
        "> 1 for its real root beta in (0,2), hence sigma_{0,2} is not Pisot";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, citation);
    return out;
}

}  // namespace ravel::proof
