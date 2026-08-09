// A thirteenth/fourteenth Class-II reflection connection:
// class_ii_pre_contact_first_backward_layer()/_second_backward_
// layer() (include/ravel/class_ii_boundary_family.hpp) match
// lean/class_ii_affine_shells.lean's firstBackwardNode/
// secondBackwardNode exactly (already independently cross-checked at
// runtime by tests/lean_class_ii_catalogue_cross_check_test.cpp,
// entry [7]). Threads the concrete nodes; the renderer decides
// membership/equality against the Lean definitions.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_backward_layers_batch");
    std::set<ravel::ANode<3>> first, second;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        first = ravel::class_ii_pre_contact_first_backward_layer();
        second = ravel::class_ii_pre_contact_second_backward_layer();
    }
    assert(first.size() == 6);
    assert(second.size() == 1);

    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIFixedTableCertificate nodes\n";
    assert(nodes.size() == 2);
    assert(nodes[0].second->table == "first_backward");
    assert(nodes[1].second->table == "second_backward");

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("firstBackwardNodeG") != std::string::npos);
    assert(lean.find("secondBackwardNodeG") != std::string::npos);
    assert(lean.find("class_ii_fixed_table_instance_0") != std::string::npos);
    assert(lean.find("class_ii_fixed_table_instance_1") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_backward_layers_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_backward_layers_reflection_test: PASS\n";
    return 0;
}
