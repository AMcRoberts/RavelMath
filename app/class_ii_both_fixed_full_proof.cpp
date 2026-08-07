// Completes class_ii_both_fixed_corrected_condition.cpp's finding
// (rhs2 = slope_q - slope_p holds for every CURRENTLY valid both-fixed
// edge, checked at a=7 and a=20) with an actual closed-form proof
// that it holds for EVERY integer a>=7, not just the two tested
// points -- by solving directly, not sampling further.
//
// For a both-fixed edge, x2'(a) = CONST + a*[(slope_q - slope_p) -
// rhs2] is fully determined (CONST and the slope coefficient) once
// the shape and node.x are fixed -- both are computable in closed
// form from the exhaustively-enumerated occurrence-type formulas
// (class_ii_hybrid_window_slope_derivation.cpp's 5 types: p_len(a) =
// slope*a + b, with (slope,b) one of (0,0) [parent=2], (1,0)
// [parent=0,inner=1], (1,1) [parent=0,inner=2], (1,-1)
// [parent=1,inner=2], (1,0) [parent=1,inner=0,past marker]).
//
// If the resulting slope of x2'(a) is nonzero, x2'(a) = T.x[2] (for
// any specific candidate target T with matching x0',x1') has AT MOST
// ONE integer solution a, computable directly: a* = (T.x[2] -
// CONST) / slope. This file computes that required a* for every
// (node, shape-combination, candidate target) triple where the slope
// is nonzero -- not just the currently-valid ones -- and checks
// whether a* is ever an integer >= 7. If never, this is a genuine
// closed-form proof (not another finite check) that no
// slope-nonzero edge can ever be valid for any integer a>=7: the
// corrected condition (rhs2 = slope_q - slope_p) is therefore
// necessary, not just observed, for every integer a>=7.

#include <cstdio>

#include "ravel/proof/class_ii_both_fixed_affine.hpp"

using namespace ravel;

int main() {
    // Shared with tests/class_ii_both_fixed_affine_reflection_test.cpp
    // (extracted from this file's own former inline computation).
    const auto instances = ravel::proof::class_ii_both_fixed_affine_instances();

    long long total_would_need_a_ge_7 = 0;
    for (const auto& inst : instances) {
        if (inst.a_required >= 7) {
            ++total_would_need_a_ge_7;
            std::printf(
                "COUNTEREXAMPLE POSSIBLE: CONST=%lld slope=%lld target=%lld "
                "requires a=%lld (>=7!)\n",
                inst.const_, inst.slope, inst.target, inst.a_required);
        }
    }

    std::printf(
        "total_instances_with_integer_solution=%zu would_need_a>=7=%lld\n",
        instances.size(), total_would_need_a_ge_7);
    std::printf(
        "%s\n",
        total_would_need_a_ge_7 == 0
            ? "CLOSED_FORM_PROOF_COMPLETE: every slope-nonzero "
              "(node, shape) combination either has no integer "
              "solution at all, or requires a < 7 -- computed "
              "directly, not sampled. No slope-nonzero both-fixed "
              "edge can ever be valid for any integer a>=7. The "
              "corrected condition (rhs2 = slope_q - slope_p) is "
              "therefore NECESSARY, not merely observed, for every "
              "integer a>=7"
            : "COUNTEREXAMPLE EXISTS FOR SOME a>=7 -- see detail above, "
              "the both-fixed closure needs revisiting");
    return total_would_need_a_ge_7 == 0 ? 0 : 1;
}
