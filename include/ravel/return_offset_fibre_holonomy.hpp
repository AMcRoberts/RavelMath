// Exact edge-defect holonomy on the finite return/offset quotient.
//
// This is a diagnostic for the non-AR transport extension: quotienting a
// return/contact lift keeps a finite base, while each edge carries the
// left/right occurrence-position defect.  SCC-local cycle residues measure
// whether that finite base supports a genuine integer transport direction.
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <map>
#include <numeric>
#include <tuple>
#include <vector>

#include "ravel/return_offset_fibre_certificate.hpp"

namespace ravel {

struct ReturnOffsetFibreHolonomyScc {
    std::size_t nodes = 0;
    long long gcd_cycle_residue = 0;
    long long max_abs_cycle_residue = 0;
    bool coboundary = true;
};

struct ReturnOffsetFibreHolonomyCertificate {
    std::vector<ReturnOffsetFibreHolonomyScc> sccs;
    std::size_t recurrent_sccs = 0;
    std::size_t gcd_one_sccs = 0;
    bool finite = false;
};

template <std::size_t d>
ReturnOffsetFibreHolonomyCertificate derive_return_offset_fibre_holonomy(
        const ReturnContactLift<d>& lift, const ReturnPhaseSystem& phases) {
    using Key = std::tuple<std::size_t, std::size_t, std::size_t,
                           std::size_t, long long, long long>;
    std::map<Key, std::size_t> ids;
    for (const auto& state : lift.states) {
        const auto& left = phases.states.at(state.left_phase);
        const auto& right = phases.states.at(state.right_phase);
        ids.emplace(Key{left.return_word, left.offset,
                        right.return_word, right.offset,
                        state.contact.i, state.contact.j}, ids.size());
    }
    struct Edge { std::size_t to; long long weight; };
    std::vector<std::vector<Edge>> graph(ids.size());
    for (const auto& edge : lift.edges) {
        const auto& source = lift.states.at(edge.source);
        const auto& target = lift.states.at(edge.destination);
        const auto& sl = phases.states.at(source.left_phase);
        const auto& sr = phases.states.at(source.right_phase);
        const auto& tl = phases.states.at(target.left_phase);
        const auto& tr = phases.states.at(target.right_phase);
        const auto source_key = Key{sl.return_word, sl.offset,
                                    sr.return_word, sr.offset,
                                    source.contact.i, source.contact.j};
        const auto target_key = Key{tl.return_word, tl.offset,
                                    tr.return_word, tr.offset,
                                    target.contact.i, target.contact.j};
        graph.at(ids.at(source_key)).push_back(
            {ids.at(target_key), static_cast<long long>(edge.left_position) -
                                 static_cast<long long>(edge.right_position)});
    }

    const std::size_t n = graph.size();
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<bool> active(n, false);
    int next = 0;
    ReturnOffsetFibreHolonomyCertificate out;
    std::function<void(std::size_t)> visit = [&](std::size_t v) {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v)); active[v] = true;
        for (const auto edge : graph[v]) {
            if (index[edge.to] < 0) {
                visit(edge.to); low[v] = std::min(low[v], low[edge.to]);
            } else if (active[edge.to]) low[v] = std::min(low[v], index[edge.to]);
        }
        if (low[v] != index[v]) return;
        std::vector<std::size_t> component;
        while (true) {
            const auto u = static_cast<std::size_t>(stack.back());
            stack.pop_back(); active[u] = false; component.push_back(u);
            if (u == v) break;
        }
        bool cyclic = component.size() > 1;
        if (!cyclic && !component.empty())
            for (const auto edge : graph[component.front()])
                if (edge.to == component.front()) cyclic = true;
        if (!cyclic) return;

        std::vector<bool> in(n, false);
        for (const auto u : component) in[u] = true;
        std::vector<long long> potential(n, 0);
        std::vector<bool> assigned(n, false);
        std::vector<std::size_t> todo{component.front()};
        assigned[component.front()] = true;
        long long gcd = 0, max_abs = 0;
        while (!todo.empty()) {
            const auto u = todo.back(); todo.pop_back();
            for (const auto edge : graph[u]) {
                if (!in[edge.to]) continue;
                if (!assigned[edge.to]) {
                    potential[edge.to] = potential[u] + edge.weight;
                    assigned[edge.to] = true; todo.push_back(edge.to);
                    continue;
                }
                const long long residual = potential[u] + edge.weight -
                                            potential[edge.to];
                gcd = std::gcd(gcd, std::llabs(residual));
                max_abs = std::max(max_abs, std::llabs(residual));
            }
        }
        out.sccs.push_back({component.size(), gcd, max_abs, gcd == 0});
        ++out.recurrent_sccs;
        if (gcd == 1) ++out.gcd_one_sccs;
    };
    for (std::size_t v = 0; v < n; ++v) if (index[v] < 0) visit(v);
    out.finite = true;
    return out;
}

}  // namespace ravel
