// Replay the finite Property-(F) birth-round grammar across the Class-II
// sigma_{a,1} family, including the genuinely non-AR a=0 control.
// This is a finite family study: it records the exact SCC rank profile and
// refuses to promote a bounded closure into a uniform theorem.

#include <array>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

#include "adelic/property_f_birth_round_grammar.hpp"
#include "adelic/property_f_class_ii_collar_grammar.hpp"
#include "adelic/property_f_class_ii_affine_tail.hpp"
#include "adelic/property_f_class_ii_branch_census.hpp"
#include "adelic/property_f_class_ii_tail_candidate_census.hpp"
#include "adelic/property_f_class_ii_prefix_role_grammar.hpp"
#include "adelic/property_f_class_ii_phase_strip.hpp"
#include "adelic/property_f_class_ii_phase_absorption.hpp"
#include "adelic/property_f_class_ii_rank_spine.hpp"
#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"
#include "math/charpoly.hpp"
#include "math/linalg_qbeta.hpp"

namespace {

std::array<std::vector<long long>, 3> sigma_a1(std::size_t a) {
    std::array<std::vector<long long>, 3> images;
    images[0].insert(images[0].end(), a, 0);
    images[0].push_back(1);
    images[0].push_back(2);
    images[1].insert(images[1].end(), a, 0);
    images[1].push_back(2);
    images[2] = {0};
    return images;
}

template <std::size_t d>
std::vector<std::vector<long long>> incidence(
        const std::array<std::vector<long long>, d>& images) {
    std::vector<std::vector<long long>> matrix(
        d, std::vector<long long>(d, 0));
    for (std::size_t source = 0; source < d; ++source)
        for (const auto letter : images[source])
            ++matrix[static_cast<std::size_t>(letter)][source];
    return matrix;
}

}  // namespace

