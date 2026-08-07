// Two more Class-II reflection connections, mechanical repeats of the
// same pattern: class_ii_d_cont_set (9 states, matches DContKind) and
// class_ii_pre_contact_set (16 states, matches PreContactKind).

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_precontact_dcont_batch");
    std::set<ravel::ANode<3>> dcont, precontact;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        dcont = ravel::class_ii_d_cont_set();
        precontact = ravel::class_ii_pre_contact_set();
    }
    std::cout << "d_cont size: " << dcont.size() << ", pre_contact size: " << precontact.size() << "\n";
    assert(dcont.size() == 9);
    assert(precontact.size() == 16);

    auto lemmas = trace.find<mathlib::reflection::LemmaApplication>();
    bool found_d = false, found_p = false;
    for (auto& [id, l] : lemmas) {
        (void)id;
        if (l->theorem_name == "dContNode_in_preContact") found_d = true;
        if (l->theorem_name == "preContactNode_partition") found_p = true;
    }
    assert(found_d && found_p);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("dContNode_in_preContact") != std::string::npos);
    assert(lean.find("preContactNode_partition") != std::string::npos);

    std::ofstream out("/tmp/class_ii_precontact_dcont_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_precontact_dcont_citation_test: PASS\n";
    return 0;
}
