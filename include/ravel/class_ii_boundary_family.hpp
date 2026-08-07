// Symbolic affine shells in the fixed-contact corona for sigma_{a,1}.
//
// For an interior corona round r (4 <= r < a), the literature-correct
// fixed-±C process adds exactly these twenty nodes.  The formula is
// independent of a; a enters only by deciding how many shells fit
// before the terminal stepped-hyperplane boundary.

#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/corona.hpp"

namespace ravel {

inline std::set<ANode<3>> class_ii_d_cont_face_candidates() {
    std::set<ANode<3>> result;
    for (long long i = 0; i < 3; ++i)
        for (long long j = 0; j < 3; ++j)
            for (long long x0 = -2; x0 <= 2; ++x0)
                for (long long x1 = -2; x1 <= 2; ++x1)
                    for (long long x2 = -2; x2 <= 2; ++x2) {
                        const std::array<long long, 3> x{x0, x1, x2};
                        if (x0 == 0 && x1 == 0 && x2 == 0 && i >= j)
                            continue;
                        if (d_cont_face_intersection_dim<3>(x, i, j) != 1)
                            continue;
                        result.insert({i, x, j});
                    }
    // This ANALYTIC geometric construction (not a hardcoded table)
    // produces the 33-state set already verified, entry-by-entry, to
    // equal `dContFaceCandidateNode`'s range in
    // `lean/class_ii_affine_shells.lean`. Thread the ACTUAL concrete
    // nodes so the renderer decides membership of THESE nodes, not a
    // citation keyed by name. The window-validity iff
    // (`class_ii_dCont_face_candidate_valid_iff`) and the x0-bound
    // theorem (`class_ii_rawContact_x0_bounded`) both additionally
    // need a CONCRETE `a` and an exact Perron-root bracket to
    // instantiate honestly (same pattern as Finding 32) -- this
    // parameter-free function has no `a` to supply, so those two
    // citations are correctly NOT wired here, not silently assumed.
    if (mathlib::reflection::enabled()) {
        mathlib::reflection::ClassIIFixedTableCertificate node;
        node.table = "d_cont_face_candidates";
        for (const auto& n : result) node.nodes.push_back({n.i, n.x[0], n.x[1], n.x[2], n.j});
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
    return result;
}

inline std::set<ANode<3>> class_ii_d_cont_set() {
    std::set<ANode<3>> result = {
        {0, {0,  0,  1}, 0},
        {0, {0,  1,  0}, 0},
        {0, {0,  0,  0}, 1},
        {0, {0,  0,  0}, 2},
        {1, {1, -1,  0}, 0},
        {1, {0,  0,  1}, 1},
        {1, {0,  0,  0}, 2},
        {2, {1,  0, -1}, 0},
        {2, {0,  1, -1}, 1},
    };
    // Thread the ACTUAL concrete nodes just constructed above -- not a
    // name -- so the renderer can decide membership of THESE nodes in
    // `dContNodeD`'s range: a divergence between this table and
    // `lean/class_ii_affine_shells.lean`'s `dContNode` would make the
    // kernel check legitimately fail, not silently miss it.
    if (mathlib::reflection::enabled()) {
        mathlib::reflection::ClassIIFixedTableCertificate node;
        node.table = "d_cont";
        for (const auto& n : result) node.nodes.push_back({n.i, n.x[0], n.x[1], n.x[2], n.j});
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
    return result;
}

inline std::set<ANode<3>>
class_ii_pre_contact_first_backward_layer() {
    return {
        {1, { 1, -1,  0}, 2},
        {2, { 0,  1,  0}, 0},
        {0, { 0,  0,  1}, 1},
        {2, { 0,  1, -1}, 0},
        {0, { 1, -1,  0}, 0},
        {0, {-1,  1,  1}, 1},
    };
}

inline std::set<ANode<3>>
class_ii_pre_contact_second_backward_layer() {
    return {
        {1, {1, 0, -1}, 0},
    };
}

struct ClassIIPreContactBackwardWitness {
    ANode<3> predecessor;
    ANode<3> destination;
    int type = 1;
    std::size_t first_prefix_position = 0;
    std::size_t second_prefix_position = 0;
};

inline std::vector<ClassIIPreContactBackwardWitness>
class_ii_pre_contact_backward_witnesses(std::size_t a) {
    if (a < 2)
        throw std::domain_error(
            "Class-II 9+6+1 backward layers require a >= 2");
    return {
        {{0, {-1, 1,  1}, 1}, {2, {1, 0, -1}, 0}, 1, 0, a},
        {{0, { 0, 0,  1}, 1}, {0, {0, 1,  0}, 0}, 1, 0, a},
        {{0, { 1,-1,  0}, 0}, {1, {1,-1,  0}, 0}, 2, 1, 0},
        {{1, { 1,-1,  0}, 2}, {0, {0, 0,  1}, 0}, 1, a, a + 1},
        {{2, { 0, 1, -1}, 0}, {0, {0, 1,  0}, 0}, 2, a + 1, 0},
        {{2, { 0, 1,  0}, 0}, {0, {0, 0,  1}, 0}, 2, a + 1, a - 1},
        {{1, { 1, 0, -1}, 0}, {2, {0, 1, -1}, 0}, 2, a, 0},
    };
}

using ClassIIBackwardCategory = std::array<long long, 4>;

// (left, x1, x2, right) categories of every raw predecessor of the
// sixteen-state pre-contact catalogue. Only x0 varies with prefix
// positions and with a.
inline std::set<ClassIIBackwardCategory>
class_ii_pre_contact_backward_categories() {
    return {
        {0,-2, 0,2}, {0,-1,-1,0}, {0,-1,-1,2},
        {0,-1, 0,0}, {0,-1, 0,1}, {0,-1, 0,2},
        {0,-1, 1,2}, {0, 0,-1,0}, {0, 0,-1,1},
        {0, 0, 0,0}, {0, 0, 0,1}, {0, 0, 0,2},
        {0, 0, 1,0}, {0, 0, 1,1}, {0, 0, 1,2},
        {0, 1, 0,0}, {0, 1, 0,1}, {0, 1, 1,0},
        {0, 1, 1,1},
        {1,-2, 0,2}, {1,-1,-1,0}, {1,-1,-1,2},
        {1,-1, 0,0}, {1,-1, 0,1}, {1,-1, 0,2},
        {1,-1, 1,2}, {1, 0,-1,0}, {1, 0,-1,1},
        {1, 0, 0,0}, {1, 0, 0,1}, {1, 0, 0,2},
        {1, 0, 1,0}, {1, 0, 1,1}, {1, 1, 0,0},
        {1, 1, 0,1},
        {2,-1, 0,2}, {2, 0,-1,0}, {2, 0,-1,2},
        {2, 0, 0,0}, {2, 0, 0,1}, {2, 0, 0,2},
        {2, 0, 1,2}, {2, 1,-1,0}, {2, 1,-1,1},
        {2, 1, 0,0}, {2, 1, 0,1}, {2, 1, 0,2},
        {2, 1, 1,0}, {2, 1, 1,1}, {2, 2, 0,0},
        {2, 2, 0,1},
    };
}

struct ClassIIBackwardCategoryRange {
    long long minimum_intercept;
    long long minimum_slope;
    long long maximum_intercept;
    long long maximum_slope;
};

// Candidate affine x0 envelope inferred from consecutive exact
// constructions and checked by the literature probe. Deriving it
// directly from symbolic prefix segments is part of the remaining
// reverse-inclusion proof.
inline std::map<ClassIIBackwardCategory, ClassIIBackwardCategoryRange>
class_ii_pre_contact_backward_category_ranges() {
    using R = ClassIIBackwardCategoryRange;
    return {
        {{0,-2, 0,2}, R{ 0,-1,-1, 0}},
        {{0,-1,-1,0}, R{ 2,-2, 0, 0}},
        {{0,-1,-1,2}, R{ 0,-2, 0,-1}},
        {{0,-1, 0,0}, R{ 1,-1, 0, 1}},
        {{0,-1, 0,1}, R{ 0,-1,-1, 0}},
        {{0,-1, 0,2}, R{-1,-1, 0, 0}},
        {{0,-1, 1,2}, R{-1, 0,-1, 1}},
        {{0, 0,-1,0}, R{ 1,-2, 0, 0}},
        {{0, 0,-1,1}, R{ 0,-2,-1,-1}},
        {{0, 0, 0,0}, R{ 0,-1, 0, 1}},
        {{0, 0, 0,1}, R{-1,-1, 0, 0}},
        {{0, 0, 0,2}, R{-1,-1, 0, 0}},
        {{0, 0, 1,0}, R{ 0, 0,-1, 2}},
        {{0, 0, 1,1}, R{-1, 0,-1, 1}},
        {{0, 0, 1,2}, R{-1, 0,-2, 1}},
        {{0, 1, 0,0}, R{ 0,-1,-1, 1}},
        {{0, 1, 0,1}, R{-1,-1,-1, 0}},
        {{0, 1, 1,0}, R{ 0, 0,-2, 2}},
        {{0, 1, 1,1}, R{-1, 0,-2, 1}},
        {{1,-2, 0,2}, R{ 0, 0, 0, 0}},
        {{1,-1,-1,0}, R{ 2,-1, 1, 0}},
        {{1,-1,-1,2}, R{ 0,-1, 1,-1}},
        {{1,-1, 0,0}, R{ 1, 0, 1, 1}},
        {{1,-1, 0,1}, R{ 0, 0, 0, 0}},
        {{1,-1, 0,2}, R{-1, 0, 1, 0}},
        {{1,-1, 1,2}, R{ 0, 1, 0, 1}},
        {{1, 0,-1,0}, R{ 1,-1, 1, 0}},
        {{1, 0,-1,1}, R{ 0,-1, 0,-1}},
        {{1, 0, 0,0}, R{ 0, 0, 1, 1}},
        {{1, 0, 0,1}, R{-1, 0, 1, 0}},
        {{1, 0, 0,2}, R{ 0, 0, 1, 0}},
        {{1, 0, 1,0}, R{ 1, 1, 0, 2}},
        {{1, 0, 1,1}, R{ 0, 1, 0, 1}},
        {{1, 1, 0,0}, R{ 1, 0, 0, 1}},
        {{1, 1, 0,1}, R{ 0, 0, 0, 0}},
        {{2,-1, 0,2}, R{-1, 0, 0, 0}},
        {{2, 0,-1,0}, R{ 2,-1, 1, 0}},
        {{2, 0,-1,2}, R{ 0,-1, 1,-1}},
        {{2, 0, 0,0}, R{ 0, 0, 1, 1}},
        {{2, 0, 0,1}, R{-1, 0, 0, 0}},
        {{2, 0, 0,2}, R{-1, 0, 1, 0}},
        {{2, 0, 1,2}, R{-1, 1, 0, 1}},
        {{2, 1,-1,0}, R{ 1,-1, 1, 0}},
        {{2, 1,-1,1}, R{ 0,-1, 0,-1}},
        {{2, 1, 0,0}, R{ 0, 0, 1, 1}},
        {{2, 1, 0,1}, R{-1, 0, 1, 0}},
        {{2, 1, 0,2}, R{ 0, 0, 1, 0}},
        {{2, 1, 1,0}, R{ 0, 1, 0, 2}},
        {{2, 1, 1,1}, R{-1, 1, 0, 1}},
        {{2, 2, 0,0}, R{ 1, 0, 0, 1}},
        {{2, 2, 0,1}, R{ 0, 0, 0, 0}},
    };
}

// Candidate closed form for the restricted pre-contact graph G_P.
// The two states absent from class_ii_contact_set are precisely the
// rank-one Red exclusions.
inline std::set<ANode<3>> class_ii_pre_contact_set() {
    std::set<ANode<3>> result = {
        {0, {-1,  1,  1}, 1},
        {0, { 0,  0,  0}, 1},
        {0, { 0,  0,  0}, 2},
        {0, { 0,  0,  1}, 0},
        {0, { 0,  0,  1}, 1},
        {0, { 0,  1,  0}, 0},
        {0, { 1, -1,  0}, 0},
        {1, { 0,  0,  0}, 2},
        {1, { 0,  0,  1}, 1},
        {1, { 1, -1,  0}, 0},
        {1, { 1, -1,  0}, 2},
        {1, { 1,  0, -1}, 0},
        {2, { 0,  1, -1}, 0},
        {2, { 0,  1, -1}, 1},
        {2, { 0,  1,  0}, 0},
        {2, { 1,  0, -1}, 0},
    };
    // Thread the ACTUAL concrete nodes -- the renderer decides
    // membership of THESE nodes in `preContactNodeD`'s range.
    if (mathlib::reflection::enabled()) {
        mathlib::reflection::ClassIIFixedTableCertificate node;
        node.table = "pre_contact";
        for (const auto& n : result) node.nodes.push_back({n.i, n.x[0], n.x[1], n.x[2], n.j});
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
    return result;
}

inline std::set<ANode<3>> class_ii_contact_red_exclusions() {
    return {
        {1, {0, 0,  1}, 1},
        {2, {0, 1, -1}, 1},
    };
}

// Candidate closed form for the reduced contact set C of sigma_{a,1}.
// Exact construction currently agrees on the enrolled finite sweep;
// a universal backward-closure/Red proof remains separate.
inline std::set<SNode<3>> class_ii_contact_set() {
    std::set<SNode<3>> result = {
        {0, {-1,  1,  1}, 1},
        {0, { 0,  0,  0}, 1},
        {0, { 0,  0,  0}, 2},
        {0, { 0,  0,  1}, 0},
        {0, { 0,  0,  1}, 1},
        {0, { 0,  1,  0}, 0},
        {0, { 1, -1,  0}, 0},
        {1, { 0,  0,  0}, 2},
        {1, { 1, -1,  0}, 0},
        {1, { 1, -1,  0}, 2},
        {1, { 1,  0, -1}, 0},
        {2, { 0,  1, -1}, 0},
        {2, { 0,  1,  0}, 0},
        {2, { 1,  0, -1}, 0},
    };
    // Thread the ACTUAL concrete nodes -- the renderer decides
    // membership of THESE nodes in `contactNodeD`'s range. The
    // window-validity fact `class_ii_contactNode_valid` additionally
    // needs a CONCRETE `a` and an exact bracket for the Class-II
    // Perron root to instantiate honestly (matching Finding 32's
    // pattern) -- this parameter-free function has no `a` to supply,
    // so that citation is correctly NOT wired here; a genuine
    // per-instance version needs a concrete `a` and an exact bracket
    // for the Perron root (same pattern as Finding 32's depressed-cubic
    // certificate) -- not yet built, not silently assumed.
    if (mathlib::reflection::enabled()) {
        mathlib::reflection::ClassIIFixedTableCertificate node;
        node.table = "contact";
        for (const auto& n : result) node.nodes.push_back({n.i, n.x[0], n.x[1], n.x[2], n.j});
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
    return result;
}

// Parameter-independent third fixed-contact layer.  Together with the
// indexed shells below, this is the finite base of the Class-II center
// layer grammar.
inline std::set<SNode<3>> class_ii_stable_base() {
    std::set<SNode<3>> result;
    const auto add = [&](long long i, long long x0, long long x1,
                         long long x2, long long j) {
        result.insert({i, {x0, x1, x2}, j});
    };
    add(0, -3, 3, 0, 0);
    add(0, -3, 3, 0, 1);
    add(0, -2, 2, -1, 0);
    add(0, -2, 2, 0, 0);
    add(0, -2, 2, 0, 1);
    add(0, -2, 3, 0, 0);
    add(0, -2, 3, 0, 1);
    add(0, -1, 0, 1, 1);
    add(0, -1, 1, -1, 0);
    add(0, -1, 1, 0, 0);
    add(0, -1, 1, 0, 1);
    add(0, -1, 1, 0, 2);
    add(0, -1, 1, 1, 1);
    add(0, -1, 2, -1, 0);
    add(0, -1, 2, 0, 0);
    add(0, -1, 2, 0, 1);
    add(0, 0, -1, 0, 0);
    add(0, 0, -1, 1, 0);
    add(0, 0, -1, 1, 1);
    add(0, 0, 0, -1, 0);
    add(0, 0, 0, 0, 1);
    add(0, 0, 0, 0, 2);
    add(0, 0, 0, 1, 0);
    add(0, 0, 0, 1, 1);
    add(0, 0, 1, -1, 0);
    add(0, 0, 1, 0, 0);
    add(0, 1, -2, 0, 0);
    add(0, 1, -2, 1, 0);
    add(0, 1, -2, 1, 1);
    add(0, 1, -1, 0, 0);
    add(0, 1, -1, 1, 0);
    add(0, 1, -1, 1, 1);
    add(0, 2, -3, 0, 0);
    add(0, 2, -2, 0, 0);
    add(0, 2, -2, 1, 0);
    add(0, 2, -2, 1, 1);
    add(0, 3, -3, 0, 0);
    add(1, -2, 2, -1, 0);
    add(1, -1, 1, -1, 0);
    add(1, -1, 2, -1, 0);
    add(1, 0, -1, 0, 0);
    add(1, 0, 0, -1, 0);
    add(1, 0, 0, 0, 0);
    add(1, 0, 0, 0, 2);
    add(1, 0, 1, -1, 0);
    add(1, 1, -2, 0, 0);
    add(1, 1, -1, -1, 0);
    add(1, 1, -1, 0, 0);
    add(1, 1, -1, 0, 2);
    add(1, 1, 0, -1, 0);
    add(1, 2, -3, 0, 0);
    add(1, 2, -2, 0, 0);
    add(1, 3, -3, 0, 0);
    add(2, -2, 2, -1, 0);
    add(2, -2, 3, -1, 0);
    add(2, -1, 1, -1, 0);
    add(2, -1, 1, 0, 1);
    add(2, -1, 2, -1, 0);
    add(2, 0, -1, 0, 0);
    add(2, 0, 0, -1, 0);
    add(2, 0, 0, 0, 0);
    add(2, 0, 0, 0, 1);
    add(2, 0, 1, -1, 0);
    add(2, 0, 1, 0, 0);
    add(2, 1, -2, 0, 0);
    add(2, 1, -1, 0, 0);
    add(2, 1, 0, -1, 0);
    add(2, 2, -2, 0, 0);
    return result;
}

inline std::set<SNode<3>> class_ii_interior_shell(std::size_t r) {
    const auto q = static_cast<long long>(r);
    std::set<SNode<3>> shell;
    const auto add = [&](long long i, long long x0, long long x1,
                         long long x2, long long j) {
        shell.insert(SNode<3>{i, {x0, x1, x2}, j});
    };

    add(0, -q, q, 0, 0);
    add(0, -q, q, 0, 1);
    add(0, -(q - 1), q - 1, -1, 0);
    add(0, -(q - 1), q, 0, 0);
    add(0, -(q - 1), q, 0, 1);
    add(0, -(q - 2), q - 1, -1, 0);

    add(0, q - 2, -(q - 1), 1, 0);
    add(0, q - 2, -(q - 1), 1, 1);
    add(0, q - 1, -q, 0, 0);
    add(0, q - 1, -(q - 1), 1, 0);
    add(0, q - 1, -(q - 1), 1, 1);
    add(0, q, -q, 0, 0);

    add(1, -(q - 1), q - 1, -1, 0);
    add(1, -(q - 2), q - 1, -1, 0);
    add(1, q - 1, -q, 0, 0);
    add(1, q, -q, 0, 0);

    add(2, -(q - 1), q - 1, -1, 0);
    add(2, -(q - 1), q, -1, 0);
    add(2, q - 2, -(q - 1), 0, 0);
    add(2, q - 1, -(q - 1), 0, 0);

    // This is exactly the 20-state table `shellNode : ShellKind -> Int
    // -> ClassIINode` in the already kernel-checked
    // `lean/class_ii_affine_shells.lean`. Record the CONCRETE round
    // `q` this call actually built -- the renderer instantiates
    // `shellNode_propagates`/`shellNode_injective_at_round` AT this
    // exact `q` via `decide` (a genuine, mechanically-checked
    // per-instance corollary, not a citation of the abstract
    // ∀-quantified fact).
    if (mathlib::reflection::enabled()) {
        mathlib::reflection::ClassIIShellRoundCertificate node;
        node.q = q;
        node.description = "interior-shell round q=" + std::to_string(q);
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }

    return shell;
}

// At r=a, the stepped-hyperplane boundary replaces the two extreme
// same-colour states of the interior template with two cross-colour
// states.  This is the terminal correction that stops shell growth.
inline std::set<SNode<3>> class_ii_terminal_shell(std::size_t a) {
    const auto q = static_cast<long long>(a);
    auto shell = class_ii_interior_shell(a);
    shell.erase(SNode<3>{0, {-q, q, 0}, 0});
    shell.erase(SNode<3>{0, {q, -q, 0}, 0});
    shell.insert(SNode<3>{0, {q - 1, -q, 1}, 2});
    shell.insert(SNode<3>{2, {-(q - 1), q - 1, -1}, 1});
    return shell;
}

inline std::set<SNode<3>> class_ii_center_layer_candidate(
        std::size_t a, std::size_t round) {
    if (a < 4 || round < 3)
        throw std::domain_error(
            "Class-II center stable layer requires a>=4 and round>=3");
    auto result = class_ii_stable_base();
    const std::size_t last_interior =
        round < a ? round : a - 1;
    for (std::size_t shell = 4; shell <= last_interior; ++shell) {
        const auto nodes = class_ii_interior_shell(shell);
        result.insert(nodes.begin(), nodes.end());
    }
    if (round >= a) {
        const auto nodes = class_ii_terminal_shell(a);
        result.insert(nodes.begin(), nodes.end());
    }
    return result;
}

// The non-dominant recurrent SCCs sit on the x0=-x1 axis.  Index k
// is their Perron root.  k=1 is the two-state terminal component;
// 2<=k<a gives a four-state component at radius m=a-k+1.
inline std::set<SNode<3>> class_ii_recurrent_shell_component(
        std::size_t a, std::size_t k) {
    std::set<SNode<3>> component;
    if (k == 0 || k >= a) return component;
    const auto m = static_cast<long long>(a - k + 1);
    component.insert(SNode<3>{1, {m, -m, 0}, 0});
    component.insert(SNode<3>{0, {-m, m, 0}, 1});
    if (k >= 2) {
        component.insert(SNode<3>{0, {m, -m, 0}, 0});
        component.insert(SNode<3>{0, {-m, m, 0}, 0});
    }
    return component;
}

inline std::vector<std::vector<long long>>
class_ii_recurrent_shell_matrix(std::size_t k) {
    if (k == 1) return {{0, 1}, {1, 0}};
    if (k < 2) return {};
    const auto r = static_cast<long long>(k - 1);
    return {
        {0, 0, r, r},
        {0, 0, 1, 1},
        {r, r, 0, 0},
        {1, 1, 0, 0},
    };
}

}  // namespace ravel
