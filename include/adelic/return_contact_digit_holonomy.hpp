// Exact Q(beta) holonomy of the full return/contact lift.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "ravel/return_contact_lift.hpp"

namespace adelic {

struct ReturnContactDigitHolonomyScc {
    std::size_t nodes = 0;
    std::size_t residual_edges = 0;
    bool coboundary = true;
};

struct ReturnContactDigitHolonomyCertificate {
    std::size_t states = 0;
    std::size_t edges = 0;
    std::size_t cyclic_sccs = 0;
    std::size_t nontrivial_holonomy_sccs = 0;
    std::vector<ReturnContactDigitHolonomyScc> sccs;
    bool exact = false;
};

template <std::size_t d>
ReturnContactDigitHolonomyCertificate derive_return_contact_digit_holonomy(
        const std::array<std::vector<long long>, d>& images,
        const ravel::ReturnContactLift<d>& lift,
        const PrefixAutomaton<d>& automaton) {
    struct Edge { std::size_t to; mathlib::QElem label; };
    const std::size_t n = lift.states.size();
    std::vector<std::vector<Edge>> graph(n);
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
        ++out.cyclic_sccs;
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
        const bool coboundary = residuals == 0;
        out.sccs.push_back({component.size(), residuals, coboundary});
        if (!coboundary) ++out.nontrivial_holonomy_sccs;
    };
    for (std::size_t v = 0; v < n; ++v) if (index[v] < 0) visit(v);
    out.exact = true;
    return out;
}

}  // namespace adelic
