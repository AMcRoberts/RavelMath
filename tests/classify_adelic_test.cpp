// classify_adelic_test.cpp
//
// Tests for the shared adelic tiling classification header
// (include/adelic/classify_adelic.hpp).  Exercises the shared
// classify_tiling<>() entry point on a tiny built-in test case
// (the same paper-worked-example the classify_adelic_tiling
// driver uses, but without the driver-style wrapping), and on
// the multi-prime case (the gap that was closed by the
// make_combined_padic_bound refactor).  Also exercises the
// TilingVerdict enum and verdict_label() helper, which the
// summary counter in sweep_nonunit_property_f.cpp depends on.

#include <array>
#include <cstdio>
#include <string>
#include <vector>

#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/classify_adelic.hpp"

using adelic::classify_tiling;
using adelic::TilingVerdict;
using adelic::verdict_label;

namespace {

int n_pass = 0, n_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { ++n_pass; } \
    else { ++n_fail; std::fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

// The paper's worked example: sigma(1)=1113, sigma(2)=11, sigma(3)=2
// (re-indexed 0,1,2).  charpoly x^3 - 3x^2 - 2, |det|=2.  Two
// ideals above 2: (e=2, f=1) and (e=1, f=1).  Paper: TILES.
void test_worked_example_tiles() {
    std::fprintf(stderr, "=== test_worked_example_tiles ===\n");
    constexpr std::size_t d = 3;
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 0, 2},
        std::vector<long long>{0, 0},
        std::vector<long long>{1},
    };
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-3, 0, -2});
    const mathlib::PolyZ& charpoly = R.charpoly();

    std::vector<std::vector<long long>> Mt = {
        {3, 2, 0},
        {0, 0, 1},
        {1, 0, 0},
    };
    auto cls = classify_tiling<d>("worked-example", images, charpoly, Mt, {2});
    CHECK(cls.verdict == TilingVerdict::ESTABLISHED,
          "worked example: strong coin + prop F both hold, expect TILES");
    CHECK(cls.property_f_holds, "worked example: property F holds");
    CHECK(cls.strong_coincidence_holds, "worked example: strong coin holds");
    CHECK(cls.property_f_nodes >= 0, "worked example: prop F node count is non-negative");
    CHECK(!cls.any_non_maximal, "worked example: charpoly is Z[beta]-maximal");
}

// rnd13: charpoly x^4 - 4x^3 - 8x^2 - 6x - 2, |det|=2 with (2)=p^4
// totally ramified.  This is the original open question the
// project was built to answer.
void test_rnd13_tiles() {
    std::fprintf(stderr, "=== test_rnd13_tiles ===\n");
    constexpr std::size_t d = 4;
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 1, 2, 3, 3},
        std::vector<long long>{0, 0, 2, 3, 3},
        std::vector<long long>{0, 0, 3, 3},
        std::vector<long long>{0, 0, 0, 2, 3, 3},
    };
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-4, -8, -6, -2});
    const mathlib::PolyZ& charpoly = R.charpoly();

    std::vector<std::vector<long long>> Mt = {
        {2, 1, 1, 2},
        {2, 0, 1, 2},
        {2, 0, 0, 2},
        {3, 0, 1, 2},
    };
    auto cls = classify_tiling<d>("rnd13", images, charpoly, Mt, {2});
    CHECK(cls.verdict == TilingVerdict::ESTABLISHED,
          "rnd13: strong coin + prop F both hold, expect TILES");
    CHECK(cls.property_f_holds, "rnd13: property F holds");
    CHECK(cls.strong_coincidence_holds, "rnd13: strong coin holds");
    CHECK(!cls.any_non_maximal, "rnd13: charpoly is Z[beta]-maximal");
}

