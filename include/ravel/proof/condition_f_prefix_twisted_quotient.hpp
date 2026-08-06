#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/condition_f_prefix_two_matrix.hpp"
#include "ravel/proof/condition_f_twisted_component_quotient.hpp"

namespace ravel::proof {

struct ConditionFTypedConcreteEdge {
    std::size_t source = 0;
    std::size_t target = 0;
    std::vector<long long> left_prefix;
    std::vector<long long> right_prefix;
};

struct ConditionFPrefixTwistedQuotientProof {
    std::size_t concrete_states = 0;
    std::size_t concrete_edges = 0;
    std::size_t base_roles = 0;
    std::size_t fibre_size = 0;
    std::size_t typed_channels = 0;
    std::vector<ConditionFPrefixChannel> channels;
    ConditionFPrefixTwoMatrixProof two_matrix;
    bool complete_sheet_table = false;
    bool typed_relations_regular = false;
    bool channels_preserve_prefix_type = false;
    bool concrete_lift_reconstructed = false;
    bool proved = false;
    std::string obstruction;
};

/** Derive the twisted quotient without forgetting the q/r channel type.
 *
 * Edge relations are grouped by (source role,target role,|delta|,sign delta)
 * before König matching decomposition.  Thus a perfect matching can never mix
 * balanced and residual channels, nor the two signed residual voltages.
 */
inline ConditionFPrefixTwistedQuotientProof
 derive_condition_f_prefix_twisted_quotient(
    const std::vector<ConditionFTypedConcreteEdge>& concrete_edges,
    const std::vector<std::size_t>& role,
    const std::vector<std::size_t>& fibre,
    std::size_t role_count,
    std::size_t fibre_size,
    std::size_t replay_word_depth = 10) {
    ConditionFPrefixTwistedQuotientProof out;
    out.concrete_states = role.size();
    out.concrete_edges = concrete_edges.size();
    out.base_roles = role_count;
    out.fibre_size = fibre_size;
    if (role.size() != fibre.size() || role_count == 0 || fibre_size == 0) {
        out.obstruction = "typed twisted quotient: malformed labels";
        return out;
    }

    std::vector<std::vector<std::size_t>> state(role_count,
        std::vector<std::size_t>(fibre_size, role.size()));
    out.complete_sheet_table = true;
    for (std::size_t v = 0; v < role.size(); ++v) {
        if (role[v] >= role_count || fibre[v] >= fibre_size ||
            state[role[v]][fibre[v]] != role.size()) {
            out.complete_sheet_table = false;
            break;
        }
        state[role[v]][fibre[v]] = v;
    }
    if (out.complete_sheet_table)
        for (const auto& row : state)
            for (auto v : row) out.complete_sheet_table &= v != role.size();
    if (!out.complete_sheet_table || role.size() != role_count * fibre_size) {
        out.obstruction = "typed twisted quotient: incomplete role-by-sheet table";
        return out;
    }

    using Matrix = std::vector<std::vector<std::size_t>>;
    using Key = std::tuple<std::size_t,std::size_t,std::size_t,long long>;
    std::map<Key,Matrix> relations;
    for (const auto& e : concrete_edges) {
        if (e.source >= role.size() || e.target >= role.size() ||
            !is_nbonacci_parent_prefix(e.left_prefix) ||
            !is_nbonacci_parent_prefix(e.right_prefix)) {
            out.obstruction = "typed twisted quotient: invalid endpoint or parent prefix";
            return out;
        }
        const long long delta =
            static_cast<long long>(e.right_prefix.size()) -
            static_cast<long long>(e.left_prefix.size());
        if (std::llabs(delta) > 1) {
            out.obstruction = "typed twisted quotient: prefix defect outside {-1,0,+1}";
            return out;
        }
        const std::size_t generator = static_cast<std::size_t>(std::llabs(delta));
        const Key key{role[e.source], role[e.target], generator, delta};
        auto [it, inserted] = relations.emplace(
            key, Matrix(fibre_size, std::vector<std::size_t>(fibre_size,0)));
        ++it->second[fibre[e.source]][fibre[e.target]];
    }

    out.typed_relations_regular = true;
    for (auto& [key,matrix] : relations) {
        std::size_t degree = 0;
        if (!condition_f_twisted_detail::regular_bipartite(matrix, degree)) {
            out.typed_relations_regular = false;
            out.obstruction = "typed twisted quotient: a typed sheet relation is not regular";
            return out;
        }
        const auto [source_role,target_role,generator,delta] = key;
        for (std::size_t k = 0; k < degree; ++k) {
            std::vector<std::size_t> permutation;
            if (!condition_f_twisted_detail::perfect_matching(matrix, permutation)) {
                out.obstruction = "typed twisted quotient: matching extraction failed";
                return out;
            }
            for (std::size_t s = 0; s < fibre_size; ++s) {
                const auto t = permutation[s];
                if (t >= fibre_size || matrix[s][t] == 0) {
                    out.obstruction = "typed twisted quotient: invalid typed matching";
                    return out;
                }
                --matrix[s][t];
            }
            std::vector<long long> left, right;
            if (delta < 0) left = {0};
            if (delta > 0) right = {0};
            if (delta == 0 && generator != 0) {
                out.obstruction = "typed twisted quotient: inconsistent generator/defect";
                return out;
            }
            out.channels.push_back({source_role,target_role,left,right,
                                    std::move(permutation)});
        }
        if (condition_f_twisted_detail::nonzero(matrix)) {
            out.obstruction = "typed twisted quotient: channels did not exhaust relation";
            return out;
        }
    }
    out.typed_channels = out.channels.size();
    out.channels_preserve_prefix_type = true;
    out.two_matrix = derive_condition_f_prefix_two_matrix(
        role_count, fibre_size, out.channels, replay_word_depth);

    // Reconstruct the typed concrete edge multiset from the channel lifts.
    std::map<std::tuple<std::size_t,std::size_t,long long>,std::size_t> expected;
    std::map<std::tuple<std::size_t,std::size_t,long long>,std::size_t> actual;
    for (const auto& e : concrete_edges) {
        const long long delta =
            static_cast<long long>(e.right_prefix.size()) -
            static_cast<long long>(e.left_prefix.size());
        ++expected[{e.source,e.target,delta}];
    }
    for (const auto& ch : out.channels) {
        const long long delta =
            static_cast<long long>(ch.right_prefix.size()) -
            static_cast<long long>(ch.left_prefix.size());
        for (std::size_t s = 0; s < fibre_size; ++s) {
            const auto u = state[ch.source][s];
            const auto v = state[ch.target][ch.permutation[s]];
            ++actual[{u,v,delta}];
        }
    }
    out.concrete_lift_reconstructed = expected == actual;
    out.proved = out.complete_sheet_table && out.typed_relations_regular &&
                 out.channels_preserve_prefix_type &&
                 out.concrete_lift_reconstructed && out.two_matrix.proved;
    if (!out.proved && out.obstruction.empty()) out.obstruction = out.two_matrix.obstruction;
    return out;
}

} // namespace ravel::proof
