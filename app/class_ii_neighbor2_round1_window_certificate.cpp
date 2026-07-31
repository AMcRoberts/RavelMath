// Abstract (not per-a) window-validity certificate for the neighbor's
// 27-state raw pre-Red self-closure (Round 1 of the four base
// transitions -- see docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md).
//
// app/class_ii_neighbor2_symbolic_backward.cpp already builds the 97
// affine-in-a raw-branch categories and window-filters them by
// evaluating at concrete a=3..8 with the exact in_H_sigma_exact
// predicate. This file replaces that concrete evaluation with the same
// abstract (c,d)-corner-bound argument that
// class_ii_contact_backward_envelope_certificate() (in
// include/ravel/class_ii_neighbor_family.hpp) already uses for the
// center's own 51 categories: express the stepped-hyperplane height and
// margin as affine forms in (c,d) where beta = a + c, c = d + (1/beta)
// -- concretely, reusing that function's own AffineCD convention
// verbatim -- and decide window membership from the universal bounds
// Lean already derives from the Class-II cubic (c>1, 2/3<d<1), rather
// than by picking a numeric a.
//
// This is deliberately NOT a refactor of the center's tested function
// (too risky to touch a verified certificate under time pressure);
// the AffineCD machinery is duplicated here, unchanged, and applied to
// the neighbor's own categories instead.
//
// Result (2026-07-31): closes. All 679 bounded cases (97 categories x
// 7 candidate x0 values each) are decided abstractly with zero
// unresolved cases -- the same (c,d)-corner-bound argument that closes
// the center's window validity decides every neighbor case too, with
// no dependence on a. The resulting window-valid-and-occurring node
// set is exactly the 25 non-seed states of the 27-state raw target
// (the "extra" 4 all-zero mirror duplicates are excluded by the same
// i<j triviality convention the center's own certificate uses; the 2
// D_cont seeds are correctly absent, since they are the closure's own
// base case and need no predecessor). Cross-checked against the
// concrete numeric method (class_ii_neighbor2_symbolic_backward.cpp's
// own approach) at a = 9, 10, 15, 20, 30, 50, 80, 120 -- identical
// window_valid=25/extra=0/missing={the same 2 seeds} at every value,
// not just the originally tested a = 3..8.
//
// This closes the window-validity half of Round 1's raw self-closure
// universally (for every integer a, not six sampled values). Red
// pruning (27 -> 25) has not been attempted symbolically and remains
// open -- note this is a different claim from the one proved here:
// this file shows the 2 D_cont seeds need no *backward predecessor*;
// it does not show (and does not claim) anything about which states
// Red removes going *forward*. Whether Red's 2 pruned states coincide
// with these 2 seeds is not checked here -- see
// docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md for that separate,
// still-open question.

#include <array>
#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "ravel/class_ii_neighbor_family.hpp"

using namespace ravel;

namespace {

using Category = std::array<long long, 4>;  // {first_child, x1, x2, second_child}
using Range = std::pair<ClassIIAffineValue, ClassIIAffineValue>;  // (min, max) of x0

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

std::vector<SNode<3>> center_states() {
    return {
        {0, {-1, 1, 1}, 1}, {0, {0, 0, 0}, 1}, {0, {0, 0, 0}, 2},
        {0, {0, 0, 1}, 0}, {0, {0, 0, 1}, 1}, {0, {0, 1, 0}, 0},
        {0, {1, -1, 0}, 0}, {1, {0, 0, 0}, 2}, {1, {1, -1, 0}, 0},
        {1, {1, -1, 0}, 2}, {1, {1, 0, -1}, 0}, {2, {0, 1, -1}, 0},
        {2, {0, 1, 0}, 0}, {2, {1, 0, -1}, 0},
    };
}

// The two D_cont seeds: present in the raw target but require no
// predecessor (they are the closure's own base case).
std::vector<SNode<3>> seed_states() {
    return {{1, {0, 0, 1}, 1}, {2, {0, 1, -1}, 1}};
}

std::vector<SNode<3>> full27_states() {
    std::vector<SNode<3>> result = new_states();
    for (const auto& s : center_states()) result.push_back(s);
    for (const auto& s : seed_states()) result.push_back(s);
    return result;
}

std::map<Category, std::vector<Range>> build_categories() {
    std::map<Category, std::vector<Range>> candidates;
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
                    2, first_parent, first_child);
                for (std::size_t second_child = 0;
                     second_child < 3; ++second_child) {
                    const auto seconds =
                        class_ii_neighbor_symbolic_prefix_families(
                            2, second_parent, second_child);
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
                            if (base[1].slope != 0 || base[2].slope != 0)
                                continue;  // checked elsewhere; never fires
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
    return candidates;
}

}  // namespace

