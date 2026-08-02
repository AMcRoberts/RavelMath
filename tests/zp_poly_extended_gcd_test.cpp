// tests/zp_poly_extended_gcd_test.cpp
//
// Tests zp_poly_extended_gcd and zp_poly_set_precision
// (include/adelic/local_field.hpp) -- the Z_p[x] analogue of
// fp_extended_gcd, building blocks for multifactor Hensel lifting
// (docs/DIRECTION_AND_OPEN_THREADS.md Item B1).
//
// Four things are verified here:
//   1. zp_poly_extended_gcd in isolation, at precision 1 (mod p),
//      cross-checked against fp_extended_gcd's independent answer.
//      This caught a real bug during development: the first version
//      omitted normalizing the gcd to exactly 1, silently returning
//      Bezout coefficients for SOME nonzero constant multiple of the
//      identity instead.
//   2. The corrected g,h assignment in zp_poly_hensel_lift_gh against
//      Moreno Maza's published worked example f=x^4-1 mod 25.  This is
//      the independent example that exposed the old reversed assignment.
//   3. The production linear factor-pair lift through 8 p-adic digits
//      for two differently structured examples: x^2+4 over p=5 and
//      x^2-3 over p=11. Both products are checked coefficientwise in
//      Z_p[x], not through a floating approximation.
//   4. The actual local_polynomial_cofactor integration on a quartic
//      having two non-simple factors over the same prime: one (e=1,f=2)
//      and one (e=2,f=1).  Each requested local polynomial has degree 2
//      and their product reconstructs the quartic at full precision.

#include <cstdio>
#include <vector>

#include "adelic/fp_poly.hpp"
#include "adelic/local_field.hpp"
#include "math/poly_z.hpp"

using namespace adelic;
using mathlib::PolyZ;

static int n_pass = 0, n_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; std::fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

namespace {

ZpPoly fp_to_zp(const FpPoly& g, long long prec) {
    ZpPoly r = zp_poly_zero(g.p, prec);
    r.coeffs.assign(g.c.size(), zp_zero(g.p, prec));
    for (std::size_t i = 0; i < g.c.size(); ++i) {
        r.coeffs[i] = zp_from_int_full(g.p, g.c[i], prec);
    }
    return r;
}

long long reconstruct_mod(const ZpInt& a, long long target_digits, long long modulus) {
    long long p = a.p, val = 0, pk = 1;
    for (long long i = 0; i < target_digits && i < static_cast<long long>(a.digits.size()); ++i) {
        val += a.digits[static_cast<std::size_t>(i)] * pk;
        pk *= p;
    }
    val %= modulus;
    if (val < 0) val += modulus;
    return val;
}

bool zp_int_eq_first_digit(const ZpInt& a, long long expected) {
    return !a.digits.empty() && a.digits[0] == ((expected % a.p) + a.p) % a.p;
}

bool factorization_holds(const ZpPoly& f, const ZpPoly& g, const ZpPoly& h) {
    return zp_poly_equal(zp_poly_mul(g, h), f);
}

}  // namespace

