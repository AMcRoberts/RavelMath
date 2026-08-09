// Wires lean/defect_spliced_covering_tube.lean (previously flat: its only
// prior C++ reference, tests/defect_spliced_tube_test.cpp, never emitted
// any Lean) to the reflection pipeline. stage_defect_splice_steps
// independently re-verifies the base path's own n-bonacci recurrence (not
// checked by certify_defect_spliced_tube itself), reusing the translation
// and transported recurrence checks that function already performs.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/covering_translation_tube.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    using namespace ravel::proof;

    mathlib::reflection::Trace trace("defect_splice_step_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Reuses the exact "n3.homogeneous" data from
        // tests/defect_spliced_tube_test.cpp.
        const auto homogeneous = certify_defect_spliced_tube(
            "n3.homogeneous", 2, 1,
            {{1, -2, 1}, {-2, 1, 1}},
            {{-1, -1, -1}, {-1, -1, 1}},
            {-1}, {0});
        assert(homogeneous.first_return_transport);
        stage_defect_splice_steps(homogeneous, "n3 homogeneous splice");

        // Negative control: an unclosed/invalid certificate must not stage.
        DefectSplicedTubeCertificate bad;
        bad.first_return_transport = false;
        stage_defect_splice_steps(bad, "invalid control");
    }

    auto nodes = trace.find<mathlib::reflection::DefectSpliceStepCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " DefectSpliceStepCertificate nodes\n";
    assert(nodes.size() == 1);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem defect_splice_step") != std::string::npos);
    assert(lean.find("defect_splice_step_instance_0") != std::string::npos);
    assert(lean.find("defect_splice_step_instance_1") == std::string::npos);

    std::ofstream out("lean/generated/defect_splice_step_batch.lean");
    out << lean;
    out.close();

    std::cout << "defect_splice_step_reflection_test: PASS\n";
    return 0;
}
