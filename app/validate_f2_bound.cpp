// validate_f2_bound.cpp
//
// Direct sanity check of make_local_field_padic_bound for an f=2
// ideal (rndW3_12's shape: p=3, ideal (e=1,f=1) + ideal (e=1,f=2),
// charpoly x^3-4x^2-x-3), since the property-F node count came out
// IDENTICAL with and without the f=2 constraint (78 nodes either
// way) -- consistent with either "the f=1 bound already implies the
// f=2 one here" (fine) or "the f=2 bound silently accepts everything"
// (a bug). Test directly against values that MUST fail integrality
// at a genuine degree-2 residue-field place if the bound is doing
// anything real.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/validate_f2_bound.cpp math/out/libmath.a -o validate_f2_bound

#include <cstdio>
#include "math/qbeta.hpp"
#include "math/bezout.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/local_field.hpp"

int main() {
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-4, -1, -3});
    const mathlib::PolyZ& charpoly = R.charpoly();
    printf("charpoly: %s\n", mathlib::str(charpoly).c_str());

    auto fac = adelic::factor_prime_in_qbeta(charpoly, 3);
    for (auto& pi : fac.prime_ideals)
        printf("ideal: p=%lld e=%lld f=%lld g=%s\n", pi.p, pi.e, pi.f, mathlib::str(pi.g).c_str());

    // The f=2 ideal.
    auto bound_f2 = adelic::make_local_field_padic_bound(3, 1, 2, charpoly, 0, 30);
    // The f=1 ideal (for comparison).
    auto bound_f1 = adelic::make_local_field_padic_bound(3, 1, 1, charpoly, 0, 30);

    auto zero = R.from_int(0);
    auto one = R.from_int(1);
    mathlib::QElem beta = zero; beta.coeff(1) = mathlib::Rat(1, 1);
    auto inv = mathlib::invert_in_qbeta(beta, R);
    mathlib::QElem beta_inv = inv.inverse;  // 1/beta -- should generically NOT be integral everywhere

    // A value with an explicit 1/3 denominator -- should fail
    // integrality at ANY place above 3 (both the f=1 and f=2 ideal),
    // since 1/3 has negative valuation everywhere above 3.
    mathlib::QElem one_third = zero;
    one_third.coeff(0) = mathlib::Rat(1, 3);

    printf("\nbound_f1(0) = %s (expect true)\n", bound_f1(zero) ? "true" : "false");
    printf("bound_f2(0) = %s (expect true)\n", bound_f2(zero) ? "true" : "false");
    printf("bound_f1(1) = %s (expect true)\n", bound_f1(one) ? "true" : "false");
    printf("bound_f2(1) = %s (expect true)\n", bound_f2(one) ? "true" : "false");
    printf("bound_f1(1/3) = %s (expect FALSE)\n", bound_f1(one_third) ? "true" : "false");
    printf("bound_f2(1/3) = %s (expect FALSE)\n", bound_f2(one_third) ? "true" : "false");
    printf("bound_f1(1/beta) = %s\n", bound_f1(beta_inv) ? "true" : "false");
    printf("bound_f2(1/beta) = %s\n", bound_f2(beta_inv) ? "true" : "false");

    // A value built from the f=2 ideal's OWN generator, scaled by
    // 1/3 -- should fail integrality at the f=2 ideal specifically in
    // a way that isn't already implied by the f=1 ideal's own check
    // (a genuine differentiator between the two bounds, if they truly
    // differ). beta^2/3 is a reasonable such probe.
    mathlib::QElem beta2_over_3 = zero;
    beta2_over_3.coeff(2) = mathlib::Rat(1, 3);
    printf("bound_f1(beta^2/3) = %s\n", bound_f1(beta2_over_3) ? "true" : "false");
    printf("bound_f2(beta^2/3) = %s\n", bound_f2(beta2_over_3) ? "true" : "false");

    // THE differentiating test: the f=1 ideal has residue g=x+0, i.e.
    // beta itself is ALREADY in that ideal (v_{p1}(beta) >= 1), so
    // beta/3 should be INTEGRAL there (valuation >= 1 - 1 = 0). The
    // f=2 ideal's residue polynomial x^2+2x+2 has no root at 0 (its
    // constant term is 2, not 0), so beta is a UNIT there
    // (v_{p2}(beta) = 0), meaning beta/3 has valuation 0 - 1 = -1:
    // NOT integral. If the f=2 bound is doing anything real, this
    // MUST come out false while f1 comes out true.
    mathlib::QElem beta_over_3 = zero;
    beta_over_3.coeff(1) = mathlib::Rat(1, 3);
    printf("\nDIFFERENTIATING TEST -- beta/3:\n");
    printf("bound_f1(beta/3) = %s (expect TRUE)\n", bound_f1(beta_over_3) ? "true" : "false");
    printf("bound_f2(beta/3) = %s (expect FALSE)\n", bound_f2(beta_over_3) ? "true" : "false");

    // Numeric valuations, direct: build the two local fields and
    // print qp_local_valuation for beta itself, 3, and beta/3.
    long long residue_f1 = 0;
    adelic::ZpPoly m1 = adelic::local_polynomial_cofactor(charpoly, 3, 1, 1, 30, residue_f1);
    adelic::ZpPoly m2 = adelic::local_polynomial_cofactor(charpoly, 3, 1, 2, 30, 0);
    printf("\nm1 (f=1 local poly): %s\n", adelic::zp_poly_str(m1).c_str());
    printf("m2 (f=2 local poly): %s\n", adelic::zp_poly_str(m2).c_str());
    auto K1 = adelic::qp_local_field(charpoly, 3, 1, 1, m1, 30);
    auto K2 = adelic::qp_local_field(charpoly, 3, 1, 2, m2, 30);

    auto show = [&](const char* label, const mathlib::QElem& g) {
        auto z1 = adelic::qp_local_project(K1, g);
        auto z2 = adelic::qp_local_project(K2, g);
        long long v1 = adelic::qp_local_valuation(K1, z1);
        long long v2 = adelic::qp_local_valuation(K2, z2);
        printf("v_p1(%s) = %lld,  v_p2(%s) = %lld\n", label, v1, label, v2);
    };
    show("beta", beta);
    show("3", [&]{ auto x = zero; x.coeff(0) = mathlib::Rat(3,1); return x; }());

    // beta/3 as a raw rational-coefficient QElem threw inside
    // qp_local_project ("denominator not coprime to p") rather than
    // computing a local value -- test whether that's a genuine bug or
    // just an unsupported input SHAPE, by building the same value the
    // way real BFS gamma values actually arise: via beta^{-1}
    // multiplication (an algebraic-integer combination divided by
    // powers of beta), not a raw rational denominator.
    printf("\n(beta/3 as raw rational coeff threw in qp_local_project -- "
           "testing beta^{-1}*beta = 1 (an integer via the beta^{-1} route) instead:)\n");
    try {
        mathlib::QElem test_val = beta_inv;  // 1/beta, real beta^{-1} value
        auto z1 = adelic::qp_local_project(K1, test_val);
        auto z2 = adelic::qp_local_project(K2, test_val);
        printf("v_p1(1/beta) = %lld,  v_p2(1/beta) = %lld\n",
               adelic::qp_local_valuation(K1, z1), adelic::qp_local_valuation(K2, z2));
    } catch (const std::exception& e) {
        printf("EXCEPTION on 1/beta: %s\n", e.what());
    }
    try {
        auto z1 = adelic::qp_local_project(K1, beta_over_3);
        printf("beta/3 projected fine, v_p1 = %lld\n", adelic::qp_local_valuation(K1, z1));
    } catch (const std::exception& e) {
        printf("EXCEPTION on beta/3: %s\n", e.what());
    }

    return 0;
}
