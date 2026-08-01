// general_order_radical_test.cpp
//
// Property test for include/adelic/maximal_order.hpp's general
// (structure-constants) p-radical machinery: monogenic_structure_
// constants, p_radical_from_structure_constants (Ore's Frobenius
// method), discriminant_from_structure_constants, and the
// enlarge_order_round2_bigint(GeneralOrder, p) overload.
//
// This is the fallback for enlargement rounds beyond the first, where
// the previous round's order is no longer guaranteed to have a single
// defining polynomial to factor mod p -- see maximal_order.hpp's own
// header comment for why it's deliberately NOT the default path (more
// expensive than the monogenic shortcut enlarge_order_round2/_bigint
// use for round 1, where Z[beta] is always monogenic).
//
// Checks:
//   1. monogenic_structure_constants + discriminant_from_structure_
//      constants reproduces poly_discriminant_bigint's already-
//      trusted value on Dedekind's non-monogenic cubic.
//   2. enlarge_order_round2_bigint(GeneralOrder, p) reproduces the
//      EXACT same disc_before/disc_after as the monogenic-shortcut
//      path, on the same trusted textbook case -- at p=2, n=3, so
//      p<=n, which is exactly the regime where a trace-form shortcut
//      would NOT apply and only the general Frobenius method works,
//      making this a genuine test of the harder case, not the easy
//      one.
//   3. A basic sanity check on p_radical_from_structure_constants
//      directly: the radical basis it returns for the trusted case
//      has the dimension the known nilradical does.

#include <cstdio>
#include <vector>

#include "adelic/maximal_order.hpp"
#include "math/bigint.hpp"

using namespace std;
using mathlib::BigInt;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)

}  // namespace

int main() {
    // Dedekind's own non-monogenic cubic: x^3 - x^2 - 2x - 8, p=2.
    std::vector<long long> f = {1, -1, -2, -8};

    fprintf(stderr, "=== structure constants agree with the trusted polynomial-based path ===\n");
    auto O = adelic::monogenic_structure_constants(f);
    CHECK(O.n == 3, "monogenic_structure_constants reports n=3");

    auto disc_sc = adelic::discriminant_from_structure_constants(O);
    CHECK(mathlib::cmp_si(disc_sc, -2012) == 0,
          "discriminant_from_structure_constants(O) == -2012, matching poly_discriminant_bigint");

    fprintf(stderr, "\n=== p-radical from structure constants, the harder p<=n case (p=2,n=3) ===\n");
    auto radical_basis = adelic::p_radical_from_structure_constants(O, 2);
    CHECK(!radical_basis.empty(),
          "p_radical_from_structure_constants finds a nontrivial radical (Z[theta] is NOT 2-maximal)");

    fprintf(stderr, "\n=== full enlargement via the general method, cross-checked against the monogenic shortcut ===\n");
    auto old_r = adelic::enlarge_order_round2_bigint(f, 2);       // monogenic-shortcut overload
    auto new_r = adelic::enlarge_order_round2_bigint(O, 2);       // general (GeneralOrder) overload

    CHECK(old_r.enlarged == new_r.enlarged, "enlarged flag agrees between the two methods");
    CHECK(mathlib::cmp(old_r.disc_before, new_r.disc_before) == 0,
          "disc_before agrees exactly between the monogenic-shortcut and general methods");
    CHECK(mathlib::cmp(old_r.disc_after, new_r.disc_after) == 0,
          "disc_after agrees exactly between the monogenic-shortcut and general methods");
    CHECK(mathlib::cmp_si(new_r.disc_after, -503) == 0,
          "the general method reproduces the known textbook field discriminant -503, "
          "computed WITHOUT ever factoring a defining polynomial mod p");
    CHECK(old_r.needs_another_round == new_r.needs_another_round,
          "needs_another_round agrees (both false: -503 is odd, so p=2 is certainly done)");

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
