// app/classify_adelic_tiling.cpp
//
// Driver for the two regression cases the project has been
// specifically interested in:
//   1. The paper's own worked example, sigma(1)=1113, sigma(2)=11,
//      sigma(3)=2 (re-indexed 0,1,2), charpoly x^3-3x^2-2,
//      |det|=2.  Paper's stated verdict: TILES.
//   2. rnd13: the 4-letter non-unimodular Pisot substitution that
//      motivates the whole adelic-tiling story, charpoly
//      x^4-4x^3-8x^2-6x-2, |det|=2 with (2) = p^4 totally
//      ramified.
//
// All the actual classification logic lives in the shared header
// `adelic/classify_adelic.hpp` -- this driver is just the
// case-by-case invocation and its long-rule header style.  See
// app/sweep_nonunit_property_f.cpp for the same shared logic
// invoked from a different (short-dash header) style.

#include <array>
#include <cstdio>
#include <vector>

#include "math/qbeta.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/ideal_arithmetic.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/classify_adelic.hpp"

using adelic::classify_tiling;
using adelic::HeaderStyle;
using adelic::print_case_header;
using adelic::TilingVerdict;
using adelic::verdict_label;

namespace {

template <std::size_t d>
void classify(const char* name,
              const std::array<std::vector<long long>, d>& images,
              std::initializer_list<long long> charpoly_low_first,
              const std::vector<std::vector<long long>>& M_transpose,
              const std::vector<long long>& primes_dividing_det) {
    print_case_header(HeaderStyle::LONG_RULE, name);

    mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first(charpoly_low_first);
    const mathlib::PolyZ& charpoly = R.charpoly();
    std::printf("charpoly: %s\n", mathlib::str(charpoly).c_str());
    std::printf("(irreducibility over Q assumed/externally verified -- see "
                "rnd13_factor_probe.cpp for the SymPy cross-check pattern; "
                "this driver does not re-derive it)\n\n");

    // Per-prime factorization + cross-check diagnostic (this driver
    // prints it in the long-rule style; the short-dash style in
    // sweep_nonunit_property_f.cpp is a different layout but the
    // same data).
    for (long long p : primes_dividing_det) {
        auto fac = adelic::factor_prime_in_qbeta(charpoly, p);
        bool cross_ok = adelic::cross_check_dedekind_factorization(fac, charpoly, static_cast<long long>(d));
        std::printf("Factoring (%lld): maximal=%s, cross-checked=%s\n", p,
                    fac.maximal ? "YES" : "NO", cross_ok ? "MATCH" : "MISMATCH");
        for (const auto& pi : fac.prime_ideals) {
            std::printf("  p=%lld e=%lld f=%lld ef=%lld g(x)=%s\n",
                        pi.p, pi.e, pi.f, pi.e * pi.f, mathlib::str(pi.g).c_str());
        }
    }
    std::printf("\n");

    auto cls = classify_tiling<d>(name, images, charpoly, M_transpose, primes_dividing_det);

    // The shared header can't print the prefix-automaton size (it
    // would have to know about the implementation detail), so we
    // build the automaton here just for the diagnostic line.  This
    // is a small redundancy -- classify_tiling built and used the
    // automaton internally and then dropped it.  If the automaton
    // becomes expensive to build, factor that step out into the
    // shared header too.
    {
        std::vector<std::vector<long long>> Mt = M_transpose;
        auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
        if (eig.ok) {
            auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);
            std::printf("Prefix automaton: %zu distinct prefixes, digit set size %zu.\n\n",
                        automaton.distinct_prefixes.size(), automaton.digit_set.size());
        }
    }

    // Property-(F) line.
    const char* bound_descriptor = primes_dividing_det.empty()
        ? "archimedean-only, VALIDATED shape (unit case)"
        : (primes_dividing_det.size() == 1
              ? "archimedean + combined p-adic bound, VALIDATED shape (unified local_field.hpp)"
              : "archimedean + combined p-adic bound across multiple primes, VALIDATED shape (unified local_field.hpp)");
    (void)bound_descriptor;  // we still print the unified line below

