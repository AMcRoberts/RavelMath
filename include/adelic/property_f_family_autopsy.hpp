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
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/property_f_escape_rank.hpp"
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
    long long property_f_terminal_sccs = -1;
    long long property_f_max_escape_height = -1;
    // Height-profile scalars distinguish a long transient collar from a
    // genuinely large recurrent component.  They are diagnostics, not a
    // theorem: rank zero is the accepted terminal shell.
    long long property_f_transient_nodes = -1;
    long long property_f_height_mass = -1;
    long long property_f_max_scc_size = -1;
    long long property_f_nonzero_cycles = -1;
    long long property_f_boundary_edges = -1;
    long long property_f_cyclic_sccs = -1;
    long long property_f_zero_cycle_components = -1;
    long long property_f_mixed_cycle_components = -1;
    long long property_f_self_loops = -1;
    long long return_words = -1;
    long long return_phase_states = -1;
    long long return_phase_edges = -1;
    long long return_phase_sccs = -1;
    long long return_phase_cycle_components = -1;
    bool return_transport_closed = false;
    bool coincidence_holds = false;
    bool property_f_holds = false;
    bool inconclusive = false;
    bool trusted_padic = false;
};

namespace detail {

struct CycleStats {
    long long cyclic_sccs = 0;
    long long zero_cycle_components = 0;
    long long mixed_cycle_components = 0;
    long long self_loops = 0;
};

inline CycleStats classify_cycles(
    const std::vector<std::vector<long long>>& graph,
    std::size_t zero_frontier) {
    const std::size_t n = graph.size();
    std::vector<long long> index(n, -1), low(n, 0), stack;
    std::vector<bool> on_stack(n, false);
    long long next = 0;
    CycleStats result;
    std::function<void(std::size_t)> visit = [&](std::size_t u) {
        index[u] = low[u] = next++;
        stack.push_back(static_cast<long long>(u));
        on_stack[u] = true;
        for (const long long raw_v : graph[u]) {
            const std::size_t v = static_cast<std::size_t>(raw_v);
            if (v >= n) continue;
            if (index[v] < 0) {
                visit(v);
                low[u] = std::min(low[u], low[v]);
            } else if (on_stack[v]) {
                low[u] = std::min(low[u], index[v]);
            }
            if (v == u) ++result.self_loops;
        }
        if (low[u] != index[u]) return;
        std::vector<std::size_t> component;
        while (true) {
            const std::size_t v = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            on_stack[v] = false;
            component.push_back(v);
            if (v == u) break;
        }
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty())
            for (const long long raw_v : graph[component.front()])
                if (raw_v == static_cast<long long>(component.front())) cyclic = true;
        if (!cyclic) return;
        ++result.cyclic_sccs;
        bool all_zero = true, all_nonzero = true;
        for (const std::size_t v : component) {
            const bool zero = v < zero_frontier;
            all_zero = all_zero && zero;
            all_nonzero = all_nonzero && !zero;
        }
        if (all_zero) ++result.zero_cycle_components;
        else if (!all_nonzero) ++result.mixed_cycle_components;
    };
    for (std::size_t u = 0; u < n; ++u)
        if (index[u] < 0) visit(u);
    return result;
}

