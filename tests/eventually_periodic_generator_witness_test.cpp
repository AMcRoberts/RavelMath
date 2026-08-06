#include <cassert>
#include <iostream>
#include "ravel/proof/eventually_periodic_generator_witness.hpp"
int main(){
 auto c = ravel::proof::derive_eventually_periodic_generator_witness();
 std::cout << "boundary_states=" << c.boundary_states << " boundary_edges=" << c.boundary_edges
           << " universal_edges=" << c.universal_edges << " obstruction=\"" << c.obstruction << "\"\n";
 std::cout << "boundary_realizes_defect_plus2=" << c.boundary_realizes_defect_plus2
           << " boundary_realizes_defect_minus2=" << c.boundary_realizes_defect_minus2 << "\n";
 std::cout << "generator_intertwines: ";
 for (std::size_t g=0; g<5; ++g) std::cout << (long long)g-2 << "=" << c.generator_intertwines[g] << " ";
 std::cout << "\n";
 std::cout << "simultaneous_intertwiner=" << c.simultaneous_intertwiner
           << " finite_positive_grammar_ready=" << c.finite_positive_grammar_ready
           << " proved=" << c.proved << "\n";
 assert(c.boundary_realizes_defect_plus2);
 assert(c.boundary_realizes_defect_minus2);
 assert(c.proved);
 std::cout << "PASS: x^3-2x^2-x+1 (first eventually-periodic witness examined) also has a "
              "genuine, concretely-realized fourth/fifth generator -- confirming this isn't "
              "specific to the terminating-expansion mechanism.\n";
}
