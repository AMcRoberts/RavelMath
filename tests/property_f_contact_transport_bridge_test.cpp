#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "adelic/property_f_contact_transport_bridge.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/poly_z.hpp"
#include "math/qbeta.hpp"

template <std::size_t d>
void run_case(const std::array<std::vector<long long>, d>& images,
              const char* name) {
    std::vector<std::vector<long long>> matrix(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (const long long a : images[j]) ++matrix[a][j];
    mathlib::PolyZ minpoly;
    minpoly.ensure_size(4);
    mathlib::set_si(minpoly.coeff(0), -2);
    mathlib::set_si(minpoly.coeff(1), 0);
    mathlib::set_si(minpoly.coeff(2), -2);
    mathlib::set_si(minpoly.coeff(3), 1);
    mathlib::QBetaRing ring(minpoly);
    const auto eig = mathlib::left_eigenvector_via_qbeta_reduced_factor(matrix, ring);
    assert(eig.ok);
    const auto automaton = adelic::build_prefix_automaton<d>(images, eig.v, ring);
    std::vector<std::vector<std::int8_t>> sigma;
    for (const auto& image : images) {
        std::vector<std::int8_t> converted;
        for (const long long a : image) converted.push_back(static_cast<std::int8_t>(a));
        sigma.push_back(std::move(converted));
    }
    ravel::SubstitutionRule rule(std::move(sigma));
    ravel::ContactBoundaryLimits limits;
    limits.closure_cap = 40'000;
    limits.corona_cap = 200'000;
    limits.max_corona_rounds = 14;
    limits.retain_boundary_matrix = false;
    const auto bridge = adelic::derive_property_f_contact_transport_bridge<d>(
        rule, automaton, 2.3593040859717767, 2, limits);
    assert(bridge.boundary_complete);
    assert(bridge.contact_prefixes_in_adelic_alphabet);
    assert(bridge.exact_digit_labels_replay);
    assert(bridge.transition_labels_commute);
    assert(bridge.boundary_edges > 0);
    assert(bridge.distinct_contact_prefixes > 0);
    assert(bridge.distinct_difference_labels > 0);
    if (std::string(name) == "first_anchor") {
        auto [bound, trusted] = adelic::make_combined_padic_bound({2}, minpoly);
        assert(trusted);
        adelic::PropertyFGraph graph;
        const auto propf = adelic::check_property_f<d>(
            automaton, 2'000'000, bound, nullptr, nullptr, nullptr,
            &graph, true);
        const auto recurrence =
            adelic::derive_property_f_contact_recurrence_certificate(
                bridge, propf, graph);
        assert(recurrence.recurrence_preserved);
        assert(recurrence.nonzero_recurrent_components == 0);
        const auto intertwiner = ravel::proof::
            derive_contact_boundary_generator_intertwiner<d>(
                rule, 2.3593040859717767, 0.0, 2, limits, 2);
        const auto paired_recurrence =
            adelic::derive_property_f_contact_recurrence_certificate(
                bridge, propf, graph, intertwiner);
        assert(paired_recurrence.universal_intertwiner_verified);
        assert(paired_recurrence.recurrence_preserved);
        std::cout << name << ": recurrence PASS graph_nodes="
                  << recurrence.graph_nodes << "\n";
    }
    std::cout << name << ": PASS boundary_edges="
              << bridge.boundary_edges << " contact_prefixes="
              << bridge.distinct_contact_prefixes << " difference_labels="
              << bridge.distinct_difference_labels << "\n";
}

int main() {
    constexpr std::size_t d = 3;
    run_case<d>({std::vector<long long>{0, 0, 1}, {2}, {0, 0}}, "first_anchor");
    run_case<d>({std::vector<long long>{0, 1, 0}, {2}, {0, 0}}, "first_010");
    run_case<d>({std::vector<long long>{1, 0, 0}, {2}, {0, 0}}, "first_100");
}
