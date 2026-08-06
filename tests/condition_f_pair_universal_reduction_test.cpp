#include <cassert>
#include <iostream>
#include "ravel/proof/condition_f_pair_universal_reduction.hpp"
using namespace ravel::proof;
int main(){
  for(std::size_t n=2;n<=128;++n){
    auto p=derive_condition_f_pair_universal_reduction(n);
    assert(p.proved);
    assert(p.voltage_twists_spectrally_eliminated);
    assert(p.old_dimension_block_transported);
    assert(p.terminal_boundary_formula_derived);
    assert(p.boundary_pair_substitution_derived);
   assert(p.joint_pair_order_preserved_by_boundary);
   assert(p.only_base_pair_comparison_remains);
   assert(p.boundary_substitution.excursions.size()==8*n-1);
  }
  std::cout<<"Condition-F pair universal reduction PASS\n";
}
