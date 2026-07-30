// tests/dedekind_factorization_test.cpp
//
// Self-test for include/adelic/dedekind_factorization.hpp.
//
// Tests:
//   1. F_p[x] polynomial arithmetic (add, sub, mul, divmod, eval, gcd)
//   2. F_p[x] factoring (root extraction + irreducible leftover)
//   3. Dedekind factorization on the worked example
//      (σ(1)=1113, σ(2)=11, σ(3)=2, char poly x^3 - 3x^2 - 2,
//      prime p=2): expect (2) = p_1^2 · p_2 with p_1 = (2, β),
//      e=2, f=1 and p_2 = (2, β+1), e=1, f=1
//   4. Dedekind factorization on a few other small cases

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "adelic/dedekind_factorization.hpp"
#include "math/bigint.hpp"
#include "math/poly_z.hpp"

using namespace std;
using adelic::FpPoly;
using adelic::fp_add;
using adelic::fp_sub;
using adelic::fp_mul;
using adelic::fp_divmod;
using adelic::fp_eval;
using adelic::fp_gcd;
using adelic::fp_linear;
using adelic::factor_fp;
using adelic::factor_prime_in_qbeta;
using adelic::reduce_to_fp;
using adelic::lift_fp_factor_to_z;
using mathlib::PolyZ;
using mathlib::PolyQ;
using mathlib::set_si;
using mathlib::str;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define CHECK_EQ_INT(a, b, msg) do { \
    long long _a = (long long)(a); \
    long long _b = (long long)(b); \
    if (_a == _b) { ++n_pass; } \
    else { ++n_fail; fprintf(stderr, "FAIL: %s (got %lld, expected %lld)\n", (msg), _a, _b); } \
} while (0)

void print_fp(const char* label, const FpPoly& f) {
    fprintf(stderr, "  %s (p=%lld): [", label, f.p);
    for (std::size_t i = 0; i < f.c.size(); ++i) {
        if (i > 0) fprintf(stderr, ", ");
        fprintf(stderr, "%lld", f.c[i]);
    }
    fprintf(stderr, "]\n");
}

void test_fp_arithmetic() {
    fprintf(stderr, "=== test_fp_arithmetic ===\n");
    // F_5[x] polynomials
    FpPoly a; a.p = 5; a.c = {1, 2};  // 1 + 2x
    FpPoly b; b.p = 5; b.c = {3, 4};  // 3 + 4x

    // a + b = 4 + 6x = 4 + 1x (mod 5)
    FpPoly s = fp_add(a, b);
    CHECK_EQ_INT(s.c[0], 4, "(1+2x) + (3+4x) in F_5: constant = 4");
    CHECK_EQ_INT(s.c[1], 1, "(1+2x) + (3+4x) in F_5: x-coeff = 6 mod 5 = 1");

    // a - b = -2 - 2x = 3 + 3x (mod 5)
    FpPoly d = fp_sub(a, b);
    CHECK_EQ_INT(d.c[0], 3, "(1+2x) - (3+4x) in F_5: constant = -2 mod 5 = 3");
    CHECK_EQ_INT(d.c[1], 3, "(1+2x) - (3+4x) in F_5: x-coeff = -2 mod 5 = 3");

    // a * b = 3 + 10x + 8x^2 = 3 + 0x + 3x^2 (mod 5)
    FpPoly p = fp_mul(a, b);
    CHECK_EQ_INT(p.c[0], 3, "(1+2x) * (3+4x) in F_5: constant = 3");
    CHECK_EQ_INT(p.c[1], 0, "(1+2x) * (3+4x) in F_5: x-coeff = 10 mod 5 = 0");
    CHECK_EQ_INT(p.c[2], 3, "(1+2x) * (3+4x) in F_5: x^2-coeff = 8 mod 5 = 3");

    // Evaluate
    CHECK_EQ_INT(fp_eval(a, 0), 1, "1 + 2*0 = 1");
    CHECK_EQ_INT(fp_eval(a, 1), 3, "1 + 2*1 = 3");
    CHECK_EQ_INT(fp_eval(a, 2), 0, "1 + 2*2 = 5 mod 5 = 0");
    CHECK_EQ_INT(fp_eval(a, 3), 2, "1 + 2*3 = 7 mod 5 = 2");
}

