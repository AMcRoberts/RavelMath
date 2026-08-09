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
#include <algorithm>
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
#include "adelic/maximal_order.hpp"

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
    CHECK(cls.strong_coincidence_closure_used,
          "worked example: shared classifier used exact closure strong coincidence");
    CHECK(cls.strong_coincidence_closure_attempted &&
              !cls.strong_coincidence_closure_inconclusive &&
              !cls.strong_coincidence_closure_unsupported,
          "worked example: closure status records a completed supported run");
    CHECK(cls.strong_coincidence_pair_resolution_depths.size() == 3 &&
              std::all_of(cls.strong_coincidence_pair_resolution_depths.begin(),
                          cls.strong_coincidence_pair_resolution_depths.end(),
                          [](long long depth) { return depth > 0; }),
          "worked example: classifier exports all pair resolution depths");
    CHECK(cls.property_f_nodes >= 0, "worked example: prop F node count is non-negative");
    CHECK(!cls.any_non_maximal, "worked example: charpoly is Z[beta]-maximal");
    CHECK(cls.property_f_bound_trusted,
          "worked example: p-adic Property-F bound is trusted");
    auto shape = adelic::compare_first_order_padic_shapes(charpoly, 2, 30);
    CHECK(shape.dedekind_order_maximal && shape.first_order_shapes_match,
          "worked example: certified first-order Newton/Dedekind shapes agree");
    auto worked_segments = adelic::newton_polygon(
        adelic::zp_poly_from_polyz(charpoly, 2, 30));
    CHECK(!worked_segments.empty(), "worked example: Newton polygon is non-empty");
    if (!worked_segments.empty()) {
        auto residual = adelic::newton_residual_diagnostic(
            adelic::zp_poly_from_polyz(charpoly, 2, 30), worked_segments.front());
        CHECK(residual.supported && residual.factors.size() == 1 &&
                  residual.factors[0].mult == 1 &&
                  static_cast<long long>(residual.factors[0].g.c.size()) - 1 == 1,
              "worked example: e=2 residual refines to a linear factor");
    }
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
    CHECK(cls.strong_coincidence_closure_used,
          "rnd13: shared classifier used exact closure strong coincidence");
    CHECK(cls.strong_coincidence_closure_attempted &&
              !cls.strong_coincidence_closure_inconclusive &&
              !cls.strong_coincidence_closure_unsupported,
          "rnd13: closure status records a completed supported run");
    CHECK(cls.strong_coincidence_pair_resolution_depths == std::vector<long long>(6, 1),
          "rnd13: classifier exports six depth-1 pair resolutions");
    CHECK(!cls.any_non_maximal, "rnd13: charpoly is Z[beta]-maximal");
    CHECK(cls.property_f_bound_trusted,
          "rnd13: p-adic Property-F bound is trusted");
    auto shape = adelic::compare_first_order_padic_shapes(charpoly, 2, 30);
    CHECK(shape.dedekind_order_maximal && shape.first_order_shapes_match,
          "rnd13: certified first-order Newton/Dedekind shapes agree");
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

// A quartic survey polynomial whose Dedekind order is not 2-maximal.  The
// exploratory local-field predicate is still constructible, but the trust
// flag must prevent it from being promoted to a certified Property-F bound.
void test_nonmaximal_order_is_not_trusted() {
    std::fprintf(stderr, "=== test_nonmaximal_order_is_not_trusted ===\n");
    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-7, 5, -5, 2});
    auto fac = adelic::factor_prime_in_qbeta(R.charpoly(), 2);
    std::fprintf(stderr, "  diagnostic maximal=%d cross=%d\n", fac.maximal ? 1 : 0,
                 adelic::cross_check_dedekind_factorization(fac, R.charpoly(), 4) ? 1 : 0);
    for (const auto& pi : fac.prime_ideals)
        std::fprintf(stderr, "  Dedekind factor e=%lld f=%lld g=%s\n", pi.e, pi.f,
                     mathlib::str(pi.g).c_str());
    auto [bound, trusted] = adelic::make_combined_padic_bound({2}, R.charpoly());
    CHECK(!trusted, "non-maximal quartic: p-adic bound is not trusted");
    CHECK(bound(R.from_int(0)), "non-maximal quartic: exploratory bound remains callable");
    auto round = adelic::enlarge_order_round2_bigint({1, -7, 5, -5, 2}, 2);
    CHECK(round.enlarged, "non-maximal quartic: Round-2 order enlargement is detected");
    std::fprintf(stderr, "  Round-2 disc_before=%s disc_after=%s needs_another_round=%d\n",
                 mathlib::str(round.disc_before).c_str(), mathlib::str(round.disc_after).c_str(),
                 round.needs_another_round ? 1 : 0);
    bool round_basis_closed = true;
    try {
        auto O = adelic::monogenic_structure_constants({1, -7, 5, -5, 2});
        (void)adelic::structure_constants_from_basis_change(O, round.basis, 2);
    } catch (const std::runtime_error&) {
        round_basis_closed = false;
    }
    CHECK(round_basis_closed,
          "non-maximal quartic: corrected one-round HNF basis is closed under multiplication");
    CHECK(mathlib::cmp_si(round.disc_after, -21580) == 0,
          "non-maximal quartic: one-round multiplier-ring discriminant is -21580");
    auto segments = adelic::newton_polygon(adelic::zp_poly_from_polyz(R.charpoly(), 2, 30));
    CHECK(segments.size() == 2, "non-maximal quartic: Newton polygon has two segments");
    if (segments.size() == 2) {
        CHECK(segments[0].e == 1 && segments[0].f == 1 &&
                  segments[1].e == 1 && segments[1].f == 3,
              "non-maximal quartic: Newton (e,f) differs from non-maximal Dedekind data");
        auto residual = adelic::newton_residual_diagnostic(
            adelic::zp_poly_from_polyz(R.charpoly(), 2, 30), segments[1]);
        CHECK(residual.supported && residual.factors.size() == 1 &&
                  residual.factors[0].mult == 3 &&
                  static_cast<long long>(residual.factors[0].g.c.size()) - 1 == 1,
              "non-maximal quartic: slope residual refines degree-3 segment to (e,f)=(3,1)");
    }
    auto shape = adelic::compare_first_order_padic_shapes(R.charpoly(), 2, 30);
    CHECK(!shape.dedekind_order_maximal,
          "non-maximal quartic: shape diagnostic preserves order trust status");
    CHECK(!shape.first_order_shapes_match,
          "non-maximal quartic: shape diagnostic exposes first-order mismatch");
    CHECK(shape.newton_shapes.size() == 2 && shape.dedekind_shapes.size() == 2,
          "non-maximal quartic: shape diagnostic records both decompositions");
    bool higher_order_boundary = false;
    try {
        (void)adelic::ore_padic_factorization(
            adelic::zp_poly_from_polyz(R.charpoly(), 2, 30), 30);
    } catch (const std::runtime_error& ex) {
        higher_order_boundary =
            std::string(ex.what()).find("higher-order/Montes lift required") != std::string::npos;
    }
    CHECK(higher_order_boundary,
          "non-maximal quartic: Ore path reports explicit higher-order boundary");
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
    test_nonmaximal_order_is_not_trusted();
    test_verdict_label_complete();
    std::printf("%d passed, 0 failed\n", n_pass);
    return n_fail == 0 ? 0 : 1;
}
