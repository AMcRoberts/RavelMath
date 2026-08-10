#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/generalized_multinacci_fiber_interaction.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/generalized_multinacci.hpp"

template <std::size_t d>
void check_case(std::size_t m) {
    const auto raw = ravel::generalized_multinacci_rule(d, m);
    std::array<std::vector<long long>, d> images;
    std::vector<std::vector<long long>> matrix(
        d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (const auto symbol : raw[j]) {
            images[j].push_back(symbol);
            ++matrix[symbol][j];
        }
    mathlib::QBetaRing ring(ravel::generalized_multinacci_polynomial(d, m));
    const auto eigen = mathlib::left_eigenvector_via_qbeta_reduced_factor(
        matrix, ring);
    const auto automaton = adelic::build_prefix_automaton<d>(
        images, eigen.v, ring);
    adelic::PropertyFGraph graph;
    const auto result = adelic::check_property_f<d>(
        automaton, 2000000, nullptr, nullptr, nullptr, nullptr, &graph, false);
    assert(result.holds && !result.inconclusive);
    const auto summary =
        adelic::classify_generalized_multinacci_fiber_interaction<d>(graph);
    assert(summary.one_way_new_fiber);
    assert(summary.new_nontrivial_scc_nodes == 0);
    assert(summary.new_to_new_edges == 0);
    assert(summary.top_to_new_edges == 0);
    std::cout << "d=" << d << " m=" << m
              << " inherited_to_new=" << summary.inherited_to_new_edges
              << " new_to_top=" << summary.new_to_top_edges
              << " new_nodes=" << summary.new_nodes << "\n";
}

int main() {
    for (std::size_t m = 1; m <= 6; ++m) check_case<3>(m);
    for (std::size_t m = 1; m <= 3; ++m) check_case<4>(m);
    check_case<5>(1);
    std::cout << "generalized multinacci fiber interaction PASS\n";
}
