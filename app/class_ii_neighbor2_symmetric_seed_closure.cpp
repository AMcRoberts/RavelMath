// Round 1's residual question, made concrete: is
// class_ii_neighbor2_signed_contact_set() (the hardcoded 50-state ±C)
// the backward_closure/red_anode result of *some* symmetric seed, or
// is it only a definitional union (C ∪ mirror(C)) with no independent
// closure justification?
//
// Already checked and refuted (docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md,
// "Correction" note): seeding backward_closure/red_anode from
// mirror(D_cont) ALONE reproduces 32 survivors, not 25 -- so the
// unsigned closure does not simply "transport" under mirroring.
//
// Not yet tried: seeding backward_closure/red_anode from the SYMMETRIC
// union D_cont ∪ mirror(D_cont) directly (same trusted machinery,
// symmetric input this time) and comparing the *unsigned* result
// against class_ii_neighbor2_signed_contact_set() (50 states) itself,
// rather than against build_signed_contact_set() of anything. This is
// the concrete test this file runs.

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

void run(long long a) {
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

    // Symmetric seed: D_cont union mirror(D_cont), deduplicated.
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

    const auto target = class_ii_neighbor2_signed_contact_set();

    std::vector<SNode<3>> extra, missing;
    for (const auto& s : sym_survivors)
        if (!target.count(s)) extra.push_back(s);
    for (const auto& s : target)
        if (!sym_survivors.count(s)) missing.push_back(s);

    std::printf(
        "SYM,a=%lld,seed=%zu,pre=%zu,sym_survivors=%zu,target=%zu,"
        "extra=%zu,missing=%zu,%s\n",
        a, sym_seed_vec.size(), pre_set.size(), sym_survivors.size(),
        target.size(), extra.size(), missing.size(),
        (extra.empty() && missing.empty()) ? "EXACT_MATCH" : "MISMATCH");

    for (const auto& s : extra)
        std::printf("  EXTRA,a=%lld,%lld,%lld,%lld,%lld,%lld\n", a, s.i,
                    s.x[0], s.x[1], s.x[2], s.j);
    for (const auto& s : missing)
        std::printf("  MISSING,a=%lld,%lld,%lld,%lld,%lld,%lld\n", a, s.i,
                    s.x[0], s.x[1], s.x[2], s.j);

    // Is the symmetric-seed survivor set itself self-mirror-closed?
    bool self_mirror_closed = true;
    for (const auto& s : sym_survivors) {
        if (!sym_survivors.count(s.mirror())) { self_mirror_closed = false; break; }
    }

    // Compare against mirror(D_cont)-only closure (the already-known
    // 32-survivor result from the docs' earlier note) to see if this
    // symmetric-seed result is the SAME 32-set or a different one.
    std::vector<ANode<3>> mirror_only_seed;
    for (const auto& n : d_cont) mirror_only_seed.push_back(mirror_anode(n));
    const auto mo_pre_contact = backward_closure<3>(subst, mirror_only_seed);
    const auto mo_induced = induced_restricted_edges<3>(subst, mo_pre_contact);
    const std::set<ANode<3>> mo_pre_set(mo_pre_contact.begin(), mo_pre_contact.end());
    const auto mo_reduced = red_anode<3>(mo_pre_set, mo_induced);
    std::set<SNode<3>> mo_survivors;
    for (const auto& n : mo_reduced.first) mo_survivors.insert(to_snode(n));

    const bool same_as_mirror_only = (mo_survivors == sym_survivors);

    std::printf(
        "  SYM_SELF_MIRROR_CLOSED,a=%lld,%s SAME_AS_MIRROR_ONLY_CLOSURE,a=%lld,%s\n",
        a, self_mirror_closed ? "YES" : "NO",
        a, same_as_mirror_only ? "YES" : "NO");

    // Characterize the 18 missing states structurally: are they exactly
    // the neighbor's initial_extension_states (E_1, 22 states) minus
    // some subset, or something else entirely?
    const auto e1 = class_ii_neighbor2_initial_extension_states();
    std::size_t missing_in_e1 = 0;
    for (const auto& s : missing) if (e1.count(s)) ++missing_in_e1;
    std::size_t sym_in_e1 = 0;
    for (const auto& s : sym_survivors) if (e1.count(s)) ++sym_in_e1;
    const auto center_set = class_ii_contact_set();
    std::size_t sym_in_center = 0;
    for (const auto& s : sym_survivors) if (center_set.count(s)) ++sym_in_center;
    std::size_t missing_in_center = 0;
    for (const auto& s : missing) if (center_set.count(s)) ++missing_in_center;

    std::set<SNode<3>> mirror_center;
    for (const auto& s : center_set) mirror_center.insert(s.mirror());
    std::size_t sym_in_mirror_center = 0;
    for (const auto& s : sym_survivors) if (mirror_center.count(s)) ++sym_in_mirror_center;
    std::size_t missing_in_mirror_center = 0;
    for (const auto& s : missing) if (mirror_center.count(s)) ++missing_in_mirror_center;

    std::printf(
        "  BREAKDOWN,a=%lld,e1_size=%zu,center_size=%zu,mirror_center_size=%zu,"
        "missing_in_e1=%zu,missing_in_center=%zu,missing_in_mirror_center=%zu,"
        "sym_in_e1=%zu,sym_in_center=%zu,sym_in_mirror_center=%zu\n",
        a, e1.size(), center_set.size(), mirror_center.size(),
        missing_in_e1, missing_in_center, missing_in_mirror_center,
        sym_in_e1, sym_in_center, sym_in_mirror_center);
}

}  // namespace

int main(int argc, char** argv) {
    const long long a_min = argc > 1 ? std::atoll(argv[1]) : 3;
    const long long a_max = argc > 2 ? std::atoll(argv[2]) : 8;
    for (long long a = a_min; a <= a_max; ++a) run(a);
    return 0;
}