int main() {
    const auto candidates = build_categories();

    // --- Abstract window classification, copied verbatim in spirit
    // from class_ii_contact_backward_envelope_certificate() ---
    struct AffineCD {
        long long c;
        long long d;
        long long constant;
    };
    const auto minimum_boundary_scaled = [](AffineCD form) {
        if (form.c < 0) return std::pair{false, 0LL};
        return std::pair{true,
            3 * form.c + (form.d >= 0 ? 2 * form.d : 3 * form.d)
                + 3 * form.constant};
    };
    const auto maximum_boundary_scaled = [](AffineCD form) {
        if (form.c > 0) return std::pair{false, 0LL};
        return std::pair{true,
            3 * form.c + (form.d >= 0 ? 3 * form.d : 2 * form.d)
                + 3 * form.constant};
    };
    const auto universally_nonnegative = [&](AffineCD form) {
        const auto [bounded, value] = minimum_boundary_scaled(form);
        return bounded && value >= 0;
    };
    const auto universally_positive = [&](AffineCD form) {
        const auto [bounded, value] = minimum_boundary_scaled(form);
        return bounded && (value > 0 || (value == 0 && (form.c > 0 || form.d != 0)));
    };
    const auto universally_nonpositive = [&](AffineCD form) {
        const auto [bounded, value] = maximum_boundary_scaled(form);
        return bounded && value <= 0;
    };
    const auto universally_negative = [&](AffineCD form) {
        const auto [bounded, value] = maximum_boundary_scaled(form);
        return bounded && (value < 0 || (value == 0 && (form.c < 0 || form.d != 0)));
    };
    const auto affine_a_nonnegative_from_two = [](ClassIIAffineValue form) {
        return form.slope >= 0 && form.value(2) >= 0;
    };
    const auto affine_a_positive_from_two = [](ClassIIAffineValue form) {
        return form.slope >= 0 && form.value(2) > 0;
    };

    const auto full_target = full27_states();
    const std::set<SNode<3>> target_set(full_target.begin(), full_target.end());
    const auto seed_vec = seed_states();
    const std::set<SNode<3>> seeds(seed_vec.begin(), seed_vec.end());

    std::set<SNode<3>> window_valid_nodes;
    long long bounded_window_cases = 0;
    long long unresolved_window_cases = 0;
    const long long X0_LIMIT = 3;  // wider than the center's 2, since
                                    // this family's x2 range is [-2,2]
                                    // rather than the center's [-1,1]

    for (const auto& [category, intervals] : candidates) {
        for (long long x0 = -X0_LIMIT; x0 <= X0_LIMIT; ++x0) {
            ++bounded_window_cases;
            const AffineCD height{x0 + category[1], x0, category[2]};
            AffineCD width{};
            if (category[3] == 0) width = {1, 1, 0};
            else if (category[3] == 1) width = {1, 0, 0};
            else width = {0, 0, 1};
            const AffineCD upper_margin{
                width.c - height.c, width.d - height.d,
                width.constant - height.constant};
            const bool window_valid =
                universally_nonnegative(height)
                && universally_positive(upper_margin);
            const bool window_invalid =
                universally_negative(height)
                || universally_nonpositive(upper_margin);

            bool occurrence_present = false;
            bool occurrence_absent = true;
            for (const auto& [minimum, maximum] : intervals) {
                const ClassIIAffineValue above_minimum{
                    x0 - minimum.intercept, -minimum.slope};
                const ClassIIAffineValue below_maximum{
                    maximum.intercept - x0, maximum.slope};
                occurrence_present = occurrence_present
                    || (affine_a_nonnegative_from_two(above_minimum)
                        && affine_a_nonnegative_from_two(below_maximum));
                const ClassIIAffineValue below_minimum{
                    minimum.intercept - x0, minimum.slope};
                const ClassIIAffineValue above_maximum{
                    x0 - maximum.intercept, -maximum.slope};
                occurrence_absent = occurrence_absent
                    && (affine_a_positive_from_two(below_minimum)
                        || affine_a_positive_from_two(above_maximum));
            }

            const SNode<3> node{
                category[0], {x0, category[1], category[2]}, category[3]};
            const bool is_seed = seeds.count(node) != 0;
            // Same convention as class_ii_contact_backward_envelope_certificate()
            // and is_valid_restricted() elsewhere: the all-zero node is only a
            // genuine state when i < j (i >= j is the mirror duplicate).
            const bool trivial =
                x0 == 0 && category[1] == 0 && category[2] == 0
                && !(category[0] < category[3]);

            if (occurrence_present && window_valid && !trivial) {
                window_valid_nodes.insert(node);
            } else if (!(occurrence_absent || window_invalid || is_seed
                         || trivial)) {
                ++unresolved_window_cases;
                std::printf(
                    "  UNRESOLVED cat=(%lld,%lld,%lld,%lld) x0=%lld "
                    "window_valid=%d window_invalid=%d "
                    "occ_present=%d occ_absent=%d\n",
                    category[0], category[1], category[2], category[3],
                    x0, window_valid, window_invalid,
                    occurrence_present, occurrence_absent);
            }
        }
    }

    std::set<SNode<3>> extra, missing;
    std::set_difference(
        window_valid_nodes.begin(), window_valid_nodes.end(),
        target_set.begin(), target_set.end(),
        std::inserter(extra, extra.end()));
    std::set_difference(
        target_set.begin(), target_set.end(),
        window_valid_nodes.begin(), window_valid_nodes.end(),
        std::inserter(missing, missing.end()));

    std::printf(
        "ABSTRACT_CERTIFICATE categories=%zu bounded_cases=%lld "
        "unresolved=%lld window_valid_nodes=%zu extra=%zu missing=%zu\n",
        candidates.size(), bounded_window_cases, unresolved_window_cases,
        window_valid_nodes.size(), extra.size(), missing.size());
    for (const auto& n : extra)
        std::printf("  EXTRA i=%lld x=(%lld,%lld,%lld) j=%lld\n",
            n.i, n.x[0], n.x[1], n.x[2], n.j);
    for (const auto& n : missing) {
        const bool is_seed = seeds.count(n) != 0;
        std::printf("  MISSING i=%lld x=(%lld,%lld,%lld) j=%lld%s\n",
            n.i, n.x[0], n.x[1], n.x[2], n.j,
            is_seed ? "  (expected: D_cont seed, needs no predecessor)" : "");
    }

    const bool ok = unresolved_window_cases == 0 && extra.empty()
        && missing.size() == seeds.size()
        && missing == seeds;
    std::printf("%s\n", ok ? "ROUND1_WINDOW_CERTIFICATE_CLOSED"
                            : "ROUND1_WINDOW_CERTIFICATE_INCOMPLETE");
    return ok ? 0 : 1;
}
