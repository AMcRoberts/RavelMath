// tests/ideal_arithmetic_test.cpp
//
// Self-test for include/adelic/ideal_arithmetic.hpp.
//
// Per ADELIC_TILING_PLAN.md §3.1's four-case ladder (run here against
// the ideal-arithmetic implementation, exactly as it was already run
// against dedekind_factorization.hpp):
//   1. HNF sanity check, no number theory (pure Z^n lattice reduction)
//   2. Z[i] warm-up (classical, hand-checkable prime splitting)
//   3. The paper's own worked example ((2) = p1^2 p2)
//   4. Dedekind's non-monogenic cubic (the cross-check should also
//      fail here, as a second, independently-computed confirmation
//      of dedekind_factorization_test.cpp's maximal=false result)
// Plus: rnd13 itself ((2) = p^4, totally ramified).

#include <cstdio>
#include <vector>

#include "adelic/ideal_arithmetic.hpp"
#include "math/bigint.hpp"
#include "math/poly_z.hpp"

using namespace adelic;
using mathlib::PolyZ;
using mathlib::set_si;
using mathlib::BigInt;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

IntVec v2(long long a, long long b) {
    IntVec v(2);
    set_si(v[0], a);
    set_si(v[1], b);
    return v;
}

void test_hnf_sanity() {
    fprintf(stderr, "=== test_hnf_sanity ===\n");
    // (4,0), (0,6) should HNF-reduce to exactly themselves (already
    // in canonical upper-triangular-by-column form: pivot[0]=(4,0),
    // pivot[1]=(0,6), and 0 <= pivot[1][0]=0 < pivot[0][0]=4).
    IntCols cols1 = { v2(4, 0), v2(0, 6) };
    IntCols h1 = hnf_reduce(cols1, 2);
    CHECK(mathlib::cmp(h1[0][0], BigInt(4)) == 0 && mathlib::cmp(h1[0][1], BigInt(0)) == 0,
          "HNF((4,0),(0,6)) column 0 = (4,0)");
    CHECK(mathlib::cmp(h1[1][0], BigInt(0)) == 0 && mathlib::cmp(h1[1][1], BigInt(6)) == 0,
          "HNF((4,0),(0,6)) column 1 = (0,6)");

    // (2,3), (4,5): determinant of the 2x2 matrix with these as
    // columns is 2*5 - 3*4 = -2, so the HNF basis should have
    // |det| = 2 (product of diagonal entries = 2, since HNF is
    // upper triangular by column here -- pivot[0][0] * pivot[1][1]).
    IntCols cols2 = { v2(2, 3), v2(4, 5) };
    IntCols h2 = hnf_reduce(cols2, 2);
    BigInt det;
    mathlib::mul(det, h2[0][0], h2[1][1]);
    CHECK(mathlib::cmp(det, BigInt(2)) == 0, "HNF((2,3),(4,5)): det(basis) = 2");
    // pivot[0] row 1 must be 0 (upper triangular by column convention).
    CHECK(mathlib::sgn(h2[0][1]) == 0, "HNF((2,3),(4,5)): column 0 has 0 in row 1");
}

void test_z_i_warmup() {
    fprintf(stderr, "=== test_z_i_warmup ===\n");
    // Z[i]: charpoly x^2 + 1.  PolyZ low-first coeffs [1, 0, 1].
    PolyZ f;
    f.ensure_size(3);
    set_si(f.coeff(0), 1);
    set_si(f.coeff(1), 0);
    set_si(f.coeff(2), 1);

    // p=2: ramified, (2) = (1+i)^2 up to a unit.  Dedekind gives
    // g = x+1, e=2, f=1.  Cross-check via direct ideal arithmetic.
    auto fac2 = factor_prime_in_qbeta(f, 2);
    CHECK(fac2.maximal, "Z[i] is 2-maximal");
    CHECK(cross_check_dedekind_factorization(fac2, f, 2),
          "ideal arithmetic independently confirms (2) = p^2 in Z[i]");

    // p=5: splits, (5) = (2+i)(2-i).
    auto fac5 = factor_prime_in_qbeta(f, 5);
    CHECK(fac5.maximal, "Z[i] is 5-maximal");
    CHECK(cross_check_dedekind_factorization(fac5, f, 2),
          "ideal arithmetic independently confirms (5) = p1*p2 in Z[i]");

    // p=3: inert.
    auto fac3 = factor_prime_in_qbeta(f, 3);
    CHECK(fac3.maximal, "Z[i] is 3-maximal");
    CHECK(cross_check_dedekind_factorization(fac3, f, 2),
          "ideal arithmetic independently confirms (3) stays inert (= itself) in Z[i]");
}

