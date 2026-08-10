// Return-collar refinement of the finite Property-F digit cocycle.
//
// The bare letter-role cocycle can forget recognizability/offset data.  This
// variant keeps ordered pairs of ReturnPhaseSystem states as its finite base,
// while using the exact Q(beta) prefix digit as the edge label.
#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "ravel/return_contact_lift.hpp"

namespace adelic {

struct ReturnPhaseDigitCocycleCertificate {
    std::size_t phase_states = 0;
    std::size_t pair_states = 0;
    std::size_t edge_count = 0;
    std::size_t word_cap = 0;
    std::size_t zero_kernel_pairs = 0;
    std::size_t zero_kernel_missing_pairs = 0;
    std::size_t recurrent_zero_kernel_missing_pairs = 0;
    bool cocycle_edges_exact = false;
    bool recurrent_zero_kernel_complete = false;
    bool proved = false;
    std::string obstruction;
};

template <std::size_t d>
ReturnPhaseDigitCocycleCertificate derive_return_phase_digit_cocycle(
        const std::array<std::vector<long long>, d>& images,
        const ravel::ReturnPhaseSystem& phases,
        const PrefixAutomaton<d>& automaton,
        std::size_t word_cap = 16,
        std::size_t state_cap = 1'000'000) {
    ReturnPhaseDigitCocycleCertificate out;
    out.phase_states = phases.states.size();
    out.pair_states = out.phase_states * out.phase_states;
    out.word_cap = word_cap;
    if (word_cap == 0 || state_cap == 0 || out.phase_states == 0) {
        out.obstruction = "return-phase cocycle requires positive caps";
        return out;
    }
    struct Edge { std::size_t target; mathlib::QElem label; };
    std::vector<std::vector<Edge>> edges(out.pair_states);
    auto pair = [=](std::size_t left, std::size_t right) {
        return left * out.phase_states + right;
    };
    auto prefix_digit = [&](const std::vector<long long>& prefix) {
        auto value = automaton.ring.zero();
        for (const long long letter : prefix) {
            if (letter < 0 || static_cast<std::size_t>(letter) >= d)
                return automaton.ring.zero();
            value = automaton.ring.add(
                value, automaton.left_eigenvector[static_cast<std::size_t>(letter)]);
        }
        return value;
    };
    for (std::size_t left_parent = 0; left_parent < out.phase_states;
         ++left_parent) {
        const auto left_letter = ravel::return_phase_letter(phases, left_parent);
        if (left_letter < 0 || static_cast<std::size_t>(left_letter) >= d)
            continue;
        for (std::size_t right_parent = 0; right_parent < out.phase_states;
             ++right_parent) {
            const auto right_letter = ravel::return_phase_letter(phases, right_parent);
            if (right_letter < 0 || static_cast<std::size_t>(right_letter) >= d)
                continue;
            const auto& left_image = images[static_cast<std::size_t>(left_letter)];
            const auto& right_image = images[static_cast<std::size_t>(right_letter)];
            for (std::size_t lp = 0; lp < left_image.size(); ++lp)
                for (std::size_t rp = 0; rp < right_image.size(); ++rp) {
                    const auto left_child = phases.phase_images[left_parent][lp];
                    const auto right_child = phases.phase_images[right_parent][rp];
                    const std::vector<long long> left_prefix(
                        left_image.begin(), left_image.begin() +
                        static_cast<std::ptrdiff_t>(lp));
                    const std::vector<long long> right_prefix(
                        right_image.begin(), right_image.begin() +
                        static_cast<std::ptrdiff_t>(rp));
                    const auto label = automaton.ring.sub(
                        prefix_digit(left_prefix), prefix_digit(right_prefix));
                    edges[pair(left_child, right_child)].push_back({
                        pair(left_parent, right_parent), label});
                    ++out.edge_count;
                }
        }
    }
    out.cocycle_edges_exact = true;
    const auto zero = automaton.ring.zero();
    const std::string zero_key = qelem_key(zero);
    std::vector<std::vector<bool>> reach(
        out.pair_states, std::vector<bool>(out.pair_states, false));
    for (std::size_t source = 0; source < out.pair_states; ++source)
        for (const auto& edge : edges[source]) reach[source][edge.target] = true;
    for (std::size_t k = 0; k < out.pair_states; ++k)
        for (std::size_t i = 0; i < out.pair_states; ++i)
            if (reach[i][k])
                for (std::size_t j = 0; j < out.pair_states; ++j)
                    reach[i][j] = reach[i][j] || reach[k][j];
    for (std::size_t source = 0; source < out.pair_states; ++source) {
        std::set<std::pair<std::size_t, std::string>> seen;
        std::vector<std::pair<std::size_t, mathlib::QElem>> frontier{{source, zero}};
        seen.insert({source, zero_key});
        for (std::size_t depth = 0; depth < word_cap && !frontier.empty(); ++depth) {
            std::vector<std::pair<std::size_t, mathlib::QElem>> next;
            for (const auto& state : frontier) {
                for (const auto& edge : edges[state.first]) {
                    const auto sum = automaton.ring.add(state.second, edge.label);
                    const auto key = std::make_pair(edge.target, qelem_key(sum));
                    if (seen.insert(key).second) {
                        if (++out.zero_kernel_missing_pairs > state_cap) {
                            out.obstruction = "return-phase cocycle state cap reached";
                            return out;
                        }
                        next.push_back({edge.target, sum});
                    }
                }
            }
            frontier.swap(next);
        }
        std::set<std::size_t> zero_targets;
        for (const auto& state : seen)
            if (state.second == zero_key) zero_targets.insert(state.first);
        out.zero_kernel_pairs += zero_targets.size();
        if (zero_targets.size() < out.pair_states)
            out.zero_kernel_missing_pairs += out.pair_states - zero_targets.size();
        for (std::size_t target = 0; target < out.pair_states; ++target)
            if (reach[source][target] && reach[target][source] &&
                !zero_targets.count(target))
                ++out.recurrent_zero_kernel_missing_pairs;
    }
    out.recurrent_zero_kernel_complete =
        out.recurrent_zero_kernel_missing_pairs == 0;
    out.proved = out.cocycle_edges_exact && out.recurrent_zero_kernel_complete;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "return-collar Q(beta) zero-kernel is incomplete";
    return out;
}

}  // namespace adelic
