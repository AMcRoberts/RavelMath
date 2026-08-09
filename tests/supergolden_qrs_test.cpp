#include <cassert>
#include <iostream>
#include "ravel/proof/supergolden_qrs_closure.hpp"
int main(){
 auto c=ravel::proof::derive_supergolden_qrs_closure();
 assert(c.proved);
 assert((c.greedy_digits == std::vector<long long>{1,0,1}));
 assert(c.alphabet_size==3);
 assert(c.total_parent_decompositions==4);
 assert(c.distinct_prefixes==2);
 assert(c.defect_classes==3);
 assert(c.universal_role_states==9);
 assert(c.incidence_polynomial_matches_minpoly_exactly);
 assert(c.incidence_cyclotomic.proved);
 assert(c.incidence_has_no_cyclotomic_factor);
 assert(c.no_fourth_generator);
 assert(c.contact_boundary_complete);
 assert(c.simultaneous_three_generator_intertwiner);
 assert(c.boundary_states > 0);
 std::cout<<"supergolden Q/R/S closure PASS parents="<<c.total_parent_decompositions
          <<" role_edges="<<c.universal_parent_pair_edges
          <<" classes="<<c.defect_class_edges[0]<<","<<c.defect_class_edges[1]<<","<<c.defect_class_edges[2]<<"\n";
}
