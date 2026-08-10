#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/marker_power_return_core.hpp"
#include "adelic/prefix_automaton.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/property_f_role_digit_cocycle.hpp"
#include "adelic/property_f_escape_rank.hpp"
#include "adelic/property_f_birth_round_grammar.hpp"
#include "adelic/return_phase_digit_cocycle.hpp"
#include "adelic/return_contact_digit_holonomy.hpp"
#include "adelic/return_contact_gamma_relation.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"
#include "ravel/return_contact_lift.hpp"
#include "ravel/return_history_factor.hpp"
#include "ravel/return_offset_fibre_certificate.hpp"
#include "ravel/return_offset_fibre_holonomy.hpp"
#include "ravel/return_contact_textile_certificate.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {
int tests = 0;
int failures = 0;
void expect(bool value, const char* label) {
    ++tests;
    if (value) std::printf("  [ok]   %s\n", label);
    else { std::printf("  [FAIL] %s\n", label); ++failures; }
}

std::size_t largest_nonzero_recurrent_scc(const ReturnContactLift<3>& lift) {
    const std::size_t n = lift.states.size();
    std::vector<std::vector<std::size_t>> graph(n);
    for (const auto& edge : lift.edges) graph[edge.source].push_back(edge.destination);
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<bool> active(n, false);
    int next = 0;
    std::size_t largest = 0;
    std::function<void(std::size_t)> visit = [&](std::size_t v) {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v));
        active[v] = true;
        for (std::size_t w : graph[v]) {
            if (index[w] < 0) {
                visit(w);
                low[v] = std::min(low[v], low[w]);
            } else if (active[w]) {
                low[v] = std::min(low[v], index[w]);
            }
        }
        if (low[v] != index[v]) return;
        std::vector<std::size_t> component;
        while (true) {
            const std::size_t w = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            active[w] = false;
            component.push_back(w);
            if (w == v) break;
        }
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty()) {
            const auto u = component.front();
            for (std::size_t w : graph[u]) if (w == u) cyclic = true;
        }
        if (!cyclic) return;
        for (std::size_t u : component) {
            const auto& x = lift.states[u].contact.x;
            bool nonzero = false;
            for (long long coordinate : x) if (coordinate != 0) nonzero = true;
            if (nonzero) largest = std::max(largest, component.size());
        }
    };
    for (std::size_t v = 0; v < n; ++v)
        if (index[v] < 0) visit(v);
    return largest;
}

mathlib::QElem parse_property_gamma(const adelic::PropertyFGraphNode& node) {
    mathlib::QElem gamma(node.gamma_coefficients.size());
    for (std::size_t i = 0; i < node.gamma_coefficients.size(); ++i) {
        mathlib::Rat value;
        const auto& [numerator, denominator] = node.gamma_coefficients[i];
        const std::string encoded = numerator + "/" + denominator;
        if (mpq_set_str(value.get(), encoded.c_str(), 10) != 0)
            throw std::invalid_argument("non-AR test: invalid Property-F rational");
        mpq_canonicalize(value.get());
        gamma.coeff(i) = value;
    }
    return gamma;
}

std::vector<long long> prime_support(long long value) {
    std::vector<long long> out;
    long long remaining = std::llabs(value);
    for (long long p = 2; p <= remaining / p; ++p) {
        if (remaining % p != 0) continue;
        out.push_back(p);
        while (remaining % p == 0) remaining /= p;
    }
    if (remaining > 1) out.push_back(remaining);
    return out;
}
}

