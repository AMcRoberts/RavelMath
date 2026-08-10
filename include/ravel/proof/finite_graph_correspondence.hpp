#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ravel::proof {

struct CorrespondenceEdge {
    std::size_t destination = 0;
    std::size_t target_edge = 0;
};

struct FiniteToOneGraphMap {
    std::size_t domain_vertices = 0;
    std::size_t target_vertices = 0;
    std::size_t fibre_product_vertices = 0;
    std::size_t recurrent_fibre_components = 0;
    std::size_t branching_recurrent_components = 0;
    bool vertex_map_valid = false;
    bool edge_map_valid = false;
    bool finite_to_one = false;
    std::string obstruction;
};

namespace correspondence_detail {

inline std::vector<std::vector<std::size_t>> sccs(
    const std::vector<std::vector<std::size_t>>& out) {
    const std::size_t n = out.size();
    std::vector<int> index(n,-1), low(n), stack;
    std::vector<bool> on(n,false);
    int next=0;
    std::vector<std::vector<std::size_t>> result;
    std::function<void(std::size_t)> dfs = [&](std::size_t u) {
        index[u]=low[u]=next++; stack.push_back((int)u); on[u]=true;
        for (auto v:out[u]) {
            if (index[v]<0) { dfs(v); low[u]=std::min(low[u],low[v]); }
            else if (on[v]) low[u]=std::min(low[u],index[v]);
        }
        if (low[u]!=index[u]) return;
        result.push_back({});
        while (true) {
            auto v=(std::size_t)stack.back(); stack.pop_back(); on[v]=false;
            result.back().push_back(v); if (v==u) break;
        }
    };
    for (std::size_t u=0;u<n;++u) if(index[u]<0) dfs(u);
    return result;
}

} // namespace correspondence_detail

/** Finite-to-one test for a graph map by its off-diagonal fibre product.
 *
 * Recurrent ambiguity is allowed only as a finite permutation of sheets.  A
 * branching recurrent SCC in the fibre product would create exponentially
 * many domain paths with the same target path and therefore is rejected.
 */
inline FiniteToOneGraphMap derive_finite_to_one_graph_map(
    const std::vector<std::size_t>& vertex_target,
    std::size_t target_vertices,
    const std::vector<std::vector<CorrespondenceEdge>>& domain_out,
    const std::vector<std::pair<std::size_t,std::size_t>>& target_edges) {
    FiniteToOneGraphMap c;
    c.domain_vertices=vertex_target.size(); c.target_vertices=target_vertices;
    if (domain_out.size()!=vertex_target.size()) {
        c.obstruction="finite graph map: domain size mismatch"; return c;
    }
    c.vertex_map_valid=std::all_of(vertex_target.begin(),vertex_target.end(),
        [&](auto v){return v<target_vertices;});
    if(!c.vertex_map_valid){c.obstruction="finite graph map: bad vertex image";return c;}
    c.edge_map_valid=true;
    for(std::size_t u=0;u<domain_out.size();++u) for(const auto&e:domain_out[u]){
        if(e.destination>=domain_out.size()||e.target_edge>=target_edges.size()){
            c.edge_map_valid=false;break;
        }
        const auto [a,b]=target_edges[e.target_edge];
        if(a!=vertex_target[u]||b!=vertex_target[e.destination]){
            c.edge_map_valid=false;break;
        }
    }
    if(!c.edge_map_valid){c.obstruction="finite graph map: edge image mismatch";return c;}

    std::vector<std::pair<std::size_t,std::size_t>> pairs;
    std::map<std::pair<std::size_t,std::size_t>,std::size_t> id;
    for(std::size_t a=0;a<domain_out.size();++a) for(std::size_t b=a+1;b<domain_out.size();++b)
        if(vertex_target[a]==vertex_target[b]) { id[{a,b}]=pairs.size(); pairs.push_back({a,b}); }
    c.fibre_product_vertices=pairs.size();
    std::vector<std::vector<std::size_t>> product(pairs.size());
    for(std::size_t p=0;p<pairs.size();++p){
        auto [a,b]=pairs[p];
        for(const auto&ea:domain_out[a]) for(const auto&eb:domain_out[b]){
            if(ea.target_edge!=eb.target_edge||ea.destination==eb.destination) continue;
            auto x=std::min(ea.destination,eb.destination), y=std::max(ea.destination,eb.destination);
            auto it=id.find({x,y}); if(it!=id.end()) product[p].push_back(it->second);
        }
    }
    const auto comps=correspondence_detail::sccs(product);
    for(const auto&comp:comps){
        std::set<std::size_t> inside(comp.begin(),comp.end());
        bool recurrent=comp.size()>1;
        if(comp.size()==1) for(auto v:product[comp[0]]) recurrent|=v==comp[0];
        if(!recurrent) continue;
        ++c.recurrent_fibre_components;
        bool permutation=true;
        std::map<std::size_t,std::size_t> indeg;
        for(auto u:comp){
            std::size_t outdeg=0;
            for(auto v:product[u]) if(inside.count(v)){++outdeg;++indeg[v];}
            if(outdeg!=1) permutation=false;
        }
        for(auto u:comp) if(indeg[u]!=1) permutation=false;
        if(!permutation) ++c.branching_recurrent_components;
    }
    c.finite_to_one=c.branching_recurrent_components==0;
    if(!c.finite_to_one)c.obstruction="finite graph map: branching recurrent ambiguity";
    return c;
}

