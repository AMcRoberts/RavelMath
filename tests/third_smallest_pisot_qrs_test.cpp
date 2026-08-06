#include <cassert>
#include <iostream>
#include "ravel/proof/third_smallest_pisot_qrs_closure.hpp"
int main(){
 auto c=ravel::proof::derive_third_smallest_pisot_qrs_closure();
 assert(c.proved);
 assert(c.greedy_digits == std::vector<int>({1,0,0,1,0,0,1}));
 assert(c.distinct_prefixes==2);
 assert(c.defect_classes==3);
 assert(c.universal_role_states==49);
 assert(c.parry_factorization_exact);
 assert(c.no_fourth_or_fifth_generator);
 std::cout<<"third-smallest Pisot Q/R/S closure PASS parents="<<c.total_parent_decompositions
          <<" role_edges="<<c.universal_parent_pair_edges
          <<" classes="<<c.defect_class_edges[0]<<","<<c.defect_class_edges[1]<<","<<c.defect_class_edges[2]<<"\n";
}
