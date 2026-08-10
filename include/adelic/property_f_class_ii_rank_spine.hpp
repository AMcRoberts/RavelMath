// Explicit rank-spine replay for the three-letter Class-II sigma_{a,1}
// family.  The spine is a lower-bound witness for the finite SCC escape
// height; replaying it does not by itself prove that every other state lies
// below the same height.
#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "adelic/property_f_birth_round_grammar.hpp"

namespace adelic {

struct PropertyFClassIIRankSpineCertificate {
    std::size_t parameter_a = 0;
    std::size_t expected_height = 0;
    std::size_t matched_nodes = 0;
    std::size_t missing_states = 0;
    std::size_t missing_edges = 0;
    std::size_t missing_labels = 0;
    std::size_t rank_mismatches = 0;
    std::vector<std::size_t> node_ids;
    bool parameter_domain = false;
    bool rank_valid = false;
    bool states_present = false;
    bool edges_replayed = false;
    bool labels_replayed = false;
    bool rank_replayed = false;
    bool valid = false;
};

// The largest coefficient is a^3-1.  Keep the closed integer replay inside
// signed 64-bit arithmetic; callers needing larger parameters must promote
// the coefficient representation rather than allowing silent wraparound.
inline constexpr std::size_t property_f_class_ii_spine_max_safe_a = 2'000'000;

inline std::string property_f_class_ii_spine_key(
        const std::array<long long, 3>& coefficients) {
    return std::to_string(coefficients[0]) + ";" +
           std::to_string(coefficients[1]) + ";" +
           std::to_string(coefficients[2]) + ";";
}

// The second coordinate of each spine digit is the carry amount in the
// prefix delta [0,j,0].  The first two steps are the seed and the unique
// letter-1 return; the middle alternates k and a-k, and the final four steps
// are the fixed boundary correction.
inline std::vector<long long> property_f_class_ii_rank_spine_digits(
        std::size_t a) {
    if (a < 4) return {};
    std::vector<long long> out{1, static_cast<long long>(a)};
    for (std::size_t k = 1; k + 1 < a; ++k) {
        out.push_back(static_cast<long long>(k));
        out.push_back(static_cast<long long>(a - k));
    }
    out.push_back(static_cast<long long>(a - 2));
    out.push_back(0);
    out.push_back(static_cast<long long>(a - 1));
    out.push_back(0);
    return out;
}

inline std::string property_f_class_ii_spine_digit_key(long long digit) {
    return "0/1;" + std::to_string(digit) + "/1;0/1;";
}

// States on the observed maximal-height chain, in forward graph order.  For
// a>=4 this contains 2a+3 states, hence a candidate height 2a+2.  The first
// and last three states are the boundary corrections; the middle pairs are
// the alternating affine carry pattern.
inline std::vector<std::array<long long, 3>>
property_f_class_ii_rank_spine_coefficients(std::size_t a) {
    if (a < 4 || a > property_f_class_ii_spine_max_safe_a) return {};
    std::vector<std::array<long long, 3>> out;
    out.push_back({0, 0, 0});
    out.push_back({1, 0, 0});
    for (std::size_t k = 1; k + 1 < a; ++k) {
        const auto kk = static_cast<long long>(k);
        const auto aa = static_cast<long long>(a);
        out.push_back({-kk, -(kk * (aa + 1) - 1), kk});
        out.push_back({kk + 1, kk * (aa + 1), -kk});
    }
    const auto aa = static_cast<long long>(a);
    const auto last = static_cast<long long>(a - 1);
    out.push_back({-last, -(last * (aa + 1) - 1), last});
    out.push_back({last, aa * aa - 1, -last});
    out.push_back({0, -(aa * aa - 1), aa - 1});
    out.push_back({-aa * (aa - 1), aa - 1, 0});
    out.push_back({aa * aa * aa - 1, aa * aa * (aa - 1),
                   -aa * (aa - 1)});
    return out;
}

inline PropertyFClassIIRankSpineCertificate
derive_property_f_class_ii_rank_spine(
        const PropertyFGraph& graph, std::size_t a) {
    PropertyFClassIIRankSpineCertificate out;
    out.parameter_a = a;
    const auto coefficients = property_f_class_ii_rank_spine_coefficients(a);
    out.parameter_domain = !coefficients.empty();
    if (!out.parameter_domain) return out;
    out.expected_height = coefficients.size() - 1;

    const auto rank = derive_property_f_escape_rank(graph);
    out.rank_valid = rank.valid;
    if (!rank.valid) return out;

    std::map<std::pair<long long, std::string>, std::size_t> lookup;
    for (std::size_t node = 0; node < graph.nodes.size(); ++node)
        lookup.emplace(std::make_pair(graph.nodes[node].letter,
                                      graph.nodes[node].gamma_key), node);

    out.node_ids.reserve(coefficients.size());
    out.states_present = true;
    for (std::size_t step = 0; step < coefficients.size(); ++step) {
        const long long letter = step == 0 ? 0 : (step == 1 ? 1 : 0);
        const auto key = property_f_class_ii_spine_key(coefficients[step]);
        const auto it = lookup.find({letter, key});
        if (it == lookup.end()) {
            out.states_present = false;
            ++out.missing_states;
            out.node_ids.push_back(graph.nodes.size());
            continue;
        }
        out.node_ids.push_back(it->second);
        ++out.matched_nodes;
        const auto expected = out.expected_height - step;
        if (rank.node_height[it->second] != expected)
            ++out.rank_mismatches;
    }
    out.rank_replayed = out.rank_mismatches == 0;

    out.edges_replayed = true;
    out.labels_replayed = true;
    bool label_data_present = false;
    bool label_data_complete = true;
    for (const auto& node : graph.nodes) {
        if (!node.edge_digit_coefficients.empty()) label_data_present = true;
        if (node.edge_digit_coefficients.size() != node.successors.size())
            label_data_complete = false;
    }
    if (out.states_present && out.node_ids.size() == coefficients.size()) {
        const auto digits = property_f_class_ii_rank_spine_digits(a);
        for (std::size_t step = 0; step + 1 < out.node_ids.size(); ++step) {
            const auto source = out.node_ids[step];
            const auto target = out.node_ids[step + 1];
            bool found = false;
            bool found_label = false;
            const auto expected_label = step < digits.size()
                ? property_f_class_ii_spine_digit_key(digits[step])
                : std::string{};
            for (std::size_t edge = 0;
                 edge < graph.nodes[source].successors.size(); ++edge) {
                const auto raw_target = graph.nodes[source].successors[edge];
                if (raw_target == static_cast<long long>(target)) {
                    found = true;
                    if (edge < graph.nodes[source].edge_digit_coefficients.size() &&
                        property_f_birth_round_digit_key(
                            graph.nodes[source].edge_digit_coefficients[edge]) ==
                            expected_label)
                        found_label = true;
                }
            }
            if (!found) {
                out.edges_replayed = false;
                ++out.missing_edges;
            }
            if (label_data_present && label_data_complete && !found_label) {
                out.labels_replayed = false;
                ++out.missing_labels;
            }
        }
    } else {
        out.edges_replayed = false;
        out.labels_replayed = false;
    }
    if (!label_data_present) out.labels_replayed = true;
    out.valid = out.parameter_domain && out.rank_valid &&
                out.states_present && out.edges_replayed &&
                out.rank_replayed && out.labels_replayed;
    return out;
}

}  // namespace adelic
