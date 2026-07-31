// Symbolic (affine-in-a) backward-branch derivation for the neighbor's
// 11 states genuinely new beyond the center's 14, built by mirroring
// class_ii_contact_backward_envelope_certificate()'s own type-1/type-2
// combination logic exactly (that logic is validated -- it's what
// backward_edges implements, confirmed against known-correct ground
// truth in the bfs_v2 driver), but pointed at the neighbor's actual
// image structure via class_ii_neighbor_symbolic_prefix_families(2,...)
// instead of the center's own hardcoded pre_contact_set() enumeration.
//
// This produces, per (destination, type, first_child, second_child),
// an affine-in-a (min,max) range for x0 -- the symbolic generalization
// of the numeric harvest from earlier passes, but built from the
// validated formula this time, not the mismatched transition_weight.

#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"
#include "ravel/contact_boundary.hpp"
#include "ravel/core.hpp"
#include "ravel/d_cont_check.hpp"
#include "ravel/spectral.hpp"
#include "ravel/substitution.hpp"
#include "ravel/substitution_neighborhood.hpp"
#include "ravel/survey.hpp"

using namespace ravel;

namespace {

// The 11 states genuinely new to the neighbor beyond the center's 14
// (confirmed by backward_closure/red_anode, node-for-node, a=3..15,
// zero exceptions -- see docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md
// "Round 1: a precise structural narrowing"). E_1 is the mirror-closure
// of exactly this set.
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

// The center's own already-proved 14-state contact set, present
// unchanged in the neighbor -- see class_ii_contact_set().
std::vector<SNode<3>> center_states() {
    return {
        {0, {-1, 1, 1}, 1}, {0, {0, 0, 0}, 1}, {0, {0, 0, 0}, 2},
        {0, {0, 0, 1}, 0}, {0, {0, 0, 1}, 1}, {0, {0, 1, 0}, 0},
        {0, {1, -1, 0}, 0}, {1, {0, 0, 0}, 2}, {1, {1, -1, 0}, 0},
        {1, {1, -1, 0}, 2}, {1, {1, 0, -1}, 0}, {2, {0, 1, -1}, 0},
        {2, {0, 1, 0}, 0}, {2, {1, 0, -1}, 0},
    };
}

// The full 27-state raw pre-Red closure the destinations below need
// to reproduce under self-closure: 14 shared + 11 new + the 2 that
// Red later prunes (present in the raw closure, absent from the
// post-Red survivor sets used above).
std::vector<SNode<3>> full27_states() {
    std::vector<SNode<3>> result = new_states();
    for (const auto& s : center_states()) result.push_back(s);
    result.push_back({1, {0, 0, 1}, 1});   // pruned by Red
    result.push_back({2, {0, 1, -1}, 1});  // pruned by Red
    return result;
}

using Category = std::array<long long, 4>;  // first_child, x1, x2, second_child
using Range = std::pair<ClassIIAffineValue, ClassIIAffineValue>;  // (min, max)

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

bool is_valid_restricted(const Substitution<3>& subst, const SNode<3>& n) {
    const bool all_zero = n.x[0] == 0 && n.x[1] == 0 && n.x[2] == 0;
    if (all_zero && !(n.i < n.j)) return false;
    IVec<3> xv{n.x[0], n.x[1], n.x[2]};
    return subst.in_H_sigma_exact(xv, static_cast<std::size_t>(n.j));
}

}  // namespace

