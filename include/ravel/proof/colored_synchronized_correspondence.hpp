#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/finite_graph_correspondence.hpp"

namespace ravel::proof {

struct ColoredEdgeOccurrence {
    std::size_t destination=0;
    std::size_t color=0;
};

struct ColoredSynchronizedCorrespondence {
    std::size_t source_vertices=0,target_vertices=0,product_vertices=0,live_vertices=0;
    bool source_path_surjective=false;
    FiniteGraphCorrespondenceEntropyBound entropy;
    bool spectral_radius_at_most_target=false;
    std::string obstruction;
};

/** Fibre product of two colored graphs, followed by the greatest subgraph
 * locally surjective over the source.  The two projections are then checked
 * for finite ambiguity by off-diagonal fibre products.  This is the finite
 * sofic/textile form of a Condition-F zero-expansion synchronization.
 */
inline ColoredSynchronizedCorrespondence derive_colored_synchronized_correspondence(
    const std::vector<std::vector<ColoredEdgeOccurrence>>& source,
    const std::vector<std::vector<ColoredEdgeOccurrence>>& target) {
    ColoredSynchronizedCorrespondence out;
    out.source_vertices=source.size();out.target_vertices=target.size();
    if(source.empty()||target.empty()){out.obstruction="colored synchronization: empty graph";return out;}
    struct P{std::size_t s,t;};std::vector<P> states;
    std::vector<std::vector<std::size_t>> id(source.size(),std::vector<std::size_t>(target.size()));
    for(std::size_t s=0;s<source.size();++s)for(std::size_t t=0;t<target.size();++t){id[s][t]=states.size();states.push_back({s,t});}
    out.product_vertices=states.size();

    std::vector<std::pair<std::size_t,std::size_t>> source_edges,target_edges;
    std::vector<std::vector<std::size_t>> seid(source.size()),teid(target.size());
    for(std::size_t s=0;s<source.size();++s)for(auto&e:source[s]){seid[s].push_back(source_edges.size());source_edges.push_back({s,e.destination});}
    for(std::size_t t=0;t<target.size();++t)for(auto&e:target[t]){teid[t].push_back(target_edges.size());target_edges.push_back({t,e.destination});}

    struct PE{std::size_t dest,se,te;};std::vector<std::vector<PE>> edges(states.size());
    for(std::size_t p=0;p<states.size();++p){auto [s,t]=states[p];
        for(std::size_t a=0;a<source[s].size();++a)for(std::size_t b=0;b<target[t].size();++b)
            if(source[s][a].color==target[t][b].color)
                edges[p].push_back({id[source[s][a].destination][target[t][b].destination],seid[s][a],teid[t][b]});
    }
    std::vector<bool> live(states.size(),true);bool changed=true;
    while(changed){changed=false;for(std::size_t p=0;p<states.size();++p){if(!live[p])continue;auto s=states[p].s;bool keep=!source[s].empty();
        for(auto se:seid[s]){bool found=false;for(auto&e:edges[p])if(e.se==se&&live[e.dest]){found=true;break;}if(!found){keep=false;break;}}
        if(!keep){live[p]=false;changed=true;}}
    }
    std::vector<std::size_t> remap(states.size(),(std::size_t)-1);for(std::size_t p=0;p<states.size();++p)if(live[p])remap[p]=out.live_vertices++;
    std::vector<std::size_t> smap(out.live_vertices),tmap(out.live_vertices);
    std::vector<std::vector<CorrespondenceEdge>> ps(out.live_vertices),pt(out.live_vertices);
    for(std::size_t p=0;p<states.size();++p)if(live[p]){auto q=remap[p];smap[q]=states[p].s;tmap[q]=states[p].t;
        for(auto&e:edges[p])if(live[e.dest]){ps[q].push_back({remap[e.dest],e.se});pt[q].push_back({remap[e.dest],e.te});}}
    out.source_path_surjective=true;for(std::size_t s=0;s<source.size();++s){bool found=false;for(std::size_t p=0;p<states.size();++p)if(live[p]&&states[p].s==s){found=true;break;}out.source_path_surjective&=found;}
    auto fs=derive_finite_to_one_graph_map(smap,source.size(),ps,source_edges);
    auto ft=derive_finite_to_one_graph_map(tmap,target.size(),pt,target_edges);
    out.entropy=compose_finite_correspondence_bound(std::move(fs),std::move(ft),out.source_path_surjective);
    out.spectral_radius_at_most_target=out.entropy.entropy_source_at_most_target;
    if(!out.spectral_radius_at_most_target)out.obstruction=out.entropy.obstruction;
    return out;
}

} // namespace ravel::proof
