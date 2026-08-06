#pragma once
#include <array>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>
#include "ravel/contact_boundary.hpp"
#include "ravel/plastic_number_substitution.hpp"
#include "ravel/corona.hpp"
namespace ravel::proof {
struct PlasticThreeGeneratorCertificate {
  std::size_t boundary_states{}; long long edges{};
  long long neutral_edges{}, positive_edges{}, negative_edges{};
  long long involution_paired{}, involution_unpaired{}, involution_wrong_class{};
  bool exactly_three_classes{}, signed_classes_globally_inequivalent{};
};
inline PlasticThreeGeneratorCertificate derive_plastic_three_generator_grammar(){
  ravel::SubstitutionRule rule(ravel::plastic_number_rule());
  constexpr double beta=1.3247179572447458, b2=0.8688369618327093;
  ravel::ContactBoundaryLimits lim; lim.closure_cap=20000; lim.corona_cap=100000; lim.max_corona_rounds=10;
  auto rep=ravel::compute_contact_boundary_from_subst<3>(rule,beta,b2,3,lim);
  auto subst=ravel::make_substitution<3>(rule,beta);
  std::set<ravel::SNode<3>> gb;
  for(auto const& t:rep.boundary_nodes){ ravel::SNode<3> n; n.i=std::get<0>(t);n.j=std::get<2>(t);auto x=std::get<1>(t);for(int q=0;q<3;++q)n.x[q]=x[q];gb.insert(n);}
  std::map<std::tuple<ravel::SNode<3>,ravel::SNode<3>>,int> cls;
  PlasticThreeGeneratorCertificate c; c.boundary_states=gb.size();
  for(auto const& src:gb) for(auto const& [dst,pq]:ravel::simple_forward_targets_exact<3>(subst,src)) if(gb.count(dst)){
    long long l=0,r=0; for(auto x:pq.first) if(x==0)++l; for(auto x:pq.second) if(x==0)++r; long long d=r-l;
    if(d==0){c.neutral_edges++;cls[{src,dst}]=0;} else if(d==1){c.positive_edges++;cls[{src,dst}]=1;} else if(d==-1){c.negative_edges++;cls[{src,dst}]=-1;} else throw std::runtime_error("plastic prefix defect outside {-1,0,1}"); c.edges++;
  }
  c.exactly_three_classes=c.neutral_edges&&c.positive_edges&&c.negative_edges;
  for(auto const& [ed,k]:cls){auto s=std::get<0>(ed),d=std::get<1>(ed);ravel::SNode<3> si,di;si.i=s.j;si.j=s.i;di.i=d.j;di.j=d.i;for(int z=0;z<3;++z){si.x[z]=-s.x[z];di.x[z]=-d.x[z];}auto it=cls.find({si,di});if(it==cls.end())c.involution_unpaired++;else{c.involution_paired++;if(it->second!=-k)c.involution_wrong_class++;}}
  c.signed_classes_globally_inequivalent=c.involution_unpaired>0;
  return c;
}
}
