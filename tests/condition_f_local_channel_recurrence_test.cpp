#include <cassert>
#include <iostream>
#include "ravel/proof/condition_f_local_channel_recurrence.hpp"
using namespace ravel::proof;
int main(){
  for(std::size_t D=2;D<=64;++D){
    auto p=derive_condition_f_local_channel_recurrence(D);
    assert(p.proved);
    // New channels: (0,0) gains 2D+1 parent pairs; each of the
    // 2(D-1) one-zero ordered inner pairs gains one channel.
    assert(p.new_boundary_channels.size()==(2*D+1)+2*(D-1));
    for(const auto&e:p.new_boundary_channels){
      assert(e.left_parent==D || e.right_parent==D);
      if(e.left_parent==D) assert(e.left_inner==0);
      if(e.right_parent==D) assert(e.right_inner==0);
    }
  }
  std::cout<<"Condition-F local channel recurrence PASS\n";
}
