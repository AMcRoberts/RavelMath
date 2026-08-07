// ravel/proof/class_ii_round1_red_pruning_data.hpp
//
// The 27-state raw pre-Red target for Class-II neighbor tau_a's
// Round 1, extracted into a shared, reusable function -- previously
// duplicated inline inside
// app/class_ii_neighbor2_round1_red_forward_check.cpp (and referenced,
// not re-derived, by lean/class_ii_round1_red_pruning.lean's own
// `round1Raw27`, per that file's own "Source of truth" comment).
//
// Recording the concrete node list here lets the reflection pipeline
// thread it into a `ClassIIFixedTableCertificate`-style corollary,
// checked against `round1Raw27` via `decide` -- the same "verify what
// C++ actually built, not a citation by name" pattern used throughout
// this project's Class-II retrofit.

#pragma once

#include <array>
#include <vector>

#include "math/proof_reflection.hpp"

namespace ravel {

// The 27-state raw pre-Red target, reproduced exactly (same order) as
// app/class_ii_neighbor2_round1_red_forward_check.cpp's own
// `target_states()`, which is `lean/class_ii_round1_red_pruning.lean`'s
// cited source of truth for `round1Raw27`.
inline std::vector<std::array<long long, 5>> class_ii_round1_raw27_targets() {
    std::vector<std::array<long long, 5>> result = {
        {0,-1, 1, 1,0}, {0,-1, 1, 1,2}, {0, 1,-1, 0,2}, {0, 1,-1, 1,0},
        {0, 1, 0,-1,0}, {1, 0, 1, 0,0}, {1, 1,-1, 0,1}, {1, 2,-1,-1,0},
        {2,-1, 2, 0,1}, {2, 1,-1, 0,0}, {2, 1,-1, 0,2},
        {0,-1, 1, 1,1}, {0, 0, 0, 0,1}, {0, 0, 0, 0,2}, {0, 0, 0, 1,0},
        {0, 0, 0, 1,1}, {0, 0, 1, 0,0}, {0, 1,-1, 0,0}, {1, 0, 0, 0,2},
        {1, 1,-1, 0,0}, {1, 1,-1, 0,2}, {1, 1, 0,-1,0}, {2, 0, 1,-1,0},
        {2, 0, 1, 0,0}, {2, 1, 0,-1,0},
        {1, 0, 0, 1,1}, {2, 0, 1,-1,1},
    };
    // Thread the ACTUAL concrete nodes so the renderer decides
    // membership of THESE nodes against `round1Raw27` -- a divergence
    // between this list and the Lean file's own copy would make the
    // kernel check legitimately fail.
    if (mathlib::reflection::enabled()) {
        mathlib::reflection::ClassIIFixedTableCertificate node;
        node.table = "round1_raw27";
        for (const auto& s : result)
            node.nodes.push_back({s[0], s[1], s[2], s[3], s[4]});
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
    return result;
}

}  // namespace ravel
