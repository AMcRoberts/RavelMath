// tests/zp_poly_extended_gcd_test.cpp
//
// Tests zp_poly_extended_gcd and zp_poly_set_precision
// (include/adelic/local_field.hpp) -- the Z_p[x] analogue of
// fp_extended_gcd, building blocks for multifactor Hensel lifting
// (docs/DIRECTION_AND_OPEN_THREADS.md Item B1).
//
// Two things are verified here, precisely matching what's actually
// confirmed correct (not more):
//   1. zp_poly_extended_gcd in isolation, at precision 1 (mod p),
//      cross-checked against fp_extended_gcd's independent answer.
//      This caught a real bug during development: the first version
//      omitted normalizing the gcd to exactly 1, silently returning
//      Bezout coefficients for SOME nonzero constant multiple of the
//      identity instead.
//   2. ONE full quadratic Hensel lift round -- g,h lifted via the
//      standard formula, AND fresh Bezout coefficients re-derived via
//      zp_poly_extended_gcd on the newly-lifted g,h -- against a real
//      worked example: f(x)=x^2+4 over Z, which splits mod 5 as
//      (x-1)(x+1) and 5-adically as (x-a)(x+a) for a with a^2=-4.
//      Verified exactly mod p^2=25 by direct integer reconstruction.
//
// NOT yet verified (real, separate future work, not attempted here):
// iterating this across MULTIPLE rounds to reach arbitrary precision.
// A first attempt at that hit a further, distinct bug (only the
// single-round case below is confirmed) -- see
// .ravel/TODAY.md's B1 entries (2026-08-01) for the full, honest
// history of what was tried and what wasn't yet resolved.

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

    // ---- Part 2: one full Hensel lift round (g,h AND s,t together),
    // f(x) = x^2 + 4, splitting mod 5 as (x-1)(x+1). ----
    {
        long long p = 5, target_prec = 6;
        PolyZ f_z({4, 0, 1});
        ZpPoly f = zp_poly_from_polyz(f_z, p, target_prec);

        FpPoly g0_fp{p, {4, 1}}, h0_fp{p, {1, 1}};
        auto bez = fp_extended_gcd(g0_fp, h0_fp);
        bez.s = fp_divmod(bez.s, h0_fp).second;
        bez.t = fp_divmod(bez.t, g0_fp).second;

        ZpPoly g = fp_to_zp(g0_fp, target_prec);
        ZpPoly h = fp_to_zp(h0_fp, target_prec);
        ZpPoly s = fp_to_zp(bez.s, target_prec);
        ZpPoly t = fp_to_zp(bez.t, target_prec);

        // g,h lift step (the standard quadratic Hensel formula).
        ZpPoly e = zp_poly_sub(f, zp_poly_mul(g, h));
        ZpPoly se = zp_poly_mul(s, e);
        auto qr = zp_poly_divmod(se, h);
        ZpPoly delta_g = qr.second;
        ZpPoly delta_h = zp_poly_add(zp_poly_mul(t, e), zp_poly_mul(qr.first, g));
        ZpPoly g1 = zp_poly_add(g, delta_g);
        ZpPoly h1 = zp_poly_add(h, delta_h);

        long long modulus = 25;  // p^2, what one quadratic step promises
        ZpPoly check_gh = zp_poly_mul(g1, h1);
        bool ok_gh = true;
        for (std::size_t i = 0; i < f.coeffs.size(); ++i) {
            long long want = reconstruct_mod(f.coeffs[i], target_prec, modulus);
            long long got = i < check_gh.coeffs.size()
                                 ? reconstruct_mod(check_gh.coeffs[i], target_prec, modulus)
                                 : 0;
            if (want != got) ok_gh = false;
        }
        CHECK(ok_gh, "one Hensel round: g1*h1 == f exactly mod p^2");

        // Re-derive fresh Bezout coefficients for g1,h1 (truncated to
        // their actually-valid precision p^2 first).
        ZpPoly g1_trunc = zp_poly_set_precision(g1, 2);
        ZpPoly h1_trunc = zp_poly_set_precision(h1, 2);
        auto ext1 = zp_poly_extended_gcd(g1_trunc, h1_trunc);
        CHECK(zp_poly_degree(ext1.s) == 0, "re-derived s has the required degree bound (< deg(h)=1)");
        CHECK(zp_poly_degree(ext1.t) == 0, "re-derived t has the required degree bound (< deg(g)=1)");

        ZpPoly s1 = zp_poly_set_precision(ext1.s, target_prec);
        ZpPoly t1 = zp_poly_set_precision(ext1.t, target_prec);
        ZpPoly check_bez = zp_poly_add(zp_poly_mul(s1, g1), zp_poly_mul(t1, h1));
        bool ok_bez = true;
        for (std::size_t i = 0; i < check_bez.coeffs.size(); ++i) {
            long long want = (i == 0) ? 1 : 0;
            long long got = reconstruct_mod(check_bez.coeffs[i], target_prec, modulus);
            if (want != got) ok_bez = false;
        }
        CHECK(ok_bez, "one Hensel round: re-derived s1*g1 + t1*h1 == 1 exactly mod p^2");
    }

    std::printf("\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
