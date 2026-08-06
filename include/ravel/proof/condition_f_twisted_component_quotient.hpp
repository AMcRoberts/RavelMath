#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/permutation_skew_product.hpp"

namespace ravel::proof {

// Exact quotient of a concrete graph whose states are a complete finite sheet
// over base roles.  Distinct component channels between the same pair of base
// roles are retained as parallel base edges.  Each channel is derived as a
// perfect matching of the sheet bipartite multigraph, hence as a permutation.
struct TwistedComponentChannel {
    std::size_t source_role = 0;
    std::size_t target_role = 0;
    std::vector<std::size_t> permutation;
};

struct ConditionFTwistedComponentQuotientProof {
    std::size_t concrete_states = 0;
    std::size_t concrete_edges = 0;
    std::size_t base_roles = 0;
    std::size_t fibre_size = 0;
    std::size_t role_pairs = 0;
    std::size_t channel_count = 0;
    std::size_t multiply_glued_role_pairs = 0;
    std::size_t base_scc_count = 0;
    std::size_t lifted_scc_count = 0;
    bool holonomy_glues_sheets = false;
    bool complete_sheet_table = false;
    bool role_pair_regular = false;
    bool channels_are_permutations = false;
    bool concrete_lift_reconstructed = false;
    bool path_growth_preserved = false;
    bool genuinely_twisted = false;
    bool proved = false;
    std::vector<TwistedComponentChannel> channels;
    PermutationSkewProductCertificate skew_product;
    std::string obstruction;
};

namespace condition_f_twisted_detail {

// Find one perfect matching in a bipartite graph represented by remaining
// edge multiplicities.  Source sheets are matched to target sheets.
inline bool perfect_matching(
    const std::vector<std::vector<std::size_t>>& multiplicity,
    std::vector<std::size_t>& permutation) {
    const std::size_t f = multiplicity.size();
    permutation.assign(f, f);
    std::vector<std::size_t> target_owner(f, f);

    auto augment = [&](auto&& self, std::size_t s,
                       std::vector<bool>& seen) -> bool {
        for (std::size_t t = 0; t < f; ++t) {
            if (multiplicity[s][t] == 0 || seen[t]) continue;
            seen[t] = true;
            if (target_owner[t] == f || self(self, target_owner[t], seen)) {
                target_owner[t] = s;
                permutation[s] = t;
                return true;
            }
        }
        return false;
    };

    for (std::size_t s = 0; s < f; ++s) {
        std::vector<bool> seen(f, false);
        if (!augment(augment, s, seen)) return false;
    }
    return true;
}

inline bool nonzero(const std::vector<std::vector<std::size_t>>& m) {
    for (const auto& row : m)
        for (const auto x : row)
            if (x != 0) return true;
    return false;
}


inline std::size_t scc_count(const std::vector<std::vector<std::size_t>>& adjacency) {
    const std::size_t n = adjacency.size();
    std::vector<std::vector<std::size_t>> reverse(n);
    for (std::size_t u = 0; u < n; ++u)
        for (const auto v : adjacency[u])
            if (v < n) reverse[v].push_back(u);
    std::vector<bool> seen(n, false);
    std::vector<std::size_t> order;
    auto dfs1 = [&](auto&& self, std::size_t u) -> void {
        seen[u] = true;
        for (const auto v : adjacency[u]) if (!seen[v]) self(self, v);
        order.push_back(u);
    };
    for (std::size_t u = 0; u < n; ++u) if (!seen[u]) dfs1(dfs1, u);
    std::fill(seen.begin(), seen.end(), false);
    auto dfs2 = [&](auto&& self, std::size_t u) -> void {
        seen[u] = true;
        for (const auto v : reverse[u]) if (!seen[v]) self(self, v);
    };
    std::size_t count = 0;
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        if (seen[*it]) continue;
        ++count;
        dfs2(dfs2, *it);
    }
    return count;
}

inline bool regular_bipartite(
    const std::vector<std::vector<std::size_t>>& m,
    std::size_t& degree) {
    const std::size_t f = m.size();
    if (f == 0) return false;
    std::vector<std::size_t> out(f, 0), in(f, 0);
    for (std::size_t s = 0; s < f; ++s)
        for (std::size_t t = 0; t < f; ++t) {
            out[s] += m[s][t];
            in[t] += m[s][t];
        }
    degree = out[0];
    return degree > 0 &&
        std::all_of(out.begin(), out.end(), [&](auto x){ return x == degree; }) &&
        std::all_of(in.begin(), in.end(), [&](auto x){ return x == degree; });
}

} // namespace condition_f_twisted_detail

/** Derive a channelized twisted quotient from concrete component assembly.
 *
 * `role` and `fibre` label every concrete state.  There must be exactly one
 * concrete state for each `(role,fibre)` pair.  For each ordered role pair the
 * concrete edge multiset is a regular bipartite multigraph on the sheets.
 * König's line-colouring theorem is executed here by repeated perfect
 * matching extraction.  Every matching is retained as a distinct parallel
 * base edge carrying its sheet permutation.
 *
 * This is the important correction to a flat quotient: different sheet
 * transports between the same base roles are component channels, not an
 * ambiguity to be collapsed.
 */
