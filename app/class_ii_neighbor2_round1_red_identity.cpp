// Identifies, by direct comparison against the trusted ground-truth
// pipeline (search_D_cont -> backward_closure -> red_anode, the same
// functions app/class_ii_neighbor2_dump_backward.cpp already calls),
// exactly which two of the neighbor's 27 raw pre-Red states rank-one
// Red removes, and checks whether they coincide with the two D_cont
// seeds that class_ii_neighbor2_round1_window_certificate.cpp already
// showed need no backward predecessor.
//
// This is NOT a symbolic proof. It is an exact finite check, repeated
// over a wide concrete range, of a claim that was previously open
// (see docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's Round-1
// sections): whether "the 2 states absent from backward self-closure"
// and "the 2 states Red prunes" are the same pair or two different
// pairs. Answer, checked at every a in [3,60]: the same pair, every
// time. Symbolically proving *why* Red prunes exactly these two for
// every integer a (i.e. that their forward images never land back in
// the 27-state set) remains open and is not attempted here.

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

// The two D_cont seeds that class_ii_neighbor2_round1_window_certificate
// showed abstractly (for every a) need no backward predecessor in the
// 27-state raw self-closure.
std::set<SNode<3>> claimed_pruned_pair() {
    return {{1, {0, 0, 1}, 1}, {2, {0, 1, -1}, 1}};
}

}  // namespace

int main() {
    const auto claimed = claimed_pruned_pair();
    long long mismatches = 0;

    for (long long a = 3; a <= 60; ++a) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const auto seeds = search_D_cont<3>(subst, 2);
        std::vector<ANode<3>> d_cont;
        for (const auto& seed : seeds) {
            ANode<3> node;
            node.i = seed.i;
            node.j = seed.j;
            node.x = seed.x;
            d_cont.push_back(node);
        }
        const auto pre_contact = backward_closure<3>(subst, d_cont);
        const auto induced = induced_restricted_edges<3>(subst, pre_contact);
        const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
        const auto reduced = red_anode<3>(pre_set, induced);

        std::set<SNode<3>> pruned;
        for (const auto& node : pre_set) {
            if (reduced.first.count(node) == 0) {
                pruned.insert(to_simple<3>(node));
            }
        }

        const bool matches = pruned.size() == claimed.size()
            && pruned == claimed;
        if (!matches) {
            ++mismatches;
            std::printf(
                "MISMATCH a=%lld pre_contact=%zu survivors=%zu "
                "pruned_count=%zu (expected %zu)\n",
                a, pre_set.size(), reduced.first.size(), pruned.size(),
                claimed.size());
            for (const auto& n : pruned)
                std::printf("  actual-pruned i=%lld x=(%lld,%lld,%lld) j=%lld\n",
                    n.i, n.x[0], n.x[1], n.x[2], n.j);
        }
    }

    std::printf(
        "ROUND1_RED_IDENTITY a=3..60 mismatches=%lld -> %s\n",
        mismatches,
        mismatches == 0
            ? "the 2 Red-pruned states equal the 2 D_cont seeds "
              "shown to need no predecessor, at every tested a"
            : "CLAIM FALSIFIED for at least one tested a");
    return mismatches == 0 ? 0 : 1;
}
