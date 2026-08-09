// A sixteenth Class-II reflection connection: `class_ii_neighbor2_
// penultimate_pair(a)` (extracted this session from `class_ii_
// neighbor2_penultimate_extension_states(a)`'s inline construction,
// include/ravel/class_ii_neighbor_family.hpp) matches lean/class_ii_
// neighbor2_extensions.lean's GENERAL (in a) `neighbor2PenultimatePair`
// exactly (already independently cross-checked at runtime for a in
// [4,40] by tests/lean_class_ii_catalogue_cross_check_test.cpp, entry
// [5]'s "penultimate pair" line). Threads the concrete 2-node pair at
// two different `a`; the renderer decides set equality against the
// Lean function evaluated at that exact `a`.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_penultimate_pair_batch");
    std::set<ravel::SNode<3>> pair4, pair20;
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        pair4 = ravel::class_ii_neighbor2_penultimate_pair(4);
        pair20 = ravel::class_ii_neighbor2_penultimate_pair(20);
    }
    assert(pair4.size() == 2);
    assert(pair20.size() == 2);

    // Regression: class_ii_neighbor2_penultimate_extension_states
    // (which now calls the extracted function) still returns 24+2=26.
    assert(ravel::class_ii_neighbor2_penultimate_extension_states(4).size() == 26);

    auto nodes = trace.find<mathlib::reflection::ClassIIPenultimatePairCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIPenultimatePairCertificate nodes\n";
    assert(nodes.size() == 2);
    assert(nodes[0].second->a == 4);
    assert(nodes[1].second->a == 20);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_penultimate_pair_instance_0") != std::string::npos);
    assert(lean.find("class_ii_penultimate_pair_instance_1") != std::string::npos);
    assert(lean.find("neighbor2PenultimatePairG") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_penultimate_pair_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_penultimate_pair_reflection_test: PASS\n";
    return 0;
}