void test_fp_divmod() {
    fprintf(stderr, "=== test_fp_divmod ===\n");
    // (x^2 + 1) / (x + 1) in F_5: x^2 + 1 = (x - 1)(x + 1) + 2 in F_5
    // Wait, x^2 - 1 = (x-1)(x+1), so x^2 + 1 = (x-1)(x+1) + 2.  Hmm.
    // Actually x^2 + 1 in F_5: at x=1, 1+1=2; at x=4 (-1), 16+1=2.  So 1 and 4 are roots.
    // x^2 + 1 = (x-1)(x-4) = (x-1)(x+1) (since -4 = 1 mod 5)
    // Wait, -1 = 4 mod 5, not 1.  Let me recompute: (x-1)(x-4) = x^2 - 5x + 4 = x^2 + 4 in F_5.
    // That's not x^2 + 1.
    // Let me just check: (x-1)(x+1) = x^2 - 1.  In F_5, x^2 + 1 = x^2 - 4.  Hmm.
    // Actually in F_5, -1 = 4, so x^2 - 1 = x^2 + 4.  And (x-1)(x+1) = x^2 + 4.  Hmm.
    // Wait, (x - 1)(x - 4) = x^2 - 5x + 4 = x^2 + 4 in F_5 (since 5x = 0).  And 4 = -1.
    // So x^2 - 1 = (x-1)(x-4) in F_5.  And x^2 + 1 = x^2 - 4 = (x-2)(x+2) in F_5.
    // Hmm, let me just do the division algorithm by hand.
    FpPoly a; a.p = 5; a.c = {1, 0, 1};  // 1 + x^2
    FpPoly b; b.p = 5; b.c = {1, 1};  // 1 + x
    auto dm = fp_divmod(a, b);
    print_fp("a", a);
    print_fp("b", b);
    print_fp("q = a / b", dm.first);
    print_fp("r = a mod b", dm.second);
    // a = x^2 + 1, b = x + 1
    // q should be x - 1 (since (x-1)(x+1) = x^2 - 1 ≠ x^2 + 1)
    // Hmm actually let me check: (x - 1)(x + 1) = x^2 - 1, so x^2 + 1 = (x-1)(x+1) + 2
    // So q = x - 1 (mod 5 = x + 4), r = 2.
    CHECK_EQ_INT(dm.first.c[0], 4, "(x^2+1)/(x+1) in F_5: q constant = -1 mod 5 = 4");
    CHECK_EQ_INT(dm.first.c[1], 1, "(x^2+1)/(x+1) in F_5: q x-coeff = 1");
    CHECK_EQ_INT(dm.second.c[0], 2, "(x^2+1)/(x+1) in F_5: r = 2");
}

void test_fp_gcd() {
    fprintf(stderr, "=== test_fp_gcd ===\n");
    // gcd(x^2 + 1, x + 1) in F_5: should be 1 (since x^2 + 1 = (x-1)(x+1) + 2, and
    // x + 1 doesn't divide x^2 + 1 mod 5)
    FpPoly a; a.p = 5; a.c = {1, 0, 1};
    FpPoly b; b.p = 5; b.c = {1, 1};
    FpPoly g = fp_gcd(a, b);
    print_fp("gcd", g);
    CHECK_EQ_INT(g.c[0], 1, "gcd should be 1 (constant)");

    // gcd(x^2 - 1, x + 1) in F_5: should be x + 1
    FpPoly c; c.p = 5; c.c = {4, 0, 1};  // -1 + x^2 = 4 + x^2
    FpPoly d; d.p = 5; d.c = {1, 1};
    FpPoly g2 = fp_gcd(c, d);
    print_fp("gcd(x^2-1, x+1)", g2);
    CHECK_EQ_INT(g2.c[1], 1, "gcd should have x-coeff = 1 (monic)");
    CHECK_EQ_INT(g2.c[0], 1, "gcd should have constant = 1 (so it's x+1)");
}