struct FiniteGraphCorrespondenceEntropyBound {
    FiniteToOneGraphMap to_source;
    FiniteToOneGraphMap to_target;
    bool source_path_surjective=false;
    bool entropy_source_at_most_target=false;
    std::string obstruction;
};

// A finite boundary/escape certificate for a directed relation.  `live`
// selects the greatest surviving subrelation, while `terminal` marks vertices
// accepted as boundary exits.  The certificate is intentionally iterative:
// the completion relations used by the adelic bridge can have millions of
// candidates, so recursion is not an acceptable proof-side implementation.
struct FiniteEscapeBoundaryCertificate {
    std::size_t live_vertices = 0;
    std::size_t live_edges = 0;
    std::size_t max_terminal_distance = 0;
    std::size_t live_vertices_without_terminal_route = 0;
    std::vector<std::size_t> terminal_distance;
    bool acyclic = false;
    bool every_live_vertex_reaches_terminal = false;
};

// Local certificate for the symbolic height premise of an escape proof.
// `height` is a nonnegative integer rank supplied by the caller.  Every live
// edge out of a nonterminal vertex must strictly decrease it, and accepted
// terminal vertices must have no live outgoing edge.  This is deliberately a
// separate contract from terminal reachability: strict descent proves that a
// recurrent obstruction cannot survive, while the boundary certificate above
// proves that every surviving vertex actually reaches the accepted shell.
struct FiniteEscapeHeightCertificate {
    std::size_t live_vertices = 0;
    std::size_t live_edges = 0;
    std::size_t checked_edges = 0;
    std::size_t height_violations = 0;
    std::size_t terminal_outgoing_edges = 0;
    std::size_t maximum_live_height = 0;
    bool strictly_decreasing = false;
    bool terminals_absorbing = false;
    bool proves_acyclic = false;
};

// Lexicographic version of the local escape-height contract.  The primary
// coordinate describes a boundary/SCC layer; the secondary coordinate is a
// bounded phase or carry residual used only when the primary layer is fixed.
// A live nonterminal edge must decrease the ordered pair
// `(primary, secondary)`.  This is the direct finite analogue of a
// Lyapunov function with a neutral fibre, and avoids encoding an arbitrary
// weighting constant into a theorem-specific caller.
struct FiniteEscapeLexicographicHeightCertificate {
    std::size_t live_vertices = 0;
    std::size_t live_edges = 0;
    std::size_t checked_edges = 0;
    std::size_t primary_decreases = 0;
    std::size_t secondary_tie_decreases = 0;
    std::size_t height_violations = 0;
    std::size_t terminal_outgoing_edges = 0;
    std::size_t maximum_primary = 0;
    std::size_t maximum_secondary = 0;
    bool strictly_decreasing = false;
    bool terminals_absorbing = false;
    bool proves_acyclic = false;
};

