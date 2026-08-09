// Finding 32, GENERALIZED and wired through the reflection pipeline
// in the original per-instance pattern (not a fixed citation) --
// widened to a systematic sweep of depressed cubics, not just
// sigma_{0,2}'s own polynomial, to demonstrate the general lemma
// (depressed_cubic_q_gt_one_iff_beta_lt_neg_d) and the C++ staging
// function are genuinely reusable, the same "widen an already-correct
// pipeline" move used earlier for Barge-Diamond (Finding 42).
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

        // sigma_{0,2}'s own charpoly (Finding 32's headline case).
        stage_depressed_cubic_not_pisot(-1, -2, 0, 2);

        // Negative control: x^3-1 has complex-pair modulus exactly 1
        // (not >1) -- must NOT stage.
        stage_depressed_cubic_not_pisot(0, -1, -2, 2);

        // A systematic sweep (c in [-3,0], d in [-6,-1]) over the SAME
        // general mechanism, each with its own exact integer bracket
        // found by direct search -- real, independently verified
        // instances, not copies of the headline case.
        for (long long c = -3; c <= 0; ++c) {
            for (long long d = -6; d <= -1; ++d) {
                auto cubic = [&](long long x) { return x * x * x + c * x + d; };
                long long lo = -1;
                for (long long x = 0; x <= 10; ++x) {
                    if (cubic(x) < 0) lo = x; else break;
                }
                if (lo < 0) continue;
                long long hi = -1;
                for (long long x = lo + 1; x <= 12; ++x) {
                    if (cubic(x) > 0) { hi = x; break; }
                }
                if (hi < 0) continue;
                stage_depressed_cubic_not_pisot(c, d, lo, hi);
            }
        }
    }

    auto nodes = trace.find<mathlib::reflection::DepressedCubicNotPisotCertificate>();
    std::cout << "trace recorded " << nodes.size() << " DepressedCubicNotPisotCertificate nodes\n";
    assert(nodes.size() == 20);   // 1 headline + 19 from the sweep; the negative control declined

    std::string lean = render_reflective_lean_module(trace);
    assert(lean.find("depressed_cubic_q_gt_one_iff_beta_lt_neg_d") != std::string::npos);
    assert(lean.find("depressed_cubic_instance_19") != std::string::npos);

    std::ofstream out("lean/generated/depressed_cubic_not_pisot_batch.lean");
    out << lean;
    out.close();

    std::cout << "depressed_cubic_not_pisot_reflection_test: PASS\n";
    return 0;
}
