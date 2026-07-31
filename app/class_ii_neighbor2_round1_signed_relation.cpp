// Checks, precisely (exact set equality, not counting), the relationship
// between the *unsigned* Round-1 objects this investigation has been
// proving (class_ii_neighbor2_round1_window_certificate.cpp,
// class_ii_neighbor2_round1_red_symbolic.cpp) and
// class_ii_neighbor2_signed_contact_set() (50 states), which is what
// the base-premises table in
// docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md names as Round 1's
// actual "neighbor signed-contact set" target.
//
// Three facts, checked rather than assumed -- including one that
// corrects an earlier, too-optimistic guess from the same session:
//
// 1. tau_a's own search_D_cont seeds are NOT self-mirror-closed (the
//    mirror of each of the 9 seeds is a state not among the 9 -- so the
//    backward-closure computation this whole investigation proves is
//    genuinely a one-sided, oriented computation, not a disguised
//    symmetric one).
// 2. class_ii_neighbor2_signed_contact_set() equals EXACTLY (not just
//    in size) the union of the unsigned 25-survivor set
//    (center_states + new_states) with its own mirror image. This is a
//    DEFINITIONAL fact about how build_signed_contact_set is built
//    (union each state with its own mirror), not a closure-symmetry
//    result.
// 3. (Correction of an earlier guess in this same session.) Running
//    backward_closure -> red_anode from the *mirrored* D_cont seeds
//    does NOT reproduce mirror(unsigned_survivors) -- it gives a
//    different survivor count entirely. So "mirroring is a correct
//    closure operation" is false in the naive form; fact 2 above is
//    sound only as a definitional union, not as evidence that the
//    union is itself a complete closure of anything symmetric.
//
// Conclusion: the unsigned reverse-inclusion result (closed elsewhere,
// for a>=3) is real and the definitional relationship to the signed
// object is now exact rather than assumed -- but whether the signed
// union is itself the correct/complete answer to "reverse inclusion
// for the neighbor signed-contact set" in the closure sense the
// base-premises table means remains genuinely open, narrower than
// before but not resolved.

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

std::vector<SNode<3>> new_states() {
    return {
        {0, {-1, 1, 1}, 0}, {0, {-1, 1, 1}, 2},
        {0, {1, -1, 0}, 2}, {0, {1, -1, 1}, 0},
        {0, {1, 0, -1}, 0}, {1, {0, 1, 0}, 0},
        {1, {1, -1, 0}, 1}, {1, {2, -1, -1}, 0},
        {2, {-1, 2, 0}, 1}, {2, {1, -1, 0}, 0},
        {2, {1, -1, 0}, 2},
    };
}

}  // namespace

int main() {
    long long dcont_self_mirror_failures = 0;
    long long naive_mirror_closure_matches = 0;
    const std::vector<long long> test_as = {5, 8, 13};

    for (long long a : test_as) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const auto seeds = search_D_cont<3>(subst, 2);
        std::vector<ANode<3>> d_cont;
        std::set<SNode<3>> sset, mirrored_seed_set;
        for (const auto& s : seeds) {
            ANode<3> n{s.i, s.x, s.j};
            d_cont.push_back(n);
            SNode<3> sn{s.i, s.x, s.j};
            sset.insert(sn);
            mirrored_seed_set.insert(sn.mirror());
        }
        const bool self_mirror = sset == mirrored_seed_set;
        std::printf("a=%lld D_cont size=%zu self_mirror_closed=%s\n",
            a, sset.size(), self_mirror ? "yes" : "no");
        if (self_mirror) ++dcont_self_mirror_failures;  // unexpected if it fires

        // Fact 3: does mirroring the seeds and rerunning the same
        // trusted pipeline reproduce the mirror of the real survivors?
        const auto pre_contact = backward_closure<3>(subst, d_cont);
        const auto induced = induced_restricted_edges<3>(subst, pre_contact);
        const std::set<ANode<3>> pre_set(pre_contact.begin(), pre_contact.end());
        const auto reduced = red_anode<3>(pre_set, induced);
        std::set<SNode<3>> real_survivors;
        for (const auto& n : reduced.first) real_survivors.insert(to_simple<3>(n));

        std::vector<ANode<3>> mirrored_seeds;
        for (const auto& n : d_cont) {
            const SNode<3> sn{n.i, n.x, n.j};
            const SNode<3> m = sn.mirror();
            mirrored_seeds.push_back(ANode<3>{m.i, m.x, m.j});
        }
        const auto pre_contact_m = backward_closure<3>(subst, mirrored_seeds);
        const auto induced_m = induced_restricted_edges<3>(subst, pre_contact_m);
        const std::set<ANode<3>> pre_set_m(pre_contact_m.begin(), pre_contact_m.end());
        const auto reduced_m = red_anode<3>(pre_set_m, induced_m);
        std::set<SNode<3>> mirrored_run_survivors;
        for (const auto& n : reduced_m.first)
            mirrored_run_survivors.insert(to_simple<3>(n));

        std::set<SNode<3>> expected_mirror;
        for (const auto& s : real_survivors) expected_mirror.insert(s.mirror());
        const bool naive_matches = expected_mirror == mirrored_run_survivors;
        std::printf(
            "  naive mirror-and-rerun: real_survivors=%zu "
            "mirrored_run_survivors=%zu matches_mirror(real)=%s\n",
            real_survivors.size(), mirrored_run_survivors.size(),
            naive_matches ? "yes" : "no");
        if (naive_matches) ++naive_mirror_closure_matches;
    }

    const auto center_contact = class_ii_contact_set();
    const auto news = new_states();
    std::set<SNode<3>> unsigned_survivors(center_contact.begin(), center_contact.end());
    for (const auto& s : news) unsigned_survivors.insert(s);

    std::set<SNode<3>> full;
    for (const auto& s : unsigned_survivors) {
        full.insert(s);
        full.insert(s.mirror());
    }

    const auto actual_signed = class_ii_neighbor2_signed_contact_set();
    const bool definitional_union_equal = full == actual_signed;
    std::printf(
        "unsigned_survivors=%zu, unsigned_survivors+mirror=%zu, "
        "class_ii_neighbor2_signed_contact_set=%zu, "
        "DEFINITIONAL_UNION_EXACT=%s\n",
        unsigned_survivors.size(), full.size(), actual_signed.size(),
        definitional_union_equal ? "yes" : "no");

    // Expected shape of this result: D_cont genuinely oriented (checked
    // true), the definitional union exact (checked true), and the naive
    // mirror-and-rerun claim FALSE at every tested a (0 matches) --
    // confirming the correction, not silently regressing to "actually
    // it works after all."
    const bool ok = dcont_self_mirror_failures == 0
        && definitional_union_equal
        && naive_mirror_closure_matches == 0;
    std::printf(
        "%s\n",
        ok ? "ROUND1_SIGNED_RELATION_CONFIRMED: D_cont is genuinely "
             "oriented; the signed contact set is exactly the "
             "definitional mirror-union of the unsigned 25-survivor "
             "set; and naive mirror-and-rerun closure does NOT "
             "reproduce that union (confirmed false at every tested a, "
             "as the correction above states) -- so the signed object's "
             "correctness rests on the corona framework's own +/-C "
             "convention, not on closure symmetry"
           : "UNEXPECTED RESULT SHAPE -- see output above, one of the "
             "three checks did not land where expected");
    return ok ? 0 : 1;
}
