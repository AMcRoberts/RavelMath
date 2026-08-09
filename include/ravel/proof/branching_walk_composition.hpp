#pragma once

#include <array>
#include <algorithm>
#include <cstddef>
#include <set>
#include <vector>

namespace ravel::proof {

struct BranchingJump {
    long long from_letter = 0;
    long long child_index = 0;
    long long consumed_steps = 1;
    long long target_branch = -1; // -1 means the deterministic tail never branches
};

struct BranchingSkeleton {
    std::vector<long long> branching_letters;
    std::vector<std::vector<BranchingJump>> jumps;
};

// Build the finite state machine obtained by contracting every deterministic
// (single-letter image) run.  A jump records the exact number of substitution
// levels consumed before the next branching letter, preserving the landmark
// event at the instant a child is selected.
template <std::size_t d>
inline BranchingSkeleton build_branching_skeleton(
    const std::array<std::vector<long long>, d>& images) {
    BranchingSkeleton out;
    std::array<long long, d> state_index{};
    state_index.fill(-1);
    for (std::size_t letter = 0; letter < d; ++letter) {
        if (images[letter].size() > 1) {
            state_index[letter] = static_cast<long long>(out.branching_letters.size());
            out.branching_letters.push_back(static_cast<long long>(letter));
            out.jumps.emplace_back();
        }
    }
    for (std::size_t state = 0; state < out.branching_letters.size(); ++state) {
        const long long from = out.branching_letters[state];
        for (std::size_t child = 0; child < images[static_cast<std::size_t>(from)].size(); ++child) {
            long long current = images[static_cast<std::size_t>(from)][child];
            long long consumed = 1;
            std::set<long long> seen;
            long long target = -1;
            while (state_index[static_cast<std::size_t>(current)] < 0 &&
                   images[static_cast<std::size_t>(current)].size() == 1) {
                if (!seen.insert(current).second) break;
                current = images[static_cast<std::size_t>(current)][0];
                ++consumed;
            }
            if (state_index[static_cast<std::size_t>(current)] >= 0)
                target = state_index[static_cast<std::size_t>(current)];
            out.jumps[state].push_back({from, static_cast<long long>(child), consumed, target});
        }
    }
    return out;
}

struct BranchingLandmarkEvent {
    long long depth = 0;
    long long from_letter = 0;
    long long child_index = 0;
    friend bool operator==(const BranchingLandmarkEvent& a, const BranchingLandmarkEvent& b) {
        return a.depth == b.depth && a.from_letter == b.from_letter && a.child_index == b.child_index;
    }
    friend bool operator<(const BranchingLandmarkEvent& a, const BranchingLandmarkEvent& b) {
        if (a.depth != b.depth) return a.depth < b.depth;
        if (a.from_letter != b.from_letter) return a.from_letter < b.from_letter;
        return a.child_index < b.child_index;
    }
};

using BranchingLandmarkTrace = std::vector<BranchingLandmarkEvent>;

inline void enumerate_branching_traces(
    long long depth, long long state, const BranchingSkeleton& skeleton,
    BranchingLandmarkTrace current, std::set<BranchingLandmarkTrace>& out) {
    if (depth <= 0) { out.insert(std::move(current)); return; }
    for (const auto& jump : skeleton.jumps[static_cast<std::size_t>(state)]) {
        auto next = current;
        if (jump.child_index > 0)
            next.push_back({depth, jump.from_letter, jump.child_index});
        if (jump.consumed_steps > depth) {
            out.insert(std::move(next));
        } else if (jump.target_branch < 0) {
            out.insert(std::move(next));
        } else {
            enumerate_branching_traces(depth - jump.consumed_steps,
                                       jump.target_branch, skeleton, std::move(next), out);
        }
    }
}

template <std::size_t d>
inline std::set<BranchingLandmarkTrace> enumerate_branching_traces_from_letter(
    long long start_letter, long long depth,
    const std::array<std::vector<long long>, d>& images,
    const BranchingSkeleton& skeleton) {
    std::set<BranchingLandmarkTrace> out;
    long long current = start_letter;
    long long remaining = depth;
    std::set<long long> seen;
    while (remaining > 0 && images[static_cast<std::size_t>(current)].size() == 1) {
        if (!seen.insert(current).second) { out.insert(BranchingLandmarkTrace{}); return out; }
        current = images[static_cast<std::size_t>(current)][0];
        --remaining;
    }
    if (remaining <= 0 || skeleton.branching_letters.empty()) {
        out.insert(BranchingLandmarkTrace{});
        return out;
    }
    const auto it = std::find(skeleton.branching_letters.begin(), skeleton.branching_letters.end(), current);
    if (it == skeleton.branching_letters.end()) { out.insert(BranchingLandmarkTrace{}); return out; }
    enumerate_branching_traces(remaining,
                               static_cast<long long>(it - skeleton.branching_letters.begin()),
                               skeleton, {}, out);
    return out;
}

} // namespace ravel::proof
