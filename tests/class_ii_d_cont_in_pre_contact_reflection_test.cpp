// A twentieth Class-II reflection connection, zero new C++ cost
// (same reuse pattern Finding 26 established): reuses the ALREADY-
// RECORDED "d_cont" ClassIIFixedTableCertificate data (class_ii_d_
// cont_set() was already instrumented for the fixed-table connection)
// for a SECOND consequence -- lean/class_ii_affine_shells.lean also
// proves dContNode_in_preContact (every D_cont seed is a pre-contact
// node, for all nine DContKind cases). Checks the same containment
// fact directly against the concrete C++-built nodes.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_d_cont_in_pre_contact_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::class_ii_d_cont_set();
    }

    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIFixedTableCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes[0].second->table == "d_cont");
    assert(nodes[0].second->nodes.size() == 9);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_d_cont_in_pre_contact_instance_0") != std::string::npos);
    // The pre-existing fixed-table membership check should ALSO still
    // fire from the same trace data (this test doesn't remove it).
    assert(lean.find("class_ii_fixed_table_instance_0") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_d_cont_in_pre_contact_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_d_cont_in_pre_contact_reflection_test: PASS\n";
    return 0;
}
