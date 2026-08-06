#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ravel::proof {

template<class Label> struct LabeledEdge { std::size_t destination=0; Label label{}; };

struct RightResolvingLanguageInclusion {
    std::size_t source_vertices=0,target_vertices=0,initial_pairs=0,surviving_pairs=0,covered_source_vertices=0;
    bool source_right_resolving=false,target_right_resolving=false,total=false;
    bool path_language_included=false,spectral_radius_nonstrict=false;
    std::string obstruction;
};

template<class Label>
RightResolvingLanguageInclusion derive_right_resolving_language_inclusion(
    const std::vector<std::vector<LabeledEdge<Label>>>& source,
    const std::vector<std::vector<LabeledEdge<Label>>>& target) {
    RightResolvingLanguageInclusion c;c.source_vertices=source.size();c.target_vertices=target.size();
    if(source.empty()||target.empty()){c.obstruction="language inclusion: empty graph";return c;}
    auto rr=[](const auto&g){for(const auto&row:g){std::map<Label,std::size_t>seen;for(const auto&e:row)if(!seen.emplace(e.label,e.destination).second)return false;}return true;};
    c.source_right_resolving=rr(source);c.target_right_resolving=rr(target);
    if(!c.source_right_resolving||!c.target_right_resolving){c.obstruction="language inclusion: graph is not right resolving";return c;}
    std::vector<std::vector<bool>> live(source.size(),std::vector<bool>(target.size(),true));c.initial_pairs=source.size()*target.size();
    bool changed=true;while(changed){changed=false;for(std::size_t s=0;s<source.size();++s)for(std::size_t t=0;t<target.size();++t){if(!live[s][t])continue;bool keep=true;
        std::map<Label,std::size_t> tout;for(const auto&e:target[t])tout[e.label]=e.destination;
        for(const auto&e:source[s]){auto it=tout.find(e.label);if(it==tout.end()||!live[e.destination][it->second]){keep=false;break;}}
        if(!keep){live[s][t]=false;changed=true;}
    }}
    for(std::size_t s=0;s<source.size();++s){bool covered=false;for(std::size_t t=0;t<target.size();++t)if(live[s][t]){++c.surviving_pairs;covered=true;}c.covered_source_vertices+=covered;}
    c.total=c.covered_source_vertices==source.size();
    c.path_language_included=c.total&&c.source_right_resolving&&c.target_right_resolving;
    c.spectral_radius_nonstrict=c.path_language_included;
    if(!c.total)c.obstruction="language inclusion: some source follower language is absent from target";
    return c;
}

} // namespace ravel::proof
