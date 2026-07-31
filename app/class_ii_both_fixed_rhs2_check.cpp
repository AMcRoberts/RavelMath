// Tests the natural guess for extending the hybrid category's proof
// (class_ii_hybrid_window_slope_derivation.cpp) to the both-fixed
// category: that rhs[2]=0 exactly for every SURVIVING both-fixed
// edge, with any rhs[2]!=0 edge eventually and permanently excluded
// once its unboundedly-growing x2' leaves the bounded target window
// (Round 1's original escaping-candidate mechanism).
//
// Checked directly at a=7, across every both-fixed raw candidate
// (valid or not) for every state in Round 2's pre_red set: **11 of
// 408 raw candidates have rhs[2]!=0 and ARE currently valid, landing
// in pre_red**. This contradicts the simple guess -- it is not true
// that only rhs[2]=0 edges survive.
//
// This does NOT mean the whole-graph a-independence claim is wrong
// (that remains exact-finite-checked through a=50, unaffected). It
// means an rhs[2]!=0 edge's destination can land on a DIFFERENT
// member of the same a-independent pre_red set as a varies, so
// per-edge non-constancy doesn't automatically break per-state
// destination-SET constancy -- the both-fixed category's mechanism is
// subtler than the hybrid category's clean two-case slope split, and
// its proof (as opposed to its exact finite check) remains open.

#include <cstdio>
#include <set>
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

bool rule_range(long long inner, long long parent, long long l1, long long l2) {
    if (inner != 0) return false;
    if (parent == 2) return false;
    if (l1 != 0 || l2 != 0) return false;
    return true;
}

}  // namespace

int main() {
    const long long a = 7;
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

    long long nonzero_total = 0, nonzero_valid_in_pre_red = 0;
    long long zero_total = 0, zero_valid_in_pre_red = 0;

    for (const auto& node : pre_red) {
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);
        for (const auto& pd : parents_i) {
            auto lp1 = abelianization<3>(pd.p);
            if (rule_range(node.i, pd.parent_letter, lp1[1], lp1[2])) continue;
            for (const auto& qd : parents_j) {
                auto lq1 = abelianization<3>(qd.p);
                if (rule_range(node.j, qd.parent_letter, lq1[1], lq1[2]))
                    continue;
                const long long rhs2 = node.x[2] + lq1[2] - lp1[2];
                std::array<long long, 3> rhs;
                for (int k = 0; k < 3; ++k)
                    rhs[k] = node.x[k] + lq1[k] - lp1[k];
                auto xprime_opt = solve_Mx_eq_rhs_exact<3>(subst.M, rhs);
                bool ok = false;
                if (xprime_opt.has_value()) {
                    SNode<3> cand{pd.parent_letter, *xprime_opt,
                                   qd.parent_letter};
                    ok = is_valid_simple_node<3>(subst, cand)
                         && pre_red.count(cand) > 0;
                }
                if (rhs2 != 0) {
                    ++nonzero_total;
                    if (ok) ++nonzero_valid_in_pre_red;
                } else {
                    ++zero_total;
                    if (ok) ++zero_valid_in_pre_red;
                }
            }
        }
    }
    std::printf("a=%lld rhs2!=0: total=%lld valid_in_pre_red=%lld\n", a,
                nonzero_total, nonzero_valid_in_pre_red);
    std::printf("a=%lld rhs2==0: total=%lld valid_in_pre_red=%lld\n", a,
                zero_total, zero_valid_in_pre_red);
    std::printf(
        "%s\n",
        nonzero_valid_in_pre_red > 0
            ? "SIMPLE_GUESS_FALSE: rhs2!=0 edges do currently survive "
              "as valid, in-pre_red edges -- the both-fixed category "
              "needs a subtler argument than the hybrid category's, "
              "not assumed to follow automatically"
            : "no rhs2!=0 survivors found at this a (would support "
              "the simple guess, but check more a before trusting it)");
    return 0;
}
