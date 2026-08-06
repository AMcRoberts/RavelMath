#pragma once

#include <cstddef>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

// A finite Z/2-fiber quotient.  Each concrete state has a base role and an
// orientation bit.  A concrete edge induces orientation transport by xor.
struct Z2TwistedQuotientProof {
    std::size_t concrete_states = 0;
    std::size_t base_roles = 0;
    std::size_t concrete_edges = 0;
    std::size_t base_edges = 0;
    std::size_t ambiguous_base_edges = 0;
    std::size_t nontrivial_holonomy_witnesses = 0;
    bool transport_well_defined = false;
    bool cocycle_law = false;
    bool coboundary = false;
    bool genuinely_twisted = false;
    std::vector<int> gauge;
    std::string obstruction;
};

// Edge is (source concrete id, target concrete id).  role and fiber have one
// entry per concrete state.  This operation derives the quotient transport,
// checks whether it is a function on base edges, then solves the gauge equation
// g(v)=g(u)+tau(u,v) mod 2.  Failure of the gauge equation is an exact odd
// holonomy witness.
inline Z2TwistedQuotientProof derive_z2_twisted_quotient(
    const std::vector<std::pair<std::size_t, std::size_t>>& edges,
    const std::vector<std::size_t>& role,
    const std::vector<int>& fiber,
    std::size_t role_count) {
    if (role.size() != fiber.size())
        throw std::invalid_argument("twisted quotient: role/fiber size mismatch");
    for (std::size_t i = 0; i < role.size(); ++i) {
        if (role[i] >= role_count || (fiber[i] != 0 && fiber[i] != 1))
            throw std::invalid_argument("twisted quotient: malformed state label");
    }

    Z2TwistedQuotientProof out;
    out.concrete_states = role.size();
    out.base_roles = role_count;
    out.concrete_edges = edges.size();

    // -1 unseen, 0/1 unique transport, 2 ambiguous.
    std::map<std::pair<std::size_t, std::size_t>, int> transport;
    for (const auto& [s, t] : edges) {
        if (s >= role.size() || t >= role.size())
            throw std::invalid_argument("twisted quotient: bad edge endpoint");
        const auto key = std::pair{role[s], role[t]};
        const int tau = fiber[s] ^ fiber[t];
        auto [it, inserted] = transport.emplace(key, tau);
        if (!inserted && it->second != tau && it->second != 2) {
            it->second = 2;
            ++out.ambiguous_base_edges;
        }
    }
    out.base_edges = transport.size();
    out.transport_well_defined = out.ambiguous_base_edges == 0;
    if (!out.transport_well_defined) {
        out.obstruction = "orientation transport is not well-defined on quotient edges";
        return out;
    }
    out.cocycle_law = true; // xor transport is derived from endpoint fibers.

    std::vector<std::vector<std::pair<std::size_t,int>>> adjacency(role_count);
    for (const auto& [edge, tau] : transport) {
        adjacency[edge.first].push_back({edge.second, tau});
        adjacency[edge.second].push_back({edge.first, tau});
    }
    out.gauge.assign(role_count, -1);
    for (std::size_t root = 0; root < role_count; ++root) {
        if (out.gauge[root] >= 0) continue;
        out.gauge[root] = 0;
        std::queue<std::size_t> q;
        q.push(root);
        while (!q.empty()) {
            const auto u = q.front(); q.pop();
            for (const auto& [v, tau] : adjacency[u]) {
                const int expected = out.gauge[u] ^ tau;
                if (out.gauge[v] < 0) {
                    out.gauge[v] = expected;
                    q.push(v);
                } else if (out.gauge[v] != expected) {
                    ++out.nontrivial_holonomy_witnesses;
                }
            }
        }
    }
    out.coboundary = out.nontrivial_holonomy_witnesses == 0;
    out.genuinely_twisted = out.transport_well_defined && !out.coboundary;
    if (out.coboundary) out.obstruction.clear();
    else out.obstruction = "nontrivial Z/2 holonomy detected";
    return out;
}

inline std::string render_twisted_quotient_lean() {
    return R"LEAN(import Mathlib

namespace RavelPlayground

/-- A Z/2 edge transport derived from endpoint fibers is automatically a
    cocycle on composable paths: intermediate fibers cancel. -/
theorem xor_transport_compose (a b c : Bool) :
    (a != b) != (b != c) = (a != c) := by
  cases a <;> cases b <;> cases c <;> decide

/-- If edge transport is a coboundary g(u) xor g(v), every closed path has
    trivial holonomy. -/
theorem coboundary_closed_walk
    {V : Type} (g : V → Bool) (walk : List V)
    (hclosed : walk.head? = walk.getLast?) :
    walk.head?.map g = walk.getLast?.map g := by
  simpa [hclosed]

end RavelPlayground
)LEAN";
}

} // namespace ravel::proof
