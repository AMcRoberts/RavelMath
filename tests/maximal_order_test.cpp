// maximal_order_test.cpp
//
// Self-test for include/adelic/maximal_order.hpp against the
// textbook example named explicitly in docs/ADELIC_TILING_PLAN.md
// §6 ("Dedekind's own non-monogenic cubic"): theta a root of
//   f(x) = x^3 - x^2 - 2x - 8
// disc(f) = disc(Z[theta]) = -2012 = -4 * 503 (503 prime).  The
// classical fact (Dedekind's own counterexample to "factor f mod p
// to factor (p)") is that Z[theta] is NOT 2-maximal: f mod 2 =
// x^2(x+1) suggests (2) = p_1^2 p_2, but the TRUE factorization of
// (2) in O_K is into three distinct degree-1 primes (2 splits
// completely) -- which is only visible after passing to the true
// maximal order O_K, which has disc(O_K) = -503 (index [O_K:Z[theta]]
// = 2, so disc ratio is 1/4).
//
// This test checks:
//   1. Dedekind's criterion (is_p_maximal) correctly reports NOT
//      2-maximal for Z[theta] (a regression guard: if this ever
//      flips, the rest of this test's premise is void).
//   2. enlarge_order_round2 produces a strictly larger order.
//   3. The enlarged order's discriminant is EXACTLY -503, matching
//      the known field discriminant -- i.e. one round suffices and
//      lands on the true maximal order for this classic example.

#include "adelic/dedekind_factorization.hpp"
#include "adelic/maximal_order.hpp"
#include "math/poly_z.hpp"

#include <cstdio>
#include <vector>

using namespace adelic;

static int total_tests = 0;
static int failed = 0;

#define CHECK(expr, label) do {                                       \
    ++total_tests;                                                    \
    if (!(expr)) { std::printf("  [FAIL] %s\n", label); ++failed; }    \
    else         { std::printf("  [ok]   %s\n", label); }              \
} while (0)

int main() {
    std::printf("maximal_order self-tests (Dedekind's non-monogenic cubic):\n");

    // f(x) = x^3 - x^2 - 2x - 8, highest-degree-first: {1, -1, -2, -8}.
    std::vector<long long> f = {1, -1, -2, -8};

    long long disc = poly_discriminant_ll(f);
    std::printf("  disc(f) = %lld (expected -2012 = -4 * 503)\n", disc);
    CHECK(disc == -2012, "poly_discriminant_ll matches the textbook value -2012");

    // Cross-check Dedekind's criterion agrees this is NOT 2-maximal
    // (using dedekind_factorization.hpp's own is_p_maximal, exercised
    // via the same low-level FpPoly/factor_fp path enlarge_order_round2
    // uses internally, so this is a genuine cross-check, not a
    // tautology).
    {
        mathlib::PolyZ fz;
        fz.ensure_size(4);
        // PolyZ convention: check the low-degree-first storage used
        // elsewhere in this file (mathlib::set_si on coeff(i) for
        // degree i) -- mirror dedekind_factorization.hpp's own
        // worked-example test setup.
        mathlib::set_si(fz.coeff(0), -8);
        mathlib::set_si(fz.coeff(1), -2);
        mathlib::set_si(fz.coeff(2), -1);
        mathlib::set_si(fz.coeff(3), 1);
        FpPoly f2 = reduce_z_to_fp(fz, 2);
        auto factors = factor_fp(f2);
        bool maximal = is_p_maximal(fz, 2, factors);
        std::printf("  is_p_maximal(f, p=2) = %d (expected 0: Z[theta] is NOT 2-maximal)\n", maximal);
        CHECK(!maximal, "Dedekind's criterion confirms Z[theta] is NOT 2-maximal at p=2");
    }

    auto result = enlarge_order_round2(f, 2);
    CHECK(result.enlarged, "enlarge_order_round2 finds a strictly larger order at p=2");
    std::printf("  det(H) = %lld, disc(before) = %lld, disc(after) = %lld\n",
                result.det_over_p_n_numerator, result.disc_before, result.disc_after);
    CHECK(result.disc_before == -2012, "enlarge_order_round2 recomputes disc(Z[theta]) = -2012");
    CHECK(result.disc_after == -503,
          "enlarged order's discriminant is EXACTLY -503 (the true field discriminant -- "
          "one Round-2 step reaches O_K for this classic example)");

    // -503 is squarefree (503 is prime), so any order with that exact
    // discriminant is automatically maximal at every prime -- a
    // strong, checkable corollary that the enlargement didn't just
    // get closer, it reached the true ring of integers.
    CHECK(result.disc_after % (503LL * 503LL) != 0, "disc(O') = -503 is squarefree (sanity)");

    // Second sanity check on a case that IS already 2-maximal
    // (Tetrabonacci: x^4 - x^3 - x^2 - x - 1, unimodular, known to
    // have no index issues at any prime for this project's existing
    // survey) -- enlarge_order_round2 should report no enlargement.
    {
        std::vector<long long> tetra = {1, -1, -1, -1, -1};
        auto r2 = enlarge_order_round2(tetra, 2);
        std::printf("  Tetrabonacci at p=2: enlarged=%d, disc=%lld\n",
                    r2.enlarged, r2.disc_before);
        CHECK(!r2.enlarged, "Tetrabonacci's order is already 2-maximal (no enlargement found)");
    }

    std::printf("\n%d tests run, %d failed.\n", total_tests, failed);
    return failed == 0 ? 0 : 1;
}