void test_factor_fp() {
    fprintf(stderr, "=== test_factor_fp ===\n");
    // Factor x^2 + 1 in F_5: should be (x - 2)(x + 2) = (x + 3)(x + 2)
    // (since -2 = 3, +2 = 2)
    FpPoly f; f.p = 5; f.c = {1, 0, 1};
    auto facs = factor_fp(f);
    fprintf(stderr, "  x^2 + 1 in F_5: factors found = %zu\n", facs.size());
    for (const auto& fac : facs) {
        print_fp("  factor", fac.g);
        fprintf(stderr, "    mult = %lld\n", fac.mult);
    }
    CHECK_EQ_INT(facs.size(), 2, "x^2 + 1 in F_5 should have 2 factors");
    // Each factor should be (x - 2) or (x + 2)
    bool has_2 = false, has_neg_2 = false;
    for (const auto& fac : facs) {
        if (fac.g.c[0] == 2 && fac.g.c[1] == 1) has_2 = true;
        if (fac.g.c[0] == 3 && fac.g.c[1] == 1) has_neg_2 = true;
    }
    CHECK(has_2, "should have factor (x - 2) i.e. (x + 3)");
    CHECK(has_neg_2, "should have factor (x + 2) i.e. (x + 2)");

    // Factor x^3 - 3x^2 - 2 in F_2: should be x^2 (x + 1)
    FpPoly g; g.p = 2; g.c = {0, 0, 1, 1};  // -2 + 0x - 3x^2 + x^3 = 0 + 0x + x^2 + x^3
    auto facs2 = factor_fp(g);
    fprintf(stderr, "  x^3 - 3x^2 - 2 in F_2: factors found = %zu\n", facs2.size());
    for (const auto& fac : facs2) {
        print_fp("  factor", fac.g);
        fprintf(stderr, "    mult = %lld\n", fac.mult);
    }
    CHECK_EQ_INT(facs2.size(), 2, "x^3 - 3x^2 - 2 in F_2 should have 2 factors");
    // Should be x with mult 2 and (x+1) with mult 1
    bool has_x_mult2 = false, has_xp1 = false;
    for (const auto& fac : facs2) {
        if (fac.g.c.size() == 2 && fac.g.c[0] == 0 && fac.g.c[1] == 1 && fac.mult == 2) has_x_mult2 = true;
        if (fac.g.c.size() == 2 && fac.g.c[0] == 1 && fac.g.c[1] == 1 && fac.mult == 1) has_xp1 = true;
    }
    CHECK(has_x_mult2, "should have factor x with mult 2");
    CHECK(has_xp1, "should have factor (x+1) with mult 1");
}

void test_factorize_worked_example() {
    fprintf(stderr, "=== test_factorize_worked_example ===\n");
    // Char poly of the worked example: x^3 - 3x^2 - 2
    // PolyZ convention: coeffs_[i] = coefficient of x^i.
    //   coeffs_[0] = -2 (constant)
    //   coeffs_[1] =  0
    //   coeffs_[2] = -3 (x^2)
    //   coeffs_[3] =  1 (leading)
    // So ensure_size(4).
    PolyZ charpoly;
    charpoly.ensure_size(4);
    set_si(charpoly.coeff(0), -2);
    set_si(charpoly.coeff(1), 0);
    set_si(charpoly.coeff(2), -3);
    set_si(charpoly.coeff(3), 1);
    fprintf(stderr, "  charpoly: %s\n", str(charpoly).c_str());

    // Factor p = 2
    auto fac = factor_prime_in_qbeta(charpoly, 2);
    fprintf(stderr, "  p = 2: %zu prime ideals\n", fac.prime_ideals.size());
    for (const auto& pi : fac.prime_ideals) {
        fprintf(stderr, "    p = %lld, e = %lld, f = %lld, g = %s\n",
                pi.p, pi.e, pi.f, str(pi.g).c_str());
    }
    CHECK_EQ_INT(fac.prime_ideals.size(), 2, "p=2 should give 2 prime ideals");
    CHECK_EQ_INT(fac.p, 2, "factor returned p=2");

    // p_1: g_1 = x, e=2, f=1
    bool found_p1 = false, found_p2 = false;
    for (const auto& pi : fac.prime_ideals) {
        if (pi.e == 2 && pi.f == 1 && pi.g.degree() == 1) {
            // Check g_1 = x (constant = 0, leading = 1)
            if (mpz_cmp_ui(pi.g.coeff(0).get(), 0) == 0 &&
                mpz_cmp_ui(pi.g.coeff(1).get(), 1) == 0) found_p1 = true;
        }
        if (pi.e == 1 && pi.f == 1 && pi.g.degree() == 1) {
            // Check g_2 = x + 1 (constant = 1, leading = 1)
            if (mpz_cmp_ui(pi.g.coeff(0).get(), 1) == 0 &&
                mpz_cmp_ui(pi.g.coeff(1).get(), 1) == 0) found_p2 = true;
        }
    }
    CHECK(found_p1, "p_1 should be (2, β) with e=2, f=1, g=x");
    CHECK(found_p2, "p_2 should be (2, β+1) with e=1, f=1, g=x+1");
}

