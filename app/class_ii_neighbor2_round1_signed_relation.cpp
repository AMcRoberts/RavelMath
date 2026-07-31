// Checks, precisely (exact set equality, not counting), the relationship
// between the *unsigned* Round-1 objects this investigation has been
// proving (class_ii_neighbor2_round1_window_certificate.cpp,
// class_ii_neighbor2_round1_red_symbolic.cpp) and
// class_ii_neighbor2_signed_contact_set() (50 states), which is what
// the base-premises table in
// docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md names as Round 1's
// actual "neighbor signed-contact set" target.
//
// Two facts, checked rather than assumed:
//
// 1. tau_a's own search_D_cont seeds are NOT self-mirror-closed (the
//    mirror of each of the 9 seeds is a state not among the 9 -- so the
//    backward-closure computation this whole investigation proves is
//    genuinely a one-sided, oriented computation, not a disguised
//    symmetric one).
// 2. class_ii_neighbor2_signed_contact_set() equals EXACTLY (not just
//    in size) the union of the unsigned 25-survivor set
//    (center_states + new_states) with its own mirror image.
//
// Conclusion: "reverse inclusion for the neighbor signed-contact set"
// reduces to the unsigned reverse-inclusion result (closed elsewhere,
// for a>=3) plus the general fact that mirroring a correct
// backward-closure/Red result gives a correct one for the mirrored
// destinations -- a structural property of the corona framework's own
// +/-C convention, not independently re-derived here.

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

    for (long long a : {5LL, 8LL, 13LL}) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const auto seeds = search_D_cont<3>(subst, 2);
        std::set<SNode<3>> sset, mirrored;
        for (const auto& s : seeds) {
            SNode<3> n{s.i, s.x, s.j};
            sset.insert(n);
            mirrored.insert(n.mirror());
        }
        const bool self_mirror = sset == mirrored;
        std::printf("a=%lld D_cont size=%zu self_mirror_closed=%s\n",
            a, sset.size(), self_mirror ? "yes" : "no");
        if (self_mirror) ++dcont_self_mirror_failures;  // unexpected if it fires
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
    const bool equal = full == actual_signed;
    std::printf(
        "unsigned_survivors=%zu, unsigned_survivors+mirror=%zu, "
        "class_ii_neighbor2_signed_contact_set=%zu, EXACT_EQUAL=%s\n",
        unsigned_survivors.size(), full.size(), actual_signed.size(),
        equal ? "yes" : "no");

    const bool ok = dcont_self_mirror_failures == 0 && equal;
    std::printf(
        "%s\n",
        ok ? "ROUND1_SIGNED_RELATION_CONFIRMED: D_cont is genuinely "
             "oriented (not self-mirror-closed), and the signed contact "
             "set is exactly the mirror-closure of the unsigned "
             "25-survivor set this investigation proves reverse "
             "inclusion for"
           : "RELATION DOES NOT HOLD AS EXPECTED -- see output above");
    return ok ? 0 : 1;
}