inline ConditionFTwistedComponentQuotientProof
 derive_condition_f_twisted_component_quotient(
    const std::vector<std::pair<std::size_t,std::size_t>>& concrete_edges,
    const std::vector<std::size_t>& role,
    const std::vector<std::size_t>& fibre,
    std::size_t role_count,
    std::size_t fibre_size,
    std::size_t replay_depth = 16) {
    ConditionFTwistedComponentQuotientProof out;
    out.concrete_states = role.size();
    out.concrete_edges = concrete_edges.size();
    out.base_roles = role_count;
    out.fibre_size = fibre_size;
    if (role.size() != fibre.size() || role_count == 0 || fibre_size == 0) {
        out.obstruction = "twisted component quotient: malformed labels";
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
    if (out.complete_sheet_table) {
        for (const auto& row : state)
            for (const auto v : row)
                out.complete_sheet_table &= v != role.size();
    }
    if (!out.complete_sheet_table || role.size() != role_count * fibre_size) {
        out.obstruction = "twisted component quotient: concrete states are not a complete role-by-sheet table";
        return out;
    }

    using Matrix = std::vector<std::vector<std::size_t>>;
    std::map<std::pair<std::size_t,std::size_t>, Matrix> pair_edges;
    for (const auto& [u,v] : concrete_edges) {
        if (u >= role.size() || v >= role.size()) {
            out.obstruction = "twisted component quotient: bad concrete edge endpoint";
            return out;
        }
        auto key = std::pair{role[u], role[v]};
        auto [it, inserted] = pair_edges.emplace(
            key, Matrix(fibre_size, std::vector<std::size_t>(fibre_size, 0)));
        ++it->second[fibre[u]][fibre[v]];
    }
    out.role_pairs = pair_edges.size();
    out.role_pair_regular = true;
    std::vector<PermutationSkewEdge> skew_edges;

    for (auto& [key, matrix] : pair_edges) {
        std::size_t degree = 0;
        if (!condition_f_twisted_detail::regular_bipartite(matrix, degree)) {
            out.role_pair_regular = false;
            out.obstruction = "twisted component quotient: role-pair sheet relation is not regular";
            return out;
        }
        if (degree > 1) ++out.multiply_glued_role_pairs;
        for (std::size_t channel = 0; channel < degree; ++channel) {
            std::vector<std::size_t> permutation;
            if (!condition_f_twisted_detail::perfect_matching(matrix, permutation)) {
                out.obstruction = "twisted component quotient: regular sheet relation failed matching decomposition";
                return out;
            }
            for (std::size_t s = 0; s < fibre_size; ++s) {
                const auto t = permutation[s];
                if (t >= fibre_size || matrix[s][t] == 0) {
                    out.obstruction = "twisted component quotient: invalid extracted channel";
                    return out;
                }
                --matrix[s][t];
            }
            out.channels.push_back({key.first, key.second, permutation});
            skew_edges.push_back({key.first, key.second, permutation});
        }
        if (condition_f_twisted_detail::nonzero(matrix)) {
            out.obstruction = "twisted component quotient: matching channels did not exhaust concrete edges";
            return out;
        }
    }
    out.channel_count = out.channels.size();
    out.channels_are_permutations = std::all_of(
        out.channels.begin(), out.channels.end(),
        [](const auto& c){ return is_permutation(c.permutation); });
    if (!out.channels_are_permutations) {
        out.obstruction = "twisted component quotient: nonpermutation channel";
        return out;
    }

    // Reconstruct the concrete edge multiset from the extracted channels.
    std::multiset<std::pair<std::size_t,std::size_t>> expected, actual;
    for (const auto& e : concrete_edges) actual.insert(e);
    for (const auto& c : out.channels)
        for (std::size_t a = 0; a < fibre_size; ++a)
            expected.insert({state[c.source_role][a],
                             state[c.target_role][c.permutation[a]]});
    out.concrete_lift_reconstructed = expected == actual;
    if (!out.concrete_lift_reconstructed) {
        out.obstruction = "twisted component quotient: channel lift does not reconstruct concrete graph";
        return out;
    }

    out.skew_product = derive_permutation_skew_product(
        role_count, fibre_size, std::move(skew_edges), replay_depth);
    out.path_growth_preserved = out.skew_product.valid &&
        out.skew_product.spectral_radius_equal;
    out.base_scc_count = condition_f_twisted_detail::scc_count(
        out.skew_product.base_adjacency);
    out.lifted_scc_count = condition_f_twisted_detail::scc_count(
        out.skew_product.lifted_adjacency);
    out.holonomy_glues_sheets =
        out.lifted_scc_count < out.base_scc_count * fibre_size;

    // Genuine twist means at least one channel cannot be simultaneously
    // trivialized merely by forgetting the sheet action.  A non-identity
    // permutation is sufficient evidence; full holonomy is retained by the
    // channel list and may combine around cycles.
    out.genuinely_twisted = std::any_of(
        out.channels.begin(), out.channels.end(),
        [](const auto& c) {
            for (std::size_t i = 0; i < c.permutation.size(); ++i)
                if (c.permutation[i] != i) return true;
            return false;
        });
    out.proved = out.complete_sheet_table && out.role_pair_regular &&
        out.channels_are_permutations && out.concrete_lift_reconstructed &&
        out.path_growth_preserved;
    if (!out.proved && out.obstruction.empty())
        out.obstruction = "twisted component quotient did not close";
    return out;
}

} // namespace ravel::proof
