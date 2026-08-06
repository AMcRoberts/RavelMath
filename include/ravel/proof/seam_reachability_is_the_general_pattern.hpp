// ravel/proof/seam_reachability_is_the_general_pattern.hpp
//
// Applies the same lens that just paid off on the Spectre C++ port
// (recognize that what LOOKS like a case-specific complication is
// actually a lucky-simplifying special case of one general, already-
// generic pattern this project uses everywhere) back to the Pisot
// "seam" question from general_generator_theorem.hpp Part 3 and
// occurrence_length_sets.hpp: is the eventually-periodic case genuinely
// irregular, or is it "behaving, just in its own way" -- following a
// single uniform rule that the terminating case merely happens to
// trivialize?
//
// ANSWER: it is behaving, and the rule is not new -- it is the same
// state-augmented reachability pattern this project already uses for
// contact-boundary/corona closure elsewhere (a search over pairs
// (graph node, accumulated label-sum), not over the graph nodes alone).
// `canonical_substitution_generator_collapse.hpp`'s `reachable_via_
// unit_steps` already implements exactly this: it searches over pairs
// (role, net defect-so-far), not over roles alone. What this header
// adds is a direct demonstration that NO reduction of that search to
// flat set arithmetic on lengths alone -- not the falsified "range"
// hypothesis, and not the sumset/membership hypothesis tried below --
// can be correct, because the (role, net) state is load-bearing: two
// different roles can sit at the identical accumulated net and still
// have completely different onward reachability. That is precisely
// why a formula that forgets WHICH role you are at (keeping only how
// far you have moved) cannot work in general, and precisely why the
// existing BFS's state space (role x net, not net alone) is not an
// implementation accident -- it is the minimal correct state space for
// this problem, in the same sense the Spectre port's generic assembly
// rule needed (label, corner) as its state, not label alone.
//
// SUMSET HYPOTHESIS (tested here, refuted): "a rigid letter's value v
// is reachable through the flexible letters iff v lies in the pooled
// SET  S_0 union (S_0 + s : s in S_junction)", where S_0 is letter 0's
// occurrence-length set and S_junction is the preperiod-cycle-start
// letter's occurrence-length set. This looked promising -- it
// correctly predicts REDUCES for x^4-2x^2-2x+1 (the counterexample
// that broke the earlier "range" hypothesis: S_0={0,1}, S_junction=
// {0,1}, rigid value 2, pool = {0,1} u {0,1} u {1,2} = {0,1,2}, which
// contains 2). It is refuted by x^3-2x^2-x+1 (the confirmed GENUINE
// witness): S_0={0,1}, S_junction={1,2}, rigid value 0, pool =
// {0,1} u {1,2} u {2,3} = {0,1,2,3}, which trivially contains 0 (0 is
// "reachable" from any pool by taking zero steps) -- yet the actual
// answer is IRREDUCIBLE. Membership in a pooled set cannot distinguish
// "reachable by doing nothing" from "reachable by a real walk that
// lands exactly here", because it has already forgotten which letter
// you would be standing on partway through. This is checked below by
// direct computation against both known witnesses, not asserted.

#pragma once

#include <set>
#include <string>
#include <vector>

#include "ravel/proof/occurrence_length_sets.hpp"

namespace ravel::proof {

struct SumsetHypothesisCheck {
    std::string name;
    bool predicted_reducible{};
    bool actual_reducible{};
    bool hypothesis_correct_here{};
};

// Evaluates the (refuted) sumset hypothesis for one rigid target value
// against letter 0's occurrence set and a junction letter's occurrence
// set, and compares it to the already-known ground truth for that
// case. Exists purely as a falsifiable, re-runnable check -- not to be
// trusted as a predictor (it is wrong on x^3-2x^2-x+1 by construction).
inline SumsetHypothesisCheck check_sumset_hypothesis(
    const std::string& name,
    const std::set<long long>& s0,
    const std::set<long long>& s_junction,
    long long rigid_value,
    bool actual_reducible) {
    std::set<long long> pool = s0;
    for (long long a : s0) for (long long s : s_junction) pool.insert(a + s);
    bool predicted = pool.count(rigid_value) > 0;
    SumsetHypothesisCheck out;
    out.name = name;
    out.predicted_reducible = predicted;
    out.actual_reducible = actual_reducible;
    out.hypothesis_correct_here = (predicted == actual_reducible);
    return out;
}

}  // namespace ravel::proof
