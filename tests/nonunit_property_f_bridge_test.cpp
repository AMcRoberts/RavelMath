#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/property_f_contact_transport_bridge.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/qbeta.hpp"
#include "ravel/simplest_nonunit_pisot_substitution.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

int main() {
    using namespace ravel;
    constexpr std::size_t d = 2;
    const auto images8 = simplest_nonunit_pisot_rule();
    SubstitutionRule rule(images8);
    std::vector<std::vector<long long>> matrix(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (auto letter : images8[j]) ++matrix[static_cast<std::size_t>(letter)][j];
    const auto spectral = classify_matrix_spectral(matrix);
    const auto charpoly = mathlib::charpoly_faddeev_leverrier(matrix);
    const mathlib::QBetaRing ring(charpoly);
    const auto eig = mathlib::left_eigenvector_via_qbeta_reduced_factor(matrix, ring);
    assert(eig.ok);
    std::array<std::vector<long long>, d> images{};
    for (std::size_t j = 0; j < d; ++j)
        for (auto letter : images8[j]) images[j].push_back(letter);
    const auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, ring);
    ContactBoundaryLimits limits;
    limits.closure_cap = 40'000;
    limits.corona_cap = 200'000;
    limits.max_corona_rounds = 14;
    limits.retain_boundary_matrix = false;
    const auto bridge = adelic::derive_property_f_contact_transport_bridge<d>(
        rule, automaton, spectral.beta, 2, limits);
    assert(bridge.boundary_complete);
    assert(bridge.contact_prefixes_in_adelic_alphabet);
    assert(bridge.exact_digit_labels_replay);
    assert(bridge.transition_labels_commute);

    const auto bound_info = adelic::make_combined_padic_bound(
        std::vector<long long>{2}, charpoly);
    assert(bound_info.second);
    adelic::PropertyFGraph graph;
    long long zero_beyond = -1;
    const auto propf = adelic::check_property_f<d>(
        automaton, 1'000'000, bound_info.first, nullptr, nullptr,
        &zero_beyond, &graph, true);
    const auto recurrence = adelic::derive_property_f_contact_recurrence_certificate(
        bridge, propf, graph);
    std::cout << "nonunit bridge: beta=" << spectral.beta
              << " contact_edges=" << bridge.boundary_edges
              << " differences=" << bridge.distinct_difference_labels
              << " property_nodes=" << propf.nodes_explored
              << " zero_beyond=" << zero_beyond
              << " recurrence_obstruction=" << recurrence.obstruction << "\n";
    assert(propf.holds);
    assert(zero_beyond == 0);
    assert(recurrence.recurrence_preserved);
    assert(recurrence.no_nonzero_recurrent_component);
    return 0;
}
