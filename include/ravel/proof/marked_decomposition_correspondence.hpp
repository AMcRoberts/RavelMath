#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/finite_graph_correspondence.hpp"
#include "ravel/proof/marked_atom_core_extension.hpp"

namespace ravel::proof {

struct MarkedDecompositionCorrespondence {
    std::size_t source_vertices=0;
    std::size_t marked_vertices=0;
    std::size_t marked_edges=0;
    std::size_t live_marked_vertices=0;
    bool source_path_surjective=false;
    FiniteGraphCorrespondenceEntropyBound entropy;
    bool spectral_radius_at_most_core=false;
    std::string obstruction;
};

template<class SuccessorFn,class PairFn,class VectorFn>
MarkedDecompositionCorrespondence derive_marked_decomposition_correspondence(
    const DisplacementSumCatalogue& catalogue,
    std::size_t vertex_count,
    SuccessorFn successors,
    PairFn pair_of,
    VectorFn vector_of) {
    MarkedDecompositionCorrespondence out; out.source_vertices=vertex_count;
    const auto n=catalogue.dimension();
    std::vector<std::vector<GradeDescentWitness>> fibres(vertex_count);
    std::vector<std::vector<std::size_t>> source_out(vertex_count);
    struct MV{std::size_t source,local;}; std::vector<MV> mv;
    std::vector<std::vector<std::size_t>> mid(vertex_count);
    for(std::size_t u=0;u<vertex_count;++u){
        fibres[u]=marked_atom_detail::complete_marked_fibre(catalogue,pair_of(u),vector_of(u));
        source_out[u]=successors(u); mid[u].resize(fibres[u].size());
        for(std::size_t w=0;w<fibres[u].size();++w){mid[u][w]=mv.size();mv.push_back({u,w});}
    }
    out.marked_vertices=mv.size();
    if(!vertex_count||mv.empty()){out.obstruction="marked correspondence: empty graph";return out;}

    // Edge occurrences in the source graph.
    std::vector<std::pair<std::size_t,std::size_t>> source_edges;
    std::vector<std::vector<std::size_t>> source_edge_ids(vertex_count);
    for(std::size_t u=0;u<vertex_count;++u) for(auto v:source_out[u]){
        source_edge_ids[u].push_back(source_edges.size()); source_edges.push_back({u,v});
    }

    // Enumerate all predicted-core states that occur as marked projections.
    std::map<nbonacci_margin::CoreState,std::size_t> core_id;
    for(std::size_t id=0;id<mv.size();++id){auto [u,w]=mv[id];
        core_id.try_emplace({pair_of(u),fibres[u][w].active_descriptor},core_id.size());}
    // Include destinations reached by core edges so edge images are complete.
    bool grew=true; while(grew){grew=false; std::vector<nbonacci_margin::CoreState> states;
        for(auto&[s,i]:core_id){(void)i;states.push_back(s);} for(auto&s:states)
        for(auto&t:nbonacci_margin::predicted_core_successors(n,s))
            if(core_id.try_emplace(t,core_id.size()).second) grew=true;
    }
    std::vector<std::pair<std::size_t,std::size_t>> core_edges;
    std::map<std::pair<std::size_t,std::size_t>,std::size_t> core_edge_id;
    for(auto&[s,si]:core_id) for(auto&t:nbonacci_margin::predicted_core_successors(n,s)){
        auto ti=core_id.at(t); core_edge_id[{si,ti}]=core_edges.size();core_edges.push_back({si,ti});}

    std::vector<std::vector<CorrespondenceEdge>> marked_to_source(mv.size()), marked_to_core(mv.size());
    for(std::size_t id=0;id<mv.size();++id){auto [u,w]=mv[id]; const auto&mark=fibres[u][w];
        for(std::size_t epos=0;epos<source_out[u].size();++epos){auto v=source_out[u][epos];
            for(std::size_t z=0;z<fibres[v].size();++z) if(marked_atom_detail::same_mark_transport(
                n,pair_of(u),mark,pair_of(v),fibres[v][z])){
                auto dest=mid[v][z]; auto seid=source_edge_ids[u][epos];
                marked_to_source[id].push_back({dest,seid});
                const auto cs=core_id.at({pair_of(u),mark.active_descriptor});
                const auto ct=core_id.at({pair_of(v),fibres[v][z].active_descriptor});
                auto ce=core_edge_id.find({cs,ct}); if(ce!=core_edge_id.end())
                    marked_to_core[id].push_back({dest,ce->second});
                ++out.marked_edges;
            }
        }
    }

    // Greatest subgraph locally surjective over every source edge occurrence.
    std::vector<bool> live(mv.size(),true); bool changed=true;
    while(changed){changed=false;for(std::size_t id=0;id<mv.size();++id){if(!live[id])continue;
        auto u=mv[id].source; bool keep=!source_out[u].empty();
        for(auto seid:source_edge_ids[u]){bool found=false;for(auto&e:marked_to_source[id])
            if(e.target_edge==seid&&live[e.destination]){found=true;break;} if(!found){keep=false;break;}}
        if(!keep){live[id]=false;changed=true;}
    }}
    std::vector<std::size_t> remap(mv.size(),(std::size_t)-1); for(std::size_t i=0;i<mv.size();++i)
        if(live[i]){remap[i]=out.live_marked_vertices++;}
    std::vector<std::size_t> source_vertex_map(out.live_marked_vertices), core_vertex_map(out.live_marked_vertices);
    std::vector<std::vector<CorrespondenceEdge>> ms(out.live_marked_vertices), mc(out.live_marked_vertices);
    for(std::size_t id=0;id<mv.size();++id)if(live[id]){auto ni=remap[id];auto [u,w]=mv[id];
        source_vertex_map[ni]=u; core_vertex_map[ni]=core_id.at({pair_of(u),fibres[u][w].active_descriptor});
        for(auto&e:marked_to_source[id])if(live[e.destination])ms[ni].push_back({remap[e.destination],e.target_edge});
        for(auto&e:marked_to_core[id])if(live[e.destination])mc[ni].push_back({remap[e.destination],e.target_edge});
    }
    out.source_path_surjective=true;
    for(std::size_t u=0;u<vertex_count;++u){bool covered=false;for(std::size_t id=0;id<mv.size();++id)
        if(live[id]&&mv[id].source==u){covered=true;break;} out.source_path_surjective&=covered;}
    auto ps=derive_finite_to_one_graph_map(source_vertex_map,vertex_count,ms,source_edges);
    auto pc=derive_finite_to_one_graph_map(core_vertex_map,core_id.size(),mc,core_edges);
    out.entropy=compose_finite_correspondence_bound(std::move(ps),std::move(pc),out.source_path_surjective);
    out.spectral_radius_at_most_core=out.entropy.entropy_source_at_most_target;
    if(!out.spectral_radius_at_most_core)out.obstruction=out.entropy.obstruction;
    return out;
}

} // namespace ravel::proof
