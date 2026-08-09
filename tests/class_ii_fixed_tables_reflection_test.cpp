// Findings 1-16 (Class-II thread), original per-instance pattern:
// class_ii_contact_set, class_ii_pre_contact_set, class_ii_d_cont_set,
// and class_ii_d_cont_face_candidates now thread their ACTUAL
// concrete node lists into ClassIIFixedTableCertificate nodes. The
// renderer decides membership of THOSE EXACT nodes against the
// corresponding Lean table -- if the C++ and Lean tables ever
// diverged, the kernel check would legitimately fail; nothing is
// keyed by a citation name alone.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_fixed_tables_batch");
    std::set<ravel::SNode<3>> contact;
    std::set<ravel::ANode<3>> pre_contact, d_cont, face_candidates;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        contact = ravel::class_ii_contact_set();
        pre_contact = ravel::class_ii_pre_contact_set();
        d_cont = ravel::class_ii_d_cont_set();
        face_candidates = ravel::class_ii_d_cont_face_candidates();
    }
    std::cout << "contact=" << contact.size() << " pre_contact=" << pre_contact.size()
              << " d_cont=" << d_cont.size() << " face_candidates=" << face_candidates.size() << "\n";
    assert(contact.size() == 14);
    assert(pre_contact.size() == 16);
    assert(d_cont.size() == 9);
    assert(face_candidates.size() == 33);

    auto nodes = trace.find<mathlib::reflection::ClassIIFixedTableCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIFixedTableCertificate nodes\n";
    assert(nodes.size() == 4);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_fixed_table_instance_0") != std::string::npos);
    assert(lean.find("class_ii_fixed_table_instance_3") != std::string::npos);
    assert(lean.find("contactNodeG") != std::string::npos);
    assert(lean.find("preContactNodeG") != std::string::npos);
    assert(lean.find("dContNodeG") != std::string::npos);
    assert(lean.find("dContFaceCandidateNodeG") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_fixed_tables_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_fixed_tables_reflection_test: PASS\n";
    return 0;
}
