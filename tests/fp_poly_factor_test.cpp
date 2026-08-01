// fp_poly_factor_test.cpp
//
// Property test for include/adelic/fp_poly_factor.hpp's general F_p[x]
// factoring (squarefree factorization + distinct-degree factorization
// + Cantor-Zassenhaus equal-degree factorization), against:
//   1. Agreement with the existing (root-extraction-only, degree<=4
//      scoped) factor_fp on every case that old function actually
//      gets right.
//   2. A genuine, previously-undetected bug in factor_fp, found (not
//      sought out) while scoping this file: factor_fp assumes any
//      degree>=2 residual after root extraction is a SINGLE
//      irreducible factor, which is simply false whenever that
//      residual is a product of two-or-more irreducibles of the same
//      degree with no roots in F_p. Demonstrated with an explicit
//      degree-4 example (product of two irreducible quadratics mod 5)
//      where factor_fp returns one wrong quartic "factor" and
//      factor_fp_general correctly returns two quadratics,
//      cross-checked by multiplying the found factors back together.
//   3. A char-2 equal-degree-factorization case (product of two
//      irreducible cubics mod 2) -- the odd-p and p=2 splitting
//      polynomials in Cantor-Zassenhaus are genuinely different
//      formulas, and this project's own existing test cases already
//      use p=2, so this path needs its own coverage.
//   4. A characteristic-p squarefree-factorization edge case: a
//      polynomial whose formal derivative is IDENTICALLY ZERO (every
//      exponent present is a multiple of p), which needs the
//      "p-th root via coefficient re-indexing" trick (valid because
//      Frobenius is the identity map on F_p) rather than the ordinary
//      gcd(f,f') loop.
//   5. Repeated-factor multiplicity tracking on a case with an actual
//      repeated root, cross-checked against factor_fp (which already
//      handles multiplicities correctly for roots, so this confirms
//      the new squarefree-factorization stage doesn't regress that).

#include <cstdio>
#include <vector>

#include "adelic/dedekind_factorization.hpp"
#include "adelic/fp_poly_factor.hpp"
#include "math/bigint.hpp"
#include "math/poly_z.hpp"

using namespace std;
using adelic::FpPoly;
using adelic::FpFactor;
using adelic::factor_fp;
using adelic::factor_fp_general;
using adelic::factor_prime_in_qbeta;
using adelic::factor_prime_in_qbeta_general;
using adelic::fp_mul;
using adelic::fp_eval;
using adelic::fp_degree;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; fprintf(stderr, "  [ok]   %s\n", (msg)); } \
    else { ++n_fail; fprintf(stderr, "  [FAIL] %s\n", (msg)); } \
} while (0)

bool has_root(const FpPoly& f) {
    for (long long a = 0; a < f.p; ++a) if (fp_eval(f, a) == 0) return true;
    return false;
}

FpPoly reconstruct(const std::vector<FpFactor>& factors, long long p) {
    FpPoly r; r.p = p; r.c = {1};
    for (const auto& f : factors)
        for (long long k = 0; k < f.mult; ++k) r = fp_mul(r, f.g);
    return r;
}

bool same_poly(const FpPoly& a, const FpPoly& b) {
    if (a.c.size() != b.c.size()) return false;
    for (std::size_t i = 0; i < a.c.size(); ++i) if (a.c[i] != b.c[i]) return false;
    return true;
}

}  // namespace

