// Exact affine Q(beta) holonomy of the full return/contact lift.
// Each edge acts by gamma' = beta^{-1}(gamma + delta); all SCC and frontier
// consistency checks below use that affine action, not plain additive sums.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "adelic/prefix_automaton.hpp"
#include "ravel/return_contact_lift.hpp"

namespace adelic {

struct ReturnContactDigitHolonomyScc {
    std::size_t nodes = 0;
    std::size_t residual_edges = 0;
    std::size_t left_residual_edges = 0;
    std::size_t right_residual_edges = 0;
    // Rank of residuals obtained with the root anchored at zero.  These are
    // frontier-relative diagnostics, not the affine cohomology rank.
    std::size_t residual_rank = 0;
    std::size_t left_residual_rank = 0;
    std::size_t right_residual_rank = 0;
    std::size_t zero_contact_nodes = 0;
    std::size_t nonzero_contact_nodes = 0;
    // True iff a free Q(beta) root potential solves every affine edge
    // equation in this SCC.  The one-sided flags are checked independently.
    bool coboundary = true;
    bool left_coboundary = true;
    bool right_coboundary = true;
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
    std::size_t left_frontier_ambiguous_states = 0;
    std::size_t right_frontier_ambiguous_states = 0;
    bool single_valued_frontier_projection = true;
    std::vector<ReturnContactDigitHolonomyScc> sccs;
    bool exact = false;
};

template <std::size_t d>
ReturnContactDigitHolonomyCertificate derive_return_contact_digit_holonomy(
        const std::array<std::vector<long long>, d>& images,
        const ravel::ReturnContactLift<d>& lift,
        const PrefixAutomaton<d>& automaton,
        const std::vector<ravel::ReturnContactState<d>>& seeds = {}) {
    struct Edge {
        std::size_t to;
        mathlib::QElem left_label;
        mathlib::QElem right_label;
    };
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
        graph.at(edge.source).push_back({edge.destination, left, right});
    }

