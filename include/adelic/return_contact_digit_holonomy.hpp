// Exact Q(beta) holonomy of the full return/contact lift.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "ravel/return_contact_lift.hpp"

namespace adelic {

struct ReturnContactDigitHolonomyScc {
    std::size_t nodes = 0;
    std::size_t residual_edges = 0;
    std::size_t zero_contact_nodes = 0;
    std::size_t nonzero_contact_nodes = 0;
    bool coboundary = true;
};

struct ReturnContactDigitHolonomyCertificate {
    std::size_t states = 0;
    std::size_t edges = 0;
    std::size_t cyclic_sccs = 0;
    std::size_t nontrivial_holonomy_sccs = 0;
    std::size_t zero_seed_count = 0;
    std::size_t zero_seed_reachable_states = 0;
    std::size_t zero_seed_reachable_nontrivial_nodes = 0;
    bool zero_seed_reaches_nontrivial_holonomy = false;
    std::vector<ReturnContactDigitHolonomyScc> sccs;
    bool exact = false;
};

template <std::size_t d>
ReturnContactDigitHolonomyCertificate derive_return_contact_digit_holonomy(
        const std::array<std::vector<long long>, d>& images,
        const ravel::ReturnContactLift<d>& lift,
        const PrefixAutomaton<d>& automaton,
        const std::vector<ravel::ReturnContactState<d>>& seeds = {}) {
    struct Edge { std::size_t to; mathlib::QElem label; };
    const std::size_t n = lift.states.size();
    std::vector<std::vector<Edge>> graph(n);
    std::map<ravel::ReturnContactState<d>, std::size_t> state_index;
    for (std::size_t i = 0; i < n; ++i) state_index.emplace(lift.states[i], i);
    auto prefix_digit = [&](long long letter, std::size_t length) {
        auto value = automaton.ring.zero();
        if (letter < 0 || static_cast<std::size_t>(letter) >= d) return value;
        const auto& image = images[static_cast<std::size_t>(letter)];
        if (length > image.size()) return value;
        for (std::size_t i = 0; i < length; ++i)
            value = automaton.ring.add(
                value, automaton.left_eigenvector[static_cast<std::size_t>(image[i])]);
        return value;
    };
    for (const auto& edge : lift.edges) {
        const auto& destination = lift.states.at(edge.destination).contact;
        const auto left = prefix_digit(destination.i, edge.left_position);
        const auto right = prefix_digit(destination.j, edge.right_position);
        graph.at(edge.source).push_back({edge.destination,
                                         automaton.ring.sub(right, left)});
    }

    ReturnContactDigitHolonomyCertificate out;
    out.states = n;
    out.edges = lift.edges.size();
    std::vector<int> index(n, -1), low(n, 0), stack;
    std::vector<int> scc_of(n, -1);
    std::vector<bool> nontrivial_scc;
    std::vector<bool> active(n, false);
    int next = 0;
    std::function<void(std::size_t)> visit = [&](std::size_t v) {
        index[v] = low[v] = next++;
        stack.push_back(static_cast<int>(v)); active[v] = true;
        for (const auto& edge : graph[v]) {
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
            for (const auto& edge : graph[component.front()])
                if (edge.to == component.front()) cyclic = true;
        if (!cyclic) return;
        const int component_id = static_cast<int>(nontrivial_scc.size());
        ++out.cyclic_sccs;
        nontrivial_scc.push_back(false);
        std::vector<bool> in(n, false), assigned(n, false);
        for (const auto u : component) in[u] = true;
        std::vector<mathlib::QElem> potential(n, automaton.ring.zero());
        std::vector<std::size_t> todo{component.front()};
        assigned[component.front()] = true;
        std::size_t residuals = 0;
        const auto zero = automaton.ring.zero();
        while (!todo.empty()) {
            const auto u = todo.back(); todo.pop_back();
            for (const auto& edge : graph[u]) {
                if (!in[edge.to]) continue;
                if (!assigned[edge.to]) {
                    potential[edge.to] = automaton.ring.add(
                        potential[u], edge.label);
                    assigned[edge.to] = true;
                    todo.push_back(edge.to);
                    continue;
                }
                const auto residual = automaton.ring.sub(
                    automaton.ring.add(potential[u], edge.label),
                    potential[edge.to]);
                if (qelem_key(residual) != qelem_key(zero)) ++residuals;
            }
        }
        std::size_t zero_contact = 0;
        for (const auto u : component) {
            bool nonzero = false;
            for (const auto coordinate : lift.states[u].contact.x)
                if (coordinate != 0) nonzero = true;
            if (!nonzero) ++zero_contact;
        }
        const bool coboundary = residuals == 0;
        nontrivial_scc.back() = !coboundary;
        for (const auto u : component) scc_of[u] = component_id;
        out.sccs.push_back({component.size(), residuals, zero_contact,
                            component.size() - zero_contact, coboundary});
        if (!coboundary) ++out.nontrivial_holonomy_sccs;
    };
    for (std::size_t v = 0; v < n; ++v) if (index[v] < 0) visit(v);
    std::vector<bool> reached(n, false);
    std::vector<std::size_t> queue;
    for (const auto& seed : seeds) {
        bool zero = true;
        for (const auto coordinate : seed.contact.x)
            if (coordinate != 0) zero = false;
        if (!zero) continue;
        const auto it = state_index.find(seed);
        if (it == state_index.end()) continue;
        ++out.zero_seed_count;
        if (!reached[it->second]) {
            reached[it->second] = true;
            queue.push_back(it->second);
        }
    }
    for (std::size_t qi = 0; qi < queue.size(); ++qi) {
        const auto u = queue[qi];
        ++out.zero_seed_reachable_states;
        const int sid = scc_of[u];
        if (sid >= 0 && nontrivial_scc[static_cast<std::size_t>(sid)]) {
            out.zero_seed_reaches_nontrivial_holonomy = true;
            ++out.zero_seed_reachable_nontrivial_nodes;
        }
        for (const auto& edge : graph[u]) {
            if (!reached[edge.to]) {
                reached[edge.to] = true;
                queue.push_back(edge.to);
            }
        }
    }
    out.exact = true;
    return out;
}

}  // namespace adelic