int main() {
    const auto env_or = [](const char* name, std::size_t fallback) {
        const char* raw = std::getenv(name);
        if (raw == nullptr || *raw == '\0') return fallback;
        return static_cast<std::size_t>(std::strtoull(raw, nullptr, 10));
    };
    const std::size_t max_a = env_or("CLASSII_BIRTH_MAX_A", 13);
    const long long node_budget = static_cast<long long>(
        env_or("CLASSII_BIRTH_NODE_BUDGET", 300'000));
    std::size_t strict = 0;
    std::size_t maximum_layers = 0;
    // The default 300k cap closes the exact family through a=13; larger
    // values are opt-in resource probes rather than default regressions.
    for (std::size_t a = 0; a <= max_a; ++a) {
        const auto images = sigma_a1(a);
        const auto matrix = incidence(images);
        const auto charpoly = mathlib::charpoly_faddeev_leverrier(matrix);
        mathlib::QBetaRing ring(charpoly);
        const auto eigen = mathlib::right_eigenvector_via_qbeta(
            [&] {
                std::vector<std::vector<long long>> transpose(
                    matrix.size(), std::vector<long long>(matrix.size(), 0));
                for (std::size_t i = 0; i < matrix.size(); ++i)
                    for (std::size_t j = 0; j < matrix.size(); ++j)
                        transpose[j][i] = matrix[i][j];
                return transpose;
            }(), ring);
        assert(eigen.ok);
        const auto automaton = adelic::build_prefix_automaton<3>(
            images, eigen.v, ring);
        adelic::PropertyFGraph graph;
        const auto result = adelic::check_property_f<3>(
            automaton, node_budget, nullptr, nullptr, nullptr, nullptr,
            &graph, true);
        assert(result.holds && !result.inconclusive);
        const auto grammar =
            adelic::derive_property_f_birth_round_grammar(graph);
        assert(grammar.valid);
        assert(grammar.terminal_layer_zero);
        assert(grammar.edge_rank_strictly_decreasing);
        if (a >= 4 && node_budget >= 300'000)
            assert(grammar.layer_count == 2 * a + 3);
        if (a >= 4) {
            for (std::size_t height = 6;
                 height < grammar.layer_count; ++height)
                assert(grammar.layer_sizes.at(height) == 1);
            const auto spine =
                adelic::derive_property_f_class_ii_rank_spine(graph, a);
            assert(spine.valid);
            assert(spine.labels_replayed);
            assert(adelic::property_f_class_ii_rank_spine_digits(a).size() ==
                   spine.node_ids.size() - 1);
            const auto affine_tail =
                adelic::derive_property_f_class_ii_affine_tail_certificate(a);
            assert(affine_tail.valid);
            assert(affine_tail.recurrence_steps == spine.node_ids.size() - 1);
            assert(spine.expected_height + 1 == grammar.layer_count);
            const auto census =
                adelic::derive_property_f_class_ii_branch_census(graph, a);
            assert(census.valid);
            assert(census.tail_non_spine_nodes == 0);
            assert(census.tail_nonchain_edges == 0);
            assert(census.tail_nodes == grammar.layer_count - 1 - 5);
            const auto collar =
                adelic::derive_property_f_class_ii_collar_grammar(graph, a);
            assert(collar.valid);
            assert(collar.no_tail_reentry);
            assert(collar.no_collar_internal_transport);
            assert(collar.height_support_valid);
            assert(collar.collar_to_tail_edges == 0);
            assert(collar.maximum_target_height <=
                   adelic::PropertyFClassIICollarGrammar::collar_height);
            assert(collar.collar_nodes ==
                   graph.nodes.size() - census.tail_nodes);
            const auto candidates =
                adelic::derive_property_f_class_ii_tail_candidate_census(graph, a);
            assert(candidates.valid);
            assert(candidates.no_alternate_high);
            assert(candidates.alternate_high_edges == 0);
            assert(candidates.digit_support_valid);
            assert(candidates.predecessor_unique);
            assert(candidates.no_collar_predecessor);
            assert(candidates.no_alternate_high_predecessor);
            const auto absorption =
                adelic::derive_property_f_class_ii_phase_absorption(graph, a);
            assert(absorption.valid);
            assert(absorption.phase_seed_nodes == 1);
            assert(absorption.phase_invalid_unexpected_high_nodes == 0);
            if (std::getenv("CLASSII_BIRTH_TAIL_CANDIDATE_PROBE") != nullptr &&
                a == max_a) {
                std::cout << "tail_candidates a=" << a
                          << " sources=" << candidates.tail_sources
                          << " expected=" << candidates.expected_edges
                          << " alternate_collar="
                          << candidates.alternate_collar_edges
                          << " alternate_high="
                          << candidates.alternate_high_edges
                          << " internal=" << candidates.internal_edges
                          << "\n";
                for (const auto& [step, labels] :
                     candidates.collar_labels_by_step) {
                    std::cout << "tail_step=" << step << " collar_labels=";
                    for (const auto& label : labels) std::cout << label << ",";
                    std::cout << "\n";
                }
            }
            if (std::getenv("CLASSII_BIRTH_COLLAR_PROBE") != nullptr &&
                a == max_a) {
                std::cout << "collar a=" << a
                          << " nodes=" << collar.collar_nodes
                          << " edges=" << collar.collar_edges
                          << " branching_nodes=" << collar.branching_nodes
                          << " internal=" << collar.collar_internal_edges
                          << " cross_scc=" << collar.collar_cross_scc_edges
                          << " max_target_height="
                          << collar.maximum_target_height << " layers=";
                for (const auto& [height, count] : collar.layer_sizes)
                    if (height <= adelic::PropertyFClassIICollarGrammar::collar_height)
                        std::cout << height << ":" << count << ",";
                std::cout << " transitions=";
                for (const auto& [heights, count] :
                     collar.height_transition_counts)
                    std::cout << heights.first << "->" << heights.second
                              << ":" << count << ",";
                std::cout << "\n";
                if (std::getenv("CLASSII_BIRTH_COLLAR_DETAIL_PROBE") != nullptr) {
                    const auto rank = adelic::derive_property_f_escape_rank(graph);
                    for (std::size_t node = 0; node < graph.nodes.size(); ++node) {
                        if (rank.node_height[node] < 4 ||
                            rank.node_height[node] > 5)
                            continue;
                        std::cout << "collar_state node=" << node
                                  << " h=" << rank.node_height[node]
                                  << " letter=" << graph.nodes[node].letter
                                  << " gamma=" << graph.nodes[node].gamma_key
                                  << " edges=";
                        for (std::size_t edge = 0;
                             edge < graph.nodes[node].successors.size(); ++edge) {
                            const auto target = static_cast<std::size_t>(
                                graph.nodes[node].successors[edge]);
                            const auto label = edge <
                                    graph.nodes[node].edge_digit_coefficients.size()
                                ? adelic::property_f_birth_round_digit_key(
                                      graph.nodes[node].edge_digit_coefficients[edge])
                                : std::string{};
                            std::cout << rank.node_height[target] << "["
                                      << label << "],";
                        }
                        std::cout << "\n";
                    }
                }
            }
            if (a == 4) {
                auto bad = graph;
                bad.nodes[spine.node_ids.front()].successors.clear();
                assert(!adelic::derive_property_f_class_ii_rank_spine(
                                bad, a).valid);
            }
        }
        ++strict;
        maximum_layers = std::max(maximum_layers, grammar.layer_count);

        std::cout << "a=" << a << " nodes=" << graph.nodes.size()
                  << " edges=" << grammar.edge_count
                  << " layers=" << grammar.layer_count << " profile=";
        for (const auto& [layer, count] : grammar.layer_sizes)
            std::cout << layer << ":" << count << ",";
        std::cout << " drops=";
        for (const auto& [drop, count] : grammar.drop_histogram)
            std::cout << drop << ":" << count << ",";
        std::cout << "\n";
        if (std::getenv("CLASSII_BIRTH_TOP_PROBE") != nullptr &&
            a == max_a) {
            const auto rank = adelic::derive_property_f_escape_rank(graph);
            const auto cutoff = rank.maximum_height > 3
                ? rank.maximum_height - 3 : 0;
            for (std::size_t node = 0; node < graph.nodes.size(); ++node) {
                if (rank.node_height[node] < cutoff) continue;
                const auto& state = graph.nodes[node];
                std::cout << "top node=" << node
                          << " h=" << rank.node_height[node]
                          << " letter=" << state.letter
                          << " zero=" << state.zero
                          << " succ=" << state.successors.size()
                          << " gamma=" << state.gamma_key << "\n";
            }
            std::size_t node = 0;
            for (std::size_t step = 0; step <= rank.maximum_height &&
                                            node < graph.nodes.size(); ++step) {
                const auto& state = graph.nodes[node];
                std::cout << "rank chain step=" << step << " node=" << node
                          << " h=" << rank.node_height[node]
                          << " letter=" << state.letter
                          << " gamma=" << state.gamma_key << "\n";
                std::size_t next = graph.nodes.size();
                std::string next_label;
                for (const auto raw_target : state.successors) {
                    if (raw_target < 0 ||
                        static_cast<std::size_t>(raw_target) >= graph.nodes.size())
                        continue;
                    const auto candidate = static_cast<std::size_t>(raw_target);
                    if (rank.node_height[candidate] + 1 ==
                        rank.node_height[node]) {
                        next = candidate;
                        const auto edge = std::find(state.successors.begin(),
                                                    state.successors.end(), raw_target);
                        const auto edge_index = static_cast<std::size_t>(
                            edge - state.successors.begin());
                        if (edge_index < state.edge_digit_coefficients.size())
                            next_label =
                                adelic::property_f_birth_round_digit_key(
                                    state.edge_digit_coefficients[edge_index]);
                        break;
                    }
                }
                if (next == graph.nodes.size()) break;
                std::cout << "rank chain edge label=" << next_label << "\n";
                node = next;
            }
        }
        if (std::getenv("CLASSII_BIRTH_FEATURE_PROBE") != nullptr &&
            a == max_a) {
            const auto rank = adelic::derive_property_f_escape_rank(graph);
            for (std::size_t height = 0; height <= rank.maximum_height;
                 ++height) {
                std::array<long long, 3> lo{
                    std::numeric_limits<long long>::max(),
                    std::numeric_limits<long long>::max(),
                    std::numeric_limits<long long>::max()};
                std::array<long long, 3> hi{
                    std::numeric_limits<long long>::min(),
                    std::numeric_limits<long long>::min(),
                    std::numeric_limits<long long>::min()};
                std::size_t count = 0;
                for (std::size_t node = 0; node < graph.nodes.size(); ++node) {
                    if (rank.node_height[node] != height) continue;
                    ++count;
                    for (std::size_t coordinate = 0; coordinate < 3;
                         ++coordinate) {
                        // The classifier stores exact integer coefficient
                        // strings for this unimodular family.
                        const auto& coefficient =
                            graph.nodes[node].gamma_coefficients[coordinate].first;
                        const auto value = std::stoll(coefficient);
                        lo[coordinate] = std::min(lo[coordinate], value);
                        hi[coordinate] = std::max(hi[coordinate], value);
                    }
                }
                std::cout << "features h=" << height << " count=" << count
                          << " x=" << lo[0] << ":" << hi[0]
                          << " y=" << lo[1] << ":" << hi[1]
                          << " z=" << lo[2] << ":" << hi[2] << "\n";
            }
        }
    }
    assert(strict == max_a + 1);
    assert(maximum_layers > 0);
    // The affine recurrence is cheap and independent of graph closure, so
    // exercise its parameter formula beyond the resource-bounded graph sweep.
    for (std::size_t a = 4; a <= 256; ++a)
        assert(adelic::derive_property_f_class_ii_affine_tail_certificate(a).valid);
    for (std::size_t a = 4; a <= 256; ++a) {
        const auto roles =
            adelic::derive_property_f_class_ii_prefix_role_grammar(a);
        assert(roles.valid);
        assert(roles.zero_prefix_count == a);
        const auto phase =
            adelic::derive_property_f_class_ii_phase_strip_certificate(a);
        assert(phase.valid);
        assert(phase.phase_preserving_digit_mismatches == 0);
    }
    assert(!adelic::derive_property_f_class_ii_affine_tail_certificate(
        adelic::property_f_class_ii_spine_max_safe_a + 1).parameter_domain);
    std::cout << "property_f_class_ii_birth_round: PASS family_size="
              << strict << " max_layers=" << maximum_layers << "\n";
}
