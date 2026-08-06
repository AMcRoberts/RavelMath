// Confirms FACT 1 (nonempty prefix => strictly nonzero digit) against
// the real, computed prefix automaton for the whole Finding 5
// AR-partial family (a=1..5) and the non-AR control (a=0), not just
// asserted from the Perron-Frobenius argument in the header comment.

#include <cassert>
#include <iostream>

#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "adelic/prefix_automaton.hpp"
#include "ravel/proof/constant_factor_gives_nothing_for_property_f.hpp"

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

std::vector<std::vector<long long>> transpose(const std::vector<std::vector<long long>>& M) {
    std::size_t n = M.size();
    std::vector<std::vector<long long>> T(n, std::vector<long long>(n, 0));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j) T[j][i] = M[i][j];
    return T;
}

int main() {
    for (int a = 0; a <= 5; ++a) {
        auto sigma = sigma_ab(a, 1);
        std::array<std::vector<long long>, 3> images = {sigma[0], sigma[1], sigma[2]};
        std::vector<std::vector<long long>> M(3, std::vector<long long>(3, 0));
        for (std::size_t j = 0; j < 3; ++j)
            for (long long letter : sigma[j]) M[static_cast<std::size_t>(letter)][j] += 1;
        auto charpoly = mathlib::charpoly_faddeev_leverrier(M);
        mathlib::QBetaRing R(charpoly);
        auto Mt = transpose(M);
        auto eig = mathlib::right_eigenvector_via_qbeta(Mt, R);
        assert(eig.ok);

        auto automaton = adelic::build_prefix_automaton<3>(images, eig.v, R);
        auto cert = ravel::proof::check_zero_edges_are_exactly_empty_prefix<3>(
            automaton.distinct_prefixes, automaton.digit_set, R);

        std::cout << "a=" << a << ": nonempty_prefixes_checked=" << cert.nonempty_prefixes_checked
                  << " every_strictly_positive=" << cert.every_nonempty_prefix_strictly_positive << "\n";
        assert(cert.every_nonempty_prefix_strictly_positive);
        assert(cert.nonempty_prefixes_checked > 0);  // sanity: the family always has some nonempty prefix
    }
    std::cout << "constant_factor_gives_nothing_for_property_f: FACT 1 confirmed on the whole "
                 "family (a=0..5) directly against the computed automaton, not just asserted.\n";
    return 0;
}
