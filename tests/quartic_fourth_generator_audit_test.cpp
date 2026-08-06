#include <cassert>
#include <iostream>
#include "ravel/proof/quartic_fourth_generator_audit.hpp"
int main(){
 auto c=ravel::proof::derive_quartic_fourth_generator_audit();
 assert(c.proved);
 assert(!c.genuine_fourth_generator_found);
 std::cout<<"quartic fourth-generator audit PASS states="<<c.boundary_states
          <<" edges="<<c.boundary_edges<<" raw="
          <<c.ordered_prefix_pair_edges[0]<<","<<c.ordered_prefix_pair_edges[1]<<","
          <<c.ordered_prefix_pair_edges[2]<<","<<c.ordered_prefix_pair_edges[3]<<"\n";
}
