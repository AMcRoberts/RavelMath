// tests/simplest_nonunit_pisot_adelic_tiling_test.cpp
//
// The question the transport-generator closure (Q/R/S collapse) in
// simplest_nonunit_pisot_closure.hpp does NOT answer: does the
// substitution actually TILE its representation space? For a unit
// Pisot substitution the archimedean (real/complex contracting) space
// alone decides this; for a non-unit one -- x^2-2x-2 is the simplest
// possible non-unit Pisot substitution this project has derived -- the
// classical Pisot-conjecture machinery (strong coincidence + geometric
// property (F)) needs the additional adelic/p-adic representation space
// (`include/adelic/*`, built for exactly this situation) because the
// archimedean space alone no longer has the right covolume.
//
// x^2-2x-2 has |det|=|norm|=2, and 2 is totally ramified in Z[beta]
// ((2) = p^2, e=2, f=1, ef=2=degree) -- confirmed here via
// `factor_prime_in_qbeta` + `cross_check_dedekind_factorization`, not
// assumed.

#include <array>
#include <cassert>
#include <cstdio>
#include <vector>

#include "math/qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/classify_adelic.hpp"

using adelic::classify_tiling;
using adelic::TilingVerdict;
using adelic::verdict_label;

int main() {
    constexpr std::size_t d = 2;
    // 0->001, 1->00 (see simplest_nonunit_pisot_substitution.hpp).
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 1},
        std::vector<long long>{0, 0},
    };
    // matrix[destination][source] convention, matching
    // ReturnPhaseSystem::incidence_matrix() elsewhere in this project.
    std::vector<std::vector<long long>> M = {{2, 2}, {1, 0}};

    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-2, -2});
    const mathlib::PolyZ& charpoly = R.charpoly();

    auto fac = adelic::factor_prime_in_qbeta(charpoly, 2);
    bool cross_ok = adelic::cross_check_dedekind_factorization(fac, charpoly, (long long)d);
    std::printf("Factoring (2): maximal=%s cross-checked=%s\n",
                fac.maximal ? "YES" : "NO", cross_ok ? "MATCH" : "MISMATCH");
    for (const auto& pi : fac.prime_ideals) {
        std::printf("  p=%lld e=%lld f=%lld ef=%lld\n", pi.p, pi.e, pi.f, pi.e * pi.f);
    }
    assert(fac.maximal);
    assert(cross_ok);
    assert(fac.prime_ideals.size() == 1);
    assert(fac.prime_ideals[0].e == 2 && fac.prime_ideals[0].f == 1);  // totally ramified

    auto cls = classify_tiling<d>("x^2-2x-2 (simplest non-unit Pisot substitution)",
                                  images, charpoly, M, {2});
    std::printf("Strong coincidence: %s (depth %lld)\n",
                cls.strong_coincidence_holds ? "HOLDS"
                    : cls.strong_coincidence_inconclusive ? "INCONCLUSIVE" : "FAILS",
                cls.strong_coincidence_depth);
    std::printf("Property (F): %s (nodes %lld)\n",
                cls.property_f_holds ? "HOLDS"
                    : cls.property_f_inconclusive ? "INCONCLUSIVE" : "FAILS",
                cls.property_f_nodes);
    std::printf("VERDICT: %s\n", verdict_label(cls.verdict));

    assert(cls.strong_coincidence_holds);
    assert(cls.property_f_holds);
    assert(cls.verdict == TilingVerdict::ESTABLISHED);
    std::printf("PASS: the simplest non-unit Pisot substitution's adelic representation "
                "space is a genuine simple tiling (Pisot conjecture established).\n");
}
