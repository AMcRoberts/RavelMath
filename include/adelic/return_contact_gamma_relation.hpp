// Finite relation between the reachable return/contact lift and an exact
// Property-F gamma graph.  This deliberately records a partial,
// relation-valued simulation: a lift state may correspond to several
// left/right gamma pairs, and some lift states may have no synchronized pair.
#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "adelic/property_f_types.hpp"
#include "adelic/prefix_automaton.hpp"
#include "ravel/proof/finite_graph_correspondence.hpp"
#include "ravel/return_contact_lift.hpp"

namespace adelic {

struct ReturnContactGammaRelationCertificate {
    // `exact` means exact closure of the explored relation away from retained
    // terminal sinks.  It does not assert source-surjectivity onto the full
    // contact lift; consult thread_source_path_surjective and the recurrent
    // coverage fields for that stronger property.
    std::size_t lift_states = 0;
    std::size_t product_states = 0;
    std::size_t relation_pairs = 0;
    std::size_t max_relation_fibre = 0;
    std::size_t frontier_seeds = 0;
    std::size_t transition_misses = 0;
    std::size_t gamma_lookup_misses = 0;
    std::size_t successor_misses = 0;
    std::size_t prefix_edge_misses = 0;
    std::size_t terminal_sink_misses = 0;
    std::size_t nonterminal_misses = 0;
    std::size_t product_edges = 0;
    std::size_t cyclic_product_sccs = 0;
    std::size_t cyclic_product_states = 0;
    std::size_t cyclic_product_max_size = 0;
    std::size_t cyclic_product_sccs_with_nonzero_image = 0;
    std::size_t cyclic_product_sccs_with_terminal_escape = 0;
    std::size_t recurrent_lift_states = 0;
    std::size_t thread_lift_vertices = 0;
    std::size_t lift_cyclic_sccs = 0;
    std::size_t threaded_lift_cyclic_sccs = 0;
    std::size_t unthreaded_lift_cyclic_sccs = 0;
    std::size_t cyclic_lift_states = 0;
    std::size_t threaded_cyclic_lift_states = 0;
    std::size_t unthreaded_cyclic_lift_states = 0;
    std::size_t relation_ambiguous_lift_states = 0;
    std::size_t relation_ambiguous_cyclic_lift_states = 0;
    std::size_t relation_ambiguous_left_lift_states = 0;
    std::size_t relation_ambiguous_right_lift_states = 0;
    std::size_t relation_ambiguous_left_cyclic_lift_states = 0;
    std::size_t relation_ambiguous_right_cyclic_lift_states = 0;
    std::size_t relation_ambiguous_both_lift_states = 0;
    std::size_t relation_ambiguous_both_cyclic_lift_states = 0;
    std::size_t relation_left_right_fibre_mismatch_states = 0;
    std::size_t relation_left_right_fibre_mismatch_cyclic_lift_states = 0;
    std::size_t relation_max_left_fibre = 0;
    std::size_t relation_max_right_fibre = 0;
    // Per recurrent lift component coverage.  The aggregate counts above
    // are useful for bounds, but they can hide a small omitted recurrent
    // piece behind a large component.
    std::vector<std::size_t> cyclic_lift_component_sizes;
    std::vector<std::size_t> threaded_cyclic_component_sizes;
    std::vector<std::size_t> ambiguous_cyclic_component_sizes;
    std::vector<std::size_t> unthreaded_cyclic_lift_state_indices;
    bool all_cyclic_lift_states_threaded = false;
    std::size_t thread_pair_vertices = 0;
    std::size_t thread_lift_branching_components = 0;
    std::size_t thread_pair_branching_components = 0;
    bool thread_source_path_surjective = false;
    bool thread_lift_finite_to_one = false;
    bool thread_pair_finite_to_one = false;
    bool thread_entropy_bound = false;
    std::string first_missing_left;
    std::string first_missing_right;
    std::string first_left_alternate_letters;
    std::string first_right_alternate_letters;
    std::string first_source_right;
    std::string first_right_digit;
    bool graph_closed = false;
    bool exact = false;
    bool cap_hit = false;
};

template <std::size_t d>
ReturnContactGammaRelationCertificate derive_return_contact_gamma_relation(
        const std::array<std::vector<long long>, d>& images,
        const ravel::ReturnContactLift<d>& lift,
        const PrefixAutomaton<d>& automaton,
        const PropertyFGraph& property_graph,
        const std::vector<ravel::ReturnContactState<d>>& seeds,
        std::size_t product_cap = 2'000'000) {
    struct Product { std::size_t lift; long long left; long long right; };
    ReturnContactGammaRelationCertificate out;
    out.lift_states = lift.states.size();
    std::map<ravel::ReturnContactState<d>, std::size_t> lift_index;
    for (std::size_t i = 0; i < lift.states.size(); ++i)
        lift_index.emplace(lift.states[i], i);
    std::map<std::pair<std::string, long long>, long long> gamma_index;
    for (std::size_t i = 0; i < property_graph.nodes.size(); ++i)
        gamma_index.emplace(
            std::make_pair(property_graph.nodes[i].gamma_key,
                           property_graph.nodes[i].letter),
            static_cast<long long>(i));
    const auto inverse = mathlib::invert_in_qbeta(
        automaton.ring.beta_k(1), automaton.ring);
    if (!inverse.invertible)
        throw std::runtime_error("return contact gamma relation: beta is not invertible");
    const auto inverse_beta = inverse.inverse;
    auto step = [&](const mathlib::QElem& gamma,
                    const mathlib::QElem& digit) {
        return automaton.ring.mul(inverse_beta,
            automaton.ring.add(gamma, digit));
    };
    auto prefix_digit = [&](long long letter, std::size_t length) {
        auto value = automaton.ring.zero();
        if (letter < 0 || static_cast<std::size_t>(letter) >= d) return value;
        const auto& image = images[static_cast<std::size_t>(letter)];
        if (length > image.size()) return value;
        for (std::size_t i = 0; i < length; ++i)
            value = automaton.ring.add(value,
                automaton.left_eigenvector[static_cast<std::size_t>(image[i])]);
        return value;
    };
    auto parse_rat = [](const std::string& numerator,
                        const std::string& denominator) {
        mathlib::Rat out_rat;
        const std::string encoded = numerator + "/" + denominator;
        if (mpq_set_str(out_rat.get(), encoded.c_str(), 10) != 0)
            throw std::invalid_argument("return contact gamma relation: invalid rational");
        mpq_canonicalize(out_rat.get());
        return out_rat;
    };
    auto parse_gamma = [&](const PropertyFGraphNode& node) {
        if (node.gamma_coefficients.size() != automaton.ring.degree())
            throw std::invalid_argument("return contact gamma relation: bad gamma degree");
        mathlib::QElem out_gamma(automaton.ring.degree());
        for (std::size_t i = 0; i < node.gamma_coefficients.size(); ++i)
            out_gamma.coeff(i) = parse_rat(node.gamma_coefficients[i].first,
                                            node.gamma_coefficients[i].second);
        return out_gamma;
    };
    struct LiftEdge {
        std::size_t to;
        std::size_t left;
        std::size_t right;
        std::size_t source_edge;
    };
    std::vector<std::vector<LiftEdge>> edges(lift.states.size());
    for (std::size_t source_edge = 0; source_edge < lift.edges.size(); ++source_edge) {
        const auto& edge = lift.edges[source_edge];
        edges[edge.source].push_back({edge.destination, edge.left_position,
                                      edge.right_position, source_edge});
    }
    std::vector<std::set<std::pair<long long, long long>>> relation(lift.states.size());
    using ProductKey = std::tuple<std::size_t, long long, long long>;
    std::map<ProductKey, std::size_t> product_index;
    std::vector<Product> products;
    struct ProductTransition {
        std::size_t destination = 0;
        std::size_t source_edge = 0;
    };
    std::vector<std::vector<ProductTransition>> product_adjacency;
    std::vector<bool> product_terminal_escape;
    std::deque<std::size_t> queue;
    auto enqueue_product = [&](const Product& product) {
        const ProductKey key = std::make_tuple(product.lift, product.left,
                                                product.right);
        const auto found = product_index.find(key);
        if (found != product_index.end()) return found->second;
        const std::size_t id = products.size();
        product_index.emplace(key, id);
        products.push_back(product);
        product_adjacency.emplace_back();
        product_terminal_escape.push_back(false);
        queue.push_back(id);
        return id;
    };
    const auto zero = automaton.ring.zero();
    for (const auto& seed : seeds) {
        bool zero_contact = true;
        for (const auto coordinate : seed.contact.x)
            if (coordinate != 0) zero_contact = false;
        if (!zero_contact) continue;
        const auto li = lift_index.find(seed);
        if (li == lift_index.end()) continue;
        const auto left = gamma_index.find({qelem_key(zero), seed.contact.i});
        const auto right = gamma_index.find({qelem_key(zero), seed.contact.j});
        if (left == gamma_index.end() || right == gamma_index.end()) {
            ++out.transition_misses;
            continue;
        }
        ++out.frontier_seeds;
        enqueue_product({li->second, left->second, right->second});
    }
    out.graph_closed = true;
    if (products.size() > product_cap) {
        out.cap_hit = true;
        out.graph_closed = false;
    }
    if (product_cap == 0) {
        out.cap_hit = true;
        out.graph_closed = false;
    }
    while (!queue.empty() && !out.cap_hit) {
        const auto current_id = queue.front();
        queue.pop_front();
        const auto current = products[current_id];
        ++out.product_states;
        relation[current.lift].insert({current.left, current.right});
        for (const auto& edge : edges[current.lift]) {
            const auto& destination = lift.states.at(edge.to).contact;
            const auto& source_contact = lift.states.at(current.lift).contact;
            auto has_prefix_edge = [&](long long source, long long target,
                                       std::size_t length) {
                if (source < 0 || static_cast<std::size_t>(source) >= d) return false;
                const auto& image = images[static_cast<std::size_t>(source)];
                if (length > image.size()) return false;
                std::vector<long long> prefix(image.begin(), image.begin() + length);
                for (const auto& candidate : automaton.by_source[static_cast<std::size_t>(source)])
                    if (candidate.first == target && candidate.second == prefix) return true;
                return false;
            };
            if (!has_prefix_edge(destination.i, source_contact.i, edge.left) ||
                !has_prefix_edge(destination.j, source_contact.j, edge.right))
                ++out.prefix_edge_misses;
            const auto left_digit = prefix_digit(destination.i, edge.left);
            const auto right_digit = prefix_digit(destination.j, edge.right);
            const auto& left_node = property_graph.nodes.at(
                static_cast<std::size_t>(current.left));
            const auto& right_node = property_graph.nodes.at(
                static_cast<std::size_t>(current.right));
            const auto left_next_gamma = step(parse_gamma(left_node), left_digit);
            const auto right_next_gamma = step(parse_gamma(right_node), right_digit);
            const auto left_next_it = gamma_index.find(
                {qelem_key(left_next_gamma), destination.i});
            const auto right_next_it = gamma_index.find(
                {qelem_key(right_next_gamma), destination.j});
            long long next_left = left_next_it == gamma_index.end()
                ? -1 : left_next_it->second;
            long long next_right = right_next_it == gamma_index.end()
                ? -1 : right_next_it->second;
            if (next_left >= 0) {
                const auto& successors = left_node.successors;
                if (std::find(successors.begin(), successors.end(), next_left) == successors.end())
                    next_left = -1;
            }
            if (next_right >= 0) {
                const auto& successors = right_node.successors;
                if (std::find(successors.begin(), successors.end(), next_right) == successors.end())
                    next_right = -1;
            }
            if (next_left < 0 || next_right < 0) {
                ++out.transition_misses;
                if (left_next_it == gamma_index.end() ||
                    right_next_it == gamma_index.end()) ++out.gamma_lookup_misses;
                else ++out.successor_misses;
                if (out.gamma_lookup_misses == 1) {
                    out.first_missing_left = qelem_key(left_next_gamma) +
                        "|" + std::to_string(destination.i);
                    out.first_missing_right = qelem_key(right_next_gamma) +
                        "|" + std::to_string(destination.j);
                    out.first_source_right = right_node.gamma_key + "|" +
                        std::to_string(right_node.letter) + " -> " +
                        std::to_string(destination.j);
                    out.first_right_digit = qelem_key(right_digit);
                    for (std::size_t i = 0; i < property_graph.nodes.size(); ++i) {
                        if (property_graph.nodes[i].gamma_key == qelem_key(left_next_gamma))
                            out.first_left_alternate_letters +=
                                std::to_string(property_graph.nodes[i].letter) + ",";
                        if (property_graph.nodes[i].gamma_key == qelem_key(right_next_gamma))
                            out.first_right_alternate_letters +=
                                std::to_string(property_graph.nodes[i].letter) + ",";
                    }
                }
                const bool left_terminal = left_node.successors.empty();
                const bool right_terminal = right_node.successors.empty();
                if (left_terminal || right_terminal) {
                    ++out.terminal_sink_misses;
                    product_terminal_escape[current_id] = true;
                } else {
                    ++out.nonterminal_misses;
                    out.graph_closed = false;
                }
                continue;
            }
            if (products.size() >= product_cap) {
                out.cap_hit = true;
                out.graph_closed = false;
                break;
            }
            const auto next_id = enqueue_product({edge.to, next_left, next_right});
            product_adjacency[current_id].push_back({next_id, edge.source_edge});
            ++out.product_edges;
        }
    }
    for (const auto& states : relation) {
        out.relation_pairs += states.size();
        out.max_relation_fibre = std::max(out.max_relation_fibre, states.size());
    }
    // Tarjan SCCs on the finite product relation.  A terminal sink can only
    // be an outgoing frontier escape: it has no graph successor and therefore
    // cannot itself lie in a cyclic product component.
    const std::size_t product_count = products.size();
    std::vector<int> index(product_count, -1), low(product_count, 0), stack;
    std::vector<bool> active(product_count, false);
    std::size_t next_index = 0;
    std::size_t recurrent_lift_count = 0;
    std::function<void(std::size_t)> visit = [&](std::size_t node) {
        index[node] = low[node] = static_cast<int>(next_index++);
        stack.push_back(static_cast<int>(node));
        active[node] = true;
        for (const auto transition : product_adjacency[node]) {
            const auto successor = transition.destination;
            if (index[successor] < 0) {
                visit(successor);
                low[node] = std::min(low[node], low[successor]);
            } else if (active[successor]) {
                low[node] = std::min(low[node], index[successor]);
            }
        }
        if (low[node] != index[node]) return;
        std::vector<std::size_t> component;
        while (true) {
            const auto member = static_cast<std::size_t>(stack.back());
            stack.pop_back();
            active[member] = false;
            component.push_back(member);
            if (member == node) break;
        }
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty()) {
            const auto member = component.front();
            cyclic = std::any_of(product_adjacency[member].begin(),
                                 product_adjacency[member].end(),
                                 [member](const auto& transition) {
                                     return transition.destination == member;
                                 });
        }
        if (!cyclic) return;
        ++out.cyclic_product_sccs;
        out.cyclic_product_states += component.size();
        out.cyclic_product_max_size = std::max(out.cyclic_product_max_size,
                                               component.size());
        bool nonzero = false;
        bool terminal_escape = false;
        std::set<std::size_t> recurrent_lifts;
        for (const auto member : component) {
            const auto& product = products[member];
            nonzero = nonzero || !property_graph.nodes[
                static_cast<std::size_t>(product.left)].zero;
            nonzero = nonzero || !property_graph.nodes[
                static_cast<std::size_t>(product.right)].zero;
            terminal_escape = terminal_escape || product_terminal_escape[member];
            recurrent_lifts.insert(product.lift);
        }
        if (nonzero) ++out.cyclic_product_sccs_with_nonzero_image;
        if (terminal_escape) ++out.cyclic_product_sccs_with_terminal_escape;
        recurrent_lift_count += recurrent_lifts.size();
    };
    for (std::size_t node = 0; node < product_count; ++node)
        if (index[node] < 0) visit(node);
    out.recurrent_lift_states = recurrent_lift_count;

