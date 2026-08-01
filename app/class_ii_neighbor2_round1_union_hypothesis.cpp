// PROVEN, universally, zero per-a computation needed (2026-08-01):
//
//   class_ii_neighbor2_signed_contact_set()
//     == class_ii_contact_set()
//        UNION mirror(class_ii_contact_set())
//        UNION class_ii_neighbor2_initial_extension_states()
//
// i.e. target == plus_minus_C(sigma_a) union E_1, EXACTLY as
// docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's own "Next
// implementation" section already stated as the goal. All three
// operands are parameter-free (no `a` argument anywhere in their
// signatures) -- class_ii_contact_set() is the already-proven
// a-independent 14-state contact catalogue
// (docs/CLASS_II_CONTACT_BASE_PROOF.md), and
// class_ii_neighbor2_initial_extension_states() /
// class_ii_neighbor2_signed_contact_set() are the fixed 22- and
// 50-state hardcoded catalogues this whole investigation treats as
// candidate targets. So this identity, once checked once, holds for
// EVERY integer a simultaneously -- there is no sweep to run, no
// outlier to stress-test, because none of the three operands depend
// on a to begin with. Verified below via `main()`'s single call.
//
// IMPORTANT SCOPE NOTE, not to be conflated with the above: this
// proves the TARGET's own internal decomposition (fact A: the three
// named catalogues really do union to exactly the claimed 50-state
// set). It does NOT by itself prove Round 1's actual open obligation
// (fact B: that this 50-state set is the CORRECT reverse-inclusion
// closure of tau_a's own D_cont seeds, for every a) --
// class_ii_neighbor2_first_missing_premise(a) correctly continues to
// report round 1 as open, and this file does not touch that. Fact B
// is checked numerically, per-a, by
// app/class_ii_neighbor2_round1_sweep.cpp (against the real corona
// trace) and remains the actual next step.
//
// How fact A was found: app/class_ii_neighbor2_symmetric_seed_closure.cpp's
// own breakdown data (a=3..6) showed that a THIRD, auxiliary object --
// the backward_closure/red_anode result seeded from
// D_cont(tau_a) union mirror(D_cont(tau_a)), 32 states -- already
// contains all 14 of class_ii_contact_set()'s states, plus exactly
// half of E_1 (11/22) and half of mirror(class_ii_contact_set())
// (7/14). 11+7=18 matched that app's own "missing" count exactly, and
// 32+18=50 matched the target's size -- a strong hint, checked here
// first as the literal SETS (not just the counts: two different
// 18-element sets could each make the arithmetic work), confirming a
// broader (33-state-larger, a-dependent) union identity across 64
// values of a from 3 to 1000 (zero exceptions, see
// `check_broader_identity_with_backward_closure` below) BEFORE the
// much simpler, fully a-independent identity above was tried directly
// and also found to hold exactly. The simpler identity is the one
// worth keeping as the actual result; the broader one is kept here as
// corroborating structural evidence, not because it's needed.

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

// The pure, a-independent identity: proven by one call, no sweep.
bool check_pure_identity() {
    const auto e1 = class_ii_neighbor2_initial_extension_states();
    const auto center_set = class_ii_contact_set();
    std::set<SNode<3>> mirror_center;
    for (const auto& s : center_set) mirror_center.insert(s.mirror());

    std::set<SNode<3>> u;
    for (const auto& s : e1) u.insert(s);
    for (const auto& s : center_set) u.insert(s);
    for (const auto& s : mirror_center) u.insert(s);

    const auto target = class_ii_neighbor2_signed_contact_set();

    std::vector<SNode<3>> extra, missing;
    for (const auto& s : u) if (!target.count(s)) extra.push_back(s);
    for (const auto& s : target) if (!u.count(s)) missing.push_back(s);

    const bool ok = extra.empty() && missing.empty();
    std::printf(
        "PURE_IDENTITY,e1=%zu,center=%zu,mirror_center=%zu,union=%zu,"
        "target=%zu,extra=%zu,missing=%zu,%s\n",
        e1.size(), center_set.size(), mirror_center.size(), u.size(),
        target.size(), extra.size(), missing.size(),
        ok ? "EXACT_MATCH" : "MISMATCH");
    for (const auto& s : extra)
        std::printf("  EXTRA,%lld,%lld,%lld,%lld,%lld\n", s.i, s.x[0], s.x[1], s.x[2], s.j);
    for (const auto& s : missing)
        std::printf("  MISSING,%lld,%lld,%lld,%lld,%lld\n", s.i, s.x[0], s.x[1], s.x[2], s.j);
    return ok;
}

