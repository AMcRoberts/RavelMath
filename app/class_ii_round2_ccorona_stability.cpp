// Strengthens the "unverified lead" note in
// docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's Round 2 section from
// "the survivor sets look identical across a" to "the actual c_corona
// composition, called directly (not reimplemented), produces the
// identical raw candidate count across a" -- still not a proof, but a
// more specific, more actionable starting point.
//
// tau_a's own round-1 layer (the signed contact set, 50 states) and
// its own +/-C connector set are literally the same 50-state object
// (confirmed: T1 == pmC every time below), and both are a-independent
// by construction -- so c_corona(T1, pmC)'s raw vector-addition step
// never touches `a` at all. The only place `a` (via beta) can enter is
// the same_letter_H window-validity gate applied during composition.
// Calling the real, trusted c_corona() directly (not a reimplemented
// approximation) at a = 6, 7, 8, 15 gives the identical raw candidate
// count (195) every time.
//
// What this does NOT establish: that the window-validity decision for
// each of the (at most 50*50=2500) candidate pairs is provably stable
// for literally every a>=3 (or whatever the right domain is) -- that
// would need the same AffineCD-style corner-bound argument Round 1's
// window certificate used, applied to round 2's specific fixed
// candidates instead of Round 1's affine-in-a ones. Structurally
// plausible (each candidate is a single fixed point rather than a
// growing range, which is if anything an easier case than Round 1's),
// but not attempted here -- flagged as the concrete next step, not
// claimed as done.

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
        const Substitution<3>& subst) {
    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& s : seeds) d_cont.push_back(ANode<3>{s.i, s.x, s.j});
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
    long long raw_count = -1;
    long long mismatches = 0;

    for (long long a : {6LL, 7LL, 8LL, 15LL}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const auto trace = corona_trace(subst);
        const auto& T1 = trace.layers[0].nodes;
        const auto& pmC = trace.signed_contact;
        const bool t1_equals_pmc = T1 == pmC;

        const auto raw2 = c_corona<3>(subst, T1, pmC);

        std::printf(
            "a=%lld T1_size=%zu pmC_size=%zu T1==pmC=%s "
            "raw_c_corona_size=%zu\n",
            a, T1.size(), pmC.size(), t1_equals_pmc ? "yes" : "no",
            raw2.size());

        if (!t1_equals_pmc) ++mismatches;
        if (raw_count == -1) {
            raw_count = static_cast<long long>(raw2.size());
        } else if (raw_count != static_cast<long long>(raw2.size())) {
            ++mismatches;
        }
    }

    const bool ok = mismatches == 0;
    std::printf(
        "%s\n",
        ok ? "ROUND2_CCORONA_STABILITY_CONFIRMED: the real c_corona(T1,pmC) "
             "call gives an identical raw candidate count across every "
             "tested a -- a concrete, checked starting point for Round "
             "2's reverse-inclusion gap, not yet a proof of it"
           : "INSTABILITY DETECTED -- the lead this file was meant to "
             "strengthen does not hold as stated, see output above");
    return ok ? 0 : 1;
}