int main() {
    const SubstitutionRule base({{1, 2}, {2}, {0}});
    const auto powered = marker_power_rule(base, 3);
    const auto core = analyze_marker_power_return_core(base, 0);
    expect(core.holds && core.power == 3,
           "non-AR base has a certified marker-power core");

    std::vector<std::vector<long long>> incidence(
        powered.alphabet_size(), std::vector<long long>(powered.alphabet_size(), 0));
    for (std::size_t source = 0; source < powered.alphabet_size(); ++source)
        for (auto letter : powered.image(source))
            ++incidence[static_cast<std::size_t>(letter)][source];
    const auto spectral = classify_matrix_spectral(incidence);
    const auto substitution = make_substitution<3>(powered, spectral.beta);
    std::vector<std::tuple<long long, std::vector<long long>, long long>> d_cont;
    for (const auto& candidate : search_D_cont<3>(substitution, 2))
        d_cont.emplace_back(candidate.i,
                            std::vector<long long>(candidate.x.begin(), candidate.x.end()),
                            candidate.j);
    const auto boundary = compute_contact_boundary_dispatch(
        powered, spectral.beta, spectral.b2, d_cont);
    expect(boundary.converged && !boundary.closure_stopped_early
               && !boundary.corona_capped,
           "powered non-AR contact boundary closes");
    expect(!boundary.boundary_nodes.empty(),
           "powered non-AR contact boundary is nonempty");

    std::vector<SNode<3>> bare;
    for (const auto& [i, x, j] : boundary.boundary_nodes) {
        SNode<3> node{};
        node.i = i;
        node.j = j;
        for (std::size_t k = 0; k < 3; ++k) node.x[k] = x[k];
        bare.push_back(node);
    }
    const auto phases = build_return_phase_system(powered, 0);
    std::vector<ReturnContactState<3>> seeds;
    for (const auto& node : bare) {
        const auto compatible = compatible_return_contact_seeds(node, phases);
        seeds.insert(seeds.end(), compatible.begin(), compatible.end());
    }
    ReturnContactLimits limits;
    limits.max_states = 1 << 18;
    limits.max_edges = 1 << 20;
    const auto lift = build_reachable_return_contact_lift(
        substitution, bare, phases, seeds, limits);
    expect(lift.complete(), "non-AR return/contact lift closes");
    expect(lift.projected_node_count == bare.size(),
           "lift projects onto every powered contact node");
    expect(!lift.edges.empty(), "non-AR return/contact lift has transport edges");
    const auto history = probe_return_history_factor(lift, phases);
    std::printf("return-history keys=%zu conflicts=%zu offset_variants=%zu offset_sensitive=%zu max_variants=%zu\n",
                history.history_keys, history.conflicting_keys,
                history.offset_variant_keys, history.offset_sensitive_keys,
                history.maximum_offset_variants);
    expect(history.conflicts_are_offset_driven
               && history.offset_sensitive_keys == history.offset_variant_keys,
           "every multi-offset history key has offset-sensitive transport");
    const auto fibre = derive_return_offset_fibre_certificate(lift, phases);
    std::printf("offset-fibre states=%zu edges=%zu cyclic_sccs=%zu largest=%zu "
                "class_sizes=%zu..%zu uniform=%d\n",
                fibre.states, fibre.edges, fibre.cyclic_sccs,
                fibre.largest_cyclic_scc, fibre.minimum_class_size,
                fibre.maximum_class_size, fibre.uniform_class_sizes ? 1 : 0);
    expect(fibre.finite && fibre.cyclic_sccs > 0,
           "offset fibre is finite and carries recurrent phase transport");
    expect(!fibre.uniform_class_sizes,
           "offset quotient is a relation, not a uniform permutation cover");
    const auto textile = derive_return_contact_textile_certificate(lift, phases);
    std::printf("textile pairs=%zu biregular=%zu permutation=%zu irregular=%zu "
                "branching=%d\n", textile.nonempty_pairs,
                textile.biregular_pairs, textile.permutation_pairs,
                textile.irregular_pairs, textile.has_branching_channels ? 1 : 0);
    expect(textile.finite && textile.nonempty_pairs > 0,
           "contact-aware quotient has a finite textile relation");
    expect(textile.irregular_pairs > 0 || textile.has_branching_channels,
           "contact-aware relation retains branching or irregular channels");
    const auto holonomy = derive_return_offset_fibre_holonomy(lift, phases);
    std::printf("holonomy recurrent_sccs=%zu gcd_one_sccs=%zu",
                holonomy.recurrent_sccs, holonomy.gcd_one_sccs);
    for (const auto& scc : holonomy.sccs)
        std::printf(" [nodes=%zu gcd=%lld max=%lld]", scc.nodes,
                    scc.gcd_cycle_residue, scc.max_abs_cycle_residue);
    std::printf("\n");
    expect(holonomy.finite && holonomy.recurrent_sccs > 0,
           "finite offset quotient has classified recurrent holonomy");
    expect(holonomy.recurrent_sccs == 1 && holonomy.sccs.front().gcd_cycle_residue == 3 &&
               !holonomy.sccs.front().coboundary,
           "non-AR transport has a genuine period-three twist");

    // The stronger Q(beta) test asks whether that integer transport survives
    // as a recurrent zero-expansion obstruction.  Build the live automaton
    // for the same powered rule, rather than reusing a hand-entered label set.
    std::vector<std::vector<long long>> powered_images(
        powered.alphabet_size(), std::vector<long long>{});
    std::vector<std::vector<long long>> powered_incidence(
        powered.alphabet_size(),
        std::vector<long long>(powered.alphabet_size(), 0));
    for (std::size_t source = 0; source < powered.alphabet_size(); ++source) {
        for (const auto letter : powered.image(source)) {
            powered_images[source].push_back(letter);
            ++powered_incidence[static_cast<std::size_t>(letter)][source];
        }
    }
    const auto charpoly = mathlib::charpoly_faddeev_leverrier(powered_incidence);
    const mathlib::QBetaRing ring(charpoly);
    const auto eigen = mathlib::left_eigenvector_via_qbeta_reduced_factor(
        powered_incidence, ring);
    std::array<std::vector<long long>, 3> automaton_images{};
    for (std::size_t source = 0; source < 3; ++source)
        automaton_images[source] = powered_images[source];
    const auto automaton = adelic::build_prefix_automaton<3>(
        automaton_images, eigen.v, ring);
    long long zero_beyond_frontier = -1;
    adelic::PropertyFGraph property_graph;
    const auto property_f = adelic::check_property_f<3>(
        automaton, 1'000'000, nullptr, nullptr, nullptr,
        &zero_beyond_frontier, &property_graph, true);
    std::printf("exact Property-F: holds=%d closed=%d nodes=%lld zero=%lld "
                "nonzero=%lld zero_beyond=%lld\n",
                property_f.holds ? 1 : 0, property_f.closure_reached ? 1 : 0,
                property_f.nodes_explored, property_f.zero_nodes,
                property_f.nonzero_nodes, zero_beyond_frontier);
    expect(property_f.holds && property_f.closure_reached &&
               zero_beyond_frontier == 0,
           "powered non-AR exact zero-expansion graph closes");
    const bool run_completion_probe = std::getenv("NONAR_COMPLETION_PROBE") != nullptr;
    const bool strict_completion_probe =
        std::getenv("NONAR_STRICT_COMPLETION_PROBE") != nullptr;
    if (run_completion_probe && !strict_completion_probe) {
        std::size_t terminal_nodes = 0;
        std::size_t terminal_zero_nodes = 0;
        std::printf("Property-F terminal nodes:");
        for (std::size_t node = 0; node < property_graph.nodes.size(); ++node) {
            const auto& terminal = property_graph.nodes[node];
            if (!terminal.successors.empty()) continue;
            ++terminal_nodes;
            if (terminal.zero) ++terminal_zero_nodes;
            std::printf(" %zu:%lld:%d:%s", node, terminal.letter,
                        terminal.zero ? 1 : 0, terminal.gamma_key.c_str());
        }
        std::printf("\n");
        expect(terminal_nodes == 54 && terminal_zero_nodes == 0,
               "all retained Property-F terminal sinks are nonzero in the powered control");
    }
    const auto gamma_escape_rank =
        adelic::derive_property_f_escape_rank(property_graph);
    expect(gamma_escape_rank.valid && gamma_escape_rank.terminal_sccs > 0,
           "Property-F SCC condensation yields a finite boundary rank");
    std::map<std::tuple<long long, bool, std::size_t>, std::size_t>
        height_classes;
    std::map<std::size_t, std::size_t> edge_drop_histogram;
    std::size_t internal_edges = 0;
    std::size_t nondecreasing_cross_scc_edges = 0;
    for (std::size_t node = 0; node < property_graph.nodes.size(); ++node) {
        ++height_classes[{property_graph.nodes[node].letter,
                          property_graph.nodes[node].zero,
                          gamma_escape_rank.node_height[node]}];
        for (const auto raw_target : property_graph.nodes[node].successors) {
            const auto target = static_cast<std::size_t>(raw_target);
            const auto source_height = gamma_escape_rank.node_height[node];
            const auto target_height = gamma_escape_rank.node_height[target];
            if (source_height < target_height) {
                ++nondecreasing_cross_scc_edges;
            } else if (source_height == target_height) {
                if (property_graph.scc_labels[node] ==
                    property_graph.scc_labels[target])
                    ++internal_edges;
                else
                    ++nondecreasing_cross_scc_edges;
            } else {
                ++edge_drop_histogram[source_height - target_height];
            }
        }
    }
    expect(nondecreasing_cross_scc_edges == 0,
           "finite Property-F graph rank strictly decreases across SCC edges");
    const auto birth_round =
        adelic::derive_property_f_birth_round_grammar(property_graph);
    expect(birth_round.valid && birth_round.layer_count == 7 &&
               birth_round.terminal_layer_nodes == 54 &&
               birth_round.internal_edges == internal_edges &&
               birth_round.edge_rank_strictly_decreasing,
           "birth-round grammar replays the powered non-AR boundary");
    expect(birth_round.layer_sizes ==
               std::map<std::size_t, std::size_t>{{0, 54}, {1, 18},
                                                   {2, 9}, {3, 5}, {4, 3},
                                                   {5, 3}, {6, 3}},
           "powered non-AR birth rounds have the expected seven-layer profile");
    expect(birth_round.drop_histogram == edge_drop_histogram,
           "birth-round transport drops match the direct SCC replay");
    if (std::getenv("NONAR_RANK_SHAPE_PROBE")) {
        std::printf("rank-shape heights=%zu terminal_sccs=%zu max=%zu "
                    "internal_edges=%zu nondecreasing_cross_scc=%zu classes:",
                    gamma_escape_rank.node_height.size(),
                    gamma_escape_rank.terminal_sccs,
                    gamma_escape_rank.maximum_height,
                    internal_edges, nondecreasing_cross_scc_edges);
        for (const auto& [key, count] : height_classes)
            std::printf(" letter%lld/%d/h%zu=%zu", std::get<0>(key),
                        std::get<1>(key) ? 1 : 0, std::get<2>(key), count);
        std::printf(" drops:");
        for (const auto& [drop, count] : edge_drop_histogram)
            std::printf(" %zu:%zu", drop, count);
        std::printf("\n");
    }
    std::size_t maximum_phase_residual = 0;
    for (const auto& phase : phases.states)
        maximum_phase_residual = std::max(
            maximum_phase_residual,
            phases.induced.words.at(phase.return_word).size() - phase.offset);
    const std::size_t phase_rank_weight = 2 * maximum_phase_residual + 1;
    const auto phase_residual = [&](std::size_t phase) {
        const auto& p = phases.states.at(phase);
        return phases.induced.words.at(p.return_word).size() - p.offset;
    };
    const auto completion_scalar_height =
        [&](const ReturnContactState<3>& state, std::size_t left,
            std::size_t right) {
            // Derive the lexicographic base from the actual phase system;
            // one gamma-layer drop dominates every possible phase wrap.
            return (gamma_escape_rank.node_height[left] +
                    gamma_escape_rank.node_height[right]) * phase_rank_weight +
                   phase_residual(state.left_phase) +
                   phase_residual(state.right_phase);
        };
    const auto completion_lexicographic_height =
        [&](const ReturnContactState<3>& state, std::size_t left,
            std::size_t right) {
            return std::make_pair(
                gamma_escape_rank.node_height[left] +
                    gamma_escape_rank.node_height[right],
                phase_residual(state.left_phase) +
                    phase_residual(state.right_phase));
        };
    const auto gamma_relation = adelic::derive_return_contact_gamma_relation<3>(
        automaton_images, lift, automaton, property_graph, seeds,
        2'000'000, run_completion_probe || strict_completion_probe,
        run_completion_probe && !strict_completion_probe,
        completion_scalar_height, completion_lexicographic_height);
    std::printf("gamma relation: products=%zu pairs=%zu max_fibre=%zu "
                "frontier=%zu misses=%zu gamma_misses=%zu successor_misses=%zu "
                "exact=%d cap=%d\n",
                gamma_relation.product_states, gamma_relation.relation_pairs,
                gamma_relation.max_relation_fibre, gamma_relation.frontier_seeds,
                gamma_relation.transition_misses,
                gamma_relation.gamma_lookup_misses,
                gamma_relation.successor_misses, gamma_relation.exact ? 1 : 0,
                gamma_relation.cap_hit ? 1 : 0);
    std::printf("  prefix_edge_misses=%zu\n", gamma_relation.prefix_edge_misses);
    std::printf("  terminal_sink_misses=%zu nonterminal_misses=%zu\n",
                gamma_relation.terminal_sink_misses,
                gamma_relation.nonterminal_misses);
    std::printf("  product_edges=%zu cyclic_product_sccs=%zu cyclic_product_states=%zu "
                "max_cyclic=%zu nonzero_sccs=%zu terminal_escape_sccs=%zu "
                "recurrent_lift_states=%zu\n",
                gamma_relation.product_edges,
                gamma_relation.cyclic_product_sccs,
                gamma_relation.cyclic_product_states,
                gamma_relation.cyclic_product_max_size,
                gamma_relation.cyclic_product_sccs_with_nonzero_image,
                gamma_relation.cyclic_product_sccs_with_terminal_escape,
                gamma_relation.recurrent_lift_states);
    std::printf("  threaded_pair_vertices=%zu source_surjective=%d lift_finite_to_one=%d "
                "pair_finite_to_one=%d lift_branching=%zu pair_branching=%zu entropy_bound=%d\n",
                gamma_relation.thread_pair_vertices,
                gamma_relation.thread_source_path_surjective ? 1 : 0,
                gamma_relation.thread_lift_finite_to_one ? 1 : 0,
                gamma_relation.thread_pair_finite_to_one ? 1 : 0,
                gamma_relation.thread_lift_branching_components,
                gamma_relation.thread_pair_branching_components,
                gamma_relation.thread_entropy_bound ? 1 : 0);
    std::printf("  threaded_lift_vertices=%zu lift_cyclic_sccs=%zu "
                "threaded_cyclic_sccs=%zu unthreaded_cyclic_sccs=%zu\n",
                gamma_relation.thread_lift_vertices,
                gamma_relation.lift_cyclic_sccs,
                gamma_relation.threaded_lift_cyclic_sccs,
                gamma_relation.unthreaded_lift_cyclic_sccs);
    std::printf("  cyclic_lift_states=%zu threaded=%zu unthreaded=%zu all_threaded=%d\n",
                gamma_relation.cyclic_lift_states,
                gamma_relation.threaded_cyclic_lift_states,
                gamma_relation.unthreaded_cyclic_lift_states,
                gamma_relation.all_cyclic_lift_states_threaded ? 1 : 0);
    std::printf("  relation_ambiguous_states=%zu cyclic_ambiguous=%zu "
                "left=%zu right=%zu cyclic_left=%zu cyclic_right=%zu "
                "max_left_fibre=%zu max_right_fibre=%zu\n",
                gamma_relation.relation_ambiguous_lift_states,
                gamma_relation.relation_ambiguous_cyclic_lift_states,
                gamma_relation.relation_ambiguous_left_lift_states,
                gamma_relation.relation_ambiguous_right_lift_states,
                gamma_relation.relation_ambiguous_left_cyclic_lift_states,
                gamma_relation.relation_ambiguous_right_cyclic_lift_states,
                gamma_relation.relation_max_left_fibre,
                gamma_relation.relation_max_right_fibre);
    std::printf("  ambiguity both=%zu cyclic_both=%zu fibre_mismatch=%zu "
                "cyclic_mismatch=%zu\n",
                gamma_relation.relation_ambiguous_both_lift_states,
                gamma_relation.relation_ambiguous_both_cyclic_lift_states,
                gamma_relation.relation_left_right_fibre_mismatch_states,
                gamma_relation.relation_left_right_fibre_mismatch_cyclic_lift_states);
    std::printf("  cyclic component sizes:");
    for (std::size_t i = 0; i < gamma_relation.cyclic_lift_component_sizes.size(); ++i)
        std::printf(" %zu/%zu/%zu", gamma_relation.cyclic_lift_component_sizes[i],
                    gamma_relation.threaded_cyclic_component_sizes[i],
                    gamma_relation.ambiguous_cyclic_component_sizes[i]);
    std::printf("\n  unthreaded cyclic states:");
    for (const auto state : gamma_relation.unthreaded_cyclic_lift_state_indices) {
        const auto& s = lift.states[state];
        std::printf(" %zu:[%lld,(%lld,%lld,%lld),%lld](%zu,%zu)", state,
                    s.contact.i, s.contact.x[0], s.contact.x[1],
                    s.contact.x[2], s.contact.j, s.left_phase, s.right_phase);
    }
    std::printf("\n");
    if (run_completion_probe || strict_completion_probe) {
        std::printf("  completion mode=%s\n",
                    strict_completion_probe ? "strict" : "terminal-aware");
        std::printf("  completion candidates=%zu locally_valid=%zu/%zu "
                    "live_products=%zu live_lift=%zu source_surjective=%d "
                    "cyclic_live=%zu/%zu cap=%d\n",
                    gamma_relation.completion_candidate_states,
                    gamma_relation.completion_locally_valid_products,
                    gamma_relation.completion_locally_valid_lift_states,
                    gamma_relation.completion_live_product_states,
                    gamma_relation.completion_live_lift_states,
                    gamma_relation.completion_source_surjective ? 1 : 0,
                    gamma_relation.completion_live_cyclic_lift_states,
                    gamma_relation.completion_cyclic_lift_states,
                    gamma_relation.completion_cap_hit ? 1 : 0);
            std::printf("  completion product cycles=%zu nonzero=%zu terminal_escape=%zu\n",
                        gamma_relation.completion_cyclic_product_sccs,
                        gamma_relation.completion_cyclic_product_nonzero_sccs,
                        gamma_relation.completion_cyclic_product_terminal_escape_sccs);
            std::printf("  completion live products without terminal route=%zu\n",
                        gamma_relation.completion_live_products_without_terminal_route);
            std::printf("  completion finite_escape=%d\n",
                        gamma_relation.completion_finite_escape ? 1 : 0);
            std::printf("  completion height lexicographic checked=%zu violations=%zu "
                        "terminal_outgoing=%zu max=%zu strict=%d absorbing=%d\n",
                        gamma_relation.completion_height_checked_edges,
                        gamma_relation.completion_height_violations,
                        gamma_relation.completion_height_terminal_outgoing_edges,
                        gamma_relation.completion_max_height,
                        gamma_relation.completion_height_strictly_decreasing ? 1 : 0,
                        gamma_relation.completion_height_terminals_absorbing ? 1 : 0);
            std::printf("  completion pair-rank checked=%zu primary=%zu tie=%zu "
                        "violations=%zu terminal_outgoing=%zu strict=%d absorbing=%d\n",
                        gamma_relation.completion_lexicographic_checked_edges,
                        gamma_relation.completion_lexicographic_primary_decreases,
                        gamma_relation.completion_lexicographic_secondary_tie_decreases,
                        gamma_relation.completion_lexicographic_violations,
                        gamma_relation.completion_lexicographic_terminal_outgoing_edges,
                        gamma_relation.completion_lexicographic_strictly_decreasing ? 1 : 0,
                        gamma_relation.completion_lexicographic_terminals_absorbing ? 1 : 0);
            std::printf("  completion boundary witnesses zero/nonzero=%zu "
                        "nonzero/zero=%zu two-sided=%zu\n",
                        gamma_relation.completion_zero_nonzero_terminal_witnesses,
                        gamma_relation.completion_nonzero_zero_terminal_witnesses,
                        gamma_relation.completion_two_sided_terminal_witnesses);
        std::printf("  completion product_acyclic=%d max_terminal_distance=%zu "
                    "cyclic-state distances:",
                    gamma_relation.completion_product_acyclic ? 1 : 0,
                    gamma_relation.completion_max_terminal_distance);
        for (const auto state : gamma_relation.unthreaded_cyclic_lift_state_indices) {
            const auto distance = gamma_relation.completion_min_terminal_distance_by_lift[state];
            if (distance == std::numeric_limits<std::size_t>::max())
                std::printf(" %zu:inf", state);
            else {
                const auto witness =
                    gamma_relation.completion_min_terminal_witness_by_lift[state];
                const auto& left = property_graph.nodes[witness.first];
                const auto& right = property_graph.nodes[witness.second];
                std::printf(" %zu:%zu(%s|%s;z=%d/%d;t=%d/%d)", state,
                            distance, left.gamma_key.c_str(),
                            right.gamma_key.c_str(), left.zero ? 1 : 0,
                            right.zero ? 1 : 0,
                            left.successors.empty() ? 1 : 0,
                            right.successors.empty() ? 1 : 0);
            }
        }
        std::printf("\n");
        if (!strict_completion_probe &&
            !gamma_relation.unthreaded_cyclic_lift_state_indices.empty()) {
            const long long determinant =
                powered_incidence[0][0] *
                    (powered_incidence[1][1] * powered_incidence[2][2] -
                     powered_incidence[1][2] * powered_incidence[2][1]) -
                powered_incidence[0][1] *
                    (powered_incidence[1][0] * powered_incidence[2][2] -
                     powered_incidence[1][2] * powered_incidence[2][0]) +
                powered_incidence[0][2] *
                    (powered_incidence[1][0] * powered_incidence[2][1] -
                     powered_incidence[1][1] * powered_incidence[2][0]);
            const auto primes = prime_support(determinant);
            const auto [combined_padic_bound, trusted_padic] =
                adelic::make_combined_padic_bound(primes, charpoly);
            std::printf("  terminal witness p-adic support det=%lld:", determinant);
            for (const auto state : gamma_relation.unthreaded_cyclic_lift_state_indices) {
                const auto witness =
                    gamma_relation.completion_min_terminal_witness_by_lift[state];
                const auto& left = property_graph.nodes[witness.first];
                const auto& right = property_graph.nodes[witness.second];
                std::printf(" %zu:%d/%d", state,
                            combined_padic_bound(parse_property_gamma(left)) ? 1 : 0,
                            combined_padic_bound(parse_property_gamma(right)) ? 1 : 0);
            }
            std::printf(" trusted=%d primes=", trusted_padic ? 1 : 0);
            for (const auto p : primes) std::printf(" %lld", p);
            std::printf("\n");
        }
        std::printf("  completion-unthreaded-count=%zu",
                    gamma_relation.completion_unthreaded_lift_state_indices.size());
        if (gamma_relation.completion_unthreaded_lift_state_indices.size() <= 20) {
            std::printf(" states:");
            for (const auto state : gamma_relation.completion_unthreaded_lift_state_indices)
                std::printf(" %zu", state);
        }
        std::printf("\n");
    }
    std::printf("\n");
    if (!gamma_relation.first_missing_left.empty()) {
        std::printf("  first_missing_left=%s right=%s\n",
                    gamma_relation.first_missing_left.c_str(),
                    gamma_relation.first_missing_right.c_str());
        std::printf("  alternate_letters left=%s right=%s\n",
                    gamma_relation.first_left_alternate_letters.c_str(),
                    gamma_relation.first_right_alternate_letters.c_str());
        std::printf("  source_right=%s digit=%s\n",
                    gamma_relation.first_source_right.c_str(),
                    gamma_relation.first_right_digit.c_str());
    }
    expect(gamma_relation.exact && gamma_relation.frontier_seeds > 0 &&
               gamma_relation.max_relation_fibre > 1,
           "contact lift factors through a finite relation over gamma graph");
    expect(gamma_relation.thread_lift_finite_to_one &&
               gamma_relation.thread_pair_finite_to_one &&
               gamma_relation.thread_lift_branching_components == 0 &&
               gamma_relation.thread_pair_branching_components == 0,
           "both finite relation projections have no recurrent branching");
    expect(!gamma_relation.thread_source_path_surjective &&
               gamma_relation.unthreaded_cyclic_lift_states > 0,
           "relation remains a partial threading of the full lift");
    expect(gamma_relation.cyclic_product_sccs_with_nonzero_image == 0 &&
               gamma_relation.cyclic_product_sccs_with_terminal_escape == 0,
           "synchronized recurrent threads have zero gamma image and no escape");
    expect(gamma_relation.relation_ambiguous_left_cyclic_lift_states > 0 &&
               gamma_relation.relation_ambiguous_right_cyclic_lift_states > 0,
           "one-sided ambiguity persists on recurrent lift states");
    expect(gamma_relation.relation_ambiguous_both_lift_states ==
               gamma_relation.relation_ambiguous_lift_states &&
               gamma_relation.relation_left_right_fibre_mismatch_states == 0,
           "left/right ambiguity is synchronized rather than an asymmetric leak");
    if (run_completion_probe || strict_completion_probe) {
        if (strict_completion_probe) {
            expect(gamma_relation.completion_live_lift_states == 0 &&
                       gamma_relation.completion_cyclic_product_sccs == 0,
                   "strict completion falsifies a full source-surjective factor");
            expect(!gamma_relation.completion_finite_escape,
                   "empty strict completion is not reported as a finite escape verdict");
        } else {
        // This branch is intentionally permissive: reaching a retained
        // terminal node is a boundary diagnostic, not a proof that the
        // original nonterminal relation extends to a full factor.
        expect(gamma_relation.completion_candidate_states > 0 &&
                   !gamma_relation.completion_cap_hit,
               "all compatible lift/gamma candidates fit the completion cap");
        bool recurrent_completion =
            gamma_relation.completion_live_cyclic_lift_states ==
            gamma_relation.completion_cyclic_lift_states;
        bool omitted_cyclic_have_terminal_route = true;
        bool omitted_cyclic_have_zero_to_nonzero_terminal = true;
        for (const auto state : gamma_relation.unthreaded_cyclic_lift_state_indices)
        {
            omitted_cyclic_have_terminal_route = omitted_cyclic_have_terminal_route &&
                gamma_relation.completion_min_terminal_distance_by_lift[state] !=
                    std::numeric_limits<std::size_t>::max();
            const auto witness =
                gamma_relation.completion_min_terminal_witness_by_lift[state];
            const auto& left = property_graph.nodes[witness.first];
            const auto& right = property_graph.nodes[witness.second];
            omitted_cyclic_have_zero_to_nonzero_terminal =
                omitted_cyclic_have_zero_to_nonzero_terminal && left.zero &&
                !right.zero && !left.successors.empty() &&
                right.successors.empty();
        }
        expect(recurrent_completion && omitted_cyclic_have_terminal_route &&
                   gamma_relation.completion_product_acyclic &&
                   gamma_relation.completion_cyclic_product_nonzero_sccs == 0 &&
                   gamma_relation.completion_live_products_without_terminal_route == 0 &&
                   gamma_relation.completion_finite_escape,
               "permissive terminal completion reaches every recurrent lift state without a recurrent gamma cycle");
        expect(gamma_relation.completion_height_checked_edges > 0 &&
                   gamma_relation.completion_height_terminal_outgoing_edges == 0,
               "lexicographic gamma-layer/phase rank replay is populated");
        expect(gamma_relation.completion_lexicographic_checked_edges > 0 &&
                   gamma_relation.completion_lexicographic_violations == 0 &&
                   gamma_relation.completion_lexicographic_terminal_outgoing_edges == 0 &&
                   gamma_relation.completion_lexicographic_strictly_decreasing &&
                   gamma_relation.completion_lexicographic_terminals_absorbing,
               "ordered boundary-layer/phase rank replays without scalar weighting");
        expect(gamma_relation.completion_zero_nonzero_terminal_witnesses >=
                   gamma_relation.unthreaded_cyclic_lift_state_indices.size(),
               "the omitted recurrent states are explicitly classified as one-sided boundary escapes");
        expect(gamma_relation.completion_nonzero_zero_terminal_witnesses == 0 &&
                   gamma_relation.completion_two_sided_terminal_witnesses == 0,
               "the finite boundary escape has a single zero-state-to-nonzero-state orientation");
        expect(omitted_cyclic_have_zero_to_nonzero_terminal,
               "omitted cyclic states escape from zero interior to nonzero terminal sinks");
        }
    }
    const auto capped_relation = adelic::derive_return_contact_gamma_relation<3>(
        automaton_images, lift, automaton, property_graph, seeds, 1);
    expect(capped_relation.cap_hit && !capped_relation.exact,
           "relation certificate distinguishes an explicit product cap");
    const auto contact_holonomy = adelic::derive_return_contact_digit_holonomy<3>(
        automaton_images, lift, automaton, seeds);
    std::printf("contact Q(beta) holonomy: cyclic_sccs=%zu nontrivial=%zu\n",
                contact_holonomy.cyclic_sccs,
                contact_holonomy.nontrivial_holonomy_sccs);
    std::printf("  zero_seed_count=%zu zero_seed_reachable=%zu "
                "nontrivial_reachable=%zu reaches_nontrivial=%d "
                "ambiguous_left=%zu ambiguous_right=%zu single=%d\n",
                contact_holonomy.zero_seed_count,
                contact_holonomy.zero_seed_reachable_states,
                contact_holonomy.zero_seed_reachable_nontrivial_nodes,
                contact_holonomy.zero_seed_reaches_nontrivial_holonomy ? 1 : 0,
                contact_holonomy.left_frontier_ambiguous_states,
                contact_holonomy.right_frontier_ambiguous_states,
                contact_holonomy.single_valued_frontier_projection ? 1 : 0);
    for (const auto& scc : contact_holonomy.sccs)
        std::printf("  contact_scc nodes=%zu residual_edges=%zu left=%zu "
                    "right=%zu ranks=%zu/%zu/%zu zero=%zu nonzero=%zu coboundary=%d "
                    "left_cob=%d right_cob=%d\n", scc.nodes,
                    scc.residual_edges, scc.left_residual_edges,
                    scc.right_residual_edges, scc.residual_rank,
                    scc.left_residual_rank, scc.right_residual_rank,
                    scc.zero_contact_nodes,
                    scc.nonzero_contact_nodes, scc.coboundary ? 1 : 0,
                    scc.left_coboundary ? 1 : 0,
                    scc.right_coboundary ? 1 : 0);
    expect(contact_holonomy.exact && contact_holonomy.cyclic_sccs > 0,
           "full contact lift has classified Q(beta) recurrent cycles");
    bool full_rank_frontier_residual = false;
    bool all_difference_sccs_consistent = true;
    for (const auto& scc : contact_holonomy.sccs) {
        all_difference_sccs_consistent &= scc.coboundary;
        if ((scc.nodes == 52 || scc.nodes == 401) &&
            scc.residual_rank == 3 && scc.left_residual_rank == 3 &&
            scc.right_residual_rank == 3)
            full_rank_frontier_residual = true;
    }
    expect(contact_holonomy.nontrivial_holonomy_sccs == 0 &&
               all_difference_sccs_consistent && full_rank_frontier_residual,
           "affine difference holonomy is cohomologically consistent");
    expect(contact_holonomy.zero_seed_count > 0 &&
               contact_holonomy.zero_seed_reachable_states > 0 &&
               contact_holonomy.left_frontier_ambiguous_states > 0 &&
               contact_holonomy.right_frontier_ambiguous_states > 0,
           "zero-contact frontier reaches multivalued affine transport");
    expect(!contact_holonomy.single_valued_frontier_projection,
           "frontier-to-contact projection is genuinely multivalued");
    const auto digit_cocycle = adelic::derive_property_f_role_digit_cocycle<3>(
        automaton_images, automaton, 16, 1'000'000);
    std::printf("powered digit cocycle: zero_pairs=%zu missing=%zu "
                "recurrent_missing=%zu\n",
                digit_cocycle.zero_kernel_pairs,
                digit_cocycle.zero_kernel_missing_pairs,
                digit_cocycle.recurrent_zero_kernel_missing_pairs);
    // This deliberately fails the naïve bridge hypothesis: the coarse
    // letter-role cocycle forgets the return collar and misses recurrent zero
    // witnesses even though the full Property-F graph closes.
    expect(!digit_cocycle.proved &&
               digit_cocycle.recurrent_zero_kernel_missing_pairs > 0,
           "coarse role cocycle is correctly rejected as an incomplete bridge");
    const auto collar_cocycle = adelic::derive_return_phase_digit_cocycle<3>(
        automaton_images, phases, automaton, 16, 1'000'000);
    std::printf("return-collar cocycle: phase_states=%zu edges=%zu "
                "zero_pairs=%zu missing=%zu recurrent_missing=%zu\n",
                collar_cocycle.phase_states, collar_cocycle.edge_count,
                collar_cocycle.zero_kernel_pairs,
                collar_cocycle.zero_kernel_missing_pairs,
                collar_cocycle.recurrent_zero_kernel_missing_pairs);
    expect(collar_cocycle.cocycle_edges_exact &&
               !collar_cocycle.recurrent_zero_kernel_complete &&
               collar_cocycle.recurrent_zero_kernel_missing_pairs > 0,
           "phase collar alone is correctly rejected as an incomplete bridge");
    const std::size_t nonzero_scc = largest_nonzero_recurrent_scc(lift);
    expect(nonzero_scc > 0,
           "non-AR contact lift exposes a recurrent nonzero transport component");

    std::printf("%d tests run, %d failed. boundary=%zu phases=%zu lift=%zu edges=%zu\n",
                tests, failures, bare.size(), phases.states.size(),
                lift.states.size(), lift.edges.size());
    std::printf("largest_nonzero_contact_scc=%zu\n", nonzero_scc);
    return failures == 0 ? 0 : 1;
}
