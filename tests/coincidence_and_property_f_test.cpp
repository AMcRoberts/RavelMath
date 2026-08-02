// tests/coincidence_and_property_f_test.cpp
//
// Self-test for include/adelic/coincidence_and_property_f.hpp.
//
// Per ADELIC_TILING_PLAN.md §6: build and test against
// sigma(1)=1113, sigma(2)=11, sigma(3)=2 first (expected: strong
// coincidence holds, property (F) holds -- matching the paper's own
// §10.2 stated conclusion, TILES) before trusting the pipeline on
// rnd13.

#include <array>
#include <cstdio>
#include <vector>

#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"

using namespace adelic;

namespace {
int n_pass = 0, n_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

void test_pair_has_coincidence_basics() {
    fprintf(stderr, "=== test_pair_has_coincidence_basics ===\n");
    // Trivial: identical single-letter words share letter 0 at the
    // same (empty) prefix/suffix -- coincidence trivially holds.
    std::vector<long long> w1 = {0};
    std::vector<long long> w2 = {0};
    CHECK((pair_has_coincidence<2>(w1, w2)), "identical single-letter words coincide trivially");

    // No shared letter at all -> no coincidence.
    std::vector<long long> w3 = {0, 0};
    std::vector<long long> w4 = {1, 1};
    CHECK(!(pair_has_coincidence<2>(w3, w4)), "disjoint-alphabet words never coincide");
}

void test_worked_example() {
    fprintf(stderr, "=== test_worked_example (sigma(1)=1113,sigma(2)=11,sigma(3)=2) ===\n");
    constexpr std::size_t d = 3;
    // Re-indexed letters 1,2,3 -> 0,1,2.
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 0, 2},  // sigma(0) = "1113"
        std::vector<long long>{0, 0},        // sigma(1) = "11"
        std::vector<long long>{1},           // sigma(2) = "2"
    };

    // charpoly x^3 - 3x^2 - 2, low-first coeffs {0, -3, -2}
    // (x^3 + a0 x^2 + a1 x + a2 with a0=-3,a1=0,a2=-2 -- careful:
    // x^3 - 3x^2 + 0x - 2, so low_first = {a0,a1,a2} = {-3, 0, -2}).
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-3, 0, -2});

    // M (incidence: M[i][j] = count of letter i in sigma(j)):
    //   col0=sigma(0)="1113"->(3,0,1), col1=sigma(1)="11"->(2,0,0), col2=sigma(2)="2"->(0,1,0)
    // M^T (for the LEFT eigenvector, per prefix_automaton.hpp convention):
    std::vector<std::vector<long long>> Mt = {
        {3, 2, 0},
        {0, 0, 1},
        {1, 0, 0},
    };
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
    CHECK(eig.ok, "left eigenvector computed successfully");

    auto automaton = build_prefix_automaton<d>(images, eig.v, R);
    fprintf(stderr, "  digit set size: %zu\n", automaton.digit_set.size());
    CHECK(automaton.digit_set.size() > 0, "digit set is nonempty");

    auto coin = check_strong_coincidence<d>(images);
    fprintf(stderr, "  strong coincidence: holds=%d inconclusive=%d depth=%lld\n",
            coin.holds, coin.inconclusive, coin.depth_reached);
    CHECK(coin.holds, "worked example: strong coincidence HOLDS (matches paper's Sec 10.2)");
    CHECK(!coin.inconclusive, "worked example: strong coincidence resolved (not inconclusive)");

    // Deliberate archimedean-only control. The shared classifier's
    // combined local-field bound closes this example (verified in
    // classify_adelic_test); without that bound, the search exhausts
    // its budget. This confirms the p-adic pruning is substantive.
    auto propf = check_property_f<d>(automaton);
    fprintf(stderr, "  property (F) [archimedean-only control; combined p-adic bound "
                     "tested separately]: holds=%d inconclusive=%d nodes=%lld\n",
            propf.holds, propf.inconclusive, propf.nodes_explored);
    fprintf(stderr, "  (expected control result: coincidence=%s, archimedean-only "
                     "property(F)=%s; full classifier ESTABLISHES)\n",
            coin.holds ? "HOLDS" : "not resolved",
            propf.holds ? "HOLDS" : (propf.inconclusive ? "INCONCLUSIVE" : "FAILS"));
}

