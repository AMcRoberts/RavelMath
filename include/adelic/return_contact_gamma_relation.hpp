// Finite relation between the reachable return/contact lift and an exact
// Property-F gamma graph.  This deliberately records a relation-valued
// simulation: a lift state may correspond to several left/right gamma pairs.
#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "adelic/property_f_types.hpp"
#include "adelic/prefix_automaton.hpp"
#include "ravel/return_contact_lift.hpp"

namespace adelic {

struct ReturnContactGammaRelationCertificate {
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
    struct LiftEdge { std::size_t to; std::size_t left; std::size_t right; };
    std::vector<std::vector<LiftEdge>> edges(lift.states.size());
    for (const auto& edge : lift.edges)
        edges[edge.source].push_back({edge.destination, edge.left_position,
                                      edge.right_position});
    std::vector<std::set<std::pair<long long, long long>>> relation(lift.states.size());
    std::set<std::tuple<std::size_t, long long, long long>> seen;
    std::deque<Product> queue;
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
        const auto item = std::make_tuple(li->second, left->second, right->second);
        if (seen.insert(item).second)
            queue.push_back({li->second, left->second, right->second});
    }
    out.graph_closed = true;
    if (seen.size() > product_cap) {
        out.cap_hit = true;
        out.graph_closed = false;
    }
    if (product_cap == 0) {
        out.cap_hit = true;
        out.graph_closed = false;
    }
    while (!queue.empty() && !out.cap_hit) {
        const auto current = queue.front();
        queue.pop_front();
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
                } else {
                    ++out.nonterminal_misses;
                    out.graph_closed = false;
                }
                continue;
            }
            if (seen.size() >= product_cap) {
                out.cap_hit = true;
                out.graph_closed = false;
                break;
            }
            const auto item = std::make_tuple(edge.to, next_left, next_right);
            if (seen.insert(item).second)
                queue.push_back({edge.to, next_left, next_right});
        }
    }
    for (const auto& states : relation) {
        out.relation_pairs += states.size();
        out.max_relation_fibre = std::max(out.max_relation_fibre, states.size());
    }
    out.exact = !out.cap_hit && out.graph_closed && out.nonterminal_misses == 0;
    return out;
}

}  // namespace adelic
