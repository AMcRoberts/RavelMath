#include <iostream>
#include <map>
#include <set>
#include <tuple>
#include <vector>
#include <cmath>
#include <stdexcept>
#include "ravel/plastic_number_substitution.hpp"
#include "ravel/substitution.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/corona.hpp"

int main(){
  using namespace ravel;
  SubstitutionRule rule(plastic_number_rule());
  constexpr double beta=1.3247179572447458;
  constexpr double b2=0.8688369618327093;
  ContactBoundaryLimits lim; lim.closure_cap=20000; lim.corona_cap=100000; lim.max_corona_rounds=10;
  auto rep=compute_contact_boundary_from_subst<3>(rule,beta,b2,3,lim);
  if(rep.boundary_size==0) throw std::runtime_error("plastic boundary empty");
  auto subst=make_substitution<3>(rule,beta);
  std::set<SNode<3>> gb;
  for(auto const& t:rep.boundary_nodes){ SNode<3> n; n.i=std::get<0>(t); n.j=std::get<2>(t); auto x=std::get<1>(t); for(int q=0;q<3;++q)n.x[q]=x[q]; gb.insert(n); }
  std::map<std::string,long long> counts;
  std::map<std::tuple<SNode<3>,SNode<3>>,std::string> edgeclass;
  std::map<std::string,std::set<std::pair<long long,long long>>> roles;
  long long total=0;
  for(auto const& src:gb){
    for(auto const& [dst,pq]: simple_forward_targets_exact<3>(subst,src)){
      if(!gb.count(dst)) continue;
      std::array<long long,3> dl{0,0,0},dr{0,0,0};
      for(auto x:pq.first) dl[(size_t)x]++;
      for(auto x:pq.second) dr[(size_t)x]++;
      std::array<long long,3> d{}; for(int z=0;z<3;++z)d[z]=dr[z]-dl[z];
      std::string key=std::to_string(d[0])+","+std::to_string(d[1])+","+std::to_string(d[2]);
      counts[key]++; roles[key].insert({src.i,src.j}); edgeclass[{src,dst}]=key; total++;
    }
  }
  std::cout<<"plastic boundary="<<rep.boundary_size<<" edges="<<total<<" beta="<<rep.boundary_eigenvalue<<" bp="<<rep.bp_rho_nc<<"\n";
  for(auto const& [k,v]:counts) std::cout<<"delta "<<k<<" edges="<<v<<" source_roles="<<roles[k].size()<<"\n";
  if(counts.size()!=3) throw std::runtime_error("expected exactly three signed prefix-difference classes");
  if(!counts.count("0,0,0")||!counts.count("1,0,0")||!counts.count("-1,0,0")) throw std::runtime_error("unexpected plastic classes");
  long long paired=0, unpaired=0, wrong=0;
  for(auto const& [ed,k]:edgeclass){
    auto src=std::get<0>(ed), dst=std::get<1>(ed);
    SNode<3> isrc, idst; isrc.i=src.j; isrc.j=src.i; idst.i=dst.j; idst.j=dst.i;
    for(int z=0;z<3;++z){isrc.x[z]=-src.x[z];idst.x[z]=-dst.x[z];}
    auto it=edgeclass.find({isrc,idst});
    if(it==edgeclass.end()) {unpaired++; continue;}
    paired++;
    std::string expected = k=="1,0,0" ? "-1,0,0" : (k=="-1,0,0" ? "1,0,0" : k);
    if(it->second!=expected) wrong++;
  }
  std::cout<<"involution paired="<<paired<<" unpaired="<<unpaired<<" wrong_class="<<wrong<<"\n";
  if(unpaired==0 && wrong==0) throw std::runtime_error("plastic signed classes unexpectedly collapse to orientation twist");
  std::cout<<"plastic grammar probe PASS\n";
}
