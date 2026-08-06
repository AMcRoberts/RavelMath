// Locks in the property-(F) verdict-logic correction (see the long
// comment in include/adelic/coincidence_and_property_f.hpp dated
// 2026-08-06): the failure condition is "a cycle that is not entirely
// zero-nodes", not "a cycle containing BOTH zero and nonzero nodes".
// Reproduces every historical ESTABLISHED/HOLDS case this project has
// on record and checks the node counts match exactly (not just the
// boolean verdict) -- if the graph construction ever changes
// incidentally, this will catch it.

#include <cassert>
#include <iostream>

#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/dedekind_factorization.hpp"
#include "adelic/prefix_automaton.hpp"

namespace {
std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& M) {
    std::size_t n = M.size();
    std::vector<std::vector<long long>> T(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j) T[j][i] = M[i][j];
    return T;
}
}  // namespace

int main() {
    // Fibonacci: the ground-truth case (Rauzy's classical result).
    // Pure DAG among nonzero nodes once you leave U -- must HOLD, at
    // exactly 8 nodes, unchanged from before the fix.
    {
        std::array<std::vector<long long>, 2> images = {std::vector<long long>{0, 1}, std::vector<long long>{0}};
        std::vector<std::vector<long long>> M = {{1, 1}, {1, 0}};
        auto charpoly = mathlib::charpoly_faddeev_leverrier(M);
        mathlib::QBetaRing R(charpoly);
        auto eig = mathlib::right_eigenvector_via_qbeta(transpose(M), R);
        assert(eig.ok);
        auto automaton = adelic::build_prefix_automaton<2>(images, eig.v, R);
        auto propf = adelic::check_property_f<2>(automaton, 300000);
        std::cout << "Fibonacci: holds=" << propf.holds << " nodes=" << propf.nodes_explored << "\n";
        assert(propf.holds);
        assert(propf.nodes_explored == 8);
    }

    // rnd13: the project's largest closure. Must still HOLD, at
    // exactly 33185 nodes, the documented figure.
    {
        constexpr std::size_t d = 4;
        std::array<std::vector<long long>, d> images = {
            std::vector<long long>{0, 0, 1, 2, 3, 3},
            std::vector<long long>{0, 0, 2, 3, 3},
            std::vector<long long>{0, 0, 3, 3},
            std::vector<long long>{0, 0, 0, 2, 3, 3},
        };
        std::vector<std::vector<long long>> Mt = {{2, 1, 1, 2}, {2, 0, 1, 2}, {2, 0, 0, 2}, {3, 0, 1, 2}};
        mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-4, -8, -6, -2});
        auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
        assert(eig.ok);
        auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);
        auto [bound, trusted] = adelic::make_combined_padic_bound(std::vector<long long>{2}, R.charpoly());
        (void)trusted;
        auto propf = adelic::check_property_f<d>(automaton, 300000, bound);
        std::cout << "rnd13: holds=" << propf.holds << " nodes=" << propf.nodes_explored << "\n";
        assert(propf.holds);
        assert(propf.nodes_explored == 33185);
    }

    // x^2-2x-2: simplest known non-unit Pisot substitution. Must HOLD
    // at exactly 47 nodes (docs/SMALLEST_NONUNIT_PISOT_2026-08-05.md).
    {
        std::array<std::vector<long long>, 2> images = {std::vector<long long>{0, 0, 1}, std::vector<long long>{0, 0}};
        mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-2, -2});
        // Matches tests/simplest_nonunit_pisot_adelic_tiling_test.cpp exactly:
        // M itself (matrix[destination][source] convention) is passed as
        // classify_tiling's "M_transpose" argument there -- reproduced
        // verbatim here, not re-derived, since the eigenvector routine's
        // expected orientation is a convention fixed by that call site.
        std::vector<std::vector<long long>> Mt = {{2, 2}, {1, 0}};
        auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
        assert(eig.ok);
        auto automaton = adelic::build_prefix_automaton<2>(images, eig.v, R);
        auto [bound, trusted] = adelic::make_combined_padic_bound(std::vector<long long>{2}, R.charpoly());
        (void)trusted;
        auto propf = adelic::check_property_f<2>(automaton, 300000, bound);
        std::cout << "x^2-2x-2: holds=" << propf.holds << " nodes=" << propf.nodes_explored << "\n";
        assert(propf.holds);
        assert(propf.nodes_explored == 47);
    }

    std::cout << "property_f_correct_verdict: all historical ground-truth cases reproduce "
                 "identically under the corrected verdict logic.\n";
    return 0;
}