    // Reuse the repository's finite-correspondence threading primitive.  The
    // product relation projects to the concrete lift and to the paired gamma
    // graph.  Its off-diagonal fibre products classify recurrent ambiguity:
    // a finite permutation fibre is harmless, while a branching recurrent
    // fibre would falsify the finite-threading hypothesis.
    std::vector<std::pair<std::size_t, std::size_t>> lift_edges;
    lift_edges.reserve(lift.edges.size());
    for (const auto& edge : lift.edges)
        lift_edges.push_back({edge.source, edge.destination});
    std::vector<std::size_t> product_to_lift(product_count);
    std::vector<std::vector<ravel::proof::CorrespondenceEdge>> to_lift(product_count);
    for (std::size_t i = 0; i < product_count; ++i)
        product_to_lift[i] = products[i].lift;
    for (std::size_t source = 0; source < product_count; ++source)
        for (const auto& transition : product_adjacency[source])
            to_lift[source].push_back({transition.destination, transition.source_edge});

    using GammaPair = std::pair<long long, long long>;
    std::map<GammaPair, std::size_t> pair_index;
    std::vector<GammaPair> pair_vertices;
    for (const auto& product : products) {
        const GammaPair key{product.left, product.right};
        if (!pair_index.count(key)) {
            pair_index.emplace(key, pair_vertices.size());
            pair_vertices.push_back(key);
        }
    }
    out.thread_pair_vertices = pair_vertices.size();
    std::vector<std::size_t> product_to_pair(product_count);
    for (std::size_t i = 0; i < product_count; ++i)
        product_to_pair[i] = pair_index.at({products[i].left, products[i].right});