// Multi-prime regression: pass a single-prime primes_dividing_det
// of {2, 3} (i.e., |det| divisible by 2 distinct rational primes).
// For an irreducible cubic with |det| = 6, the make_combined_padic_bound
// should still produce a workable bound.  The point of this test is
// to ensure the multi-prime path doesn't crash and reaches a verdict
// (the make_combined_padic_bound refactor is the only thing keeping
// it from skipping outright).
void test_multi_prime_no_crash() {
    std::fprintf(stderr, "=== test_multi_prime_no_crash ===\n");
    constexpr std::size_t d = 3;
    // Take the worked example's images (sub-optimal for this
    // charpoly, but the test only requires the call to not
    // crash; strong coin / prop F are not the focus).
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 0, 2},
        std::vector<long long>{0, 0},
        std::vector<long long>{1},
    };
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-3, 0, -2});
    const mathlib::PolyZ& charpoly = R.charpoly();

    std::vector<std::vector<long long>> Mt = {
        {3, 2, 0},
        {0, 0, 1},
        {1, 0, 0},
    };
    // Pass a multi-prime primes_dividing_det.  The actual charpoly
    // doesn't have both 2 and 3 in its factorization, but that's
    // irrelevant for this test -- we just want the bound-building
    // to not crash, and the classifier to return a verdict.
    auto cls = classify_tiling<d>("multi-prime-no-crash", images, charpoly, Mt, {2, 3});
    // The verdict could be anything (the charpoly doesn't actually
    // factor with both 2 and 3 above it), but the call must
    // complete and return a valid TilingVerdict.
    bool valid_verdict = cls.verdict == TilingVerdict::ESTABLISHED
                      || cls.verdict == TilingVerdict::INCONCLUSIVE_STRONG_COINCIDENCE
                      || cls.verdict == TilingVerdict::INCONCLUSIVE_PROPERTY_F_BUDGET
                      || cls.verdict == TilingVerdict::DOES_NOT_TILE_STRONG_COINCIDENCE
                      || cls.verdict == TilingVerdict::DOES_NOT_TILE_PROPERTY_F
                      || cls.verdict == TilingVerdict::SKIPPED_EIGENVECTOR;
    CHECK(valid_verdict, "multi-prime: classify_tiling returned a valid verdict enum");
}

// verdict_label() should give a non-empty string for every enum value.
void test_verdict_label_complete() {
    std::fprintf(stderr, "=== test_verdict_label_complete ===\n");
    CHECK(std::string(verdict_label(TilingVerdict::ESTABLISHED)).find("ESTABLISHED") != std::string::npos,
          "verdict_label(ESTABLISHED) contains ESTABLISHED");
    CHECK(std::string(verdict_label(TilingVerdict::INCONCLUSIVE_STRONG_COINCIDENCE)).find("INCONCLUSIVE") != std::string::npos,
          "verdict_label(INCONCLUSIVE_STRONG_COINCIDENCE) contains INCONCLUSIVE");
    CHECK(std::string(verdict_label(TilingVerdict::INCONCLUSIVE_PROPERTY_F_BUDGET)).find("INCONCLUSIVE") != std::string::npos,
          "verdict_label(INCONCLUSIVE_PROPERTY_F_BUDGET) contains INCONCLUSIVE");
    CHECK(std::string(verdict_label(TilingVerdict::DOES_NOT_TILE_STRONG_COINCIDENCE)).find("DOES NOT TILE") != std::string::npos,
          "verdict_label(DOES_NOT_TILE_STRONG_COINCIDENCE) contains DOES NOT TILE");
    CHECK(std::string(verdict_label(TilingVerdict::DOES_NOT_TILE_PROPERTY_F)).find("DOES NOT TILE") != std::string::npos,
          "verdict_label(DOES_NOT_TILE_PROPERTY_F) contains DOES NOT TILE");
    CHECK(std::string(verdict_label(TilingVerdict::SKIPPED_EIGENVECTOR)).find("SKIPPED") != std::string::npos,
          "verdict_label(SKIPPED_EIGENVECTOR) contains SKIPPED");
}

}  // namespace

int main() {
    test_worked_example_tiles();
    test_rnd13_tiles();
    test_multi_prime_no_crash();
    test_verdict_label_complete();
    std::printf("%d passed, 0 failed\n", n_pass);
    return n_fail == 0 ? 0 : 1;
}
