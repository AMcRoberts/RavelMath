#include <cassert>
#include <iostream>
#include "ravel/proof/universal_dominance_reduction.hpp"
#include "ravel/proof/finite_quotient_core_maximality.hpp"
using namespace ravel::proof;
int main(){
  auto open=compose_universal_dominance_from_finite_quotient(9,true,true,false);
  assert(!open.equality_derived);
  auto q=derive_finite_quotient_core_maximality({{1,1},{1,0}}, {{{1}}}, 32);
  auto closed=compose_universal_dominance_from_finite_quotient(9,true,true,q);
  assert(closed.equality_derived);
  auto pair=derive_condition_f_joint_pair_comparison(32);
  auto pair_closed=compose_universal_dominance_from_finite_quotient(32,true,true,pair);
  assert(pair_closed.equality_derived);
  std::cout<<"universal dominance reduction PASS\n";
}
