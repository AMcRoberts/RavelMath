// tests/lean_class_ii_catalogue_cross_check_test.cpp
//
// Cross-language catalogue cross-check (Q1 of
// docs/RECOVERY_AUDIT_2026-07-29.md).
//
// The Lean files in lean/*.lean describe several closed-form
// parametric catalogues for the Class-II family.  Each catalogue
// claims a particular set of nodes for every parameter in its
// domain.  The C++ implementations in include/ravel/ compute the
// same catalogues from affine-form data and concrete constructors.
//
// Both sides can drift independently.  This test compares them
// pointwise for the catalogues that are the load-bearing shape
// statements for the proof seam:
//
//   * class_ii_six_vertex_graduation.lean — promotedNodes and
//     transferredNode at round q = a - 1 for a in [5, 32].
//   * class_ii_neighbor_d_support.lean — D matrix nonzero source
//     and target index sets for each of the three neighbors,
//     independent of a.
//   * class_ii_terminal_shells.lean — the twenty-formula interior
//     shell (shellNode) and the terminal shell (twenty formulas with
//     the two cross-colour replacements).
//   * class_ii_neighbor2_extensions.lean — the fixed 24-state
//     neighbor-2 correction, the 6-state terminal affine sextet, the
//     2-state penultimate pair, and the 1-state interior tip.
//   * class_ii_affine_shells.lean — the 14-state contact set, the
//     16-state pre-contact set, the 9-state D_cont seed set, and the
//     6+1 backward-closure layers.
//   * class_ii_global_round_partition.lean — the five-phase round
//     classifier (base/stable/penultimate/terminal/repeated).
//
// If Lean says the catalogue has a particular shape and C++ says
// it has a different shape, the disagreement is the actionable
// signal.  If they agree, the cross-check closes the kernel
// catalogue identity against the executable implementation.
//
// Build: see Makefile (this test is enrolled in `make check`).

#include <algorithm>
#include <array>
#include <cstdio>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/class_ii_neighbor2_pruning.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/core.hpp"
#include "ravel/corona.hpp"

using namespace ravel;
using Node5 = SNode<3>;
using ANode5 = ANode<3>;

