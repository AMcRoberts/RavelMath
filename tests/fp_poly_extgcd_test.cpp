// tests/fp_poly_extgcd_test.cpp
//
// Tests fp_extended_gcd (include/adelic/fp_poly.hpp): the extended
// Euclidean algorithm over F_p[x], returning Bezout coefficients
// s, t with s*a + t*b = gcd(a,b). Building block for multifactor
// Hensel lifting (docs/DIRECTION_AND_OPEN_THREADS.md Item B1) --
// local_polynomial_cofactor (include/adelic/local_field.hpp) originally
// handled only a single non-simple prime ideal via cofactor division.
// Its replacement uses this primitive's F_p Bezout certificate to lift
// one selected factor against the product of all remaining factors.
// This file verifies the certificate independently; the integration
// regression lives in zp_poly_extended_gcd_test.cpp.

#include <cstdio>
#include <vector>

#include "adelic/fp_poly.hpp"

using namespace adelic;

static int n_pass = 0, n_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; std::fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

namespace {

bool poly_eq(FpPoly a, FpPoly b) {
    while (a.c.size() > 1 && a.c.back() == 0) a.c.pop_back();
    while (b.c.size() > 1 && b.c.back() == 0) b.c.pop_back();
    return a.c == b.c;
}

void check_identity(const char* label, const FpPoly& a, const FpPoly& b) {
    auto r = fp_extended_gcd(a, b);
    FpPoly lhs = fp_add(fp_mul(r.s, a), fp_mul(r.t, b));
    FpPoly g_direct = fp_gcd(a, b);
    std::string msg1 = std::string(label) + ": s*a + t*b == gcd(a,b)";
    std::string msg2 = std::string(label) + ": gcd matches fp_gcd's independent computation";
    CHECK(poly_eq(lhs, r.g), msg1.c_str());
    CHECK(poly_eq(r.g, g_direct), msg2.c_str());
}

}  // namespace

int main() {
    // Coprime linear factors.
    check_identity("coprime linear factors mod 5",
                    FpPoly{5, {4, 1}}, FpPoly{5, {3, 1}});

    // Non-coprime: (x-1)(x-2) and (x-1)(x-3) mod 5, shared factor (x-1).
    FpPoly f1 = fp_mul(FpPoly{5, {4, 1}}, FpPoly{5, {3, 1}});
    FpPoly f2 = fp_mul(FpPoly{5, {4, 1}}, FpPoly{5, {2, 1}});
    check_identity("shared linear factor mod 5", f1, f2);

    // Coprime, different degrees.
    check_identity("degree 2 vs degree 3 mod 7",
                    FpPoly{7, {1, 0, 1}}, FpPoly{7, {2, 1, 0, 1}});

    // A handful of varied primes.
    for (long long p : {11LL, 13LL, 17LL, 101LL}) {
        FpPoly a{p, {2, 3, 1}};
        FpPoly b{p, {5, 0, 0, 1}};
        check_identity("varied prime", a, b);
    }

    // Case matching local_polynomial_cofactor's actual failure mode:
    // two coprime irreducible factors of different degree (e.g. one
    // linear, one quadratic irreducible mod p), the exact shape a
    // future multifactor lift needs to handle.
    FpPoly linear{5, {1, 1}};        // x + 1
    FpPoly quad_irred{5, {2, 1, 1}}; // x^2 + x + 2 (irreducible mod 5: no root in F_5)
    bool has_root = false;
    for (long long v = 0; v < 5; ++v) if (fp_eval(quad_irred, v) == 0) has_root = true;
    CHECK(!has_root, "quad_irred is genuinely irreducible mod 5 (sanity check on the test case itself)");
    check_identity("linear vs irreducible quadratic mod 5", linear, quad_irred);

    std::printf("\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
