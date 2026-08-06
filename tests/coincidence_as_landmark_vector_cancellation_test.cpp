// Verifies the landmark-vector-cancellation reduction against
// sigma_{0,1} (sigma(0)=1,2 sigma(1)=2 sigma(2)=0), the non-AR control:
// (1) abelianization_from_walk reproduces the true prefix
//     abelianization exactly, cross-checked against direct word
//     materialization -- not just against itself;
// (2) demonstrates the actual landmark reduction: only "letter 0,
//     choose second child" levels contribute, and the depth-12 vs
//     depth-12 match for pair (0,2) at position 17 (found earlier by
//     brute-force search) is exactly a landmark-vector cancellation.

#include <cassert>
#include <iostream>
#include <vector>

#include "ravel/proof/coincidence_as_landmark_vector_cancellation.hpp"

using namespace ravel::proof;

namespace {

std::array<std::vector<long long>, 3> images = {
    std::vector<long long>{1, 2}, std::vector<long long>{2}, std::vector<long long>{0}};
std::array<std::array<long long, 3>, 3> M = {{{0, 0, 1}, {1, 0, 0}, {1, 1, 0}}};

long long len_at_depth(long long letter, int depth) {
    if (depth == 0) return 1;
    long long total = 0;
    for (auto c : images[static_cast<std::size_t>(letter)]) total += len_at_depth(c, depth - 1);
    return total;
}

std::vector<long long> materialize(long long letter, int depth) {
    if (depth == 0) return {letter};
    std::vector<long long> out;
    for (auto c : images[static_cast<std::size_t>(letter)]) {
        auto sub = materialize(c, depth - 1);
        out.insert(out.end(), sub.begin(), sub.end());
    }
    return out;
}

LandmarkVec<3> abel_prefix_direct(const std::vector<long long>& w, long long pos) {
    LandmarkVec<3> v{};
    for (long long i = 0; i < pos; ++i) v[static_cast<std::size_t>(w[static_cast<std::size_t>(i)])]++;
    return v;
}

std::vector<WalkStep> decompose(long long letter, int depth, long long pos) {
    std::vector<WalkStep> walk;
    while (depth > 0) {
        long long offset = 0;
        bool found = false;
        for (std::size_t idx = 0; idx < images[static_cast<std::size_t>(letter)].size(); ++idx) {
            long long child = images[static_cast<std::size_t>(letter)][idx];
            long long clen = len_at_depth(child, depth - 1);
            if (pos < offset + clen) {
                walk.push_back({letter, static_cast<long long>(idx)});
                letter = child;
                depth -= 1;
                pos -= offset;
                found = true;
                break;
            }
            offset += clen;
        }
        assert(found);
    }
    walk.push_back({letter, -1});
    return walk;
}

}  // namespace

int main() {
    // Part 1: formula reproduces the true abelianization exactly, for
    // both starting letters involved in the depth-12/13 pair matches.
    for (auto [start, depth, pos] : {std::tuple{0LL, 12, 17LL}, std::tuple{2LL, 12, 17LL},
                                       std::tuple{1LL, 13, 17LL}, std::tuple{2LL, 13, 17LL}}) {
        auto walk = decompose(start, depth, pos);
        auto P_formula = abelianization_from_walk<3>(walk, images, M);
        auto full = materialize(start, depth);
        auto P_direct = abel_prefix_direct(full, pos);
        std::cout << "start=" << start << " depth=" << depth << " pos=" << pos
                  << "  formula=(" << P_formula[0] << "," << P_formula[1] << "," << P_formula[2] << ")"
                  << "  direct=(" << P_direct[0] << "," << P_direct[1] << "," << P_direct[2] << ")\n";
        assert(P_formula == P_direct);
    }

    // Part 2: the landmark reduction itself. Only levels with
    // (letter==0 && chosen_child_index==1) contribute; verify the
    // depth-12 match for pair (0,2) at position 17 is exactly a
    // landmark-vector cancellation between two SPARSE subsets.
    auto walk1 = decompose(0, 12, 17);
    auto walk2 = decompose(2, 12, 17);
    auto landmark_sum = [&](const std::vector<WalkStep>& walk) {
        LandmarkVec<3> total{};
        long long depth_top = static_cast<long long>(walk.size()) - 1;
        std::vector<long long> contributing_depths;
        for (std::size_t level = 0; level < walk.size(); ++level) {
            if (walk[level].letter == 0 && walk[level].chosen_child_index == 1) {
                long long depth_here = depth_top - static_cast<long long>(level);
                contributing_depths.push_back(depth_here);
                LandmarkVec<3> v0 = {0, 1, 0};  // P([1]), the sibling prefix before child index 1 in sigma(0)
                for (long long p = 0; p < depth_here - 1; ++p) v0 = landmark_matvec<3>(M, v0);
                total = landmark_add<3>(total, v0);
            }
        }
        std::cout << "  contributing depths: ";
        for (auto d : contributing_depths) std::cout << d << " ";
        std::cout << "\n";
        return total;
    };
    std::cout << "walk1 (start=0) landmark levels:\n";
    auto L1 = landmark_sum(walk1);
    std::cout << "walk2 (start=2) landmark levels:\n";
    auto L2 = landmark_sum(walk2);
    std::cout << "L1=(" << L1[0] << "," << L1[1] << "," << L1[2] << ")  L2=(" << L2[0] << "," << L2[1] << "," << L2[2] << ")\n";
    assert(L1 == L2);
    assert(L1[0] == 5 && L1[1] == 5 && L1[2] == 7);  // matches the known full abelianization

    std::cout << "coincidence_as_landmark_vector_cancellation: reduction verified exact "
                 "on both counts.\n";
    return 0;
}
