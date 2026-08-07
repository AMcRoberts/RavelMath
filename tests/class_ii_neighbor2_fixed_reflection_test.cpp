// An eighth Class-II reflection connection: the fixed 24-state
// neighbor-2 correction table (`class_ii_neighbor2_fixed_extension_
// states()`, include/ravel/class_ii_neighbor_family.hpp) matches
// lean/class_ii_neighbor2_extensions.lean's neighbor2FixedNode exactly
// (already independently verified at runtime, entry-by-entry, by the
// pre-existing tests/lean_class_ii_catalogue_cross_check_test.cpp).
// Threads the concrete 24 nodes C++ actually built; the renderer
// checks membership against neighbor2FixedNodeG via decide.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_neighbor2_fixed_batch");
    std::set<ravel::SNode<3>> fixed;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        fixed = ravel::class_ii_neighbor2_fixed_extension_states();
    }
    std::cout << "neighbor2 fixed extension state count: " << fixed.size() << "\n";
    assert(fixed.size() == 24);

    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIFixedTableCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes[0].second->table == "neighbor2_fixed");
    assert(nodes[0].second->nodes.size() == 24);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_fixed_table_instance_0") != std::string::npos);
    assert(lean.find("neighbor2FixedNodeG") != std::string::npos);

    std::ofstream out("/tmp/class_ii_neighbor2_fixed_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_neighbor2_fixed_reflection_test: PASS\n";
    return 0;
}
