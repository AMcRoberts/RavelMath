#include <array>
#include <cassert>
#include <iostream>
#include <set>
#include <vector>

#include "ravel/proof/branching_walk_composition.hpp"

using namespace ravel::proof;

namespace {
using Images = std::array<std::vector<long long>, 4>;
const Images images = {
    std::vector<long long>{1, 2},
    std::vector<long long>{0, 3},
    std::vector<long long>{0},
    std::vector<long long>{0}};

void brute(long long letter, long long depth, BranchingLandmarkTrace trace,
           std::set<BranchingLandmarkTrace>& out) {
    if (depth == 0) { out.insert(std::move(trace)); return; }
    const auto& image = images[static_cast<std::size_t>(letter)];
    for (std::size_t child = 0; child < image.size(); ++child) {
        auto next = trace;
        if (image.size() > 1 && child > 0)
            next.push_back({depth, letter, static_cast<long long>(child)});
        brute(image[child], depth - 1, std::move(next), out);
    }
}
}

int main() {
    const auto skeleton = build_branching_skeleton<4>(images);
    assert(skeleton.branching_letters.size() == 2);
    for (long long depth = 1; depth <= 10; ++depth) {
        for (long long start = 0; start < 4; ++start) {
            std::set<BranchingLandmarkTrace> expected;
            brute(start, depth, {}, expected);
            const auto actual = enumerate_branching_traces_from_letter(start, depth, images, skeleton);
            if (actual != expected) {
                std::cerr << "mismatch depth=" << depth << " start=" << start
                          << " actual=" << actual.size() << " expected=" << expected.size() << "\n";
                return 1;
            }
        }
    }
    std::cout << "branching_walk_composition: multi-junction model matches brute force\n";
}
