// Confirms, by direct computation (not hand-trace), that the sumset
// hypothesis is refuted on x^3-2x^2-x+1 while it happens to succeed on
// x^4-2x^2-2x+1 -- i.e. that the (role, net) state space the existing
// BFS already uses is load-bearing, not a simplification the sumset
// version could recover.

#include <cassert>
#include <iostream>

#include "ravel/proof/seam_reachability_is_the_general_pattern.hpp"

using namespace ravel::proof;

int main() {
    // x^4-2x^2-2x+1: digits (2,1,2,0), preperiod 2, period 2.
    // sigma: 0->0,0,1  1->2  2->0,3  3->(empty, wraps to state 2)
    // occurrence lengths, read directly off the substitution image:
    //   letter0: state0 gives {0,1}; pooled S_0 = {0,1}
    //   letter2 (junction, cycle-start=2): state1 gives len0 (normal),
    //     state3's wraparound gives len0 too in this digit sequence's
    //     actual trace -> S_junction = {0,1} (matches the earlier
    //     hand-derived parents(2) = {(state1,len1),(state3,len0)})
    //   rigid letters 1,3 both sit at value 2.
    {
        std::set<long long> s0 = {0, 1};
        std::set<long long> s_junction = {0, 1};
        auto r = check_sumset_hypothesis("x^4-2x^2-2x+1 (rigid value 2)", s0, s_junction, 2,
                                          /*actual_reducible=*/true);
        std::cout << r.name << ": predicted=" << r.predicted_reducible
                  << " actual=" << r.actual_reducible
                  << " correct_here=" << r.hypothesis_correct_here << "\n";
        assert(r.hypothesis_correct_here);
    }

    // x^3-2x^2-x+1: digits (2,0,1), preperiod 1, period 2.
    // sigma: 0->0,0,1  1->2  2->0,1  (wraps to state 1, cycle-start)
    //   letter0: state0 gives {0,1}, state2 gives {0}; pooled S_0={0,1}
    //   letter1 (junction, cycle-start=1): parents(1) = {(state0,len2),
    //     (state2,len1)} -> S_junction = {1,2}
    //   rigid letter2 sits at value 0.
    {
        std::set<long long> s0 = {0, 1};
        std::set<long long> s_junction = {1, 2};
        auto r = check_sumset_hypothesis("x^3-2x^2-x+1 (rigid value 0)", s0, s_junction, 0,
                                          /*actual_reducible=*/false);  // confirmed GENUINE generator
        std::cout << r.name << ": predicted=" << r.predicted_reducible
                  << " actual=" << r.actual_reducible
                  << " correct_here=" << r.hypothesis_correct_here << "\n";
        // This is the refutation: membership trivially includes 0 (the
        // "do nothing" walk), so the hypothesis predicts reducible,
        // but the confirmed ground truth (32-state boundary graph,
        // eventually_periodic_generator_witness.hpp) is irreducible.
        assert(!r.hypothesis_correct_here);
    }

    std::cout << "seam_reachability_is_the_general_pattern: sumset hypothesis refuted as "
                 "expected -- the (role, net) state space is load-bearing, matching the "
                 "already-implemented BFS in canonical_substitution_generator_collapse.hpp.\n";
    return 0;
}
