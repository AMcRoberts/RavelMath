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
#include "adelic/property_f_class_ii_branch_census.hpp"
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
            assert(spine.expected_height + 1 == grammar.layer_count);
            const auto census =
                adelic::derive_property_f_class_ii_branch_census(graph, a);
            assert(census.valid);
            assert(census.tail_non_spine_nodes == 0);
            assert(census.tail_nonchain_edges == 0);
            assert(census.tail_nodes == grammar.layer_count - 1 - 5);
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
                for (const auto raw_target : state.successors) {
                    if (raw_target < 0 ||
                        static_cast<std::size_t>(raw_target) >= graph.nodes.size())
                        continue;
                    const auto candidate = static_cast<std::size_t>(raw_target);
                    if (rank.node_height[candidate] + 1 ==
                        rank.node_height[node]) {
                        next = candidate;
                        break;
                    }
                }
                if (next == graph.nodes.size()) break;
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
    std::cout << "property_f_class_ii_birth_round: PASS family_size="
              << strict << " max_layers=" << maximum_layers << "\n";
}
