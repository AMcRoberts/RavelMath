#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/right_resolving_language_inclusion.hpp"

namespace ravel::proof {

struct RightResolvingGlobalLanguageInclusion {
    std::size_t source_vertices = 0;
    std::size_t target_vertices = 0;
    std::size_t reachable_follower_states = 0;
    std::size_t maximum_target_subset = 0;
    bool source_right_resolving = false;
    bool target_right_resolving = false;
    bool every_source_word_occurs_in_target = false;
    bool spectral_radius_nonstrict = false;
    std::string obstruction;
};

/** Decide inclusion of the finite-path language of a right-resolving source in
 * the union of the follower languages of all target states.
 *
 * Unlike a graph simulation, the target start state may depend on the whole
 * source word.  A state of the subset construction is (s,S), where s is the
 * current source state and S is the set of all target endpoints reachable by
 * the same label word from any target start state.  Inclusion holds exactly
 * when no reachable source edge sends S to the empty set.
 *
 * If both presentations are right resolving, a fixed source start has at most
 * one path with a given word.  Hence source path counts are bounded by
 * |source| times target path counts, proving rho(source) <= rho(target).
 */
template<class Label>
RightResolvingGlobalLanguageInclusion
 derive_right_resolving_global_language_inclusion(
    const std::vector<std::vector<LabeledEdge<Label>>>& source,
    const std::vector<std::vector<LabeledEdge<Label>>>& target,
    std::size_t state_cap = 1000000) {
    RightResolvingGlobalLanguageInclusion out;
    out.source_vertices = source.size();
    out.target_vertices = target.size();
    if (source.empty() || target.empty()) {
        out.obstruction = "global language inclusion: empty graph";
        return out;
    }
    auto is_right_resolving = [](const auto& graph) {
        for (const auto& row : graph) {
            std::map<Label, std::size_t> seen;
            for (const auto& edge : row) {
                const auto [it, inserted] = seen.emplace(edge.label, edge.destination);
                if (!inserted && it->second != edge.destination) return false;
            }
        }
        return true;
    };
    out.source_right_resolving = is_right_resolving(source);
    out.target_right_resolving = is_right_resolving(target);
    if (!out.source_right_resolving || !out.target_right_resolving) {
        out.obstruction = "global language inclusion: graph is not right resolving";
        return out;
    }

    std::vector<std::map<Label, std::size_t>> target_step(target.size());
    for (std::size_t u = 0; u < target.size(); ++u)
        for (const auto& edge : target[u]) target_step[u][edge.label] = edge.destination;

    using State = std::pair<std::size_t, std::vector<std::size_t>>;
    std::queue<State> pending;
    std::set<State> seen;
    std::vector<std::size_t> all_target(target.size());
    for (std::size_t t = 0; t < target.size(); ++t) all_target[t] = t;
    for (std::size_t s = 0; s < source.size(); ++s) {
        State initial{s, all_target};
        seen.insert(initial);
        pending.push(std::move(initial));
    }

    while (!pending.empty()) {
        auto [s, subset] = std::move(pending.front());
        pending.pop();
        out.maximum_target_subset = std::max(out.maximum_target_subset, subset.size());
        for (const auto& source_edge : source[s]) {
            std::vector<std::size_t> next;
            next.reserve(subset.size());
            for (const auto t : subset) {
                const auto found = target_step[t].find(source_edge.label);
                if (found != target_step[t].end()) next.push_back(found->second);
            }
            std::sort(next.begin(), next.end());
            next.erase(std::unique(next.begin(), next.end()), next.end());
            if (next.empty()) {
                out.obstruction = "global language inclusion: a source word is absent from target";
                out.reachable_follower_states = seen.size();
                return out;
            }
            State candidate{source_edge.destination, std::move(next)};
            if (seen.insert(candidate).second) {
                if (seen.size() > state_cap) {
                    out.obstruction = "global language inclusion: follower-set cap exceeded";
                    out.reachable_follower_states = seen.size();
                    return out;
                }
                pending.push(std::move(candidate));
            }
        }
    }
    out.reachable_follower_states = seen.size();
    out.every_source_word_occurs_in_target = true;
    out.spectral_radius_nonstrict = true;
    return out;
}

} // namespace ravel::proof
