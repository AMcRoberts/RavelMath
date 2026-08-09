// Wires lean/radial_translation_defect.lean (previously flat: its only
// prior consumer, tests/radial_translation_defect_test.cpp, never emitted
// any Lean) to the reflection pipeline. certify_translation_defect
// independently computes the translated block-dynamics output and the
// same-translation defect from raw integer matrix/vector arithmetic.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/radial_translation_defect.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

int main() {
    using namespace ravel::proof;

    mathlib::reflection::Trace trace("radial_translation_defect_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);

        // Reuses the exact data from tests/radial_translation_defect_test.cpp.
        IntegerMatrix B{{2, 1}, {0, 1}};
        IntegerVector x{3, -2}, t{5, 5}, f{1, -1};
        stage_translation_defect(B, x, t, f, "2x2 n-bonacci-shaped block, dim 2");

        // A second, distinct concrete instance.
        IntegerMatrix B2{{1, 0, 2}, {0, 1, 1}, {0, 0, 1}};
        IntegerVector x2{1, 1, 1}, t2{2, -1, 3}, f2{0, 0, 0};
        stage_translation_defect(B2, x2, t2, f2, "3x3 upper-triangular block, dim 3");
    }

    auto nodes = trace.find<mathlib::reflection::RadialTranslationDefectCertificate>();
    std::cout << "trace recorded " << nodes.size()
              << " RadialTranslationDefectCertificate nodes\n";
    assert(nodes.size() == 2);

    std::string lean = ravel::proof::render_reflective_lean_module(trace);
    assert(lean.find("theorem affine_block_same_translation_defect") != std::string::npos);
    assert(lean.find("radial_translation_defect_instance_0") != std::string::npos);
    assert(lean.find("radial_translation_defect_instance_1") != std::string::npos);
    assert(lean.find("radial_translation_defect_instance_2") == std::string::npos);

    std::ofstream out("lean/generated/radial_translation_defect_batch.lean");
    out << lean;
    out.close();

    std::cout << "radial_translation_defect_reflection_test: PASS\n";
    return 0;
}
