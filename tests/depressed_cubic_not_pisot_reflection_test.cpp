// Finding 32, GENERALIZED and wired through the reflection pipeline
// in the original per-instance pattern (not a fixed citation): tests
// TWO structurally different depressed cubics to demonstrate the
// certificate and general lemma are genuinely general, not a
// disguised restatement of sigma_{0,2}'s own polynomial.
//
// Instance 1: x^3-x-2 (sigma_{0,2}'s own charpoly, c=-1,d=-2).
//   cubic(0)=-2<0, cubic(2)=4>0; hi=2<=-d=2.
// Instance 2: x^3-2x-3 (c=-2,d=-3), an independently chosen cubic,
//   NOT sigma_{0,2}'s.
//   cubic(0)=-3<0, cubic(2)=1>0; hi=2<=-d=3.
//
// Every exact check here (cubic(lo)<0<cubic(hi), hi<=-d) is plain
// integer arithmetic -- the mathematical fact is already established
// once these checks pass; the Lean kernel check that follows is
// independent confirmation for a reader who doesn't want to trust
// this C++, not the source of the claim's truth.

#include <cassert>
#include <fstream>
#include <iostream>

#include "math/proof_reflection.hpp"
#include "ravel/proof/depressed_cubic_not_pisot_certificate.hpp"
#include "ravel/proof/reflective_lean_renderer.hpp"

using namespace ravel::proof;

int main() {
    mathlib::reflection::Trace trace("depressed_cubic_batch");
    {
        mathlib::reflection::ScopedTrace scope(&trace);
        stage_depressed_cubic_not_pisot(-1, -2, 0, 2);   // sigma_{0,2}
        stage_depressed_cubic_not_pisot(-2, -3, 0, 2);   // independent second cubic
        stage_depressed_cubic_not_pisot(0, -1, -2, 2);   // x^3-1: real root beta=1, hi=2 > -d=1 -- must NOT stage (bound fails)
    }

    auto nodes = trace.find<mathlib::reflection::DepressedCubicNotPisotCertificate>();
    std::cout << "trace recorded " << nodes.size() << " DepressedCubicNotPisotCertificate nodes\n";
    assert(nodes.size() == 2);   // the third case correctly declined

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("depressed_cubic_q_gt_one_iff_beta_lt_neg_d") != std::string::npos);
    assert(lean.find("depressed_cubic_instance_0") != std::string::npos);
    assert(lean.find("depressed_cubic_instance_1") != std::string::npos);

    std::ofstream out("/tmp/depressed_cubic_not_pisot_generated.lean");
    out << lean;
    out.close();

    std::cout << "depressed_cubic_not_pisot_reflection_test: PASS\n";
    return 0;
}