    std::printf("Strong coincidence: %s (depth reached: %lld%s)\n",
                cls.strong_coincidence_holds ? "HOLDS"
                    : cls.strong_coincidence_inconclusive ? "INCONCLUSIVE" : "FAILS",
                cls.strong_coincidence_depth,
                cls.strong_coincidence_inconclusive ? ", cutoff hit before resolving" : "");
    std::printf("Geometric property (F) [%s]: %s (nodes explored: %lld%s)\n\n",
                primes_dividing_det.empty() ? "archimedean-only, VALIDATED shape (unit case)"
                    : (primes_dividing_det.size() == 1
                          ? "archimedean + combined p-adic bound, VALIDATED shape (unified local_field.hpp)"
                          : "archimedean + combined p-adic bound across multiple primes, VALIDATED shape (unified local_field.hpp)"),
                cls.property_f_holds ? "HOLDS" : (cls.property_f_inconclusive ? "INCONCLUSIVE" : "FAILS"),
                cls.property_f_nodes,
                cls.property_f_inconclusive ? ", node budget exhausted" : "");

    // Verdict + reason.
    std::printf("VERDICT: %s\n", verdict_label(cls.verdict));
    const char* reason = nullptr;
    switch (cls.verdict) {
        case TilingVerdict::ESTABLISHED:
            reason = "strong coincidence HOLDS and geometric property (F) HOLDS, both under "
                     "validated bounds -- per plan §2.6, this is a genuine simple tiling of K_sigma";
            break;
        case TilingVerdict::INCONCLUSIVE_STRONG_COINCIDENCE:
            reason = "strong coincidence search did not resolve within the depth/length cutoff";
            break;
        case TilingVerdict::INCONCLUSIVE_PROPERTY_F_BUDGET:
            reason = "strong coincidence holds and property (F) has a validated bound for this "
                     "shape, but the search did not close within the node budget";
            break;
        case TilingVerdict::DOES_NOT_TILE_STRONG_COINCIDENCE:
            reason = "strong coincidence definitively fails (both conditions are "
                     "individually necessary for a simple tiling, per plan §2.6)";
            break;
        case TilingVerdict::DOES_NOT_TILE_PROPERTY_F:
            reason = "strong coincidence holds but geometric property (F) definitively FAILS "
                     "(under a validated bound) -- both conditions are necessary, so this fails "
                     "the sufficient condition for a simple tiling";
            break;
        case TilingVerdict::SKIPPED_EIGENVECTOR:
            reason = "left Perron eigenvector could not be computed; pipeline aborted early";
            break;
    }
    std::printf("Reason: %s\n", reason);
    if (cls.any_non_maximal) {
        std::printf("CAVEAT: at least one prime's factorization was reported non-maximal "
                    "-- the factorization used above may not be O_K's true factorization; "
                    "see dedekind_factorization.hpp.\n");
    }
    std::printf("\n");
    (void)bound_descriptor;  // silence unused-variable
}

}  // namespace

int main() {
    // Regression case first, per ADELIC_TILING_PLAN.md §6: the
    // paper's own worked example.
    {
        constexpr std::size_t d = 3;
        std::array<std::vector<long long>, d> images = {
            std::vector<long long>{0, 0, 0, 2},
            std::vector<long long>{0, 0},
            std::vector<long long>{1},
        };
        std::vector<std::vector<long long>> Mt = {
            {3, 2, 0},
            {0, 0, 1},
            {1, 0, 0},
        };
        classify<d>("worked example (paper Sec 10.2; expected TILES)", images,
                    {-3, 0, -2}, Mt, {2});
    }

    // rnd13: the 4-letter non-unimodular Pisot substitution that
    // motivates the whole adelic-tiling story.
    {
        constexpr std::size_t d = 4;
        std::array<std::vector<long long>, d> images = {
            std::vector<long long>{0, 0, 1, 2, 3, 3},
            std::vector<long long>{0, 0, 2, 3, 3},
            std::vector<long long>{0, 0, 3, 3},
            std::vector<long long>{0, 0, 0, 2, 3, 3},
        };
        std::vector<std::vector<long long>> Mt = {
            {2, 1, 1, 2},
            {2, 0, 1, 2},
            {2, 0, 0, 2},
            {3, 0, 1, 2},
        };
        classify<d>("rnd13 (closed: TILES the adelic representation space)", images,
                    {-4, -8, -6, -2}, Mt, {2});
    }

    return 0;
}
