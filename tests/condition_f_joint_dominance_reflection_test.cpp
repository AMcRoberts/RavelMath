// Wires lean/generated/condition_f_joint_qr_playground.lean -- the actual
// closure of the flagship universal n-bonacci boundary-dominance theorem
// (rho(G_B(n)) = rho(predicted_core(n)) for every n >= 3), via the
// canonical Q/R parent-prefix split -- to the reflection pipeline.
// derive_condition_f_joint_pair_comparison's own test
// (condition_f_joint_pair_comparison_test.cpp) already checks D=2..128 but
// never emitted any Lean. NOT the earlier shell-rank/carry-bound route,
// which diary 2026-08-05 records as explicitly refuted ("universal
// maximum-shell exclusion refuted... Do not resume").

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/condition_f_joint_pair_comparison.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    using namespace ravel::proof;

    mathlib::reflection::Trace trace("condition_f_joint_dominance_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        const auto cert = derive_condition_f_joint_pair_comparison(128);
        assert(cert.proved);
        assert(cert.base_scc_count == 1);
        stage_condition_f_joint_dominance(
            cert, "canonical Q/R joint dominance, checked through dimension 128");

        // Negative control: dimension 1 is below the framework's minimum.
        const auto bad = derive_condition_f_joint_pair_comparison(1);
        assert(!bad.proved);
        stage_condition_f_joint_dominance(bad, "invalid control");
    }

    auto nodes = trace.find<mathlib::reflection::ConditionFJointDominanceCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " ConditionFJointDominanceCertificate nodes\n";
    assert(nodes.size() == 1);
    assert(nodes.front().second->target_dimension == 128);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem qr_word_intertwiner") != std::string::npos);
    assert(lean.find("theorem universal_dominance_sandwich") != std::string::npos);
    assert(lean.find("condition_f_joint_dominance_instance_0") != std::string::npos);
    assert(lean.find("condition_f_joint_dominance_instance_1") == std::string::npos);

    std::ofstream out("lean/generated/condition_f_joint_dominance_batch.lean");
    out << lean;
    out.close();

    std::cout << "condition_f_joint_dominance_reflection_test: PASS\n";
    return 0;
}
