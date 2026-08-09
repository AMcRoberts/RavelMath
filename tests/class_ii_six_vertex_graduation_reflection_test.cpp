// A twelfth Class-II reflection connection: `class_ii_neighbor2_
// penultimate_promoted_states(a)`/`_survivor_transfer(a)` (include/
// ravel/class_ii_neighbor2_pruning.hpp) match lean/class_ii_six_
// vertex_graduation.lean's GENERAL (in q) `promotedNodes`/
// `transferredNode` functions exactly (already independently
// cross-checked at runtime for a in [5,32] by tests/lean_class_ii_
// catalogue_cross_check_test.cpp, entry [1]). Threads the concrete
// nodes at two different `a`; the renderer decides them equal to the
// Lean functions evaluated at q=a-1 and instantiates the
// already-proven general Nodup/disjoint lemmas at that q.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor2_pruning.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_six_vertex_graduation_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::class_ii_six_vertex_graduation_reflect(5);
        ravel::class_ii_six_vertex_graduation_reflect(11);
    }

    auto nodes = trace.find<mathlib::reflection::ClassIISixVertexGraduationCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIISixVertexGraduationCertificate nodes\n";
    assert(nodes.size() == 2);
    assert(nodes[0].second->a == 5);
    assert(nodes[1].second->a == 11);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_six_vertex_graduation_instance_0") != std::string::npos);
    assert(lean.find("class_ii_six_vertex_graduation_instance_1") != std::string::npos);
    assert(lean.find("promotedNodesG_nodup") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_six_vertex_graduation_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_six_vertex_graduation_reflection_test: PASS\n";
    return 0;
}
