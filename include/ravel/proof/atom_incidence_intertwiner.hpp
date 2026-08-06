#pragma once
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "ravel/proof/complete_atom_tuple_extension.hpp"
namespace ravel::proof {
struct AtomIncidenceIntertwinerAudit {
 std::size_t source_vertices=0, core_vertices=0, nonzero_entries=0;
 std::size_t ap_le_pb_violations=0, ap_ge_pb_violations=0;
 long long maximum_le_defect=0, maximum_ge_defect=0;
 bool subintertwiner=false, superintertwiner=false;
 std::vector<std::string> sample_violations;
 std::string obstruction;
};
template<class SuccessorFn,class PairFn,class VectorFn>
AtomIncidenceIntertwinerAudit derive_atom_incidence_intertwiner_audit(
 const DisplacementSumCatalogue& catalogue,std::size_t source_count,
 SuccessorFn successors,PairFn pair_of,VectorFn vector_of){
 AtomIncidenceIntertwinerAudit out;out.source_vertices=source_count;auto n=catalogue.dimension();
 std::vector<nbonacci_margin::CoreState> core;std::map<nbonacci_margin::CoreState,size_t> cid;
 for(auto p:nbonacci_margin::label_pairs(n))for(auto d:nbonacci_margin::displacement_descriptors(n))
  if(nbonacci_margin::predicted_core_member(n,p,d)){cid[{p,d}]=core.size();core.push_back({p,d});}
 out.core_vertices=core.size();
 std::vector<std::map<size_t,long long>> P(source_count);
 for(size_t u=0;u<source_count;++u){auto dec=complete_atom_tuple_detail::exact_atom_decompositions(catalogue,vector_of(u));
  for(auto&t:dec)for(auto d:t){auto it=cid.find({pair_of(u),d});if(it!=cid.end())++P[u][it->second];}
  out.nonzero_entries+=P[u].size();
 }
 std::vector<std::vector<size_t>> bout(core.size());
 for(size_t c=0;c<core.size();++c)for(auto t:nbonacci_margin::predicted_core_successors(n,core[c]))bout[c].push_back(cid.at(t));
 for(size_t u=0;u<source_count;++u){std::map<size_t,long long> ap,pb;
  for(auto v:successors(u))for(auto [c,w]:P[v])ap[c]+=w;
  for(auto [c,w]:P[u])for(auto d:bout[c])pb[d]+=w;
  std::set<size_t> cols;for(auto [c,w]:ap)cols.insert(c);for(auto[c,w]:pb)cols.insert(c);
  for(auto c:cols){auto a=ap[c],b=pb[c];if(a>b){++out.ap_le_pb_violations;out.maximum_le_defect=std::max(out.maximum_le_defect,a-b);if(out.sample_violations.size()<12){const auto& cs=core[c];out.sample_violations.push_back("u="+std::to_string(u)+" pair="+std::to_string(pair_of(u).i)+","+std::to_string(pair_of(u).j)+" colPair="+std::to_string(cs.pair.i)+","+std::to_string(cs.pair.j)+" kind="+std::to_string((int)cs.displacement.kind)+" sign="+std::to_string(cs.displacement.sign)+" abc="+std::to_string(cs.displacement.a)+","+std::to_string(cs.displacement.b)+","+std::to_string(cs.displacement.c)+" AP="+std::to_string(a)+" PB="+std::to_string(b));}}if(a<b){++out.ap_ge_pb_violations;out.maximum_ge_defect=std::max(out.maximum_ge_defect,b-a);}}
 }
 out.subintertwiner=out.ap_le_pb_violations==0;out.superintertwiner=out.ap_ge_pb_violations==0;
 if(!out.subintertwiner)out.obstruction="atom incidence: AP <= PB fails";return out;
}
}