// Contract for a shell/birth-round stratification of a finite graph.  The
// caller supplies the SCC (or recurrent-block) labels, a recurrent flag for
// each component, and the proposed birth round of each node.  This operation
// verifies the structural claims used by the Class-II round audit without
// hiding the derivation of those labels or rounds in a probe.
struct StratifiedEscapeCertificate {
    std::size_t node_count = 0;
    std::size_t component_count = 0;
    std::size_t recurrent_components = 0;
    std::size_t transient_nodes = 0;
    std::size_t transient_groups = 0;
    std::size_t transient_groups_with_escape = 0;
    std::size_t recurrent_round_inconsistencies = 0;
    std::size_t recurrent_to_earlier_transient_edges = 0;
    std::size_t transient_escape_edges = 0;
    std::vector<std::size_t> component_birth_round;
    bool labels_valid = false;
    bool component_rounds_consistent = false;
    bool recurrent_no_earlier_return = false;
    bool every_transient_group_escapes = false;
    bool valid = false;
};

inline StratifiedEscapeCertificate derive_stratified_escape_certificate(
    const std::vector<std::vector<std::size_t>>& adjacency,
    const std::vector<long long>& component,
    const std::vector<bool>& recurrent_component,
    const std::vector<std::size_t>& birth_round) {
    if (adjacency.size() != component.size() ||
        component.size() != birth_round.size())
        throw std::invalid_argument(
            "stratified escape: graph/vector size mismatch");
    StratifiedEscapeCertificate out;
    out.node_count = adjacency.size();
    for (const auto label : component) {
        if (label < 0)
            throw std::invalid_argument(
                "stratified escape: negative component label");
        out.component_count = std::max(
            out.component_count, static_cast<std::size_t>(label) + 1);
    }
    out.labels_valid = recurrent_component.size() == out.component_count;
    if (!out.labels_valid) return out;

    std::vector<std::size_t> component_size(out.component_count, 0);
    const auto unknown = std::numeric_limits<std::size_t>::max();
    out.component_birth_round.assign(out.component_count, unknown);
    std::vector<bool> component_round_consistent(out.component_count, true);
    for (std::size_t u = 0; u < out.node_count; ++u) {
        const auto c = static_cast<std::size_t>(component[u]);
        ++component_size[c];
        if (out.component_birth_round[c] == unknown)
            out.component_birth_round[c] = birth_round[u];
        else if (out.component_birth_round[c] != birth_round[u]) {
            component_round_consistent[c] = false;
            ++out.recurrent_round_inconsistencies;
        }
    }
    out.labels_valid = std::all_of(
        component_size.begin(), component_size.end(),
        [](const auto size) { return size != 0; });
    out.component_rounds_consistent =
        out.labels_valid && std::all_of(
            component_round_consistent.begin(),
            component_round_consistent.end(), [](const bool okay) { return okay; });
    for (std::size_t c = 0; c < out.component_count; ++c)
        if (recurrent_component[c]) ++out.recurrent_components;

    std::set<std::size_t> transient_rounds;
    for (std::size_t u = 0; u < out.node_count; ++u) {
        const auto c = static_cast<std::size_t>(component[u]);
        if (recurrent_component[c]) continue;
        ++out.transient_nodes;
        transient_rounds.insert(birth_round[u]);
    }
    out.transient_groups = transient_rounds.size();

    std::vector<bool> group_has_escape;
    group_has_escape.assign(transient_rounds.size(), false);
    std::map<std::size_t, std::size_t> group_index;
    std::size_t next_group = 0;
    for (const auto round : transient_rounds)
        group_index.emplace(round, next_group++);

    for (std::size_t u = 0; u < out.node_count; ++u) {
        const auto source_component = static_cast<std::size_t>(component[u]);
        for (const auto v : adjacency[u]) {
            if (v >= out.node_count)
                throw std::invalid_argument(
                    "stratified escape: edge out of range");
            const auto target_component =
                static_cast<std::size_t>(component[v]);
            if (recurrent_component[source_component]) {
                if (!recurrent_component[target_component] &&
                    birth_round[v] < out.component_birth_round[source_component])
                    ++out.recurrent_to_earlier_transient_edges;
                continue;
            }
            const auto source_round = birth_round[u];
            const bool escapes = recurrent_component[target_component] ||
                birth_round[v] > source_round;
            if (!escapes) continue;
            ++out.transient_escape_edges;
            group_has_escape[group_index.at(source_round)] = true;
        }
    }
    out.transient_groups_with_escape = static_cast<std::size_t>(
        std::count(group_has_escape.begin(), group_has_escape.end(), true));
    out.recurrent_no_earlier_return =
        out.recurrent_to_earlier_transient_edges == 0;
    out.every_transient_group_escapes =
        out.transient_groups_with_escape == out.transient_groups;
    out.valid = out.labels_valid && out.component_rounds_consistent &&
        out.recurrent_no_earlier_return && out.every_transient_group_escapes;
    return out;
}

