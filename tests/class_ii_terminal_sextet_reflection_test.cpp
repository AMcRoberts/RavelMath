// A fifteenth Class-II reflection connection: `class_ii_neighbor2_
// terminal_affine_states(a)` (include/ravel/class_ii_neighbor_
// family.hpp) matches lean/class_ii_neighbor2_extensions.lean's
// GENERAL (in a) `neighbor2TerminalSextet` exactly (already
// independently cross-checked at runtime for a in [3,40] by
// tests/lean_class_ii_catalogue_cross_check_test.cpp, entry [5]).
// Threads the concrete 6-node list at two different `a`; the renderer
// decides list equality against the Lean function evaluated at that
// exact `a`.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_terminal_sextet_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::class_ii_neighbor2_terminal_affine_states(3);
        ravel::class_ii_neighbor2_terminal_affine_states(15);
    }

    auto nodes = trace.find<mathlib::reflection::ClassIITerminalSextetCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIITerminalSextetCertificate nodes\n";
    assert(nodes.size() == 2);
    assert(nodes[0].second->a == 3);
    assert(nodes[1].second->a == 15);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_terminal_sextet_instance_0") != std::string::npos);
    assert(lean.find("class_ii_terminal_sextet_instance_1") != std::string::npos);
    assert(lean.find("neighbor2TerminalSextetG") != std::string::npos);

    std::ofstream out("/tmp/class_ii_terminal_sextet_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_terminal_sextet_reflection_test: PASS\n";
    return 0;
}