void test_factorize_fully_ramified() {
    fprintf(stderr, "=== test_factorize_fully_ramified ===\n");
    // Char poly x^2 - 2 (so β = √2, totally ramified at p=2)
    // PolyZ convention: coeffs_[0] = -2 (constant), coeffs_[1] = 0,
    // coeffs_[2] = 1 (leading).  So ensure_size(3).
    PolyZ charpoly;
    charpoly.ensure_size(3);
    set_si(charpoly.coeff(0), -2);
    set_si(charpoly.coeff(1), 0);
    set_si(charpoly.coeff(2), 1);
    fprintf(stderr, "  charpoly: %s\n", str(charpoly).c_str());

    auto fac = factor_prime_in_qbeta(charpoly, 2);
    fprintf(stderr, "  p = 2: %zu prime ideals\n", fac.prime_ideals.size());
    for (const auto& pi : fac.prime_ideals) {
        fprintf(stderr, "    p = %lld, e = %lld, f = %lld, g = %s\n",
                pi.p, pi.e, pi.f, str(pi.g).c_str());
    }
    // Expect one prime ideal: p_1 = (2, β) with e=2, f=1, g=x
    CHECK_EQ_INT(fac.prime_ideals.size(), 1, "x^2 - 2 in F_2 should give 1 prime ideal");
    CHECK_EQ_INT(fac.prime_ideals[0].e, 2, "fully ramified: e=2");
    CHECK_EQ_INT(fac.prime_ideals[0].f, 1, "fully ramified: f=1");
    CHECK(mpz_cmp_ui(fac.prime_ideals[0].g.coeff(0).get(), 0) == 0,
          "fully ramified: g=x, constant = 0");
}

void test_factorize_unramified() {
    fprintf(stderr, "=== test_factorize_unramified ===\n");
    // Char poly x^2 + 1 (so β = i).  This is the Q(i) number field.
    // The splitting behavior at p is determined by the Legendre
    // symbol (-1/p): p splits iff p ≡ 1 mod 4, stays inert iff
    // p ≡ 3 mod 4, and ramifies iff p = 2.
    // PolyZ convention: coeffs_[0] = 1 (constant), coeffs_[1] = 0,
    // coeffs_[2] = 1 (leading).
    PolyZ charpoly;
    charpoly.ensure_size(3);
    set_si(charpoly.coeff(0), 1);
    set_si(charpoly.coeff(1), 0);
    set_si(charpoly.coeff(2), 1);
    fprintf(stderr, "  charpoly: %s\n", str(charpoly).c_str());

    // p=2: x^2 + 1 mod 2 = (x+1)^2 (since 1 = -1 in F_2).  Ramified.
    //   1 prime ideal with e=2, f=1.
    auto fac2 = factor_prime_in_qbeta(charpoly, 2);
    fprintf(stderr, "  p = 2: %zu prime ideals\n", fac2.prime_ideals.size());
    CHECK_EQ_INT(fac2.prime_ideals.size(), 1, "x^2 + 1 in F_2: 1 prime ideal (ramified)");
    CHECK_EQ_INT(fac2.prime_ideals[0].e, 2, "x^2 + 1 in F_2: e=2 (ramified)");
    CHECK_EQ_INT(fac2.prime_ideals[0].f, 1, "x^2 + 1 in F_2: f=1");
    CHECK(mpz_cmp_ui(fac2.prime_ideals[0].g.coeff(0).get(), 1) == 0 &&
          mpz_cmp_ui(fac2.prime_ideals[0].g.coeff(1).get(), 1) == 0,
          "x^2 + 1 in F_2: g = x+1");

    // p=3 (≡ 3 mod 4): x^2 + 1 in F_3 has no roots (0² + 1 = 1, 1² + 1 = 2,
    //   2² + 1 = 5 = 2).  Stays inert: 1 prime ideal with e=1, f=2.
    auto fac3 = factor_prime_in_qbeta(charpoly, 3);
    fprintf(stderr, "  p = 3: %zu prime ideals\n", fac3.prime_ideals.size());
    CHECK_EQ_INT(fac3.prime_ideals.size(), 1, "x^2 + 1 in F_3: 1 prime ideal (inert)");
    CHECK_EQ_INT(fac3.prime_ideals[0].e, 1, "x^2 + 1 in F_3: e=1");
    CHECK_EQ_INT(fac3.prime_ideals[0].f, 2, "x^2 + 1 in F_3: f=2 (inert)");

    // p=5 (≡ 1 mod 4): x^2 + 1 in F_5 splits as (x+2)(x+3) (since
    //   -1 = 4 ≡ -1 mod 5, and √(-1) = ±2).  2 prime ideals with e=1, f=1.
    auto fac5 = factor_prime_in_qbeta(charpoly, 5);
    fprintf(stderr, "  p = 5: %zu prime ideals\n", fac5.prime_ideals.size());
    CHECK_EQ_INT(fac5.prime_ideals.size(), 2, "x^2 + 1 in F_5: 2 prime ideals (split)");
    for (const auto& pi : fac5.prime_ideals) {
        CHECK_EQ_INT(pi.e, 1, "split: e=1");
        CHECK_EQ_INT(pi.f, 1, "split: f=1");
    }
}

