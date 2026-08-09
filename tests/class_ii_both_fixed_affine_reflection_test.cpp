// A first reflection connection for Finding 12: the closed-form
// both-fixed proof (app/class_ii_both_fixed_full_proof.cpp, extracted
// this session into ravel::proof::class_ii_both_fixed_affine_
// instances()) finds 323 (CONST, slope, target, a_required) instances
// with an integer solution, all with a_required < 7. Threads a stride
// sample of 20 into the trace; the renderer instantiates lean/class_
// ii_round234_shape_closure.lean's already-proven general
// `affine_no_solution_at_or_above_threshold` (threshold=7) at each.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/class_ii_both_fixed_affine.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    // Regression: the extracted function reproduces the app's own
    // headline result exactly (323 instances, none needing a>=7).
    const auto instances = ravel::proof::class_ii_both_fixed_affine_instances();
    std::cout << "class_ii_both_fixed_affine_instances: " << instances.size() << " total\n";
    assert(instances.size() == 323);
    long long need_ge_7 = 0;
    for (const auto& inst : instances)
        if (inst.a_required >= 7) ++need_ge_7;
    assert(need_ge_7 == 0);

    mathlib::reflection::Trace trace("both_fixed_affine_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        ravel::proof::stage_class_ii_both_fixed_affine_sample(20);
    }

    auto nodes = trace.find<mathlib::reflection::BothFixedAffineCertificate>();
    std::cout << "trace recorded " << nodes.size() << " BothFixedAffineCertificate nodes\n";
    assert(nodes.size() >= 18 && nodes.size() <= 22);
    for (const auto& [id, node] : nodes) {
        (void)id;
        assert(node->a_required < 7);
        assert(node->const_ + node->a_required * node->slope == node->target);
    }

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("both_fixed_affine_instance_0") != std::string::npos);
    assert(lean.find("affine_no_solution_at_or_above_threshold") != std::string::npos);

    std::ofstream out("lean/generated/class_ii_both_fixed_affine_batch.lean");
    out << lean;
    out.close();

    std::cout << "class_ii_both_fixed_affine_reflection_test: PASS\n";
    return 0;
}
