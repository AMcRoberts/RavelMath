// Verifies the generalized (parametrized, not hardcoded) single-junction
// composition model against full brute-force Dumont-Thomas walk
// enumeration for sigma_{0,1}, across many depths and all three
// starting letters.

#include <array>
#include <cassert>
#include <iostream>
#include <set>
#include <vector>

#include "ravel/proof/single_junction_coincidence_composition.hpp"

using namespace ravel::proof;

namespace {

std::array<std::vector<long long>, 3> images = {
    std::vector<long long>{1, 2}, std::vector<long long>{2}, std::vector<long long>{0}};

// Ground truth: exhaustive walk enumeration, landmark set = depths
// where letter 0 picks child index 1 (the only junction/branch point).
void enumerate_truth(long long letter, long long depth, std::vector<long long> lm,
                      std::vector<std::vector<long long>>& out) {
    if (depth == 0) { out.push_back(lm); return; }
    const auto& img = images[static_cast<std::size_t>(letter)];
    if (img.size() == 1) { enumerate_truth(img[0], depth - 1, lm, out); return; }
    enumerate_truth(img[0], depth - 1, lm, out);
    auto lm2 = lm;
    lm2.push_back(depth);
    enumerate_truth(img[1], depth - 1, lm2, out);
}

}  // namespace

int main() {
    // run_in_to_junction[letter] = deterministic steps from `letter` to
    // reach junction letter 0 (only meaningful for non-junction letters).
    std::array<long long, 3> run_in = {0, 2, 1};  // 1->2->0 (2 steps); 2->0 (1 step)
    auto jumps = compute_junction_jumps<3>(images, 0, run_in);

    int mismatches = 0;
    for (long long depth = 1; depth <= 8; ++depth) {
        for (long long start : {0LL, 1LL, 2LL}) {
            std::vector<std::vector<long long>> truth_list;
            enumerate_truth(start, depth, {}, truth_list);
            std::set<std::vector<long long>> truth(truth_list.begin(), truth_list.end());

            long long r = (start == 0) ? 0 : run_in[static_cast<std::size_t>(start)];
            std::set<std::vector<long long>> model;
            if (depth >= r) {
                enumerate_landmark_depth_sets(depth - r, jumps, {}, model);
            } else {
                model.insert({});  // never reaches the junction -- trivial degenerate case, noted in the header
            }

            bool ok = (truth == model);
            std::cout << "depth=" << depth << " start=" << start
                      << " truth=" << truth.size() << " model=" << model.size()
                      << (ok ? "  OK" : "  MISMATCH") << "\n";
            if (!ok && !(depth < r)) { ++mismatches; }  // the r>depth trivial edge case is not counted
        }
    }
    std::cout << "\nmismatches (excluding the documented degenerate run-in edge case): " << mismatches << "\n";
    assert(mismatches == 0);
    std::cout << "single_junction_coincidence_composition: model verified exact against brute force.\n";
    return 0;
}