    ReturnContactDigitHolonomyCertificate out;
    out.states = n;
    out.edges = lift.edges.size();
    const auto inverse = mathlib::invert_in_qbeta(
        automaton.ring.beta_k(1), automaton.ring);
    if (!inverse.invertible)
        throw std::runtime_error("return contact holonomy: beta is not invertible");
    const auto inverse_beta = inverse.inverse;
    auto affine_step = [&](const mathlib::QElem& value,
                           const mathlib::QElem& label) {
        return automaton.ring.mul(inverse_beta,
            automaton.ring.add(value, label));
    };
    auto exact_rank = [](const std::vector<mathlib::QElem>& values) {
        std::size_t dimension = 0;
        for (const auto& value : values) dimension = std::max(dimension, value.coeffs_.size());
        std::vector<std::vector<mathlib::Rat>> basis;
        for (const auto& value : values) {
            std::vector<mathlib::Rat> row(dimension, mathlib::Rat(0));
            for (std::size_t i = 0; i < value.coeffs_.size(); ++i)
                row[i] = value.coeffs_[i];
            for (const auto& pivot : basis) {
                std::size_t p = 0;
                while (p < dimension && mathlib::is_zero(pivot[p])) ++p;
                if (p == dimension || mathlib::is_zero(row[p])) continue;
                mathlib::Rat factor;
                mathlib::div(factor, row[p], pivot[p]);
                for (std::size_t i = p; i < dimension; ++i) {
                    mathlib::Rat product, reduced;
                    mathlib::mul(product, factor, pivot[i]);
                    mathlib::sub(reduced, row[i], product);
                    row[i] = reduced;
                }
            }
            bool nonzero = false;
            for (const auto& coefficient : row)
                if (!mathlib::is_zero(coefficient)) { nonzero = true; break; }
            if (!nonzero) continue;
            std::size_t pivot_index = 0;
            while (pivot_index < dimension && mathlib::is_zero(row[pivot_index])) ++pivot_index;
            if (pivot_index == dimension) continue;
            mathlib::Rat scale;
            mathlib::div(scale, mathlib::Rat(1), row[pivot_index]);
            for (auto& coefficient : row) {
                mathlib::Rat normalized;
                mathlib::mul(normalized, coefficient, scale);
                coefficient = normalized;
            }
            basis.push_back(std::move(row));
        }
        return basis.size();
    };
    auto affine_consistent = [&](const std::vector<std::size_t>& component,
                                 const std::vector<bool>& in,
                                 const std::vector<mathlib::QElem>& multiplier,
                                 const std::vector<mathlib::QElem>& constant,
                                 const auto& selector) {
        bool candidate_set = false;
        mathlib::QElem candidate = automaton.ring.zero();
        const auto zero = automaton.ring.zero();
        for (const auto u : component) {
            for (const auto& edge : graph[u]) {
                if (!in[edge.to]) continue;
                const auto label = selector(edge);
                const auto lhs = automaton.ring.sub(
                    multiplier[edge.to],
                    automaton.ring.mul(inverse_beta, multiplier[u]));
                const auto rhs = automaton.ring.sub(
                    automaton.ring.mul(inverse_beta,
                        automaton.ring.add(constant[u], label)),
                    constant[edge.to]);
                if (qelem_key(lhs) == qelem_key(zero)) {
                    if (qelem_key(rhs) != qelem_key(zero)) return false;
                    continue;
                }
                const auto inv_lhs = mathlib::invert_in_qbeta(lhs, automaton.ring);
                if (!inv_lhs.invertible) return false;
                const auto proposed = automaton.ring.mul(
                    inv_lhs.inverse,
                    automaton.ring.sub(zero, rhs));
                if (!candidate_set) {
                    candidate = proposed;
                    candidate_set = true;
                } else if (qelem_key(candidate) != qelem_key(proposed)) {
                    return false;
                }
            }
        }
        return true;
    };
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
        std::vector<mathlib::QElem> left_potential(n, automaton.ring.zero());
        std::vector<mathlib::QElem> right_potential(n, automaton.ring.zero());
        std::vector<mathlib::QElem> multiplier(n, automaton.ring.zero());
        std::vector<mathlib::QElem> left_multiplier(n, automaton.ring.zero());
        std::vector<mathlib::QElem> right_multiplier(n, automaton.ring.zero());
        std::vector<std::size_t> todo{component.front()};
        assigned[component.front()] = true;
        std::size_t residuals = 0;
        std::size_t left_residuals = 0;
        std::size_t right_residuals = 0;
        std::vector<mathlib::QElem> residual_values;
        std::vector<mathlib::QElem> left_residual_values;
        std::vector<mathlib::QElem> right_residual_values;
        const auto zero = automaton.ring.zero();
        const auto one = automaton.ring.one();
        multiplier[component.front()] = one;
        left_multiplier[component.front()] = one;
        right_multiplier[component.front()] = one;
        while (!todo.empty()) {
            const auto u = todo.back(); todo.pop_back();
            for (const auto& edge : graph[u]) {
                if (!in[edge.to]) continue;
                if (!assigned[edge.to]) {
                    potential[edge.to] = affine_step(
                        potential[u], automaton.ring.sub(
                            edge.right_label, edge.left_label));
                    left_potential[edge.to] = affine_step(
                        left_potential[u], edge.left_label);
                    right_potential[edge.to] = affine_step(
                        right_potential[u], edge.right_label);
                    multiplier[edge.to] = automaton.ring.mul(
                        inverse_beta, multiplier[u]);
                    left_multiplier[edge.to] = automaton.ring.mul(
                        inverse_beta, left_multiplier[u]);
                    right_multiplier[edge.to] = automaton.ring.mul(
                        inverse_beta, right_multiplier[u]);
                    assigned[edge.to] = true;
                    todo.push_back(edge.to);
                    continue;
                }
                const auto residual = automaton.ring.sub(
                    affine_step(potential[u], automaton.ring.sub(
                        edge.right_label, edge.left_label)), potential[edge.to]);
                if (qelem_key(residual) != qelem_key(zero)) {
                    ++residuals;
                    residual_values.push_back(residual);
                }
                const auto left_residual = automaton.ring.sub(
                    affine_step(left_potential[u], edge.left_label),
                    left_potential[edge.to]);
                const auto right_residual = automaton.ring.sub(
                    affine_step(right_potential[u], edge.right_label),
                    right_potential[edge.to]);
                if (qelem_key(left_residual) != qelem_key(zero)) {
                    ++left_residuals;
                    left_residual_values.push_back(left_residual);
                }
                if (qelem_key(right_residual) != qelem_key(zero)) {
                    ++right_residuals;
                    right_residual_values.push_back(right_residual);
                }
            }
        }
        std::size_t zero_contact = 0;
        for (const auto u : component) {
            bool nonzero = false;
            for (const auto coordinate : lift.states[u].contact.x)
                if (coordinate != 0) nonzero = true;
            if (!nonzero) ++zero_contact;
        }
        const bool coboundary = affine_consistent(
            component, in, multiplier, potential,
            [&](const Edge& edge) {
                return automaton.ring.sub(edge.right_label, edge.left_label);
            });
        const bool left_coboundary = affine_consistent(
            component, in, left_multiplier, left_potential,
            [](const Edge& edge) { return edge.left_label; });
        const bool right_coboundary = affine_consistent(
            component, in, right_multiplier, right_potential,
            [](const Edge& edge) { return edge.right_label; });
        const auto residual_rank = exact_rank(residual_values);
        const auto left_residual_rank = exact_rank(left_residual_values);
        const auto right_residual_rank = exact_rank(right_residual_values);
        nontrivial_scc.back() = !coboundary;
        for (const auto u : component) scc_of[u] = component_id;
        out.sccs.push_back({component.size(), residuals, left_residuals,
                            right_residuals, residual_rank, left_residual_rank,
                            right_residual_rank, zero_contact,
                            component.size() - zero_contact, coboundary,
                            left_coboundary, right_coboundary});
        if (!coboundary) ++out.nontrivial_holonomy_sccs;
    };
    for (std::size_t v = 0; v < n; ++v) if (index[v] < 0) visit(v);
    std::vector<bool> reached(n, false);
    std::vector<bool> left_ambiguous(n, false), right_ambiguous(n, false);
    std::vector<mathlib::QElem> left_frontier_potential(
        n, automaton.ring.zero());
    std::vector<mathlib::QElem> right_frontier_potential(
        n, automaton.ring.zero());
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
            const auto left_next = affine_step(
                left_frontier_potential[u], edge.left_label);
            const auto right_next = affine_step(
                right_frontier_potential[u], edge.right_label);
            if (!reached[edge.to]) {
                reached[edge.to] = true;
                left_frontier_potential[edge.to] = left_next;
                right_frontier_potential[edge.to] = right_next;
                queue.push_back(edge.to);
            } else {
                if (qelem_key(left_frontier_potential[edge.to]) !=
                    qelem_key(left_next)) left_ambiguous[edge.to] = true;
                if (qelem_key(right_frontier_potential[edge.to]) !=
                    qelem_key(right_next)) right_ambiguous[edge.to] = true;
            }
        }
    }
    for (std::size_t i = 0; i < n; ++i) {
        if (left_ambiguous[i]) ++out.left_frontier_ambiguous_states;
        if (right_ambiguous[i]) ++out.right_frontier_ambiguous_states;
    }
    out.single_valued_frontier_projection =
        out.left_frontier_ambiguous_states == 0 &&
        out.right_frontier_ambiguous_states == 0;
    out.exact = true;
    return out;
}

}  // namespace adelic
