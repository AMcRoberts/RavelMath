// irreducibility_test.cpp
//
// Tests adelic::is_irreducible_over_q_via_small_primes (include/adelic/
// fp_poly_factor.hpp), the rigorous mod-p sufficient-condition
// irreducibility test built to generalize app/sweep_nonunit_property_f.cpp's
// cubic-only rational-root check to arbitrary degree (needed for a
// wider, e.g. 4-letter, non-unimodular Pisot survey -- see
// docs/DIRECTION_AND_OPEN_THREADS.md Item B1).
//
// Checks against known irreducible AND known reducible polynomials,
// including a case specifically chosen to defeat a naive
// "no rational root implies irreducible" check (true only at degree 3,
// not degree 4): (x^2-2)(x^2-3) = x^4-5x^2+6 has no rational roots
// (its roots are +-sqrt(2), +-sqrt(3)) but is reducible.

#include <cstdio>
#include <vector>

#include "adelic/fp_poly_factor.hpp"

using namespace std;
using namespace mathlib;
using namespace adelic;

namespace {
int n_pass = 0, n_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)

PolyZ mk(std::vector<long long> c) {
    PolyZ p;
    p.ensure_size(c.size());
    for (size_t i = 0; i < c.size(); ++i) set_si(p.coeff(i), c[i]);
    return p;
}
}  // namespace

int main() {
    fprintf(stderr, "=== known irreducible polynomials ===\n");
    CHECK(is_irreducible_over_q_via_small_primes(mk({-1, -1, -1, 1})),
          "Tribonacci x^3-x^2-x-1 is irreducible");
    CHECK(is_irreducible_over_q_via_small_primes(mk({-1, -1, -1, -1, 1})),
          "Tetrabonacci x^4-x^3-x^2-x-1 is irreducible");
    CHECK(is_irreducible_over_q_via_small_primes(mk({-1, -1, -1, -1, -1, 1})),
          "Pentanacci x^5-x^4-x^3-x^2-x-1 is irreducible");
    CHECK(is_irreducible_over_q_via_small_primes(mk({-2, 0, 0, 1})),
          "x^3-2 (Eisenstein at 2) is irreducible");
    CHECK(is_irreducible_over_q_via_small_primes(mk({1, 1})),
          "x+1 (degree 1) is trivially irreducible");

    fprintf(stderr, "\n=== known reducible polynomials, including the degree-4 trap ===\n");
    CHECK(!is_irreducible_over_q_via_small_primes(mk({6, 0, -5, 0, 1})),
          "(x^2-2)(x^2-3) = x^4-5x^2+6 is reducible despite having NO rational roots "
          "-- the case that defeats a naive rational-root-only check at degree 4");
    CHECK(!is_irreducible_over_q_via_small_primes(mk({-1, 0, 0, 0, 1})),
          "x^4-1 = (x-1)(x+1)(x^2+1) is reducible");
    CHECK(!is_irreducible_over_q_via_small_primes(mk({-6, 11, -6, 1})),
          "x^3-6x^2+11x-6 = (x-1)(x-2)(x-3) is reducible");
    CHECK(!is_irreducible_over_q_via_small_primes(mk({0, -3, 0, 1})),
          "x^3-3x = x(x^2-3) is reducible (has the rational root 0)");

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
