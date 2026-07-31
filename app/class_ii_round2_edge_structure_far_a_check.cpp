// Stress test for class_ii_round2_edge_structure_a_independence_probe.cpp's
// finding (identical forward-edge graph at a=6..15): if forward images
// really are affine in a (as the M(a) back-substitution mechanism
// implies generally, not just for Round 1's two states), then the
// graph found identical at even two values of a must be identical for
// literally every integer a -- so checking a value far outside the
// sampled cluster is a meaningful test of "genuinely affine/constant"
// versus "coincidence over a narrow sampled range."

#include <cstdio>
#include <map>
#include <set>
#include <vector>

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

Matrix incidence(const FiniteSubstitution& substitution) {
    Matrix matrix(3, std::vector<long long>(3, 0));
    for (std::size_t image = 0; image < 3; ++image) {
        for (const auto letter : substitution[image]) {
            ++matrix[static_cast<std::size_t>(letter)][image];
        }
    }
    return matrix;
}

std::pair<std::set<SNode<3>>, std::map<SNode<3>, std::set<SNode<3>>>>
edge_structure_at(long long a) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const Substitution<3> subst_n = make_substitution<3>(
        SubstitutionRule(neighbors[2].substitution), spectral.beta);

    const auto seeds = search_D_cont<3>(subst_n, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
    const auto pre_contact = backward_closure<3>(subst_n, d_cont);
    const auto induced = induced_restricted_edges<3>(subst_n, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
    const auto trace = algorithm2_trace<3>(
        subst_n, contact, CoronaConnectorPolicy::fixed_signed_contact,
        8, 0, CoronaEdgeArithmetic::exact_rational);

    const auto& layer = trace.layers[1];
    const auto& pre_red = layer.pre_red_nodes;

    std::map<SNode<3>, std::set<SNode<3>>> edges;
    for (const auto& n : pre_red) {
        std::set<SNode<3>> dests;
        for (const auto& [dest, pq] : simple_forward_targets_exact<3>(subst_n, n))
            if (pre_red.count(dest)) dests.insert(dest);
        edges[n] = dests;
    }
    return {pre_red, edges};
}

}  // namespace

int main() {
    const auto [base_states, base_edges] = edge_structure_at(6);
    const long long far_a = 50;
    const auto [far_states, far_edges] = edge_structure_at(far_a);

    const bool nodes_same = far_states == base_states;
    long long mismatches = 0;
    if (nodes_same) {
        for (const auto& n : far_states) {
            auto it_b = base_edges.find(n);
            auto it_f = far_edges.find(n);
            if (it_b == base_edges.end() || it_f == far_edges.end()
                || it_b->second != it_f->second) {
                ++mismatches;
            }
        }
    }

    std::printf(
        "a=6 vs a=%lld: nodes_same=%d edge_structure_mismatches=%lld "
        "(over %zu states)\n",
        far_a, nodes_same, nodes_same ? mismatches : -1, far_states.size());

    const bool ok = nodes_same && mismatches == 0;
    std::printf(
        "%s\n",
        ok ? "FAR_A_CHECK_CONFIRMS_AFFINE_HYPOTHESIS: the graph found "
             "identical at a=6..15 is still identical at a=50 -- "
             "strong evidence the forward-edge connectivity is "
             "genuinely a-independent (affine with slope 0), not a "
             "coincidence of small sampled a"
           : "FAR_A_CHECK_REFUTES_AFFINE_HYPOTHESIS -- structure "
             "differs at a=50, so agreement at a=6..15 was "
             "coincidental over a narrow range, not genuine "
             "a-independence");
    return ok ? 0 : 1;
}
