#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/shell_shadow_simulation.hpp"

namespace ravel::proof {

struct ConcreteShellRankCertificate {
    ShellReturnGraph graph;
    std::map<IntegerState,std::int64_t> rank;
    bool acyclic = false;
    bool strict_on_edges = false;
    bool valid = false;
    std::string obstruction;
};

inline ConcreteShellRankCertificate derive_concrete_shell_rank(
    ShellReturnGraph graph) {
    ConcreteShellRankCertificate c;
    c.graph=std::move(graph);
    const auto states=graph_states(c.graph);
    std::map<IntegerState,std::size_t> indegree;
    for(const auto& x:states) indegree[x]=0;
    for(const auto& [x,ys]:c.graph)
        for(const auto& y:ys) ++indegree[y];
    std::deque<IntegerState> q;
    for(const auto& [x,d]:indegree) if(d==0) q.push_back(x);
    std::size_t seen=0;
    for(const auto& x:states)c.rank[x]=0;
    while(!q.empty()){
        auto x=q.front();q.pop_front();++seen;
        const auto it=c.graph.find(x);
        if(it==c.graph.end())continue;
        for(const auto& y:it->second){
            c.rank[y]=std::max(c.rank[y],c.rank[x]+1);
            if(--indegree[y]==0)q.push_back(y);
        }
    }
    c.acyclic=seen==states.size();
    if(!c.acyclic){c.obstruction="shell graph contains a cycle";return c;}
    c.strict_on_edges=true;
    for(const auto& [x,ys]:c.graph)
        for(const auto& y:ys)c.strict_on_edges &= c.rank[x]<c.rank[y];
    c.valid=c.acyclic&&c.strict_on_edges;
    if(!c.valid)c.obstruction="concrete shell rank replay failed";
    return c;
}

struct SimulatedShellRankCertificate {
    ShellShadowSimulationCertificate simulation;
    ConcreteShellRankCertificate lower_rank;
    std::map<IntegerState,std::int64_t> upper_rank;
    bool maximum_lift = false;
    bool strict_on_upper_edges = false;
    bool valid = false;
    std::string obstruction;
};

/** Transport a strict lower-shell rank through a total forward simulation.
 *
 * Define R_M(x)=max{R_base(r):(x,r) in S}.  For x->y, choose a maximizing
 * r.  Simulation supplies r->s with (y,s) in S, and strictness below gives
 * R_base(s)>R_base(r).  Hence R_M(y)>R_M(x).
 */
inline SimulatedShellRankCertificate transport_shell_rank_through_simulation(
    ShellShadowSimulationCertificate simulation,
    ConcreteShellRankCertificate lower_rank) {
    SimulatedShellRankCertificate c;
    c.simulation=std::move(simulation);
    c.lower_rank=std::move(lower_rank);
    if(!c.simulation.recurrent_descent){
        c.obstruction="shell simulation is not total";return c;
    }
    if(!c.lower_rank.valid){
        c.obstruction="lower shell rank is invalid";return c;
    }
    c.maximum_lift=true;
    for(const auto& [x,rs]:c.simulation.relation){
        if(rs.empty()){c.maximum_lift=false;break;}
        bool first=true;std::int64_t value=0;
        for(const auto& r:rs){
            const auto it=c.lower_rank.rank.find(r);
            if(it==c.lower_rank.rank.end()){c.maximum_lift=false;break;}
            if(first||it->second>value){value=it->second;first=false;}
        }
        if(!c.maximum_lift)break;
        c.upper_rank[x]=value;
    }
    if(!c.maximum_lift){c.obstruction="simulation escaped lower rank domain";return c;}
    c.strict_on_upper_edges=true;
    for(const auto& [x,ys]:c.simulation.upper)
        for(const auto& y:ys){
            const auto xi=c.upper_rank.find(x), yi=c.upper_rank.find(y);
            c.strict_on_upper_edges &= xi!=c.upper_rank.end() &&
                                       yi!=c.upper_rank.end() &&
                                       xi->second<yi->second;
        }
    c.valid=c.maximum_lift&&c.strict_on_upper_edges;
    if(!c.valid)c.obstruction="maximum rank lift failed strict replay";
    return c;
}

} // namespace ravel::proof
