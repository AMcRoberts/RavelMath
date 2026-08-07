// A second Class-II reflection connection, same shape as
// class_ii_shell_citation_test.cpp: class_ii_contact_set matches
// contactNode's 14-state table exactly, and lean/class_ii_affine_shells.lean
// already proves class_ii_contactNode_valid -- window validity for
// ANY a>=2 and its actual Perron root, not a per-a check.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_contact_batch");
    std::set<ravel::SNode<3>> contact;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        contact = ravel::class_ii_contact_set();
    }
    std::cout << "contact set size: " << contact.size() << "\n";
    assert(contact.size() == 14);

    auto lemmas = trace.find<mathlib::reflection::LemmaApplication>();
    bool found = false;
    for (auto& [id, l] : lemmas) { (void)id; if (l->theorem_name == "class_ii_contactNode_valid") found = true; }
    assert(found);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_contactNode_valid") != std::string::npos);
    assert(lean.find("class_ii_perron_gap_lt_one") != std::string::npos);

    std::ofstream out("/tmp/class_ii_contact_citation_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_contact_citation_test: PASS\n";
    return 0;
}
