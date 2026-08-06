#include <cassert>
#include <iostream>
#include "ravel/proof/condition_f_joint_pair_comparison.hpp"
using namespace ravel::proof;
int main(){
  for(std::size_t D=2;D<=128;++D){
    auto p=derive_condition_f_joint_pair_comparison(D);
    assert(p.proved);
    assert(p.base_scc_count==1);
    assert(p.base_identity_joint_intertwiner);
    assert(p.joint_order_propagates_dimensionwise);
  }
  std::cout<<"Condition-F joint pair comparison PASS\n";
}
