#include <cassert>
#include <fstream>
#include <iostream>
#include "math/proof_reflection.hpp"
#include "ravel/proof/supergolden_three_generator_intertwiner.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"
int main(){
 mathlib::reflection::Trace trace("supergolden_three_generator_intertwiner_batch");
 mathlib::reflection::ScopedTrace scope(&trace);

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
 ravel::proof::stage_supergolden_three_generator_intertwiner(
     c, "supergolden number's own 9-role boundary/universal Q/R/S intertwiner");

 auto nodes = trace.find<mathlib::reflection::ThreeGeneratorIntertwinerFamilyReflectionCertificate>();
 assert(nodes.size() == 1);

 std::string lean = ravel::proof::render_reflective_lean_module(trace);
 assert(lean.find("supergolden_word_intertwiner") != std::string::npos);

 std::ofstream out("lean/generated/supergolden_three_generator_intertwiner.lean");
 out << lean;
 out.close();

 std::cout << "supergolden three-generator intertwiner PASS\n";
}
