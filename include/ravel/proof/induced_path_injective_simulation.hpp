#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "ravel/proof/path_injective_simulation.hpp"

namespace ravel::proof {

struct InducedPathInjection {
    std::size_t source_vertices=0,section_vertices=0,target_vertices=0;
    std::size_t return_words=0,maximum_return_length=0;
    std::size_t initial_pairs=0,surviving_pairs=0,covered_section_vertices=0;
    bool interior_acyclic=true;
    bool multiplicity_preserving=false;
    bool complete=false;
    bool spectral_radius_nonstrict=false;
    std::string obstruction;
};

/** Equal-time path injection after inducing on a recurrent section.
 *
 * Every source first-return word is matched injectively to a distinct target
 * path of the same length.  The strict interior outside the section must be
 * acyclic, so every bi-infinite source path decomposes into bounded return
 * words.  Concatenating the local injections yields an injection of source
 * paths into target paths with exactly the same elapsed time.
 */
template<class SourceSuccessorFn,class TargetSuccessorFn,class SectionFn,class AllowedFn>
InducedPathInjection derive_induced_path_injective_simulation(
    std::size_t source_count,std::size_t target_count,
    SourceSuccessorFn source_successors,TargetSuccessorFn target_successors,
    SectionFn in_section,AllowedFn allowed,std::size_t max_length=32,
    std::size_t path_cap=200000) {
    InducedPathInjection out;out.source_vertices=source_count;out.target_vertices=target_count;
    std::vector<MultiAdjacency::value_type> source(source_count),target(target_count);
    for(std::size_t s=0;s<source_count;++s)source[s]=source_successors(s);
    for(std::size_t t=0;t<target_count;++t)target[t]=target_successors(t);
    std::vector<std::size_t> section,local(source_count,source_count);
    for(std::size_t s=0;s<source_count;++s)if(in_section(s)){local[s]=section.size();section.push_back(s);}
    out.section_vertices=section.size();if(section.empty()){out.obstruction="induced injection: empty section";return out;}
    struct Word{std::size_t destination,length;};
    std::vector<std::vector<Word>> words(section.size());
    for(std::size_t r=0;r<section.size();++r){
        struct Item{std::size_t v,len;};std::vector<Item> q;
        for(auto v:source[section[r]])q.push_back({v,1});
        for(std::size_t h=0;h<q.size();++h){auto [v,len]=q[h];
            if(in_section(v)){words[r].push_back({local[v],len});++out.return_words;out.maximum_return_length=std::max(out.maximum_return_length,len);continue;}
            if(len>=max_length||q.size()>path_cap){out.interior_acyclic=false;continue;}
            for(auto w:source[v])q.push_back({w,len+1});
        }
    }
    if(!out.interior_acyclic){out.obstruction="induced injection: nonacyclic or oversized return interior";return out;}
    // Exact target path occurrences by length and start; endpoints repeated.
    std::vector<std::vector<std::vector<std::size_t>>> reach(out.maximum_return_length+1,
        std::vector<std::vector<std::size_t>>(target_count));
    for(std::size_t t=0;t<target_count;++t)reach[0][t]={t};
    for(std::size_t l=1;l<=out.maximum_return_length;++l)for(std::size_t t=0;t<target_count;++t){
        auto&dst=reach[l][t];for(auto u:target[t])for(auto v:reach[l-1][u]){dst.push_back(v);if(dst.size()>path_cap)break;}
    }
    std::vector<std::vector<bool>> live(section.size(),std::vector<bool>(target_count,false));
    for(std::size_t r=0;r<section.size();++r)for(std::size_t t=0;t<target_count;++t)if(allowed(section[r],t)){live[r][t]=true;++out.initial_pairs;}
    bool changed=true;while(changed){changed=false;for(std::size_t r=0;r<section.size();++r)for(std::size_t t=0;t<target_count;++t){if(!live[r][t])continue;
        std::vector<std::vector<std::size_t>> candidates(words[r].size());
        // Flatten target paths of the required lengths into distinct slots.
        std::vector<std::pair<std::size_t,std::size_t>> slots;
        std::map<std::size_t,std::vector<std::size_t>> slots_by_length;
        for(const auto&w:words[r])if(!slots_by_length.count(w.length)){
            auto&ids=slots_by_length[w.length];for(auto endpoint:reach[w.length][t]){ids.push_back(slots.size());slots.push_back({w.length,endpoint});}
        }
        for(std::size_t i=0;i<words[r].size();++i)for(auto slot:slots_by_length[words[r][i].length])
            if(live[words[r][i].destination][slots[slot].second])candidates[i].push_back(slot);
        if(!bipartite_injection_exists(candidates,slots.size())){live[r][t]=false;changed=true;}
    }}
    out.multiplicity_preserving=true;
    for(std::size_t r=0;r<section.size();++r){bool covered=false;for(std::size_t t=0;t<target_count;++t)if(live[r][t]){++out.surviving_pairs;covered=true;}out.covered_section_vertices+=covered;}
    out.complete=out.covered_section_vertices==section.size();
    out.spectral_radius_nonstrict=out.complete&&out.interior_acyclic&&out.multiplicity_preserving;
    if(!out.complete)out.obstruction="induced injection: no total equal-time return-word injection";
    return out;
}

} // namespace ravel::proof
