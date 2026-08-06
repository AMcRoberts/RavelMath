#pragma once
#include <array>
#include <map>
#include <vector>
#include "ravel/proof/quartic_next_pisot_three_generator.hpp"
#include "ravel/proof/condition_f_pair_boundary_substitution.hpp"
namespace ravel::proof {
struct QuarticNextPisotIntertwinerCertificate {
 std::size_t boundary_states{}; long long boundary_edges{}, universal_edges{};
 std::array<bool,3> generator_intertwines{};
 bool every_boundary_edge_has_universal_witness{}, proved{};
};
inline QuarticNextPisotIntertwinerCertificate derive_quartic_next_pisot_intertwiner(){
 using NM=NonnegativeMatrix; QuarticNextPisotIntertwinerCertificate o;
 SubstitutionRule rule(quartic_next_pisot_rule()); constexpr double beta=1.3802775690976141,b2=0.884419273294316;
 ContactBoundaryLimits lim;lim.closure_cap=100000;lim.corona_cap=300000;lim.max_corona_rounds=12;
 auto rep=compute_contact_boundary_from_subst<4>(rule,beta,b2,4,lim);auto subst=make_substitution<4>(rule,beta);
 std::vector<SNode<4>> b;std::map<SNode<4>,std::size_t> bi;
 for(auto const&t:rep.boundary_nodes){SNode<4>n;n.i=std::get<0>(t);n.j=std::get<2>(t);auto x=std::get<1>(t);for(int q=0;q<4;++q)n.x[q]=x[q];bi.emplace(n,b.size());b.push_back(n);}o.boundary_states=b.size();
 auto zero=[](std::size_t n,std::size_t m){return NM(n,std::vector<long long>(m,0));};
 std::array<NM,3> B{zero(b.size(),b.size()),zero(b.size(),b.size()),zero(b.size(),b.size())};
 std::array<NM,3> U{zero(16,16),zero(16,16),zero(16,16)};
 auto role=[](long long i,long long j){return (std::size_t)i*4+(std::size_t)j;};
 auto gen=[](auto const&l,auto const&r){std::array<long long,4>d{};for(auto z:r)++d[(std::size_t)z];for(auto z:l)--d[(std::size_t)z];if(d==std::array<long long,4>{0,0,0,0})return 0;if(d==std::array<long long,4>{1,0,0,0})return 1;if(d==std::array<long long,4>{-1,0,0,0})return 2;throw std::runtime_error("quartic defect outside three classes");};
 std::map<std::tuple<std::size_t,std::size_t,int>,long long>w;
 for(long long i=0;i<4;++i)for(long long j=0;j<4;++j){auto lp=parent_decompositions<4>(subst.images,i),rp=parent_decompositions<4>(subst.images,j);for(auto const&a:lp)for(auto const&c:rp){int g=gen(a.p,c.p);auto s=role(i,j),t=role(a.parent_letter,c.parent_letter);++U[(std::size_t)g][s][t];++w[{s,t,g}];++o.universal_edges;}}
 o.every_boundary_edge_has_universal_witness=true;
 for(std::size_t s=0;s<b.size();++s)for(auto const&[dst,pq]:simple_forward_targets_exact<4>(subst,b[s])){auto it=bi.find(dst);if(it==bi.end())continue;int g=gen(pq.first,pq.second);++B[(std::size_t)g][s][it->second];++o.boundary_edges;if(!w.count({role(b[s].i,b[s].j),role(dst.i,dst.j),g}))o.every_boundary_edge_has_universal_witness=false;}
 NM P=zero(b.size(),16);for(std::size_t s=0;s<b.size();++s)P[s][role(b[s].i,b[s].j)]=1;
 for(std::size_t g=0;g<3;++g){auto lhs=condition_f_pair_boundary_detail::rectangular_product(B[g],P);auto rhs=condition_f_pair_boundary_detail::rectangular_product(P,U[g]);o.generator_intertwines[g]=condition_f_pair_boundary_detail::leq(lhs,rhs);}
 o.proved=o.boundary_states>0&&o.every_boundary_edge_has_universal_witness&&o.generator_intertwines[0]&&o.generator_intertwines[1]&&o.generator_intertwines[2];return o;
}
}