inline FiniteEscapeHeightCertificate derive_finite_escape_height_certificate(
    const std::vector<std::vector<std::size_t>>& adjacency,
    const std::vector<bool>& live,
    const std::vector<bool>& terminal,
    const std::vector<std::size_t>& height) {
    if (adjacency.size() != live.size() || live.size() != terminal.size() ||
        terminal.size() != height.size())
        throw std::invalid_argument("escape height: graph/vector size mismatch");
    const std::size_t n = adjacency.size();
    FiniteEscapeHeightCertificate out;
    for (std::size_t u = 0; u < n; ++u) {
        if (!live[u]) continue;
        ++out.live_vertices;
        out.maximum_live_height = std::max(out.maximum_live_height, height[u]);
        for (const auto v : adjacency[u]) {
            if (v >= n) throw std::invalid_argument("escape height: edge out of range");
            if (!live[v]) continue;
            ++out.live_edges;
            if (terminal[u]) {
                ++out.terminal_outgoing_edges;
                continue;
            }
            ++out.checked_edges;
            if (height[u] <= height[v]) ++out.height_violations;
        }
    }
    out.strictly_decreasing = out.height_violations == 0;
    out.terminals_absorbing = out.terminal_outgoing_edges == 0;
    out.proves_acyclic = out.strictly_decreasing && out.terminals_absorbing;
    return out;
}

inline FiniteEscapeLexicographicHeightCertificate
derive_finite_escape_lexicographic_height_certificate(
    const std::vector<std::vector<std::size_t>>& adjacency,
    const std::vector<bool>& live,
    const std::vector<bool>& terminal,
    const std::vector<std::size_t>& primary,
    const std::vector<std::size_t>& secondary) {
    if (adjacency.size() != live.size() || live.size() != terminal.size() ||
        terminal.size() != primary.size() || primary.size() != secondary.size())
        throw std::invalid_argument(
            "lexicographic escape height: graph/vector size mismatch");
    const std::size_t n = adjacency.size();
    FiniteEscapeLexicographicHeightCertificate out;
    for (std::size_t u = 0; u < n; ++u) {
        if (!live[u]) continue;
        ++out.live_vertices;
        out.maximum_primary = std::max(out.maximum_primary, primary[u]);
        out.maximum_secondary = std::max(out.maximum_secondary, secondary[u]);
        for (const auto v : adjacency[u]) {
            if (v >= n)
                throw std::invalid_argument(
                    "lexicographic escape height: edge out of range");
            if (!live[v]) continue;
            ++out.live_edges;
            if (terminal[u]) {
                ++out.terminal_outgoing_edges;
                continue;
            }
            ++out.checked_edges;
            if (primary[u] > primary[v]) {
                ++out.primary_decreases;
            } else if (primary[u] == primary[v] &&
                       secondary[u] > secondary[v]) {
                ++out.secondary_tie_decreases;
            } else {
                ++out.height_violations;
            }
        }
    }
    out.strictly_decreasing = out.height_violations == 0;
    out.terminals_absorbing = out.terminal_outgoing_edges == 0;
    out.proves_acyclic = out.strictly_decreasing && out.terminals_absorbing;
    return out;
}

