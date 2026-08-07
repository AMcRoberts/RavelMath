// A fifth Class-II reflection connection: class_ii_d_cont_face_candidates
// is an ANALYTIC geometric construction (not a hardcoded table) that
// was verified, entry-by-entry, to produce exactly the 33-state table
// lean/class_ii_affine_shells.lean's dContFaceCandidateNode already
// proves a window-validity iff about.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_face_candidate_batch");
    std::set<ravel::ANode<3>> candidates;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        candidates = ravel::class_ii_d_cont_face_candidates();
    }
    std::cout << "face candidate count: " << candidates.size() << "\n";
    assert(candidates.size() == 33);

    auto lemmas = trace.find<mathlib::reflection::LemmaApplication>();
    bool found_iff = false, found_x0 = false;
    for (auto& [id, l] : lemmas) {
        (void)id;
        if (l->theorem_name == "class_ii_dCont_face_candidate_valid_iff") found_iff = true;
        if (l->theorem_name == "class_ii_rawContact_x0_bounded") found_x0 = true;
    }
    assert(found_iff && found_x0);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_dCont_face_candidate_valid_iff") != std::string::npos);
    assert(lean.find("class_ii_rawContact_x0_bounded") != std::string::npos);

    std::ofstream out("/tmp/class_ii_face_candidate_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_face_candidate_citation_test: PASS\n";
    return 0;
}
