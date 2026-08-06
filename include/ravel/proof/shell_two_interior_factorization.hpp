#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ravel/proof/maximum_shell_exclusion.hpp"
#include "ravel/proof/parametric_maximum_shell_reduction.hpp"

namespace ravel::proof {

struct ShellTwoInteriorFactorizationCertificate {
    std::size_t dimension = 0;
    std::set<ShellState> interior_states;
    std::map<ShellState,std::set<ShellState>> interior_edges;
    std::map<ShellState,std::set<ShellState>> boundary_entries;
    std::map<ShellState,std::set<ShellState>> boundary_exits;
    ShellReturnGraph factored_first_return;
    ShellFirstReturnGraphCertificate literal_first_return;
    ConcreteShellRankCertificate rank;
    bool interior_exact = false;
    bool entry_exit_exact = false;
    bool factorization_replays = false;
    bool acyclic = false;
    bool valid = false;
    std::string obstruction;
};

inline ShellTwoInteriorFactorizationCertificate
 derive_shell_two_interior_factorization(std::size_t n) {
    ShellTwoInteriorFactorizationCertificate c;
    c.dimension = n;
    if (n < 2) {
        c.obstruction = "shell-two factorization requires n>=2";
        return c;
    }

    ShellState state(n,0);
    enumerate_box_states_rec(n,1,0,state,[&](const ShellState& x){
        c.interior_states.insert(x);
        c.interior_edges.try_emplace(x);
    });
    for (const auto& x : c.interior_states)
        for (const auto& y : bounded_carry_successors(x,2))
            if (c.interior_states.contains(y)) c.interior_edges[x].insert(y);
    c.interior_exact = c.interior_states.size() > 0;

    std::vector<ShellState> boundary;
    enumerate_box_states_rec(n,2,0,state,[&](const ShellState& x){
        if (shell_radius(x)==2) {
            boundary.push_back(x);
            c.factored_first_return.try_emplace(x);
        }
    });

    for (const auto& b : boundary) {
        for (const auto& y : bounded_carry_successors(b,2)) {
            if (c.interior_states.contains(y)) c.boundary_entries[b].insert(y);
            else if (shell_radius(y)==2) c.factored_first_return[b].insert(y);
        }
    }
    for (const auto& x : c.interior_states)
        for (const auto& y : bounded_carry_successors(x,2))
            if (shell_radius(y)==2) c.boundary_exits[x].insert(y);

    // Reachability inside the strict interior, followed by the first boundary
    // exit, is exactly the shell-two first-return relation.
    for (const auto& b : boundary) {
        std::deque<ShellState> q;
        std::set<ShellState> seen;
        for (const auto& x : c.boundary_entries[b]) {
            q.push_back(x); seen.insert(x);
        }
        while (!q.empty()) {
            const auto x=q.front(); q.pop_front();
            if (auto it=c.boundary_exits.find(x); it!=c.boundary_exits.end())
                c.factored_first_return[b].insert(it->second.begin(),it->second.end());
            if (auto it=c.interior_edges.find(x); it!=c.interior_edges.end())
                for (const auto& y : it->second)
                    if (seen.insert(y).second) q.push_back(y);
        }
    }
    c.entry_exit_exact = true;

    c.literal_first_return = derive_shell_first_return_graph(n,2);
    c.factorization_replays = c.literal_first_return.valid &&
                              c.factored_first_return == c.literal_first_return.graph;
    if (!c.factorization_replays) {
        c.obstruction = "interior factorization differs from literal first-return graph";
        return c;
    }
    c.rank = derive_concrete_shell_rank(c.factored_first_return);
    c.acyclic = c.rank.valid;
    c.valid = c.interior_exact && c.entry_exit_exact &&
              c.factorization_replays && c.acyclic;
    if (!c.valid) c.obstruction = "shell-two factored graph is not ranked";
    return c;
}

} // namespace ravel::proof