void test_worked_example() {
    fprintf(stderr, "=== test_worked_example ===\n");
    // sigma(1)=1113, sigma(2)=11, sigma(3)=2: charpoly x^3-3x^2-2,
    // p=2: claimed (2) = p1^2 * p2, p1=(2,beta), p2=(2,beta+1).
    PolyZ f;
    f.ensure_size(4);
    set_si(f.coeff(0), -2);
    set_si(f.coeff(1), 0);
    set_si(f.coeff(2), -3);
    set_si(f.coeff(3), 1);

    auto fac = factor_prime_in_qbeta(f, 2);
    CHECK(fac.maximal, "worked example is 2-maximal");
    CHECK(fac.prime_ideals.size() == 2, "worked example: 2 prime ideals from Dedekind");
    CHECK(cross_check_dedekind_factorization(fac, f, 3),
          "ideal arithmetic independently confirms (2) = p1^2 * p2 in the worked example");
}

void test_non_monogenic_cross_check_also_fails() {
    fprintf(stderr, "=== test_non_monogenic_cross_check_also_fails ===\n");
    // Dedekind's non-monogenic cubic: theta^3 - theta^2 - 2theta - 8.
    // dedekind_factorization_test.cpp already showed maximal=false
    // via Dedekind's criterion.  This test asks the SAME question a
    // completely different way: does prod p_i^{e_i} (as computed
    // from Dedekind's naive-mod-p factors) actually multiply back
    // out (via independent HNF ideal arithmetic) to (2)?  If Z[beta]
    // were secretly fine at 2, this would have to hold; the point of
    // this test is to see it demonstrably NOT hold, as a second,
    // independently-implemented confirmation of the maximal=false
    // finding -- agreement between two different broken things isn't
    // expected, but this is not a proof that non-maximality always
    // shows up this way (see the doc comment on
    // cross_check_dedekind_factorization for the caveat).
    PolyZ f;
    f.ensure_size(4);
    set_si(f.coeff(0), -8);
    set_si(f.coeff(1), -2);
    set_si(f.coeff(2), -1);
    set_si(f.coeff(3), 1);

    auto fac = factor_prime_in_qbeta(f, 2);
    CHECK(!fac.maximal, "non-monogenic cubic: Dedekind's criterion says NOT 2-maximal (sanity re-check)");
    bool matches = cross_check_dedekind_factorization(fac, f, 3);
    fprintf(stderr, "  cross-check on the non-maximal case: %s (informative, not required to be false)\n",
            matches ? "MATCHED (unexpected but not a contradiction)" : "did NOT match, consistent with non-maximality");
    // Not a hard CHECK: literature confirms the naive factorization
    // is simply wrong in the non-maximal case, but "wrong" doesn't
    // logically guarantee ideal multiplication fails to reconstruct
    // (p) -- e.g. a wrong e_i/g_i could coincidentally still multiply
    // out correctly for a specific p. We record the outcome for the
    // record rather than asserting it.
    (void)matches;
}

void test_rnd13() {
    fprintf(stderr, "=== test_rnd13 ===\n");
    // rnd13 quartic charpoly x^4 - 4x^3 - 8x^2 - 6x - 2.  Per
    // rnd13_factor_probe.cpp / dedekind_factorization_test.cpp:
    // 2-maximal, (2) = p^4 (single totally ramified prime, e=4, f=1).
    PolyZ f;
    f.ensure_size(5);
    set_si(f.coeff(0), -2);
    set_si(f.coeff(1), -6);
    set_si(f.coeff(2), -8);
    set_si(f.coeff(3), -4);
    set_si(f.coeff(4), 1);

    auto fac = factor_prime_in_qbeta(f, 2);
    CHECK(fac.maximal, "rnd13 is 2-maximal (re-check)");
    CHECK(fac.prime_ideals.size() == 1, "rnd13: 1 prime ideal above 2");
    CHECK(fac.prime_ideals[0].e == 4 && fac.prime_ideals[0].f == 1,
          "rnd13: e=4, f=1 (re-check)");
    CHECK(cross_check_dedekind_factorization(fac, f, 4),
          "ideal arithmetic independently confirms (2) = p^4 for rnd13 "
          "-- the key result needed before trusting the "
          "K_2 construction in padic_test.cpp's rnd13 case");
}

}  // namespace

int main() {
    test_hnf_sanity();
    test_z_i_warmup();
    test_worked_example();
    test_non_monogenic_cross_check_also_fails();
    test_rnd13();
    fprintf(stderr, "\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
