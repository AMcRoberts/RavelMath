// Extends the Round-1 Red-pruning investigation
// (class_ii_neighbor2_round1_red_identity.cpp) one step further: checks,
// densely and at large a, a closed-form guess for the *forward* images
// of the two D_cont seeds `A = {1,{0,0,1},1}` and `B = {2,{0,1,-1},1}`
// (the pair class_ii_neighbor2_round1_red_identity.cpp already confirmed
// is exactly what Red prunes, for a = 3..60), and combines it with an
// elementary bound to give a well-scoped -- but explicitly NOT
// symbolically re-derived -- reason why.
//
// Observed (checked exactly, not just printed, at a = 3..200 plus the
// sparse samples 500, 1000, 5000, 20000):
//   forward_edges(A) = { (0, {0, 1, -a}, 0) }                     -- always exactly 1 edge
//   forward_edges(B) = { (1, {1, -2, a+1}, 0), (0, {0, 1, -a}, 0) } -- always exactly 2 edges
//
// Both images have an x2-coordinate that is unbounded in a (-a for A's
// target, -a and a+1 for B's two targets). Every one of the 27 raw
// pre-Red target states (new_states + center_states + the seed pair
// itself) has all three coordinates in [-2,2] -- checked directly
// below, not assumed. So for every a >= 3, neither A's nor B's forward
// image can equal any of the 27 targets (|-a| = a >= 3 > 2, and
// a+1 >= 4 > 2), meaning induced_restricted_edges finds zero edges
// from A or B into the 27-state raw set, for every a >= 3 -- which is
// exactly the rank-one Red-pruning criterion (red_anode removes any
// node with no outgoing edge within the given node set).
//
// What this is NOT: a symbolic re-derivation of forward_edges_type1/
// type2's parent_decompositions + linear-solve machinery showing the
// closed form holds for literal every a. It is dense + sparse-at-scale
// numeric confirmation (a=3..200, then 500/1000/5000/20000) of an exact
// match against the guessed closed form, which is strong evidence but
// not the same thing as a derivation from the substitution's actual
// image structure. That derivation is the remaining honest gap.

#include <cstdio>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/proof/class_ii_round1_red_pruning_data.hpp"
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

// The 27-state raw pre-Red target (same list as
// class_ii_neighbor2_round1_window_certificate.cpp), now shared via
// ravel::class_ii_round1_raw27_targets() (also the source Lean's
// round1Raw27 cites) rather than duplicated inline here.
std::vector<std::array<long long, 5>> target_states() {
    return ravel::class_ii_round1_raw27_targets();
}

}  // namespace

int main() {
    // --- Bound check on the 27 targets: exact, not assumed ---
    long long max_abs = 0;
    for (const auto& s : target_states())
        for (int k = 1; k <= 3; ++k)
            max_abs = std::max(max_abs, std::llabs(s[k]));
    std::printf("target_bound max_abs_coordinate=%lld (over %zu states)\n",
        max_abs, target_states().size());

    // --- Dense + sparse closed-form check for forward(A), forward(B) ---
    long long mismatches = 0;
    std::vector<long long> as;
    for (long long a = 3; a <= 200; ++a) as.push_back(a);
    for (long long a : {500LL, 1000LL, 5000LL, 20000LL}) as.push_back(a);

    for (long long a : as) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const Substitution<3> subst = make_substitution<3>(
            SubstitutionRule(neighbors[2].substitution), spectral.beta);

        const ANode<3> A{1, {0, 0, 1}, 1};
        const ANode<3> B{2, {0, 1, -1}, 1};
        const auto fa = forward_edges<3>(subst, A);
        const auto fb = forward_edges<3>(subst, B);

        const bool a_ok = fa.size() == 1
            && fa[0].i == 0 && fa[0].x[0] == 0 && fa[0].x[1] == 1
            && fa[0].x[2] == -a && fa[0].j == 0;
        bool b_has_first = false, b_has_second = false;
        for (const auto& n : fb) {
            if (n.i == 1 && n.x[0] == 1 && n.x[1] == -2 && n.x[2] == a + 1
                    && n.j == 0)
                b_has_first = true;
            if (n.i == 0 && n.x[0] == 0 && n.x[1] == 1 && n.x[2] == -a
                    && n.j == 0)
                b_has_second = true;
        }
        const bool b_ok = fb.size() == 2 && b_has_first && b_has_second;

        if (!a_ok || !b_ok) {
            ++mismatches;
            std::printf(
                "MISMATCH a=%lld a_ok=%d b_ok=%d fa.size=%zu fb.size=%zu\n",
                a, a_ok, b_ok, fa.size(), fb.size());
        }
    }

    std::printf(
        "CLOSED_FORM_CHECK tested=%zu mismatches=%lld\n",
        as.size(), mismatches);

    // --- The elementary conclusion, stated precisely ---
    const bool bound_argument_valid = max_abs == 2;
    const bool closed_form_confirmed = mismatches == 0;
    const bool conclusion =
        bound_argument_valid && closed_form_confirmed;
    std::printf(
        "%s\n",
        conclusion
            ? "IF the closed form for forward(A)/forward(B) holds for "
              "every integer a (confirmed here at a dense/sparse sample, "
              "not derived from parent_decompositions), THEN neither A "
              "nor B has any edge into the 27-state raw target for any "
              "a>=3 (their images' |x2| >= a >= 3 > 2, the target bound), "
              "so red_anode removes both at rank one for every a>=3 -- "
              "matching class_ii_neighbor2_round1_red_identity.cpp's "
              "exact check at a=3..60 exactly."
            : "CONCLUSION DOES NOT FOLLOW -- see mismatches/bound above");
    return conclusion ? 0 : 1;
}
