#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/covering_translation_tube.hpp"

namespace ravel::proof {

using ShellReturnGraph = std::map<IntegerState,std::set<IntegerState>>;

struct ShellShadowSimulationCertificate {
    std::int64_t upper_bound = 0;
    std::int64_t lower_bound = 0;
    ShellReturnGraph upper;
    ShellReturnGraph lower;
    std::map<IntegerState,std::set<IntegerState>> relation;
    std::size_t initial_pairs = 0;
    std::size_t surviving_pairs = 0;
    std::size_t refinement_rounds = 0;
    bool radius_one_candidates = false;
    bool forward_simulation = false;
    bool total_on_upper = false;
    bool recurrent_descent = false;
    std::string obstruction;
};

inline std::set<IntegerState> graph_states(const ShellReturnGraph& graph) {
    std::set<IntegerState> out;
    for (const auto& [source,targets] : graph) {
        out.insert(source);
        out.insert(targets.begin(),targets.end());
    }
    return out;
}

inline bool shell_translation_difference(const IntegerState& upper,
                                         const IntegerState& lower,
                                         std::int64_t translation_bound) {
    if (upper.size()!=lower.size() || upper.empty() || translation_bound<=0) return false;
    std::int64_t norm=0;
    for(std::size_t i=0;i<upper.size();++i) {
        const auto d=upper[i]-lower[i];
        if(d < -translation_bound || d > translation_bound) return false;
        norm=std::max(norm,static_cast<std::int64_t>(std::llabs(d)));
    }
    return norm==translation_bound;
}

/** Greatest forward simulation from shell M into shell M-1.
 *
 * Initial pairs are exactly radius-one differences.  Refinement removes
 * (x,r) whenever an upper first-return edge x->y has no lower first-return
 * edge r->s with (y,s) still present.  A total fixed point transports every
 * infinite upper return path to an infinite lower return path.  Finiteness of
 * the lower shell then turns upper recurrence into lower recurrence.
 */
inline ShellShadowSimulationCertificate derive_shell_shadow_simulation_to_bound(
    std::int64_t upper_bound, std::int64_t lower_bound,
    ShellReturnGraph upper, ShellReturnGraph lower) {
    ShellShadowSimulationCertificate c;
    c.upper_bound=upper_bound;
    c.lower_bound=lower_bound;
    c.upper=std::move(upper);
    c.lower=std::move(lower);
    if(lower_bound<1 || upper_bound<=lower_bound){c.obstruction="invalid shell simulation bounds";return c;}
    const auto translation_bound=upper_bound-lower_bound;
    const auto upper_states=graph_states(c.upper);
    const auto lower_states=graph_states(c.lower);
    for(const auto& x:upper_states)
        for(const auto& r:lower_states)
            if(shell_translation_difference(x,r,translation_bound)) {
                c.relation[x].insert(r);
                ++c.initial_pairs;
            }
    c.radius_one_candidates=true;
    for(const auto& x:upper_states)
        if(c.relation[x].empty()) c.radius_one_candidates=false;
    if(!c.radius_one_candidates){c.obstruction="an upper state has no shell-translation lower candidate";return c;}

    bool changed=true;
    while(changed) {
        changed=false;
        ++c.refinement_rounds;
        auto next=c.relation;
        for(const auto& x:upper_states) {
            auto& keep=next[x];
            for(auto it=keep.begin();it!=keep.end();) {
                const auto& r=*it;
                bool good=true;
                const auto uit=c.upper.find(x);
                if(uit!=c.upper.end()) {
                    for(const auto& y:uit->second) {
                        bool matched=false;
                        const auto lit=c.lower.find(r);
                        if(lit!=c.lower.end())
                            for(const auto& s:lit->second) {
                                const auto ry=c.relation.find(y);
                                if(ry!=c.relation.end() && ry->second.contains(s)) {
                                    matched=true; break;
                                }
                            }
                        if(!matched){good=false;break;}
                    }
                }
                if(!good){it=keep.erase(it);changed=true;}
                else ++it;
            }
        }
        c.relation=std::move(next);
    }
    c.surviving_pairs=0;
    c.total_on_upper=true;
    for(const auto& x:upper_states){
        c.surviving_pairs+=c.relation[x].size();
        c.total_on_upper &= !c.relation[x].empty();
    }
    c.forward_simulation=c.total_on_upper;
    if(c.forward_simulation) {
        for(const auto& [x,rs]:c.relation)
            for(const auto& r:rs) {
                const auto uit=c.upper.find(x);
                if(uit==c.upper.end()) continue;
                for(const auto& y:uit->second) {
                    bool matched=false;
                    const auto lit=c.lower.find(r);
                    if(lit!=c.lower.end())
                        for(const auto& s:lit->second)
                            if(c.relation.at(y).contains(s)){matched=true;break;}
                    c.forward_simulation &= matched;
                }
            }
    }
    c.recurrent_descent=c.forward_simulation && c.total_on_upper;
    if(!c.recurrent_descent) c.obstruction="greatest shell-shadow simulation is not total";
    return c;
}

inline ShellShadowSimulationCertificate derive_shell_shadow_simulation(
    std::int64_t upper_bound, ShellReturnGraph upper, ShellReturnGraph lower) {
    return derive_shell_shadow_simulation_to_bound(
        upper_bound, upper_bound-1, std::move(upper), std::move(lower));
}

} // namespace ravel::proof
