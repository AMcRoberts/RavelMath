#include <filesystem>
#include <iostream>
#include <vector>
#include "ravel/proof/first_return_joint_product.hpp"
using namespace ravel::proof;
int main(){
 for(size_t n: {3ul,4ul}){
  std::vector<std::filesystem::path> fs={"out/cover_tube_defect_splice/n"+std::to_string(n)+"_M2.json"}; if(n==3)fs.push_back("out/cover_tube_defect_splice/n3_M3.json");
  std::vector<FirstReturnWitness> ws;for(auto&f:fs){auto p=load_first_return_witnesses(f);ws.insert(ws.end(),p.begin(),p.end());}
  auto states=enumerate_radius_one_states(n);auto plant=make_symbolic_controller_plant(states);
  size_t any=0; size_t maxpairs=0;
  for(auto&w:ws){auto sfs=detail::target_faces(w.base_path.front(),w.bound);auto tfs=detail::target_faces(w.target,w.bound);size_t good=0;
   for(auto sf:sfs)for(auto tf:tfs){ResidualController r(states.size(),false);for(size_t i=0;i<states.size();++i)if(states[i][tf.first]==tf.second)r[i]=true;for(size_t k=w.digits.size();k-->0;)r=predecessor_controller(plant,r,w.digits[k]);bool all=true;for(size_t i=0;i<states.size();++i)if(states[i][sf.first]==sf.second&&!r[i]){all=false;break;}if(all)++good;}
   if(good)++any;maxpairs=std::max(maxpairs,good);
  }
  std::cout<<"n="<<n<<" any pair total="<<any<<"/"<<ws.size()<<" maxpairs="<<maxpairs<<"\n";
 }
}
