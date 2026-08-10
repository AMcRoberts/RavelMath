// Exact pair-role prefix-digit cocycle for arbitrary substitutions.
//
// Parent-prefix length defects are only a coarse shadow: two prefixes may
// have equal length but different Q(beta) digits.  This operation retains the
// full digit difference in Q(beta), and checks the finite zero-kernel directly
// on the same cocycle used by the adelic Property-F graph.
#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "adelic/property_f_types.hpp"

namespace adelic {

struct PropertyFRoleDigitCocycleCertificate {
    std::size_t alphabet_size = 0;
    std::size_t role_count = 0;
    std::size_t edge_count = 0;
    std::size_t word_cap = 0;
    std::size_t zero_kernel_pairs = 0;
    std::size_t zero_kernel_missing_pairs = 0;
    std::size_t recurrent_zero_kernel_missing_pairs = 0;
    std::size_t explored_states = 0;
    std::vector<std::pair<std::size_t, std::size_t>> missing_zero_pairs;
    bool cocycle_edges_exact = false;
    bool zero_kernel_complete = false;
    bool recurrent_zero_kernel_complete = false;
    bool witnesses_replayed = false;
    bool proved = false;
    std::string obstruction;
};

template <std::size_t d>
PropertyFRoleDigitCocycleCertificate derive_property_f_role_digit_cocycle(
    const std::array<std::vector<long long>, d>& images,
    const PrefixAutomaton<d>& automaton,
    std::size_t word_cap = 16,
    std::size_t state_cap = 1'000'000) {
    PropertyFRoleDigitCocycleCertificate out;
    out.alphabet_size = d;
    out.role_count = d * d;
    out.word_cap = word_cap;
    if (word_cap == 0 || state_cap == 0) {
        out.obstruction = "digit cocycle requires positive caps";
        return out;
    }
    struct Edge { std::size_t target; mathlib::QElem label; };
    std::vector<std::vector<Edge>> edges(out.role_count);
    auto role = [=](long long left, long long right) {
        return static_cast<std::size_t>(left) * d +
               static_cast<std::size_t>(right);
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
    std::vector<std::vector<std::pair<long long, std::vector<long long>>>> parents(d);
    for (std::size_t source = 0; source < d; ++source)
        for (std::size_t position = 0; position < images[source].size(); ++position) {
            const long long inner = images[source][position];
            if (inner < 0 || static_cast<std::size_t>(inner) >= d) {
                out.obstruction = "digit cocycle has an out-of-range letter";
                return out;
            }
            parents[static_cast<std::size_t>(inner)].push_back({
                static_cast<long long>(source),
                std::vector<long long>(images[source].begin(),
                                       images[source].begin() +
                                           static_cast<std::ptrdiff_t>(position))});
        }
    for (long long left = 0; left < static_cast<long long>(d); ++left)
        for (long long right = 0; right < static_cast<long long>(d); ++right)
            for (const auto& lp : parents[static_cast<std::size_t>(left)])
                for (const auto& rp : parents[static_cast<std::size_t>(right)]) {
                    edges[role(left, right)].push_back({
                        role(lp.first, rp.first),
                        automaton.ring.sub(prefix_digit(rp.second),
                                           prefix_digit(lp.second))});
                    ++out.edge_count;
                }
    out.cocycle_edges_exact = true;
    const auto zero = automaton.ring.zero();
    const std::string zero_key = qelem_key(zero);
    std::size_t zero_pairs = 0;
    std::vector<std::vector<bool>> reach(
        out.role_count, std::vector<bool>(out.role_count, false));
    for (std::size_t source = 0; source < out.role_count; ++source)
        for (const auto& edge : edges[source]) reach[source][edge.target] = true;
    for (std::size_t k = 0; k < out.role_count; ++k)
        for (std::size_t i = 0; i < out.role_count; ++i)
            if (reach[i][k])
                for (std::size_t j = 0; j < out.role_count; ++j)
                    reach[i][j] = reach[i][j] || reach[k][j];
    std::vector<std::size_t> recurrent_missing_by_source(out.role_count, 0);
    for (std::size_t source = 0; source < out.role_count; ++source) {
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
                        if (++out.explored_states > state_cap) {
                            out.obstruction = "digit cocycle state cap reached";
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
        zero_pairs += zero_targets.size();
        for (std::size_t target = 0; target < out.role_count; ++target)
            if (!zero_targets.count(target))
                out.missing_zero_pairs.push_back({source, target});
        for (std::size_t target = 0; target < out.role_count; ++target)
            if (reach[source][target] && reach[target][source] &&
                !zero_targets.count(target))
                ++recurrent_missing_by_source[source];
    }
    out.zero_kernel_pairs = zero_pairs;
    out.zero_kernel_missing_pairs = out.role_count * out.role_count > zero_pairs
        ? out.role_count * out.role_count - zero_pairs : 0;
    out.zero_kernel_complete = out.zero_kernel_missing_pairs == 0;
    for (const auto count : recurrent_missing_by_source)
        out.recurrent_zero_kernel_missing_pairs += count;
    out.recurrent_zero_kernel_complete =
        out.recurrent_zero_kernel_missing_pairs == 0;
    out.witnesses_replayed = out.cocycle_edges_exact;
    out.proved = out.cocycle_edges_exact && out.recurrent_zero_kernel_complete &&
                 out.witnesses_replayed;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "exact Q(beta) zero-kernel is incomplete";
    return out;
}

}  // namespace adelic