// Corroborating structural evidence: the broader, a-dependent identity
// discovered first (see file header). Not needed for the pure identity
// above, but confirms the two are consistent (since center_set is
// fully contained in sym_survivors at every tested a below).

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

ANode<3> mirror_anode(const ANode<3>& n) {
    ANode<3> m;
    m.i = n.j;
    m.j = n.i;
    for (std::size_t k = 0; k < 3; ++k) m.x[k] = -n.x[k];
    return m;
}

SNode<3> to_snode(const ANode<3>& n) {
    SNode<3> s;
    s.i = n.i;
    s.j = n.j;
    s.x = n.x;
    return s;
}

bool check_broader_identity_with_backward_closure(long long a) {
    const auto center = class_ii(static_cast<std::size_t>(a));
    const auto spectral = classify_matrix_spectral(incidence(center));
    const auto neighbors = adjacent_swap_neighbors(center);
    const auto& tau = neighbors[2].substitution;
    const Substitution<3> subst =
        make_substitution<3>(SubstitutionRule(tau), spectral.beta);

    const auto seeds = search_D_cont<3>(subst, 2);
    std::vector<ANode<3>> d_cont;
    for (const auto& seed : seeds) {
        ANode<3> node;
        node.i = seed.i;
        node.j = seed.j;
        node.x = seed.x;
        d_cont.push_back(node);
    }

    std::vector<ANode<3>> sym_seed = d_cont;
    for (const auto& n : d_cont) sym_seed.push_back(mirror_anode(n));
    std::set<ANode<3>> sym_seed_set(sym_seed.begin(), sym_seed.end());
    std::vector<ANode<3>> sym_seed_vec(sym_seed_set.begin(), sym_seed_set.end());

    const auto pre_contact = backward_closure<3>(subst, sym_seed_vec);
    const auto induced = induced_restricted_edges<3>(subst, pre_contact);
    const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
    const auto reduced = red_anode<3>(pre_set, induced);

    std::set<SNode<3>> sym_survivors;
    for (const auto& n : reduced.first) sym_survivors.insert(to_snode(n));

    const auto e1 = class_ii_neighbor2_initial_extension_states();
    const auto center_set = class_ii_contact_set();
    std::set<SNode<3>> mirror_center;
    for (const auto& s : center_set) mirror_center.insert(s.mirror());

    std::set<SNode<3>> hypothesis;
    for (const auto& s : sym_survivors) hypothesis.insert(s);
    for (const auto& s : e1) hypothesis.insert(s);
    for (const auto& s : mirror_center) hypothesis.insert(s);

    const auto target = class_ii_neighbor2_signed_contact_set();
    const bool ok = (hypothesis == target);
    std::printf(
        "BROADER_IDENTITY,a=%lld,sym=%zu,union=%zu,target=%zu,%s\n",
        a, sym_survivors.size(), hypothesis.size(), target.size(),
        ok ? "EXACT_MATCH" : "MISMATCH");
    return ok;
}

}  // namespace

int main(int argc, char** argv) {
    // Default range kept modest (the pure identity above is the actual
    // result and needs zero per-a computation); pass explicit args for
    // a wider sweep. Already run and recorded once, exact match, at
    // a=3..60 dense plus 100/500/1000 outliers -- see
    // docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md.
    bool all_ok = check_pure_identity();

    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 3;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 15;
    for (long long a = a_min; a <= a_max; ++a) {
        all_ok = check_broader_identity_with_backward_closure(a) && all_ok;
    }
    all_ok = check_broader_identity_with_backward_closure(50) && all_ok;
    std::printf(
        "UNION_HYPOTHESIS_SUMMARY,a_min=%lld,a_max=%lld,plus_outliers,%s\n",
        a_min, a_max, all_ok ? "ALL_MATCH" : "SOME_MISMATCH");
    return all_ok ? 0 : 1;
}
