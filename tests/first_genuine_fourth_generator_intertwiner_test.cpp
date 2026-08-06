#include <cassert>
#include <iostream>
#include "ravel/proof/first_genuine_fourth_generator_intertwiner.hpp"
int main(){
 auto c = ravel::proof::derive_first_genuine_fourth_generator_intertwiner();
 std::cout << "boundary_states=" << c.boundary_states << " boundary_edges=" << c.boundary_edges
           << " universal_edges=" << c.universal_edges << " obstruction=\"" << c.obstruction << "\"\n";
 std::cout << "boundary_realizes_defect_plus2=" << c.boundary_realizes_defect_plus2
           << " boundary_realizes_defect_minus2=" << c.boundary_realizes_defect_minus2 << "\n";
 std::cout << "generator_intertwines: ";
 for (std::size_t g=0; g<5; ++g) std::cout << (long long)g-2 << "=" << c.generator_intertwines[g] << " ";
 std::cout << "\n";
 std::cout << "parent_catalogue_complete=" << c.parent_catalogue_complete
           << " every_boundary_edge_has_universal_witness=" << c.every_boundary_edge_has_universal_witness
           << " base_role_projection_exact=" << c.base_role_projection_exact
           << " simultaneous_five_generator_intertwiner=" << c.simultaneous_five_generator_intertwiner
           << " finite_positive_grammar_ready=" << c.finite_positive_grammar_ready
           << " proved=" << c.proved << "\n";
 assert(c.boundary_realizes_defect_plus2);
 assert(c.boundary_realizes_defect_minus2);
 assert(c.proved);
 std::cout << "PASS: x^3-2x^2-2's fourth and fifth generators are realized as genuine "
              "edges in the concrete contact-boundary graph, not just the abstract "
              "universal-role catalogue.\n";
}
