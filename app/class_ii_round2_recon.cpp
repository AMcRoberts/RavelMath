// Round 2 reconnaissance only (see docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's
// "Round 2: reconnaissance only, not attempted" note) -- confirms the
// concrete per-round raw/survivor counts for tau_a against sigma_a
// using the already-trusted algorithm2_trace pipeline (the same one
// app/class_ii_neighbor_probe.cpp's corona_trace calls), so a future
// session attempting Round 2 starts from checked numbers rather than
// re-deriving them.
//
// Confirms: at round 2, center (sigma_a) survivors = 47, neighbor
// (tau_a) survivors = 72, difference = 25 -- matching
// docs/CLASS_II_NEIGHBOR2_TRANSPORT_THEOREM.md's "E_2 is a fixed
// 25-state correction" exactly, for a = 7, 8 (the a>=7 domain the rest
// of the global round partition uses). a=5 gives a slightly different
// round-4 count (114 vs 113), consistent with a=5 being outside that
// domain -- a small-parameter case, not a bug in this recon.
//
// This is NOT an attempt at Round 2's reverse-inclusion or Red
// exclusion. It only confirms the target numbers are what the docs
// say, using existing trusted machinery, not new derivation.

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

CoronaTrace<3> corona_trace(
        const FiniteSubstitution& substitution, double beta) {
    const Substitution<3> subst = make_substitution<3>(
        SubstitutionRule(substitution), beta);
    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) {
        ANode<3> n{s.i, s.x, s.j};
        d_cont.push_back(n);
    }
    const auto pre_contact = backward_closure<3>(subst, d_cont);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);
    std::set<SNode<3>> contact;
    for (const auto& n : reduced.first) contact.insert(to_simple<3>(n));
    return algorithm2_trace<3>(
        subst, contact, CoronaConnectorPolicy::fixed_signed_contact,
        8, 0, CoronaEdgeArithmetic::exact_rational);
}

}  // namespace

int main() {
    long long round2_diff_mismatches = 0;
    for (long long a : {7LL, 8LL}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);

        const auto center_trace = corona_trace(center, spectral.beta);
        const auto neighbor_trace =
            corona_trace(neighbors[2].substitution, spectral.beta);

        std::printf("a=%lld\n", a);
        for (std::size_t i = 0;
             i < std::min<std::size_t>(4, center_trace.layers.size()); ++i) {
            const auto& cl = center_trace.layers[i];
            const auto& nl = neighbor_trace.layers[i];
            const long long diff =
                static_cast<long long>(nl.nodes.size())
                - static_cast<long long>(cl.nodes.size());
            std::printf(
                "  round=%d center_survivors=%zu neighbor_survivors=%zu "
                "diff=%lld\n",
                cl.round, cl.nodes.size(), nl.nodes.size(), diff);
            if (cl.round == 2 && diff != 25) ++round2_diff_mismatches;
        }
    }
    std::printf(
        "%s\n",
        round2_diff_mismatches == 0
            ? "ROUND2_RECON: E_2's 25-state size confirmed against the "
              "trusted corona-trace pipeline at a=7,8 (not derived, "
              "not a reverse-inclusion or Red-exclusion proof)"
            : "ROUND2_RECON: round-2 difference did not match the "
              "documented 25 -- investigate before reusing this number");
    return round2_diff_mismatches == 0 ? 0 : 1;
}
