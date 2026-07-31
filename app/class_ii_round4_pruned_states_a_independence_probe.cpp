// The Round 2/3 pruned-state a-independence probe applied to Round 4,
// completing the check across all four base rounds: does Round 4
// share the "fixed pruned-state set, a-varying forward weights"
// pattern (Rounds 2/3), or is it an outlier the way Round 1 is
// (pruned coordinates affine in a)? Checked, not assumed.

#include <cstdio>
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

std::set<SNode<3>> pruned_states_at(long long a) {
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

    const auto& layer = trace.layers[3];
    std::set<SNode<3>> pruned;
    for (const auto& rank : layer.red_pruning_ranks)
        for (const auto& n : rank) pruned.insert(n);
    return pruned;
}

}  // namespace

int main() {
    const std::vector<long long> as = {6, 7, 8, 9, 10, 12, 15, 20, 30};
    const auto baseline = pruned_states_at(as.front());
    bool all_identical = true;
    long long max_abs_coord = 0;
    for (const auto& n : baseline) {
        for (int k = 0; k < 3; ++k) {
            long long v = n.x[k] < 0 ? -n.x[k] : n.x[k];
            if (v > max_abs_coord) max_abs_coord = v;
        }
    }

    for (long long a : as) {
        const auto pruned = pruned_states_at(a);
        const bool same = pruned == baseline;
        all_identical = all_identical && same;
        std::printf("a=%lld pruned_count=%zu identical_to_baseline=%d\n",
                    a, pruned.size(), same);
    }

    std::printf("baseline_max_abs_coord=%lld\n", max_abs_coord);
    std::printf(
        "%s\n",
        all_identical
            ? "ROUND4_PRUNED_STATE_SET_IS_A_INDEPENDENT: the pruned "
              "states are the literal same coordinate set for every "
              "tested a in {6,7,8,9,10,12,15,20,30}, matching Rounds "
              "2/3 -- all three of Rounds 2/3/4 share this pattern; "
              "Round 1 is the outlier (pruned coordinates affine in a)"
            : "PRUNED STATE SET VARIES WITH a -- recon hypothesis "
              "false, see per-a output above");
    return 0;
}
