#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "ravel/ambient_graph.hpp"
#include "ravel/proof/monotone_profile_corridor_closure.hpp"
#include "ravel/simple_parry_profile.hpp"
using namespace ravel;

template<std::size_t D> void cross(std::size_t k) {
  const auto p=proof::derive_monotone_profile_corridor_closure(D,k);
  if(!p.proved || !p.only_qr_words || !p.boundary_is_deletion_only || !p.parent_role_suspension_intertwiner)
    throw std::runtime_error("symbolic closure failed");
  if(p.zero_parent_occurrences!=D+k || p.total_parent_occurrences!=2*D+k-1)
    throw std::runtime_error("count identity failed");
  const auto raw=simple_parry_profile_rule(p.profile);
  std::array<std::vector<long long>,D> images;
  for(std::size_t i=0;i<D;++i) images[i]=std::vector<long long>(raw[i].begin(),raw[i].end());
  std::size_t total=0;
  for(std::size_t inner=0;inner<D;++inner) {
    const auto got=parent_decompositions<D>(images,(long long)inner);
    const auto exp=proof::monotone_profile_parent_choices(D,k,inner);
    if(got.size()!=exp.size()) throw std::runtime_error("catalogue cardinality mismatch");
    for(std::size_t j=0;j<got.size();++j) {
      if((std::size_t)got[j].parent_letter!=exp[j].parent || got[j].p.size()!=exp[j].cut)
        throw std::runtime_error("catalogue mismatch");
      for(auto x:got[j].p) if(x!=0) throw std::runtime_error("nonzero prefix");
    }
    total+=got.size();
  }
  if(total!=p.total_parent_occurrences) throw std::runtime_error("total mismatch");
}
int main(){
 for(std::size_t D=2;D<=256;++D) for(std::size_t k=0;k<D;++k){
   auto p=proof::derive_monotone_profile_corridor_closure(D,k);
   if(!p.proved) throw std::runtime_error("range closure failed");
 }
 for(std::size_t k=0;k<2;++k) cross<2>(k);
 for(std::size_t k=0;k<3;++k) cross<3>(k);
 for(std::size_t k=0;k<4;++k) cross<4>(k);
 for(std::size_t k=0;k<5;++k) cross<5>(k);
 for(std::size_t k=0;k<6;++k) cross<6>(k);
 for(std::size_t k=0;k<7;++k) cross<7>(k);
 for(std::size_t k=0;k<8;++k) cross<8>(k);
 std::cout<<"monotone profile corridor closure PASS\n";
}
