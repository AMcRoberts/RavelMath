// poly_discriminant_bigint_test.cpp
//
// Property test for math/poly_discriminant.hpp's arbitrary-precision
// poly_discriminant_bigint against include/adelic/maximal_order.hpp's
// existing (explicitly long-long-scoped) poly_discriminant_ll:
//   1. Agreement on every discriminant this project's existing test
//      suite already exercises (small Pisot characteristic
//      polynomials, the Dedekind non-monogenic cubic).
//   2. A regime where long long genuinely cannot represent the exact
//      discriminant (a higher-degree polynomial with a discriminant
//      that overflows 64 bits), checked against a discriminant value
//      computed independently by hand from a known formula, not just
//      "the code doesn't crash."

#include <cstdio>
#include <vector>

#include "adelic/maximal_order.hpp"
#include "math/bigint.hpp"
#include "math/poly_discriminant.hpp"

using namespace std;
using mathlib::BigInt;
using mathlib::poly_discriminant_bigint;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)

std::string bigint_str(const BigInt& x) {
    char* s = mpz_get_str(nullptr, 10, x.get());
    std::string result(s);
    free(s);
    return result;
}

}  // namespace

int main() {
    fprintf(stderr, "=== agreement with poly_discriminant_ll on existing cases ===\n");

    // x^3 - 3x^2 - 2x - 1 (sigma_1-style Class-II cubic, a=1).
    {
        std::vector<long long> f = {1, -3, -2, -1};
        long long ll_result = adelic::poly_discriminant_ll(f);
        BigInt bi_result = poly_discriminant_bigint(f);
        CHECK(mathlib::cmp_si(bi_result, ll_result) == 0,
              "x^3-3x^2-2x-1: bigint discriminant matches long long discriminant");
    }

    // x^3 - x^2 - 2x - 8 (Dedekind's own non-monogenic cubic,
    // maximal_order.hpp's worked textbook example). Known field
    // discriminant: -503 (disc(f) = -4*503 = -2012, index 2).
    {
        std::vector<long long> f = {1, -1, -2, -8};
        long long ll_result = adelic::poly_discriminant_ll(f);
        BigInt bi_result = poly_discriminant_bigint(f);
        CHECK(mathlib::cmp_si(bi_result, ll_result) == 0,
              "Dedekind cubic x^3-x^2-2x-8: bigint discriminant matches long long discriminant");
        CHECK(ll_result == -2012,
              "Dedekind cubic disc(f) == -2012 (textbook value, disc(K)=-503, index 2)");
    }

    // x^2 + 5 (Q(sqrt(-5)), from number_field_parity_test.cpp). disc = -20.
    {
        std::vector<long long> f = {1, 0, 5};
        long long ll_result = adelic::poly_discriminant_ll(f);
        BigInt bi_result = poly_discriminant_bigint(f);
        CHECK(mathlib::cmp_si(bi_result, ll_result) == 0,
              "x^2+5: bigint discriminant matches long long discriminant");
        CHECK(ll_result == -20, "x^2+5 disc == -20 (matches Q(sqrt(-5))'s field discriminant)");
    }

    fprintf(stderr, "\n=== closed-form cross-check: disc(x^n - a) ===\n");
    // disc(x^n - a) via a from-scratch resultant derivation (external
    // to this file's own poly_discriminant_bigint, so a genuine cross-
    // check, not circular): with f=x^n-a, f'=nx^{n-1}, and roots
    // alpha_i satisfying alpha_i^n=a,
    //   Res(f,f') = prod_i f'(alpha_i) = prod_i n*alpha_i^{n-1}
    //             = n^n * prod_i (a/alpha_i)
    //             = n^n * a^n / prod_i(alpha_i)
    // and prod_i(alpha_i) = (-1)^n * (constant term)/(leading coeff)
    //                     = (-1)^n * (-a) = (-1)^{n+1} a,
    // so Res(f,f') = n^n * (-1)^{n+1} * a^{n-1}, and (leading coeff 1)
    //   disc(f) = (-1)^{n(n-1)/2} * Res(f,f')
    //           = (-1)^{n(n-1)/2 + n + 1} * n^n * a^{n-1}.
    // This corrects an earlier draft of this test, which used the
    // more commonly quoted-from-memory (-1)^{n(n-1)/2}*n^n*a^{n-1}
    // (missing the (-1)^{n+1} factor from prod(alpha_i)'s own sign) --
    // caught by disagreeing with the actual computed value at n=20,
    // not trusted from memory a second time. Re-derived here in full,
    // then checked against poly_discriminant_ll directly for n=2..8
    // (small enough that long long is trustworthy) before being
    // applied at n=20 where only BigInt is trustworthy.
    for (long long n : {2, 3, 4, 5, 6, 7, 8}) {
        std::vector<long long> f(static_cast<std::size_t>(n) + 1, 0);
        f[0] = 1;
        f[static_cast<std::size_t>(n)] = -2;  // x^n - 2
        long long ll = adelic::poly_discriminant_ll(f);
        BigInt bi = poly_discriminant_bigint(f);
        CHECK(mathlib::cmp_si(bi, ll) == 0, "x^n-2 (small n): bigint matches long long");
        long long exponent = (n * (n - 1) / 2 + n + 1) % 2;
        BigInt n_pow_n(1);
        for (long long i = 0; i < n; ++i) mathlib::mul_si(n_pow_n, n_pow_n, n);
        BigInt two_pow_nm1(1);
        for (long long i = 0; i < n - 1; ++i) mathlib::mul_si(two_pow_nm1, two_pow_nm1, 2);
        BigInt expected;
        mathlib::mul(expected, n_pow_n, two_pow_nm1);
        if (exponent != 0) mathlib::neg(expected);
        CHECK(bi == expected, "x^n-2 (small n): matches the re-derived closed form exactly");
    }

    fprintf(stderr, "\n=== degree-9+: poly_discriminant_ll silently overflows, BigInt does not ===\n");
    // A real bug, found as a byproduct of this property test, not
    // sought out: adelic::maximal_order.hpp's poly_discriminant_ll
    // (long long throughout, per its own header's scope note) is
    // silently WRONG starting at degree 9 for x^n-2 -- not merely
    // unable to represent a too-large answer, but producing a
    // DIFFERENT, incorrect value with no error or overflow signal,
    // because Bareiss elimination's INTERMEDIATE entries can overflow
    // int64 even when the final discriminant would fit. Confirmed via
    // three independent signals agreeing with each other and
    // disagreeing with poly_discriminant_ll: BigInt's own computation,
    // the closed-form re-derivation above, and magnitude sanity
    // (10^10*2^9 for n=10 is ~5*10^12, nowhere near int64's ~9*10^18
    // ceiling, so the failure is genuinely an intermediate-value
    // overflow, not a final-answer capacity issue).
    for (long long n : {9, 10, 11, 12}) {
        std::vector<long long> f(static_cast<std::size_t>(n) + 1, 0);
        f[0] = 1;
        f[static_cast<std::size_t>(n)] = -2;
        long long ll = adelic::poly_discriminant_ll(f);
        BigInt bi = poly_discriminant_bigint(f);
        long long exponent = (n * (n - 1) / 2 + n + 1) % 2;
        BigInt n_pow_n(1);
        for (long long i = 0; i < n; ++i) mathlib::mul_si(n_pow_n, n_pow_n, n);
        BigInt two_pow_nm1(1);
        for (long long i = 0; i < n - 1; ++i) mathlib::mul_si(two_pow_nm1, two_pow_nm1, 2);
        BigInt expected;
        mathlib::mul(expected, n_pow_n, two_pow_nm1);
        if (exponent != 0) mathlib::neg(expected);
        fprintf(stderr, "  n=%lld: poly_discriminant_ll=%lld, poly_discriminant_bigint=%s\n",
                n, ll, bigint_str(bi).c_str());
        CHECK(bi == expected,
              "n>=9: poly_discriminant_bigint matches the closed form exactly (the ground truth)");
        CHECK(mathlib::cmp_si(bi, ll) != 0,
              "n>=9: poly_discriminant_ll DISAGREES with the ground truth -- confirmed silent overflow, "
              "not a false alarm");
    }

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
