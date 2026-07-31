// Ground-truth dump of the raw pre-contact (post backward_closure,
// pre-Red) and post-Red contact ANode/SNode sets for tau_a, using
// exactly the same trusted functions app/class_ii_neighbor_probe.cpp's
// corona_trace already calls successfully. No reimplementation of
// backward_closure/red_anode semantics -- just calling them directly
// and printing what they actually produce, to calibrate against rather
// than guess from reading the source a third time.

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

void dump(
        const char* label, long long a,
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

    std::printf(
        "DUMP,%s,%lld,d_cont=%zu,pre_contact=%zu,contact_survivors=%zu\n",
        label, a, d_cont.size(), pre_set.size(), reduced.first.size());
    for (const auto& node : pre_set) {
        const bool survives = reduced.first.count(node) > 0;
        std::printf(
            "  PRE,%s,%lld,%lld,%lld,%lld,%lld,%lld,%s\n",
            label, a, node.i, node.x[0], node.x[1], node.x[2], node.j,
            survives ? "SURVIVES" : "PRUNED");
    }
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 3;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 4;
    for (long long a = a_min; a <= a_max; ++a) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        dump("CENTER", a, center, spectral.beta);
        const auto neighbors = adjacent_swap_neighbors(center);
        dump("NEIGHBOR2", a, neighbors[2].substitution, spectral.beta);
    }
    return 0;
}
