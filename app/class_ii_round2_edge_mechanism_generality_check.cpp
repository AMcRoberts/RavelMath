// Tests whether the mechanism found on one representative Round 4
// state (class_ii_round4_edge_mechanism_probe.cpp: x0'/x1' constant
// across all surviving (p_k,q_k) pairs, x2' a function of q_k-p_k
// alone) generalizes across Round 2's 195 raw states. It does NOT --
// this file exists to record that negative result plainly rather than
// let the earlier "plausible generally" language stand unchecked.
//
// Result at a=8: of the 73 raw states with more than one surviving
// edge, only 17 have x0'/x1' constant across every edge, and 65 have
// x2' a function of q_k-p_k alone (weaker but still not universal).
// The one-parameter model from the representative state is a special
// case, not the general mechanism -- most states plausibly mix
// contributions from multiple underlying letters (0, 1, 2) with
// different occurrence-count behavior (letter 0's count scales with
// a per tau_a's fixed word forms; letters 1/2 do not, per Round 1's
// own finding), giving more than one effectively-free parameter per
// state rather than the single q_k-p_k difference seen in the
// representative case.

#include <cstdio>
#include <map>
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

}  // namespace

int main() {
    const long long a = 8;
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

    long long states_with_multi_edges = 0;
    long long states_with_constant_x0x1 = 0;
    long long states_with_x2_fn_of_diff_only = 0;

    struct Rec {
        long long p_len, q_len;
        SNode<3> dest;
    };

    for (const auto& node : pre_red) {
        auto parents_i = parent_decompositions<3>(subst.images, node.i);
        auto parents_j = parent_decompositions<3>(subst.images, node.j);

        std::vector<Rec> recs;
        for (const auto& pd : parents_i) {
            auto lp1 = abelianization<3>(pd.p);
            for (const auto& qd : parents_j) {
                auto lq1 = abelianization<3>(qd.p);
                std::array<long long, 3> rhs;
                for (int k = 0; k < 3; ++k)
                    rhs[k] = node.x[k] + lq1[k] - lp1[k];
                auto xprime_opt = solve_Mx_eq_rhs_exact<3>(subst.M, rhs);
                if (!xprime_opt.has_value()) continue;
                auto xprime = *xprime_opt;
                SNode<3> cand{pd.parent_letter, xprime, qd.parent_letter};
                if (!is_valid_simple_node<3>(subst, cand)) continue;
                if (!pre_red.count(cand)) continue;
                recs.push_back(
                    {static_cast<long long>(pd.p.size()),
                     static_cast<long long>(qd.p.size()), cand});
            }
        }
        if (recs.size() < 2) continue;
        ++states_with_multi_edges;

        bool x0x1_constant = true;
        for (auto& r : recs)
            if (r.dest.x[0] != recs[0].dest.x[0]
                || r.dest.x[1] != recs[0].dest.x[1]) {
                x0x1_constant = false;
                break;
            }
        if (x0x1_constant) ++states_with_constant_x0x1;

        std::map<long long, std::set<long long>> diff_to_x2;
        for (auto& r : recs) diff_to_x2[r.q_len - r.p_len].insert(r.dest.x[2]);
        bool x2_fn_of_diff = true;
        for (auto& [diff, x2s] : diff_to_x2)
            if (x2s.size() > 1) {
                x2_fn_of_diff = false;
                break;
            }
        if (x2_fn_of_diff) ++states_with_x2_fn_of_diff_only;
    }

    std::printf(
        "a=%lld states_with_multi_edges=%lld "
        "x0x1_constant=%lld/%lld x2_fn_of_diff_only=%lld/%lld\n",
        a, states_with_multi_edges, states_with_constant_x0x1,
        states_with_multi_edges, states_with_x2_fn_of_diff_only,
        states_with_multi_edges);
    std::printf(
        "MECHANISM_DOES_NOT_GENERALIZE_NAIVELY: the representative "
        "state's one-parameter model (constant x0'/x1', x2' a "
        "function of q_k-p_k alone) is a special case -- most "
        "multi-edge states have neither property universally, "
        "consistent with a mix of letter-0 (a-scaling) and letter-1/2 "
        "(fixed) occurrence contributions rather than a single free "
        "parameter\n");
    return 0;
}
