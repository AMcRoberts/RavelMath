// A seventeenth Class-II reflection connection: `class_ii_neighbor2_
// interior_tip(r)` (extracted this session from `class_ii_neighbor2_
// interior_extension_states(round)`'s inline construction, include/
// ravel/class_ii_neighbor_family.hpp) matches lean/class_ii_
// neighbor2_extensions.lean's GENERAL (in r) `neighbor2InteriorTip`
// exactly (already independently cross-checked at runtime for r in
// [2,40] by tests/lean_class_ii_catalogue_cross_check_test.cpp, entry
// [5]'s "interior tip" line). Threads the concrete node at two
// different `r`; the renderer decides equality against the Lean
// function evaluated at that exact `r`.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_interior_tip_batch");
    ravel::SNode<3> tip2, tip20;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        tip2 = ravel::class_ii_neighbor2_interior_tip(2);
        tip20 = ravel::class_ii_neighbor2_interior_tip(20);
    }
    assert((tip2 == ravel::SNode<3>{2, {-2, 2, -1}, 0}));
    assert((tip20 == ravel::SNode<3>{2, {-20, 20, -1}, 0}));

    // Regression: class_ii_neighbor2_interior_extension_states (which
    // now calls the extracted function) still returns 24+1=25.
    assert(ravel::class_ii_neighbor2_interior_extension_states(2).size() == 25);

    auto nodes = trace.find<mathlib::reflection::ClassIIInteriorTipCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIInteriorTipCertificate nodes\n";
    assert(nodes.size() == 2);
    assert(nodes[0].second->r == 2);
    assert(nodes[1].second->r == 20);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_interior_tip_instance_0") != std::string::npos);
    assert(lean.find("class_ii_interior_tip_instance_1") != std::string::npos);
    assert(lean.find("neighbor2InteriorTipG") != std::string::npos);

    std::ofstream out("/tmp/class_ii_interior_tip_generated.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_interior_tip_reflection_test: PASS\n";
    return 0;
}
