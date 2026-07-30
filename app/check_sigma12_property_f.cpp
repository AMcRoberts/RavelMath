// check_sigma12_property_f.cpp
//
// Follow-up to check_sigma12_coincidence.cpp: strong coincidence alone
// (checked there, HOLDS trivially at depth 1 for both) is necessary
// but not sufficient for the Pisot Substitution Conjecture. This
// driver runs the FULL classify() pipeline from
// app/classify_adelic_tiling.cpp -- strong coincidence AND geometric
// property (F) together -- against sigma1 and sigma2, thread A's own
// unimodular Pisot counterexamples to lambda(G_B) = BP-rho_nc.
//
// Both substitutions are unimodular (det M = 1, checked by hand --
// see the conversation this driver came out of), so this lands
// exactly in the ONE case coincidence_and_property_f.hpp's own STATUS
// section already calls trustworthy without qualification: "the UNIT
// case (no prime factors at all -- the archimedean-only bound
// suffices, verified on Fibonacci)". No p-adic machinery is invoked;
// primes_dividing_det is passed as {} for both.
//
// If both strong coincidence AND property (F) come back HOLDS here,
// that is not a numerical curiosity -- Minervino-Thuswaldner Thm 9.9
// (the theorem this project's adelic machinery implements) says that
// combination upgrades the "multiple tiling" fact to a genuine simple
// tiling, and (per the classical route surveyed in
// docs/ADELIC_TILING_PLAN.md) that is the standard sufficient route
// to the Pisot Substitution Conjecture for a unimodular substitution.
// It would mean sigma1/sigma2 DO satisfy the conjecture, via an
// established theorem plus this project's own exact computation --
// even though they are not covered by any *named* prior result (see
// conversation) and even though lambda(G_B) != BP-rho_nc for them.
//
// M_transpose convention matches app/classify_adelic_tiling.cpp's
// OWN validated rnd13 call exactly (Mt = the transpose of the
// standard "column j = abelianized image of letter j" incidence
// matrix) -- NOT its own "worked example" call, which is annotated
// "not yet reachable" in that file and is the likely source of the
// apparent inconsistency between its two example calls.
//
// Build: g++ -std=c++20 -O2 -Iinclude -Imath/include -Imath/include/mini-gmp \
//        app/check_sigma12_property_f.cpp math/out/libmath.a -o check_sigma12_property_f

#include <array>
#include <cstdio>
#include <vector>

#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"

namespace {

// Identical to app/classify_adelic_tiling.cpp's own classify<d>() --
// duplicated here rather than shared across a header, matching this
// project's existing convention of self-contained app/ drivers (see
// docs/CPP_DESIGN_PHILOSOPHY.md).
template <std::size_t d>
void classify(const char* name,
              const std::array<std::vector<long long>, d>& images,
              std::initializer_list<long long> charpoly_low_first,
              const std::vector<std::vector<long long>>& M_transpose,
              const std::vector<long long>& primes_dividing_det) {
    printf("=================================================================\n");
    printf("Adelic tiling classification: %s\n", name);
    printf("=================================================================\n");

    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first(charpoly_low_first);
    const mathlib::PolyZ& charpoly = R.charpoly();
    printf("charpoly: %s\n", mathlib::str(charpoly).c_str());

    bool any_non_maximal = false;
    (void)any_non_maximal;
    for (long long p : primes_dividing_det) {
        auto fac = adelic::factor_prime_in_qbeta(charpoly, p);
        bool cross_ok = adelic::cross_check_dedekind_factorization(fac, charpoly, static_cast<long long>(d));
        printf("Factoring (%lld): maximal=%s, cross-checked=%s\n", p,
               fac.maximal ? "YES" : "NO", cross_ok ? "MATCH" : "MISMATCH");
    }

    std::vector<std::vector<long long>> Mt = M_transpose;
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
    if (!eig.ok) {
        printf("FAILED to compute left Perron eigenvector; cannot proceed.\n\n");
        return;
    }
    auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);
    printf("Prefix automaton: %zu distinct prefixes, digit set size %zu.\n\n",
           automaton.distinct_prefixes.size(), automaton.digit_set.size());

    auto coin = adelic::check_strong_coincidence<d>(images);
    printf("Strong coincidence: %s (depth reached: %lld%s)\n",
           coin.holds ? "HOLDS" : (coin.inconclusive ? "INCONCLUSIVE" : "FAILS"),
           coin.depth_reached,
           coin.inconclusive ? ", cutoff hit before resolving" : "");

    // Unit case only (primes_dividing_det is {} for both sigma1 and
    // sigma2): no p-adic bound needed or used, matching the ONE case
    // this header's own STATUS section calls unqualifiedly trustworthy.
    adelic::PropertyFResult propf = adelic::check_property_f<d>(automaton, 300000);
    printf("Property (F): %s (nodes explored: %lld%s)\n",
           propf.holds ? "HOLDS" : (propf.inconclusive ? "INCONCLUSIVE" : "FAILS"),
           propf.nodes_explored,
           propf.inconclusive ? ", node budget exhausted" : "");

    printf("\nVerdict: %s\n\n",
           (coin.holds && propf.holds)
               ? "strong coincidence + property (F) BOTH HOLD -- sufficient for the Pisot conjecture here"
               : "inconclusive / does not close with this check alone");
}

}  // namespace

int main() {
    // sigma1: 0 -> 0001, 1 -> 002, 2 -> 0. Unimodular (det M = 1).
    {
        constexpr std::size_t d = 3;
        std::array<std::vector<long long>, d> images = {
            std::vector<long long>{0, 0, 0, 1},
            std::vector<long long>{0, 0, 2},
            std::vector<long long>{0},
        };
        std::vector<std::vector<long long>> Mt = {
            {3, 1, 0},
            {2, 0, 1},
            {1, 0, 0},
        };
        classify<d>("sigma1 (thread A counterexample: lambda(G_B) != BP-rho_nc)",
                    images, {-3, -2, -1}, Mt, {});
    }

    // sigma2: 0 -> 001, 1 -> 0002, 2 -> 0. Unimodular (det M = 1).
    {
        constexpr std::size_t d = 3;
        std::array<std::vector<long long>, d> images = {
            std::vector<long long>{0, 0, 1},
            std::vector<long long>{0, 0, 0, 2},
            std::vector<long long>{0},
        };
        std::vector<std::vector<long long>> Mt = {
            {2, 1, 0},
            {3, 0, 1},
            {1, 0, 0},
        };
        classify<d>("sigma2 (thread A counterexample: lambda(G_B) != BP-rho_nc)",
                    images, {-2, -3, -1}, Mt, {});
    }

    return 0;
}
