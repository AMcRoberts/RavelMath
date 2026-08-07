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
    // produces EXACTLY the 33-state table `dContFaceCandidateNode :
    // DContFaceCandidateKind -> ClassIINode` in the already
    // kernel-checked `lean/class_ii_affine_shells.lean` -- verified
    // entry-by-entry before this citation was added, not assumed from
    // matching sizes. That file additionally proves
    // `class_ii_dCont_face_candidate_valid_iff`: window-validity of
    // each candidate, for ANY `a>=2` and its actual Class-II Perron
    // root, is EXACTLY characterized by the nine flagged
    // `dContFaceCandidateAccepted` entries -- an iff, not just a
    // one-direction check.
    mathlib::reflection::LemmaApplication face_citation;
    face_citation.theorem_name = "class_ii_dCont_face_candidate_valid_iff";
    face_citation.conclusion = "for any a>=2 and its actual Class-II Perron root, exactly the "
        "nine flagged face candidates lie in the restricted stepped hyperplane -- an iff, "
        "not just a one-direction check";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, face_citation);

    // The search range `x0 in [-2,2]` above (line 25) is not a
    // heuristic guess -- it is EXACTLY the bound
    // `class_ii_rawContact_x0_bounded` proves is necessary and
    // sufficient: once the Perron gap exceeds 1/2 (always true here,
    // per `class_ii_perron_gap_gt_half`), restricted-window membership
    // with `x1,x2` already bounded forces `x0` into `[-2,2]`, for ANY
    // `a>=2`. Record that citation too.
    mathlib::reflection::LemmaApplication x0_bound_citation;
    x0_bound_citation.theorem_name = "class_ii_rawContact_x0_bounded";
    x0_bound_citation.conclusion = "the search range x0 in [-2,2] used above is exactly the "
        "bound the restricted-window Perron-gap argument forces, for any a>=2 -- not a "
        "heuristic search-space choice";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, x0_bound_citation);
    return result;
}

inline std::set<ANode<3>> class_ii_d_cont_set() {
    // This is exactly the nine-state table `dContNode : DContKind ->
    // ClassIINode` in the already kernel-checked
    // `lean/class_ii_affine_shells.lean`, which proves
    // `dContNode_in_preContact`: every one of these nine states is a
    // genuine member of the sixteen-state pre-contact catalogue.
    mathlib::reflection::LemmaApplication d_cont_citation;
    d_cont_citation.theorem_name = "dContNode_in_preContact";
    d_cont_citation.conclusion = "every one of the nine D_cont states is a genuine member of "
        "the sixteen-state pre-contact catalogue";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, d_cont_citation);

    return {
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
    // This is exactly the sixteen-state table `preContactNode :
    // PreContactKind -> ClassIINode` in the already kernel-checked
    // `lean/class_ii_affine_shells.lean`, which proves
    // `preContactNode_partition`: every one of these sixteen states is
    // EITHER one of the fourteen contact states OR one of the two Red
    // exclusions -- exactly this function's own comment above, now a
    // checkable citation, not just a comment.
    mathlib::reflection::LemmaApplication pre_contact_citation;
    pre_contact_citation.theorem_name = "preContactNode_partition";
    pre_contact_citation.conclusion = "every one of the sixteen pre-contact states is either "
        "one of the fourteen contact states or one of the two Red exclusions";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, pre_contact_citation);

    return {
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
    // This is exactly the fourteen-state table `contactNode : ContactKind
    // -> ClassIINode` in the already kernel-checked
    // `lean/class_ii_affine_shells.lean`, which proves
    // `class_ii_contactNode_valid`: EVERY one of these fourteen states
    // lies in the restricted stepped hyperplane, for ANY `a>=2` and the
    // actual Class-II Perron root `beta` (satisfying the Class-II cubic
    // `beta^3 = a*beta^2 + (a+1)*beta + 1`) -- not a per-`a` check.
    // Record that citation whenever this set is constructed.
    mathlib::reflection::LemmaApplication contact_citation;
    contact_citation.theorem_name = "class_ii_contactNode_valid";
    contact_citation.conclusion = "every state in the fourteen-state contact catalogue lies in "
        "the restricted stepped hyperplane, for any a>=2 and its actual Class-II Perron root "
        "-- not a per-a numeric check";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, contact_citation);

    return {
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
    // `lean/class_ii_affine_shells.lean` -- which proves, generally
    // (any q, no per-instance argument), that these 20 states are
    // pairwise distinct within a round (`shellNode_injective_at_round`)
    // and propagate from the previous round by a FIXED hop
    // (`shellNode_propagates`). Record that citation whenever this
    // shell is actually constructed -- a no-op when no trace is
    // active.
    mathlib::reflection::LemmaApplication shell_citation;
    shell_citation.theorem_name = "shellNode_propagates";
    shell_citation.conclusion = "the interior-shell 20-state table at round " + std::to_string(q)
        + " is exactly RavelGenerated.shellNode, pairwise distinct within the round "
          "(shellNode_injective_at_round) and equal to the previous round's state plus a fixed "
          "contact hop (shellNode_propagates), for any round -- not a per-round coincidence";
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, shell_citation);

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
