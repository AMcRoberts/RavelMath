#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <set>
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
