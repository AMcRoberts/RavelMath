#include <cassert>
#include <iostream>
#include "ravel/proof/supergolden_three_generator_intertwiner.hpp"
int main(){
 auto c=ravel::proof::derive_supergolden_three_generator_intertwiner();
 std::cout<<"boundary_states="<<c.boundary_states<<" boundary_edges="<<c.boundary_edges
          <<" universal_edges="<<c.universal_edges<<" obstruction=\""<<c.obstruction<<"\"\n";
 std::cout<<"parent_catalogue_complete="<<c.parent_catalogue_complete
          <<" every_boundary_edge_has_universal_witness="<<c.every_boundary_edge_has_universal_witness
          <<" base_role_projection_exact="<<c.base_role_projection_exact
          <<" g0="<<c.g0_intertwines<<" g+="<<c.gplus_intertwines<<" g-="<<c.gminus_intertwines
          <<" finite_positive_grammar_ready="<<c.finite_positive_grammar_ready
          <<" proved="<<c.proved<<"\n";
 assert(c.proved);
}
