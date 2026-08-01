// The actual remaining gap in the global occurrence theorem, per the
// base-premises table: "connect the bounded round-four grammar to the
// universal stable reverse/exclusion theorem whose stated domain
// begins at round five."
//
// The stable machinery's own composition witnesses
// (class_ii_neighbor2_stable_pre_red_composition_witnesses(a, round),
// include/ravel/class_ii_neighbor2_pruning.hpp) compute their SOURCES
// at round=5 using the *generic* formula
//   class_ii_center_layer_candidate(a, 4)
//     union class_ii_neighbor2_layer_extension(a, 4)
// -- the same generic formula used for every round >= 3/2 respectively,
// not a round-4-specific hardcoded catalogue. THEOREM_STATUS.md
// already records the stable raw-corona composition certificate as
// "universal for a>=7" using exactly this generic source formula.
//
// Round 4's own base-round closure this session (class_ii_round4_
// structure.cpp) instead computed A_4 -- the round-4 post-Red survivor
// set -- via the independent, trusted ground-truth pipeline
// (search_D_cont -> backward_closure -> corona iteration -> red), and
// found T4 = B4 union E4 (113 = 88 + 25) for a>=7.
//
// These are two independently-arrived-at objects. If they are the
// SAME set, node for node, then the already-proven stable composition
// certificate (which only needs its assumed round-4 source shape to
// be correct) applies directly to the real round 4, closing the
// bridge -- no new machinery needed, just this one equality. This
// file checks that equality directly, across a wide a range.
//
// A further point, checked separately and by direct code-path
// inspection rather than sampling: the "predicted" side is *provably*
// a-independent for every a>=6, not just observed to be constant.
// class_ii_center_layer_candidate(a, round=4) takes
// last_interior = (round<a ? round : a-1); since round=4<a for any
// a>=5, this is always literally 4, so only class_ii_interior_shell(4)
// (parametrized by shell/round index, never by the substitution
// parameter) is added to the fixed class_ii_stable_base(). Likewise
// class_ii_neighbor2_layer_extension(a, round=4) takes the
// round<a-1 branch for any a>=6, returning the fixed
// class_ii_neighbor2_interior_extension_states(4) -- again no
// a-dependence. So the predicted round-4 catalogue is one specific
// fixed 113-state set for literally every a in this investigation's
// domain, checked identical node-for-node at a in
// {7,8,9,10,20,50,200,1000} directly (not merely same cardinality).
//
// Round 4's own closure this session (the closed-form Red-exclusion
// argument, class_ii_both_fixed_full_proof.cpp /
// class_ii_round234_shape_closure.lean) already shows every
// slope-nonzero raw-candidate edge in T_4's shape categories requires
// a<7 to exist, so for a>=7 the only edges present among the
// (definitionally a-independent) raw pool T_4=B_4union E_4 are the
// slope-zero ones -- themselves trivially a-independent. A fixed node
// set with a fixed edge set gives a fixed Red-pruning result, so A_4
// (the ground-truth side computed here) is therefore ALSO provably
// a-independent for every a>=7, not merely checked at the sampled
// values below. Given that, checking equality with the (separately
// provably-constant) predicted side at even one a>=7 would suffice;
// checking many is redundant confirmation, not the load-bearing
// evidence.

#include <cstdio>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor2_pruning.hpp"
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

std::set<SNode<3>> ground_truth_round4_survivors(
        const Substitution<3>& subst_n) {
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
    // layers[0] = round 1, so layers[3] = round 4.
    return trace.layers.at(3).nodes;
}

void run(long long a) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const Substitution<3> subst_n = make_substitution<3>(
        SubstitutionRule(neighbors[2].substitution), spectral.beta);

    const auto ground_truth = ground_truth_round4_survivors(subst_n);

    const auto predicted =
        [&]() {
            auto result = class_ii_center_layer_candidate(
                static_cast<std::size_t>(a), 4);
            const auto ext = class_ii_neighbor2_layer_extension(a, 4);
            result.insert(ext.begin(), ext.end());
            return result;
        }();

    std::vector<SNode<3>> extra, missing;
    for (const auto& s : predicted)
        if (!ground_truth.count(s)) extra.push_back(s);
    for (const auto& s : ground_truth)
        if (!predicted.count(s)) missing.push_back(s);

    std::printf(
        "BRIDGE,a=%lld,ground_truth=%zu,predicted=%zu,extra=%zu,"
        "missing=%zu,%s\n",
        a, ground_truth.size(), predicted.size(), extra.size(),
        missing.size(),
        (extra.empty() && missing.empty()) ? "EXACT_MATCH" : "MISMATCH");

    for (const auto& s : extra)
        std::printf("  EXTRA,a=%lld,%lld,%lld,%lld,%lld,%lld\n", a, s.i,
                    s.x[0], s.x[1], s.x[2], s.j);
    for (const auto& s : missing)
        std::printf("  MISSING,a=%lld,%lld,%lld,%lld,%lld,%lld\n", a, s.i,
                    s.x[0], s.x[1], s.x[2], s.j);
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 7;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 10;
    for (long long a = a_min; a <= a_max; ++a) run(a);
    return 0;
}
