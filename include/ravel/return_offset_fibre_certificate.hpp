// Finite quotient of a return/contact lift after forgetting bare contact
// coordinates.  The quotient retains return-word identities and phase
// offsets, exposing the finite fibre dynamics independently of the large
// geometric boundary.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "ravel/return_contact_lift.hpp"

namespace ravel {

struct ReturnOffsetFibreCertificate {
    std::size_t states = 0;
    std::size_t edges = 0;
    std::size_t cyclic_sccs = 0;
    std::size_t largest_cyclic_scc = 0;
    std::size_t minimum_class_size = 0;
    std::size_t maximum_class_size = 0;
    bool uniform_class_sizes = false;
    bool finite = false;
};

template <std::size_t d>
ReturnOffsetFibreCertificate derive_return_offset_fibre_certificate(
        const ReturnContactLift<d>& lift, const ReturnPhaseSystem& phases) {
    using Key = std::tuple<std::size_t, std::size_t, std::size_t,
                           std::size_t, std::size_t>;
    std::map<Key, std::size_t> ids;
    for (const auto& state : lift.states) {
        const auto& left = phases.states.at(state.left_phase);
        const auto& right = phases.states.at(state.right_phase);
        ids.emplace(Key{left.return_word, left.offset,
                        right.return_word, right.offset,
                        static_cast<std::size_t>(state.contact.i * 8
                                                 + state.contact.j)},
                    ids.size());
    }
    std::set<std::pair<std::size_t, std::size_t>> edge_set;
    for (const auto& edge : lift.edges) {
        const auto& source = lift.states.at(edge.source);
        const auto& target = lift.states.at(edge.destination);
        const auto& sl = phases.states.at(source.left_phase);
        const auto& sr = phases.states.at(source.right_phase);
        const auto& tl = phases.states.at(target.left_phase);
        const auto& tr = phases.states.at(target.right_phase);
        const auto source_key = Key{sl.return_word, sl.offset,
                                    sr.return_word, sr.offset,
                                    static_cast<std::size_t>(source.contact.i * 8
                                                             + source.contact.j)};
        const auto target_key = Key{tl.return_word, tl.offset,
                                    tr.return_word, tr.offset,
                                    static_cast<std::size_t>(target.contact.i * 8
                                                             + target.contact.j)};
        edge_set.insert({ids.at(source_key), ids.at(target_key)});
    }
    const std::size_t n = ids.size();
    ReturnOffsetFibreCertificate result;
    std::vector<std::size_t> class_sizes(n, 0);
    for (const auto& state : lift.states) {
        const auto& left = phases.states.at(state.left_phase);
        const auto& right = phases.states.at(state.right_phase);
        const auto key = Key{left.return_word, left.offset,
                             right.return_word, right.offset,
                             static_cast<std::size_t>(state.contact.i * 8 +
                                                      state.contact.j)};
        ++class_sizes.at(ids.at(key));
    }
    if (!class_sizes.empty()) {
        result.minimum_class_size = *std::min_element(
            class_sizes.begin(), class_sizes.end());
        result.maximum_class_size = *std::max_element(
            class_sizes.begin(), class_sizes.end());
        result.uniform_class_sizes = result.minimum_class_size ==
                                     result.maximum_class_size;
    }
    std::vector<std::vector<std::size_t>> graph(n);
    for (const auto& [source, target] : edge_set) graph[source].push_back(target);
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<bool> active(n, false);
    int next = 0;
    result.states = n;
    result.edges = edge_set.size();
    std::function<void(std::size_t)> visit = [&](std::size_t v) {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v)); active[v] = true;
        for (const auto w : graph[v]) {
            if (index[w] < 0) { visit(w); low[v] = std::min(low[v], low[w]); }
            else if (active[w]) low[v] = std::min(low[v], index[w]);
        }
        if (low[v] != index[v]) return;
        std::vector<std::size_t> component;
        while (true) {
            const auto w = static_cast<std::size_t>(stack.back());
            stack.pop_back(); active[w] = false; component.push_back(w);
            if (w == v) break;
        }
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty())
            for (const auto w : graph[component.front()])
                if (w == component.front()) cyclic = true;
        if (cyclic) {
            ++result.cyclic_sccs;
            result.largest_cyclic_scc =
                std::max(result.largest_cyclic_scc, component.size());
        }
    };
    for (std::size_t v = 0; v < n; ++v) if (index[v] < 0) visit(v);
    result.finite = true;
    return result;
}

}  // namespace ravel