void test_maximal_flag_on_easy_cases() {
    fprintf(stderr, "=== test_maximal_flag_on_easy_cases ===\n");
    // The worked example (x^3 - 3x^2 - 2, p=2) and Z[i] (x^2+1) are
    // both known-maximal orders at every prime tested above -- the
    // `maximal` flag should say so, otherwise the earlier tests'
    // factorizations can't be trusted either.
    PolyZ worked;
    worked.ensure_size(4);
    set_si(worked.coeff(0), -2);
    set_si(worked.coeff(1), 0);
    set_si(worked.coeff(2), -3);
    set_si(worked.coeff(3), 1);
    auto fac = adelic::factor_prime_in_qbeta(worked, 2);
    CHECK(fac.maximal, "worked example (x^3-3x^2-2) should be 2-maximal");

    PolyZ gaussian;
    gaussian.ensure_size(3);
    set_si(gaussian.coeff(0), 1);
    set_si(gaussian.coeff(1), 0);
    set_si(gaussian.coeff(2), 1);
    auto facg2 = adelic::factor_prime_in_qbeta(gaussian, 2);
    CHECK(facg2.maximal, "Z[i] should be 2-maximal (ramified but monogenic)");
    auto facg5 = adelic::factor_prime_in_qbeta(gaussian, 5);
    CHECK(facg5.maximal, "Z[i] should be 5-maximal (split)");
}

void test_dedekind_non_monogenic_cubic() {
    fprintf(stderr, "=== test_dedekind_non_monogenic_cubic ===\n");
    // Dedekind's own classical example: theta^3 - theta^2 - 2*theta - 8 = 0.
    // Z[theta] is NOT the full ring of integers at p=2 -- this is the
    // textbook case where naively factoring the minimal polynomial
    // mod p gives a wrong answer.  Per ADELIC_TILING_PLAN.md §3.1
    // test #4: this is the single most important test in the list,
    // since it tests whether the code recognizes the case it is
    // *not* built to solve, rather than whether it gets easy cases
    // right.  Getting only the easy cases right and silently
    // mishandling this one is exactly the failure mode being guarded
    // against here.
    //
    // f(x) = x^3 - x^2 - 2x - 8.  Low-first PolyZ coeffs: [-8, -2, -1, 1].
    PolyZ f;
    f.ensure_size(4);
    set_si(f.coeff(0), -8);
    set_si(f.coeff(1), -2);
    set_si(f.coeff(2), -1);
    set_si(f.coeff(3), 1);
    fprintf(stderr, "  charpoly: %s\n", str(f).c_str());

    auto fac = factor_prime_in_qbeta(f, 2);
    fprintf(stderr, "  p = 2: maximal = %s, %zu prime ideal(s) reported (in Z[theta], "
                     "NOT trustworthy as O_K's factorization since maximal=false "
                     "is expected)\n",
            fac.maximal ? "true" : "false", fac.prime_ideals.size());
    for (const auto& pi : fac.prime_ideals) {
        fprintf(stderr, "    p = %lld, e = %lld, f = %lld, g = %s\n",
                pi.p, pi.e, pi.f, str(pi.g).c_str());
    }
    // f mod 2 = x^3 + x^2 + 1 (irreducible over F_2 -- check that too,
    // since an easy-case implementation might report this as a single
    // inert prime with e=1,f=3, which would *also* happen to be
    // consistent with maximality; the real content of this test is
    // the `maximal` flag itself).
    CHECK(!fac.maximal, "Dedekind's non-monogenic cubic MUST be reported "
                         "as non-2-maximal, not silently trusted");

    // Sanity: p=3 does not divide the discriminant issue here in the
    // same way; this is not part of the classical example's point,
    // so no assertion is made about p=3's maximal flag -- the test's
    // whole purpose is the p=2 case above.
}

}  // namespace

int main() {
    test_fp_arithmetic();
    test_fp_divmod();
    test_fp_gcd();
    test_factor_fp();
    test_factorize_worked_example();
    test_factorize_fully_ramified();
    test_factorize_unramified();
    test_maximal_flag_on_easy_cases();
    test_dedekind_non_monogenic_cubic();
    fprintf(stderr, "\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
