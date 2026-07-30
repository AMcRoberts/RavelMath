// app/rnd13_prefix_automaton_probe.cpp
//
// Adelic tiling plan (docs/ADELIC_TILING_PLAN.md) §4, step 3, applied
// to the 4-letter rnd13 substitution: build the actual prefix
// automaton and digit set, using the real machinery in
// include/adelic/prefix_automaton.hpp (not the degree-3 worked
// example prefix_automaton_test.cpp already exercises).
//
// rnd13: sigma(0)=(0,0,1,2,3,3), sigma(1)=(0,0,2,3,3),
//        sigma(2)=(0,0,3,3), sigma(3)=(0,0,0,2,3,3)
// charpoly: x^4 - 4x^3 - 8x^2 - 6x - 2 (irreducible, confirmed via
// SymPy; also rnd13_factor_probe.cpp confirms Z[beta] is 2-maximal).
//
// The left Perron eigenvector v (needed for delta(p) = <P(p), v>)
// is the LEFT eigenvector of the incidence matrix M, i.e. v^T M =
// beta v^T -- computed here by calling right_eigenvector_via_qbeta
// on M^T, per prefix_automaton.hpp's own documented convention.

#include <array>
#include <cstdio>
#include <vector>

#include "math/qbeta.hpp"
#include "math/linalg_qbeta.hpp"
#include "adelic/prefix_automaton.hpp"

int main() {
    using mathlib::QBetaRing;
    using mathlib::right_eigenvector_via_qbeta;
    using mathlib::str;

    constexpr std::size_t d = 4;

    // sigma images.
    std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 1, 2, 3, 3},
        std::vector<long long>{0, 0, 2, 3, 3},
        std::vector<long long>{0, 0, 3, 3},
        std::vector<long long>{0, 0, 0, 2, 3, 3},
    };

    // charpoly x^4 - 4x^3 - 8x^2 - 6x - 2, low-first coeffs
    // {a0,a1,a2,a3} with x^4 + a0 x^3 + a1 x^2 + a2 x + a3.
    QBetaRing R = QBetaRing::from_low_first({-4, -8, -6, -2});

    // Incidence matrix M (M[i][j] = count of letter i in sigma(j)),
    // transposed (right_eigenvector_via_qbeta(M^T, R) gives the LEFT
    // eigenvector of M, per prefix_automaton.hpp's documented
    // convention).
    std::vector<std::vector<long long>> Mt = {
        {2, 1, 1, 2},
        {2, 0, 1, 2},
        {2, 0, 0, 2},
        {3, 0, 1, 2},
    };

    auto eig = right_eigenvector_via_qbeta(Mt, R);
    if (!eig.ok) {
        printf("FAILED to compute left eigenvector.\n");
        return 1;
    }
    printf("Left Perron eigenvector v (Q(beta), v[3]=1):\n");
    for (std::size_t i = 0; i < d; ++i) {
        printf("  v[%zu] = %s\n", i, str(eig.v[i]).c_str());
    }

    auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);

    printf("\nPrefix automaton edges (source, prefix, target):\n");
    for (std::size_t c = 0; c < d; ++c) {
        for (const auto& [target, prefix] : automaton.by_source[c]) {
            printf("  %zu --", c);
            for (auto x : prefix) printf("%lld", x);
            printf("--> %lld\n", target);
        }
    }

    printf("\nDistinct prefixes: %zu\n", automaton.distinct_prefixes.size());
    printf("Digit set D (size %zu):\n", automaton.digit_set.size());
    for (std::size_t i = 0; i < automaton.distinct_prefixes.size(); ++i) {
        printf("  prefix=");
        if (automaton.distinct_prefixes[i].empty()) printf("(empty)");
        for (auto x : automaton.distinct_prefixes[i]) printf("%lld", x);
        printf("  delta(p) = %s\n", str(automaton.digit_set[i]).c_str());
    }

    printf("\nDigit set is finite (size %zu), as required by the adelic\n"
           "classifier (Minervino-Thuswaldner Sec 2.3).\n",
           automaton.digit_set.size());

    return 0;
}
