// ravel/proof/class_ii_both_fixed_affine.hpp
//
// Extracted from app/class_ii_both_fixed_full_proof.cpp's inline
// main() computation (previously not callable from a test) so the
// concrete (CONST, slope, target, a_required) instances it finds can
// be threaded into the reflection trace. The general fact these
// instances feed -- an affine integer function with nonzero slope can
// equal a fixed target for at most one input, so if that unique
// solution is below a threshold it never re-hits the target above it
// -- is `affine_no_solution_at_or_above_threshold`, already
// kernel-checked in lean/class_ii_round234_shape_closure.lean.

#pragma once

#include <algorithm>
#include <cstddef>
#include <set>
#include <vector>

#include "math/proof_reflection.hpp"
#include "ravel/ambient_graph.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

namespace ravel::proof {

namespace detail_both_fixed {

using Matrix = std::vector<std::vector<long long>>;

inline FiniteSubstitution class_ii_both_fixed_substitution(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

inline Matrix both_fixed_incidence(const FiniteSubstitution& s) {
    Matrix m(3, std::vector<long long>(3, 0));
    for (std::size_t im = 0; im < 3; ++im)
        for (auto l : s[im]) ++m[static_cast<std::size_t>(l)][im];
    return m;
}

// (inner_letter, parent_letter, l1, l2, slope, intercept_b) -- the
// exhaustively-enumerated fixed occurrence types, reproduced exactly
// from the app file.
struct FixedType {
    long long inner, parent, l1, l2, slope, b;
};

inline const std::vector<FixedType>& both_fixed_types() {
    static const std::vector<FixedType> types = {
        {0, 2, 0, 0, 0, 0},
        {1, 0, 0, 0, 1, 0},
        {2, 0, 1, 0, 1, 1},
        {2, 1, 0, 0, 1, -1},
        {0, 1, 0, 1, 1, 0},
    };
    return types;
}

}  // namespace detail_both_fixed

// One (CONST, slope, target, a_required) instance found for a
// slope-nonzero (node, pt, qt, T) combination with an integer
// solution to CONST + a*slope = target.
struct BothFixedAffineInstance {
    long long const_ = 0;
    long long slope = 0;
    long long target = 0;
    long long a_required = 0;
};

// Reproduces app/class_ii_both_fixed_full_proof.cpp's core computation
// exactly (same pre_red set at the same reference a=15, same nested
// loops), returning every (node, pt, qt, T) instance with an integer
// solution -- not just the ones the app prints as counterexamples.
inline std::vector<BothFixedAffineInstance> class_ii_both_fixed_affine_instances() {
    using namespace detail_both_fixed;
    const long long a_ref = 15;
    const auto center = class_ii_both_fixed_substitution(static_cast<std::size_t>(a_ref));
    const auto spectral = classify_matrix_spectral(both_fixed_incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const Substitution<3> subst = make_substitution<3>(
        SubstitutionRule(neighbors[2].substitution), spectral.beta);

    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
    const auto pre_contact = backward_closure<3>(subst, d_cont);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
    const auto trace = algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        8, 0, CoronaEdgeArithmetic::exact_rational);
    const auto& pre_red = trace.layers[1].pre_red_nodes;

    std::vector<BothFixedAffineInstance> result;
    for (const auto& node : pre_red) {
        for (const auto& pt : detail_both_fixed::both_fixed_types()) {
            if (pt.inner != node.i) continue;
            for (const auto& qt : detail_both_fixed::both_fixed_types()) {
                if (qt.inner != node.j) continue;

                const long long rhs2 = node.x[2] + qt.l2 - pt.l2;
                const long long slope_diff = qt.slope - pt.slope;
                const long long x2_slope = slope_diff - rhs2;
                const long long CONST = node.x[0]
                    + (qt.b - qt.l1 - qt.l2) - (pt.b - pt.l1 - pt.l2);
                const long long x0p = node.x[1] + qt.l1 - pt.l1;
                const long long x1p = rhs2 - x0p;

                if (x2_slope == 0) continue;

                for (const auto& T : pre_red) {
                    if (T.i != pt.parent || T.j != qt.parent) continue;
                    if (T.x[0] != x0p || T.x[1] != x1p) continue;
                    const long long numerator = T.x[2] - CONST;
                    if (numerator % x2_slope != 0) continue;
                    const long long a_required = numerator / x2_slope;
                    result.push_back({CONST, x2_slope, T.x[2], a_required});
                }
            }
        }
    }
    return result;
}

// Threads a representative STRIDE SAMPLE (not just the first few) of
// the CONCRETE instances into the trace -- only those with
// `a_required < 7`, matching `affine_no_solution_at_or_above_
// threshold`'s `hbelow` hypothesis (threshold=7). There are 323 such
// instances total; embedding all of them would make the generated
// Lean file unwieldy for no honesty gain (each is an independent,
// trivially-checked arithmetic fact), so this samples `count` of them
// spread evenly across the full list.
inline void stage_class_ii_both_fixed_affine_sample(std::size_t count = 20) {
    if (!mathlib::reflection::enabled()) return;
    const auto instances = class_ii_both_fixed_affine_instances();
    std::vector<BothFixedAffineInstance> below7;
    for (const auto& inst : instances)
        if (inst.a_required < 7) below7.push_back(inst);
    if (below7.empty()) return;
    const std::size_t stride = below7.size() / std::min(count, below7.size());
    for (std::size_t i = 0; i < below7.size(); i += std::max<std::size_t>(stride, 1)) {
        const auto& inst = below7[i];
        mathlib::reflection::BothFixedAffineCertificate node;
        node.const_ = inst.const_;
        node.slope = inst.slope;
        node.target = inst.target;
        node.a_required = inst.a_required;
        mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
    }
}

}  // namespace ravel::proof
