#include <cassert>
#include <iostream>
#include "ravel/proof/shift_branch_three_generator_continuation.hpp"
int main(){
  for(std::size_t D=2;D<=256;++D){
    auto c=ravel::proof::derive_shift_branch_three_generator_continuation(D);
    assert(c.proved); assert(c.parent_occurrences==D+1);
    assert(c.ordered_prefix_pair_counts[1]==D && c.ordered_prefix_pair_counts[2]==D);
    assert(c.ordered_prefix_pair_counts[0]+2*D==(D+1)*(D+1));
  }
  std::cout<<"shift-branch three-generator continuation PASS\n";
}
