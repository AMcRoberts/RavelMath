// ravel/proof/zero_run_forces_bounded_coincidence_periodic.hpp
//
// Extends Finding 39 (zero_run_forces_bounded_coincidence.hpp) from
// the terminating canonical family to the EVENTUALLY-PERIODIC one
// (canonical_beta_substitution_eventually_periodic): roadmap Stage 1,
// continuing past the terminating family.
//
// A real correction was needed, not just a mechanical restatement: a
// first attempt reused Finding 39's flat-sequence longest-zero-run
// computation directly and got genuine mismatches (predicted depth
// too SMALL) whenever a zero-run spans the PERIOD'S OWN wraparound
// boundary -- e.g. period=(0,1,0) has trailing digit 0 and leading
// digit 0, which are ADJACENT once the period repeats (...,0,1,0,0,1,
// 0,0,1,0,...), giving a genuine run of 2 zeros that a flat scan of
// the single period (0,1,0) alone would only see as two separate
// runs of length 1. Concretely: period=(0,1,0) has flat-longest-run 1
// (predicting depth <=2) but actually resolves at depth 3; period=
// (0,0,1,0,0) has flat-longest-run 2 (predicting depth<=3) but
// actually resolves at depth 5 -- both cases matching the CYCLIC
// longest run instead (2 and 4 respectively).
//
// THEOREM (corrected). Let R = max(longest run of consecutive zero
// digits in the flat sequence preperiod++period, longest run of
// consecutive zero digits in period++period). Every pair of letters
// resolves strong coincidence at depth <= R+1.
//
// PROOF sketch: identical mechanism to Finding 39 (a run of k
// consecutive zero-digit letters is k pass-through, non-branching
// single-letter images, delaying any leading-zero image by k steps),
// generalized to account for the fact that the LAST state (index n-1)
// wraps to letter `p` (period start, per
// canonical_beta_substitution_eventually_periodic's own construction)
// rather than terminating -- so a zero-run beginning near the end of
// the period and continuing (via the wrap) into the period's own
// start is a genuine, single pass-through chain in the substitution's
// actual graph, exactly as if the digits were literally adjacent.
// Taking `period++period` (two full copies) is sufficient to detect
// any such wraparound run, since a maximal run can span at most one
// full wrap (the period itself is finite).
//
// VERIFIED computationally against a from-scratch coincidence search
// on 10 structurally diverse (preperiod, period) pairs, including two
// specifically constructed to stress-test the wraparound case (where
// the naive flat-only formula gives a real, confirmed WRONG/too-small
// prediction) -- see the accompanying test.

#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace ravel::proof {

struct EventuallyPeriodicZeroRunCertificate {
    std::vector<long long> preperiod;
    std::vector<long long> period;
    long long flat_run{0};
    long long cyclic_run{0};
    long long longest_zero_run{0};       // R = max(flat_run, cyclic_run)
    long long predicted_max_depth{1};    // R+1
    std::string note;
};

namespace zero_run_periodic_detail {
inline long long longest_run_of_zeros(const std::vector<long long>& seq) {
    long long run = 0, best = 0;
    for (auto d : seq) { if (d == 0) { ++run; best = std::max(best, run); } else run = 0; }
    return best;
}
}  // namespace zero_run_periodic_detail

inline EventuallyPeriodicZeroRunCertificate derive_eventually_periodic_zero_run_bound(
    const std::vector<long long>& preperiod, const std::vector<long long>& period) {
    using namespace zero_run_periodic_detail;
    EventuallyPeriodicZeroRunCertificate out;
    out.preperiod = preperiod;
    out.period = period;
    if (preperiod.empty() || period.empty()) {
        out.note = "empty preperiod or period; premise does not apply";
        return out;
    }
    std::vector<long long> flat = preperiod;
    flat.insert(flat.end(), period.begin(), period.end());
    std::vector<long long> doubled_period = period;
    doubled_period.insert(doubled_period.end(), period.begin(), period.end());

    out.flat_run = longest_run_of_zeros(flat);
    out.cyclic_run = longest_run_of_zeros(doubled_period);
    out.longest_zero_run = std::max(out.flat_run, out.cyclic_run);
    out.predicted_max_depth = out.longest_zero_run + 1;
    out.note = "R = max(flat run, cyclic run across the period's own wraparound) = "
               + std::to_string(out.longest_zero_run)
               + "; every pair resolves by depth " + std::to_string(out.predicted_max_depth);
    return out;
}

}  // namespace ravel::proof
