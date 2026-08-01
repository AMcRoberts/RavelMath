// number_field_parity_test.cpp
//
// Property/parity test for include/adelic/dedekind_factorization.hpp
// and include/adelic/ideal_arithmetic.hpp against a classical, fully
// worked textbook example that has NOTHING to do with a Pisot
// substitution: Q(sqrt(-5)), the standard non-unique-factorization
// example (6 = 2*3 = (1+sqrt(-5))(1-sqrt(-5))).
//
// Why this test exists: every existing caller of this machinery
// (dedekind_factorization_test.cpp, ideal_arithmetic_test.cpp) feeds
// it a Pisot substitution's characteristic polynomial -- a REAL,
// totally-real-embedded number field with a dominant eigenvalue. The
// actual algorithms (Dedekind's criterion, F_p[x] factoring, HNF
// ideal-lattice arithmetic) never reference "real," "Pisot," or
// "dominant eigenvalue" anywhere in their logic -- they only need a
// monic integer polynomial defining a degree-n ring Z[beta]/(f(beta)).
// This test is the first check of that claim: feed the SAME machinery
// a CM-type field (Q(sqrt(-5)) has no real embedding at all) and
// confirm it reproduces a known, citable classical result -- not
// because Q(sqrt(-5)) is a stepping stone to anything Pisot, but
// because "does this code actually only need what it claims to need"
// is a checkable question, and checking it is cheaper than assuming.
//
// beta = sqrt(-5), minimal polynomial x^2 + 5 (degree n=2).
// disc(x^2+5) = -20 = disc(Q(sqrt(-5))) (since -5 = 3 mod 4, the
// field discriminant is 4*(-5) = -20, matching the polynomial
// discriminant exactly) -- so Z[beta] = O_K, no maximal-order
// enlargement (maximal_order.hpp) is needed for this example, keeping
// the test scoped to exactly the two pieces being checked.
//
// Known facts checked against (any number theory reference, e.g.
// Marcus's "Number Fields" ch. 1, or Cohen's "A Course in
// Computational Algebraic Number Theory" ch. 4 worked examples):
//   (2) = p2^2            (2 ramifies: x^2+5 = (x+1)^2 mod 2)
//   (3) = p3a * p3b        (3 splits: x^2+5 = (x-1)(x-2) mod 3)
//   N(1+beta) = N(1-beta) = 6
//   (1+beta) = p2 * p3? for exactly one of p3a, p3b (computed, not
//   asserted from a hand derivation -- this file finds out which)

#include <cstdio>
#include <vector>

#include "adelic/dedekind_factorization.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "math/bigint.hpp"
#include "math/poly_z.hpp"

using namespace std;
using mathlib::PolyZ;
using mathlib::set_si;
using adelic::DedekindFactorization;
using adelic::PrimeIdeal;
using adelic::factor_prime_in_qbeta;
using adelic::IdealHNF;
using adelic::ideal_from_generators;
using adelic::ideal_from_prime;
using adelic::ideal_mul;
using adelic::ideal_pow;
using adelic::ideal_equal;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)

PolyZ minpoly_x2_plus_5() {
    // x^2 + 5, low-first: coeff(0)=5, coeff(1)=0, coeff(2)=1.
    PolyZ f;
    f.ensure_size(3);
    set_si(f.coeff(0), 5);
    set_si(f.coeff(1), 0);
    set_si(f.coeff(2), 1);
    return f;
}

}  // namespace

