// Stress-tests the claim in ravel/proof/property_f_unconditional.hpp
// -- no nonzero-gamma node can ever have an edge into a zero-gamma
// node -- against the REAL, trusted check_property_f (via its new
// out_zero_nodes_beyond_frontier diagnostic parameter), not a
// reimplemented copy. 8 structurally diverse cases: Finding 5's whole
// a=0..5 family (AR-exact through non-AR, 3-letter, unimodular), the
// simplest known non-unit Pisot substitution (x^2-2x-2, 2-letter),
// and rnd13 (the project's flagship 4-letter non-unimodular case,
// 33185 nodes -- the largest closure this project has ever run).
//
// If this ever fails on some future substitution, it falsifies the
// theorem and should be treated as a serious finding in its own
// right, not silently patched around.

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

std::vector<std::vector<long long>> sigma_ab(int a, int b) {
    std::vector<std::vector<long long>> s(3);
    for (int i = 0; i < a; ++i) s[0].push_back(0);
    for (int i = 0; i < b; ++i) s[0].push_back(1);
    s[0].push_back(2);
    for (int i = 0; i < a; ++i) s[1].push_back(0);
    s[1].push_back(2);
    s[2] = {0};
    return s;
}

void check_zero_nodes(const char* name, long long expected_min_nodes) {
    (void)expected_min_nodes;
    std::cout << name << ": ";
}

}  // namespace

int main() {
    // Finding 5's whole family, a=0 (non-AR) through a=5 (AR-partial).
    for (int a = 0; a <= 5; ++a) {
        auto sigma = sigma_ab(a, 1);
        std::array<std::vector<long long>, 3> images = {sigma[0], sigma[1], sigma[2]};
        std::vector<std::vector<long long>> M(3, std::vector<long long>(3, 0));
        for (std::size_t j = 0; j < 3; ++j)
            for (long long letter : sigma[j]) M[static_cast<std::size_t>(letter)][j] += 1;
        auto charpoly = mathlib::charpoly_faddeev_leverrier(M);
        mathlib::QBetaRing R(charpoly);
        auto eig = mathlib::right_eigenvector_via_qbeta(transpose(M), R);
        assert(eig.ok);
        auto automaton = adelic::build_prefix_automaton<3>(images, eig.v, R);
        long long zero_beyond = -1;
        auto propf = adelic::check_property_f<3>(automaton, 300000, nullptr, nullptr, nullptr, &zero_beyond);
        std::cout << "sigma_{" << a << ",1}: holds=" << propf.holds
                  << " nodes=" << propf.nodes_explored
                  << " zero_nodes_beyond_frontier=" << zero_beyond << "\n";
        assert(propf.holds);
        assert(zero_beyond == 0);
    }

    // Simplest known non-unit Pisot substitution: x^2-2x-2.
    {
        std::array<std::vector<long long>, 2> images = {std::vector<long long>{0, 0, 1}, std::vector<long long>{0, 0}};
        mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-2, -2});
        std::vector<std::vector<long long>> Mt = {{2, 1}, {2, 0}};
        auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
        assert(eig.ok);
        auto automaton = adelic::build_prefix_automaton<2>(images, eig.v, R);
        auto [bound, trusted] = adelic::make_combined_padic_bound(std::vector<long long>{2}, R.charpoly());
        (void)trusted;
        long long zero_beyond = -1;
        auto propf = adelic::check_property_f<2>(automaton, 300000, bound, nullptr, nullptr, &zero_beyond);
        std::cout << "x^2-2x-2: holds=" << propf.holds << " nodes=" << propf.nodes_explored
                  << " zero_nodes_beyond_frontier=" << zero_beyond << "\n";
        assert(propf.holds);
        assert(zero_beyond == 0);
    }

    // rnd13: 4-letter non-unimodular, the project's largest closure (33185 nodes).
    {
        constexpr std::size_t d = 4;
        std::array<std::vector<long long>, d> images = {
            std::vector<long long>{0, 0, 1, 2, 3, 3},
            std::vector<long long>{0, 0, 2, 3, 3},
            std::vector<long long>{0, 0, 3, 3},
            std::vector<long long>{0, 0, 0, 2, 3, 3},
        };
        std::vector<std::vector<long long>> Mt = {
            {2, 1, 1, 2}, {2, 0, 1, 2}, {2, 0, 0, 2}, {3, 0, 1, 2},
        };
        mathlib::QBetaRing R = mathlib::QBetaRing::from_low_first({-4, -8, -6, -2});
        auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
        assert(eig.ok);
        auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, R);
        auto [bound, trusted] = adelic::make_combined_padic_bound(std::vector<long long>{2}, R.charpoly());
        (void)trusted;
        long long zero_beyond = -1;
        auto propf = adelic::check_property_f<d>(automaton, 300000, bound, nullptr, nullptr, &zero_beyond);
        std::cout << "rnd13: holds=" << propf.holds << " nodes=" << propf.nodes_explored
                  << " zero_nodes_beyond_frontier=" << zero_beyond << "\n";
        assert(propf.holds);
        assert(propf.nodes_explored == 33185);  // matches the project's documented figure exactly
        assert(zero_beyond == 0);
    }

    std::cout << "\nproperty_f_unconditional: zero_nodes_beyond_frontier == 0 in all 8 cases, "
                 "exactly as the Perron-positivity proof predicts. Confirmed against the real, "
                 "trusted check_property_f, not a reimplemented copy.\n";
    return 0;
}