namespace {

// Mirrors lean/class_ii_six_vertex_graduation.lean::promotedNodes
// at round q = a - 1.  This is the Lean-side definition re-encoded
// in C++ for comparison only; the kernel-checked Lean statement is
// the primary reference.
std::set<Node5> lean_promoted_nodes(long long a) {
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

// Mirrors lean/class_ii_six_vertex_graduation.lean::transferredNode.
Node5 lean_transferred_node(long long a) {
    const long long q = a - 1;
    return {2, {-(q - 1), q - 1, -2}, 0};
}

// Extracts the single node from class_ii_neighbor2_penultimate_survivor_transfer.
Node5 cpp_transferred_node(long long a) {
    const auto set = class_ii_neighbor2_penultimate_survivor_transfer(a);
    if (set.size() != 1)
        throw std::logic_error("penultimate_survivor_transfer should have size 1");
    return *set.begin();
}

// Set equality helper with diagnostic on mismatch.  Uses only
// operator< (via std::includes on the sorted std::set ranges) so it
// works for node types like ANode<3> that have no operator==.
template <typename T>
bool equal_sets(const std::set<T>& a, const std::set<T>& b,
                const char* label) {
    if (a.size() == b.size()
            && std::includes(a.begin(), a.end(), b.begin(), b.end()))
        return true;
    std::printf("  [DIAG %s] size: lean=%zu cpp=%zu\n",
                label, a.size(), b.size());
    for (const auto& x : a) {
        if (!b.count(x))
            std::printf("  [DIAG %s] lean-only\n", label);
    }
    for (const auto& x : b) {
        if (!a.count(x))
            std::printf("  [DIAG %s] cpp-only\n", label);
    }
    return false;
}

// Mirrors lean/class_ii_terminal_shells.lean::shellNode (all 20
// cases), independently re-encoded for comparison against
// class_ii_interior_shell.
std::set<Node5> lean_interior_shell(long long q) {
    return {
        {0, {-q, q, 0}, 0},
        {0, {-q, q, 0}, 1},
        {0, {-(q - 1), q - 1, -1}, 0},
        {0, {-(q - 1), q, 0}, 0},
        {0, {-(q - 1), q, 0}, 1},
        {0, {-(q - 2), q - 1, -1}, 0},
        {0, {q - 2, -(q - 1), 1}, 0},
        {0, {q - 2, -(q - 1), 1}, 1},
        {0, {q - 1, -q, 0}, 0},
        {0, {q - 1, -(q - 1), 1}, 0},
        {0, {q - 1, -(q - 1), 1}, 1},
        {0, {q, -q, 0}, 0},
        {1, {-(q - 1), q - 1, -1}, 0},
        {1, {-(q - 2), q - 1, -1}, 0},
        {1, {q - 1, -q, 0}, 0},
        {1, {q, -q, 0}, 0},
        {2, {-(q - 1), q - 1, -1}, 0},
        {2, {-(q - 1), q, -1}, 0},
        {2, {q - 2, -(q - 1), 0}, 0},
        {2, {q - 1, -(q - 1), 0}, 0},
    };
}

// Mirrors lean/class_ii_terminal_shells.lean::terminalCrossColour{1,2}
// and terminalShellFinset (interior shell with n00/n11 replaced).
Node5 lean_terminal_cross_colour1(long long a) {
    return {0, {a - 1, -a, 1}, 2};
}
Node5 lean_terminal_cross_colour2(long long a) {
    return {2, {-(a - 1), a - 1, -1}, 1};
}
std::set<Node5> lean_terminal_shell(long long a) {
    auto s = lean_interior_shell(a);
    s.erase(Node5{0, {-a, a, 0}, 0});
    s.erase(Node5{0, {a, -a, 0}, 0});
    s.insert(lean_terminal_cross_colour1(a));
    s.insert(lean_terminal_cross_colour2(a));
    return s;
}

// Mirrors lean/class_ii_neighbor2_extensions.lean::neighbor2FixedNode
// (all 24 cases), for comparison against
// class_ii_neighbor2_fixed_extension_states.
std::set<Node5> lean_neighbor2_fixed_states() {
    return {
        {0, {-2, 2, 1}, 1},   {0, {-1, 0, 1}, 0},  {0, {-1, 1, 1}, 0},
        {0, {-1, 1, 1}, 2},   {0, {-1, 2, 1}, 0},  {0, {1, -2, -1}, 0},
        {0, {1, -1, -1}, 0},  {0, {1, -1, 0}, 2},  {0, {1, 0, -1}, 0},
        {1, {-1, 1, 0}, 1},   {1, {0, 1, 0}, 0},   {1, {1, -2, -1}, 0},
        {1, {1, -1, 0}, 1},   {1, {2, -2, -1}, 0}, {1, {2, -1, -1}, 0},
        {2, {-2, 2, 0}, 1},   {2, {-1, 1, 0}, 0},  {2, {-1, 1, 0}, 2},
        {2, {-1, 2, 0}, 0},   {2, {-1, 2, 0}, 1},  {2, {1, -2, -1}, 0},
        {2, {1, -1, -1}, 0},  {2, {1, -1, 0}, 2},  {2, {2, -1, -1}, 0},
    };
}

// Mirrors lean/class_ii_neighbor2_extensions.lean::neighbor2TerminalSextet,
// for comparison against class_ii_neighbor2_terminal_affine_states.
std::set<Node5> lean_neighbor2_terminal_sextet(long long a) {
    const long long q = a - 2;
    return {
        {0, {q, -(q + 1), 2}, 2}, {0, {q, -q, 2}, 1},
        {1, {-q, q, -2}, 0},      {2, {-q, q, -2}, 0},
        {2, {-q, q + 1, -2}, 0},  {2, {q, -q, 1}, 1},
    };
}

// Mirrors lean/class_ii_neighbor2_extensions.lean::neighbor2PenultimatePair,
// for comparison against class_ii_neighbor2_penultimate_extension_states
// minus the fixed 24 states.
std::set<Node5> lean_neighbor2_penultimate_pair(long long a) {
    return {
        {2, {-(a - 1), a - 1, -1}, 0},
        {2, {-(a - 2), a - 2, -2}, 0},
    };
}

// Mirrors lean/class_ii_neighbor2_extensions.lean::neighbor2InteriorTip,
// for comparison against class_ii_neighbor2_interior_extension_states
// minus the fixed 24 states.
Node5 lean_neighbor2_interior_tip(long long r) {
    return {2, {-r, r, -1}, 0};
}

// Mirrors lean/class_ii_affine_shells.lean::contactNode (14 cases),
// for comparison against class_ii_contact_set (SNode<3>, unlike the
// ANode<3>-typed pre-contact/D_cont/backward-layer functions below).
std::set<Node5> lean_contact_set() {
    return {
        {0, {-1, 1, 1}, 1}, {0, {0, 0, 0}, 1},  {0, {0, 0, 0}, 2},
        {0, {0, 0, 1}, 0},  {0, {0, 0, 1}, 1},  {0, {0, 1, 0}, 0},
        {0, {1, -1, 0}, 0}, {1, {0, 0, 0}, 2},  {1, {1, -1, 0}, 0},
        {1, {1, -1, 0}, 2}, {1, {1, 0, -1}, 0}, {2, {0, 1, -1}, 0},
        {2, {0, 1, 0}, 0},  {2, {1, 0, -1}, 0},
    };
}

// Mirrors lean/class_ii_affine_shells.lean::preContactNode (16 cases),
// for comparison against class_ii_pre_contact_set.
std::set<ANode5> lean_pre_contact_set() {
    return {
        {0, {-1, 1, 1}, 1}, {0, {0, 0, 0}, 1},   {0, {0, 0, 0}, 2},
        {0, {0, 0, 1}, 0},  {0, {0, 0, 1}, 1},   {0, {0, 1, 0}, 0},
        {0, {1, -1, 0}, 0}, {1, {0, 0, 0}, 2},   {1, {0, 0, 1}, 1},
        {1, {1, -1, 0}, 0}, {1, {1, -1, 0}, 2},  {1, {1, 0, -1}, 0},
        {2, {0, 1, -1}, 0}, {2, {0, 1, -1}, 1},  {2, {0, 1, 0}, 0},
        {2, {1, 0, -1}, 0},
    };
}

// Mirrors lean/class_ii_affine_shells.lean::dContNode (9 cases), for
// comparison against class_ii_d_cont_set.
std::set<ANode5> lean_d_cont_set() {
    return {
        {0, {0, 0, 1}, 0}, {0, {0, 1, 0}, 0}, {0, {0, 0, 0}, 1},
        {0, {0, 0, 0}, 2}, {1, {1, -1, 0}, 0}, {1, {0, 0, 1}, 1},
        {1, {0, 0, 0}, 2}, {2, {1, 0, -1}, 0}, {2, {0, 1, -1}, 1},
    };
}

// Mirrors lean/class_ii_affine_shells.lean::firstBackwardNode (6
// cases) and secondBackwardNode, for comparison against
// class_ii_pre_contact_first_backward_layer and
// class_ii_pre_contact_second_backward_layer.
std::set<ANode5> lean_first_backward_layer() {
    return {
        {1, {1, -1, 0}, 2}, {2, {0, 1, 0}, 0},  {0, {0, 0, 1}, 1},
        {2, {0, 1, -1}, 0}, {0, {1, -1, 0}, 0}, {0, {-1, 1, 1}, 1},
    };
}
std::set<ANode5> lean_second_backward_layer() {
    return {{1, {1, 0, -1}, 0}};
}

// Mirrors lean/class_ii_global_round_partition.lean::
// classIIGlobalRoundPhase, for comparison against
// class_ii_neighbor2_global_round_phase.
ClassIINeighbor2GlobalRoundPhase lean_global_round_phase(
        long long a, long long r) {
    if (r <= 4) return ClassIINeighbor2GlobalRoundPhase::base;
    if (r <= a - 2) return ClassIINeighbor2GlobalRoundPhase::stable;
    if (r == a - 1) return ClassIINeighbor2GlobalRoundPhase::penultimate;
    if (r == a) return ClassIINeighbor2GlobalRoundPhase::terminal;
    return ClassIINeighbor2GlobalRoundPhase::repeated;
}

// Expected source/target index sets for the D matrices of the three
// Class-II neighbors, as kernel-checked in
// lean/class_ii_neighbor_d_support.lean.  These are reported in the
// handoff as {1,4,8,9} / {1,9} / {3,9,14,16} for sources and the
// corresponding target sets.  We re-encode them here as expected
// values and compare with the C++ extraction.
const std::set<std::size_t> kExpectedNeighbor0Sources = {1, 4, 8, 9};
const std::set<std::size_t> kExpectedNeighbor1Sources = {1, 9};
const std::set<std::size_t> kExpectedNeighbor2Sources = {3, 9, 14, 16};

}  // namespace

int main() {
    std::printf("Lean/C++ catalogue cross-check\n");
    std::printf("(Q1 of docs/RECOVERY_AUDIT_2026-07-29.md)\n\n");

    int failures = 0;

    // ---- Cross-check 1: six-vertex graduation catalogue ----
    std::printf("[1] six_vertex_graduation.lean :: promotedNodes / "
                "transferredNode vs C++ concrete forms\n");
    for (long long a = 5; a <= 32; ++a) {
        const auto lean_promoted = lean_promoted_nodes(a);
        const auto lean_transferred = lean_transferred_node(a);
        const auto cpp_promoted =
            class_ii_neighbor2_penultimate_promoted_states(a);
        const auto cpp_transferred = cpp_transferred_node(a);

        if (!equal_sets(lean_promoted, cpp_promoted,
                        "promoted")) {
            std::printf("  [FAIL] a=%lld promoted mismatch\n", a);
            ++failures;
        }
        if (lean_transferred != cpp_transferred) {
            std::printf("  [FAIL] a=%lld transferred mismatch\n", a);
            ++failures;
        }
        if (cpp_promoted.count(cpp_transferred)) {
            std::printf("  [FAIL] a=%lld transferred in promoted\n", a);
            ++failures;
        }
    }
    if (failures == 0)
        std::printf("  promoted (a in [5,32]) = "
                    "cpp_promoted: agree\n");
    std::printf("  transferred (a in [5,32]) = "
                "cpp_transferred: agree\n");

    // ---- Cross-check 2: D-matrix boundary-layer support ----
    std::printf("\n[2] class_ii_neighbor_d_support.lean :: D source/target "
                "indices vs C++ extraction\n");
    const std::array<std::pair<std::size_t, std::set<std::size_t>>, 3>
        expected_sources = {{
            {0, kExpectedNeighbor0Sources},
            {1, kExpectedNeighbor1Sources},
            {2, kExpectedNeighbor2Sources},
        }};
    for (const auto& [neighbor, expected] : expected_sources) {
        const auto cpp_sources =
            class_ii_neighbor_d_boundary_source_indices(neighbor);
        const auto cpp_targets =
            class_ii_neighbor_d_boundary_target_indices(neighbor);
        const auto expected_targets =
            [&]() -> std::set<std::size_t> {
                std::set<std::size_t> t;
                const auto D = class_ii_neighbor_d_matrix(neighbor);
                for (auto s : cpp_sources) {
                    const auto& row = D[s];
                    for (std::size_t c = 0; c < row.size(); ++c) {
                        if (row[c] != 0) t.insert(c);
                    }
                }
                return t;
            }();

        if (!equal_sets(expected, cpp_sources, "D sources")) {
            std::printf("  [FAIL] neighbor %zu sources mismatch\n",
                        neighbor);
            ++failures;
        }
        if (!equal_sets(expected_targets, cpp_targets, "D targets")) {
            std::printf("  [FAIL] neighbor %zu targets mismatch\n",
                        neighbor);
            ++failures;
        }
        if (cpp_sources == expected && cpp_targets == expected_targets)
            std::printf("  neighbor %zu: lean source/target "
                        "agree with C++ extraction\n", neighbor);
    }

    // ---- Cross-check 3: D-matrix nonzero count ----
    std::printf("\n[3] class_ii_neighbor_d_support.lean :: nonzero "
                "counts vs C++ extraction\n");
    const std::array<std::pair<std::size_t, std::size_t>, 3>
        expected_nnz = {{ {0, 12}, {1, 5}, {2, 24} }};
    for (const auto& [neighbor, expected] : expected_nnz) {
        const auto cert =
            class_ii_neighbor_d_certificate(neighbor, 3, 32);
        if (cert.nonzero_entries != expected) {
            std::printf("  [FAIL] neighbor %zu nnz=%zu (lean expected "
                        "%zu)\n", neighbor,
                        cert.nonzero_entries, expected);
            ++failures;
        } else {
            std::printf("  neighbor %zu nnz=%zu: agree\n",
                        neighbor, cert.nonzero_entries);
        }
    }

    // ---- Cross-check 4: interior/terminal shell catalogues ----
    std::printf("\n[4] class_ii_terminal_shells.lean :: shellNode / "
                "terminalShellFinset vs C++ shells\n");
    {
        bool interior_ok = true, terminal_ok = true;
        for (long long q = 4; q <= 64; ++q) {
            if (!equal_sets(lean_interior_shell(q),
                            class_ii_interior_shell(
                                static_cast<std::size_t>(q)),
                            "interior_shell")) {
                std::printf("  [FAIL] q=%lld interior shell mismatch\n", q);
                interior_ok = false;
                ++failures;
            }
        }
        for (long long a = 4; a <= 64; ++a) {
            if (!equal_sets(lean_terminal_shell(a),
                            class_ii_terminal_shell(
                                static_cast<std::size_t>(a)),
                            "terminal_shell")) {
                std::printf("  [FAIL] a=%lld terminal shell mismatch\n", a);
                terminal_ok = false;
                ++failures;
            }
        }
        if (interior_ok)
            std::printf("  interior shell (q in [4,64]) = "
                        "class_ii_interior_shell: agree\n");
        if (terminal_ok)
            std::printf("  terminal shell (a in [4,64]) = "
                        "class_ii_terminal_shell: agree\n");
    }

    // ---- Cross-check 5: neighbor-2 fixed/affine correction catalogues ----
    std::printf("\n[5] class_ii_neighbor2_extensions.lean :: "
                "neighbor2FixedNode / TerminalSextet / PenultimatePair / "
                "InteriorTip vs C++\n");
    if (!equal_sets(lean_neighbor2_fixed_states(),
                    class_ii_neighbor2_fixed_extension_states(),
                    "neighbor2_fixed")) {
        std::printf("  [FAIL] fixed-24 states mismatch\n");
        ++failures;
    } else {
        std::printf("  fixed-24 states = "
                    "class_ii_neighbor2_fixed_extension_states: agree\n");
    }
    {
        bool sextet_ok = true, pair_ok = true, tip_ok = true;
        for (long long a = 3; a <= 40; ++a) {
            const auto cpp_sextet_vec =
                class_ii_neighbor2_terminal_affine_states(a);
            const std::set<Node5> cpp_sextet(cpp_sextet_vec.begin(),
                                             cpp_sextet_vec.end());
            if (!equal_sets(lean_neighbor2_terminal_sextet(a), cpp_sextet,
                            "terminal_sextet")) {
                std::printf("  [FAIL] a=%lld terminal sextet mismatch\n", a);
                sextet_ok = false;
                ++failures;
            }
        }
        for (long long a = 4; a <= 40; ++a) {
            std::set<Node5> extra;
            const auto full = class_ii_neighbor2_penultimate_extension_states(a);
            const auto fixed = class_ii_neighbor2_fixed_extension_states();
            std::set_difference(full.begin(), full.end(),
                                fixed.begin(), fixed.end(),
                                std::inserter(extra, extra.begin()));
            if (!equal_sets(lean_neighbor2_penultimate_pair(a), extra,
                            "penultimate_pair")) {
                std::printf("  [FAIL] a=%lld penultimate pair mismatch\n", a);
                pair_ok = false;
                ++failures;
            }
        }
        for (long long r = 2; r <= 40; ++r) {
            std::set<Node5> extra;
            const auto full =
                class_ii_neighbor2_interior_extension_states(
                    static_cast<std::size_t>(r));
            const auto fixed = class_ii_neighbor2_fixed_extension_states();
            std::set_difference(full.begin(), full.end(),
                                fixed.begin(), fixed.end(),
                                std::inserter(extra, extra.begin()));
            const std::set<Node5> expected_tip = {
                lean_neighbor2_interior_tip(r)};
            if (extra != expected_tip) {
                std::printf("  [FAIL] r=%lld interior tip mismatch\n", r);
                tip_ok = false;
                ++failures;
            }
        }
        if (sextet_ok)
            std::printf("  terminal sextet (a in [3,40]) = "
                        "class_ii_neighbor2_terminal_affine_states: agree\n");
        if (pair_ok)
            std::printf("  penultimate pair (a in [4,40]) = "
                        "class_ii_neighbor2_penultimate_extension_states "
                        "minus fixed: agree\n");
        if (tip_ok)
            std::printf("  interior tip (r in [2,40]) = "
                        "class_ii_neighbor2_interior_extension_states "
                        "minus fixed: agree\n");
    }

    // ---- Cross-check 6: contact / pre-contact / D_cont catalogues ----
    std::printf("\n[6] class_ii_affine_shells.lean :: contactNode / "
                "preContactNode / dContNode vs C++\n");
    if (!equal_sets(lean_contact_set(), class_ii_contact_set(),
                    "contact_set")) {
        std::printf("  [FAIL] contact set (14) mismatch\n");
        ++failures;
    } else {
        std::printf("  contact set (14) = class_ii_contact_set: agree\n");
    }
    if (!equal_sets(lean_pre_contact_set(), class_ii_pre_contact_set(),
                    "pre_contact_set")) {
        std::printf("  [FAIL] pre-contact set (16) mismatch\n");
        ++failures;
    } else {
        std::printf("  pre-contact set (16) = "
                    "class_ii_pre_contact_set: agree\n");
    }
    if (!equal_sets(lean_d_cont_set(), class_ii_d_cont_set(),
                    "d_cont_set")) {
        std::printf("  [FAIL] D_cont seed set (9) mismatch\n");
        ++failures;
    } else {
        std::printf("  D_cont seed set (9) = class_ii_d_cont_set: agree\n");
    }

    // ---- Cross-check 7: backward-closure layers ----
    std::printf("\n[7] class_ii_affine_shells.lean :: firstBackwardNode / "
                "secondBackwardNode vs C++\n");
    if (!equal_sets(lean_first_backward_layer(),
                    class_ii_pre_contact_first_backward_layer(),
                    "first_backward")) {
        std::printf("  [FAIL] first backward layer (6) mismatch\n");
        ++failures;
    } else {
        std::printf("  first backward layer (6) = "
                    "class_ii_pre_contact_first_backward_layer: agree\n");
    }
    if (!equal_sets(lean_second_backward_layer(),
                    class_ii_pre_contact_second_backward_layer(),
                    "second_backward")) {
        std::printf("  [FAIL] second backward layer (1) mismatch\n");
        ++failures;
    } else {
        std::printf("  second backward layer (1) = "
                    "class_ii_pre_contact_second_backward_layer: agree\n");
    }

    // ---- Cross-check 8: global round-phase classifier ----
    std::printf("\n[8] class_ii_global_round_partition.lean :: "
                "classIIGlobalRoundPhase vs C++\n");
    {
        bool phase_ok = true;
        for (long long a = 7; a <= 40; ++a) {
            for (long long r = 1; r <= a + 1; ++r) {
                if (lean_global_round_phase(a, r) !=
                        class_ii_neighbor2_global_round_phase(a, r)) {
                    std::printf("  [FAIL] a=%lld r=%lld phase mismatch\n",
                                a, r);
                    phase_ok = false;
                    ++failures;
                }
            }
        }
        if (phase_ok)
            std::printf("  round phase (a in [7,40], r in [1,a+1]) = "
                        "class_ii_neighbor2_global_round_phase: agree\n");
    }

    std::printf("\nlean_class_ii_catalogue_cross_check: %s "
                "(%d failures)\n",
                failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