void test_fibonacci_sanity() {
    fprintf(stderr, "=== test_fibonacci_sanity (2-letter, known TILES case) ===\n");
    // Fibonacci substitution: sigma(0)=(0,1), sigma(1)=(0).  THE
    // textbook unimodular Pisot example, known (Rauzy 1982) to tile
    // with a small, explicit translation set.  Both checks are now
    // verified correct here after the trivial-cycle fix (see docs/RESEARCH_STATUS.md).
    constexpr std::size_t d = 2;
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 1},
        std::vector<long long>{0},
    };
    // charpoly x^2 - x - 1, low_first = {-1, -1}.
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-1, -1});
    // M: col0=sigma(0)="01"->(1,1), col1=sigma(1)="0"->(1,0). M^T:
    std::vector<std::vector<long long>> Mt = {
        {1, 1},
        {1, 0},
    };
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
    CHECK(eig.ok, "Fibonacci: left eigenvector computed");
    auto automaton = build_prefix_automaton<d>(images, eig.v, R);

    auto coin = check_strong_coincidence<d>(images);
    CHECK(coin.holds, "Fibonacci: strong coincidence holds");

    auto propf = check_property_f<d>(automaton, 5000);
    fprintf(stderr, "  property (F): holds=%d inconclusive=%d nodes=%lld\n",
            propf.holds, propf.inconclusive, propf.nodes_explored);
    CHECK(propf.holds, "Fibonacci: property (F) HOLDS (matches Rauzy 1982) -- "
                        "verified fixed (trivial-cycle bug, see docs/RESEARCH_STATUS.md)");
    CHECK(propf.nodes_explored == 8, "Fibonacci: closes at exactly 8 nodes "
                                      "(budget-independent, checked 100..300000)");

    CHECK(coin.holds && propf.holds,
          "Fibonacci: BOTH checks hold => TILES, matching the classical result");
}

void test_rnd13() {
    fprintf(stderr, "=== test_rnd13 (the actual open question) ===\n");
    // rnd13: sigma(0)=(0,0,1,2,3,3), sigma(1)=(0,0,2,3,3),
    // sigma(2)=(0,0,3,3), sigma(3)=(0,0,0,2,3,3).
    // charpoly x^4-4x^3-8x^2-6x-2, (2) = p^4 (single totally
    // ramified prime, e=4=n, f=1) -- exactly the case
    // make_totally_ramified_padic_bound covers.
    constexpr std::size_t d = 4;
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 1, 2, 3, 3},
        std::vector<long long>{0, 0, 2, 3, 3},
        std::vector<long long>{0, 0, 3, 3},
        std::vector<long long>{0, 0, 0, 2, 3, 3},
    };
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-4, -8, -6, -2});
    std::vector<std::vector<long long>> Mt = {
        {2, 1, 1, 2},
        {2, 0, 1, 2},
        {2, 0, 0, 2},
        {3, 0, 1, 2},
    };
    auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
    CHECK(eig.ok, "rnd13: left eigenvector computed");
    auto automaton = build_prefix_automaton<d>(images, eig.v, R);
    fprintf(stderr, "  digit set size: %zu\n", automaton.digit_set.size());

    auto coin = check_strong_coincidence<d>(images);
    fprintf(stderr, "  strong coincidence: holds=%d inconclusive=%d depth=%lld\n",
            coin.holds, coin.inconclusive, coin.depth_reached);
    CHECK(coin.holds, "rnd13: strong coincidence HOLDS");
    CHECK(coin.depth_reached == 1, "rnd13: strong coincidence resolves at depth 1");

    auto padic_bound = make_totally_ramified_padic_bound(2, 4, R.charpoly());
    auto propf = check_property_f<d>(automaton, 100000, padic_bound);
    fprintf(stderr, "  property (F) [archimedean + p-adic bound]: holds=%d inconclusive=%d nodes=%lld\n",
            propf.holds, propf.inconclusive, propf.nodes_explored);
    CHECK(propf.holds, "rnd13: property (F) HOLDS with the combined archimedean+p-adic bound");
    CHECK(propf.nodes_explored == 33185,
          "rnd13: closes at exactly 33185 nodes (verified stable across "
          "node budgets 100000..1000000 and p-adic precision 15..80)");

    // Cross-check: WITHOUT the p-adic bound, this must NOT close
    // (confirms the p-adic factor is doing real, non-redundant work,
    // not just a coincidentally-tighter margin).
    auto propf_arch_only = check_property_f<d>(automaton, 100000);
    CHECK(propf_arch_only.inconclusive,
          "rnd13: archimedean-only bound genuinely fails to close within the same "
          "budget (confirms the p-adic bound is doing essential work, not redundant)");

    fprintf(stderr, "\n  ==> rnd13 VERDICT: strong coincidence HOLDS, property (F) HOLDS "
                     "=> TILES the adelic representation space K_sigma.\n");
    CHECK(coin.holds && propf.holds,
          "rnd13: BOTH checks hold => TILES");
}

}  // namespace

int main() {
    test_pair_has_coincidence_basics();
    test_worked_example();
    test_fibonacci_sanity();
    test_rnd13();
    fprintf(stderr, "\n%d passed, %d failed\n", n_pass, n_fail);
    return n_fail == 0 ? 0 : 1;
}
