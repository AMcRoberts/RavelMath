// Sharper recon than class_ii_round2_pruned_states_a_independence_probe.cpp:
// that file showed the pruned-state SET is a-independent. This checks
// whether the forward-edge GRAPH STRUCTURE among the 256 raw states
// (which state's simple_forward_targets_exact output includes which
// other states, as a set of destinations -- not the exact rational
// edge weights, just connectivity) is ALSO a-independent. If so, the
// still-open symbolic Red-exclusion proof reduces to checking one
// fixed, a-independent graph once (a finite, decidable fact), rather
// than needing an affine-in-a argument per state the way Round 1's
// two-state case did -- a much more tractable path to closing rounds
// 2/3/4 than a from-scratch M-matrix derivation per state.

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

// Returns: (pre_red raw state set, map from each state to its set of
// forward-destination states that land within pre_red).
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

    const auto& layer = trace.layers[2];
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
    const std::vector<long long> as = {6, 7, 8, 9, 10, 12, 15};
    const auto [baseline_states, baseline_edges] = edge_structure_at(as.front());

    bool nodes_all_same = true;
    bool edges_all_same = true;
    long long total_mismatched_states = 0;

    for (long long a : as) {
        const auto [states, edges] = edge_structure_at(a);
        const bool nodes_same = states == baseline_states;
        nodes_all_same = nodes_all_same && nodes_same;

        long long mismatches = 0;
        if (nodes_same) {
            for (const auto& n : states) {
                auto it_b = baseline_edges.find(n);
                auto it_a = edges.find(n);
                if (it_b == baseline_edges.end() || it_a == edges.end()
                    || it_b->second != it_a->second) {
                    ++mismatches;
                }
            }
        } else {
            mismatches = -1;  // nodes differ, comparison not meaningful
        }
        edges_all_same = edges_all_same && (mismatches == 0);
        total_mismatched_states += (mismatches > 0 ? mismatches : 0);

        std::printf(
            "a=%lld nodes_same=%d edge_structure_mismatches=%lld "
            "(over %zu states)\n",
            a, nodes_same, mismatches, states.size());
    }

    std::printf("total_mismatched_states_across_all_a=%lld\n",
                total_mismatched_states);
    std::printf(
        "%s\n",
        edges_all_same
            ? "ROUND3_EDGE_STRUCTURE_IS_A_INDEPENDENT: the entire "
              "forward-edge connectivity graph over the 256 raw "
              "states (not just the pruned node set) is the literal "
              "same across a=6,7,8,9,10,12,15 -- the symbolic proof "
              "reduces to checking one fixed graph, not an affine "
              "argument per state"
            : "EDGE STRUCTURE VARIES WITH a -- the connectivity "
              "itself depends on a, not just the edge weights; the "
              "symbolic proof needs the harder per-state affine "
              "argument after all");
    return 0;
}