int main() {
    const std::size_t neighbor = 2;
    std::map<Category, std::vector<Range>> candidates;
    long long assumption_violations = 0;

    for (const auto& destination : full27_states()) {
        const std::array<ClassIIAffineValue, 3> mx{{
            {destination.x[2], destination.x[0] + destination.x[1]},
            {destination.x[0], 0},
            {destination.x[0] + destination.x[1], 0},
        }};
        for (int type = 1; type <= 2; ++type) {
            const std::size_t first_parent = static_cast<std::size_t>(
                type == 1 ? destination.i : destination.j);
            const std::size_t second_parent = static_cast<std::size_t>(
                type == 1 ? destination.j : destination.i);
            const long long sign = type == 1 ? 1 : -1;
            for (std::size_t first_child = 0; first_child < 3; ++first_child) {
                const auto firsts = class_ii_neighbor_symbolic_prefix_families(
                    neighbor, first_parent, first_child);
                for (std::size_t second_child = 0;
                     second_child < 3; ++second_child) {
                    const auto seconds =
                        class_ii_neighbor_symbolic_prefix_families(
                            neighbor, second_parent, second_child);
                    for (const auto& first : firsts)
                        for (const auto& second : seconds) {
                            std::array<ClassIIAffineValue, 3> base{};
                            for (std::size_t c = 0; c < 3; ++c) {
                                base[c] = {
                                    sign * mx[c].intercept
                                        + first.base[c].intercept
                                        - second.base[c].intercept,
                                    sign * mx[c].slope
                                        + first.base[c].slope
                                        - second.base[c].slope};
                            }
                            // Sanity check, not an assumption: category
                            // coordinates 1 and 2 must be plain integers
                            // (slope 0). If this ever fails the category
                            // map below is silently wrong -- catch it
                            // instead of trusting it.
                            if (base[1].slope != 0 || base[2].slope != 0) {
                                ++assumption_violations;
                                continue;
                            }
                            auto minimum = base[0];
                            auto maximum = base[0];
                            if (first.varying_coordinate == 0)
                                maximum = maximum + first.length
                                    - ClassIIAffineValue{1, 0};
                            if (second.varying_coordinate == 0)
                                minimum = minimum - second.length
                                    + ClassIIAffineValue{1, 0};
                            const Category category{
                                static_cast<long long>(first_child),
                                base[1].intercept, base[2].intercept,
                                static_cast<long long>(second_child)};
                            candidates[category].push_back(
                                {minimum, maximum});
                        }
                }
            }
        }
    }

    std::printf(
        "CATEGORIES,%zu,ASSUMPTION_VIOLATIONS,%lld\n",
        candidates.size(), assumption_violations);

    // Numeric validation: evaluate every category's range at concrete a,
    // window-filter with the exact (not approximate) predicate, and
    // compare the resulting node set against the trusted 27-state
    // ground truth from backward_closure/red_anode -- not asserted,
    // checked.
    for (long long a = 3; a <= 8; ++a) {
        const auto center = class_ii(static_cast<std::size_t>(a));
        const auto spectral = classify_matrix_spectral(incidence(center));
        const auto neighbors = adjacent_swap_neighbors(center);
        const SubstitutionRule rule(neighbors[2].substitution);
        const auto subst = make_substitution<3>(rule, spectral.beta);

        std::set<SNode<3>> window_valid_raw;
        for (const auto& [category, ranges] : candidates) {
            for (const auto& [minimum, maximum] : ranges) {
                const long long lo = minimum.value(a);
                const long long hi = maximum.value(a);
                for (long long x0 = lo; x0 <= hi; ++x0) {
                    const SNode<3> node{
                        category[0], {x0, category[1], category[2]},
                        category[3]};
                    if (is_valid_restricted(subst, node))
                        window_valid_raw.insert(node);
                }
            }
        }
        const auto full_target = full27_states();
        const std::set<SNode<3>> target(full_target.begin(), full_target.end());
        std::set<SNode<3>> extra, missing;
        std::set_difference(
            window_valid_raw.begin(), window_valid_raw.end(),
            target.begin(), target.end(),
            std::inserter(extra, extra.end()));
        std::set_difference(
            target.begin(), target.end(),
            window_valid_raw.begin(), window_valid_raw.end(),
            std::inserter(missing, missing.end()));
        std::printf(
            "VALIDATE,%lld,raw_window_valid=%zu,extra=%zu,missing=%zu\n",
            a, window_valid_raw.size(), extra.size(), missing.size());
        for (const auto& n : extra)
            std::printf(
                "  EXTRA,%lld,%lld,%lld,%lld,%lld,%lld\n",
                a, n.i, n.x[0], n.x[1], n.x[2], n.j);
        for (const auto& n : missing)
            std::printf(
                "  MISSING,%lld,%lld,%lld,%lld,%lld,%lld\n",
                a, n.i, n.x[0], n.x[1], n.x[2], n.j);
    }
    return 0;
}