inline std::pair<long long, long long> phase_scc_stats(
    const std::vector<std::vector<std::size_t>>& graph) {
    const std::size_t n = graph.size();
    std::vector<long long> index(n, -1), low(n, 0), stack;
    std::vector<bool> on_stack(n, false);
    long long next = 0, sccs = 0, cyclic = 0;
    std::function<void(std::size_t)> visit = [&](std::size_t u) {
        index[u] = low[u] = next++;
        stack.push_back(static_cast<long long>(u));
        on_stack[u] = true;
        for (const std::size_t v : graph[u]) {
            if (index[v] < 0) {
                visit(v);
                low[u] = std::min(low[u], low[v]);
            } else if (on_stack[v]) {
                low[u] = std::min(low[u], index[v]);
            }
        }
        if (low[u] != index[u]) return;
        std::vector<std::size_t> component;
        while (true) {
            const std::size_t v = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            on_stack[v] = false;
            component.push_back(v);
            if (v == u) break;
        }
        ++sccs;
        bool has_cycle = component.size() > 1;
        if (!has_cycle && !component.empty())
            for (const auto v : graph[component.front()])
                if (v == component.front()) has_cycle = true;
        if (has_cycle) ++cyclic;
    };
    for (std::size_t u = 0; u < n; ++u)
        if (index[u] < 0) visit(u);
    return {sccs, cyclic};
}

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
    std::vector<std::vector<long long>> property_graph;
    PropertyFGraph property_graph_exact;
    const auto propf = check_property_f<d>(automaton, node_budget, padic_bound,
                                           &property_graph, nullptr, nullptr,
                                           &property_graph_exact);

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
    if (propf.closure_reached) {
        const auto rank = derive_property_f_escape_rank(property_graph_exact);
        if (rank.valid) {
            out.property_f_terminal_sccs = static_cast<long long>(rank.terminal_sccs);
            out.property_f_max_escape_height = static_cast<long long>(rank.maximum_height);
            long long transient_nodes = 0;
            long long height_mass = 0;
            for (const auto height : rank.node_height) {
                if (height > 0) ++transient_nodes;
                height_mass += static_cast<long long>(height);
            }
            out.property_f_transient_nodes = transient_nodes;
            out.property_f_height_mass = height_mass;
            for (const auto size : property_graph_exact.scc_sizes)
                out.property_f_max_scc_size = std::max(
                    out.property_f_max_scc_size, size);
        }
    }
    out.property_f_nonzero_cycles = propf.nonzero_cycle_components;
    out.property_f_boundary_edges = propf.boundary_edges;
    const auto cycles = classify_cycles(property_graph, d);
    out.property_f_cyclic_sccs = cycles.cyclic_sccs;
    out.property_f_zero_cycle_components = cycles.zero_cycle_components;
    out.property_f_mixed_cycle_components = cycles.mixed_cycle_components;
    out.property_f_self_loops = cycles.self_loops;
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
        const auto [phase_sccs, phase_cycles] = phase_scc_stats(phase.phase_images);
        out.return_phase_sccs = phase_sccs;
        out.return_phase_cycle_components = phase_cycles;
        out.return_transport_closed = true;
    } catch (const std::exception&) {
        // A marker need not be recognizable for every permutation.  That
        // is data: retain the failure as a non-closed transport lift rather
        // than turning the entire family run into an exception.
        out.return_words = 0;
        out.return_phase_states = 0;
        out.return_phase_edges = 0;
        out.return_phase_sccs = 0;
        out.return_phase_cycle_components = 0;
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
    return "name\timages\tprefix_states\tdistinct_prefixes\tcoincidence_depth\tcoincidence_unresolved\tproperty_f_nodes\tproperty_f_zero_nodes\tproperty_f_nonzero_nodes\tproperty_f_sccs\tproperty_f_terminal_sccs\tproperty_f_max_escape_height\tproperty_f_transient_nodes\tproperty_f_height_mass\tproperty_f_max_scc_size\tproperty_f_nonzero_cycles\tproperty_f_boundary_edges\tproperty_f_cyclic_sccs\tproperty_f_zero_cycle_components\tproperty_f_mixed_cycle_components\tproperty_f_self_loops\treturn_words\treturn_phase_states\treturn_phase_edges\treturn_phase_sccs\treturn_phase_cycle_components\treturn_transport_closed\tcoincidence_holds\tproperty_f_holds\tinconclusive\ttrusted_padic\n";
}

inline std::string property_f_family_tsv_row(const PropertyFFamilyObservation& r) {
    std::ostringstream out;
    out << r.name << '\t' << r.images << '\t' << r.prefix_states << '\t'
        << r.distinct_prefixes << '\t' << r.coincidence_depth << '\t'
        << r.coincidence_unresolved << '\t' << r.property_f_nodes << '\t'
        << r.property_f_zero_nodes << '\t' << r.property_f_nonzero_nodes << '\t'
        << r.property_f_sccs << '\t' << r.property_f_terminal_sccs << '\t'
        << r.property_f_max_escape_height << '\t'
        << r.property_f_transient_nodes << '\t' << r.property_f_height_mass << '\t'
        << r.property_f_max_scc_size << '\t'
        << r.property_f_nonzero_cycles << '\t'
        << r.property_f_boundary_edges << '\t' << r.property_f_cyclic_sccs << '\t'
        << r.property_f_zero_cycle_components << '\t'
        << r.property_f_mixed_cycle_components << '\t' << r.property_f_self_loops << '\t'
        << r.return_words << '\t'
        << r.return_phase_states << '\t' << r.return_phase_edges << '\t'
        << r.return_phase_sccs << '\t' << r.return_phase_cycle_components << '\t'
        << r.return_transport_closed << '\t'
        << r.coincidence_holds << '\t'
        << r.property_f_holds << '\t' << r.inconclusive << '\t'
        << r.trusted_padic << '\n';
    return out.str();
}

}  // namespace adelic
