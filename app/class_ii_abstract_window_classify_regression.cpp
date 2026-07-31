// Regression check for the new reusable
// class_ii_abstract_window_classify() (include/ravel/class_ii_neighbor_family.hpp),
// factored out of app/class_ii_neighbor2_round1_window_certificate.cpp's
// own duplicated AffineCD logic so a future Round 2/3/4 attempt doesn't
// have to re-derive or re-duplicate it a third time.
//
// This rebuilds the exact same 97 Round-1 categories and checks that
// the shared function reproduces the already-verified result exactly:
// zero unresolved cases, 25 window-valid nodes, with the two D_cont
// seeds the only target states absent (expected, not a bug -- see
// class_ii_neighbor2_round1_window_certificate.cpp). If this ever
// disagrees with that file's own result, the extraction introduced a
// regression and this exits nonzero rather than silently drifting.

#include <cstdio>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"

using namespace ravel;

namespace {

using Range = std::pair<ClassIIAffineValue, ClassIIAffineValue>;

std::vector<SNode<3>> new_states() {
    return {
        {0, {-1, 1, 1}, 0}, {0, {-1, 1, 1}, 2},
        {0, {1, -1, 0}, 2}, {0, {1, -1, 1}, 0},
        {0, {1, 0, -1}, 0}, {1, {0, 1, 0}, 0},
        {1, {1, -1, 0}, 1}, {1, {2, -1, -1}, 0},
        {2, {-1, 2, 0}, 1}, {2, {1, -1, 0}, 0},
        {2, {1, -1, 0}, 2},
    };
}

std::vector<SNode<3>> center_states() {
    return {
        {0, {-1, 1, 1}, 1}, {0, {0, 0, 0}, 1}, {0, {0, 0, 0}, 2},
        {0, {0, 0, 1}, 0}, {0, {0, 0, 1}, 1}, {0, {0, 1, 0}, 0},
        {0, {1, -1, 0}, 0}, {1, {0, 0, 0}, 2}, {1, {1, -1, 0}, 0},
        {1, {1, -1, 0}, 2}, {1, {1, 0, -1}, 0}, {2, {0, 1, -1}, 0},
        {2, {0, 1, 0}, 0}, {2, {1, 0, -1}, 0},
    };
}

std::vector<SNode<3>> seed_states() {
    return {{1, {0, 0, 1}, 1}, {2, {0, 1, -1}, 1}};
}

std::vector<SNode<3>> full27_states() {
    std::vector<SNode<3>> result = new_states();
    for (const auto& s : center_states()) result.push_back(s);
    for (const auto& s : seed_states()) result.push_back(s);
    return result;
}

std::map<ClassIIBackwardCategory, std::vector<Range>> build_categories() {
    std::map<ClassIIBackwardCategory, std::vector<Range>> candidates;
    for (const auto& destination : full27_states()) {
        const std::array<ClassIIAffineValue, 3> mx{{
            {destination.x[2], destination.x[0] + destination.x[1]},
            {destination.x[0], 0},
            {destination.x[0] + destination.x[1], 0},
        }};
        for (int type = 1; type <= 2; ++type) {
            const std::size_t first_parent = static_cast<std::size_t>(
                type == 1 ? destination.i : destination.j);
            const std::size_t second_parent = static_cast<std::size_t>(
                type == 1 ? destination.j : destination.i);
            const long long sign = type == 1 ? 1 : -1;
            for (std::size_t first_child = 0; first_child < 3; ++first_child) {
                const auto firsts = class_ii_neighbor_symbolic_prefix_families(
                    2, first_parent, first_child);
                for (std::size_t second_child = 0;
                     second_child < 3; ++second_child) {
                    const auto seconds =
                        class_ii_neighbor_symbolic_prefix_families(
                            2, second_parent, second_child);
                    for (const auto& first : firsts)
                        for (const auto& second : seconds) {
                            std::array<ClassIIAffineValue, 3> base{};
                            for (std::size_t c = 0; c < 3; ++c) {
                                base[c] = {
                                    sign * mx[c].intercept
                                        + first.base[c].intercept
                                        - second.base[c].intercept,
                                    sign * mx[c].slope
                                        + first.base[c].slope
                                        - second.base[c].slope};
                            }
                            if (base[1].slope != 0 || base[2].slope != 0)
                                continue;
                            auto minimum = base[0];
                            auto maximum = base[0];
                            if (first.varying_coordinate == 0)
                                maximum = maximum + first.length
                                    - ClassIIAffineValue{1, 0};
                            if (second.varying_coordinate == 0)
                                minimum = minimum - second.length
                                    + ClassIIAffineValue{1, 0};
                            const ClassIIBackwardCategory category{
                                static_cast<long long>(first_child),
                                base[1].intercept, base[2].intercept,
                                static_cast<long long>(second_child)};
                            candidates[category].push_back(
                                {minimum, maximum});
                        }
                }
            }
        }
    }
    return candidates;
}

}  // namespace

int main() {
    const auto candidates = build_categories();
    const auto result = class_ii_abstract_window_classify(candidates, 3);

    const auto full_target = full27_states();
    const std::set<SNode<3>> target_set(full_target.begin(), full_target.end());
    const auto seed_vec = seed_states();
    const std::set<SNode<3>> seeds(seed_vec.begin(), seed_vec.end());

    std::set<SNode<3>> extra, missing;
    std::set_difference(
        result.window_valid_nodes.begin(), result.window_valid_nodes.end(),
        target_set.begin(), target_set.end(),
        std::inserter(extra, extra.end()));
    std::set_difference(
        target_set.begin(), target_set.end(),
        result.window_valid_nodes.begin(), result.window_valid_nodes.end(),
        std::inserter(missing, missing.end()));

    std::printf(
        "shared_function: bounded_cases=%lld unresolved=%lld "
        "window_valid_nodes=%zu extra=%zu missing=%zu\n",
        result.bounded_cases, result.unresolved_cases,
        result.window_valid_nodes.size(), extra.size(), missing.size());

    const bool ok = result.bounded_cases == 679
        && result.unresolved_cases == 0
        && extra.empty()
        && missing == seeds;
    std::printf(
        "%s\n",
        ok ? "REGRESSION_OK: class_ii_abstract_window_classify() "
             "reproduces class_ii_neighbor2_round1_window_certificate.cpp's "
             "own result exactly"
           : "REGRESSION_FAILURE: shared function disagrees with the "
             "already-verified Round-1 result -- do not trust the "
             "extraction");
    return ok ? 0 : 1;
}
