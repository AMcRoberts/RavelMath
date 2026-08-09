#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
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
    const std::array<std::array<long long, 4>, 4> matrix = {{
        {{0, 1, 1, 1}},
        {{1, 0, 0, 0}},
        {{1, 0, 0, 0}},
        {{0, 1, 0, 0}}}};
    const auto traces0 = enumerate_weighted_landmark_traces_from_letter(0, 6, images, skeleton);
    const auto traces1 = enumerate_weighted_landmark_traces_from_letter(1, 6, images, skeleton);
    const auto collision = find_weighted_landmark_collision<4>(traces0, traces1, matrix);
    assert(collision.has_value());
    std::cout << "weighted landmark collision value=" << collision->value[0] << ","
              << collision->value[1] << "," << collision->value[2] << ","
              << collision->value[3] << "\n";
    WeightedLandmarkTrace synthetic_a{{3, 1, 0, 1, {1, 0, 0, 0}}};
    WeightedLandmarkTrace synthetic_b{{3, 1, 0, 2, {0, 1, 0, 0}}};
    std::set<WeightedLandmarkTrace> synthetic_left{synthetic_a};
    std::set<WeightedLandmarkTrace> synthetic_right{synthetic_b};
    const std::array<std::array<long long, 4>, 4> collision_matrix = {{
        {{1, 1, 0, 0}}, {{0, 0, 0, 0}}, {{0, 0, 0, 0}}, {{0, 0, 0, 0}}}};
    const auto nontrivial = find_nontrivial_weighted_landmark_collision<4>(
        synthetic_left, synthetic_right, collision_matrix);
    assert(nontrivial.has_value());
    bool overflow_rejected = false;
    try {
        const WeightedLandmarkTrace overflowing{{2, 1, 0, 1,
            {std::numeric_limits<long long>::max(), 0, 0, 0}}};
        auto overflow_matrix = collision_matrix;
        overflow_matrix[0][0] = 2;
        (void)weighted_landmark_sum<4>(overflowing, overflow_matrix);
    } catch (const std::overflow_error&) {
        overflow_rejected = true;
    }
    assert(overflow_rejected);
    std::cout << "branching_walk_composition: multi-junction model matches brute force\n";
}
