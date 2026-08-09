// Independent finite Pisot checks for the second genuine fourth-generator
// witness x^3-x^2-2x-2, whose canonical images are
// 0->001, 1->2, 2->03, 3->4, 4->00.
//
// This is deliberately separate from the contact-boundary probe: the latter
// is resource-heavy and remains inconclusive, while this Q(beta)/adelic path
// closes quickly with a trusted p-adic bound.

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/qbeta.hpp"
#include "ravel/proof/coincidence_closure.hpp"

int main() {
    constexpr std::size_t d = 5;
    const std::array<std::vector<long long>, d> images = {
        std::vector<long long>{0, 0, 1}, std::vector<long long>{2},
        std::vector<long long>{0, 3}, std::vector<long long>{4},
        std::vector<long long>{0, 0}};
    std::array<std::array<long long, d>, d> matrix{};
    std::vector<std::vector<long long>> dense(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (long long letter : images[j]) {
            ++matrix[static_cast<std::size_t>(letter)][j];
            ++dense[static_cast<std::size_t>(letter)][j];
        }

    const auto incidence_cp = mathlib::charpoly_faddeev_leverrier(dense);
    mathlib::PolyZ minpoly;
    minpoly.ensure_size(4);
    mathlib::set_si(minpoly.coeff(0), -2);
    mathlib::set_si(minpoly.coeff(1), -2);
    mathlib::set_si(minpoly.coeff(2), -1);
    mathlib::set_si(minpoly.coeff(3), 1);
    mathlib::QBetaRing ring(minpoly);
    const auto eig = mathlib::left_eigenvector_via_qbeta_reduced_factor(dense, ring);
    assert(eig.ok);

    const auto closure = ravel::proof::check_strong_coincidence_closure<d>(
        images, matrix, 64, 1'000'000);
    assert(closure.holds);
    assert(!closure.inconclusive);
    assert(closure.depth_reached == 9);
    assert(closure.unresolved_pairs == 0);

    auto [padic_bound, trusted] = adelic::make_combined_padic_bound(
        std::vector<long long>{2}, minpoly);
    assert(trusted);
    const auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, ring);
    const auto propf = adelic::check_property_f<d>(automaton, 10'000'000,
                                                    padic_bound);
    assert(propf.holds);
    assert(!propf.inconclusive);
    assert(propf.nodes_explored == 4277);
    assert(propf.nonzero_cycle_components == 0);

    std::cout << "second witness: charpoly=" << mathlib::str(incidence_cp)
              << " coincidence=HOLDS(depth " << closure.depth_reached
              << ") propertyF=HOLDS(nodes " << propf.nodes_explored
              << ", nonzero_cycles " << propf.nonzero_cycle_components
              << ", trusted_padic=1)\n";
    return 0;
}
