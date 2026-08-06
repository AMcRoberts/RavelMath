#include <filesystem>
#include <iostream>
#include <vector>
#include "ravel/proof/first_return_joint_product.hpp"
using namespace ravel::proof;
int main(){
 for(size_t n: {3ul,4ul}){
  std::vector<std::filesystem::path> fs={"out/cover_tube_defect_splice/n"+std::to_string(n)+"_M2.json"};
  if(n==3) fs.push_back("out/cover_tube_defect_splice/n3_M3.json");
  std::vector<FirstReturnWitness> ws; for(auto&f:fs){auto p=load_first_return_witnesses(f);ws.insert(ws.end(),p.begin(),p.end());}
  auto states=enumerate_radius_one_states(n); auto plant=make_symbolic_controller_plant(states);
  size_t ok=0, total=0, worst=0; FirstReturnWitness bad;
  for(auto&w:ws){
   auto tf=detail::target_faces(w.target,w.bound); ResidualController r(states.size(),false);
   for(size_t i=0;i<states.size();++i) for(auto [c,s]:tf) if(states[i][c]==s){r[i]=true;break;}
   for(size_t k=w.digits.size();k-->0;) r=predecessor_controller(plant,r,w.digits[k]);
   auto sf=detail::target_faces(w.base_path.front(),w.bound);
   bool all=true;size_t misses=0, sources=0;
   for(size_t i=0;i<states.size();++i){bool aligned=false;for(auto[c,s]:sf)if(states[i][c]==s){aligned=true;break;} if(aligned){++sources;if(!r[i]){all=false;++misses;}}}
   ++total;if(all)++ok; if(misses>worst){worst=misses;bad=w;}
  }
  std::cout<<"n="<<n<<" total="<<total<<" all_source_total="<<ok<<" worstmiss="<<worst<<"\n";
  if(ok<total){std::cout<<"bad word len="<<bad.digits.size()<<" src";for(auto x:bad.base_path.front())std::cout<<' '<<x;std::cout<<" tgt";for(auto x:bad.target)std::cout<<' '<<x;std::cout<<"\n";}
 }
}
