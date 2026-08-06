// Verifies the memoized coincidence closure against the already-known
// ground truth (brute-force materialization found depth 12 for
// sigma_{0,1}'s pair (0,2)) before trusting it on anything new.

#include <array>
#include <cassert>
#include <iostream>

#include "ravel/proof/coincidence_closure.hpp"

using namespace ravel::proof;

int main() {
    // sigma_{0,1}: sigma(0)=[1,2], sigma(1)=[2], sigma(2)=[0]
    std::array<std::vector<long long>, 3> images = {
        std::vector<long long>{1, 2}, std::vector<long long>{2}, std::vector<long long>{0}};
    std::array<std::array<long long, 3>, 3> M = {{{0, 0, 1}, {1, 0, 0}, {1, 1, 0}}};

    auto edges = build_junction_graph<3>(images);
    std::cout << "junction edges: " << edges.size() << "\n";
    for (auto& e : edges)
        std::cout << "  " << e.from_junction << " -[size=" << e.jump_size << "]-> " << e.to_junction << "\n";

    CoincidenceClosure<3> closure(edges, M);

    // Only letter 0 is a junction; letters 1 and 2 need their own
    // run-in before the closure logic applies. For this test, directly
    // check depth K by looking at reachable(0, K) vs reachable via the
    // run-in-adjusted starting point for letter 2 (run-in 1: 2->0).
    bool found = false;
    for (long long K = 1; K <= 20; ++K) {
        const auto& from0 = closure.reachable(0, K);
        // letter 2's run-in is 1 step (2->0); so its depth-K walk equals
        // a depth-(K-1) walk from junction 0.
        if (K - 1 < 0) continue;
        const auto& from2 = closure.reachable(0, K - 1);
        for (auto& s0 : from0) {
            if (from2.count(s0)) {
                std::cout << "coincidence found at K=" << K << " (via closure, not brute force)\n";
                found = true;
                break;
            }
        }
        if (found) { assert(K == 12); break; }
    }
    assert(found);
    std::cout << "coincidence_closure: matches known ground truth (depth 12), computed via the "
                 "memoized exact-vector closure instead of raw word materialization.\n";
    return 0;
}