    std::map<std::pair<std::size_t, std::size_t>, std::size_t> pair_edge_index;
    std::vector<std::pair<std::size_t, std::size_t>> pair_edges;
    std::vector<std::vector<ravel::proof::CorrespondenceEdge>> to_pair(product_count);
    for (std::size_t source = 0; source < product_count; ++source) {
        for (const auto& transition : product_adjacency[source]) {
            const auto pair_edge = std::make_pair(
                product_to_pair[source], product_to_pair[transition.destination]);
            auto [it, inserted] = pair_edge_index.emplace(pair_edge, pair_edges.size());
            if (inserted) pair_edges.push_back(pair_edge);
            to_pair[source].push_back({transition.destination, it->second});
        }
    }
    const auto lift_thread = ravel::proof::derive_finite_to_one_graph_map(
        product_to_lift, lift.states.size(), to_lift, lift_edges);
    const auto pair_thread = ravel::proof::derive_finite_to_one_graph_map(
        product_to_pair, pair_vertices.size(), to_pair, pair_edges);
    std::set<std::size_t> represented_lift_states(product_to_lift.begin(),
                                                  product_to_lift.end());
    out.thread_lift_vertices = represented_lift_states.size();
    out.thread_source_path_surjective =
        represented_lift_states.size() == lift.states.size();
    out.thread_lift_branching_components = lift_thread.branching_recurrent_components;
    out.thread_pair_branching_components = pair_thread.branching_recurrent_components;
    out.thread_lift_finite_to_one = lift_thread.finite_to_one;
    out.thread_pair_finite_to_one = pair_thread.finite_to_one;
    out.thread_entropy_bound = out.thread_source_path_surjective &&
        lift_thread.finite_to_one && pair_thread.finite_to_one;
    std::vector<std::vector<std::size_t>> lift_adjacency(lift.states.size());
    for (const auto& edge : lift.edges)
        lift_adjacency[edge.source].push_back(edge.destination);
    std::vector<bool> cyclic_lift_state(lift.states.size(), false);
    for (const auto& component : ravel::proof::correspondence_detail::sccs(
             lift_adjacency)) {
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty()) {
            const auto state = component.front();
            cyclic = std::find(lift_adjacency[state].begin(),
                               lift_adjacency[state].end(), state)
                     != lift_adjacency[state].end();
        }
        if (!cyclic) continue;
        for (const auto state : component) cyclic_lift_state[state] = true;
        ++out.lift_cyclic_sccs;
        bool represented = false;
        std::size_t represented_count = 0;
        std::size_t ambiguous_count = 0;
        for (const auto state : component)
            if (represented_lift_states.count(state) != 0) {
                represented = true;
                ++represented_count;
            }
        for (const auto state : component) {
            std::set<long long> left_values;
            std::set<long long> right_values;
            for (const auto [left, right] : relation[state]) {
                left_values.insert(left);
                right_values.insert(right);
            }
            if (left_values.size() > 1 || right_values.size() > 1)
                ++ambiguous_count;
        }
        out.cyclic_lift_states += component.size();
        out.threaded_cyclic_lift_states += represented_count;
        out.unthreaded_cyclic_lift_states += component.size() - represented_count;
        out.cyclic_lift_component_sizes.push_back(component.size());
        out.threaded_cyclic_component_sizes.push_back(represented_count);
        out.ambiguous_cyclic_component_sizes.push_back(ambiguous_count);
        if (represented) ++out.threaded_lift_cyclic_sccs;
        else ++out.unthreaded_lift_cyclic_sccs;
        for (const auto state : component)
            if (represented_lift_states.count(state) == 0)
                out.unthreaded_cyclic_lift_state_indices.push_back(state);
    }
    out.all_cyclic_lift_states_threaded =
        out.cyclic_lift_states == out.threaded_cyclic_lift_states;
    for (std::size_t state = 0; state < relation.size(); ++state) {
        std::set<long long> left_values;
        std::set<long long> right_values;
        for (const auto [left, right] : relation[state]) {
            left_values.insert(left);
            right_values.insert(right);
        }
        out.relation_max_left_fibre = std::max(out.relation_max_left_fibre,
                                               left_values.size());
        out.relation_max_right_fibre = std::max(out.relation_max_right_fibre,
                                               right_values.size());
        const bool left_ambiguous = left_values.size() > 1;
        const bool right_ambiguous = right_values.size() > 1;
        if (left_ambiguous) {
            ++out.relation_ambiguous_left_lift_states;
            if (cyclic_lift_state[state])
                ++out.relation_ambiguous_left_cyclic_lift_states;
        }
        if (right_ambiguous) {
            ++out.relation_ambiguous_right_lift_states;
            if (cyclic_lift_state[state])
                ++out.relation_ambiguous_right_cyclic_lift_states;
        }
        if (left_ambiguous && right_ambiguous) {
            ++out.relation_ambiguous_both_lift_states;
            if (cyclic_lift_state[state])
                ++out.relation_ambiguous_both_cyclic_lift_states;
        }
        if (left_values.size() != right_values.size()) {
            ++out.relation_left_right_fibre_mismatch_states;
            if (cyclic_lift_state[state])
                ++out.relation_left_right_fibre_mismatch_cyclic_lift_states;
        }
        if (left_ambiguous || right_ambiguous) {
            ++out.relation_ambiguous_lift_states;
            if (cyclic_lift_state[state])
                ++out.relation_ambiguous_cyclic_lift_states;
        }
    }
    out.exact = !out.cap_hit && out.graph_closed && out.nonterminal_misses == 0;
    return out;
}

}  // namespace adelic