int main() {
    const long long n = 2;
    PolyZ f = minpoly_x2_plus_5();

    fprintf(stderr, "=== Q(sqrt(-5)): Dedekind factorization, a non-Pisot field ===\n");

    DedekindFactorization fac2 = factor_prime_in_qbeta(f, 2);
    CHECK(fac2.maximal, "Z[beta]=Z[sqrt(-5)] is 2-maximal (disc(f)=disc(K)=-20)");
    CHECK(fac2.prime_ideals.size() == 1, "(2) factors as a single prime ideal (ramified)");
    if (fac2.prime_ideals.size() == 1) {
        CHECK(fac2.prime_ideals[0].e == 2, "(2)'s prime ideal has ramification index e=2");
        CHECK(fac2.prime_ideals[0].f == 1, "(2)'s prime ideal has residue degree f=1");
    }

    DedekindFactorization fac3 = factor_prime_in_qbeta(f, 3);
    CHECK(fac3.maximal, "Z[beta] is 3-maximal");
    CHECK(fac3.prime_ideals.size() == 2, "(3) factors as two distinct prime ideals (split)");
    for (const auto& pi : fac3.prime_ideals) {
        CHECK(pi.e == 1 && pi.f == 1, "each prime above 3 has e=1, f=1");
    }

    fprintf(stderr, "\n=== independent HNF-ideal cross-check ===\n");

    IdealHNF p2 = ideal_from_prime(fac2.prime_ideals[0], n, f);
    IdealHNF two_Z = adelic::principal_int_ideal(2, n);
    IdealHNF p2_squared = ideal_pow(p2, 2, f);
    CHECK(ideal_equal(p2_squared, two_Z),
          "p2^2 == (2) as ideals, independently re-derived via HNF lattice arithmetic");

    std::vector<IdealHNF> p3s;
    for (const auto& pi : fac3.prime_ideals) p3s.push_back(ideal_from_prime(pi, n, f));
    IdealHNF three_Z = adelic::principal_int_ideal(3, n);
    IdealHNF p3_product = ideal_mul(p3s[0], p3s[1], f);
    CHECK(ideal_equal(p3_product, three_Z),
          "p3a * p3b == (3) as ideals, independently re-derived via HNF lattice arithmetic");

    fprintf(stderr, "\n=== the actual non-unique-factorization example: (1+beta) ===\n");
    // 1 + beta, low-first: [1, 1].
    PolyZ one_plus_beta;
    one_plus_beta.ensure_size(2);
    set_si(one_plus_beta.coeff(0), 1);
    set_si(one_plus_beta.coeff(1), 1);
    IdealHNF principal = ideal_from_generators({one_plus_beta}, f, n);

    // N(1+sqrt(-5)) = (1+sqrt(-5))(1-sqrt(-5)) = 1 - (-5) = 6 = 2*3, so
    // (1+beta) should equal p2 * p3a or p2 * p3b for exactly one of
    // the two primes above 3 -- computed here, not assumed.
    bool matched_a = ideal_equal(principal, ideal_mul(p2, p3s[0], f));
    bool matched_b = ideal_equal(principal, ideal_mul(p2, p3s[1], f));
    CHECK(matched_a != matched_b,
          "(1+beta) equals p2 times EXACTLY ONE of the two primes above 3 (found by computing both, not guessed)");
    fprintf(stderr, "  (matched_a=%d matched_b=%d)\n", matched_a, matched_b);

    // The classical punchline, checked directly rather than narrated:
    // 6 = 2*3 as elements, but the ideal (6) = (1+beta)*(1-beta) as
    // elements factors, via ideals, into FOUR prime ideal factors
    // (p2^2 * p3a * p3b), not two -- which is exactly why 2*3 and
    // (1+beta)(1-beta) can both be "irreducible factorizations of 6"
    // in the ring without contradicting unique factorization: unique
    // factorization holds for IDEALS, not elements, in a non-PID.
    IdealHNF six_Z = adelic::principal_int_ideal(6, n);
    IdealHNF two_times_three = ideal_mul(two_Z, three_Z, f);
    CHECK(ideal_equal(six_Z, two_times_three),
          "(2)*(3) == (6) as ideals (sanity: principal_int_ideal is multiplicative)");
    IdealHNF one_plus_beta_squared_norm =
        ideal_mul(principal, ideal_from_generators(
            {[]() { PolyZ p; p.ensure_size(2); set_si(p.coeff(0), 1);
                     set_si(p.coeff(1), -1); return p; }()}, f, n), f);
    CHECK(ideal_equal(one_plus_beta_squared_norm, six_Z),
          "(1+beta)*(1-beta) == (6) as ideals -- both element factorizations of 6 "
          "resolve to the SAME ideal factorization p2^2*p3a*p3b underneath");

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
