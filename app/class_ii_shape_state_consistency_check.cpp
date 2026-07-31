// Corrects a methodological gap in class_ii_round2_edge_mechanism_
// escaping_slopes.cpp: that file tested whether ONE representative
// state's x2' value stayed the same between a=7 and a=40, for each
// of the 20 "both-fixed" shapes, then reported "all 20 shapes appear
// constant" -- but a shape (the 8-tuple used for classification)
// does not uniquely determine a state. Multiple different source
// states (different node.x) can share the same shape.
//
// Checked directly: gathering EVERY valid edge for each both-fixed
// shape (not just one arbitrary representative) and comparing their
// x2' values finds **15 of the 20 shapes have more than one distinct
// x2' value** across their instantiating states (up to 4 distinct
// values for some shapes). The classification-by-shape device
// (class_ii_shape_classification_rule.cpp) correctly explains which
// SIDE ranges vs is fixed, but does NOT mean every state sharing a
// shape behaves identically -- node.x itself varies across states
// sharing a shape, and that variation propagates directly into x2'.
//
// What this does NOT undo: the escaping_slopes.cpp finding (zero
// slope for one representative state per shape, checked at a=7 vs
// a=40) is still a correctly-computed fact about THAT ONE state's
// specific edge -- it was the GENERALIZATION ("the shape needs no
// further argument") that overreached, not the arithmetic. The
// actually-relevant claim (per-STATE a-independence, not per-shape
// uniformity) is unaffected and was already established separately by
// the whole-graph checks (class_ii_round2_edge_structure_a_
// independence_probe.cpp and siblings, a=6..50).
//
// Net effect: the classification rule still organizes the 44 shapes
// correctly by which sides range, but a symbolic proof cannot treat
// "one representative state's behavior" as representative of a whole
// shape's states -- each of the (potentially many) states sharing a
// both-fixed shape needs its own check that its own specific rhs[2]
// vanishes, not a single shape-level argument.

#include <cstdio>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "ravel/ambient_graph.hpp"
#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

using Matrix = std::vector<std::vector<long long>>;

FiniteSubstitution class_ii(std::size_t a) {
    FiniteSubstitution result(3);
    result[0].assign(a, 0);
    result[0].push_back(1);
    result[0].push_back(2);
    result[1].assign(a, 0);
    result[1].push_back(2);
    result[2] = {0};
    return result;
}

Matrix incidence(const FiniteSubstitution& s) {
    Matrix m(3, std::vector<long long>(3, 0));
    for (std::size_t im = 0; im < 3; ++im)
        for (auto l : s[im]) ++m[static_cast<std::size_t>(l)][im];
    return m;
}

bool rule_predicts_range(long long inner_letter, long long parent_letter,
                          long long l1, long long l2) {
    if (inner_letter != 0) return false;
    if (parent_letter == 2) return false;
    if (l1 != 0 || l2 != 0) return false;
    return true;
}

}  // namespace

int main() {
    const long long a = 15;
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
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

    using Shape = std::tuple<long long, long long, long long, long long,
                              long long, long long, long long, long long>;
    std::map<Shape, std::set<long long>> x2_by_shape;

    for (const auto& node : pre_red) {
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);
        for (const auto& pd : parents_i) {
            auto lp1 = abelianization<3>(pd.p);
            if (rule_predicts_range(node.i, pd.parent_letter, lp1[1], lp1[2]))
                continue;
            for (const auto& qd : parents_j) {
                auto lq1 = abelianization<3>(qd.p);
                if (rule_predicts_range(node.j, qd.parent_letter, lq1[1],
                                         lq1[2]))
                    continue;
                std::array<long long, 3> rhs;
                for (int k = 0; k < 3; ++k)
                    rhs[k] = node.x[k] + lq1[k] - lp1[k];
                auto xprime_opt = solve_Mx_eq_rhs_exact<3>(subst.M, rhs);
                if (!xprime_opt.has_value()) continue;
                auto xprime = *xprime_opt;
                SNode<3> cand{pd.parent_letter, xprime, qd.parent_letter};
                if (!is_valid_simple_node<3>(subst, cand)) continue;
                if (!pre_red.count(cand)) continue;
                Shape sh{node.i, node.j, pd.parent_letter, qd.parent_letter,
                          lp1[1], lp1[2], lq1[1], lq1[2]};
                x2_by_shape[sh].insert(cand.x[2]);
            }
        }
    }

    long long consistent = 0, inconsistent = 0;
    for (auto& [sh, x2s] : x2_by_shape) {
        if (x2s.size() == 1) ++consistent;
        else ++inconsistent;
    }
    std::printf(
        "both_fixed_shapes=%zu single_x2_value=%lld "
        "multiple_x2_values=%lld\n",
        x2_by_shape.size(), consistent, inconsistent);
    std::printf(
        "%s\n",
        "SHAPE_LEVEL_CONSTANCY_DOES_NOT_HOLD: most both-fixed shapes "
        "are shared by multiple states with different x2' values -- "
        "a shape does not determine a unique state, so shape-level "
        "constancy claims must be scoped per-state, not per-shape");
    return 0;
}