int main() {
    fprintf(stderr, "=== agreement with factor_fp on cases it handles correctly ===\n");
    {
        // x^3 - 2 mod 2, from dedekind_factorization_test.cpp's worked example.
        FpPoly f; f.p = 2; f.c = {0, 0, 0, 1};  // x^3
        auto old_f = factor_fp(f);
        auto new_f = factor_fp_general(f);
        CHECK(reconstruct(old_f, 2).c == reconstruct(new_f, 2).c,
              "x^3 mod 2: factor_fp and factor_fp_general reconstruct to the same product");
    }
    {
        // A degree-2 irreducible: x^2+2 mod 3 (no root: 0->2,1->0(!) wait check)
        // Use x^2+1 mod 3 (roots: 0->1,1->2,2->2, no root -> irreducible).
        FpPoly f; f.p = 3; f.c = {1, 0, 1};
        CHECK(!has_root(f), "x^2+1 mod 3 has no root (sanity check on the test case itself)");
        auto old_f = factor_fp(f);
        auto new_f = factor_fp_general(f);
        CHECK(old_f.size() == 1 && new_f.size() == 1,
              "x^2+1 mod 3 (irreducible quadratic): both agree on a single factor");
        CHECK(same_poly(reconstruct(old_f, 3), f) && same_poly(reconstruct(new_f, 3), f),
              "both reconstruct exactly to the original polynomial");
    }

    fprintf(stderr, "\n=== a real bug in factor_fp, found while scoping this file ===\n");
    {
        // x^4+1 mod 5 = (x^2+2)(x^2+3), a product of two DISTINCT
        // irreducible quadratics with no roots in F_5 -- found by
        // direct search (see the file's own commit for the search),
        // not picked to look nice.
        FpPoly f; f.p = 5; f.c = {1, 0, 0, 0, 1};
        CHECK(!has_root(f), "x^4+1 mod 5 has no roots");

        auto old_f = factor_fp(f);
        CHECK(old_f.size() == 1,
              "factor_fp WRONGLY reports a single degree-4 factor (the bug)");
        CHECK(fp_degree(old_f[0].g) == 4,
              "that single wrong factor has degree 4 (should have split into two degree-2 factors)");

        auto new_f = factor_fp_general(f);
        CHECK(new_f.size() == 2, "factor_fp_general correctly finds two factors");
        bool both_degree_2 = new_f.size() == 2 &&
            fp_degree(new_f[0].g) == 2 && fp_degree(new_f[1].g) == 2;
        CHECK(both_degree_2, "both factors have degree 2, as expected for a product of two irreducible quadratics");
        for (const auto& fac : new_f) {
            CHECK(!has_root(fac.g), "each found quadratic factor is itself irreducible (no root)");
        }
        CHECK(same_poly(reconstruct(new_f, 5), f),
              "factor_fp_general's factors multiply back to exactly x^4+1 mod 5");
    }

    fprintf(stderr, "\n=== char-2 equal-degree factorization (odd-p formula doesn't apply) ===\n");
    {
        // Product of two irreducible cubics mod 2: x^3+x+1 and x^3+x^2+1
        // (the only two irreducible cubics over F_2).
        FpPoly a; a.p = 2; a.c = {1, 1, 0, 1};  // 1 + x + x^3
        FpPoly b; b.p = 2; b.c = {1, 0, 1, 1};  // 1 + x^2 + x^3
        CHECK(!has_root(a) && !has_root(b), "both cubics are root-free over F_2");
        FpPoly prod = fp_mul(a, b);
        auto new_f = factor_fp_general(prod);
        CHECK(new_f.size() == 2, "factor_fp_general splits the degree-6 product into two factors");
        bool both_degree_3 = new_f.size() == 2 &&
            fp_degree(new_f[0].g) == 3 && fp_degree(new_f[1].g) == 3;
        CHECK(both_degree_3, "both factors have degree 3");
        CHECK(same_poly(reconstruct(new_f, 2), prod),
              "factors multiply back to the original degree-6 product exactly (char-2 EDF path)");
    }

    fprintf(stderr, "\n=== characteristic-p squarefree edge case: f' identically zero ===\n");
    {
        // g(x) = x^2+x+1 mod 2 (the unique irreducible quadratic over
        // F_2), f(x) = g(x)^2 = x^4+x^2+1. f'(x) = 4x^3+2x = 0 mod 2
        // identically -- the "p-th root via re-indexing" branch must
        // fire, not the ordinary gcd(f,f') loop.
        FpPoly g; g.p = 2; g.c = {1, 1, 1};
        FpPoly f = fp_mul(g, g);
        fprintf(stderr, "  f = g^2 = ");
        for (auto c : f.c) fprintf(stderr, "%lld ", c);
        fprintf(stderr, "(expect 1 0 1 0 1, i.e. x^4+x^2+1)\n");
        auto new_f = factor_fp_general(f);
        CHECK(new_f.size() == 1, "x^4+x^2+1 mod 2 factors as a single irreducible with multiplicity 2");
        CHECK(new_f.size() == 1 && new_f[0].mult == 2,
              "the multiplicity is correctly reported as 2, not 1, via the p-th-root recursion");
        CHECK(new_f.size() == 1 && same_poly(new_f[0].g, g),
              "the found irreducible factor is exactly g = x^2+x+1");
    }

    fprintf(stderr, "\n=== repeated-root multiplicity, cross-checked against factor_fp ===\n");
    {
        // (x-1)^2 * (x-2) mod 5.
        FpPoly f; f.p = 5;
        FpPoly lin1; lin1.p = 5; lin1.c = {4, 1};  // x - 1
        FpPoly lin2; lin2.p = 5; lin2.c = {3, 1};  // x - 2
        f = fp_mul(fp_mul(lin1, lin1), lin2);
        auto old_f = factor_fp(f);
        auto new_f = factor_fp_general(f);
        CHECK(same_poly(reconstruct(old_f, 5), f), "factor_fp reconstructs (x-1)^2(x-2) correctly");
        CHECK(same_poly(reconstruct(new_f, 5), f), "factor_fp_general reconstructs (x-1)^2(x-2) correctly");
        long long mult_at_1_old = 0, mult_at_1_new = 0;
        for (auto& fac : old_f) if (fp_degree(fac.g) == 1 && fp_eval(fac.g, 1) == 0) mult_at_1_old = fac.mult;
        for (auto& fac : new_f) if (fp_degree(fac.g) == 1 && fp_eval(fac.g, 1) == 0) mult_at_1_new = fac.mult;
        CHECK(mult_at_1_old == 2 && mult_at_1_new == 2,
              "both report multiplicity 2 for the repeated root at x=1");
    }

    fprintf(stderr, "\n=== full Dedekind-level integration: the bug at the actual entry point ===\n");
    {
        // x^4+1 (the 8th cyclotomic polynomial, a genuine irreducible
        // quartic over Q -- not picked for being a Pisot minimal
        // polynomial, just for reducing mod 5 to exactly the bug case
        // demonstrated above). PolyZ low-first: 1 + 0x + 0x^2 + 0x^3 + x^4.
        mathlib::PolyZ charpoly;
        charpoly.ensure_size(5);
        mathlib::set_si(charpoly.coeff(0), 1);
        mathlib::set_si(charpoly.coeff(1), 0);
        mathlib::set_si(charpoly.coeff(2), 0);
        mathlib::set_si(charpoly.coeff(3), 0);
        mathlib::set_si(charpoly.coeff(4), 1);

        auto old_dedekind = factor_prime_in_qbeta(charpoly, 5);
        CHECK(old_dedekind.prime_ideals.size() == 1,
              "factor_prime_in_qbeta (old, at the Dedekind level) WRONGLY reports one prime above 5");
        CHECK(old_dedekind.prime_ideals.size() == 1 && old_dedekind.prime_ideals[0].f == 4,
              "that one wrong prime ideal has residue degree f=4 (should be two primes with f=2 each)");

        auto new_dedekind = factor_prime_in_qbeta_general(charpoly, 5);
        CHECK(new_dedekind.prime_ideals.size() == 2,
              "factor_prime_in_qbeta_general correctly reports two primes above 5");
        bool both_f2 = new_dedekind.prime_ideals.size() == 2 &&
            new_dedekind.prime_ideals[0].f == 2 && new_dedekind.prime_ideals[1].f == 2;
        CHECK(both_f2, "both primes above 5 have residue degree f=2, as expected");
        long long total_ef = 0;
        for (const auto& pi : new_dedekind.prime_ideals) total_ef += pi.e * pi.f;
        CHECK(total_ef == 4, "sum of e_i*f_i over both primes equals the field degree 4 (sanity)");
    }

    fprintf(stderr, "\n%d/%d checks passed\n", n_pass, n_pass + n_fail);
    return n_fail == 0 ? 0 : 1;
}
