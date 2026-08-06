#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/path_injective_simulation.hpp"

namespace ravel::proof {

struct HigherBlockPresentation {
    MultiAdjacency adjacency;
    std::size_t original_vertices=0;
    std::size_t block_vertices=0;
    bool nonzero_spectrum_preserved=false;
};

// Directed edge shift.  Vertices are edge occurrences, so parallel edges are
// retained.  The edge shift is the BA partner of the original AB adjacency;
// hence the nonzero spectrum, in particular spectral radius for a recurrent
// graph, is unchanged.
inline HigherBlockPresentation derive_edge_shift_presentation(
    const MultiAdjacency& graph) {
    HigherBlockPresentation out; out.original_vertices=graph.size();
    struct Edge{std::size_t source,destination;}; std::vector<Edge> edges;
    std::vector<std::vector<std::size_t>> outgoing_edge_ids(graph.size());
    for(std::size_t u=0;u<graph.size();++u) for(auto v:graph[u]){
        if(v>=graph.size()) throw std::invalid_argument("edge shift: bad endpoint");
        outgoing_edge_ids[u].push_back(edges.size()); edges.push_back({u,v});
    }
    out.adjacency.resize(edges.size());
    for(std::size_t e=0;e<edges.size();++e)
        for(auto f:outgoing_edge_ids[edges[e].destination]) out.adjacency[e].push_back(f);
    out.block_vertices=edges.size(); out.nonzero_spectrum_preserved=!edges.empty();
    return out;
}

struct HigherBlockPathInjection {
    std::size_t depth=0;
    std::size_t presentation_vertices=0;
    PathInjectiveSimulationCertificate injection;
    bool spectral_radius_nonstrict=false;
    std::string obstruction;
};

inline HigherBlockPathInjection derive_higher_block_path_injection(
    const MultiAdjacency& source,const MultiAdjacency& target,
    std::size_t maximum_depth=4) {
    HigherBlockPathInjection out;
    MultiAdjacency current=source;
    for(std::size_t depth=1;depth<=maximum_depth;++depth){
        auto injection=derive_path_injective_simulation(current,target);
        if(injection.spectral_radius_nonstrict){
            out.depth=depth; out.presentation_vertices=current.size();
            out.injection=std::move(injection); out.spectral_radius_nonstrict=true; return out;
        }
        auto next=derive_edge_shift_presentation(current);
        if(!next.nonzero_spectrum_preserved||next.block_vertices>200000) break;
        current=std::move(next.adjacency);
    }
    out.obstruction="no bounded higher-block path injection"; return out;
}

inline HigherBlockPathInjection derive_synchronous_higher_block_path_injection(
    const MultiAdjacency& source,const MultiAdjacency& target,
    std::size_t maximum_depth=4) {
    HigherBlockPathInjection out;
    MultiAdjacency current_source=source, current_target=target;
    for(std::size_t depth=1;depth<=maximum_depth;++depth){
        auto injection=derive_path_injective_simulation(current_source,current_target);
        if(injection.spectral_radius_nonstrict){
            out.depth=depth; out.presentation_vertices=current_source.size();
            out.injection=std::move(injection); out.spectral_radius_nonstrict=true; return out;
        }
        auto ns=derive_edge_shift_presentation(current_source);
        auto nt=derive_edge_shift_presentation(current_target);
        if(!ns.nonzero_spectrum_preserved||!nt.nonzero_spectrum_preserved||
           ns.block_vertices>200000||nt.block_vertices>200000) break;
        current_source=std::move(ns.adjacency); current_target=std::move(nt.adjacency);
    }
    out.obstruction="no synchronous bounded higher-block path injection"; return out;
}

} // namespace ravel::proof
