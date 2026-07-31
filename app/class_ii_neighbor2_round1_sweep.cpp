// Extended numerical check for Round 1 of the four base transitions
// (docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md). Reuses exactly the
// same generic, substitution-agnostic pipeline app/class_ii_neighbor_probe.cpp
// already trusts for a=2..8 (adjacent_swap_neighbors + corona_trace),
// swept over a much larger range, to de-risk the symbolic reverse-inclusion
// derivation before it is attempted. Not itself a proof.

#include <cstdio>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/class_ii_neighbor2_pruning.hpp"
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

CoronaTrace<3> corona_trace(
        const FiniteSubstitution& substitution, double beta) {
    const Substitution<3> subst =
        make_substitution<3>(SubstitutionRule(substitution), beta);
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
    std::set<SNode<3>> contact;
    for (const auto& node : reduced.first) contact.insert(to_simple<3>(node));
    return algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        50, 0, CoronaEdgeArithmetic::exact_rational);
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_max = argc > 1 ? std::atoll(argv[1]) : 30;
    bool all_match = true;
    for (long long a = 3; a <= a_max; ++a) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        if (neighbors.size() != 3) {
            std::printf("NEIGHBOR_COUNT_WRONG,%lld,%zu\n", a, neighbors.size());
            all_match = false;
            continue;
        }
        const auto neighbor_trace =
            corona_trace(neighbors[2].substitution, spectral.beta);
        const bool matches =
            neighbor_trace.signed_contact
                == class_ii_neighbor2_signed_contact_set();
        std::printf(
            "ROUND1_SIGNED_CONTACT,%lld,%s,%zu,%zu\n",
            a, matches ? "MATCH" : "MISMATCH",
            neighbor_trace.signed_contact.size(),
            class_ii_neighbor2_signed_contact_set().size());
        if (!matches) {
            all_match = false;
            std::set<SNode<3>> extra, missing;
            std::set_difference(
                neighbor_trace.signed_contact.begin(),
                neighbor_trace.signed_contact.end(),
                class_ii_neighbor2_signed_contact_set().begin(),
                class_ii_neighbor2_signed_contact_set().end(),
                std::inserter(extra, extra.end()));
            std::set_difference(
                class_ii_neighbor2_signed_contact_set().begin(),
                class_ii_neighbor2_signed_contact_set().end(),
                neighbor_trace.signed_contact.begin(),
                neighbor_trace.signed_contact.end(),
                std::inserter(missing, missing.end()));
            for (const auto& node : extra) {
                std::printf(
                    "  EXTRA_IN_TRUE_SET,%lld,%lld,%lld,%lld,%lld,%lld\n",
                    a, node.i, node.x[0], node.x[1], node.x[2], node.j);
            }
            for (const auto& node : missing) {
                std::printf(
                    "  MISSING_FROM_TRUE_SET,%lld,%lld,%lld,%lld,%lld,%lld\n",
                    a, node.i, node.x[0], node.x[1], node.x[2], node.j);
            }
        }
    }
    std::printf(
        "ROUND1_SWEEP_SUMMARY,%lld,%s\n", a_max,
        all_match ? "ALL_MATCH" : "SOME_MISMATCH");
    return all_match ? 0 : 1;
}
