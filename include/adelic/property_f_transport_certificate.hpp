// Reusable bounded transport certificate for the Property-(F) lift.
// This is the family-facing boundary between exact graph construction and
// later contact/sofic projections.  It deliberately records transient growth
// separately from recurrent SCC data.
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <queue>
#include <vector>

#include "adelic/coincidence_and_property_f.hpp"
#include "adelic/prefix_automaton.hpp"

namespace adelic {

struct PropertyFTransportCertificate {
    PropertyFResult result;
    long long edges = 0;
    long long max_depth = -1;
    long long boundary_edges = 0;
    long long partial_nonzero_backedges = 0;
    std::vector<long long> depth_counts;
};

template <std::size_t d>
PropertyFTransportCertificate derive_property_f_transport_certificate(
    const PrefixAutomaton<d>& automaton,
    long long node_budget,
    const std::function<bool(const mathlib::QElem&)>& extra_bound = nullptr,
    bool retain_boundary_sinks = false) {
    PropertyFTransportCertificate out;
    std::vector<std::vector<long long>> graph;
    out.result = check_property_f<d>(automaton, node_budget, extra_bound,
                                     &graph, nullptr, nullptr, nullptr,
                                     retain_boundary_sinks);
    out.boundary_edges = out.result.boundary_edges;
    std::vector<long long> depth(graph.size(), -1);
    std::queue<long long> pending;
    for (long long a = 0; a < static_cast<long long>(d) &&
                              a < static_cast<long long>(graph.size()); ++a) {
        depth[static_cast<std::size_t>(a)] = 0;
        pending.push(a);
    }
    while (!pending.empty()) {
        const long long u = pending.front();
        pending.pop();
        for (const long long v : graph[static_cast<std::size_t>(u)]) {
            if (v < 0 || v >= static_cast<long long>(graph.size())) continue;
            auto& dv = depth[static_cast<std::size_t>(v)];
            if (dv < 0) {
                dv = depth[static_cast<std::size_t>(u)] + 1;
                pending.push(v);
            }
        }
    }
    for (const auto d0 : depth) {
        if (d0 < 0) continue;
        if (d0 >= static_cast<long long>(out.depth_counts.size()))
            out.depth_counts.resize(static_cast<std::size_t>(d0 + 1), 0);
        ++out.depth_counts[static_cast<std::size_t>(d0)];
        out.max_depth = std::max(out.max_depth, d0);
    }
    for (const auto& successors : graph) out.edges += successors.size();

    // A back-edge in the nonzero partial graph is an obstruction candidate
    // even when the node budget prevents the full SCC pass.
    std::vector<unsigned char> color(graph.size(), 0);
    std::function<void(std::size_t)> visit = [&](std::size_t u) {
        color[u] = 1;
        for (const long long raw_v : graph[u]) {
            if (raw_v < static_cast<long long>(d) ||
                raw_v >= static_cast<long long>(graph.size())) continue;
            const std::size_t v = static_cast<std::size_t>(raw_v);
            if (color[v] == 1) ++out.partial_nonzero_backedges;
            else if (color[v] == 0) visit(v);
        }
        color[u] = 2;
    };
    for (std::size_t u = d; u < graph.size(); ++u)
        if (color[u] == 0) visit(u);
    return out;
}

}  // namespace adelic
