// Bounded family-study runner for Property (F) and strong coincidence.
//
// The family varies only the order of letters inside substitution images.
// Thus each member has the same incidence matrix and Pisot polynomial; any
// change in the finite graph is an ordering/branching effect, not a spectral
// effect.  Results are deliberately scalar and TSV-friendly so a study can
// be archived without retaining a dense graph.
#pragma once

#include <array>
#include <cstddef>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "math/qbeta.hpp"
#include "ravel/proof/coincidence_closure.hpp"
#include "ravel/return_substitution.hpp"

namespace adelic {

struct PropertyFFamilyObservation {
    std::string name;
    std::string images;
    long long prefix_states = 0;
    long long distinct_prefixes = 0;
    long long coincidence_depth = -1;
    long long coincidence_unresolved = -1;
    long long property_f_nodes = -1;
    long long property_f_zero_nodes = -1;
    long long property_f_nonzero_nodes = -1;
    long long property_f_sccs = -1;
    long long property_f_nonzero_cycles = -1;
    long long property_f_boundary_edges = -1;
    long long return_words = -1;
    long long return_phase_states = -1;
    long long return_phase_edges = -1;
    bool return_transport_closed = false;
    bool coincidence_holds = false;
    bool property_f_holds = false;
    bool inconclusive = false;
    bool trusted_padic = false;
};

namespace detail {

template <std::size_t d>
inline std::string image_code(const std::array<std::vector<long long>, d>& images) {
    std::ostringstream out;
    for (std::size_t i = 0; i < d; ++i) {
        if (i) out << '/';
        for (long long a : images[i]) out << a;
    }
    return out.str();
}

template <std::size_t d>
inline PropertyFFamilyObservation analyze_case(
    const std::string& name,
    const std::array<std::vector<long long>, d>& images,
    const mathlib::PolyZ& minpoly,
    const std::vector<long long>& primes,
    long long node_budget,
    long long coincidence_budget) {
    std::array<std::array<long long, d>, d> matrix{};
    std::vector<std::vector<long long>> dense(d, std::vector<long long>(d, 0));
    for (std::size_t j = 0; j < d; ++j)
        for (long long a : images[j]) {
            ++matrix[static_cast<std::size_t>(a)][j];
            ++dense[static_cast<std::size_t>(a)][j];
        }
    mathlib::QBetaRing ring(minpoly);
    const auto eig = mathlib::left_eigenvector_via_qbeta_reduced_factor(dense, ring);
    if (!eig.ok) throw std::runtime_error("family autopsy: eigenvector failed");
    const auto closure = ravel::proof::check_strong_coincidence_closure<d>(
        images, matrix, 64, static_cast<std::size_t>(coincidence_budget));
    auto [padic_bound, trusted] = make_combined_padic_bound(primes, minpoly);
    const auto automaton = build_prefix_automaton<d>(images, eig.v, ring);
    const auto propf = check_property_f<d>(automaton, node_budget, padic_bound);

    PropertyFFamilyObservation out;
    out.name = name;
    out.images = image_code(images);
    for (const auto& edges : automaton.by_source) out.prefix_states += static_cast<long long>(edges.size());
    out.distinct_prefixes = static_cast<long long>(automaton.distinct_prefixes.size());
    out.coincidence_depth = closure.depth_reached;
    out.coincidence_unresolved = closure.unresolved_pairs;
    out.property_f_nodes = propf.nodes_explored;
    out.property_f_zero_nodes = propf.zero_nodes;
    out.property_f_nonzero_nodes = propf.nonzero_nodes;
    out.property_f_sccs = propf.strongly_connected_components;
    out.property_f_nonzero_cycles = propf.nonzero_cycle_components;
    out.property_f_boundary_edges = propf.boundary_edges;
    std::vector<std::vector<std::int8_t>> sigma;
    sigma.reserve(d);
    for (const auto& image : images) {
        std::vector<std::int8_t> converted;
        for (long long a : image) converted.push_back(static_cast<std::int8_t>(a));
        sigma.push_back(std::move(converted));
    }
    try {
        const ravel::ReturnPhaseSystem phase = ravel::build_return_phase_system(
            ravel::SubstitutionRule(std::move(sigma)), 0, 100'000);
        out.return_words = static_cast<long long>(phase.induced.words.size());
        out.return_phase_states = static_cast<long long>(phase.states.size());
        out.return_phase_edges = 0;
        for (const auto& image : phase.phase_images)
            out.return_phase_edges += static_cast<long long>(image.size());
        out.return_transport_closed = true;
    } catch (const std::exception&) {
        // A marker need not be recognizable for every permutation.  That
        // is data: retain the failure as a non-closed transport lift rather
        // than turning the entire family run into an exception.
        out.return_words = 0;
        out.return_phase_states = 0;
        out.return_phase_edges = 0;
    }
    out.coincidence_holds = closure.holds;
    out.property_f_holds = propf.holds;
    out.inconclusive = closure.inconclusive || propf.inconclusive;
    out.trusted_padic = trusted;
    return out;
}

}  // namespace detail

// Produces the fixed-spectrum permutation family around the two fourth-
// generator witnesses.  No expensive contact-boundary computation occurs.
inline std::vector<PropertyFFamilyObservation> run_property_f_family(
    long long node_budget, long long coincidence_budget) {
    std::vector<PropertyFFamilyObservation> rows;
    mathlib::PolyZ first_poly;
    first_poly.ensure_size(4);
    mathlib::set_si(first_poly.coeff(0), -2);
    mathlib::set_si(first_poly.coeff(1), 0);
    mathlib::set_si(first_poly.coeff(2), -2);
    mathlib::set_si(first_poly.coeff(3), 1);
    const std::array<std::vector<long long>, 3> first_base = {
        std::vector<long long>{0, 0, 1}, {2}, {0, 0}};
    const std::array<std::vector<long long>, 3> first_mid = {
        std::vector<long long>{0, 1, 0}, {2}, {0, 0}};
    const std::array<std::vector<long long>, 3> first_last = {
        std::vector<long long>{1, 0, 0}, {2}, {0, 0}};
    rows.push_back(detail::analyze_case("first_anchor", first_base, first_poly, {2}, node_budget, coincidence_budget));
    rows.push_back(detail::analyze_case("first_010", first_mid, first_poly, {2}, node_budget, coincidence_budget));
    rows.push_back(detail::analyze_case("first_100", first_last, first_poly, {2}, node_budget, coincidence_budget));

    mathlib::PolyZ second_poly;
    second_poly.ensure_size(4);
    mathlib::set_si(second_poly.coeff(0), -2);
    mathlib::set_si(second_poly.coeff(1), -2);
    mathlib::set_si(second_poly.coeff(2), -1);
    mathlib::set_si(second_poly.coeff(3), 1);
    const std::array<std::vector<long long>, 5> second_base = {
        std::vector<long long>{0, 0, 1}, {2}, {0, 3}, {4}, {0, 0}};
    const std::array<std::vector<long long>, 5> second_a = {
        std::vector<long long>{0, 0, 1}, {2}, {3, 0}, {4}, {0, 0}};
    const std::array<std::vector<long long>, 5> second_b = {
        std::vector<long long>{0, 1, 0}, {2}, {0, 3}, {4}, {0, 0}};
    const std::array<std::vector<long long>, 5> second_c = {
        std::vector<long long>{0, 1, 0}, {2}, {3, 0}, {4}, {0, 0}};
    const std::array<std::vector<long long>, 5> second_d = {
        std::vector<long long>{1, 0, 0}, {2}, {0, 3}, {4}, {0, 0}};
    const std::array<std::vector<long long>, 5> second_e = {
        std::vector<long long>{1, 0, 0}, {2}, {3, 0}, {4}, {0, 0}};
    rows.push_back(detail::analyze_case("second_anchor", second_base, second_poly, {2}, node_budget, coincidence_budget));
    rows.push_back(detail::analyze_case("second_001_30", second_a, second_poly, {2}, node_budget, coincidence_budget));
    rows.push_back(detail::analyze_case("second_010_03", second_b, second_poly, {2}, node_budget, coincidence_budget));
    rows.push_back(detail::analyze_case("second_010_30", second_c, second_poly, {2}, node_budget, coincidence_budget));
    rows.push_back(detail::analyze_case("second_100_03", second_d, second_poly, {2}, node_budget, coincidence_budget));
    rows.push_back(detail::analyze_case("second_100_30", second_e, second_poly, {2}, node_budget, coincidence_budget));
    return rows;
}

// Stable, header-only serialization contract used by the artifact generator.
inline std::string property_f_family_tsv_header() {
    return "name\timages\tprefix_states\tdistinct_prefixes\tcoincidence_depth\tcoincidence_unresolved\tproperty_f_nodes\tproperty_f_zero_nodes\tproperty_f_nonzero_nodes\tproperty_f_sccs\tproperty_f_nonzero_cycles\tproperty_f_boundary_edges\treturn_words\treturn_phase_states\treturn_phase_edges\treturn_transport_closed\tcoincidence_holds\tproperty_f_holds\tinconclusive\ttrusted_padic\n";
}

inline std::string property_f_family_tsv_row(const PropertyFFamilyObservation& r) {
    std::ostringstream out;
    out << r.name << '\t' << r.images << '\t' << r.prefix_states << '\t'
        << r.distinct_prefixes << '\t' << r.coincidence_depth << '\t'
        << r.coincidence_unresolved << '\t' << r.property_f_nodes << '\t'
        << r.property_f_zero_nodes << '\t' << r.property_f_nonzero_nodes << '\t'
        << r.property_f_sccs << '\t' << r.property_f_nonzero_cycles << '\t'
        << r.property_f_boundary_edges << '\t' << r.return_words << '\t'
        << r.return_phase_states << '\t' << r.return_phase_edges << '\t'
        << r.return_transport_closed << '\t'
        << r.coincidence_holds << '\t'
        << r.property_f_holds << '\t' << r.inconclusive << '\t'
        << r.trusted_padic << '\n';
    return out.str();
}

}  // namespace adelic
