#include <cassert>
#include <iostream>
#include "ravel/proof/condition_f_boundary_interface.hpp"
using namespace ravel::proof;
int main(){
 for(std::size_t D=2;D<=256;++D){
   auto p=derive_condition_f_boundary_interface(D);
   assert(p.proved);
   assert(p.balanced_corner_fan==2*D+1);
   assert(p.positive_residual_strip==D-1);
   assert(p.negative_residual_strip==D-1);
   assert(p.channels.size()==4*D-1);
 }
 std::cout<<"Condition-F boundary interface PASS\n";
}
