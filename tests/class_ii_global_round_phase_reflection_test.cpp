// An eighteenth Class-II reflection connection: `class_ii_neighbor2_
// global_round_phase(a, round)` (include/ravel/class_ii_neighbor2_
// pruning.hpp) matches lean/class_ii_global_round_partition.lean's
// GENERAL (in a, r) `classIIGlobalRoundPhase` exactly (already
// independently cross-checked at runtime for a in [7,40], r in
// [1,a+1] by tests/lean_class_ii_catalogue_cross_check_test.cpp,
// entry [8]). Threads the concrete phase at one representative round
// per phase; the renderer decides equality against the Lean function
// evaluated at that exact (a, round).

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/class_ii_neighbor2_pruning.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    const long long a = 10;  // a-2=8, a-1=9, a=10, a+1=11
    mathlib::reflection::Trace trace("class_ii_global_round_phase_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::class_ii_global_round_phase_reflect(a, 3);   // base (<=4)
        ravel::class_ii_global_round_phase_reflect(a, 6);   // stable (5..a-2=8)
        ravel::class_ii_global_round_phase_reflect(a, 9);   // penultimate (a-1)
        ravel::class_ii_global_round_phase_reflect(a, 10);  // terminal (a)
        ravel::class_ii_global_round_phase_reflect(a, 11);  // repeated (a+1)
    }

    auto nodes = trace.find<mathlib::reflection::ClassIIGlobalRoundPhaseCertificate>();
    std::cout << "trace recorded " << nodes.size() << " ClassIIGlobalRoundPhaseCertificate nodes\n";
    assert(nodes.size() == 5);
    // 0=base, 1=stable, 2=penultimate, 3=terminal, 4=repeated
    assert(nodes[0].second->phase == 0);
    assert(nodes[1].second->phase == 1);
    assert(nodes[2].second->phase == 2);
    assert(nodes[3].second->phase == 3);
    assert(nodes[4].second->phase == 4);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("class_ii_global_round_phase_instance_0") != std::string::npos);
    assert(lean.find("class_ii_global_round_phase_instance_4") != std::string::npos);
    assert(lean.find("classIIGlobalRoundPhaseG_spec") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_global_round_phase_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_global_round_phase_reflection_test: PASS\n";
    return 0;
}
