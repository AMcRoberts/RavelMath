// Ranked Red-exclusion grammar for the stable interior layers of the
// third adjacent-swap Class-II neighbor.
//
// This is deliberately separate from the positive successor grammar:
// a successor proves survival, whereas these ranks are intended to
// partition every state outside the proposed reduced layer.

#pragma once

#include <cstddef>
#include <numeric>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"

namespace ravel {

enum class ClassIINeighbor2GlobalRoundPhase {
    base,
    stable,
    penultimate,
    terminal,
    repeated,
};

// Total phase selector mirrored by the kernel-checked theorem in
// lean/class_ii_global_round_partition.lean.
inline ClassIINeighbor2GlobalRoundPhase
class_ii_neighbor2_global_round_phase(long long a, long long round) {
    if (a < 7 || round < 1 || round > a + 1)
        throw std::domain_error(
            "Class-II neighbor-2 global round phase domain");
    if (round <= 4)
        return ClassIINeighbor2GlobalRoundPhase::base;
    if (round <= a - 2)
        return ClassIINeighbor2GlobalRoundPhase::stable;
    if (round == a - 1)
        return ClassIINeighbor2GlobalRoundPhase::penultimate;
    if (round == a)
        return ClassIINeighbor2GlobalRoundPhase::terminal;
    return ClassIINeighbor2GlobalRoundPhase::repeated;
}

// Threads the CONCRETE phase C++ actually computed for (a, round) at
// the reflection trace -- a dedicated wrapper rather than
// instrumenting class_ii_neighbor2_global_round_phase itself (which
// runs inside loops at several other call sites in this file; adding
// tracing there would flood any active trace with unrelated entries).
inline void class_ii_global_round_phase_reflect(long long a, long long round) {
    if (!mathlib::reflection::enabled()) return;
    const auto phase = class_ii_neighbor2_global_round_phase(a, round);
    mathlib::reflection::ClassIIGlobalRoundPhaseCertificate node;
    node.a = a;
    node.round = round;
    node.phase = static_cast<int>(phase);
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

inline std::set<SNode<3>>
class_ii_neighbor2_interior_pruning_rank1_seed() {
    std::set<SNode<3>> result;
    const auto add = [&](long long i, long long x0, long long x1,
                         long long x2, long long j) {
        result.insert({i, {x0, x1, x2}, j});
    };
    add(0, -4, 4, -1, 0);
    add(0, -4, 4, 1, 0);
    add(0, -4, 4, 1, 1);
    add(0, -3, 3, -2, 0);
    add(0, -3, 3, -1, 1);
    add(0, -3, 3, 1, 0);
    add(0, -3, 3, 2, 1);
    add(0, -3, 4, -1, 0);
    add(0, -3, 4, 1, 0);
    add(0, -3, 4, 1, 1);
    add(0, -2, 1, 2, 0);
    add(0, -2, 1, 2, 1);
    add(0, -2, 2, -2, 0);
    add(0, -2, 2, -1, 1);
    add(0, -2, 2, 2, 0);
    add(0, -2, 2, 2, 1);
    add(0, -2, 2, 2, 2);
    add(0, -2, 3, -2, 0);
    add(0, -2, 3, -1, 1);
    add(0, -2, 3, 2, 0);
    add(0, -1, 0, 2, 0);
    add(0, -1, 0, 2, 1);
    add(0, -1, 1, -2, 0);
    add(0, -1, 1, -1, 1);
    add(0, -1, 1, 2, 0);
    add(0, -1, 1, 2, 1);
    add(0, -1, 2, -2, 0);
    add(0, -1, 2, -1, 1);
    add(0, 0, -1, 2, 0);
    add(0, 0, -1, 2, 1);
    add(0, 0, 0, -2, 0);
    add(0, 0, 0, -1, 1);
    add(0, 0, 0, 2, 0);
    add(0, 0, 0, 2, 1);
    add(0, 0, 1, -2, 0);
    add(0, 0, 1, -1, 1);
    add(0, 1, -2, 2, 0);
    add(0, 1, -2, 2, 1);
    add(0, 1, -1, -2, 0);
    add(0, 1, -1, -1, 1);
    add(0, 1, -1, -1, 2);
    add(0, 1, -1, 2, 0);
    add(0, 1, -1, 2, 1);
    add(0, 1, 0, -2, 0);
    add(0, 1, 0, -1, 1);
    add(0, 2, -3, -2, 0);
    add(0, 2, -3, 0, 1);
    add(0, 2, -3, 2, 0);
    add(0, 2, -2, -2, 0);
    add(0, 2, -2, -1, 2);
    add(0, 2, -2, 0, 1);
    add(0, 2, -2, 2, 0);
    add(0, 2, -2, 2, 1);
    add(0, 2, -1, -2, 0);
    add(0, 3, -4, -1, 0);
    add(0, 3, -4, 0, 1);
    add(0, 3, -4, 1, 0);
    add(0, 3, -3, -1, 0);
    add(0, 3, -3, 0, 1);
    add(0, 3, -3, 2, 0);
    add(0, 4, -4, -1, 0);
    add(0, 4, -4, 1, 0);
    add(1, -3, 3, -2, 0);
    add(1, -3, 3, -1, 1);
    add(1, -3, 3, 0, 0);
    add(1, -3, 3, 0, 1);
    add(1, -2, 2, -2, 0);
    add(1, -2, 2, -1, 1);
    add(1, -2, 2, 0, 0);
    add(1, -2, 2, 1, 1);
    add(1, -2, 3, -2, 0);
    add(1, -2, 3, -1, 1);
    add(1, -2, 3, 0, 0);
    add(1, -2, 3, 0, 1);
    add(1, -1, 0, 1, 0);
    add(1, -1, 0, 1, 1);
    add(1, -1, 1, -2, 0);
    add(1, -1, 1, -1, 1);
    add(1, -1, 1, 1, 0);
    add(1, -1, 1, 1, 1);
    add(1, -1, 1, 1, 2);
    add(1, -1, 2, -2, 0);
    add(1, -1, 2, -1, 1);
    add(1, -1, 2, 1, 0);
    add(1, 0, -1, 1, 0);
    add(1, 0, -1, 1, 1);
    add(1, 0, 0, -2, 0);
    add(1, 0, 0, -1, 1);
    add(1, 0, 0, 1, 0);
    add(1, 0, 0, 1, 1);
    add(1, 0, 1, -2, 0);
    add(1, 0, 1, -1, 1);
    add(1, 1, -2, 1, 0);
    add(1, 1, -2, 1, 1);
    add(1, 1, -1, -2, 0);
    add(1, 1, -1, -1, 1);
    add(1, 1, -1, -1, 2);
    add(1, 1, -1, 1, 0);
    add(1, 1, -1, 1, 1);
    add(1, 1, 0, -2, 0);
    add(1, 1, 0, -1, 1);
    add(1, 2, -3, -2, 0);
    add(1, 2, -3, -1, 1);
    add(1, 2, -3, 0, 1);
    add(1, 2, -3, 1, 0);
    add(1, 2, -3, 1, 1);
    add(1, 2, -2, -2, 0);
    add(1, 2, -2, -1, 1);
    add(1, 2, -2, -1, 2);
    add(1, 2, -2, 1, 0);
    add(1, 2, -2, 1, 1);
    add(1, 2, -1, -2, 0);
    add(1, 3, -4, -1, 0);
    add(1, 3, -4, 0, 1);
    add(1, 3, -4, 1, 0);
    add(1, 3, -3, -2, 0);
    add(1, 3, -3, 0, 1);
    add(1, 3, -3, 1, 0);
    add(1, 3, -3, 1, 1);
    add(1, 3, -2, -2, 0);
    add(1, 4, -4, -1, 0);
    add(1, 4, -4, 1, 0);
    add(2, -4, 4, -2, 0);
    add(2, -4, 4, -1, 1);
    add(2, -4, 4, 0, 0);
    add(2, -4, 4, 0, 1);
    add(2, -3, 3, -2, 0);
    add(2, -3, 3, -1, 1);
    add(2, -3, 3, 0, 0);
    add(2, -3, 3, 1, 1);
    add(2, -3, 4, -2, 0);
    add(2, -3, 4, -1, 1);
    add(2, -3, 4, 0, 0);
    add(2, -3, 4, 0, 1);
    add(2, -2, 1, 1, 0);
    add(2, -2, 2, -2, 0);
    add(2, -2, 2, -1, 1);
    add(2, -2, 2, 1, 0);
    add(2, -2, 2, 1, 1);
    add(2, -2, 2, 1, 2);
    add(2, -2, 3, -2, 0);
    add(2, -2, 3, -1, 1);
    add(2, -2, 3, 1, 0);
    add(2, -2, 3, 1, 1);
    add(2, -1, 0, 1, 0);
    add(2, -1, 0, 1, 1);
    add(2, -1, 1, -2, 0);
    add(2, -1, 1, -1, 1);
    add(2, -1, 1, 1, 0);
    add(2, -1, 1, 1, 1);
    add(2, -1, 1, 1, 2);
    add(2, -1, 2, -2, 0);
    add(2, -1, 2, -1, 1);
    add(2, -1, 2, 1, 0);
    add(2, 0, -1, 1, 0);
    add(2, 0, -1, 1, 1);
    add(2, 0, 0, -2, 0);
    add(2, 0, 0, -1, 1);
    add(2, 0, 0, 1, 0);
    add(2, 0, 0, 1, 1);
    add(2, 0, 1, -2, 0);
    add(2, 0, 1, -1, 1);
    add(2, 1, -2, 1, 0);
    add(2, 1, -2, 1, 1);
    add(2, 1, -1, -2, 0);
    add(2, 1, -1, -1, 1);
    add(2, 1, -1, -1, 2);
    add(2, 1, -1, 1, 0);
    add(2, 1, -1, 1, 1);
    add(2, 1, 0, -2, 0);
    add(2, 1, 0, -1, 1);
    add(2, 2, -3, -2, 0);
    add(2, 2, -3, 0, 1);
    add(2, 2, -3, 1, 0);
    add(2, 2, -2, -2, 0);
    add(2, 2, -2, -1, 1);
    add(2, 2, -2, -1, 2);
    add(2, 2, -2, 0, 1);
    add(2, 2, -2, 1, 0);
    add(2, 2, -2, 1, 1);
    add(2, 2, -1, -2, 0);
    add(2, 3, -3, -1, 0);
    add(2, 3, -3, 1, 0);
    add(2, 3, -2, -2, 0);
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_interior_pruning_cell(long long q) {
    if (q < 5)
        throw std::domain_error(
            "Class-II neighbor-2 pruning cell requires q >= 5");
    std::set<SNode<3>> result;
    const auto add = [&](long long i, long long x0, long long x1,
                         long long x2, long long j) {
        result.insert({i, {x0, x1, x2}, j});
    };
    add(0, -q + 2, q - 1, -1, 1);
    add(0, -q + 2, q - 1, -2, 0);
    add(0, -q + 1, q - 1, -1, 1);
    add(0, -q + 1, q - 1, -2, 0);
    add(0, -q + 1, q, 1, 0);
    add(0, -q + 1, q, 1, 1);
    add(0, -q, q, 1, 0);
    add(0, -q, q, 1, 1);
    add(0, q - 3, -q + 2, 2, 1);
    add(0, q - 2, -q + 2, 2, 1);
    add(0, q - 2, -q + 1, 2, 0);
    add(0, q - 1, -q + 1, 0, 1);
    add(0, q - 1, -q + 1, 2, 0);
    add(0, q - 1, -q, -1, 0);
    add(0, q - 1, -q, 0, 1);
    add(0, q, -q, -1, 0);
    add(1, -q + 2, q - 1, -1, 1);
    add(1, -q + 2, q - 1, -2, 0);
    add(1, -q + 2, q - 1, 0, 0);
    add(1, -q + 2, q - 1, 0, 1);
    add(1, -q + 1, q - 1, -1, 1);
    add(1, -q + 1, q - 1, -2, 0);
    add(1, -q + 1, q - 1, 0, 0);
    add(1, -q + 1, q - 1, 0, 1);
    add(1, q - 2, -q + 1, 1, 1);
    add(1, q - 1, -q + 1, 0, 1);
    add(1, q - 1, -q + 1, 1, 1);
    add(1, q - 1, -q, -1, 0);
    add(1, q - 1, -q, 0, 1);
    add(1, q - 1, -q, 1, 0);
    add(1, q, -q, -1, 0);
    add(1, q, -q, 1, 0);
    add(2, -q + 1, q, -1, 1);
    add(2, -q + 1, q, -2, 0);
    add(2, -q + 1, q, 0, 0);
    add(2, -q + 1, q, 0, 1);
    add(2, -q, q, -1, 1);
    add(2, -q, q, -2, 0);
    add(2, -q, q, 0, 0);
    add(2, -q, q, 0, 1);
    add(2, q - 3, -q + 2, 1, 1);
    add(2, q - 2, -q + 2, 0, 1);
    add(2, q - 2, -q + 2, 1, 1);
    add(2, q - 2, -q + 1, -1, 0);
    add(2, q - 2, -q + 1, 0, 1);
    add(2, q - 2, -q + 1, 1, 0);
    add(2, q - 1, -q + 1, -1, 0);
    add(2, q - 1, -q + 1, 1, 0);
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_interior_pruning_frontier(long long q) {
    if (q < 4)
        throw std::domain_error(
            "Class-II neighbor-2 pruning frontier requires q >= 4");
    return {
        {0, {-(q - 1), q, -1}, 0},
        {0, {-q, q, -1}, 0},
        {0, {q - 1, -q, 1}, 0},
        {0, {q, -q, 1}, 0},
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_interior_pruning_rank1(std::size_t round) {
    if (round < 4)
        throw std::domain_error(
            "Class-II neighbor-2 stable pruning requires round >= 4");
    auto result = class_ii_neighbor2_interior_pruning_rank1_seed();
    for (long long q = 5; q <= static_cast<long long>(round); ++q) {
        const auto old_frontier =
            class_ii_neighbor2_interior_pruning_frontier(q - 1);
        for (const auto& node : old_frontier) result.erase(node);
        const auto cell = class_ii_neighbor2_interior_pruning_cell(q);
        result.insert(cell.begin(), cell.end());
        const auto frontier =
            class_ii_neighbor2_interior_pruning_frontier(q);
        result.insert(frontier.begin(), frontier.end());
    }
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_interior_pruning_rank2() {
    std::set<SNode<3>> result;
    const auto add = [&](long long i, long long x0, long long x1,
                         long long x2, long long j) {
        result.insert({i, {x0, x1, x2}, j});
    };
    add(0, -3, 3, 1, 1);
    add(0, -2, 1, 1, 0);
    add(0, -2, 2, 1, 0);
    add(0, -2, 3, 1, 0);
    add(0, 1, -2, 0, 1);
    add(0, 1, -1, 0, 1);
    add(0, 2, -3, -1, 0);
    add(0, 2, -2, -1, 0);
    add(0, 2, -1, -1, 0);
    add(1, -2, 2, 0, 1);
    add(1, -1, 1, 0, 0);
    add(1, -1, 2, 0, 0);
    add(1, 2, -2, 0, 1);
    add(1, 3, -3, -1, 0);
    add(2, -3, 3, 0, 1);
    add(2, -2, 2, 0, 0);
    add(2, -2, 3, 0, 0);
    add(2, 1, -2, 0, 1);
    add(2, 2, -3, -1, 0);
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_interior_pruning_rank3() {
    std::set<SNode<3>> result;
    const auto add = [&](long long i, long long x0, long long x1,
                         long long x2, long long j) {
        result.insert({i, {x0, x1, x2}, j});
    };
    add(0, -2, 2, 1, 2);
    add(0, -2, 3, 1, 1);
    add(1, -1, 1, 0, 2);
    add(1, -1, 2, 0, 1);
    add(1, 1, -2, 0, 1);
    add(1, 2, -3, -1, 0);
    add(2, -2, 3, 0, 1);
    add(2, 1, -1, 0, 1);
    add(2, 2, -2, -1, 0);
    return result;
}

inline std::vector<std::set<SNode<3>>>
class_ii_neighbor2_interior_pruning_ranks(std::size_t round) {
    return {
        class_ii_neighbor2_interior_pruning_rank1(round),
        class_ii_neighbor2_interior_pruning_rank2(),
        class_ii_neighbor2_interior_pruning_rank3(),
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_penultimate_promoted_states(long long a) {
    const long long q = a - 1;
    return {
        {0, {q - 2, -(q - 2), 2}, 1},
        {2, {-(q - 2), q - 2, -2}, 0},
        {2, {-(q - 1), q - 1, -1}, 1},
        {2, {-q, q, -1}, 1},
        {2, {-q, q, -2}, 0},
        {2, {q - 2, -(q - 2), 1}, 1},
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_penultimate_survivor_transfer(long long a) {
    const long long q = a - 1;
    return {{2, {-(q - 1), q - 1, -2}, 0}};
}

// Threads the CONCRETE promoted/transferred nodes at a specific `a`
// into the trace (if one is active). Matches lean/class_ii_six_
// vertex_graduation.lean's `promotedNodes`/`transferredNode` (already
// independently cross-checked at runtime for a in [5,32] by
// tests/lean_class_ii_catalogue_cross_check_test.cpp, entry [1]).
inline void class_ii_six_vertex_graduation_reflect(long long a) {
    if (!mathlib::reflection::enabled()) return;
    const auto promoted = class_ii_neighbor2_penultimate_promoted_states(a);
    const auto transfer = class_ii_neighbor2_penultimate_survivor_transfer(a);
    if (promoted.size() != 6 || transfer.size() != 1) return;
    mathlib::reflection::ClassIISixVertexGraduationCertificate node;
    node.a = a;
    std::size_t idx = 0;
    for (const auto& n : promoted) node.promoted[idx++] = {n.i, n.x[0], n.x[1], n.x[2], n.j};
    const auto& t = *transfer.begin();
    node.transferred = {t.i, t.x[0], t.x[1], t.x[2], t.j};
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

inline std::vector<std::set<SNode<3>>>
class_ii_neighbor2_penultimate_pruning_ranks(long long a) {
    if (a < 5)
        throw std::domain_error(
            "Class-II neighbor-2 penultimate pruning requires a >= 5");
    const long long q = a - 1;
    auto result = class_ii_neighbor2_interior_pruning_ranks(
        static_cast<std::size_t>(q));
    const auto promoted =
        class_ii_neighbor2_penultimate_promoted_states(a);
    const auto removed_only =
        class_ii_neighbor2_penultimate_survivor_transfer(a);
    for (const auto& node : promoted) {
        result[0].erase(node);
        result[1].insert(node);
    }
    for (const auto& node : removed_only) result[0].erase(node);
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_rank1_removed(long long q) {
    return {
        {0, {q - 3, -(q - 3), 2}, 1},
        {0, {q - 3, -(q - 2), 2}, 1},
        {0, {q - 2, -(q - 2), 2}, 0},
        {0, {q - 2, -(q - 2), 2}, 1},
        {1, {-(q - 3), q - 3, -2}, 0},
        {1, {-(q - 2), q - 2, -1}, 1},
        {1, {-(q - 2), q - 2, -2}, 0},
        {1, {-(q - 1), q - 1, -1}, 1},
        {1, {-(q - 1), q - 1, -2}, 0},
        {1, {q - 2, -(q - 2), 1}, 1},
        {1, {q - 1, -(q - 1), 1}, 1},
        {2, {-(q - 3), q - 3, -2}, 0},
        {2, {-(q - 3), q - 2, -2}, 0},
        {2, {-(q - 2), q - 2, -1}, 1},
        {2, {-(q - 2), q - 2, -2}, 0},
        {2, {-(q - 2), q - 1, -1}, 1},
        {2, {-(q - 2), q - 1, -2}, 0},
        {2, {-(q - 1), q - 1, -1}, 1},
        {2, {-(q - 1), q - 1, -2}, 0},
        {2, {-(q - 1), q, -1}, 1},
        {2, {-(q - 1), q, -2}, 0},
        {2, {-q, q, -1}, 1},
        {2, {-q, q, -2}, 0},
        {2, {q - 3, -(q - 3), 1}, 1},
        {2, {q - 3, -(q - 2), 1}, 1},
        {2, {q - 2, -(q - 2), 1}, 0},
        {2, {q - 2, -(q - 2), 1}, 1},
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_rank1_added(long long q) {
    return {
        {2, {-(q - 3), q - 3, -3}, 0},
        {2, {-(q - 3), q - 2, -3}, 0},
        {2, {-(q - 2), q - 2, -2}, 1},
        {2, {-(q - 2), q - 2, -3}, 0},
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_rank2_added(long long q) {
    return {
        {0, {-q, q, 0}, 0},
        {0, {q - 3, -(q - 3), 2}, 1},
        {0, {q - 3, -(q - 2), 2}, 1},
        {0, {q - 2, -(q - 2), 2}, 0},
        {0, {q, -q, 0}, 0},
        {1, {-(q - 2), q - 2, -1}, 1},
        {1, {-(q - 1), q - 1, -1}, 1},
        {1, {-(q - 1), q - 1, -2}, 0},
        {1, {q - 2, -(q - 2), 1}, 1},
        {1, {q - 1, -(q - 1), 1}, 1},
        {1, {q - 1, -q, 1}, 2},
        {2, {-(q - 3), q - 3, -2}, 0},
        {2, {-(q - 2), q - 2, -1}, 1},
        {2, {-(q - 2), q - 1, -1}, 1},
        {2, {-(q - 1), q - 1, -2}, 0},
        {2, {-(q - 1), q, -1}, 1},
        {2, {-(q - 1), q, -1}, 2},
        {2, {-(q - 1), q, -2}, 0},
        {2, {-q, q, -1}, 0},
        {2, {q - 3, -(q - 3), 1}, 1},
        {2, {q - 3, -(q - 2), 1}, 1},
        {2, {q - 2, -(q - 2), 1}, 0},
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_rank3_added(long long q) {
    return {
        {1, {-(q - 3), q - 3, -2}, 0},
        {2, {-(q - 3), q - 2, -2}, 0},
    };
}

inline std::vector<std::set<SNode<3>>>
class_ii_neighbor2_terminal_pruning_ranks(long long a) {
    if (a < 5)
        throw std::domain_error(
            "Class-II neighbor-2 terminal pruning requires a >= 5");
    const long long q = a;
    auto result = class_ii_neighbor2_interior_pruning_ranks(
        static_cast<std::size_t>(q));
    const auto rank1_removed =
        class_ii_neighbor2_terminal_rank1_removed(q);
    const auto rank1_added =
        class_ii_neighbor2_terminal_rank1_added(q);
    for (const auto& node : rank1_removed) result[0].erase(node);
    result[0].insert(rank1_added.begin(), rank1_added.end());

    const auto rank2_added =
        class_ii_neighbor2_terminal_rank2_added(q);
    result[1].insert(rank2_added.begin(), rank2_added.end());
    const auto rank3_added =
        class_ii_neighbor2_terminal_rank3_added(q);
    result[2].insert(rank3_added.begin(), rank3_added.end());
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_new_rank1_sources(long long a) {
    if (a < 7)
        throw std::domain_error(
            "Class-II terminal new rank-one sources require a >= 7");
    const auto terminal =
        class_ii_neighbor2_terminal_pruning_ranks(a)[0];
    const auto inherited =
        class_ii_neighbor2_interior_pruning_rank1(
            static_cast<std::size_t>(a - 2));
    std::set<SNode<3>> result;
    for (const auto& node : terminal)
        if (!inherited.count(node)) result.insert(node);
    return result;
}

inline std::vector<std::set<SNode<3>>>
class_ii_neighbor2_fixed_pruning_ranks(long long a) {
    if (a < 5)
        throw std::domain_error(
            "Class-II neighbor-2 fixed pruning requires a >= 5");
    const long long q = a;
    auto result = class_ii_neighbor2_terminal_pruning_ranks(a);
    const auto edit = [&](std::size_t rank,
                          const std::set<SNode<3>>& removed,
                          const std::set<SNode<3>>& added) {
        for (const auto& node : removed) result[rank].erase(node);
        result[rank].insert(added.begin(), added.end());
    };
    edit(0, {
        {0, {-q + 2, q - 2, -2}, 0},
        {0, {-q + 1, q - 1, -1}, 1},
        {0, {-q + 1, q - 1, -2}, 0},
        {0, {-q + 1, q, -1}, 0},
        {0, {-q, q, -1}, 0},
        {0, {q - 4, -q + 3, 2}, 1},
        {0, {q - 3, -q + 3, 2}, 0},
        {0, {q - 2, -q + 1, 2}, 0},
        {0, {q - 1, -q + 1, 2}, 0},
        {0, {q - 1, -q, 1}, 0},
        {0, {q, -q, 1}, 0},
        {1, {-q + 3, q - 2, -2}, 0},
        {1, {-q + 2, q - 1, -1}, 1},
        {1, {-q + 2, q - 1, -2}, 0},
        {1, {q - 2, -q + 1, 1}, 1},
        {1, {q - 1, -q + 1, 1}, 0},
        {1, {q, -q, 1}, 0},
        {2, {q - 4, -q + 3, 1}, 1},
        {2, {q - 3, -q + 3, 1}, 0},
        {2, {q - 2, -q + 1, 1}, 0},
        {2, {q - 1, -q + 1, 1}, 0},
    }, {
        {0, {-q + 1, q, -1}, 1},
        {0, {-q + 1, q, -1}, 2},
        {0, {-q + 1, q, -2}, 0},
        {0, {-q, q + 1, -1}, 0},
        {0, {-q, q + 1, 0}, 2},
        {0, {-q, q + 1, 1}, 0},
        {0, {-q, q + 1, 1}, 1},
        {0, {-q - 1, q + 1, 1}, 1},
        {0, {q - 1, -q, 2}, 0},
        {0, {q, -q - 1, -1}, 0},
        {0, {q, -q - 1, 0}, 2},
        {0, {q, -q - 1, 1}, 0},
        {1, {-q + 3, q - 3, -3}, 0},
        {1, {-q + 3, q - 2, -3}, 0},
        {1, {-q + 2, q - 2, -2}, 1},
        {1, {-q + 2, q - 2, -3}, 0},
        {1, {-q + 1, q, -1}, 2},
        {1, {-q + 1, q, -2}, 0},
        {1, {-q + 1, q, 0}, 0},
        {1, {-q + 1, q, 0}, 1},
        {1, {-q, q, 0}, 0},
        {1, {-q, q, 0}, 1},
        {1, {q, -q, 0}, 1},
        {1, {q, -q - 1, -1}, 0},
        {1, {q, -q - 1, 0}, 1},
        {1, {q, -q - 1, 0}, 2},
        {1, {q, -q - 1, 1}, 0},
        {1, {q + 1, -q - 1, -1}, 0},
        {2, {-q + 3, q - 1, -3}, 0},
        {2, {-q + 2, q - 1, -2}, 1},
        {2, {-q + 2, q - 1, -2}, 2},
        {2, {-q + 2, q - 1, -3}, 0},
        {2, {-q + 1, q, -2}, 1},
        {2, {-q + 1, q, -2}, 2},
        {2, {-q + 1, q, -3}, 0},
        {2, {-q, q + 1, -1}, 2},
        {2, {-q, q + 1, -2}, 0},
        {2, {-q, q + 1, 0}, 0},
        {2, {-q, q + 1, 0}, 1},
        {2, {-q, q + 1, 0}, 2},
        {2, {-q - 1, q + 1, 0}, 1},
        {2, {q - 1, -q + 1, 0}, 1},
        {2, {q - 1, -q, -1}, 0},
        {2, {q - 1, -q, 0}, 1},
        {2, {q - 1, -q, 1}, 0},
        {2, {q, -q, -1}, 0},
        {2, {q, -q, 1}, 0},
    });
    edit(1, {
        {0, {-q, q, 0}, 0},
        {0, {q - 3, -q + 3, 2}, 1},
        {0, {q - 3, -q + 2, 2}, 1},
        {0, {q - 2, -q + 2, 2}, 0},
        {0, {q, -q, 0}, 0},
        {1, {-q + 2, q - 2, -1}, 1},
        {1, {-q + 1, q - 1, -1}, 1},
        {1, {q - 1, -q + 1, 1}, 1},
        {1, {q - 1, -q, 1}, 2},
        {2, {-q + 3, q - 3, -2}, 0},
        {2, {-q + 2, q - 1, -1}, 1},
        {2, {-q + 1, q - 1, -2}, 0},
        {2, {-q + 1, q, -1}, 1},
        {2, {-q, q, -1}, 0},
        {2, {q - 3, -q + 3, 1}, 1},
        {2, {q - 3, -q + 2, 1}, 1},
        {2, {q - 2, -q + 2, 1}, 0},
    }, {
        {0, {-q + 1, q - 1, -1}, 1},
        {0, {-q, q + 1, 0}, 0},
        {0, {-q - 1, q + 1, 0}, 1},
        {0, {q - 4, -q + 3, 2}, 1},
        {0, {q - 3, -q + 3, 2}, 0},
        {0, {q - 2, -q + 1, 2}, 0},
        {0, {q, -q - 1, 0}, 0},
        {0, {q, -q - 1, 1}, 2},
        {1, {-q + 2, q - 1, -1}, 1},
        {1, {-q + 1, q, -1}, 1},
        {1, {-q, q, -1}, 0},
        {1, {q - 2, -q + 1, 1}, 1},
        {1, {q - 1, -q + 1, 1}, 0},
        {1, {q - 1, -q, 1}, 1},
        {1, {q, -q, 1}, 0},
        {1, {q + 1, -q - 1, 0}, 0},
        {2, {-q, q + 1, -1}, 0},
        {2, {q - 4, -q + 3, 1}, 1},
        {2, {q - 3, -q + 3, 1}, 0},
        {2, {q - 2, -q + 1, 1}, 0},
        {2, {q - 1, -q + 1, 1}, 1},
        {2, {q - 1, -q, 1}, 2},
    });
    edit(2, {
        {1, {-q + 3, q - 3, -2}, 0},
        {2, {-q + 3, q - 2, -2}, 0},
    }, {
        {0, {-q, q + 1, 0}, 1},
        {0, {q - 1, -q, 1}, 1},
        {1, {-q + 2, q - 2, -1}, 1},
        {1, {-q + 1, q, -1}, 0},
        {1, {q, -q - 1, 0}, 0},
        {1, {q, -q - 1, 1}, 2},
        {2, {-q + 3, q - 3, -2}, 0},
        {2, {-q + 2, q - 1, -1}, 1},
        {2, {-q + 1, q - 1, -2}, 0},
        {2, {-q + 1, q + 1, -1}, 0},
        {2, {-q, q + 1, -1}, 1},
        {2, {q - 1, -q, 0}, 0},
        {2, {q, -q, 0}, 0},
    });
    result.push_back({
        {0, {-q + 1, q - 1, -2}, 0},
        {0, {-q + 1, q, -1}, 0},
        {0, {-q, q, -1}, 0},
        {0, {-q, q, 0}, 0},
        {0, {q - 2, -q + 1, 2}, 1},
        {0, {q - 1, -q + 1, 2}, 0},
        {0, {q - 1, -q, 1}, 0},
        {0, {q - 1, -q, 2}, 2},
        {0, {q, -q, 0}, 0},
        {0, {q, -q, 1}, 0},
        {1, {-q + 2, q - 1, -2}, 0},
        {2, {-q + 2, q, -2}, 0},
        {2, {-q, q, -1}, 0},
        {2, {q - 2, -q + 1, 1}, 1},
        {2, {q - 1, -q + 1, 1}, 0},
    });
    result.push_back({
        {0, {-q + 2, q - 2, -2}, 0},
        {0, {q - 3, -q + 2, 2}, 1},
        {0, {q - 2, -q + 2, 2}, 0},
        {1, {-q + 3, q - 2, -2}, 0},
        {1, {-q + 1, q - 1, -1}, 1},
        {1, {q - 1, -q + 1, 1}, 1},
        {1, {q - 1, -q, 1}, 2},
        {2, {-q + 1, q, -1}, 1},
        {2, {q - 3, -q + 2, 1}, 1},
        {2, {q - 2, -q + 2, 1}, 0},
    });
    result.push_back({
        {0, {q - 3, -q + 3, 2}, 1},
        {1, {-q + 3, q - 3, -2}, 0},
        {2, {-q + 3, q - 2, -2}, 0},
        {2, {q - 3, -q + 3, 1}, 1},
    });
    return result;
}

struct ClassIINeighbor2InteriorPruningGrammarCertificate {
    bool seed_exact = false;
    bool cell_exact = false;
    bool ranks_disjoint = false;
    bool endpoint_shapes = false;
    std::size_t stable_from = 4;
    std::size_t rank2_size = 0;
    std::size_t rank3_size = 0;

    bool exact() const {
        return seed_exact && cell_exact && ranks_disjoint
            && endpoint_shapes
            && rank2_size == 19 && rank3_size == 9;
    }
};

inline ClassIINeighbor2InteriorPruningGrammarCertificate
class_ii_neighbor2_interior_pruning_grammar_certificate() {
    ClassIINeighbor2InteriorPruningGrammarCertificate result;
    const auto seed = class_ii_neighbor2_interior_pruning_rank1_seed();
    const auto cell5 = class_ii_neighbor2_interior_pruning_cell(5);
    const auto cell6 = class_ii_neighbor2_interior_pruning_cell(6);
    const auto frontier4 =
        class_ii_neighbor2_interior_pruning_frontier(4);
    const auto frontier5 =
        class_ii_neighbor2_interior_pruning_frontier(5);
    const auto rank2 = class_ii_neighbor2_interior_pruning_rank2();
    const auto rank3 = class_ii_neighbor2_interior_pruning_rank3();
    result.seed_exact = seed.size() == 184;
    result.cell_exact =
        cell5.size() == 48 && cell6.size() == 48
        && frontier4.size() == 4 && frontier5.size() == 4;
    result.rank2_size = rank2.size();
    result.rank3_size = rank3.size();
    result.ranks_disjoint = true;
    for (std::size_t round : {4u, 5u, 8u}) {
        const auto rank1 =
            class_ii_neighbor2_interior_pruning_rank1(round);
        for (const auto& node : rank2)
            result.ranks_disjoint =
                result.ranks_disjoint && rank1.count(node) == 0;
        for (const auto& node : rank3)
            result.ranks_disjoint =
                result.ranks_disjoint
                && rank1.count(node) == 0 && rank2.count(node) == 0;
        result.cell_exact =
            result.cell_exact && rank1.size() == 48 * round - 8;
    }
    result.endpoint_shapes = true;
    for (long long a : {5LL, 6LL, 8LL}) {
        const auto penultimate =
            class_ii_neighbor2_penultimate_pruning_ranks(a);
        const auto terminal =
            class_ii_neighbor2_terminal_pruning_ranks(a);
        const auto fixed = class_ii_neighbor2_fixed_pruning_ranks(a);
        result.endpoint_shapes =
            result.endpoint_shapes
            && penultimate.size() == 3
            && penultimate[0].size()
                == static_cast<std::size_t>(48 * (a - 1) - 15)
            && penultimate[1].size() == 25
            && penultimate[2].size() == 9
            && terminal.size() == 3
            && terminal[0].size()
                == static_cast<std::size_t>(48 * a - 31)
            && terminal[1].size() == 41
            && terminal[2].size() == 11
            && fixed.size() == 6
            && fixed[0].size()
                == static_cast<std::size_t>(48 * a - 5)
            && fixed[1].size() == 46
            && fixed[2].size() == 22
            && fixed[3].size() == 15
            && fixed[4].size() == 10
            && fixed[5].size() == 4;
    }
    return result;
}

struct ClassIINodeAffineParameterForm {
    long long i = 0;
    std::array<ClassIIAffineValue, 3> x{};
    long long j = 0;
};

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_affine_forms_from_consecutive_sets(
        const std::set<SNode<3>>& left_set,
        const std::set<SNode<3>>& right_set,
        long long left_parameter) {
    std::vector<ClassIINodeAffineParameterForm> result;
    for (const auto& left : left_set) {
        std::vector<SNode<3>> matches;
        for (const auto& right : right_set) {
            if (left.i != right.i || left.j != right.j
                    || left.x[2] != right.x[2]
                    || left.x[0] + left.x[1]
                        != right.x[0] + right.x[1]) {
                continue;
            }
            bool unit_motion = true;
            for (std::size_t coordinate = 0; coordinate < 2; ++coordinate)
                unit_motion =
                    unit_motion
                    && std::abs(right.x[coordinate]
                                - left.x[coordinate]) <= 1;
            if (unit_motion) matches.push_back(right);
        }
        if (matches.size() != 1)
            throw std::logic_error(
                "Class-II affine role is not unique");
        ClassIINodeAffineParameterForm form;
        form.i = left.i;
        form.j = left.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
            const long long slope =
                matches[0].x[coordinate] - left.x[coordinate];
            form.x[coordinate] = {
                left.x[coordinate] - left_parameter * slope, slope};
        }
        result.push_back(form);
    }
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_affine_forms_from_ordered_sets(
        const std::set<SNode<3>>& left_set,
        const std::set<SNode<3>>& right_set,
        long long left_parameter) {
    if (left_set.size() != right_set.size())
        throw std::logic_error("Class-II ordered affine role size mismatch");
    std::vector<ClassIINodeAffineParameterForm> result;
    auto left = left_set.begin();
    auto right = right_set.begin();
    for (; left != left_set.end(); ++left, ++right) {
        if (left->i != right->i || left->j != right->j)
            throw std::logic_error(
                "Class-II ordered affine role color mismatch");
        ClassIINodeAffineParameterForm form;
        form.i = left->i;
        form.j = left->j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
            const long long slope =
                right->x[coordinate] - left->x[coordinate];
            form.x[coordinate] = {
                left->x[coordinate] - left_parameter * slope, slope};
        }
        result.push_back(form);
    }
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_affine_forms_from_three_sets(
        const std::set<SNode<3>>& left_set,
        const std::set<SNode<3>>& right_set,
        const std::set<SNode<3>>& third_set,
        long long left_parameter,
        const std::set<SNode<3>>* validation_set = nullptr,
        long long validation_parameter = 0) {
    if (left_set.size() != right_set.size()
            || left_set.size() != third_set.size())
        throw std::logic_error(
            "Class-II three-set affine role size mismatch");
    const std::vector<SNode<3>> left(left_set.begin(), left_set.end());
    const std::vector<SNode<3>> right(right_set.begin(), right_set.end());
    std::vector<std::vector<std::size_t>> candidates(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) {
        for (std::size_t j = 0; j < right.size(); ++j) {
            if (left[i].i != right[j].i || left[i].j != right[j].j)
                continue;
            SNode<3> extrapolated = right[j];
            bool bounded_motion = true;
            for (std::size_t coordinate = 0; coordinate < 3;
                 ++coordinate) {
                const long long delta =
                    right[j].x[coordinate] - left[i].x[coordinate];
                bounded_motion =
                    bounded_motion && std::abs(delta) <= 1;
                extrapolated.x[coordinate] += delta;
            }
            if (!bounded_motion || !third_set.count(extrapolated))
                continue;
            if (validation_set != nullptr) {
                auto validation = left[i];
                for (std::size_t coordinate = 0; coordinate < 3;
                     ++coordinate) {
                    const long long slope =
                        right[j].x[coordinate]
                        - left[i].x[coordinate];
                    validation.x[coordinate] +=
                        (validation_parameter - left_parameter) * slope;
                }
                if (!validation_set->count(validation)) continue;
            }
            candidates[i].push_back(j);
        }
        if (candidates[i].empty())
            throw std::logic_error(
                "Class-II three-set affine role has no continuation");
    }
    std::vector<long long> right_match(right.size(), -1);
    const auto augment = [&](auto&& self, std::size_t i,
                             std::vector<bool>& seen) -> bool {
        for (const auto j : candidates[i]) {
            if (seen[j]) continue;
            seen[j] = true;
            if (right_match[j] < 0
                    || self(self,
                        static_cast<std::size_t>(right_match[j]),
                        seen)) {
                right_match[j] = static_cast<long long>(i);
                return true;
            }
        }
        return false;
    };
    for (std::size_t i = 0; i < left.size(); ++i) {
        std::vector<bool> seen(right.size(), false);
        if (!augment(augment, i, seen))
            throw std::logic_error(
                "Class-II three-set affine roles have no perfect matching");
    }
    std::vector<std::size_t> left_match(left.size());
    for (std::size_t j = 0; j < right.size(); ++j)
        left_match[static_cast<std::size_t>(right_match[j])] = j;
    std::vector<ClassIINodeAffineParameterForm> result;
    result.reserve(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) {
        const auto& l = left[i];
        const auto& r = right[left_match[i]];
        ClassIINodeAffineParameterForm form;
        form.i = l.i;
        form.j = l.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
            const long long slope =
                r.x[coordinate] - l.x[coordinate];
            form.x[coordinate] = {
                l.x[coordinate] - left_parameter * slope, slope};
        }
        result.push_back(form);
    }
    return result;
}

// Recover the 48 role-preserving affine forms from two consecutive
// literal cells.  Matching is unique after fixing colors, x2, the
// x0+x1 hyperplane, and unit coordinate motion.
inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_pruning_cell_forms() {
    return class_ii_affine_forms_from_consecutive_sets(
        class_ii_neighbor2_interior_pruning_cell(5),
        class_ii_neighbor2_interior_pruning_cell(6), 5);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_pruning_frontier_forms() {
    return class_ii_affine_forms_from_consecutive_sets(
        class_ii_neighbor2_interior_pruning_frontier(5),
        class_ii_neighbor2_interior_pruning_frontier(6), 5);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_center_interior_shell_forms() {
    return class_ii_affine_forms_from_consecutive_sets(
        class_ii_interior_shell(5), class_ii_interior_shell(6), 5);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_rank1_removed_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_rank1_removed(7),
        class_ii_neighbor2_terminal_rank1_removed(8), 7);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_rank1_added_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_rank1_added(7),
        class_ii_neighbor2_terminal_rank1_added(8), 7);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_rank2_added_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_rank2_added(7),
        class_ii_neighbor2_terminal_rank2_added(8), 7);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_rank3_added_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_rank3_added(7),
        class_ii_neighbor2_terminal_rank3_added(8), 7);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_terminal_shell_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_terminal_shell(7), class_ii_terminal_shell(8), 7);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_new_rank1_source_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_new_rank1_sources(7),
        class_ii_neighbor2_terminal_new_rank1_sources(8), 7);
}

struct ClassIIPruningConeForm {
    long long constant = 0;
    long long a = 0;
    long long source_parameter = 0;
    long long target_parameter = 0;
};

inline ClassIIPruningConeForm operator+(
        ClassIIPruningConeForm left, ClassIIPruningConeForm right) {
    return {
        left.constant + right.constant,
        left.a + right.a,
        left.source_parameter + right.source_parameter,
        left.target_parameter + right.target_parameter,
    };
}

inline ClassIIPruningConeForm operator-(
        ClassIIPruningConeForm left, ClassIIPruningConeForm right) {
    return {
        left.constant - right.constant,
        left.a - right.a,
        left.source_parameter - right.source_parameter,
        left.target_parameter - right.target_parameter,
    };
}

inline ClassIIPruningConeForm operator-(ClassIIPruningConeForm value) {
    return {
        -value.constant, -value.a,
        -value.source_parameter, -value.target_parameter};
}

// Cone: a>=7 and 5<=q,s<=a-2, split into q=s, q<s, and s<q.
// Region 0 is equality, 1 is source<target, 2 is target<source.
inline bool class_ii_pruning_cone_nonnegative(
        ClassIIPruningConeForm value,
        int region) {
    const long long base_a = region == 0 ? 7 : 8;
    const long long base_q = region == 2 ? 6 : 5;
    const long long base_s = region == 1 ? 6 : 5;
    const long long base =
        value.constant + base_a * value.a
        + base_q * value.source_parameter
        + base_s * value.target_parameter;
    return base >= 0
        && value.a >= 0
        && value.a + value.source_parameter
             + value.target_parameter >= 0
        && (region == 0
                || (region == 1
                    ? value.a + value.target_parameter >= 0
                    : value.a + value.source_parameter >= 0));
}

inline bool class_ii_pruning_cone_positive(
        ClassIIPruningConeForm value,
        int region) {
    const long long base_a = region == 0 ? 7 : 8;
    const long long base_q = region == 2 ? 6 : 5;
    const long long base_s = region == 1 ? 6 : 5;
    const long long base =
        value.constant + base_a * value.a
        + base_q * value.source_parameter
        + base_s * value.target_parameter;
    return base > 0
        && value.a >= 0
        && value.a + value.source_parameter
             + value.target_parameter >= 0
        && (region == 0
                || (region == 1
                    ? value.a + value.target_parameter >= 0
                    : value.a + value.source_parameter >= 0));
}

inline bool class_ii_pruning_cone_zero(
        ClassIIPruningConeForm value) {
    return value.constant == 0 && value.a == 0
        && value.source_parameter == 0
        && value.target_parameter == 0;
}

inline ClassIIPruningConeForm class_ii_pruning_a_form(
        ClassIIAffineValue value) {
    return {value.intercept, value.slope, 0, 0};
}

inline bool class_ii_pruning_linear_system_feasible(
        std::vector<ClassIIPruningConeForm> constraints) {
    const auto coefficients = [](ClassIIPruningConeForm value) {
        return std::array<long long, 4>{
            value.constant, value.a,
            value.source_parameter, value.target_parameter};
    };
    const auto from_coefficients = [](const std::array<long long, 4>& c) {
        return ClassIIPruningConeForm{c[0], c[1], c[2], c[3]};
    };
    const auto normalize = [&](ClassIIPruningConeForm value) {
        auto c = coefficients(value);
        long long divisor = 0;
        for (const auto coefficient : c)
            divisor = std::gcd(divisor, std::abs(coefficient));
        if (divisor > 1)
            for (auto& coefficient : c) coefficient /= divisor;
        return from_coefficients(c);
    };
    for (int variable = 1; variable <= 3; ++variable) {
        std::vector<ClassIIPruningConeForm> positive;
        std::vector<ClassIIPruningConeForm> negative;
        std::vector<ClassIIPruningConeForm> zero;
        for (const auto constraint : constraints) {
            const auto c = coefficients(constraint);
            if (c[variable] > 0) positive.push_back(constraint);
            else if (c[variable] < 0) negative.push_back(constraint);
            else zero.push_back(constraint);
        }
        std::set<std::array<long long, 4>> unique;
        for (const auto constraint : zero)
            unique.insert(coefficients(normalize(constraint)));
        if (!positive.empty() && !negative.empty()) {
            for (const auto upper : positive) {
                const auto u = coefficients(upper);
                for (const auto lower : negative) {
                    const auto l = coefficients(lower);
                    std::array<long long, 4> combined{};
                    for (std::size_t coordinate = 0;
                         coordinate < 4; ++coordinate) {
                        if (l[variable]
                                == std::numeric_limits<long long>::min()) {
                            throw std::overflow_error(
                                "Class-II Fourier-Motzkin coefficient "
                                "overflow");
                        }
                        long long first = 0;
                        long long second = 0;
                        long long value = 0;
                        if (__builtin_mul_overflow(
                                -l[variable], u[coordinate], &first)
                                || __builtin_mul_overflow(
                                    u[variable], l[coordinate], &second)
                                || __builtin_add_overflow(
                                    first, second, &value)) {
                            throw std::overflow_error(
                                "Class-II Fourier-Motzkin coefficient "
                                "overflow");
                        }
                        combined[coordinate] = value;
                    }
                    unique.insert(coefficients(
                        normalize(from_coefficients(combined))));
                }
            }
        }
        constraints.clear();
        for (const auto& constraint : unique)
            constraints.push_back(from_coefficients(constraint));
    }
    for (const auto constraint : constraints) {
        if (constraint.constant < 0) return false;
    }
    return true;
}

inline bool class_ii_neighbor2_branch_is_zero_under(
        const ClassIISymbolicPrefixFamily& left,
        const ClassIISymbolicPrefixFamily& right,
        const std::array<ClassIIPruningConeForm, 3>& delta,
        std::vector<ClassIIPruningConeForm> constraints) {
    // All constraints are integer affine forms required to be >= 0.
    // Strict integer inequalities f>0 are recorded as f-1>=0.
    const auto require_zero = [&](ClassIIPruningConeForm value) {
        constraints.push_back(value);
        constraints.push_back(-value);
    };
    const auto require_positive = [&](ClassIIPruningConeForm value) {
        --value.constant;
        constraints.push_back(value);
    };
    const auto p_length = class_ii_pruning_a_form(left.length);
    const auto q_length = class_ii_pruning_a_form(right.length);
    if (left.varying_coordinate == right.varying_coordinate) {
        const auto coordinate = left.varying_coordinate;
        for (std::size_t other = 0; other < 3; ++other) {
            if (other == coordinate) continue;
            require_zero(delta[other]);
        }
        const auto d = delta[coordinate];
        require_positive(p_length - d);
        require_positive(q_length + d);
        return !class_ii_pruning_linear_system_feasible(
            std::move(constraints));
    }
    const auto pc = left.varying_coordinate;
    const auto qc = right.varying_coordinate;
    for (std::size_t other = 0; other < 3; ++other) {
        if (other == pc || other == qc) continue;
        require_zero(delta[other]);
    }
    constraints.push_back(delta[pc]);
    require_positive(p_length - delta[pc]);
    constraints.push_back(-delta[qc]);
    require_positive(q_length + delta[qc]);
    return !class_ii_pruning_linear_system_feasible(
        std::move(constraints));
}

inline bool class_ii_neighbor2_cell_branch_is_zero(
        const ClassIISymbolicPrefixFamily& left,
        const ClassIISymbolicPrefixFamily& right,
        const std::array<ClassIIPruningConeForm, 3>& delta,
        bool target_le_source = false) {
    std::vector<ClassIIPruningConeForm> constraints = {
        {-7, 1, 0, 0},   // a >= 7
        {-5, 0, 1, 0},   // q >= 5
        {-5, 0, 0, 1},   // s >= 5
        {-2, 1, -1, 0},  // q <= a-2
        {-2, 1, 0, -1},  // s <= a-2
    };
    if (target_le_source)
        constraints.push_back({0, 0, 1, -1});
    return class_ii_neighbor2_branch_is_zero_under(
        left, right, delta, std::move(constraints));
}

inline long long class_ii_floor_div(long long numerator,
                                    long long denominator) {
    if (denominator <= 0)
        throw std::domain_error("Class-II floor divisor must be positive");
    long long quotient = numerator / denominator;
    const long long remainder = numerator % denominator;
    if (remainder < 0) --quotient;
    return quotient;
}

inline long long class_ii_ceil_div(long long numerator,
                                   long long denominator) {
    if (denominator <= 0)
        throw std::domain_error("Class-II ceil divisor must be positive");
    return -class_ii_floor_div(-numerator, denominator);
}

// Exact integer feasibility after substituting q=a+source_offset and
// s=a+target_offset. This closes endpoint cases that a rational
// Fourier--Motzkin relaxation must conservatively leave unresolved.
inline bool class_ii_neighbor2_branch_is_zero_on_endpoint(
        const ClassIISymbolicPrefixFamily& left,
        const ClassIISymbolicPrefixFamily& right,
        const std::array<ClassIIPruningConeForm, 3>& delta,
        long long source_offset, long long target_offset,
        long long minimum_a = 7) {
    std::vector<ClassIIPruningConeForm> constraints = {
        {-minimum_a, 1, 0, 0},
    };
    const auto require_zero = [&](ClassIIPruningConeForm value) {
        constraints.push_back(value);
        constraints.push_back(-value);
    };
    const auto require_positive = [&](ClassIIPruningConeForm value) {
        --value.constant;
        constraints.push_back(value);
    };
    const auto p_length = class_ii_pruning_a_form(left.length);
    const auto q_length = class_ii_pruning_a_form(right.length);
    if (left.varying_coordinate == right.varying_coordinate) {
        const auto coordinate = left.varying_coordinate;
        for (std::size_t other = 0; other < 3; ++other)
            if (other != coordinate) require_zero(delta[other]);
        require_positive(p_length - delta[coordinate]);
        require_positive(q_length + delta[coordinate]);
    } else {
        const auto pc = left.varying_coordinate;
        const auto qc = right.varying_coordinate;
        for (std::size_t other = 0; other < 3; ++other)
            if (other != pc && other != qc) require_zero(delta[other]);
        constraints.push_back(delta[pc]);
        require_positive(p_length - delta[pc]);
        constraints.push_back(-delta[qc]);
        require_positive(q_length + delta[qc]);
    }
    long long lower = minimum_a;
    long long upper = std::numeric_limits<long long>::max();
    for (const auto& constraint : constraints) {
        long long offset_term = 0;
        long long target_term = 0;
        long long constant = 0;
        long long slope = 0;
        if (__builtin_mul_overflow(
                constraint.source_parameter, source_offset,
                &offset_term)
                || __builtin_mul_overflow(
                    constraint.target_parameter, target_offset,
                    &target_term)
                || __builtin_add_overflow(
                    constraint.constant, offset_term, &constant)
                || __builtin_add_overflow(
                    constant, target_term, &constant)
                || __builtin_add_overflow(
                    constraint.a, constraint.source_parameter, &slope)
                || __builtin_add_overflow(
                    slope, constraint.target_parameter, &slope)) {
            throw std::overflow_error(
                "Class-II endpoint constraint overflow");
        }
        if (slope == 0) {
            if (constant < 0) return true;
        } else if (slope > 0) {
            lower = std::max(
                lower, class_ii_ceil_div(-constant, slope));
        } else {
            upper = std::min(
                upper, class_ii_floor_div(constant, -slope));
        }
    }
    return lower > upper;
}

struct ClassIIAffinePairExclusionResult {
    bool exact = true;
    std::size_t source_count = 0;
    std::size_t target_count = 0;
    std::size_t pair_count = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;
    std::vector<std::pair<ClassIINodeAffineParameterForm,
                          ClassIINodeAffineParameterForm>>
        unresolved_pairs;
};

inline ClassIIAffinePairExclusionResult
class_ii_neighbor2_affine_pair_exclusion(
        const std::vector<ClassIINodeAffineParameterForm>& sources,
        const std::vector<ClassIINodeAffineParameterForm>& targets,
        bool target_le_source = false,
        const std::vector<ClassIIPruningConeForm>&
            domain_constraints = {}) {
    ClassIIAffinePairExclusionResult result;
    // Prefix families depend only on the two finite-state indices, not
    // on the affine coordinates.  Terminal grammars repeat those index
    // pairs heavily, so keep a bounded, call-local table instead of
    // regenerating their exact branch descriptions for every node pair.
    std::map<std::pair<std::size_t, std::size_t>,
             std::vector<ClassIISymbolicPrefixFamily>> prefix_cache;
    const auto prefixes = [&](long long target_index,
                              long long source_index)
            -> const std::vector<ClassIISymbolicPrefixFamily>& {
        const auto key = std::make_pair(
            static_cast<std::size_t>(target_index),
            static_cast<std::size_t>(source_index));
        const auto found = prefix_cache.find(key);
        if (found != prefix_cache.end()) return found->second;
        return prefix_cache.emplace(
            key, class_ii_neighbor_symbolic_prefix_families(
                2, key.first, key.second)).first->second;
    };
    result.source_count = sources.size();
    result.target_count = targets.size();
    for (const auto& source : sources) {
        for (const auto& target : targets) {
            ++result.pair_count;
            const auto& left = prefixes(target.i, source.i);
            const auto& right = prefixes(target.j, source.j);
            const long long target_sum_slope =
                target.x[0].slope + target.x[1].slope;
            if (target.x[2].slope != 0 || target_sum_slope != 0) {
                result.exact = false;
                continue;
            }
            const std::array<ClassIIPruningConeForm, 3> mx = {{
                {
                    target.x[2].intercept,
                    target.x[0].intercept + target.x[1].intercept,
                    0, 0,
                },
                {target.x[0].intercept, 0, 0, target.x[0].slope},
                {
                    target.x[0].intercept + target.x[1].intercept,
                    0, 0, target_sum_slope,
                },
            }};
            for (const auto& p : left) {
                for (const auto& q : right) {
                    ++result.prefix_branches;
                    std::array<ClassIIPruningConeForm, 3> delta{};
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        delta[coordinate] = {
                            source.x[coordinate].intercept
                                + q.base[coordinate].intercept
                                - p.base[coordinate].intercept,
                            q.base[coordinate].slope
                                - p.base[coordinate].slope,
                            source.x[coordinate].slope,
                            0,
                        };
                        delta[coordinate] =
                            delta[coordinate] - mx[coordinate];
                    }
                    const bool branch_zero =
                        domain_constraints.empty()
                        ? class_ii_neighbor2_cell_branch_is_zero(
                            p, q, delta, target_le_source)
                        : class_ii_neighbor2_branch_is_zero_under(
                            p, q, delta, domain_constraints);
                    if (!branch_zero) {
                        ++result.unresolved_branches;
                        result.unresolved_pairs.emplace_back(
                            source, target);
                    }
                    result.exact = result.exact && branch_zero;
                }
            }
        }
    }
    return result;
}

inline ClassIIAffinePairExclusionResult
class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
        const std::vector<ClassIINodeAffineParameterForm>& sources,
        const std::vector<ClassIINodeAffineParameterForm>& targets,
        long long source_offset, long long target_offset,
        long long minimum_a = 7) {
    ClassIIAffinePairExclusionResult result;
    std::map<std::pair<std::size_t, std::size_t>,
             std::vector<ClassIISymbolicPrefixFamily>> prefix_cache;
    const auto prefixes = [&](long long target_index,
                              long long source_index)
            -> const std::vector<ClassIISymbolicPrefixFamily>& {
        const auto key = std::make_pair(
            static_cast<std::size_t>(target_index),
            static_cast<std::size_t>(source_index));
        const auto found = prefix_cache.find(key);
        if (found != prefix_cache.end()) return found->second;
        return prefix_cache.emplace(
            key, class_ii_neighbor_symbolic_prefix_families(
                2, key.first, key.second)).first->second;
    };
    result.source_count = sources.size();
    result.target_count = targets.size();
    for (const auto& source : sources) {
        for (const auto& target : targets) {
            ++result.pair_count;
            const auto& left = prefixes(target.i, source.i);
            const auto& right = prefixes(target.j, source.j);
            const std::array<ClassIIPruningConeForm, 3> mx = {{
                {
                    target.x[2].intercept,
                    target.x[0].intercept
                        + target.x[1].intercept,
                    0, 0,
                },
                {
                    target.x[0].intercept,
                    0, 0, target.x[0].slope,
                },
                {
                    target.x[0].intercept
                        + target.x[1].intercept,
                    0, 0, 0,
                },
            }};
            for (const auto& p : left) {
                for (const auto& q : right) {
                    ++result.prefix_branches;
                    std::array<ClassIIPruningConeForm, 3> delta{};
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        delta[coordinate] = {
                            source.x[coordinate].intercept
                                + q.base[coordinate].intercept
                                - p.base[coordinate].intercept,
                            q.base[coordinate].slope
                                - p.base[coordinate].slope,
                            source.x[coordinate].slope, 0,
                        };
                        delta[coordinate] =
                            delta[coordinate] - mx[coordinate];
                    }
                    const bool zero =
                        class_ii_neighbor2_branch_is_zero_on_endpoint(
                            p, q, delta,
                            source_offset, target_offset, minimum_a);
                    if (!zero) {
                        ++result.unresolved_branches;
                        result.unresolved_pairs.emplace_back(
                            source, target);
                    }
                    result.exact = result.exact && zero;
                }
            }
        }
    }
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_constant_node_forms(const std::set<SNode<3>>& nodes) {
    std::vector<ClassIINodeAffineParameterForm> result;
    for (const auto& node : nodes) {
        ClassIINodeAffineParameterForm form;
        form.i = node.i;
        form.j = node.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
            form.x[coordinate] = {node.x[coordinate], 0};
        result.push_back(form);
    }
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_stable_affine_target_forms() {
    auto result = class_ii_neighbor2_pruning_cell_forms();
    const auto frontier = class_ii_neighbor2_pruning_frontier_forms();
    result.insert(result.end(), frontier.begin(), frontier.end());
    const auto shells = class_ii_center_interior_shell_forms();
    result.insert(result.end(), shells.begin(), shells.end());
    result.push_back({
        2,
        {{{0, -1}, {0, 1}, {-1, 0}}},
        0,
    });
    return result;
}

// Parameter q is the penultimate round index q=a-1.
inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_penultimate_promoted_forms() {
    return {
        {0, {{{-2, 1}, {2, -1}, {2, 0}}}, 1},
        {2, {{{2, -1}, {-2, 1}, {-2, 0}}}, 0},
        {2, {{{1, -1}, {-1, 1}, {-1, 0}}}, 1},
        {2, {{{0, -1}, {0, 1}, {-1, 0}}}, 1},
        {2, {{{0, -1}, {0, 1}, {-2, 0}}}, 0},
        {2, {{{-2, 1}, {2, -1}, {1, 0}}}, 1},
    };
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_penultimate_transfer_forms() {
    return {
        {2, {{{1, -1}, {-1, 1}, {-2, 0}}}, 0},
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_stable_fixed_target_nodes() {
    auto result = class_ii_neighbor2_interior_pruning_rank1_seed();
    const auto rank2 = class_ii_neighbor2_interior_pruning_rank2();
    const auto rank3 = class_ii_neighbor2_interior_pruning_rank3();
    const auto correction =
        class_ii_neighbor2_fixed_extension_states();
    const auto center_base = class_ii_stable_base();
    result.insert(rank2.begin(), rank2.end());
    result.insert(rank3.begin(), rank3.end());
    result.insert(correction.begin(), correction.end());
    result.insert(center_base.begin(), center_base.end());
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_stable_pre_red_candidate(
        long long a, std::size_t round) {
    if (a < 7 || round < 5
            || round > static_cast<std::size_t>(a - 2)) {
        throw std::domain_error(
            "Class-II neighbor-2 stable pre-Red candidate domain");
    }
    auto result = class_ii_center_layer_candidate(
        static_cast<std::size_t>(a), round);
    const auto correction =
        class_ii_neighbor2_layer_extension(a, round);
    result.insert(correction.begin(), correction.end());
    const auto pruning =
        class_ii_neighbor2_interior_pruning_ranks(round);
    for (const auto& rank : pruning)
        result.insert(rank.begin(), rank.end());
    return result;
}

inline std::vector<ClassIINeighbor2CompositionWitness>
class_ii_neighbor2_stable_pre_red_composition_witnesses(
        long long a, std::size_t round) {
    const auto targets =
        class_ii_neighbor2_stable_pre_red_candidate(a, round);
    auto sources = class_ii_center_layer_candidate(
        static_cast<std::size_t>(a), round - 1);
    const auto correction =
        class_ii_neighbor2_layer_extension(a, round - 1);
    sources.insert(correction.begin(), correction.end());
    auto hops = class_ii_neighbor2_signed_contact_set();
    for (long long color = 0; color < 3; ++color)
        hops.insert({color, {0, 0, 0}, color});

    std::map<SNode<3>, ClassIINeighbor2CompositionWitness> by_target;
    for (const auto& source : sources) {
        for (const auto& hop : hops) {
            if (source.j != hop.i) continue;
            SNode<3> target;
            target.i = source.i;
            target.j = hop.j;
            for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
                target.x[coordinate] =
                    source.x[coordinate] + hop.x[coordinate];
            if (targets.count(target) != 0)
                by_target.try_emplace(
                    target,
                    ClassIINeighbor2CompositionWitness{
                        source, hop, target});
        }
    }
    std::vector<ClassIINeighbor2CompositionWitness> result;
    for (const auto& target : targets) {
        const auto witness = by_target.find(target);
        if (witness != by_target.end()) result.push_back(witness->second);
    }
    return result;
}

struct ClassIINeighbor2StableCompositionCertificate {
    bool bounded_inclusion_exact = false;
    std::size_t layers_checked = 0;
    std::size_t witnesses_checked = 0;
};

inline ClassIINeighbor2StableCompositionCertificate
class_ii_neighbor2_stable_composition_certificate() {
    ClassIINeighbor2StableCompositionCertificate result;
    result.bounded_inclusion_exact = true;
    for (long long a = 7; a <= 32; ++a) {
        for (std::size_t round = 5;
             round <= static_cast<std::size_t>(a - 2); ++round) {
            const auto candidate =
                class_ii_neighbor2_stable_pre_red_candidate(a, round);
            const auto witnesses =
                class_ii_neighbor2_stable_pre_red_composition_witnesses(
                    a, round);
            std::set<SNode<3>> targets;
            for (const auto& witness : witnesses) {
                targets.insert(witness.target);
                bool coordinates_match = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    coordinates_match =
                        coordinates_match
                        && witness.source.x[coordinate]
                            + witness.hop.x[coordinate]
                            == witness.target.x[coordinate];
                }
                result.bounded_inclusion_exact =
                    result.bounded_inclusion_exact
                    && witness.source.j == witness.hop.i
                    && witness.source.i == witness.target.i
                    && witness.hop.j == witness.target.j
                    && coordinates_match;
            }
            result.bounded_inclusion_exact =
                result.bounded_inclusion_exact && targets == candidate;
            ++result.layers_checked;
            result.witnesses_checked += witnesses.size();
        }
    }
    return result;
}

inline bool class_ii_affine_node_form_equal(
        const ClassIINodeAffineParameterForm& left,
        const ClassIINodeAffineParameterForm& right) {
    if (left.i != right.i || left.j != right.j) return false;
    for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
        if (left.x[coordinate].intercept
                    != right.x[coordinate].intercept
                || left.x[coordinate].slope
                    != right.x[coordinate].slope) {
            return false;
        }
    }
    return true;
}

struct ClassIINeighbor2StableAffineCompositionCertificate {
    bool fixed_exact = false;
    bool affine_exact = false;
    std::size_t fixed_targets = 0;
    std::size_t affine_targets = 0;
    std::size_t source_roles = 0;
    std::size_t hop_roles = 0;

    bool exact() const {
        return fixed_exact && affine_exact
            && fixed_targets == 304 && affine_targets == 73
            && source_roles == 113 && hop_roles == 53;
    }
};

// Universal algebraic inclusion half of the stable corona theorem.
// Fixed targets compose from the round-four survivor grammar. Every
// affine target role composes from either a fixed source or the
// preceding shell/tip role; equality is checked coefficientwise, so
// one finite table covers every parameter value.
inline ClassIINeighbor2StableAffineCompositionCertificate
class_ii_neighbor2_stable_affine_composition_certificate() {
    ClassIINeighbor2StableAffineCompositionCertificate result;
    auto hops = class_ii_neighbor2_signed_contact_set();
    for (long long color = 0; color < 3; ++color)
        hops.insert({color, {0, 0, 0}, color});
    result.hop_roles = hops.size();

    auto fixed_sources = class_ii_center_layer_candidate(7, 4);
    const auto correction =
        class_ii_neighbor2_layer_extension(7, 4);
    fixed_sources.insert(correction.begin(), correction.end());
    result.source_roles = fixed_sources.size();
    const auto fixed_targets =
        class_ii_neighbor2_stable_fixed_target_nodes();
    result.fixed_targets = fixed_targets.size();
    result.fixed_exact = true;
    for (const auto& target : fixed_targets) {
        bool found = false;
        for (const auto& source : fixed_sources) {
            for (const auto& hop : hops) {
                if (source.j != hop.i
                        || source.i != target.i
                        || hop.j != target.j) {
                    continue;
                }
                bool coordinates_match = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    coordinates_match =
                        coordinates_match
                        && source.x[coordinate] + hop.x[coordinate]
                            == target.x[coordinate];
                }
                if (coordinates_match) {
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        result.fixed_exact = result.fixed_exact && found;
    }

    auto affine_sources = class_ii_constant_node_forms(
        class_ii_stable_base());
    const auto fixed_correction_forms = class_ii_constant_node_forms(
        class_ii_neighbor2_fixed_extension_states());
    affine_sources.insert(
        affine_sources.end(),
        fixed_correction_forms.begin(), fixed_correction_forms.end());
    auto preceding_shells = class_ii_center_interior_shell_forms();
    for (auto& source : preceding_shells)
        for (auto& coordinate : source.x)
            coordinate.intercept -= coordinate.slope;
    affine_sources.insert(
        affine_sources.end(),
        preceding_shells.begin(), preceding_shells.end());
    // tip(s-1) = (2,(-(s-1),s-1,-1),0).
    affine_sources.push_back({
        2,
        {{{1, -1}, {-1, 1}, {-1, 0}}},
        0,
    });
    const auto affine_targets =
        class_ii_neighbor2_stable_affine_target_forms();
    result.affine_targets = affine_targets.size();
    result.affine_exact = true;
    for (const auto& target : affine_targets) {
        bool found = false;
        for (const auto& source : affine_sources) {
            for (const auto& hop : hops) {
                if (source.j != hop.i
                        || source.i != target.i
                        || hop.j != target.j) {
                    continue;
                }
                auto composition = source;
                composition.j = hop.j;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    composition.x[coordinate].intercept +=
                        hop.x[coordinate];
                }
                if (class_ii_affine_node_form_equal(
                        composition, target)) {
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        result.affine_exact = result.affine_exact && found;
    }
    return result;
}

using ClassIINodeAffineRoleKey = std::array<long long, 8>;

inline ClassIINodeAffineRoleKey class_ii_affine_node_role_key(
        const ClassIINodeAffineParameterForm& form) {
    return {
        form.i,
        form.x[0].intercept, form.x[0].slope,
        form.x[1].intercept, form.x[1].slope,
        form.x[2].intercept, form.x[2].slope,
        form.j,
    };
}

struct ClassIIStrictLinearConstraint {
    // constant + a*A + c*C + d*D + e*E >= 0; `strict` means >0.
    std::array<long long, 5> coefficients{};
    bool strict = false;
};

// Exact Fourier--Motzkin elimination over the four scalar quantities
// used by the stepped window:
//   c=a+1/beta, d=beta-c, e=(a-2)d.
// Strictness is retained when either eliminated inequality is strict.
inline bool class_ii_strict_linear_system_feasible(
        std::vector<ClassIIStrictLinearConstraint> constraints) {
    const auto normalize = [](ClassIIStrictLinearConstraint value) {
        long long divisor = 0;
        for (const auto coefficient : value.coefficients)
            divisor = std::gcd(divisor, std::abs(coefficient));
        if (divisor > 1)
            for (auto& coefficient : value.coefficients)
                coefficient /= divisor;
        return value;
    };
    for (std::size_t variable = 1; variable < 5; ++variable) {
        std::vector<ClassIIStrictLinearConstraint> positive;
        std::vector<ClassIIStrictLinearConstraint> negative;
        std::vector<ClassIIStrictLinearConstraint> zero;
        for (const auto& constraint : constraints) {
            if (constraint.coefficients[variable] > 0)
                positive.push_back(constraint);
            else if (constraint.coefficients[variable] < 0)
                negative.push_back(constraint);
            else
                zero.push_back(constraint);
        }
        std::map<std::array<long long, 5>, bool> unique;
        for (auto constraint : zero) {
            constraint = normalize(constraint);
            unique[constraint.coefficients] =
                unique[constraint.coefficients] || constraint.strict;
        }
        if (!positive.empty() && !negative.empty()) {
            for (const auto& upper : positive) {
                for (const auto& lower : negative) {
                    ClassIIStrictLinearConstraint combined;
                    combined.strict = upper.strict || lower.strict;
                    for (std::size_t coordinate = 0;
                         coordinate < 5; ++coordinate) {
                        if (lower.coefficients[variable]
                                == std::numeric_limits<long long>::min()) {
                            throw std::overflow_error(
                                "Class-II strict Fourier-Motzkin "
                                "coefficient overflow");
                        }
                        long long first = 0;
                        long long second = 0;
                        if (__builtin_mul_overflow(
                                -lower.coefficients[variable],
                                upper.coefficients[coordinate], &first)
                                || __builtin_mul_overflow(
                                    upper.coefficients[variable],
                                    lower.coefficients[coordinate], &second)
                                || __builtin_add_overflow(
                                    first, second,
                                    &combined.coefficients[coordinate])) {
                            throw std::overflow_error(
                                "Class-II strict Fourier-Motzkin "
                                "coefficient overflow");
                        }
                    }
                    combined = normalize(combined);
                    unique[combined.coefficients] =
                        unique[combined.coefficients] || combined.strict;
                }
            }
        }
        constraints.clear();
        for (const auto& [coefficients, strict] : unique)
            constraints.push_back({coefficients, strict});
    }
    for (const auto& constraint : constraints) {
        const auto constant = constraint.coefficients[0];
        if (constant < 0 || (constant == 0 && constraint.strict))
            return false;
    }
    return true;
}

inline std::vector<ClassIIStrictLinearConstraint>
class_ii_neighbor2_window_scalar_assumptions() {
    return {
        {{{-7, 1, 0, 0, 0}}, false},  // a >= 7
        {{{0, -1, 1, 0, 0}}, true},   // c > a
        {{{1, 1, -1, 0, 0}}, true},   // c < a+1
        {{{-2, 0, 0, 3, 0}}, true},   // d > 2/3
        {{{1, 0, 0, -1, 0}}, true},   // d < 1
        {{{3, 0, -1, 0, 1}}, true},   // e > c-3
        {{{-2, 0, 1, 0, -1}}, true},  // e < c-2
        {{{2, 0, -1, 1, 1}}, true},   // d+e+2 > c
        {{{-3, 0, 1, 1, -1}}, true},  // c+d > e+3
        {{{0, 0, 1, -3, -1}}, true},  // c > 3d+e
        {{{1, 0, -1, 2, 1}}, true},   // e+2d+1 > c
        {{{1, 0, 1, -2, -1}}, true},  // c+1 > e+2d
    };
}

// sign*height-width at q=5 or q=a-2.  The width is selected by the
// target color: beta=c+d, c, or 1.
inline ClassIIStrictLinearConstraint
class_ii_neighbor2_window_margin(
        const ClassIINodeAffineParameterForm& form,
        long long sign, bool upper_parameter_endpoint) {
    ClassIIStrictLinearConstraint result;
    const long long c_coefficient =
        form.x[0].intercept + form.x[1].intercept;
    const long long q_coefficient = form.x[0].slope;
    result.coefficients[2] = sign * c_coefficient;
    result.coefficients[0] = sign * form.x[2].intercept;
    if (upper_parameter_endpoint) {
        result.coefficients[3] =
            sign * form.x[0].intercept;
        result.coefficients[4] = sign * q_coefficient;
    } else {
        result.coefficients[3] =
            sign * (form.x[0].intercept + 5 * q_coefficient);
    }
    if (form.j == 0) {
        --result.coefficients[2];
        --result.coefficients[3];
    } else if (form.j == 1) {
        --result.coefficients[2];
    } else {
        --result.coefficients[0];
    }
    return result;
}

// sign*height-width at q=a. Here q*d=e+2d because
// e=(a-2)d.
inline ClassIIStrictLinearConstraint
class_ii_neighbor2_terminal_window_margin(
        const ClassIINodeAffineParameterForm& form,
        long long sign) {
    ClassIIStrictLinearConstraint result;
    const long long c_coefficient =
        form.x[0].intercept + form.x[1].intercept;
    const long long q_coefficient = form.x[0].slope;
    result.coefficients[2] = sign * c_coefficient;
    result.coefficients[0] = sign * form.x[2].intercept;
    result.coefficients[3] =
        sign * (form.x[0].intercept + 2 * q_coefficient);
    result.coefficients[4] = sign * q_coefficient;
    if (form.j == 0) {
        --result.coefficients[2];
        --result.coefficients[3];
    } else if (form.j == 1) {
        --result.coefficients[2];
    } else {
        --result.coefficients[0];
    }
    return result;
}

inline bool class_ii_neighbor2_window_margin_strictly_negative(
        ClassIIStrictLinearConstraint margin) {
    // Disprove margin >= 0 under the scalar assumptions.
    auto constraints = class_ii_neighbor2_window_scalar_assumptions();
    margin.strict = false;
    constraints.push_back(margin);
    return !class_ii_strict_linear_system_feasible(
        std::move(constraints));
}

inline bool class_ii_neighbor2_affine_form_window_valid(
        const ClassIINodeAffineParameterForm& form) {
    for (const long long sign : {-1LL, 1LL}) {
        if (!class_ii_neighbor2_window_margin_strictly_negative(
                    class_ii_neighbor2_window_margin(
                        form, sign, false))
                || !class_ii_neighbor2_window_margin_strictly_negative(
                    class_ii_neighbor2_window_margin(
                        form, sign, true))) {
            return false;
        }
    }
    return true;
}

inline bool class_ii_neighbor2_affine_form_window_invalid(
        const ClassIINodeAffineParameterForm& form) {
    // Convexity in q makes equal-sign endpoint exclusion sufficient.
    for (const long long sign : {-1LL, 1LL}) {
        auto lower = class_ii_neighbor2_window_margin(
            form, sign, false);
        auto upper = class_ii_neighbor2_window_margin(
            form, sign, true);
        // Prove margin >= 0 at both endpoints by disproving margin < 0.
        auto lower_constraints =
            class_ii_neighbor2_window_scalar_assumptions();
        for (auto& coefficient : lower.coefficients)
            coefficient = -coefficient;
        lower.strict = true;
        lower_constraints.push_back(lower);
        auto upper_constraints =
            class_ii_neighbor2_window_scalar_assumptions();
        for (auto& coefficient : upper.coefficients)
            coefficient = -coefficient;
        upper.strict = true;
        upper_constraints.push_back(upper);
        if (!class_ii_strict_linear_system_feasible(
                    std::move(lower_constraints))
                && !class_ii_strict_linear_system_feasible(
                    std::move(upper_constraints))) {
            return true;
        }
    }
    return false;
}

inline bool class_ii_neighbor2_affine_form_terminal_valid(
        const ClassIINodeAffineParameterForm& form) {
    for (const long long sign : {-1LL, 1LL})
        if (!class_ii_neighbor2_window_margin_strictly_negative(
                class_ii_neighbor2_terminal_window_margin(
                    form, sign)))
            return false;
    return true;
}

inline bool class_ii_neighbor2_affine_form_terminal_invalid(
        const ClassIINodeAffineParameterForm& form) {
    for (const long long sign : {-1LL, 1LL}) {
        auto margin =
            class_ii_neighbor2_terminal_window_margin(form, sign);
        for (auto& coefficient : margin.coefficients)
            coefficient = -coefficient;
        margin.strict = true;
        auto constraints =
            class_ii_neighbor2_window_scalar_assumptions();
        constraints.push_back(margin);
        if (!class_ii_strict_linear_system_feasible(
                std::move(constraints)))
            return true;
    }
    return false;
}

inline bool class_ii_neighbor2_constant_instance_of_affine_role(
        const ClassIINodeAffineParameterForm& constant,
        const std::vector<ClassIINodeAffineParameterForm>& roles) {
    for (const auto& role : roles) {
        if (constant.i != role.i || constant.j != role.j) continue;
        bool parameter_set = false;
        bool matches = true;
        long long parameter = 0;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
            if (role.x[coordinate].slope == 0) {
                matches = matches
                    && constant.x[coordinate].slope == 0
                    && constant.x[coordinate].intercept
                        == role.x[coordinate].intercept;
                continue;
            }
            const long long difference =
                constant.x[coordinate].intercept
                - role.x[coordinate].intercept;
            if (difference % role.x[coordinate].slope != 0) {
                matches = false;
                break;
            }
            const long long candidate =
                difference / role.x[coordinate].slope;
            if (!parameter_set) {
                parameter = candidate;
                parameter_set = true;
            } else {
                matches = matches && parameter == candidate;
            }
        }
        if (matches && parameter_set && parameter >= 4) return true;
    }
    return false;
}

inline bool class_ii_neighbor2_shifted_affine_role(
        const ClassIINodeAffineParameterForm& composition,
        const std::vector<ClassIINodeAffineParameterForm>& roles,
        long long& shift,
        ClassIINodeAffineParameterForm& matched_role) {
    for (const auto& role : roles) {
        if (composition.i != role.i || composition.j != role.j)
            continue;
        bool shift_set = false;
        bool matches = true;
        long long candidate_shift = 0;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
            if (composition.x[coordinate].slope
                    != role.x[coordinate].slope) {
                matches = false;
                break;
            }
            if (role.x[coordinate].slope == 0) {
                matches = matches
                    && composition.x[coordinate].intercept
                        == role.x[coordinate].intercept;
                continue;
            }
            const long long difference =
                composition.x[coordinate].intercept
                - role.x[coordinate].intercept;
            if (difference % role.x[coordinate].slope != 0) {
                matches = false;
                break;
            }
            const long long current =
                difference / role.x[coordinate].slope;
            if (!shift_set) {
                candidate_shift = current;
                shift_set = true;
            } else {
                matches = matches && candidate_shift == current;
            }
        }
        if (matches && shift_set) {
            shift = candidate_shift;
            matched_role = role;
            return true;
        }
    }
    return false;
}

struct ClassIINeighbor2StableWindowExhaustionCertificate {
    bool target_window_validity_exact = false;
    bool reverse_inclusion_exact = false;
    std::size_t fixed_compositions = 0;
    std::size_t affine_compositions = 0;
    std::size_t fixed_invalid = 0;
    std::size_t affine_shifted = 0;
    std::size_t affine_invalid = 0;
    std::size_t boundary_instances = 0;
    std::size_t unresolved = 0;

    bool exact() const {
        return target_window_validity_exact && reverse_inclusion_exact
            && fixed_compositions == 835
            && affine_compositions == 353
            && fixed_invalid == 442
            && affine_shifted == 125
            && affine_invalid == 155
            && boundary_instances == 160
            && unresolved == 0;
    }
};

// Universal reverse-inclusion half of the stable raw-corona theorem.
// Every algebraic source-plus-contact composition is either a displayed
// target role (possibly after an index shift), a finite low-index
// evaluation of such a role, or universally outside the stepped
// Perron window under the kernel-proved scalar bounds.
inline ClassIINeighbor2StableWindowExhaustionCertificate
class_ii_neighbor2_stable_window_exhaustion_certificate() {
    ClassIINeighbor2StableWindowExhaustionCertificate result;
    auto hops = class_ii_neighbor2_signed_contact_set();
    for (long long color = 0; color < 3; ++color)
        hops.insert({color, {0, 0, 0}, color});

    auto fixed_sources = class_ii_center_layer_candidate(7, 4);
    const auto correction =
        class_ii_neighbor2_layer_extension(7, 4);
    fixed_sources.insert(correction.begin(), correction.end());
    auto affine_sources = class_ii_center_interior_shell_forms();
    for (auto& source : affine_sources)
        for (auto& coordinate : source.x)
            coordinate.intercept -= coordinate.slope;
    affine_sources.push_back({
        2, {{{1, -1}, {-1, 1}, {-1, 0}}}, 0});

    std::map<ClassIINodeAffineRoleKey,
             ClassIINodeAffineParameterForm> fixed_compositions;
    for (const auto& source : fixed_sources) {
        for (const auto& hop : hops) {
            if (source.j != hop.i) continue;
            SNode<3> node{source.i, source.x, hop.j};
            for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
                node.x[coordinate] += hop.x[coordinate];
            if (node.i == node.j
                    && node.x == std::array<long long, 3>{})
                continue;
            const auto form = class_ii_constant_node_forms({node})[0];
            fixed_compositions[class_ii_affine_node_role_key(form)] =
                form;
        }
    }
    std::map<ClassIINodeAffineRoleKey,
             ClassIINodeAffineParameterForm> affine_compositions;
    for (const auto& source : affine_sources) {
        for (const auto& hop : hops) {
            if (source.j != hop.i) continue;
            auto form = source;
            form.j = hop.j;
            for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
                form.x[coordinate].intercept += hop.x[coordinate];
            affine_compositions[
                class_ii_affine_node_role_key(form)] = form;
        }
    }
    result.fixed_compositions = fixed_compositions.size();
    result.affine_compositions = affine_compositions.size();

    const auto affine_targets =
        class_ii_neighbor2_stable_affine_target_forms();
    std::set<ClassIINodeAffineRoleKey> affine_target_keys;
    for (const auto& form : affine_targets)
        affine_target_keys.insert(class_ii_affine_node_role_key(form));
    const auto fixed_targets =
        class_ii_neighbor2_stable_fixed_target_nodes();
    std::set<ClassIINodeAffineRoleKey> fixed_target_keys;
    for (const auto& form : class_ii_constant_node_forms(fixed_targets))
        fixed_target_keys.insert(class_ii_affine_node_role_key(form));

    result.target_window_validity_exact = true;
    for (const auto& form : class_ii_constant_node_forms(fixed_targets))
        result.target_window_validity_exact =
            result.target_window_validity_exact
            && class_ii_neighbor2_affine_form_window_valid(form);
    for (const auto& form : affine_targets)
        result.target_window_validity_exact =
            result.target_window_validity_exact
            && class_ii_neighbor2_affine_form_window_valid(form);

    for (const auto& [key, form] : fixed_compositions) {
        if (fixed_target_keys.count(key)
                || class_ii_neighbor2_constant_instance_of_affine_role(
                    form, affine_targets))
            continue;
        ++result.fixed_invalid;
        if (!class_ii_neighbor2_affine_form_window_invalid(form))
            ++result.unresolved;
    }

    std::set<ClassIINodeAffineRoleKey> shell_keys;
    for (const auto& form : class_ii_center_interior_shell_forms())
        shell_keys.insert(class_ii_affine_node_role_key(form));
    for (const auto& [key, form] : affine_compositions) {
        if (affine_target_keys.count(key)
                || fixed_target_keys.count(key))
            continue;
        long long shift = 0;
        ClassIINodeAffineParameterForm matched;
        if (class_ii_neighbor2_shifted_affine_role(
                form, affine_targets, shift, matched)) {
            ++result.affine_shifted;
            const long long minimum_parameter =
                shell_keys.count(class_ii_affine_node_role_key(matched))
                ? 4 : 5;
            for (long long source_parameter = 5;
                 source_parameter + shift < minimum_parameter;
                 ++source_parameter) {
                ++result.boundary_instances;
                auto constant = form;
                for (auto& coordinate : constant.x) {
                    coordinate.intercept +=
                        source_parameter * coordinate.slope;
                    coordinate.slope = 0;
                }
                const auto constant_key =
                    class_ii_affine_node_role_key(constant);
                if (!fixed_target_keys.count(constant_key)
                        && !class_ii_neighbor2_constant_instance_of_affine_role(
                            constant, affine_targets)
                        && !class_ii_neighbor2_affine_form_window_invalid(
                            constant)) {
                    ++result.unresolved;
                }
            }
            continue;
        }
        ++result.affine_invalid;
        if (!class_ii_neighbor2_affine_form_window_invalid(form))
            ++result.unresolved;
    }
    result.reverse_inclusion_exact = result.unresolved == 0;
    return result;
}

struct ClassIINeighbor2PenultimatePartitionCertificate {
    bool survivor_transfer_exact = false;
    bool total_grammar_exact = false;
    std::size_t transferred_states = 0;
    std::size_t promoted_states = 0;
    std::size_t parameters_checked = 0;

    bool exact() const {
        return survivor_transfer_exact && total_grammar_exact
            && transferred_states == 1
            && promoted_states == 6
            && parameters_checked == 2;
    }
};

// The penultimate layer changes the Red partition, not the ambient
// pre-Red set. Six states are promoted from rank one to rank two; one
// further affine state moves from rank one into the correction
// survivor set. The latter identity is checked coefficientwise at two
// consecutive parameters.
inline ClassIINeighbor2PenultimatePartitionCertificate
class_ii_neighbor2_penultimate_partition_certificate() {
    ClassIINeighbor2PenultimatePartitionCertificate result;
    std::set<SNode<3>> extension_additions[2];
    std::set<SNode<3>> pruning_removals[2];
    std::set<SNode<3>> rank1_removals[2];
    std::set<SNode<3>> rank2_additions[2];
    result.total_grammar_exact = true;
    for (long long a : {7LL, 8LL}) {
        const std::size_t index = static_cast<std::size_t>(a - 7);
        const auto interior_extension =
            class_ii_neighbor2_interior_extension_states(
                static_cast<std::size_t>(a - 1));
        const auto penultimate_extension =
            class_ii_neighbor2_penultimate_extension_states(a);
        for (const auto& node : penultimate_extension)
            if (!interior_extension.count(node))
                extension_additions[index].insert(node);

        std::set<SNode<3>> interior_pruning;
        const auto interior_ranks =
            class_ii_neighbor2_interior_pruning_ranks(
                static_cast<std::size_t>(a - 1));
        const auto penultimate_ranks =
            class_ii_neighbor2_penultimate_pruning_ranks(a);
        for (const auto& node : interior_ranks[0])
            if (!penultimate_ranks[0].count(node))
                rank1_removals[index].insert(node);
        for (const auto& node : penultimate_ranks[1])
            if (!interior_ranks[1].count(node))
                rank2_additions[index].insert(node);
        auto redistributed = rank2_additions[index];
        redistributed.insert(
            extension_additions[index].begin(),
            extension_additions[index].end());
        result.total_grammar_exact =
            result.total_grammar_exact
            && rank1_removals[index] == redistributed
            && rank2_additions[index].size() == 6;
        for (const auto& rank : interior_ranks)
            interior_pruning.insert(rank.begin(), rank.end());
        std::set<SNode<3>> penultimate_pruning;
        for (const auto& rank : penultimate_ranks)
            penultimate_pruning.insert(rank.begin(), rank.end());
        for (const auto& node : interior_pruning)
            if (!penultimate_pruning.count(node))
                pruning_removals[index].insert(node);

        auto stable_total = class_ii_center_layer_candidate(
            static_cast<std::size_t>(a),
            static_cast<std::size_t>(a - 1));
        stable_total.insert(
            interior_extension.begin(), interior_extension.end());
        stable_total.insert(
            interior_pruning.begin(), interior_pruning.end());
        auto penultimate_total = class_ii_center_layer_candidate(
            static_cast<std::size_t>(a),
            static_cast<std::size_t>(a - 1));
        penultimate_total.insert(
            penultimate_extension.begin(), penultimate_extension.end());
        penultimate_total.insert(
            penultimate_pruning.begin(), penultimate_pruning.end());
        result.total_grammar_exact =
            result.total_grammar_exact
            && stable_total == penultimate_total;
        ++result.parameters_checked;
    }
    result.transferred_states = extension_additions[0].size();
    result.promoted_states = rank2_additions[0].size();
    result.survivor_transfer_exact =
        extension_additions[0].size() == 1
        && extension_additions[1].size() == 1
        && pruning_removals[0].size() == 1
            && pruning_removals[1].size() == 1;
    if (result.survivor_transfer_exact) {
        const auto extension_form =
            class_ii_affine_forms_from_consecutive_sets(
                extension_additions[0], extension_additions[1], 7);
        const auto pruning_form =
            class_ii_affine_forms_from_consecutive_sets(
                pruning_removals[0], pruning_removals[1], 7);
        result.survivor_transfer_exact =
            extension_form.size() == 1
            && pruning_form.size() == 1
            && class_ii_affine_node_form_equal(
                extension_form[0], pruning_form[0]);
    }
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_penultimate_pre_red_grammar(long long a) {
    auto result = class_ii_center_layer_candidate(
        static_cast<std::size_t>(a),
        static_cast<std::size_t>(a - 1));
    const auto extension =
        class_ii_neighbor2_penultimate_extension_states(a);
    result.insert(extension.begin(), extension.end());
    for (const auto& rank :
         class_ii_neighbor2_penultimate_pruning_ranks(a))
        result.insert(rank.begin(), rank.end());
    return result;
}

struct ClassIINeighbor2PenultimateRank1Certificate {
    bool base_exact = false;
    bool affine_exclusion_exact = false;
    std::size_t affine_source_roles = 0;
    std::size_t endpoint_target_roles = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;

    bool exact() const {
        return base_exact && affine_exclusion_exact
            && affine_source_roles == 369
            && endpoint_target_roles == 73
            && prefix_branches == 22373
            && unresolved_branches == 0;
    }
};

// Rank-one endpoint transport. The stable theorem already excludes
// edges whose target index is at most a-2. This certificate covers the
// only new slice, target index a-1: deep cells, the three upper cells,
// the moving frontier, and the fixed seed. Seven repartitioned affine
// roles are removed before testing. The exceptional collision at a=7
// is checked directly; the affine proof covers every a>=8.
inline ClassIINeighbor2PenultimateRank1Certificate
class_ii_neighbor2_penultimate_rank1_certificate() {
    ClassIINeighbor2PenultimateRank1Certificate result;
    const auto cells = class_ii_neighbor2_pruning_cell_forms();
    const auto frontier =
        class_ii_neighbor2_pruning_frontier_forms();
    const auto targets =
        class_ii_neighbor2_stable_affine_target_forms();
    result.endpoint_target_roles = targets.size();

    const auto shift_to_a = [](
            ClassIINodeAffineParameterForm form, long long offset) {
        for (auto& coordinate : form.x)
            coordinate.intercept += offset * coordinate.slope;
        return form;
    };
    std::set<ClassIINodeAffineRoleKey> repartitioned;
    for (auto form : class_ii_neighbor2_penultimate_promoted_forms())
        repartitioned.insert(class_ii_affine_node_role_key(
            shift_to_a(form, -1)));
    for (auto form : class_ii_neighbor2_penultimate_transfer_forms())
        repartitioned.insert(class_ii_affine_node_role_key(
            shift_to_a(form, -1)));

    // 5 <= q <= a-4, target s=a-1.
    const std::vector<ClassIIPruningConeForm> deep_domain = {
        {-9, 1, 0, 0}, {-5, 0, 1, 0}, {-4, 1, -1, 0},
        {1, -1, 0, 1}, {-1, 1, 0, -1},
    };
    const auto deep =
        class_ii_neighbor2_affine_pair_exclusion(
            cells, targets, false, deep_domain);
    result.affine_source_roles += cells.size();
    result.prefix_branches += deep.prefix_branches;
    result.unresolved_branches += deep.unresolved_branches;

    for (const long long offset : {-3LL, -2LL, -1LL}) {
        std::vector<ClassIINodeAffineParameterForm> sources;
        for (const auto& form : cells)
            if (!repartitioned.count(class_ii_affine_node_role_key(
                    shift_to_a(form, offset))))
                sources.push_back(form);
        const auto exclusion =
            class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
                sources, targets, offset, -1);
        result.affine_source_roles += sources.size();
        result.prefix_branches += exclusion.prefix_branches;
        result.unresolved_branches +=
            exclusion.unresolved_branches;
    }
    std::vector<ClassIINodeAffineParameterForm> frontier_sources;
    for (const auto& form : frontier)
        if (!repartitioned.count(class_ii_affine_node_role_key(
                shift_to_a(form, -1))))
            frontier_sources.push_back(form);
    const auto frontier_exclusion =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            frontier_sources, targets, -1, -1);
    result.affine_source_roles += frontier_sources.size();
    result.prefix_branches += frontier_exclusion.prefix_branches;
    result.unresolved_branches +=
        frontier_exclusion.unresolved_branches;

    auto fixed_seed =
        class_ii_neighbor2_interior_pruning_rank1_seed();
    for (const auto& node :
         class_ii_neighbor2_interior_pruning_frontier(4))
        fixed_seed.erase(node);
    const auto fixed_exclusion =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_constant_node_forms(fixed_seed),
            targets, -2, -1, 8);
    result.affine_source_roles += fixed_seed.size();
    result.prefix_branches += fixed_exclusion.prefix_branches;
    result.unresolved_branches +=
        fixed_exclusion.unresolved_branches;
    result.affine_exclusion_exact =
        result.unresolved_branches == 0;

    const auto ranks =
        class_ii_neighbor2_penultimate_pruning_ranks(7);
    const auto grammar =
        class_ii_neighbor2_penultimate_pre_red_grammar(7);
    result.base_exact = true;
    for (const auto& source : ranks[0]) {
        for (const auto& target : grammar) {
            if (class_ii_neighbor_transition_weight(
                    2, 7, source, target) != 0) {
                result.base_exact = false;
                break;
            }
        }
        if (!result.base_exact) break;
    }
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_hypothetical_stable_terminal_grammar(long long a) {
    auto result = class_ii_stable_base();
    for (long long q = 4; q <= a; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        result.insert(shell.begin(), shell.end());
    }
    const auto extension =
        class_ii_neighbor2_interior_extension_states(
            static_cast<std::size_t>(a));
    result.insert(extension.begin(), extension.end());
    for (const auto& rank :
         class_ii_neighbor2_interior_pruning_ranks(
             static_cast<std::size_t>(a)))
        result.insert(rank.begin(), rank.end());
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_pre_red_grammar(long long a) {
    auto result = class_ii_center_layer_candidate(
        static_cast<std::size_t>(a),
        static_cast<std::size_t>(a));
    const auto extension = class_ii_neighbor2_center_extension(a);
    result.insert(extension.begin(), extension.end());
    for (const auto& rank :
         class_ii_neighbor2_terminal_pruning_ranks(a))
        result.insert(rank.begin(), rank.end());
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_new_survivor_targets(long long a) {
    if (a < 7)
        throw std::domain_error(
            "Class-II terminal new survivor targets require a >= 7");
    auto terminal = class_ii_neighbor2_terminal_pre_red_grammar(a);
    const auto terminal_ranks =
        class_ii_neighbor2_terminal_pruning_ranks(a);
    for (const auto& node : terminal_ranks[0])
        terminal.erase(node);

    auto inherited = class_ii_center_layer_candidate(
        static_cast<std::size_t>(a),
        static_cast<std::size_t>(a - 2));
    const auto correction =
        class_ii_neighbor2_interior_extension_states(
            static_cast<std::size_t>(a - 2));
    inherited.insert(correction.begin(), correction.end());
    const auto inherited_ranks =
        class_ii_neighbor2_interior_pruning_ranks(
            static_cast<std::size_t>(a - 2));
    inherited.insert(inherited_ranks[1].begin(), inherited_ranks[1].end());
    inherited.insert(inherited_ranks[2].begin(), inherited_ranks[2].end());

    std::set<SNode<3>> result;
    for (const auto& node : terminal)
        if (!inherited.count(node)) result.insert(node);
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_target_forms();

// Exact set difference using the already-proved stable decomposition,
// avoiding the much more expensive literal center-layer rebuild.
inline std::set<SNode<3>>
class_ii_neighbor2_terminal_new_survivor_targets_decomposed(
        long long a) {
    if (a < 7)
        throw std::domain_error(
            "Class-II decomposed terminal targets require a >= 7");
    auto terminal = class_ii_stable_base();
    for (long long q = 4; q < a; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        terminal.insert(shell.begin(), shell.end());
    }
    const auto terminal_shell_nodes = class_ii_terminal_shell(a);
    terminal.insert(
        terminal_shell_nodes.begin(), terminal_shell_nodes.end());
    const auto terminal_extension =
        class_ii_neighbor2_center_extension(a);
    terminal.insert(terminal_extension.begin(), terminal_extension.end());
    const auto terminal_ranks =
        class_ii_neighbor2_terminal_pruning_ranks(a);
    terminal.insert(terminal_ranks[1].begin(), terminal_ranks[1].end());
    terminal.insert(terminal_ranks[2].begin(), terminal_ranks[2].end());
    for (const auto& node : terminal_ranks[0]) terminal.erase(node);

    auto inherited = class_ii_stable_base();
    for (long long q = 4; q <= a - 2; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        inherited.insert(shell.begin(), shell.end());
    }
    const auto inherited_extension =
        class_ii_neighbor2_interior_extension_states(
            static_cast<std::size_t>(a - 2));
    inherited.insert(
        inherited_extension.begin(), inherited_extension.end());
    const auto rank2 = class_ii_neighbor2_interior_pruning_rank2();
    const auto rank3 = class_ii_neighbor2_interior_pruning_rank3();
    inherited.insert(rank2.begin(), rank2.end());
    inherited.insert(rank3.begin(), rank3.end());

    std::set<SNode<3>> result;
    for (const auto& node : terminal)
        if (!inherited.count(node)) result.insert(node);
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_new_pre_red_targets_decomposed(
        long long a) {
    if (a < 7)
        throw std::domain_error(
            "Class-II decomposed terminal pre-Red targets require a >= 7");
    auto terminal = class_ii_stable_base();
    for (long long q = 4; q < a; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        terminal.insert(shell.begin(), shell.end());
    }
    const auto terminal_shell_nodes = class_ii_terminal_shell(a);
    terminal.insert(
        terminal_shell_nodes.begin(), terminal_shell_nodes.end());
    const auto terminal_extension =
        class_ii_neighbor2_center_extension(a);
    terminal.insert(terminal_extension.begin(), terminal_extension.end());
    for (const auto& rank :
         class_ii_neighbor2_terminal_pruning_ranks(a))
        terminal.insert(rank.begin(), rank.end());

    auto inherited = class_ii_stable_base();
    for (long long q = 4; q <= a - 2; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        inherited.insert(shell.begin(), shell.end());
    }
    const auto inherited_extension =
        class_ii_neighbor2_interior_extension_states(
            static_cast<std::size_t>(a - 2));
    inherited.insert(
        inherited_extension.begin(), inherited_extension.end());
    for (const auto& rank :
         class_ii_neighbor2_interior_pruning_ranks(
             static_cast<std::size_t>(a - 2)))
        inherited.insert(rank.begin(), rank.end());

    std::set<SNode<3>> result;
    for (const auto& node : terminal)
        if (!inherited.count(node)) result.insert(node);
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_new_pre_red_target_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_new_pre_red_targets_decomposed(7),
        class_ii_neighbor2_terminal_new_pre_red_targets_decomposed(8),
        7);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_new_survivor_target_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_new_survivor_targets_decomposed(7),
        class_ii_neighbor2_terminal_new_survivor_targets_decomposed(8),
        7);
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_new_post_rank2_targets_decomposed(
        long long a) {
    auto result =
        class_ii_neighbor2_terminal_new_survivor_targets_decomposed(a);
    const auto ranks = class_ii_neighbor2_terminal_pruning_ranks(a);
    for (const auto& node : ranks[1])
        result.erase(node);
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_new_post_rank2_target_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_new_post_rank2_targets_decomposed(7),
        class_ii_neighbor2_terminal_new_post_rank2_targets_decomposed(8),
        7);
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_new_final_survivors_decomposed(
        long long a) {
    auto result =
        class_ii_neighbor2_terminal_new_post_rank2_targets_decomposed(a);
    const auto ranks = class_ii_neighbor2_terminal_pruning_ranks(a);
    for (const auto& node : ranks[2]) result.erase(node);
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_new_final_survivor_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_new_final_survivors_decomposed(7),
        class_ii_neighbor2_terminal_new_final_survivors_decomposed(8),
        7);
}

inline std::set<SNode<3>>
class_ii_neighbor2_fixed_pre_red_decomposed(long long a) {
    if (a < 7)
        throw std::domain_error(
            "Class-II decomposed fixed pre-Red grammar requires a >= 7");
    auto fixed = class_ii_stable_base();
    for (long long q = 4; q < a; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        fixed.insert(shell.begin(), shell.end());
    }
    const auto terminal_shell_nodes = class_ii_terminal_shell(a);
    fixed.insert(
        terminal_shell_nodes.begin(), terminal_shell_nodes.end());
    const auto extension = class_ii_neighbor2_center_extension(a);
    fixed.insert(extension.begin(), extension.end());
    for (const auto& rank : class_ii_neighbor2_fixed_pruning_ranks(a))
        fixed.insert(rank.begin(), rank.end());
    return fixed;
}

inline std::set<SNode<3>>
class_ii_neighbor2_fixed_new_pre_red_targets_decomposed(long long a) {
    auto fixed = class_ii_neighbor2_fixed_pre_red_decomposed(a);

    auto inherited = class_ii_stable_base();
    for (long long q = 4; q <= a - 2; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        inherited.insert(shell.begin(), shell.end());
    }
    const auto inherited_extension =
        class_ii_neighbor2_interior_extension_states(
            static_cast<std::size_t>(a - 2));
    inherited.insert(
        inherited_extension.begin(), inherited_extension.end());
    for (const auto& rank :
         class_ii_neighbor2_interior_pruning_ranks(
             static_cast<std::size_t>(a - 2)))
        inherited.insert(rank.begin(), rank.end());

    std::set<SNode<3>> result;
    for (const auto& node : fixed)
        if (!inherited.count(node)) result.insert(node);
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_fixed_new_pre_red_target_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_fixed_new_pre_red_targets_decomposed(7),
        class_ii_neighbor2_fixed_new_pre_red_targets_decomposed(8), 7);
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_fixed_rank_forms(std::size_t rank) {
    if (rank == 0 || rank >= 6)
        throw std::out_of_range(
            "Class-II fixed affine rank requires 1 <= rank < 6");
    const auto ranks7 = class_ii_neighbor2_fixed_pruning_ranks(7);
    const auto ranks8 = class_ii_neighbor2_fixed_pruning_ranks(8);
    const auto ranks9 = class_ii_neighbor2_fixed_pruning_ranks(9);
    return class_ii_affine_forms_from_three_sets(
        ranks7[rank], ranks8[rank], ranks9[rank], 7);
}

inline std::set<SNode<3>>
class_ii_neighbor2_fixed_rank1_endpoint(long long a) {
    auto result = class_ii_neighbor2_fixed_pruning_ranks(a)[0];
    auto fixed = class_ii_neighbor2_interior_pruning_rank1_seed();
    for (const auto& node :
         class_ii_neighbor2_interior_pruning_frontier(4))
        fixed.erase(node);
    for (const auto& node : fixed) result.erase(node);
    for (long long q = 5; q <= a - 4; ++q) {
        const auto cell =
            class_ii_neighbor2_interior_pruning_cell(q);
        for (const auto& node : cell) result.erase(node);
    }
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_fixed_rank1_endpoint_forms() {
    return class_ii_affine_forms_from_three_sets(
        class_ii_neighbor2_fixed_rank1_endpoint(8),
        class_ii_neighbor2_fixed_rank1_endpoint(9),
        class_ii_neighbor2_fixed_rank1_endpoint(10), 8);
}

inline std::set<SNode<3>>
class_ii_neighbor2_fixed_bounded_targets_after(
        long long a, std::size_t ranks_removed) {
    auto result = class_ii_neighbor2_fixed_pre_red_decomposed(a);
    const auto ranks = class_ii_neighbor2_fixed_pruning_ranks(a);
    for (std::size_t rank = 0;
         rank < std::min(ranks_removed, ranks.size()); ++rank)
        for (const auto& node : ranks[rank]) result.erase(node);
    for (long long q = 4; q <= a - 2; ++q) {
        const auto shell =
            class_ii_interior_shell(static_cast<std::size_t>(q));
        for (const auto& node : shell) result.erase(node);
    }
    for (long long q = 5; q <= a - 4; ++q) {
        const auto cell =
            class_ii_neighbor2_interior_pruning_cell(q);
        for (const auto& node : cell) result.erase(node);
    }
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_fixed_bounded_target_forms_after(
        std::size_t ranks_removed) {
    const auto at8 =
        class_ii_neighbor2_fixed_bounded_targets_after(8, ranks_removed);
    const auto at9 =
        class_ii_neighbor2_fixed_bounded_targets_after(9, ranks_removed);
    const auto at10 =
        class_ii_neighbor2_fixed_bounded_targets_after(10, ranks_removed);
    const auto at20 =
        class_ii_neighbor2_fixed_bounded_targets_after(20, ranks_removed);
    return class_ii_affine_forms_from_three_sets(
        at8, at9, at10, 8, &at20, 20);
}

struct ClassIINeighbor2TerminalEditCertificate {
    bool affine_edit_exact = false;
    std::size_t removed_roles = 0;
    std::size_t added_roles = 0;
    std::size_t validation_parameters = 0;

    bool exact() const {
        return affine_edit_exact
            && removed_roles == 2 && added_roles == 8
            && validation_parameters == 3;
    }
};

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_removed_forms() {
    return {
        {2, {{{0, -1}, {0, 1}, {-2, 0}}}, 0},
        {2, {{{0, -1}, {0, 1}, {-1, 0}}}, 1},
    };
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_added_forms() {
    return {
        {0, {{{-2, 1}, {1, -1}, {2, 0}}}, 2},
        {0, {{{-1, 1}, {0, -1}, {1, 0}}}, 2},
        {1, {{{-1, 1}, {0, -1}, {1, 0}}}, 2},
        {2, {{{1, -1}, {0, 1}, {-1, 0}}}, 2},
        {2, {{{2, -1}, {-2, 1}, {-3, 0}}}, 0},
        {2, {{{2, -1}, {-2, 1}, {-2, 0}}}, 1},
        {2, {{{3, -1}, {-3, 1}, {-3, 0}}}, 0},
        {2, {{{3, -1}, {-2, 1}, {-3, 0}}}, 0},
    };
}

// Universal affine roles used by the terminal raw-corona inclusion
// theorem. The parameter of every form is the ambient Class-II a.
inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_source_forms() {
    auto result = class_ii_constant_node_forms(
        class_ii_neighbor2_center_interface_states());
    const auto fixed = class_ii_constant_node_forms(
        class_ii_neighbor2_fixed_extension_states());
    result.insert(result.end(), fixed.begin(), fixed.end());

    // Center shell q=a-1.
    for (auto form : class_ii_center_interior_shell_forms()) {
        for (auto& coordinate : form.x)
            coordinate.intercept -= coordinate.slope;
        result.push_back(form);
    }
    // Two penultimate correction roles.
    result.push_back({2, {{{1, -1}, {-1, 1}, {-1, 0}}}, 0});
    result.push_back({2, {{{2, -1}, {-2, 1}, {-2, 0}}}, 0});
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_target_forms() {
    auto result = class_ii_constant_node_forms(
        class_ii_neighbor2_fixed_extension_states());
    const std::vector<ClassIINodeAffineParameterForm> affine = {
        {0, {{{-2, 1}, {1, -1}, {2, 0}}}, 2},
        {0, {{{-2, 1}, {2, -1}, {2, 0}}}, 1},
        {1, {{{2, -1}, {-2, 1}, {-2, 0}}}, 0},
        {2, {{{2, -1}, {-2, 1}, {-2, 0}}}, 0},
        {2, {{{2, -1}, {-1, 1}, {-2, 0}}}, 0},
        {2, {{{-2, 1}, {2, -1}, {1, 0}}}, 1},
    };
    result.insert(result.end(), affine.begin(), affine.end());
    return result;
}

struct ClassIINeighbor2TerminalAffineCompositionCertificate {
    std::size_t source_roles = 0;
    std::size_t target_roles = 0;
    std::size_t witnessed_targets = 0;
    bool coefficientwise_exact = false;

    bool exact() const {
        return coefficientwise_exact
            && target_roles == 30 && witnessed_targets == 30;
    }
};

// Proves raw-corona inclusion for all displayed terminal correction
// roles by coefficientwise affine identities. No parameter samples
// or expanded substitution words enter this certificate.
inline ClassIINeighbor2TerminalAffineCompositionCertificate
class_ii_neighbor2_terminal_affine_composition_certificate() {
    ClassIINeighbor2TerminalAffineCompositionCertificate result;
    const auto sources = class_ii_neighbor2_terminal_source_forms();
    const auto targets = class_ii_neighbor2_terminal_target_forms();
    const auto hops = class_ii_neighbor2_signed_contact_set();
    result.source_roles = sources.size();
    result.target_roles = targets.size();
    result.coefficientwise_exact = true;
    for (const auto& target : targets) {
        bool found = false;
        for (const auto& source : sources) {
            if (source.i != target.i) continue;
            for (const auto& hop : hops) {
                if (source.j != hop.i || hop.j != target.j) continue;
                bool same = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    same = same
                        && source.x[coordinate].slope
                            == target.x[coordinate].slope
                        && source.x[coordinate].intercept
                                + hop.x[coordinate]
                            == target.x[coordinate].intercept;
                }
                if (same) {
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        if (found) ++result.witnessed_targets;
        else result.coefficientwise_exact = false;
    }
    return result;
}

struct ClassIINeighbor2TerminalWindowExhaustionCertificate {
    bool base_exact = false;
    bool target_validity_exact = false;
    bool reverse_inclusion_exact = false;
    bool positive_inclusion_exact = false;
    bool third_parameter_grammar_exact = false;
    std::size_t source_roles = 0;
    std::size_t affine_target_roles = 0;
    std::size_t fixed_target_roles = 0;
    std::size_t compositions = 0;
    std::size_t matched = 0;
    std::size_t shifted = 0;
    std::size_t invalid = 0;
    std::size_t unresolved = 0;
    std::size_t missing_affine_targets = 0;
    std::size_t missing_fixed_targets = 0;

    bool exact() const {
        return base_exact && target_validity_exact
            && reverse_inclusion_exact && positive_inclusion_exact
            && third_parameter_grammar_exact
            && source_roles == 135
            && affine_target_roles == 215
            && fixed_target_roles == 304
            && compositions == 1201
            && matched == 571 && shifted == 56 && invalid == 574
            && unresolved == 0
            && missing_affine_targets == 0
            && missing_fixed_targets == 0;
    }
};

// Endpoint specialization of the stable corona grammar. Earlier
// indexed sources are covered by the stable exhaustion theorem; the
// moving source family is evaluated at q=a-1 and augmented by the
// single penultimate survivor transfer.
inline ClassIINeighbor2TerminalWindowExhaustionCertificate
class_ii_neighbor2_terminal_window_exhaustion_certificate() {
    ClassIINeighbor2TerminalWindowExhaustionCertificate result;
    const auto stable =
        class_ii_neighbor2_stable_window_exhaustion_certificate();
    result.base_exact = stable.exact();

    auto fixed_sources = class_ii_center_layer_candidate(7, 4);
    const auto fixed_extension =
        class_ii_neighbor2_layer_extension(7, 4);
    fixed_sources.insert(fixed_extension.begin(), fixed_extension.end());
    auto sources = class_ii_constant_node_forms(fixed_sources);
    auto endpoint_shells = class_ii_center_interior_shell_forms();
    for (auto& form : endpoint_shells)
        for (auto& coordinate : form.x)
            coordinate.intercept -= coordinate.slope;
    sources.insert(
        sources.end(), endpoint_shells.begin(), endpoint_shells.end());
    sources.push_back({
        2, {{{1, -1}, {-1, 1}, {-1, 0}}}, 0});
    for (auto form : class_ii_neighbor2_penultimate_transfer_forms()) {
        for (auto& coordinate : form.x)
            coordinate.intercept -= coordinate.slope;
        sources.push_back(form);
    }
    result.source_roles = sources.size();

    auto affine_targets =
        class_ii_neighbor2_stable_affine_target_forms();
    const auto new_targets =
        class_ii_neighbor2_terminal_new_pre_red_target_forms();
    affine_targets.insert(
        affine_targets.end(), new_targets.begin(), new_targets.end());
    result.affine_target_roles = affine_targets.size();
    std::set<ClassIINodeAffineRoleKey> affine_target_keys;
    for (const auto& form : affine_targets)
        affine_target_keys.insert(class_ii_affine_node_role_key(form));
    const auto fixed_targets =
        class_ii_neighbor2_stable_fixed_target_nodes();
    result.fixed_target_roles = fixed_targets.size();
    std::set<ClassIINodeAffineRoleKey> fixed_target_keys;
    for (const auto& form : class_ii_constant_node_forms(fixed_targets))
        fixed_target_keys.insert(class_ii_affine_node_role_key(form));

    result.target_validity_exact = stable.target_window_validity_exact;
    for (const auto& form : new_targets)
        result.target_validity_exact =
            result.target_validity_exact
            && class_ii_neighbor2_affine_form_terminal_valid(form);

    auto hops = class_ii_neighbor2_signed_contact_set();
    for (long long color = 0; color < 3; ++color)
        hops.insert({color, {0, 0, 0}, color});
    std::map<ClassIINodeAffineRoleKey,
             ClassIINodeAffineParameterForm> compositions;
    for (const auto& source : sources) {
        for (const auto& hop : hops) {
            if (source.j != hop.i) continue;
            auto form = source;
            form.j = hop.j;
            for (std::size_t coordinate = 0;
                 coordinate < 3; ++coordinate)
                form.x[coordinate].intercept += hop.x[coordinate];
            bool zero = form.i == form.j;
            for (const auto& coordinate : form.x)
                zero = zero && coordinate.intercept == 0
                    && coordinate.slope == 0;
            if (zero) continue;
            compositions[class_ii_affine_node_role_key(form)] = form;
        }
    }
    result.compositions = compositions.size();
    for (const auto& [key, form] : compositions) {
        if (affine_target_keys.count(key)
                || fixed_target_keys.count(key)
                || class_ii_neighbor2_constant_instance_of_affine_role(
                    form, affine_targets)) {
            ++result.matched;
            continue;
        }
        long long shift = 0;
        ClassIINodeAffineParameterForm matched;
        if (class_ii_neighbor2_shifted_affine_role(
                form, affine_targets, shift, matched)
                && 7 + shift >= 4) {
            ++result.shifted;
            continue;
        }
        if (class_ii_neighbor2_affine_form_terminal_invalid(form))
            ++result.invalid;
        else
            ++result.unresolved;
    }
    result.reverse_inclusion_exact = result.unresolved == 0;
    for (const auto& key : affine_target_keys)
        result.missing_affine_targets += !compositions.count(key);
    for (const auto& key : fixed_target_keys)
        result.missing_fixed_targets += !compositions.count(key);
    result.positive_inclusion_exact =
        result.missing_affine_targets == 0
        && result.missing_fixed_targets == 0;

    std::set<SNode<3>> target_at_nine;
    for (const auto& form :
         class_ii_neighbor2_terminal_new_pre_red_target_forms()) {
        SNode<3> node;
        node.i = form.i;
        node.j = form.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
            node.x[coordinate] =
                form.x[coordinate].intercept
                + 9 * form.x[coordinate].slope;
        target_at_nine.insert(node);
    }
    result.third_parameter_grammar_exact =
        target_at_nine
            == class_ii_neighbor2_terminal_new_pre_red_targets_decomposed(9);
    return result;
}

struct ClassIINeighbor2RepeatedWindowExhaustionCertificate {
    bool base_exact = false;
    bool target_validity_exact = false;
    bool reverse_inclusion_exact = false;
    bool positive_inclusion_exact = false;
    bool third_parameter_grammar_exact = false;
    std::size_t source_roles = 0;
    std::size_t affine_target_roles = 0;
    std::size_t fixed_target_roles = 0;
    std::size_t compositions = 0;
    std::size_t matched = 0;
    std::size_t shifted = 0;
    std::size_t invalid = 0;
    std::size_t unresolved = 0;
    std::size_t missing_affine_targets = 0;
    std::size_t missing_fixed_targets = 0;

    bool exact() const {
        return base_exact && target_validity_exact
            && reverse_inclusion_exact && positive_inclusion_exact
            && third_parameter_grammar_exact
            && source_roles == 158
            && affine_target_roles == 286
            && fixed_target_roles == 304
            && compositions == 1345
            && matched == 646 && shifted == 89 && invalid == 610
            && unresolved == 0
            && missing_affine_targets == 0
            && missing_fixed_targets == 0;
    }
};

inline ClassIINeighbor2RepeatedWindowExhaustionCertificate
class_ii_neighbor2_repeated_window_exhaustion_certificate() {
    ClassIINeighbor2RepeatedWindowExhaustionCertificate result;
    const auto terminal =
        class_ii_neighbor2_terminal_window_exhaustion_certificate();
    result.base_exact = terminal.exact();

    auto fixed_sources = class_ii_center_layer_candidate(7, 4);
    const auto fixed_extension =
        class_ii_neighbor2_layer_extension(7, 4);
    fixed_sources.insert(fixed_extension.begin(), fixed_extension.end());
    auto sources = class_ii_constant_node_forms(fixed_sources);
    const auto new_survivors =
        class_ii_neighbor2_terminal_new_final_survivor_forms();
    sources.insert(
        sources.end(), new_survivors.begin(), new_survivors.end());
    result.source_roles = sources.size();

    auto affine_targets =
        class_ii_neighbor2_stable_affine_target_forms();
    const auto new_targets =
        class_ii_neighbor2_fixed_new_pre_red_target_forms();
    affine_targets.insert(
        affine_targets.end(), new_targets.begin(), new_targets.end());
    result.affine_target_roles = affine_targets.size();
    std::set<ClassIINodeAffineRoleKey> affine_target_keys;
    for (const auto& form : affine_targets)
        affine_target_keys.insert(class_ii_affine_node_role_key(form));
    const auto fixed_targets =
        class_ii_neighbor2_stable_fixed_target_nodes();
    result.fixed_target_roles = fixed_targets.size();
    std::set<ClassIINodeAffineRoleKey> fixed_target_keys;
    for (const auto& form : class_ii_constant_node_forms(fixed_targets))
        fixed_target_keys.insert(class_ii_affine_node_role_key(form));

    result.target_validity_exact =
        terminal.target_validity_exact;
    for (const auto& form : new_targets)
        result.target_validity_exact =
            result.target_validity_exact
            && class_ii_neighbor2_affine_form_terminal_valid(form);

    auto hops = class_ii_neighbor2_signed_contact_set();
    for (long long color = 0; color < 3; ++color)
        hops.insert({color, {0, 0, 0}, color});
    std::map<ClassIINodeAffineRoleKey,
             ClassIINodeAffineParameterForm> compositions;
    for (const auto& source : sources) {
        for (const auto& hop : hops) {
            if (source.j != hop.i) continue;
            auto form = source;
            form.j = hop.j;
            for (std::size_t coordinate = 0;
                 coordinate < 3; ++coordinate)
                form.x[coordinate].intercept += hop.x[coordinate];
            bool zero = form.i == form.j;
            for (const auto& coordinate : form.x)
                zero = zero && coordinate.intercept == 0
                    && coordinate.slope == 0;
            if (zero) continue;
            compositions[class_ii_affine_node_role_key(form)] = form;
        }
    }
    result.compositions = compositions.size();
    for (const auto& [key, form] : compositions) {
        if (affine_target_keys.count(key)
                || fixed_target_keys.count(key)
                || class_ii_neighbor2_constant_instance_of_affine_role(
                    form, affine_targets)) {
            ++result.matched;
            continue;
        }
        long long shift = 0;
        ClassIINodeAffineParameterForm matched;
        if (class_ii_neighbor2_shifted_affine_role(
                form, affine_targets, shift, matched)
                && 7 + shift >= 4) {
            ++result.shifted;
            continue;
        }
        if (class_ii_neighbor2_affine_form_terminal_invalid(form))
            ++result.invalid;
        else
            ++result.unresolved;
    }
    result.reverse_inclusion_exact = result.unresolved == 0;
    for (const auto& key : affine_target_keys)
        result.missing_affine_targets += !compositions.count(key);
    for (const auto& key : fixed_target_keys)
        result.missing_fixed_targets += !compositions.count(key);
    result.positive_inclusion_exact =
        result.missing_affine_targets == 0
        && result.missing_fixed_targets == 0;

    std::set<SNode<3>> target_at_nine;
    for (const auto& form :
         class_ii_neighbor2_fixed_new_pre_red_target_forms()) {
        SNode<3> node;
        node.i = form.i;
        node.j = form.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
            node.x[coordinate] =
                form.x[coordinate].intercept
                + 9 * form.x[coordinate].slope;
        target_at_nine.insert(node);
    }
    result.third_parameter_grammar_exact =
        target_at_nine
            == class_ii_neighbor2_fixed_new_pre_red_targets_decomposed(9);
    return result;
}

struct ClassIINeighbor2TerminalRank1EndpointCertificate {
    std::size_t source_roles = 0;
    std::size_t target_roles = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;
    bool endpoint_absence_exact = false;
    bool indexed_shell_absence_exact = false;
    bool source_grammar_exact = false;

    bool exact() const {
        return endpoint_absence_exact && indexed_shell_absence_exact
            && source_grammar_exact
            && unresolved_branches == 0;
    }
};

// First terminal Red sublemma: the four genuinely new rank-one roles
// have no edge to any bounded survivor/later-rank endpoint role.
// Indexed earlier shell/cell target families are a separate cone case.
inline ClassIINeighbor2TerminalRank1EndpointCertificate
class_ii_neighbor2_terminal_rank1_endpoint_certificate() {
    ClassIINeighbor2TerminalRank1EndpointCertificate result;
    const auto sources =
        class_ii_neighbor2_terminal_new_rank1_source_forms();
    auto fixed_nodes = class_ii_stable_base();
    const auto correction =
        class_ii_neighbor2_fixed_extension_states();
    const auto rank2 = class_ii_neighbor2_interior_pruning_rank2();
    const auto rank3 = class_ii_neighbor2_interior_pruning_rank3();
    fixed_nodes.insert(correction.begin(), correction.end());
    fixed_nodes.insert(rank2.begin(), rank2.end());
    fixed_nodes.insert(rank3.begin(), rank3.end());
    auto targets = class_ii_constant_node_forms(fixed_nodes);
    const auto append = [&](const auto& more) {
        targets.insert(targets.end(), more.begin(), more.end());
    };
    append(class_ii_terminal_shell_forms());
    append(class_ii_neighbor2_terminal_target_forms());
    append(class_ii_neighbor2_terminal_rank2_added_forms());
    append(class_ii_neighbor2_terminal_rank3_added_forms());
    result.source_roles = sources.size();
    std::set<SNode<3>> at_nine;
    for (const auto& form : sources) {
        SNode<3> node;
        node.i = form.i;
        node.j = form.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
            node.x[coordinate] =
                form.x[coordinate].intercept
                + 9 * form.x[coordinate].slope;
        at_nine.insert(node);
    }
    result.source_grammar_exact =
        at_nine == class_ii_neighbor2_terminal_new_rank1_sources(9);
    result.target_roles = targets.size();
    const auto exclusion =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            sources, targets, 0, 0, 7);
    result.prefix_branches = exclusion.prefix_branches;
    result.unresolved_branches = exclusion.unresolved_branches;
    result.endpoint_absence_exact = exclusion.exact;
    // Source parameter is the ambient endpoint a. Target shell index
    // satisfies 4 <= s <= a-1.
    const std::vector<ClassIIPruningConeForm> shell_domain = {
        {-7, 1, 0, 0},
        {0, -1, 1, 0}, {0, 1, -1, 0},
        {-4, 0, 0, 1}, {-1, 1, 0, -1},
    };
    const auto shell_exclusion =
        class_ii_neighbor2_affine_pair_exclusion(
            sources, class_ii_center_interior_shell_forms(),
            false, shell_domain);
    result.target_roles += shell_exclusion.target_count;
    result.prefix_branches += shell_exclusion.prefix_branches;
    result.unresolved_branches += shell_exclusion.unresolved_branches;
    result.indexed_shell_absence_exact = shell_exclusion.exact;

    return result;
}

inline ClassIIAffinePairExclusionResult
class_ii_neighbor2_terminal_inherited_fixed_to_new_targets() {
    auto fixed_sources =
        class_ii_neighbor2_interior_pruning_rank1_seed();
    for (const auto& node :
         class_ii_neighbor2_interior_pruning_frontier(4))
        fixed_sources.erase(node);
    return class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
        class_ii_constant_node_forms(fixed_sources),
        class_ii_neighbor2_terminal_new_survivor_target_forms(),
        -2, 0, 7);
}

inline ClassIIAffinePairExclusionResult
class_ii_neighbor2_terminal_inherited_cells_to_new_targets() {
    // 5 <= q <= a-3 and the new target is at endpoint s=a.  The
    // q=a-2 cell is a separate terminal rank-edit endpoint.
    const std::vector<ClassIIPruningConeForm> domain = {
        {-7, 1, 0, 0},
        {-5, 0, 1, 0}, {-3, 1, -1, 0},
        {0, -1, 0, 1}, {0, 1, 0, -1},
    };
    return class_ii_neighbor2_affine_pair_exclusion(
        class_ii_neighbor2_pruning_cell_forms(),
        class_ii_neighbor2_terminal_new_survivor_target_forms(),
        false, domain);
}

inline std::set<SNode<3>>
class_ii_neighbor2_terminal_inherited_rank1_endpoint(long long a) {
    auto candidates =
        class_ii_neighbor2_interior_pruning_cell(a - 2);
    const auto frontier =
        class_ii_neighbor2_interior_pruning_frontier(a - 2);
    candidates.insert(frontier.begin(), frontier.end());
    const auto terminal_rank1 =
        class_ii_neighbor2_terminal_pruning_ranks(a)[0];
    std::set<SNode<3>> result;
    for (const auto& node : candidates)
        if (terminal_rank1.count(node)) result.insert(node);
    return result;
}

inline std::vector<ClassIINodeAffineParameterForm>
class_ii_neighbor2_terminal_inherited_rank1_endpoint_forms() {
    return class_ii_affine_forms_from_ordered_sets(
        class_ii_neighbor2_terminal_inherited_rank1_endpoint(7),
        class_ii_neighbor2_terminal_inherited_rank1_endpoint(8), 7);
}

inline ClassIIAffinePairExclusionResult
class_ii_neighbor2_terminal_inherited_rank1_endpoint_to_new_targets() {
    return class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
        class_ii_neighbor2_terminal_inherited_rank1_endpoint_forms(),
        class_ii_neighbor2_terminal_new_survivor_target_forms(),
        0, 0, 7);
}

struct ClassIINeighbor2TerminalInheritedRank1Certificate {
    std::size_t fixed_sources = 0;
    std::size_t indexed_cell_roles = 0;
    std::size_t endpoint_sources = 0;
    std::size_t target_roles = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;
    bool third_parameter_grammar_exact = false;

    bool exact() const {
        return fixed_sources == 180
            && indexed_cell_roles == 48
            && endpoint_sources == 44
            && target_roles == 69
            && third_parameter_grammar_exact
            && unresolved_branches == 0;
    }
};

inline ClassIINeighbor2TerminalInheritedRank1Certificate
class_ii_neighbor2_terminal_inherited_rank1_certificate() {
    ClassIINeighbor2TerminalInheritedRank1Certificate result;
    const auto fixed =
        class_ii_neighbor2_terminal_inherited_fixed_to_new_targets();
    const auto cells =
        class_ii_neighbor2_terminal_inherited_cells_to_new_targets();
    const auto endpoint =
        class_ii_neighbor2_terminal_inherited_rank1_endpoint_to_new_targets();
    result.fixed_sources = fixed.source_count;
    result.indexed_cell_roles = cells.source_count;
    result.endpoint_sources = endpoint.source_count;
    result.target_roles = fixed.target_count;
    result.prefix_branches =
        fixed.prefix_branches + cells.prefix_branches
        + endpoint.prefix_branches;
    result.unresolved_branches =
        fixed.unresolved_branches + cells.unresolved_branches
        + endpoint.unresolved_branches;

    const auto evaluate = [](const ClassIINodeAffineParameterForm& form,
                             long long parameter) {
        SNode<3> node;
        node.i = form.i;
        node.j = form.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
            node.x[coordinate] =
                form.x[coordinate].intercept
                + parameter * form.x[coordinate].slope;
        return node;
    };
    std::set<SNode<3>> target_at_nine;
    for (const auto& form :
         class_ii_neighbor2_terminal_new_survivor_target_forms())
        target_at_nine.insert(evaluate(form, 9));
    std::set<SNode<3>> endpoint_at_nine;
    for (const auto& form :
         class_ii_neighbor2_terminal_inherited_rank1_endpoint_forms())
        endpoint_at_nine.insert(evaluate(form, 9));
    result.third_parameter_grammar_exact =
        target_at_nine
            == class_ii_neighbor2_terminal_new_survivor_targets_decomposed(9)
        && endpoint_at_nine
            == class_ii_neighbor2_terminal_inherited_rank1_endpoint(9);
    return result;
}

// Relative to a fictitious continuation of the stable shell grammar
// through q=a, the true terminal pre-Red grammar is a bounded affine
// edit: two roles leave and eight enter. Ordered role matching is
// checked at a=7,8 and then independently regenerated at a=9.
inline ClassIINeighbor2TerminalEditCertificate
class_ii_neighbor2_terminal_edit_certificate() {
    ClassIINeighbor2TerminalEditCertificate result;
    std::array<std::set<SNode<3>>, 3> removed;
    std::array<std::set<SNode<3>>, 3> added;
    for (long long a : {7LL, 8LL, 9LL}) {
        const std::size_t index = static_cast<std::size_t>(a - 7);
        const auto stable =
            class_ii_neighbor2_hypothetical_stable_terminal_grammar(a);
        const auto terminal =
            class_ii_neighbor2_terminal_pre_red_grammar(a);
        for (const auto& node : stable)
            if (!terminal.count(node)) removed[index].insert(node);
        for (const auto& node : terminal)
            if (!stable.count(node)) added[index].insert(node);
        ++result.validation_parameters;
    }
    result.removed_roles = removed[0].size();
    result.added_roles = added[0].size();
    const auto ordered_affine_regenerates = [](
            const std::set<SNode<3>>& at7,
            const std::set<SNode<3>>& at8,
            const std::set<SNode<3>>& at9) {
        if (at7.size() != at8.size() || at7.size() != at9.size())
            return false;
        auto left = at7.begin();
        auto middle = at8.begin();
        auto right = at9.begin();
        for (; left != at7.end(); ++left, ++middle, ++right) {
            if (left->i != middle->i || left->i != right->i
                    || left->j != middle->j || left->j != right->j)
                return false;
            for (std::size_t coordinate = 0;
                 coordinate < 3; ++coordinate) {
                if (middle->x[coordinate] - left->x[coordinate]
                        != right->x[coordinate]
                            - middle->x[coordinate])
                    return false;
            }
        }
        return true;
    };
    result.affine_edit_exact =
        removed[0].size() == 2 && added[0].size() == 8
        && ordered_affine_regenerates(
            removed[0], removed[1], removed[2])
        && ordered_affine_regenerates(
            added[0], added[1], added[2]);
    return result;
}

struct ClassIINeighbor2CellExclusionCertificate {
    bool affine_roles_exact = false;
    bool zero_internal_edges = false;
    bool zero_fixed_pruning_edges = false;
    std::size_t role_count = 0;
    std::size_t affine_target_count = 0;
    std::size_t pair_count = 0;
    std::size_t fixed_target_count = 0;
    std::size_t fixed_pair_count = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;

    bool exact() const {
        return affine_roles_exact && zero_internal_edges
            && zero_fixed_pruning_edges
            && role_count == 48 && affine_target_count == 73
            && pair_count == 48 * 73
            && fixed_target_count == 304
            && fixed_pair_count == 48 * 304;
    }
};

// Universal rank-one sublemma: no state in one growing 48-cell has a
// simple-forward edge to a state in any growing 48-cell, for
// a>=7 and 5<=q,s<=a-2.  Every prefix-pair contribution is shown
// identically zero on the four-ray parameter cone; no sampling or
// expanded substitution word is used.
inline ClassIINeighbor2CellExclusionCertificate
class_ii_neighbor2_cell_exclusion_certificate() {
    ClassIINeighbor2CellExclusionCertificate result;
    const auto forms = class_ii_neighbor2_pruning_cell_forms();
    auto affine_targets = forms;
    const auto frontier_forms =
        class_ii_neighbor2_pruning_frontier_forms();
    affine_targets.insert(
        affine_targets.end(), frontier_forms.begin(), frontier_forms.end());
    const auto shell_forms = class_ii_center_interior_shell_forms();
    affine_targets.insert(
        affine_targets.end(), shell_forms.begin(), shell_forms.end());
    affine_targets.push_back({
        2,
        {{{0, -1}, {0, 1}, {-1, 0}}},
        0,
    });
    result.role_count = forms.size();
    result.affine_target_count = affine_targets.size();
    result.affine_roles_exact = forms.size() == 48;
    result.zero_internal_edges = true;
    for (const auto& source : forms) {
        for (const auto& target : affine_targets) {
            ++result.pair_count;
            const auto left =
                class_ii_neighbor_symbolic_prefix_families(
                    2, static_cast<std::size_t>(target.i),
                    static_cast<std::size_t>(source.i));
            const auto right =
                class_ii_neighbor_symbolic_prefix_families(
                    2, static_cast<std::size_t>(target.j),
                    static_cast<std::size_t>(source.j));
            const long long target_sum_slope =
                target.x[0].slope + target.x[1].slope;
            if (target.x[2].slope != 0 || target_sum_slope != 0) {
                result.zero_internal_edges = false;
                continue;
            }
            const std::array<ClassIIPruningConeForm, 3> mx = {{
                {
                    target.x[2].intercept,
                    target.x[0].intercept + target.x[1].intercept,
                    0, 0,
                },
                {target.x[0].intercept, 0, 0, target.x[0].slope},
                {
                    target.x[0].intercept + target.x[1].intercept,
                    0, 0, target_sum_slope,
                },
            }};
            for (const auto& p : left) {
                for (const auto& q : right) {
                    ++result.prefix_branches;
                    std::array<ClassIIPruningConeForm, 3> delta{};
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        delta[coordinate] = {
                            source.x[coordinate].intercept
                                + q.base[coordinate].intercept
                                - p.base[coordinate].intercept,
                            q.base[coordinate].slope
                                - p.base[coordinate].slope,
                            source.x[coordinate].slope,
                            0,
                        };
                        delta[coordinate] =
                            delta[coordinate] - mx[coordinate];
                    }
                    const bool branch_zero =
                        class_ii_neighbor2_cell_branch_is_zero(
                            p, q, delta);
                    if (!branch_zero) ++result.unresolved_branches;
                    result.zero_internal_edges =
                        result.zero_internal_edges && branch_zero;
                }
            }
        }
    }
    std::set<SNode<3>> fixed_targets =
        class_ii_neighbor2_interior_pruning_rank1_seed();
    const auto rank2 = class_ii_neighbor2_interior_pruning_rank2();
    const auto rank3 = class_ii_neighbor2_interior_pruning_rank3();
    const auto correction =
        class_ii_neighbor2_fixed_extension_states();
    const auto center_base = class_ii_stable_base();
    fixed_targets.insert(rank2.begin(), rank2.end());
    fixed_targets.insert(rank3.begin(), rank3.end());
    fixed_targets.insert(correction.begin(), correction.end());
    fixed_targets.insert(center_base.begin(), center_base.end());
    result.fixed_target_count = fixed_targets.size();
    result.zero_fixed_pruning_edges = true;
    for (const auto& source : forms) {
        for (const auto& target_node : fixed_targets) {
            ++result.fixed_pair_count;
            ClassIINodeAffineParameterForm target;
            target.i = target_node.i;
            target.j = target_node.j;
            for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
                target.x[coordinate] = {
                    target_node.x[coordinate], 0};
            const auto left =
                class_ii_neighbor_symbolic_prefix_families(
                    2, static_cast<std::size_t>(target.i),
                    static_cast<std::size_t>(source.i));
            const auto right =
                class_ii_neighbor_symbolic_prefix_families(
                    2, static_cast<std::size_t>(target.j),
                    static_cast<std::size_t>(source.j));
            const std::array<ClassIIPruningConeForm, 3> mx = {{
                {
                    target.x[2].intercept,
                    target.x[0].intercept + target.x[1].intercept,
                    0, 0,
                },
                {target.x[0].intercept, 0, 0, 0},
                {
                    target.x[0].intercept + target.x[1].intercept,
                    0, 0, 0,
                },
            }};
            for (const auto& p : left) {
                for (const auto& q : right) {
                    ++result.prefix_branches;
                    std::array<ClassIIPruningConeForm, 3> delta{};
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        delta[coordinate] = {
                            source.x[coordinate].intercept
                                + q.base[coordinate].intercept
                                - p.base[coordinate].intercept,
                            q.base[coordinate].slope
                                - p.base[coordinate].slope,
                            source.x[coordinate].slope,
                            0,
                        };
                        delta[coordinate] =
                            delta[coordinate] - mx[coordinate];
                    }
                    const bool branch_zero =
                        class_ii_neighbor2_cell_branch_is_zero(
                            p, q, delta);
                    if (!branch_zero) ++result.unresolved_branches;
                    result.zero_fixed_pruning_edges =
                        result.zero_fixed_pruning_edges && branch_zero;
                }
            }
        }
    }
    return result;
}

struct ClassIINeighbor2StableRank1Certificate {
    bool fixed_seed_exact = false;
    bool moving_frontier_exact = false;
    std::size_t fixed_source_count = 0;
    std::size_t frontier_source_count = 0;
    std::size_t target_role_count = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;

    bool exact() const {
        return fixed_seed_exact && moving_frontier_exact
            && fixed_source_count == 180
            && frontier_source_count == 4
            && target_role_count == 377
            && unresolved_branches == 0;
    }
};

// Complements the 48-cell certificate with the fixed 180-state seed
// core and the moving four-state frontier.  Together they cover the
// entire stable first pruning rank.
inline ClassIINeighbor2StableRank1Certificate
class_ii_neighbor2_stable_rank1_certificate() {
    ClassIINeighbor2StableRank1Certificate result;
    auto fixed_sources =
        class_ii_neighbor2_interior_pruning_rank1_seed();
    const auto frontier4 =
        class_ii_neighbor2_interior_pruning_frontier(4);
    for (const auto& node : frontier4) fixed_sources.erase(node);
    const auto fixed_source_forms =
        class_ii_constant_node_forms(fixed_sources);
    const auto frontier_source_forms =
        class_ii_neighbor2_pruning_frontier_forms();
    auto target_forms = class_ii_neighbor2_stable_affine_target_forms();
    const auto fixed_target_forms = class_ii_constant_node_forms(
        class_ii_neighbor2_stable_fixed_target_nodes());
    target_forms.insert(
        target_forms.end(),
        fixed_target_forms.begin(), fixed_target_forms.end());
    const auto fixed_result =
        class_ii_neighbor2_affine_pair_exclusion(
            fixed_source_forms, target_forms);
    const auto frontier_result =
        class_ii_neighbor2_affine_pair_exclusion(
            frontier_source_forms, target_forms, true);
    result.fixed_source_count = fixed_source_forms.size();
    result.frontier_source_count = frontier_source_forms.size();
    result.target_role_count = target_forms.size();
    result.fixed_seed_exact = fixed_result.exact;
    result.moving_frontier_exact = frontier_result.exact;
    result.prefix_branches =
        fixed_result.prefix_branches + frontier_result.prefix_branches;
    result.unresolved_branches =
        fixed_result.unresolved_branches
        + frontier_result.unresolved_branches;
    return result;
}

struct ClassIINeighbor2StableLaterRankCertificate {
    bool rank2_absence_exact = false;
    bool rank3_absence_exact = false;
    bool predecessor_edges_exact = false;
    std::size_t rank2_sources = 0;
    std::size_t rank3_sources = 0;
    std::size_t rank2_targets = 0;
    std::size_t rank3_targets = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;

    bool exact() const {
        return rank2_absence_exact && rank3_absence_exact
            && predecessor_edges_exact
            && rank2_sources == 19 && rank3_sources == 9
            && unresolved_branches == 0;
    }
};

inline std::vector<ClassIIRedSuccessor>
class_ii_neighbor2_stable_rank2_predecessor_edges() {
    return {
        {{0, {-3, 3, 1}, 1}, {1, {3, -3, -2}, 0}},
        {{0, {-2, 1, 1}, 0}, {1, {1, -1, -2}, 0}},
        {{0, {-2, 2, 1}, 0}, {1, {2, -2, -2}, 0}},
        {{0, {-2, 3, 1}, 0}, {1, {3, -3, -2}, 0}},
        {{0, {1, -2, 0}, 1}, {0, {-2, 2, 2}, 0}},
        {{0, {1, -1, 0}, 1}, {0, {-1, 1, 2}, 0}},
        {{0, {2, -3, -1}, 0}, {0, {-3, 3, 2}, 1}},
        {{0, {2, -2, -1}, 0}, {0, {-2, 2, 2}, 1}},
        {{0, {2, -1, -1}, 0}, {0, {-1, 1, 2}, 1}},
        {{1, {-2, 2, 0}, 1}, {0, {2, -2, -2}, 0}},
        {{1, {-1, 1, 0}, 0}, {0, {1, -1, -2}, 0}},
        {{1, {-1, 2, 0}, 0}, {0, {2, -2, -2}, 0}},
        {{1, {2, -2, 0}, 1}, {0, {-2, 2, 2}, 0}},
        {{1, {3, -3, -1}, 0}, {0, {-3, 3, 2}, 1}},
        {{2, {-3, 3, 0}, 1}, {1, {3, -3, -2}, 0}},
        {{2, {-2, 2, 0}, 0}, {1, {2, -2, -2}, 0}},
        {{2, {-2, 3, 0}, 0}, {1, {3, -3, -2}, 0}},
        {{2, {1, -2, 0}, 1}, {0, {-3, 3, 1}, 0}},
        {{2, {2, -3, -1}, 0}, {0, {-4, 4, 1}, 1}},
    };
}

inline std::vector<ClassIIRedSuccessor>
class_ii_neighbor2_stable_rank3_predecessor_edges() {
    return {
        {{0, {-2, 2, 1}, 2}, {1, {3, -3, -1}, 0}},
        {{0, {-2, 3, 1}, 1}, {1, {3, -3, -1}, 0}},
        {{1, {-1, 1, 0}, 2}, {0, {2, -2, -1}, 0}},
        {{1, {-1, 2, 0}, 1}, {0, {2, -2, -1}, 0}},
        {{1, {1, -2, 0}, 1}, {0, {-2, 2, 1}, 0}},
        {{1, {2, -3, -1}, 0}, {0, {-3, 3, 1}, 1}},
        {{2, {-2, 3, 0}, 1}, {1, {3, -3, -1}, 0}},
        {{2, {1, -1, 0}, 1}, {0, {-2, 2, 1}, 0}},
        {{2, {2, -2, -1}, 0}, {0, {-3, 3, 1}, 1}},
    };
}

inline bool class_ii_neighbor2_fixed_edge_weight_one(
        const ClassIIRedSuccessor& edge,
        long long minimum_a = 7) {
    const std::set<SNode<3>> states = {edge.source, edge.target};
    const auto c0 =
        class_ii_neighbor_transition_controls(2, minimum_a, states);
    const auto c1 =
        class_ii_neighbor_transition_controls(2, minimum_a + 1, states);
    const auto c2 =
        class_ii_neighbor_transition_controls(2, minimum_a + 2, states);
    if (c0.size() != c1.size() || c0.size() != c2.size()) return false;
    long long stable_from = minimum_a;
    for (std::size_t i = 0; i < c0.size(); ++i) {
        const long long slope = c1[i] - c0[i];
        if (c2[i] != c1[i] + slope) return false;
        if (slope == 0) continue;
        const long long intercept = c0[i] - minimum_a * slope;
        stable_from = std::max(
            stable_from,
            std::abs(intercept) / std::abs(slope) + 2);
    }
    for (long long a = minimum_a; a <= stable_from + 1; ++a) {
        if (class_ii_neighbor_transition_weight(
                2, a, edge.source, edge.target) != 1) {
            return false;
        }
    }
    return true;
}

inline ClassIINeighbor2StableLaterRankCertificate
class_ii_neighbor2_stable_later_rank_certificate() {
    ClassIINeighbor2StableLaterRankCertificate result;
    const auto rank2 = class_ii_neighbor2_interior_pruning_rank2();
    const auto rank3 = class_ii_neighbor2_interior_pruning_rank3();
    auto survivor_fixed = class_ii_stable_base();
    const auto correction =
        class_ii_neighbor2_fixed_extension_states();
    survivor_fixed.insert(correction.begin(), correction.end());
    auto survivor_affine = class_ii_center_interior_shell_forms();
    survivor_affine.push_back({
        2,
        {{{0, -1}, {0, 1}, {-1, 0}}},
        0,
    });

    auto rank2_target_nodes = survivor_fixed;
    rank2_target_nodes.insert(rank2.begin(), rank2.end());
    rank2_target_nodes.insert(rank3.begin(), rank3.end());
    auto rank2_targets = survivor_affine;
    const auto rank2_fixed_forms =
        class_ii_constant_node_forms(rank2_target_nodes);
    rank2_targets.insert(
        rank2_targets.end(),
        rank2_fixed_forms.begin(), rank2_fixed_forms.end());

    auto rank3_target_nodes = survivor_fixed;
    rank3_target_nodes.insert(rank3.begin(), rank3.end());
    auto rank3_targets = survivor_affine;
    const auto rank3_fixed_forms =
        class_ii_constant_node_forms(rank3_target_nodes);
    rank3_targets.insert(
        rank3_targets.end(),
        rank3_fixed_forms.begin(), rank3_fixed_forms.end());

    const auto rank2_result =
        class_ii_neighbor2_affine_pair_exclusion(
            class_ii_constant_node_forms(rank2), rank2_targets);
    const auto rank3_result =
        class_ii_neighbor2_affine_pair_exclusion(
            class_ii_constant_node_forms(rank3), rank3_targets);
    result.rank2_sources = rank2.size();
    result.rank3_sources = rank3.size();
    result.rank2_targets = rank2_targets.size();
    result.rank3_targets = rank3_targets.size();
    result.rank2_absence_exact = rank2_result.exact;
    result.rank3_absence_exact = rank3_result.exact;
    result.prefix_branches =
        rank2_result.prefix_branches + rank3_result.prefix_branches;
    result.unresolved_branches =
        rank2_result.unresolved_branches
        + rank3_result.unresolved_branches;
    result.predecessor_edges_exact = true;
    std::set<SNode<3>> rank2_sources_seen;
    for (const auto& edge :
         class_ii_neighbor2_stable_rank2_predecessor_edges()) {
        rank2_sources_seen.insert(edge.source);
        result.predecessor_edges_exact =
            result.predecessor_edges_exact
            && rank2.count(edge.source) == 1
            && class_ii_neighbor2_interior_pruning_rank1_seed()
                .count(edge.target) == 1
            && class_ii_neighbor2_fixed_edge_weight_one(edge);
    }
    std::set<SNode<3>> rank3_sources_seen;
    for (const auto& edge :
         class_ii_neighbor2_stable_rank3_predecessor_edges()) {
        rank3_sources_seen.insert(edge.source);
        result.predecessor_edges_exact =
            result.predecessor_edges_exact
            && rank3.count(edge.source) == 1
            && rank2.count(edge.target) == 1
            && class_ii_neighbor2_fixed_edge_weight_one(edge);
    }
    result.predecessor_edges_exact =
        result.predecessor_edges_exact
        && rank2_sources_seen == rank2
        && rank3_sources_seen == rank3;
    return result;
}

inline SNode<3> class_ii_evaluate_affine_node_form(
        const ClassIINodeAffineParameterForm& form,
        long long parameter) {
    SNode<3> result;
    result.i = form.i;
    result.j = form.j;
    for (std::size_t coordinate = 0; coordinate < 3; ++coordinate)
        result.x[coordinate] =
            form.x[coordinate].intercept
            + parameter * form.x[coordinate].slope;
    return result;
}

struct ClassIINeighbor2TerminalRankEditShapeCertificate {
    std::size_t rank1_removed = 0;
    std::size_t rank1_added = 0;
    std::size_t rank2_added = 0;
    std::size_t rank3_added = 0;
    bool third_parameter_exact = false;

    bool exact() const {
        return rank1_removed == 27 && rank1_added == 4
            && rank2_added == 22 && rank3_added == 2
            && third_parameter_exact;
    }
};

inline ClassIINeighbor2TerminalRankEditShapeCertificate
class_ii_neighbor2_terminal_rank_edit_shape_certificate() {
    ClassIINeighbor2TerminalRankEditShapeCertificate result;
    const auto removed =
        class_ii_neighbor2_terminal_rank1_removed_forms();
    const auto rank1 =
        class_ii_neighbor2_terminal_rank1_added_forms();
    const auto rank2 =
        class_ii_neighbor2_terminal_rank2_added_forms();
    const auto rank3 =
        class_ii_neighbor2_terminal_rank3_added_forms();
    result.rank1_removed = removed.size();
    result.rank1_added = rank1.size();
    result.rank2_added = rank2.size();
    result.rank3_added = rank3.size();
    const auto evaluate = [](const auto& forms, long long a) {
        std::set<SNode<3>> values;
        for (const auto& form : forms)
            values.insert(class_ii_evaluate_affine_node_form(form, a));
        return values;
    };
    result.third_parameter_exact =
        evaluate(removed, 9)
            == class_ii_neighbor2_terminal_rank1_removed(9)
        && evaluate(rank1, 9)
            == class_ii_neighbor2_terminal_rank1_added(9)
        && evaluate(rank2, 9)
            == class_ii_neighbor2_terminal_rank2_added(9)
        && evaluate(rank3, 9)
            == class_ii_neighbor2_terminal_rank3_added(9);
    return result;
}

struct ClassIIAffineRedEdge {
    ClassIINodeAffineParameterForm source;
    ClassIINodeAffineParameterForm target;
};

inline std::vector<ClassIIAffineRedEdge>
class_ii_neighbor2_penultimate_promoted_predecessor_edges() {
    const auto sources =
        class_ii_neighbor2_penultimate_promoted_forms();
    const std::vector<ClassIINodeAffineParameterForm> targets = {
        {1, {{{2, -1}, {-1, 1}, {-2, 0}}}, 0},
        {0, {{{-3, 1}, {2, -1}, {2, 0}}}, 1},
        {0, {{{-2, 1}, {1, -1}, {2, 0}}}, 0},
        {0, {{{-1, 1}, {0, -1}, {1, 0}}}, 0},
        {0, {{{-1, 1}, {0, -1}, {0, 0}}}, 1},
        {1, {{{2, -1}, {-1, 1}, {-2, 0}}}, 0},
    };
    std::vector<ClassIIAffineRedEdge> result;
    for (std::size_t index = 0; index < sources.size(); ++index)
        result.push_back({sources[index], targets[index]});
    return result;
}

inline bool class_ii_neighbor2_affine_edge_weight_one(
        const ClassIIAffineRedEdge& edge,
        long long minimum_a = 8,
        long long source_offset = -1,
        long long target_offset = -1) {
    const auto evaluate_edge = [&](long long a) {
        return ClassIIRedSuccessor{
            class_ii_evaluate_affine_node_form(
                edge.source, a + source_offset),
            class_ii_evaluate_affine_node_form(
                edge.target, a + target_offset),
        };
    };
    const auto controls = [&](long long a) {
        const auto evaluated = evaluate_edge(a);
        return class_ii_neighbor_transition_controls(
            2, a, {evaluated.source, evaluated.target});
    };
    const auto c0 = controls(minimum_a);
    const auto c1 = controls(minimum_a + 1);
    const auto c2 = controls(minimum_a + 2);
    if (c0.size() != c1.size() || c0.size() != c2.size())
        return false;
    long long stable_from = minimum_a;
    for (std::size_t index = 0; index < c0.size(); ++index) {
        const long long slope = c1[index] - c0[index];
        if (c2[index] != c1[index] + slope) return false;
        if (slope == 0) continue;
        const long long intercept =
            c0[index] - minimum_a * slope;
        stable_from = std::max(
            stable_from,
            std::abs(intercept) / std::abs(slope) + 2);
    }
    for (long long a = minimum_a; a <= stable_from + 1; ++a) {
        const auto evaluated = evaluate_edge(a);
        if (class_ii_neighbor_transition_weight(
                2, a, evaluated.source, evaluated.target) != 1)
            return false;
    }
    return true;
}

inline bool class_ii_neighbor2_affine_edge_weight_positive(
        const ClassIIAffineRedEdge& edge,
        long long minimum_a = 8,
        long long source_offset = 0,
        long long target_offset = 0) {
    const auto evaluate_edge = [&](long long a) {
        return ClassIIRedSuccessor{
            class_ii_evaluate_affine_node_form(
                edge.source, a + source_offset),
            class_ii_evaluate_affine_node_form(
                edge.target, a + target_offset),
        };
    };
    const auto controls = [&](long long a) {
        const auto evaluated = evaluate_edge(a);
        return class_ii_neighbor_transition_controls(
            2, a, {evaluated.source, evaluated.target});
    };
    const auto c0 = controls(minimum_a);
    const auto c1 = controls(minimum_a + 1);
    const auto c2 = controls(minimum_a + 2);
    if (c0.size() != c1.size() || c0.size() != c2.size())
        return false;
    long long stable_from = minimum_a;
    for (std::size_t index = 0; index < c0.size(); ++index) {
        const long long slope = c1[index] - c0[index];
        if (c2[index] != c1[index] + slope) return false;
        if (slope == 0) continue;
        const long long intercept =
            c0[index] - minimum_a * slope;
        stable_from = std::max(
            stable_from,
            std::abs(intercept) / std::abs(slope) + 2);
    }
    for (long long a = minimum_a; a <= stable_from + 1; ++a) {
        const auto evaluated = evaluate_edge(a);
        if (class_ii_neighbor_transition_weight(
                2, a, evaluated.source, evaluated.target) <= 0)
            return false;
    }
    return true;
}

struct ClassIINeighbor2FixedRedCertificate {
    bool base_exact = false;
    bool grammar_continuation_exact = false;
    bool exclusion_exact = false;
    bool predecessor_edges_exact = false;
    std::size_t fixed_rank1_sources = 0;
    std::size_t indexed_rank1_roles = 0;
    std::size_t endpoint_rank1_sources = 0;
    std::array<std::size_t, 5> later_rank_sources{};
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;
    std::size_t predecessor_edges = 0;

    bool exact() const {
        return base_exact && grammar_continuation_exact
            && exclusion_exact && predecessor_edges_exact
            && fixed_rank1_sources == 180
            && indexed_rank1_roles == 48
            && endpoint_rank1_sources == 199
            && later_rank_sources
                == std::array<std::size_t, 5>{46, 22, 15, 10, 4}
            && prefix_branches == 228185
            && unresolved_branches == 0
            && predecessor_edges == 97;
    }
};

inline ClassIINeighbor2FixedRedCertificate
class_ii_neighbor2_fixed_red_certificate() {
    ClassIINeighbor2FixedRedCertificate result;
    auto fixed_rank1 = class_ii_neighbor2_interior_pruning_rank1_seed();
    for (const auto& node :
         class_ii_neighbor2_interior_pruning_frontier(4))
        fixed_rank1.erase(node);
    const auto fixed_rank1_forms =
        class_ii_constant_node_forms(fixed_rank1);
    const auto cell_forms = class_ii_neighbor2_pruning_cell_forms();
    const auto endpoint_rank1 =
        class_ii_neighbor2_fixed_rank1_endpoint_forms();
    const auto bounded_rank1 =
        class_ii_neighbor2_fixed_bounded_target_forms_after(0);
    result.fixed_rank1_sources = fixed_rank1_forms.size();
    result.indexed_rank1_roles = cell_forms.size();
    result.endpoint_rank1_sources = endpoint_rank1.size();

    const std::vector<ClassIIPruningConeForm> cell_bounded_domain = {
        {-8, 1, 0, 0},
        {-5, 0, 1, 0}, {-4, 1, -1, 0},
        {0, -1, 0, 1}, {0, 1, 0, -1},
    };
    const std::vector<ClassIIPruningConeForm> cell_shell_domain = {
        {-8, 1, 0, 0},
        {-5, 0, 1, 0}, {-4, 1, -1, 0},
        {-4, 0, 0, 1}, {-2, 1, 0, -1},
    };
    const std::vector<ClassIIPruningConeForm> endpoint_shell_domain = {
        {-8, 1, 0, 0},
        {0, -1, 1, 0}, {0, 1, -1, 0},
        {-4, 0, 0, 1}, {-2, 1, 0, -1},
    };
    std::vector<ClassIIAffinePairExclusionResult> exclusions = {
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            fixed_rank1_forms, bounded_rank1, 0, 0, 8),
        class_ii_neighbor2_affine_pair_exclusion(
            fixed_rank1_forms, class_ii_center_interior_shell_forms(),
            false, endpoint_shell_domain),
        class_ii_neighbor2_affine_pair_exclusion(
            cell_forms, bounded_rank1, false, cell_bounded_domain),
        class_ii_neighbor2_affine_pair_exclusion(
            cell_forms, class_ii_center_interior_shell_forms(),
            false, cell_shell_domain),
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            endpoint_rank1, bounded_rank1, 0, 0, 8),
        class_ii_neighbor2_affine_pair_exclusion(
            endpoint_rank1, class_ii_center_interior_shell_forms(),
            false, endpoint_shell_domain),
    };
    result.grammar_continuation_exact =
        [&]() {
            const auto evaluate = [](const auto& forms, long long a) {
                std::set<SNode<3>> nodes;
                for (const auto& form : forms)
                    nodes.insert(
                        class_ii_evaluate_affine_node_form(form, a));
                return nodes;
            };
            bool exact =
                evaluate(endpoint_rank1, 20)
                    == class_ii_neighbor2_fixed_rank1_endpoint(20)
                && evaluate(bounded_rank1, 20)
                    == class_ii_neighbor2_fixed_bounded_targets_after(
                        20, 0);
            for (std::size_t rank = 1; rank < 6; ++rank) {
                const auto sources =
                    class_ii_neighbor2_fixed_rank_forms(rank);
                const auto bounded =
                    class_ii_neighbor2_fixed_bounded_target_forms_after(
                        rank);
                const auto ranks20 =
                    class_ii_neighbor2_fixed_pruning_ranks(20);
                exact = exact
                    && evaluate(sources, 20) == ranks20[rank]
                    && evaluate(bounded, 20)
                        == class_ii_neighbor2_fixed_bounded_targets_after(
                            20, rank);
            }
            return exact;
        }();
    for (std::size_t rank = 1; rank < 6; ++rank) {
        const auto sources =
            class_ii_neighbor2_fixed_rank_forms(rank);
        const auto bounded =
            class_ii_neighbor2_fixed_bounded_target_forms_after(rank);
        result.later_rank_sources[rank - 1] = sources.size();
        exclusions.push_back(
            class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
                sources, bounded, 0, 0, 8));
        exclusions.push_back(
            class_ii_neighbor2_affine_pair_exclusion(
                sources, class_ii_center_interior_shell_forms(),
                false, endpoint_shell_domain));
    }
    result.exclusion_exact = true;
    for (const auto& exclusion : exclusions) {
        result.prefix_branches += exclusion.prefix_branches;
        result.unresolved_branches += exclusion.unresolved_branches;
        result.exclusion_exact =
            result.exclusion_exact && exclusion.exact;
    }

    {
        constexpr long long a = 7;
        const auto nodes =
            class_ii_neighbor2_fixed_pre_red_decomposed(a);
        std::vector<std::tuple<SNode<3>, SNode<3>,
                               std::vector<long long>,
                               std::vector<long long>>> edges;
        for (const auto& source : nodes)
            for (const auto& target : nodes)
                if (class_ii_neighbor_transition_weight(
                        2, a, source, target) != 0)
                    edges.emplace_back(
                        source, target,
                        std::vector<long long>{},
                        std::vector<long long>{});
        result.base_exact =
            red_trace<3>(nodes, edges).pruning_ranks
                == class_ii_neighbor2_fixed_pruning_ranks(a);
    }

    result.predecessor_edges_exact = true;
    for (std::size_t rank = 1; rank < 6; ++rank) {
        const auto sources =
            class_ii_neighbor2_fixed_rank_forms(rank);
        const auto ranks8 = class_ii_neighbor2_fixed_pruning_ranks(8);
        const auto ranks9 = class_ii_neighbor2_fixed_pruning_ranks(9);
        std::size_t found_count = 0;
        for (const auto& source_form : sources) {
            const auto source8 =
                class_ii_evaluate_affine_node_form(source_form, 8);
            const auto source9 =
                class_ii_evaluate_affine_node_form(source_form, 9);
            bool found = false;
            for (const auto& target8 : ranks8[rank - 1]) {
                if (class_ii_neighbor_transition_weight(
                        2, 8, source8, target8) <= 0)
                    continue;
                for (const auto& target9 : ranks9[rank - 1]) {
                    if (target8.i != target9.i
                            || target8.j != target9.j
                            || target8.x[2] != target9.x[2]
                            || target8.x[0] + target8.x[1]
                                != target9.x[0] + target9.x[1]
                            || std::abs(target9.x[0] - target8.x[0]) > 1
                            || std::abs(target9.x[1] - target8.x[1]) > 1
                            || class_ii_neighbor_transition_weight(
                                2, 9, source9, target9) <= 0)
                        continue;
                    ClassIINodeAffineParameterForm target_form;
                    target_form.i = target8.i;
                    target_form.j = target8.j;
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        const long long slope =
                            target9.x[coordinate]
                            - target8.x[coordinate];
                        target_form.x[coordinate] = {
                            target8.x[coordinate] - 8 * slope, slope};
                    }
                    if (class_ii_neighbor2_affine_edge_weight_positive(
                            {source_form, target_form}, 8, 0, 0)) {
                        found = true;
                        ++found_count;
                        break;
                    }
                }
                if (found) break;
            }
        }
        result.predecessor_edges += found_count;
        result.predecessor_edges_exact =
            result.predecessor_edges_exact
            && found_count == sources.size();
    }
    return result;
}

struct ClassIINeighbor2TerminalLaterRankCertificate {
    bool changed_absence_exact = false;
    bool predecessor_edges_exact = false;
    bool base_exact = false;
    bool third_parameter_grammar_exact = false;
    std::size_t new_rank2_sources = 0;
    std::size_t new_rank3_sources = 0;
    std::size_t post_rank1_new_targets = 0;
    std::size_t post_rank2_new_targets = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;
    std::size_t predecessor_edges = 0;

    bool exact() const {
        return changed_absence_exact && predecessor_edges_exact
            && base_exact && third_parameter_grammar_exact
            && new_rank2_sources == 22 && new_rank3_sources == 2
            && post_rank1_new_targets == 69
            && post_rank2_new_targets == 47
            && prefix_branches == 5151
            && unresolved_branches == 0
            && predecessor_edges == 24;
    }
};

inline ClassIINeighbor2TerminalLaterRankCertificate
class_ii_neighbor2_terminal_later_rank_certificate() {
    ClassIINeighbor2TerminalLaterRankCertificate result;
    const auto new_rank2 =
        class_ii_neighbor2_terminal_rank2_added_forms();
    const auto new_rank3 =
        class_ii_neighbor2_terminal_rank3_added_forms();
    const auto new_post_rank1 =
        class_ii_neighbor2_terminal_new_survivor_target_forms();
    const auto new_post_rank2 =
        class_ii_neighbor2_terminal_new_post_rank2_target_forms();
    result.new_rank2_sources = new_rank2.size();
    result.new_rank3_sources = new_rank3.size();
    result.post_rank1_new_targets = new_post_rank1.size();
    result.post_rank2_new_targets = new_post_rank2.size();

    auto fixed_rank1 = class_ii_neighbor2_interior_pruning_rank1_seed();
    for (const auto& node :
         class_ii_neighbor2_interior_pruning_frontier(4))
        fixed_rank1.erase(node);
    auto inherited_fixed_post_rank1 = class_ii_stable_base();
    const auto fixed_extension =
        class_ii_neighbor2_fixed_extension_states();
    const auto stable_rank2 =
        class_ii_neighbor2_interior_pruning_rank2();
    const auto stable_rank3 =
        class_ii_neighbor2_interior_pruning_rank3();
    inherited_fixed_post_rank1.insert(
        fixed_extension.begin(), fixed_extension.end());
    inherited_fixed_post_rank1.insert(
        stable_rank2.begin(), stable_rank2.end());
    inherited_fixed_post_rank1.insert(
        stable_rank3.begin(), stable_rank3.end());
    for (const auto& node : fixed_rank1)
        inherited_fixed_post_rank1.erase(node);
    auto inherited_fixed_post_rank2 = inherited_fixed_post_rank1;
    for (const auto& node : stable_rank2)
        inherited_fixed_post_rank2.erase(node);
    const std::vector<ClassIINodeAffineParameterForm> inherited_tip = {
        {2, {{{2, -1}, {-2, 1}, {-1, 0}}}, 0},
    };
    const auto with_tip = [&](const std::set<SNode<3>>& nodes) {
        auto forms = class_ii_constant_node_forms(nodes);
        forms.insert(forms.end(), inherited_tip.begin(), inherited_tip.end());
        return forms;
    };
    const std::vector<ClassIIPruningConeForm> shell_domain = {
        {-7, 1, 0, 0},
        {0, -1, 1, 0}, {0, 1, -1, 0},
        {-4, 0, 0, 1}, {-2, 1, 0, -1},
    };
    const std::vector<ClassIIAffinePairExclusionResult> exclusions = {
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            new_rank2, new_post_rank1, 0, 0, 7),
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            new_rank3, new_post_rank2, 0, 0, 7),
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            new_rank2, with_tip(inherited_fixed_post_rank1), 0, 0, 7),
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            new_rank3, with_tip(inherited_fixed_post_rank2), 0, 0, 7),
        class_ii_neighbor2_affine_pair_exclusion(
            new_rank2, class_ii_center_interior_shell_forms(),
            false, shell_domain),
        class_ii_neighbor2_affine_pair_exclusion(
            new_rank3, class_ii_center_interior_shell_forms(),
            false, shell_domain),
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_constant_node_forms(stable_rank2),
            new_post_rank1, 0, 0, 7),
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            class_ii_constant_node_forms(stable_rank3),
            new_post_rank2, 0, 0, 7),
    };
    result.changed_absence_exact = true;
    for (const auto& exclusion : exclusions) {
        result.prefix_branches += exclusion.prefix_branches;
        result.unresolved_branches += exclusion.unresolved_branches;
        result.changed_absence_exact =
            result.changed_absence_exact && exclusion.exact;
    }

    const auto find_predecessors = [](const auto& source_forms,
                                      std::size_t target_rank) {
        std::vector<ClassIIAffineRedEdge> edges;
        const auto ranks7 =
            class_ii_neighbor2_terminal_pruning_ranks(7);
        const auto ranks8 =
            class_ii_neighbor2_terminal_pruning_ranks(8);
        for (const auto& source_form : source_forms) {
            const auto source7 =
                class_ii_evaluate_affine_node_form(source_form, 7);
            const auto source8 =
                class_ii_evaluate_affine_node_form(source_form, 8);
            bool found = false;
            for (const auto& target7 : ranks7[target_rank]) {
                if (class_ii_neighbor_transition_weight(
                        2, 7, source7, target7) != 1)
                    continue;
                for (const auto& target8 : ranks8[target_rank]) {
                    if (target7.i != target8.i
                            || target7.j != target8.j
                            || target7.x[2] != target8.x[2]
                            || target7.x[0] + target7.x[1]
                                != target8.x[0] + target8.x[1]
                            || std::abs(target8.x[0] - target7.x[0]) > 1
                            || std::abs(target8.x[1] - target7.x[1]) > 1
                            || class_ii_neighbor_transition_weight(
                                2, 8, source8, target8) != 1)
                        continue;
                    ClassIINodeAffineParameterForm target_form;
                    target_form.i = target7.i;
                    target_form.j = target7.j;
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        const long long slope =
                            target8.x[coordinate]
                            - target7.x[coordinate];
                        target_form.x[coordinate] = {
                            target7.x[coordinate] - 7 * slope, slope};
                    }
                    const ClassIIAffineRedEdge edge{
                        source_form, target_form};
                    if (class_ii_neighbor2_affine_edge_weight_one(
                            edge, 7, 0, 0)) {
                        edges.push_back(edge);
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }
        return edges;
    };
    const auto rank2_edges = find_predecessors(new_rank2, 0);
    const auto rank3_edges = find_predecessors(new_rank3, 1);
    result.predecessor_edges =
        rank2_edges.size() + rank3_edges.size();
    result.predecessor_edges_exact =
        rank2_edges.size() == new_rank2.size()
        && rank3_edges.size() == new_rank3.size();
    result.base_exact =
        class_ii_neighbor2_stable_later_rank_certificate().exact();

    const auto evaluate = [](const auto& forms, long long a) {
        std::set<SNode<3>> nodes;
        for (const auto& form : forms)
            nodes.insert(class_ii_evaluate_affine_node_form(form, a));
        return nodes;
    };
    result.third_parameter_grammar_exact =
        evaluate(new_post_rank1, 9)
            == class_ii_neighbor2_terminal_new_survivor_targets_decomposed(9)
        && evaluate(new_post_rank2, 9)
            == class_ii_neighbor2_terminal_new_post_rank2_targets_decomposed(
                9);
    return result;
}

struct ClassIINeighbor2PenultimateLaterRankCertificate {
    bool rank2_absence_exact = false;
    bool rank3_absence_exact = false;
    bool predecessor_edges_exact = false;
    bool base_exact = false;
    std::size_t changed_pair_groups = 0;
    std::size_t prefix_branches = 0;
    std::size_t unresolved_branches = 0;

    bool exact() const {
        return rank2_absence_exact && rank3_absence_exact
            && predecessor_edges_exact && base_exact
            && changed_pair_groups == 5
            && prefix_branches == 1204
            && unresolved_branches == 0;
    }
};

// Only the endpoint target slice and the six promoted rank-two sources
// are new; every other later-rank pair is inherited from the stable
// certificate. Exact endpoint elimination closes those five changed
// pair groups, and affine weight-one predecessors prevent premature
// deletion of the promoted sources.
inline ClassIINeighbor2PenultimateLaterRankCertificate
class_ii_neighbor2_penultimate_later_rank_certificate() {
    ClassIINeighbor2PenultimateLaterRankCertificate result;
    const auto promoted =
        class_ii_neighbor2_penultimate_promoted_forms();
    const auto transfer =
        class_ii_neighbor2_penultimate_transfer_forms();
    const auto shells = class_ii_center_interior_shell_forms();
    const std::vector<ClassIINodeAffineParameterForm> tip = {
        {2, {{{0, -1}, {0, 1}, {-1, 0}}}, 0},
    };
    auto endpoint_targets = shells;
    endpoint_targets.insert(
        endpoint_targets.end(), tip.begin(), tip.end());
    endpoint_targets.insert(
        endpoint_targets.end(), promoted.begin(), promoted.end());
    endpoint_targets.insert(
        endpoint_targets.end(), transfer.begin(), transfer.end());
    const auto rank2 = class_ii_constant_node_forms(
        class_ii_neighbor2_interior_pruning_rank2());
    const auto rank3 = class_ii_constant_node_forms(
        class_ii_neighbor2_interior_pruning_rank3());

    auto fixed_nodes = class_ii_stable_base();
    const auto correction =
        class_ii_neighbor2_fixed_extension_states();
    fixed_nodes.insert(correction.begin(), correction.end());
    auto fixed_targets = class_ii_constant_node_forms(fixed_nodes);
    fixed_targets.insert(
        fixed_targets.end(), rank2.begin(), rank2.end());
    fixed_targets.insert(
        fixed_targets.end(), rank3.begin(), rank3.end());

    const auto fixed_rank2_to_endpoint =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            rank2, endpoint_targets, -2, -1);
    const auto promoted_to_fixed =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            promoted, fixed_targets, -1, -2);
    const std::vector<ClassIIPruningConeForm> shell_domain = {
        {-7, 1, 0, 0},
        {1, -1, 1, 0}, {-1, 1, -1, 0},
        {-4, 0, 0, 1}, {-1, 1, 0, -1},
    };
    const auto promoted_to_shells =
        class_ii_neighbor2_affine_pair_exclusion(
            promoted, shells, false, shell_domain);
    auto endpoint_rank2_targets = tip;
    endpoint_rank2_targets.insert(
        endpoint_rank2_targets.end(),
        promoted.begin(), promoted.end());
    endpoint_rank2_targets.insert(
        endpoint_rank2_targets.end(),
        transfer.begin(), transfer.end());
    const auto promoted_to_endpoint =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            promoted, endpoint_rank2_targets, -1, -1);
    auto rank3_new_targets = shells;
    rank3_new_targets.insert(
        rank3_new_targets.end(), tip.begin(), tip.end());
    rank3_new_targets.insert(
        rank3_new_targets.end(), transfer.begin(), transfer.end());
    const auto rank3_to_endpoint =
        class_ii_neighbor2_affine_pair_exclusion_on_endpoint(
            rank3, rank3_new_targets, -2, -1);

    const std::array<ClassIIAffinePairExclusionResult, 5> groups = {
        fixed_rank2_to_endpoint,
        promoted_to_fixed,
        promoted_to_shells,
        promoted_to_endpoint,
        rank3_to_endpoint,
    };
    result.changed_pair_groups = groups.size();
    for (const auto& group : groups) {
        result.prefix_branches += group.prefix_branches;
        result.unresolved_branches += group.unresolved_branches;
    }
    result.rank2_absence_exact =
        fixed_rank2_to_endpoint.exact
        && promoted_to_fixed.exact
        && promoted_to_shells.exact
        && promoted_to_endpoint.exact;
    result.rank3_absence_exact = rank3_to_endpoint.exact;

    result.predecessor_edges_exact = true;
    for (const auto& edge :
         class_ii_neighbor2_penultimate_promoted_predecessor_edges())
        result.predecessor_edges_exact =
            result.predecessor_edges_exact
            && class_ii_neighbor2_affine_edge_weight_one(edge);
    // The stable fixed predecessor catalogues remain within the
    // corresponding penultimate ranks for a>=8.
    for (const auto& edge :
         class_ii_neighbor2_stable_rank2_predecessor_edges())
        result.predecessor_edges_exact =
            result.predecessor_edges_exact
            && class_ii_neighbor2_fixed_edge_weight_one(edge, 8);
    for (const auto& edge :
         class_ii_neighbor2_stable_rank3_predecessor_edges())
        result.predecessor_edges_exact =
            result.predecessor_edges_exact
            && class_ii_neighbor2_fixed_edge_weight_one(edge, 8);

    const auto ranks =
        class_ii_neighbor2_penultimate_pruning_ranks(7);
    const auto grammar =
        class_ii_neighbor2_penultimate_pre_red_grammar(7);
    auto after_rank1 = grammar;
    for (const auto& node : ranks[0]) after_rank1.erase(node);
    auto after_rank2 = after_rank1;
    for (const auto& node : ranks[1]) after_rank2.erase(node);
    result.base_exact = true;
    for (const auto& source : ranks[1]) {
        bool predecessor = false;
        for (const auto& target : ranks[0])
            predecessor = predecessor
                || class_ii_neighbor_transition_weight(
                    2, 7, source, target) > 0;
        for (const auto& target : after_rank1)
            if (class_ii_neighbor_transition_weight(
                    2, 7, source, target) != 0)
                result.base_exact = false;
        result.base_exact = result.base_exact && predecessor;
    }
    for (const auto& source : ranks[2]) {
        bool predecessor = false;
        for (const auto& target : ranks[1])
            predecessor = predecessor
                || class_ii_neighbor_transition_weight(
                    2, 7, source, target) > 0;
        for (const auto& target : after_rank2)
            if (class_ii_neighbor_transition_weight(
                    2, 7, source, target) != 0)
                result.base_exact = false;
        result.base_exact = result.base_exact && predecessor;
    }
    return result;
}

// Named B_r/A_r catalogues for the four universally certified
// non-base phases. These are definitions consumed by the global
// stitch, not a claim that the transition into each catalogue has
// occurred. Base rounds 1..4 remain deliberately outside this
// dispatcher until their reverse-corona and Red-exclusion premises
// are closed.
inline std::set<SNode<3>>
class_ii_neighbor2_nonbase_pre_red_catalogue(
        long long a, long long round) {
    const auto phase =
        class_ii_neighbor2_global_round_phase(a, round);
    switch (phase) {
    case ClassIINeighbor2GlobalRoundPhase::stable:
        return class_ii_neighbor2_stable_pre_red_candidate(
            a, static_cast<std::size_t>(round));
    case ClassIINeighbor2GlobalRoundPhase::penultimate:
        return class_ii_neighbor2_penultimate_pre_red_grammar(a);
    case ClassIINeighbor2GlobalRoundPhase::terminal:
        return class_ii_neighbor2_terminal_pre_red_grammar(a);
    case ClassIINeighbor2GlobalRoundPhase::repeated:
        return class_ii_neighbor2_fixed_pre_red_decomposed(a);
    case ClassIINeighbor2GlobalRoundPhase::base:
        throw std::domain_error(
            "Class-II neighbor-2 base pre-Red catalogue is not closed");
    }
    throw std::logic_error("unreachable Class-II round phase");
}

inline std::vector<std::set<SNode<3>>>
class_ii_neighbor2_nonbase_pruning_ranks(
        long long a, long long round) {
    const auto phase =
        class_ii_neighbor2_global_round_phase(a, round);
    switch (phase) {
    case ClassIINeighbor2GlobalRoundPhase::stable:
        return class_ii_neighbor2_interior_pruning_ranks(
            static_cast<std::size_t>(round));
    case ClassIINeighbor2GlobalRoundPhase::penultimate:
        return class_ii_neighbor2_penultimate_pruning_ranks(a);
    case ClassIINeighbor2GlobalRoundPhase::terminal:
        return class_ii_neighbor2_terminal_pruning_ranks(a);
    case ClassIINeighbor2GlobalRoundPhase::repeated:
        return class_ii_neighbor2_fixed_pruning_ranks(a);
    case ClassIINeighbor2GlobalRoundPhase::base:
        throw std::domain_error(
            "Class-II neighbor-2 base Red ranks are not closed");
    }
    throw std::logic_error("unreachable Class-II round phase");
}

inline std::set<SNode<3>>
class_ii_neighbor2_nonbase_post_red_catalogue(
        long long a, long long round) {
    auto result =
        class_ii_neighbor2_nonbase_pre_red_catalogue(a, round);
    for (const auto& rank :
         class_ii_neighbor2_nonbase_pruning_ranks(a, round))
        for (const auto& node : rank) result.erase(node);
    return result;
}

// Global induction status ledger.
//
// docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's "Next implementation"
// asks for a small object that does not recompute local eliminations,
// consumes the existing exact booleans, uses the kernel-checked round
// router, records the four base-round premises separately from the
// small-parameter table, and reports the first missing local premise
// rather than one opaque false flag. This is that object.
//
// For round >= 5 (stable/penultimate/terminal/repeated) this reuses
// the existing nonbase dispatcher above with no recomputation, and
// reports `closed`: raw-corona equality and the Red rank theorems for
// those four phases are proved universal for a>=7 in
// docs/THEOREM_STATUS.md ("Formalization queue" section), not merely
// bounded-checked -- that is what licenses `closed` here rather than
// a bounded-sweep claim.
//
// For round in {1,2,3,4} this does NOT attempt a catalogue: no
// symbolic pre-Red/post-Red construction for these rounds exists
// anywhere in this codebase (confirmed 2026-07-30 by direct search;
// `class_ii_neighbor2_signed_contact_set()` is only checked against
// literal corona-trace output for 3<=a<=8 in
// `app/class_ii_neighbor_probe.cpp`, which is exact finite evidence,
// not a universal reverse-inclusion proof). This object instead
// reports the honest base-premise ledger from the doc's own table.
// Do not set any base round's status to `closed` without a proved
// certificate to cite in this comment; as of 2026-07-30 all four are
// open.
//
// **Possibly stale as of 2026-08-02, flagged not fixed.** The doc's own
// base-premises table (`docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md`)
// now reads "Round 1's row is closed" (via "Round 1: the literature
// check, done", 2026-07-31 night: the paper's `Ĝ_C = ±C` is definitional,
// so raw-corona reverse inclusion for round 1 was never an independent
// obligation) and records round 4's bridge as closed the same night
// ("the remaining gap... it closes too", a provable-constant argument
// for a>=7). Both postdate this function's "as of 2026-07-30" comment
// above and its hardcoded `open_reverse_inclusion`/`open_bridge`
// returns for rounds 1 and 4. NOT changed here: it is not yet verified
// whether this enum's specific technical sense of "reverse inclusion"
// (a raw-corona base-case induction fact) is the same claim the
// literature argument closed, or a narrower/different one -- getting
// that wrong in either direction (silently stale vs. a false `closed`
// in load-bearing proof-tracking code) is worse than leaving it
// flagged. Needs a careful read of both closures against this file's
// exact definitions before touching the enum values.
enum class ClassIINeighbor2InductionStatus {
    closed,
    open_reverse_inclusion,  // missing: raw-corona reverse inclusion
    open_red_exclusion,      // missing: full Red exclusion
    open_bridge,             // missing: connect to the stable theorem
                             // whose own domain starts at round 5
                             // (round 4 specifically)
};

struct ClassIINeighbor2BaseRoundPremises {
    long long round = 0;
    // Every one of these three is independently attested for every
    // row of the base-premise table in
    // docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md
    // ("The four base premises, explicitly"):
    bool center_contact_base_closed = false;      // universal 9->16->14
    bool positive_witnesses_and_bridges_closed = false;
        // composition witnesses, Red successors/retention/bridges,
        // per round -- named differently per row in the doc table,
        // but every row lists this class of premise as already closed.
    bool window_validity_closed = false;          // stepped-hyperplane
                                                   // membership
    // What the doc table's own "still needed" column names for this
    // round. Reported as a single first-missing status per the
    // doc's explicit instruction; rounds 2 and 3 are missing BOTH
    // reverse inclusion and Red exclusion, and this reports whichever
    // is logically prior (reverse inclusion has to hold before a Red
    // rank on the correct raw set is even well posed).
    ClassIINeighbor2InductionStatus status =
        ClassIINeighbor2InductionStatus::open_reverse_inclusion;
};

inline ClassIINeighbor2BaseRoundPremises
class_ii_neighbor2_base_round_premises(long long round) {
    if (round < 1 || round > 4)
        throw std::domain_error(
            "Class-II neighbor-2 base round premises are only named"
            " for rounds 1..4");
    ClassIINeighbor2BaseRoundPremises result;
    result.round = round;
    result.center_contact_base_closed = true;
    result.positive_witnesses_and_bridges_closed = true;
    result.window_validity_closed = true;
    result.status = (round == 4)
        ? ClassIINeighbor2InductionStatus::open_bridge
        : ClassIINeighbor2InductionStatus::open_reverse_inclusion;
    return result;
}

struct ClassIINeighbor2GlobalInductionCertificate {
    long long a = 0;
    long long round = 0;
    bool is_base_round = false;
    ClassIINeighbor2InductionStatus status =
        ClassIINeighbor2InductionStatus::open_reverse_inclusion;
    // Populated only when !is_base_round. Sizes only, not the sets
    // themselves -- a caller that needs the literal catalogue already
    // has class_ii_neighbor2_nonbase_pre_red_catalogue /
    // _post_red_catalogue directly; duplicating the sets here would
    // be exactly the "recompute a local elimination" this object is
    // supposed to avoid.
    std::size_t pre_red_size = 0;
    std::size_t post_red_size = 0;
};

inline ClassIINeighbor2GlobalInductionCertificate
class_ii_neighbor2_global_induction_certificate(
        long long a, long long round) {
    ClassIINeighbor2GlobalInductionCertificate result;
    result.a = a;
    result.round = round;
    const auto phase = class_ii_neighbor2_global_round_phase(a, round);
    if (phase == ClassIINeighbor2GlobalRoundPhase::base) {
        result.is_base_round = true;
        result.status =
            class_ii_neighbor2_base_round_premises(round).status;
        return result;
    }
    result.is_base_round = false;
    result.status = ClassIINeighbor2InductionStatus::closed;
    result.pre_red_size =
        class_ii_neighbor2_nonbase_pre_red_catalogue(a, round).size();
    result.post_red_size =
        class_ii_neighbor2_nonbase_post_red_catalogue(a, round).size();
    return result;
}

// Scans every round 1..a+1 in order and returns the first one whose
// status is not `closed` -- the literal "report the first missing
// local premise" object the doc asks for. Not reachable to return a
// fully-closed result as of 2026-07-30: round 1 is always in domain
// and always open, so every call with a valid `a` returns round 1,
// open_reverse_inclusion. That is the honest current state of the
// global occurrence theorem, not a placeholder; the enrolled test
// below asserts exactly this, and updating that assertion is itself
// the signal that a base round has actually closed.
inline ClassIINeighbor2GlobalInductionCertificate
class_ii_neighbor2_first_missing_premise(long long a) {
    for (long long round = 1; round <= a + 1; ++round) {
        auto cert =
            class_ii_neighbor2_global_induction_certificate(a, round);
        if (cert.status != ClassIINeighbor2InductionStatus::closed)
            return cert;
    }
    ClassIINeighbor2GlobalInductionCertificate all_closed;
    all_closed.a = a;
    all_closed.round = 0;
    all_closed.status = ClassIINeighbor2InductionStatus::closed;
    return all_closed;
}

} // namespace ravel