int main() {
    // ---- Part 1: isolated GCD at precision 1, cross-checked against
    // fp_extended_gcd's independent answer. ----
    {
        long long p = 5, prec = 1;
        ZpPoly g = zp_poly_zero(p, prec);
        g.coeffs.assign(2, zp_zero(p, prec));
        g.coeffs[0] = zp_from_int_full(p, 4, prec);  // x + 4  (i.e. x - 1 mod 5)
        g.coeffs[1] = zp_from_int_full(p, 1, prec);
        ZpPoly h = zp_poly_zero(p, prec);
        h.coeffs.assign(2, zp_zero(p, prec));
        h.coeffs[0] = zp_from_int_full(p, 1, prec);  // x + 1
        h.coeffs[1] = zp_from_int_full(p, 1, prec);

        auto ext = zp_poly_extended_gcd(g, h);
        CHECK(zp_poly_degree(ext.g) == 0 && zp_int_eq_first_digit(ext.g.coeffs[0], 1),
              "isolated gcd: gcd(g,h) is exactly 1 (normalized, not just some nonzero constant)");

        FpPoly g_fp{p, {4, 1}}, h_fp{p, {1, 1}};
        auto fp_ext = fp_extended_gcd(g_fp, h_fp);
        CHECK(zp_int_eq_first_digit(ext.s.coeffs[0], fp_ext.s.c[0]),
              "isolated gcd: s matches fp_extended_gcd's independent answer");
        CHECK(zp_int_eq_first_digit(ext.t.coeffs[0], fp_ext.t.c[0]),
              "isolated gcd: t matches fp_extended_gcd's independent answer");

        ZpPoly check = zp_poly_add(zp_poly_mul(ext.s, g), zp_poly_mul(ext.t, h));
        CHECK(zp_poly_degree(check) == 0 && zp_int_eq_first_digit(check.coeffs[0], 1),
              "isolated gcd: s*g + t*h == 1 exactly");
    }

    // ---- Part 2: reproduce the published one-round example exactly.
    // f=x^4-1, g=x-2, h=x^3+2x^2-x-2 (mod 5). The corrected lift is
    // g1=x+18, h1=x^3+7x^2+24x+18 (mod 25). ----
    {
        const long long p = 5;
        PolyZ f_z({-1, 0, 0, 0, 1});
        FpPoly g_fp{p, {3, 1}};
        FpPoly h_fp{p, {3, 4, 2, 1}};
        auto bez = fp_extended_gcd(g_fp, h_fp);
        ZpPoly f = zp_poly_from_polyz(f_z, p, 2);
        auto lifted = zp_poly_hensel_lift_gh(
            f, fp_to_zp(g_fp, 2), fp_to_zp(h_fp, 2),
            fp_to_zp(bez.s, 2), fp_to_zp(bez.t, 2));
        CHECK(factorization_holds(f, lifted.first, lifted.second),
              "published example: corrected g1*h1 == f mod 25");
        CHECK(reconstruct_mod(lifted.first.coeffs[0], 2, 25) == 18 &&
              reconstruct_mod(lifted.first.coeffs[1], 2, 25) == 1,
              "published example: g1 == x+18 mod 25");
        const long long expected_h[] = {18, 24, 7, 1};
        bool h_matches = lifted.second.coeffs.size() == 4;
        for (std::size_t i = 0; h_matches && i < 4; ++i) {
            h_matches = reconstruct_mod(lifted.second.coeffs[i], 2, 25) == expected_h[i];
        }
        CHECK(h_matches, "published example: h1 == x^3+7x^2+24x+18 mod 25");
    }

    // ---- Part 3: the production linear lift on two independent cases. ----
    {
        const PolyZ f5({4, 0, 1});
        auto lift5 = zp_poly_hensel_lift_factor_pair(
            f5, FpPoly{5, {4, 1}}, FpPoly{5, {1, 1}}, 8);
        CHECK(factorization_holds(zp_poly_from_polyz(f5, 5, 8),
                                  lift5.first, lift5.second),
              "linear lift: x^2+4 factors correctly through precision 8 over Z_5");

        const PolyZ f11({-3, 0, 1});
        auto lift11 = zp_poly_hensel_lift_factor_pair(
            f11, FpPoly{11, {6, 1}}, FpPoly{11, {5, 1}}, 8);
        CHECK(factorization_holds(zp_poly_from_polyz(f11, 11, 8),
                                  lift11.first, lift11.second),
              "linear lift: x^2-3 factors correctly through precision 8 over Z_11");
    }

    // ---- Part 4: the multi-non-simple-ideal seam that the old
    // cofactor-of-simple-factors implementation could not separate.
    // Mod 3:
    //   f = (x^2+1) (x-1)^2,
    // giving distinct (e=1,f=2) and (e=2,f=1) targets. ----
    {
        const long long p = 3, precision = 8;
        PolyZ f_z({4, -2, 2, -2, 1});
        ZpPoly quadratic = local_polynomial_cofactor(
            f_z, p, /*e=*/1, /*f=*/2, precision);
        ZpPoly ramified_linear = local_polynomial_cofactor(
            f_z, p, /*e=*/2, /*f=*/1, precision, /*residue_a=*/1);
        CHECK(zp_poly_degree(quadratic) == 2,
              "two non-simple factors: (e=1,f=2) target has degree 2");
        CHECK(zp_poly_degree(ramified_linear) == 2,
              "two non-simple factors: (e=2,f=1) target has degree 2");
        ZpPoly f = zp_poly_from_polyz(f_z, p, precision);
        CHECK(factorization_holds(f, quadratic, ramified_linear),
              "two non-simple factors: independently requested locals reconstruct f");
    }

    std::printf("\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