inline FiniteEscapeBoundaryCertificate derive_finite_escape_boundary_certificate(
    const std::vector<std::vector<std::size_t>>& adjacency,
    const std::vector<bool>& live,
    const std::vector<bool>& terminal) {
    if (adjacency.size() != live.size() || live.size() != terminal.size())
        throw std::invalid_argument("escape boundary: graph/vector size mismatch");
    const std::size_t n = adjacency.size();
    FiniteEscapeBoundaryCertificate out;
    out.terminal_distance.assign(n, std::numeric_limits<std::size_t>::max());
    std::vector<std::size_t> indegree(n, 0);
    std::size_t live_count = 0;
    for (std::size_t u = 0; u < n; ++u) {
        if (!live[u]) continue;
        ++live_count;
        for (const auto v : adjacency[u]) {
            if (v >= n) throw std::invalid_argument("escape boundary: edge out of range");
            if (!live[v]) continue;
            ++out.live_edges;
            ++indegree[v];
        }
    }
    out.live_vertices = live_count;
    std::queue<std::size_t> ready;
    for (std::size_t u = 0; u < n; ++u)
        if (live[u] && indegree[u] == 0) ready.push(u);
    std::vector<std::size_t> order;
    order.reserve(live_count);
    while (!ready.empty()) {
        const auto u = ready.front();
        ready.pop();
        order.push_back(u);
        for (const auto v : adjacency[u]) {
            if (!live[v]) continue;
            if (--indegree[v] == 0) ready.push(v);
        }
    }
    out.acyclic = order.size() == live_count;
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        const auto u = *it;
        if (terminal[u]) {
            out.terminal_distance[u] = 0;
        } else {
            for (const auto v : adjacency[u]) {
                if (!live[v] || out.terminal_distance[v] ==
                                   std::numeric_limits<std::size_t>::max())
                    continue;
                out.terminal_distance[u] = std::min(
                    out.terminal_distance[u], out.terminal_distance[v] + 1);
            }
        }
        if (out.terminal_distance[u] != std::numeric_limits<std::size_t>::max())
            out.max_terminal_distance = std::max(
                out.max_terminal_distance, out.terminal_distance[u]);
    }
    for (std::size_t u = 0; u < n; ++u) {
        if (live[u] && out.terminal_distance[u] ==
                           std::numeric_limits<std::size_t>::max())
            ++out.live_vertices_without_terminal_route;
    }
    out.every_live_vertex_reaches_terminal =
        out.live_vertices_without_terminal_route == 0;
    return out;
}

inline FiniteGraphCorrespondenceEntropyBound compose_finite_correspondence_bound(
    FiniteToOneGraphMap to_source, FiniteToOneGraphMap to_target,
    bool source_path_surjective) {
    FiniteGraphCorrespondenceEntropyBound c;
    c.to_source=std::move(to_source); c.to_target=std::move(to_target);
    c.source_path_surjective=source_path_surjective;
    c.entropy_source_at_most_target=c.source_path_surjective&&
        c.to_source.finite_to_one&&c.to_target.finite_to_one;
    if(!c.entropy_source_at_most_target)
        c.obstruction="finite correspondence: a projection is not finite-to-one or source-surjective";
    return c;
}

} // namespace ravel::proof
