#pragma once
#include <array>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>
#include "ravel/contact_boundary.hpp"
#include "ravel/corona.hpp"
#include "ravel/shift_branch_substitution.hpp"
namespace ravel::proof {
struct QuarticNextPisotThreeGeneratorCertificate {
 std::size_t boundary_states{}; long long edges{};
 std::array<long long,3> class_edges{};
 long long involution_paired{}, involution_unpaired{}, involution_wrong{};
 bool exactly_three{}, genuinely_signed_asymmetric{}, proved{};
};
inline QuarticNextPisotThreeGeneratorCertificate derive_quartic_next_pisot_three_generator(){
  SubstitutionRule rule(quartic_next_pisot_rule());
  constexpr double beta=1.3802775690976141, b2=0.884419273294316;
  ContactBoundaryLimits lim; lim.closure_cap=100000; lim.corona_cap=300000; lim.max_corona_rounds=12;
  auto rep=compute_contact_boundary_from_subst<4>(rule,beta,b2,4,lim);
  auto subst=make_substitution<4>(rule,beta);
  std::set<SNode<4>> gb;
  for(auto const&t:rep.boundary_nodes){SNode<4> n;n.i=std::get<0>(t);n.j=std::get<2>(t);auto x=std::get<1>(t);for(int q=0;q<4;++q)n.x[q]=x[q];gb.insert(n);}
  std::map<std::tuple<SNode<4>,SNode<4>>,int> cls;
  QuarticNextPisotThreeGeneratorCertificate c;c.boundary_states=gb.size();
  for(auto const&src:gb)for(auto const&[dst,pq]:simple_forward_targets_exact<4>(subst,src))if(gb.count(dst)){
    auto ab=[](auto const&w){std::array<long long,4>a{};for(auto z:w)++a[(std::size_t)z];return a;};
    auto l=ab(pq.first),r=ab(pq.second);std::array<long long,4>d{};for(int z=0;z<4;++z)d[z]=r[z]-l[z];
    int g;if(d==std::array<long long,4>{0,0,0,0})g=0;else if(d==std::array<long long,4>{1,0,0,0})g=1;else if(d==std::array<long long,4>{-1,0,0,0})g=2;else throw std::runtime_error("quartic prefix defect outside three classes");
    ++c.class_edges[(std::size_t)g];++c.edges;cls[{src,dst}]=g;
  }
  c.exactly_three=c.class_edges[0]&&c.class_edges[1]&&c.class_edges[2];
  for(auto const&[ed,g]:cls){auto s=std::get<0>(ed),d=std::get<1>(ed);SNode<4>si,di;si.i=s.j;si.j=s.i;di.i=d.j;di.j=d.i;for(int z=0;z<4;++z){si.x[z]=-s.x[z];di.x[z]=-d.x[z];}auto it=cls.find({si,di});if(it==cls.end())++c.involution_unpaired;else{++c.involution_paired;if((g==1&&it->second!=2)||(g==2&&it->second!=1)||(g==0&&it->second!=0))++c.involution_wrong;}}
  c.genuinely_signed_asymmetric=c.involution_unpaired>0;
  c.proved=c.boundary_states>0&&c.exactly_three&&c.involution_wrong==0;
  return c;
}
}
