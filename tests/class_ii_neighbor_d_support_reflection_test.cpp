// A ninth Class-II reflection connection: the boundary-layer
// source/target index sets `class_ii_neighbor_d_boundary_source_
// indices`/`_target_indices` (include/ravel/class_ii_neighbor_
// family.hpp) actually compute, for each of the three neighbors,
// match lean/class_ii_neighbor_d_support.lean's own re-derivation
// from its independently maintained affine-edge catalog (already
// independently cross-checked at runtime by
// tests/lean_class_ii_catalogue_cross_check_test.cpp, entries [2]/[3]).
// Threads the concrete sets; the renderer decides them equal to the
// embedded, re-derived Lean facts.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    mathlib::reflection::Trace trace("class_ii_neighbor_d_support_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::class_ii_neighbor_d_support_reflect(0);
        ravel::class_ii_neighbor_d_support_reflect(1);
        ravel::class_ii_neighbor_d_support_reflect(2);
    }

    auto nodes = trace.find<mathlib::reflection::ClassIINeighborDSupportCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIINeighborDSupportCertificate nodes\n";
    assert(nodes.size() == 3);
    assert(nodes[0].second->sources.size() == 4 && nodes[0].second->targets.size() == 6);
    assert(nodes[1].second->sources.size() == 2 && nodes[1].second->targets.size() == 5);
    assert(nodes[2].second->sources.size() == 4 && nodes[2].second->targets.size() == 12);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_neighbor_d_support_instance_0") != std::string::npos);
    assert(lean.find("class_ii_neighbor_d_support_instance_2") != std::string::npos);
    assert(lean.find("neighbor2BoundaryTargetG_eq") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_neighbor_d_support_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_neighbor_d_support_reflection_test: PASS\n";
    return 0;
}
