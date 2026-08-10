#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/property_f_contact_transport_bridge.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/qbeta.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

int main() {
    using namespace ravel;
    constexpr std::size_t d = 3;
    const std::vector<std::vector<std::int8_t>> images{{1, 2}, {2}, {0}};
    SubstitutionRule rule(images);
    std::vector<std::vector<long long>> matrix(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (auto letter : images[j]) ++matrix[static_cast<std::size_t>(letter)][j];
    const auto spectral = classify_matrix_spectral(matrix);
    const auto charpoly = mathlib::charpoly_faddeev_leverrier(matrix);
    const mathlib::QBetaRing ring(charpoly);
    const auto eig = mathlib::left_eigenvector_via_qbeta_reduced_factor(matrix, ring);
    assert(eig.ok);
    std::array<std::vector<long long>, d> adelic_images{};
    for (std::size_t j = 0; j < d; ++j)
        for (auto letter : images[j]) adelic_images[j].push_back(letter);
    const auto automaton = adelic::build_prefix_automaton<d>(
        adelic_images, eig.v, ring);
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
    adelic::PropertyFGraph graph;
    long long zero_beyond = -1;
    const auto propf = adelic::check_property_f<d>(
        automaton, 2'000'000, nullptr, nullptr, nullptr, &zero_beyond,
        &graph, true);
    const auto recurrence = adelic::derive_property_f_contact_recurrence_certificate(
        bridge, propf, graph);
    std::cout << "sigma01 bridge: beta=" << spectral.beta
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
