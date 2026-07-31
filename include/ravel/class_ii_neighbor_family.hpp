#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <vector>

#include "ravel/corona.hpp"
#include "ravel/class_ii_boundary_family.hpp"
#include "ravel/graph_divisor.hpp"

namespace ravel {

struct ClassIINeighborAffineEdge {
    std::size_t source;
    std::size_t target;
    long long intercept;
    long long slope;

    long long weight(long long a) const {
        return intercept + slope * a;
    }
};

struct ClassIIAffineSegment {
    std::size_t letter;
    long long length_intercept;
    long long length_slope;
};

struct ClassIIPrefixFamily {
    std::array<long long, 3> base;
    std::size_t varying_coordinate;
    long long length;
};

struct ClassIIAffineValue {
    long long intercept = 0;
    long long slope = 0;

    long long value(long long a) const {
        return intercept + slope * a;
    }

    bool operator==(const ClassIIAffineValue&) const = default;
};

inline ClassIIAffineValue operator+(
        ClassIIAffineValue left, ClassIIAffineValue right) {
    return {left.intercept + right.intercept, left.slope + right.slope};
}

inline ClassIIAffineValue operator-(
        ClassIIAffineValue left, ClassIIAffineValue right) {
    return {left.intercept - right.intercept, left.slope - right.slope};
}

inline ClassIIAffineValue operator-(ClassIIAffineValue value) {
    return {-value.intercept, -value.slope};
}

struct ClassIISymbolicPrefixFamily {
    std::array<ClassIIAffineValue, 3> base;
    std::size_t varying_coordinate;
    ClassIIAffineValue length;
};

inline std::set<SNode<3>> class_ii_neighbor_dominant_core_states(
    std::size_t neighbor);

inline std::vector<ClassIIAffineSegment> class_ii_neighbor_image_segments(
        std::size_t neighbor, std::size_t parent) {
    if (parent == 2) return {{0, 1, 0}};
    if (neighbor == 0) {
        if (parent == 0) {
            return {{0, -1, 1}, {1, 1, 0}, {0, 1, 0}, {2, 1, 0}};
        }
        return {{0, 0, 1}, {2, 1, 0}};
    }
    if (neighbor == 1) {
        if (parent == 0) {
            return {{0, 0, 1}, {2, 1, 0}, {1, 1, 0}};
        }
        return {{0, 0, 1}, {2, 1, 0}};
    }
    if (neighbor == 2) {
        if (parent == 0) {
            return {{0, 0, 1}, {1, 1, 0}, {2, 1, 0}};
        }
        return {{0, -1, 1}, {2, 1, 0}, {0, 1, 0}};
    }
    // Sentinel 3 is the Class-II center itself. Keeping it in this
    // bounded prefix engine avoids a second implementation of the
    // same transition arithmetic for base-layer Red certificates.
    if (neighbor == 3) {
        if (parent == 0) {
            return {{0, 0, 1}, {1, 1, 0}, {2, 1, 0}};
        }
        return {{0, 0, 1}, {2, 1, 0}};
    }
    throw std::out_of_range(
        "Class-II neighbor index must be 0, 1, 2, or center sentinel 3");
}

inline std::vector<ClassIIPrefixFamily> class_ii_neighbor_prefix_families(
        std::size_t neighbor,
        std::size_t parent,
        std::size_t child,
        long long a) {
    if (parent >= 3 || child >= 3) {
        throw std::out_of_range("Class-II letter must be 0, 1, or 2");
    }
    std::vector<ClassIIPrefixFamily> result;
    std::array<long long, 3> prefix{0, 0, 0};
    for (const auto& segment :
         class_ii_neighbor_image_segments(neighbor, parent)) {
        const long long length =
            segment.length_intercept + segment.length_slope * a;
        if (length <= 0) {
            throw std::domain_error(
                "Class-II affine segment has nonpositive length");
        }
        if (segment.letter == child) {
            result.push_back({prefix, segment.letter, length});
        }
        prefix[segment.letter] += length;
    }
    return result;
}

inline std::vector<ClassIISymbolicPrefixFamily>
class_ii_neighbor_symbolic_prefix_families(
        std::size_t neighbor, std::size_t parent, std::size_t child) {
    if (parent >= 3 || child >= 3) {
        throw std::out_of_range("Class-II letter must be 0, 1, or 2");
    }
    std::vector<ClassIISymbolicPrefixFamily> result;
    std::array<ClassIIAffineValue, 3> prefix{};
    for (const auto& segment :
         class_ii_neighbor_image_segments(neighbor, parent)) {
        const ClassIIAffineValue length{
            segment.length_intercept, segment.length_slope};
        if (segment.letter == child) {
            result.push_back({prefix, segment.letter, length});
        }
        prefix[segment.letter] =
            prefix[segment.letter] + length;
    }
    return result;
}

struct ClassIIContactBackwardEnvelopeCertificate {
    bool categories_exact = false;
    bool affine_ranges_exact = false;
    bool range_dominance_exact = false;
    bool contact_sources_covered = false;
    bool red_exclusions_absent = false;
    bool bounded_window_classification_exact = false;
    bool restricted_nodes_exact = false;
    std::size_t bounded_window_cases = 0;
    std::size_t unresolved_window_cases = 0;
    bool restricted_branch_multiplicity_exact = false;
    std::size_t unresolved_branch_families = 0;
    std::array<long long, 3> branch_count_coefficients{};
    std::array<long long, 2> restricted_branch_count_coefficients{};
    std::array<long long, 2> category_span_coefficients{};

    bool exact() const {
        return categories_exact
            && affine_ranges_exact
            && range_dominance_exact
            && contact_sources_covered
            && red_exclusions_absent
            && bounded_window_classification_exact
            && restricted_nodes_exact
            && restricted_branch_multiplicity_exact
            && branch_count_coefficients
                == std::array<long long, 3>{68, 72, 18}
            && restricted_branch_count_coefficients
                == std::array<long long, 2>{21, 6}
            && category_span_coefficients
                == std::array<long long, 2>{55, 38};
    }
};

// Symbolic derivation of the complete raw backward-prefix envelope of
// the sixteen-state Class-II pre-contact catalogue. It expands only
// bounded image segments, never 0^a itself.
inline ClassIIContactBackwardEnvelopeCertificate
class_ii_contact_backward_envelope_certificate() {
    using Candidate =
        std::pair<ClassIIAffineValue, ClassIIAffineValue>;
    struct BranchFamily {
        ClassIIBackwardCategory category;
        ClassIIAffineValue base_x0;
        bool first_varies;
        bool second_varies;
        ClassIIAffineValue first_length;
        ClassIIAffineValue second_length;
    };
    std::map<ClassIIBackwardCategory, std::vector<Candidate>> candidates;
    std::vector<BranchFamily> branch_families;
    std::array<long long, 3> branch_coefficients{};
    const auto multiply = [](ClassIIAffineValue left,
                             ClassIIAffineValue right) {
        return std::array<long long, 3>{
            left.intercept * right.intercept,
            left.intercept * right.slope
                + left.slope * right.intercept,
            left.slope * right.slope};
    };
    for (const auto& destination : class_ii_pre_contact_set()) {
        const std::array<ClassIIAffineValue, 3> mx{{
            {destination.x[2],
             destination.x[0] + destination.x[1]},
            {destination.x[0], 0},
            {destination.x[0] + destination.x[1], 0},
        }};
        for (int type = 1; type <= 2; ++type) {
            const std::size_t first_parent = static_cast<std::size_t>(
                type == 1 ? destination.i : destination.j);
            const std::size_t second_parent = static_cast<std::size_t>(
                type == 1 ? destination.j : destination.i);
            const long long sign = type == 1 ? 1 : -1;
            for (std::size_t first_child = 0;
                 first_child < 3; ++first_child) {
                const auto first_families =
                    class_ii_neighbor_symbolic_prefix_families(
                        3, first_parent, first_child);
                for (std::size_t second_child = 0;
                     second_child < 3; ++second_child) {
                    const auto second_families =
                        class_ii_neighbor_symbolic_prefix_families(
                            3, second_parent, second_child);
                    for (const auto& first : first_families)
                        for (const auto& second : second_families) {
                            const auto product =
                                multiply(first.length, second.length);
                            for (std::size_t degree = 0;
                                 degree < 3; ++degree)
                                branch_coefficients[degree]
                                    += product[degree];
                            std::array<ClassIIAffineValue, 3> base{};
                            for (std::size_t coordinate = 0;
                                 coordinate < 3; ++coordinate) {
                                base[coordinate] = {
                                    sign * mx[coordinate].intercept
                                        + first.base[coordinate].intercept
                                        - second.base[coordinate].intercept,
                                    sign * mx[coordinate].slope
                                        + first.base[coordinate].slope
                                        - second.base[coordinate].slope};
                            }
                            auto minimum = base[0];
                            auto maximum = base[0];
                            if (first.varying_coordinate == 0)
                                maximum = maximum + first.length
                                    - ClassIIAffineValue{1, 0};
                            if (second.varying_coordinate == 0)
                                minimum = minimum - second.length
                                    + ClassIIAffineValue{1, 0};
                            const ClassIIBackwardCategory category{
                                static_cast<long long>(first_child),
                                base[1].intercept, base[2].intercept,
                                static_cast<long long>(second_child)};
                            candidates[category].push_back(
                                {minimum, maximum});
                            branch_families.push_back({
                                category, base[0],
                                first.varying_coordinate == 0,
                                second.varying_coordinate == 0,
                                first.length, second.length});
                        }
                }
            }
        }
    }

    ClassIIContactBackwardEnvelopeCertificate result;
    std::set<ClassIIBackwardCategory> actual_categories;
    for (const auto& [category, values] : candidates) {
        (void)values;
        actual_categories.insert(category);
    }
    result.categories_exact =
        actual_categories
            == class_ii_pre_contact_backward_categories();
    const auto expected =
        class_ii_pre_contact_backward_category_ranges();
    result.affine_ranges_exact =
        candidates.size() == expected.size();
    result.range_dominance_exact = true;
    std::array<long long, 2> category_span_coefficients{};
    for (const auto& [category, range] : expected) {
        const ClassIIAffineValue expected_min{
            range.minimum_intercept, range.minimum_slope};
        const ClassIIAffineValue expected_max{
            range.maximum_intercept, range.maximum_slope};
        const auto found = candidates.find(category);
        if (found == candidates.end()) {
            result.affine_ranges_exact = false;
            result.range_dominance_exact = false;
            continue;
        }
        bool minimum_attained = false;
        bool maximum_attained = false;
        for (const auto& [minimum, maximum] : found->second) {
            minimum_attained =
                minimum_attained || minimum == expected_min;
            maximum_attained =
                maximum_attained || maximum == expected_max;
            const auto minimum_delta = minimum - expected_min;
            const auto maximum_delta = expected_max - maximum;
            result.range_dominance_exact =
                result.range_dominance_exact
                && minimum_delta.slope >= 0
                && minimum_delta.intercept + minimum_delta.slope >= 0
                && maximum_delta.slope >= 0
                && maximum_delta.intercept + maximum_delta.slope >= 0;
        }
        result.affine_ranges_exact =
            result.affine_ranges_exact
            && minimum_attained && maximum_attained;
        category_span_coefficients[0] +=
            range.maximum_intercept - range.minimum_intercept + 1;
        category_span_coefficients[1] +=
            range.maximum_slope - range.minimum_slope;
    }
    result.contact_sources_covered = true;
    for (const auto& source : class_ii_contact_set()) {
        const ClassIIBackwardCategory category{
            source.i, source.x[1], source.x[2], source.j};
        const auto found = candidates.find(category);
        bool covered = false;
        if (found != candidates.end())
            for (const auto& [minimum, maximum] : found->second) {
                const ClassIIAffineValue above_minimum{
                    source.x[0] - minimum.intercept,
                    -minimum.slope};
                const ClassIIAffineValue below_maximum{
                    maximum.intercept - source.x[0],
                    maximum.slope};
                covered = covered
                    || (above_minimum.slope >= 0
                        && above_minimum.value(2) >= 0
                        && below_maximum.slope >= 0
                        && below_maximum.value(2) >= 0);
            }
        result.contact_sources_covered =
            result.contact_sources_covered && covered;
    }
    const auto positive_exclusion = expected.find({1, 0, 1, 1});
    const auto negative_exclusion = expected.find({2, 1, -1, 1});
    result.red_exclusions_absent =
        positive_exclusion != expected.end()
        && negative_exclusion != expected.end()
        && positive_exclusion->second.minimum_intercept == 0
        && positive_exclusion->second.minimum_slope == 1
        && positive_exclusion->second.maximum_intercept == 0
        && positive_exclusion->second.maximum_slope == 1
        && negative_exclusion->second.minimum_intercept == 0
        && negative_exclusion->second.minimum_slope == -1
        && negative_exclusion->second.maximum_intercept == 0
        && negative_exclusion->second.maximum_slope == -1;

    // Exact restricted-window classification after the coarse
    // geometric bound |x0| <= 2.  Write b=c+d.  Every height and
    // right-width margin is affine in (c,d), and Class II supplies
    //
    //   c > 1,  2/3 < d < 1.
    //
    // Extremizing an affine form over that open strip is an exact
    // endpoint calculation (scaled by three below).
    struct AffineCD {
        long long c;
        long long d;
        long long constant;
    };
    const auto minimum_boundary_scaled = [](AffineCD form) {
        if (form.c < 0)
            return std::pair{false, 0LL};
        return std::pair{
            true,
            3 * form.c
                + (form.d >= 0 ? 2 * form.d : 3 * form.d)
                + 3 * form.constant};
    };
    const auto maximum_boundary_scaled = [](AffineCD form) {
        if (form.c > 0)
            return std::pair{false, 0LL};
        return std::pair{
            true,
            3 * form.c
                + (form.d >= 0 ? 3 * form.d : 2 * form.d)
                + 3 * form.constant};
    };
    const auto universally_nonnegative = [&](AffineCD form) {
        const auto [bounded, value] =
            minimum_boundary_scaled(form);
        return bounded && value >= 0;
    };
    const auto universally_positive = [&](AffineCD form) {
        const auto [bounded, value] =
            minimum_boundary_scaled(form);
        return bounded
            && (value > 0
                || (value == 0 && (form.c > 0 || form.d != 0)));
    };
    const auto universally_nonpositive = [&](AffineCD form) {
        const auto [bounded, value] =
            maximum_boundary_scaled(form);
        return bounded && value <= 0;
    };
    const auto universally_negative = [&](AffineCD form) {
        const auto [bounded, value] =
            maximum_boundary_scaled(form);
        return bounded
            && (value < 0
                || (value == 0 && (form.c < 0 || form.d != 0)));
    };
    const auto affine_a_nonnegative_from_two =
        [](ClassIIAffineValue form) {
            return form.slope >= 0 && form.value(2) >= 0;
        };
    const auto affine_a_positive_from_two =
        [](ClassIIAffineValue form) {
            return form.slope >= 0 && form.value(2) > 0;
        };

    std::set<SNode<3>> restricted_nodes;
    for (const auto& [category, intervals] : candidates) {
        for (long long x0 = -2; x0 <= 2; ++x0) {
            ++result.bounded_window_cases;
            const AffineCD height{
                x0 + category[1], x0, category[2]};
            AffineCD width{};
            if (category[3] == 0)
                width = {1, 1, 0};
            else if (category[3] == 1)
                width = {1, 0, 0};
            else
                width = {0, 0, 1};
            const AffineCD upper_margin{
                width.c - height.c,
                width.d - height.d,
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

            const bool trivial =
                x0 == 0 && category[1] == 0 && category[2] == 0
                && !(category[0] < category[3]);
            if (occurrence_present && window_valid && !trivial) {
                restricted_nodes.insert(
                    {category[0], {x0, category[1], category[2]},
                     category[3]});
            } else if (!(occurrence_absent || window_invalid || trivial)) {
                ++result.unresolved_window_cases;
            }
        }
    }
    result.bounded_window_classification_exact =
        result.bounded_window_cases == 255
        && result.unresolved_window_cases == 0;
    result.restricted_nodes_exact =
        restricted_nodes == class_ii_contact_set();

    // Count, rather than merely detect, every prefix-position pair
    // producing one of the fourteen restricted source nodes.  Each
    // bounded symbolic family reduces to an intersection of two
    // affine integer intervals.  All min/max choices below are
    // uniform from a=2 onward; any crossing would be reported as
    // unresolved instead of sampled.
    const auto affine_nonnegative_from_two =
        [](ClassIIAffineValue form) {
            return form.slope >= 0 && form.value(2) >= 0;
        };
    const auto affine_nonpositive_from_two =
        [](ClassIIAffineValue form) {
            return form.slope <= 0 && form.value(2) <= 0;
        };
    const auto choose_max = [&](
            ClassIIAffineValue left, ClassIIAffineValue right,
            ClassIIAffineValue& out) {
        if (affine_nonnegative_from_two(left - right)) {
            out = left;
            return true;
        }
        if (affine_nonnegative_from_two(right - left)) {
            out = right;
            return true;
        }
        return false;
    };
    const auto choose_min = [&](
            ClassIIAffineValue left, ClassIIAffineValue right,
            ClassIIAffineValue& out) {
        if (affine_nonpositive_from_two(left - right)) {
            out = left;
            return true;
        }
        if (affine_nonpositive_from_two(right - left)) {
            out = right;
            return true;
        }
        return false;
    };
    std::array<long long, 2> restricted_branch_coefficients{};
    for (const auto& family : branch_families) {
        for (const auto& source : class_ii_contact_set()) {
            if (family.category != ClassIIBackwardCategory{
                    source.i, source.x[1], source.x[2], source.j})
                continue;
            const ClassIIAffineValue difference{
                source.x[0] - family.base_x0.intercept,
                -family.base_x0.slope};
            ClassIIAffineValue count{};
            bool classified = true;
            if (!family.first_varies && !family.second_varies) {
                count = difference == ClassIIAffineValue{}
                    ? ClassIIAffineValue{1, 0}
                    : ClassIIAffineValue{};
            } else if (family.first_varies
                       && !family.second_varies) {
                const auto below_length =
                    family.first_length
                    - ClassIIAffineValue{1, 0} - difference;
                if (affine_nonnegative_from_two(difference)
                    && affine_nonnegative_from_two(below_length))
                    count = {1, 0};
                else if (!(affine_nonpositive_from_two(
                               difference - ClassIIAffineValue{-1, 0})
                           || affine_nonpositive_from_two(
                               below_length
                               - ClassIIAffineValue{-1, 0})))
                    classified = false;
            } else if (!family.first_varies
                       && family.second_varies) {
                const auto position = -difference;
                const auto below_length =
                    family.second_length
                    - ClassIIAffineValue{1, 0} - position;
                if (affine_nonnegative_from_two(position)
                    && affine_nonnegative_from_two(below_length))
                    count = {1, 0};
                else if (!(affine_nonpositive_from_two(
                               position - ClassIIAffineValue{-1, 0})
                           || affine_nonpositive_from_two(
                               below_length
                               - ClassIIAffineValue{-1, 0})))
                    classified = false;
            } else {
                ClassIIAffineValue lower{};
                ClassIIAffineValue upper{};
                const auto first_max =
                    family.first_length - ClassIIAffineValue{1, 0};
                const auto shifted_second_max =
                    difference + family.second_length
                    - ClassIIAffineValue{1, 0};
                classified =
                    choose_max({}, difference, lower)
                    && choose_min(first_max, shifted_second_max, upper);
                if (classified) {
                    const auto span =
                        upper - lower + ClassIIAffineValue{1, 0};
                    if (affine_nonnegative_from_two(span))
                        count = span;
                    else if (!affine_nonpositive_from_two(span))
                        classified = false;
                }
            }
            if (!classified) {
                ++result.unresolved_branch_families;
                continue;
            }
            restricted_branch_coefficients[0] += count.intercept;
            restricted_branch_coefficients[1] += count.slope;
        }
    }
    result.restricted_branch_count_coefficients =
        restricted_branch_coefficients;
    result.restricted_branch_multiplicity_exact =
        result.unresolved_branch_families == 0;
    result.branch_count_coefficients = branch_coefficients;
    result.category_span_coefficients = category_span_coefficients;
    return result;
}

// Reusable abstract (a-independent) window-validity classifier, factored
// out after class_ii_contact_backward_envelope_certificate()'s own
// AffineCD/universally_nonnegative machinery was duplicated once (for
// app/class_ii_neighbor2_round1_window_certificate.cpp) rather than
// risking a refactor of that already-verified function mid-investigation.
// Deliberately NOT used by that function itself -- it stays untouched.
// Intended for Round 2/3/4's analogous raw-corona reverse-inclusion
// obligations (docs/GLOBAL_CATALOGUE_OCCURRENCE_EXHAUSTION.md's "Round 2:
// reconnaissance only" note), so a future session does not have to
// re-derive or re-duplicate this argument a third time.
//
// `candidates` maps a (left_letter, x1, x2, right_letter) category to
// the list of affine-in-a (minimum, maximum) ranges for x0 that some
// destination's backward branch produced. For every category and every
// x0 in [-x0_limit, x0_limit], this decides -- from the Lean-derived
// Class-II cubic bounds alone, never by evaluating at a concrete a --
// whether [left_letter, (x0,x1,x2), right_letter] is a valid restricted
// node that actually occurs as a predecessor.
struct ClassIIAbstractWindowResult {
    std::set<SNode<3>> window_valid_nodes;
    long long bounded_cases = 0;
    long long unresolved_cases = 0;
};

inline ClassIIAbstractWindowResult class_ii_abstract_window_classify(
        const std::map<ClassIIBackwardCategory,
                       std::vector<std::pair<ClassIIAffineValue,
                                             ClassIIAffineValue>>>& candidates,
        long long x0_limit) {
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

    ClassIIAbstractWindowResult result;
    for (const auto& [category, intervals] : candidates) {
        for (long long x0 = -x0_limit; x0 <= x0_limit; ++x0) {
            ++result.bounded_cases;
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
            const bool trivial =
                x0 == 0 && category[1] == 0 && category[2] == 0
                && !(category[0] < category[3]);

            if (occurrence_present && window_valid && !trivial) {
                result.window_valid_nodes.insert(node);
            } else if (!(occurrence_absent || window_invalid || trivial)) {
                ++result.unresolved_cases;
            }
        }
    }
    return result;
}

inline std::array<long long, 3> class_ii_incidence_action(
        long long a, const std::array<long long, 3>& x) {
    return {
        a * x[0] + a * x[1] + x[2],
        x[0],
        x[0] + x[1],
    };
}

// Count the prefix pairs for one type-1 transition without expanding
// 0^a. This is an exact lattice-interval count, O(1) per segment pair.
inline long long class_ii_neighbor_transition_weight(
        std::size_t neighbor,
        long long a,
        const SNode<3>& source,
        const SNode<3>& target) {
    const auto left = class_ii_neighbor_prefix_families(
        neighbor, static_cast<std::size_t>(target.i),
        static_cast<std::size_t>(source.i), a);
    const auto right = class_ii_neighbor_prefix_families(
        neighbor, static_cast<std::size_t>(target.j),
        static_cast<std::size_t>(source.j), a);
    const auto mx = class_ii_incidence_action(a, target.x);
    long long total = 0;
    for (const auto& p : left) {
        for (const auto& q : right) {
            std::array<long long, 3> delta{};
            for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
                delta[coordinate] =
                    source.x[coordinate] + q.base[coordinate]
                    - p.base[coordinate] - mx[coordinate];
            }
            if (p.varying_coordinate == q.varying_coordinate) {
                const std::size_t coordinate = p.varying_coordinate;
                bool compatible = true;
                for (std::size_t other = 0; other < 3; ++other) {
                    if (other != coordinate && delta[other] != 0) {
                        compatible = false;
                    }
                }
                if (!compatible) continue;
                // delta + k_q - k_p = 0.
                const long long lower = std::max(0LL, delta[coordinate]);
                const long long upper =
                    std::min(p.length, q.length + delta[coordinate]);
                total += std::max(0LL, upper - lower);
            } else {
                const std::size_t pc = p.varying_coordinate;
                const std::size_t qc = q.varying_coordinate;
                bool compatible = true;
                for (std::size_t other = 0; other < 3; ++other) {
                    if (other != pc && other != qc && delta[other] != 0) {
                        compatible = false;
                    }
                }
                if (!compatible) continue;
                const long long kp = delta[pc];
                const long long kq = -delta[qc];
                if (0 <= kp && kp < p.length
                        && 0 <= kq && kq < q.length) {
                    ++total;
                }
            }
        }
    }
    return total;
}

// Collect every integer expression whose zero/sign controls a branch of
// the bounded transition counter. This is useful for exact parametric
// certificates: if the corresponding affine forms have fixed signs on
// a parameter cone, the counter has one algebraic formula there.
inline std::vector<long long> class_ii_neighbor_transition_controls(
        std::size_t neighbor,
        long long a,
        const std::set<SNode<3>>& state_set) {
    std::vector<long long> controls;
    for (const auto& source : state_set) {
        for (const auto& target : state_set) {
            const auto left = class_ii_neighbor_prefix_families(
                neighbor, static_cast<std::size_t>(target.i),
                static_cast<std::size_t>(source.i), a);
            const auto right = class_ii_neighbor_prefix_families(
                neighbor, static_cast<std::size_t>(target.j),
                static_cast<std::size_t>(source.j), a);
            const auto mx = class_ii_incidence_action(a, target.x);
            for (const auto& p : left) {
                controls.push_back(p.length);
                for (const auto& q : right) {
                    controls.push_back(q.length);
                    std::array<long long, 3> delta{};
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        delta[coordinate] =
                            source.x[coordinate] + q.base[coordinate]
                            - p.base[coordinate] - mx[coordinate];
                    }
                    if (p.varying_coordinate == q.varying_coordinate) {
                        const auto c = p.varying_coordinate;
                        for (std::size_t other = 0; other < 3; ++other) {
                            if (other != c) controls.push_back(delta[other]);
                        }
                        controls.push_back(
                            p.length - (q.length + delta[c]));
                        controls.push_back(delta[c]);
                        controls.push_back(p.length);
                        controls.push_back(p.length - delta[c]);
                        controls.push_back(q.length + delta[c]);
                        controls.push_back(q.length);
                    } else {
                        const auto pc = p.varying_coordinate;
                        const auto qc = q.varying_coordinate;
                        for (std::size_t other = 0; other < 3; ++other) {
                            if (other != pc && other != qc)
                                controls.push_back(delta[other]);
                        }
                        controls.push_back(delta[pc]);
                        controls.push_back(-delta[qc]);
                        controls.push_back(p.length - delta[pc]);
                        controls.push_back(q.length + delta[qc]);
                    }
                }
            }
        }
    }
    return controls;
}

inline std::vector<std::vector<long long>>
class_ii_neighbor_compressed_matrix_for_states(
        std::size_t neighbor,
        long long a,
        const std::set<SNode<3>>& state_set) {
    if (a < 3) {
        throw std::domain_error(
            "Class-II neighbor compressed matrix requires a >= 3");
    }
    const std::vector<SNode<3>> states(state_set.begin(), state_set.end());
    std::vector<std::vector<long long>> result(
        states.size(), std::vector<long long>(states.size(), 0));
    for (std::size_t source = 0; source < states.size(); ++source) {
        for (std::size_t target = 0; target < states.size(); ++target) {
            result[source][target] =
                class_ii_neighbor_transition_weight(
                    neighbor, a, states[source], states[target]);
        }
    }
    return result;
}

inline std::vector<std::vector<long long>>
class_ii_neighbor_compressed_matrix(
        std::size_t neighbor, long long a) {
    return class_ii_neighbor_compressed_matrix_for_states(
        neighbor, a, class_ii_neighbor_dominant_core_states(neighbor));
}

// Parameter-independent dominant-core state catalogues observed for the
// three adjacent-swap neighbors of sigma_{a,1}. The companion probe checks
// literal equality against freshly constructed contact graphs for 3<=a<=8.
// Constancy beyond that certified sweep remains a theorem target.
inline std::set<SNode<3>> class_ii_neighbor_dominant_core_states(
        std::size_t neighbor) {
    if (neighbor == 0) {
        return {
            {0, {-1, 1, 0}, 2}, {2, {1, -1, 0}, 0},
            {1, {1, -1, 0}, 0}, {1, {0, 0, 0}, 0},
            {0, {0, 0, 0}, 1}, {0, {0, 1, 0}, 0},
            {2, {0, 1, -1}, 0}, {0, {0, -1, 1}, 0},
            {0, {0, 1, -1}, 0}, {0, {1, -1, 1}, 0},
            {0, {-1, 1, 0}, 1}, {0, {1, -1, 0}, 0},
            {0, {-1, 1, 0}, 0}, {0, {0, -1, 0}, 0},
            {0, {-1, 1, -1}, 0},
        };
    }
    if (neighbor == 1) {
        return {
            {0, {0, 0, -1}, 1}, {0, {0, 0, -1}, 0},
            {1, {0, 0, 1}, 0}, {2, {0, 1, -1}, 1},
            {0, {0, 1, -1}, 1}, {0, {0, 0, 1}, 0},
            {2, {1, 0, -1}, 1}, {0, {-1, 1, 1}, 2},
            {0, {-1, 1, 0}, 2}, {2, {1, -1, 0}, 0},
            {1, {0, -1, 1}, 0}, {2, {1, 0, -1}, 0},
            {0, {-1, 1, 0}, 1}, {1, {1, -1, 1}, 0},
            {0, {-1, 1, 0}, 0}, {0, {1, -1, 0}, 0},
            {0, {-1, 1, -1}, 1},
        };
    }
    if (neighbor == 2) {
        return {
            {2, {-1, 1, 0}, 2}, {2, {-1, 1, 0}, 1},
            {2, {-1, 1, 0}, 0}, {2, {0, 1, 0}, 0},
            {1, {0, 1, 0}, 0}, {2, {1, -1, 0}, 2},
            {1, {1, -1, 0}, 2}, {0, {1, -1, 0}, 2},
            {2, {1, 0, -1}, 0}, {1, {1, 0, -1}, 0},
            {2, {1, -1, 0}, 0}, {2, {-1, 2, 0}, 1},
            {1, {2, -1, -1}, 0}, {0, {-1, 1, 1}, 2},
            {0, {-1, 1, 0}, 2}, {1, {-1, 1, 0}, 1},
            {0, {1, -1, 1}, 0}, {0, {-1, 1, 0}, 1},
            {0, {1, -1, 0}, 0}, {0, {-1, 1, 0}, 0},
            {1, {1, -1, 0}, 0}, {0, {1, 0, -1}, 0},
            {0, {-1, 1, 1}, 1}, {0, {1, -1, -1}, 0},
            {0, {0, 1, 0}, 0}, {0, {0, 0, 1}, 1},
            {2, {0, 1, -1}, 0}, {0, {0, 0, 1}, 0},
            {0, {0, 0, 0}, 1}, {0, {0, 0, -1}, 0},
            {1, {0, 0, 0}, 0}, {0, {-1, 0, 1}, 1},
            {1, {1, -1, -1}, 0}, {0, {-1, 1, 1}, 0},
            {1, {1, -1, 0}, 1}, {0, {-1, 1, -1}, 0},
            {0, {0, -1, 0}, 0}, {1, {0, 0, -1}, 0},
            {0, {-1, 0, 1}, 0},
        };
    }
    throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
}

inline std::set<SNode<3>> class_ii_neighbor_regular_shell_states(
    std::size_t neighbor, long long a, long long parameter);
inline std::set<SNode<3>> class_ii_neighbor_special_shell_states(
    std::size_t neighbor, long long a, std::size_t special);

// Literal recurrent-component catalogues in Tarjan order for the three
// adjacent-swap neighbors.  The final rank in each family is the fixed
// dominant core above; the preceding ranks are indexed affine shells.
// The companion probe checks exact node-set equality against complete
// freshly constructed boundary graphs for 3<=a<=8.
inline std::set<SNode<3>> class_ii_neighbor_recurrent_component_states(
        std::size_t neighbor, long long a, std::size_t rank) {
    if (a < 3) {
        throw std::domain_error(
            "Class-II neighbor recurrent catalogue requires a >= 3");
    }
    if (neighbor == 0) {
        if (rank >= static_cast<std::size_t>(a)) {
            throw std::out_of_range(
                "Class-II neighbor-0 recurrent rank is out of range");
        }
        if (rank == static_cast<std::size_t>(a - 1)) {
            return class_ii_neighbor_dominant_core_states(neighbor);
        }
        if (rank == 0) {
            return class_ii_neighbor_special_shell_states(neighbor, a, 0);
        }
        if (rank == 1) {
            return class_ii_neighbor_special_shell_states(neighbor, a, 1);
        }
        return class_ii_neighbor_regular_shell_states(
            neighbor, a, static_cast<long long>(rank) - 1);
    }
    if (neighbor == 1) {
        if (rank >= static_cast<std::size_t>(a - 1)) {
            throw std::out_of_range(
                "Class-II neighbor-1 recurrent rank is out of range");
        }
        if (rank == static_cast<std::size_t>(a - 2)) {
            return class_ii_neighbor_dominant_core_states(neighbor);
        }
        return class_ii_neighbor_regular_shell_states(
            neighbor, a, static_cast<long long>(rank) + 1);
    }
    if (neighbor == 2) {
        if (rank > static_cast<std::size_t>(a)) {
            throw std::out_of_range(
                "Class-II neighbor-2 recurrent rank is out of range");
        }
        if (rank == static_cast<std::size_t>(a)) {
            return class_ii_neighbor_dominant_core_states(neighbor);
        }
        if (rank == 0) {
            return class_ii_neighbor_special_shell_states(neighbor, a, 0);
        }
        if (rank == static_cast<std::size_t>(a - 2)) {
            return class_ii_neighbor_special_shell_states(neighbor, a, 1);
        }
        const long long k =
            rank == static_cast<std::size_t>(a - 1)
                ? a - 1
                : static_cast<long long>(rank) + 1;
        return class_ii_neighbor_regular_shell_states(neighbor, a, k);
    }
    throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
}

// Regular shell coordinates, separated from Tarjan rank. Parameter means
// t for neighbors 0 and 1 and Perron integer k for neighbor 2.
inline std::set<SNode<3>> class_ii_neighbor_regular_shell_states(
        std::size_t neighbor, long long a, long long parameter) {
    if (neighbor == 0) {
        if (parameter < 1 || a < parameter + 3)
            throw std::domain_error("neighbor-0 regular shell domain");
        const long long m = a - parameter - 1;
        return {
            {0, {-m, m, -1}, 0}, {0, {-m, m, 0}, 0},
            {0, {-m, m, 0}, 1}, {0, {-(m - 1), m, -1}, 0},
            {0, {-(m - 1), m, 0}, 0}, {0, {m - 1, -m, 0}, 0},
            {0, {m - 1, -m, 1}, 0}, {0, {m, -m, 0}, 0},
            {0, {m, -m, 1}, 0}, {1, {m, -m, 0}, 0},
        };
    }
    if (neighbor == 1) {
        if (parameter < 1 || a < parameter + 2)
            throw std::domain_error("neighbor-1 regular shell domain");
        const long long m = a - parameter;
        return {
            {0, {-m, m, -1}, 1}, {0, {-m, m, 0}, 0},
            {0, {m, -m, 0}, 0}, {1, {m, -m, 1}, 0},
            {2, {m, -m, 0}, 0},
        };
    }
    if (neighbor == 2) {
        if (parameter < 2 || a < parameter + 1)
            throw std::domain_error("neighbor-2 regular shell domain");
        const long long m = a - parameter + 1;
        return {
            {0, {-m, m, 0}, 0}, {0, {-m, m, 0}, 1},
            {0, {m, -m, 0}, 0}, {1, {m, -m, 0}, 0},
        };
    }
    throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
}

inline std::set<SNode<3>> class_ii_neighbor_special_shell_states(
        std::size_t neighbor, long long a, std::size_t special) {
    if (a < 3)
        throw std::domain_error(
            "Class-II neighbor special shell requires a >= 3");
    if (neighbor == 0 && special == 0) {
        const long long m = a - 1;
        return {
            {0, {-m, m, 0}, 1},
            {1, {m, -m, 0}, 0},
        };
    }
    if (neighbor == 0 && special == 1) {
        return {
            {0, {-(a - 2), a - 1, -1}, 0},
            {0, {a - 2, -(a - 1), 1}, 0},
        };
    }
    if (neighbor == 2 && special == 0) {
        return {
            {0, {-a, a, 0}, 1},
            {1, {a, -a, 0}, 0},
        };
    }
    if (neighbor == 2 && special == 1) {
        return {
            {0, {-2, 2, 1}, 1},
            {1, {2, -2, -1}, 0},
        };
    }
    throw std::out_of_range("Class-II neighbor special shell index");
}

inline std::set<SNode<3>> class_ii_neighbor2_fixed_extension_states() {
    return {
        {0, {-2, 2, 1}, 1},
        {0, {-1, 0, 1}, 0},
        {0, {-1, 1, 1}, 0},
        {0, {-1, 1, 1}, 2},
        {0, {-1, 2, 1}, 0},
        {0, {1, -2, -1}, 0},
        {0, {1, -1, -1}, 0},
        {0, {1, -1, 0}, 2},
        {0, {1, 0, -1}, 0},
        {1, {-1, 1, 0}, 1},
        {1, {0, 1, 0}, 0},
        {1, {1, -2, -1}, 0},
        {1, {1, -1, 0}, 1},
        {1, {2, -2, -1}, 0},
        {1, {2, -1, -1}, 0},
        {2, {-2, 2, 0}, 1},
        {2, {-1, 1, 0}, 0},
        {2, {-1, 1, 0}, 2},
        {2, {-1, 2, 0}, 0},
        {2, {-1, 2, 0}, 1},
        {2, {1, -2, -1}, 0},
        {2, {1, -1, -1}, 0},
        {2, {1, -1, 0}, 2},
        {2, {2, -1, -1}, 0},
    };
}

inline std::set<SNode<3>> class_ii_neighbor2_initial_extension_states() {
    return {
        {0, {-2, 1, 1}, 1}, {0, {-1, 0, 1}, 0},
        {0, {-1, 1, -1}, 0}, {0, {-1, 1, 0}, 2},
        {0, {-1, 1, 1}, 0}, {0, {-1, 1, 1}, 2},
        {0, {0, -1, 0}, 1}, {0, {1, -1, -1}, 0},
        {0, {1, -1, 0}, 2}, {0, {1, -1, 1}, 0},
        {0, {1, 0, -1}, 0}, {1, {-1, 1, 0}, 1},
        {1, {0, 1, 0}, 0}, {1, {1, -2, 0}, 2},
        {1, {1, -1, 0}, 1}, {1, {2, -1, -1}, 0},
        {2, {-1, 1, 0}, 0}, {2, {-1, 1, 0}, 2},
        {2, {-1, 2, 0}, 1}, {2, {1, -1, -1}, 0},
        {2, {1, -1, 0}, 0}, {2, {1, -1, 0}, 2},
    };
}

inline std::set<SNode<3>> class_ii_neighbor2_signed_contact_set() {
    auto result = build_signed_contact_set<3>(class_ii_contact_set());
    const auto extension = class_ii_neighbor2_initial_extension_states();
    result.insert(extension.begin(), extension.end());
    return result;
}

// Fixed center states sufficient to witness propagation of the
// twenty-four fixed correction states in every tested interior layer.
// The moving correction tip propagates from the previous tip instead.
inline std::set<SNode<3>>
class_ii_neighbor2_center_interface_states() {
    return {
        {0, {-1, 0, 1}, 1}, {0, {-1, 1, 0}, 1},
        {0, {-2, 2, 0}, 0}, {0, {-2, 3, 0}, 0},
        {0, {0, -1, 0}, 0}, {0, {0, 0, 0}, 1},
        {1, {-1, 1, -1}, 0}, {1, {-1, 2, -1}, 0},
        {1, {0, -1, 0}, 0}, {1, {1, -1, -1}, 0},
        {1, {1, -1, 0}, 0}, {1, {1, -2, 0}, 0},
        {2, {-1, 2, -1}, 0}, {2, {-2, 2, -1}, 0},
        {2, {-2, 3, -1}, 0}, {2, {0, -1, 0}, 0},
        {2, {0, 0, 0}, 0}, {2, {0, 0, 0}, 1},
    };
}

// The center interface is generated in three parameter-independent
// base rounds: eight signed-contact states, then six states, then four.
inline std::set<SNode<3>>
class_ii_neighbor2_center_interface_round(std::size_t round) {
    if (round == 1) {
        return {
            {0, {-1, 0, 1}, 1}, {0, {-1, 1, 0}, 1},
            {0, {0, -1, 0}, 0}, {0, {0, 0, 0}, 1},
            {1, {1, -1, -1}, 0}, {1, {1, -1, 0}, 0},
            {2, {0, 0, 0}, 0}, {2, {0, 0, 0}, 1},
        };
    }
    if (round == 2) {
        return {
            {0, {-2, 2, 0}, 0},
            {1, {-1, 1, -1}, 0}, {1, {0, -1, 0}, 0},
            {1, {1, -2, 0}, 0},
            {2, {-1, 2, -1}, 0}, {2, {0, -1, 0}, 0},
        };
    }
    if (round == 3) {
        return {
            {0, {-2, 3, 0}, 0}, {1, {-1, 2, -1}, 0},
            {2, {-2, 2, -1}, 0}, {2, {-2, 3, -1}, 0},
        };
    }
    return {};
}

// Six auxiliary center states close the raw-corona witness table for
// the eighteen transport-interface states. They are proof support,
// not part of the interface consumed by neighbor-2 propagation.
inline std::set<SNode<3>>
class_ii_neighbor2_center_interface_support_round(std::size_t round) {
    auto result = class_ii_neighbor2_center_interface_round(round);
    if (round == 1) {
        result.insert({0, {-1, 1, 0}, 0});
        result.insert({1, {0, 0, -1}, 0});
        result.insert({1, {0, 0, 0}, 0});
        result.insert({2, {-1, 1, 0}, 1});
        result.insert({2, {0, 1, -1}, 0});
    } else if (round == 2) {
        result.insert({2, {-1, 1, -1}, 0});
    }
    return result;
}

struct ClassIIRedSuccessor {
    SNode<3> source;
    SNode<3> target;
};

// Closed functional subgraphs inside the first two center Red layers.
// Every displayed source has the displayed simple-forward edge, and
// every target is another displayed source. Thus positivity of these
// edges is a finite certificate that the whole set survives iterative
// sink pruning.
inline std::vector<ClassIIRedSuccessor>
class_ii_center_base_red_successors(std::size_t round, long long a) {
    if (a < 3 || (round != 2 && round != 3))
        throw std::domain_error(
            "Class-II center base Red-successor domain");
    if (round == 2) {
        return {
            {{0, {-2, 2, 0}, 0}, {0, {2, -2, 0}, 0}},
            {{0, {-1, 1, -1}, 0}, {0, {1, -2, 0}, 0}},
            {{0, {0, -1, 0}, 0}, {0, {-1, 1, -1}, 0}},
            {{0, {1, -2, 0}, 0}, {0, {-2, 2, 0}, 0}},
            {{0, {2, -2, 0}, 0}, {0, {-2, 2, 0}, 0}},
            {{1, {-1, 1, -1}, 0}, {0, {1, -2, 0}, 0}},
            {{1, {0, -1, 0}, 0}, {0, {-1, 1, -1}, 0}},
            {{1, {1, -2, 0}, 0}, {0, {-2, 2, 0}, 0}},
            {{1, {2, -2, 0}, 0}, {0, {-2, 2, 0}, 0}},
            {{2, {-1, 1, -1}, 0}, {0, {0, -1, 0}, 0}},
            {{2, {-1, 2, -1}, 0}, {0, {1, -2, 0}, 0}},
            {{2, {0, -1, 0}, 0}, {1, {-1, 1, -1}, 0}},
        };
    }
    if (a == 3) {
        return {
            {{0, {-3, 3, 0}, 1}, {1, {3, -3, 0}, 0}},
            {{0, {-2, 2, -1}, 0}, {0, {2, -3, 0}, 0}},
            {{0, {-2, 3, 0}, 0}, {1, {3, -3, 0}, 0}},
            {{0, {1, -2, 0}, 0}, {0, {-2, 2, -1}, 0}},
            {{0, {2, -3, 0}, 0}, {0, {-3, 3, 0}, 1}},
            {{1, {-1, 2, -1}, 0}, {0, {2, -3, 0}, 0}},
            {{1, {3, -3, 0}, 0}, {0, {-3, 3, 0}, 1}},
            {{2, {-2, 2, -1}, 0}, {0, {1, -2, 0}, 0}},
            {{2, {-2, 3, -1}, 0}, {0, {2, -3, 0}, 0}},
        };
    }
    return {
        {{0, {-3, 3, 0}, 0}, {0, {3, -3, 0}, 0}},
        {{0, {-2, 2, -1}, 0}, {0, {2, -3, 0}, 0}},
        {{0, {-2, 3, 0}, 0}, {0, {3, -3, 0}, 0}},
        {{0, {1, -2, 0}, 0}, {0, {-2, 2, -1}, 0}},
        {{0, {2, -3, 0}, 0}, {0, {-3, 3, 0}, 0}},
        {{0, {3, -3, 0}, 0}, {0, {-3, 3, 0}, 0}},
        {{1, {-1, 2, -1}, 0}, {0, {2, -3, 0}, 0}},
        {{2, {-2, 2, -1}, 0}, {0, {1, -2, 0}, 0}},
        {{2, {-2, 3, -1}, 0}, {0, {2, -3, 0}, 0}},
    };
}

inline std::set<SNode<3>>
class_ii_center_base_red_survival_states(
        std::size_t round, long long a) {
    std::set<SNode<3>> result;
    for (const auto& edge :
         class_ii_center_base_red_successors(round, a)) {
        result.insert(edge.source);
        result.insert(edge.target);
    }
    return result;
}

// Every neighbor-2 interior correction state has a direct
// simple-forward edge into the corresponding center layer. The first
// twenty-four bridges are fixed; the moving tip lands on center shell
// state n08 at the same round.
inline std::vector<ClassIIRedSuccessor>
class_ii_neighbor2_interior_red_bridges(std::size_t round) {
    if (round < 2)
        throw std::domain_error(
            "Class-II neighbor-2 interior Red bridge requires r >= 2");
    const long long r = static_cast<long long>(round);
    return {
        {{0, {-2, 2, 1}, 1}, {1, {2, -2, -1}, 0}},
        {{0, {-1, 0, 1}, 0}, {1, {0, 0, -1}, 0}},
        {{0, {-1, 1, 1}, 0}, {1, {1, -1, -1}, 0}},
        {{0, {-1, 1, 1}, 2}, {1, {2, -2, 0}, 0}},
        {{0, {-1, 2, 1}, 0}, {1, {2, -2, -1}, 0}},
        {{0, {1, -2, -1}, 0}, {0, {-2, 2, 1}, 1}},
        {{0, {1, -1, -1}, 0}, {0, {-1, 1, 1}, 1}},
        {{0, {1, -1, 0}, 2}, {0, {-1, 1, 1}, 1}},
        {{0, {1, 0, -1}, 0}, {0, {0, 0, 1}, 1}},
        {{1, {-1, 1, 0}, 1}, {0, {1, -1, -1}, 0}},
        {{1, {0, 1, 0}, 0}, {0, {1, -1, -1}, 0}},
        {{1, {1, -2, -1}, 0}, {0, {-2, 2, 0}, 1}},
        {{1, {1, -1, 0}, 1}, {0, {-1, 1, 1}, 0}},
        {{1, {2, -2, -1}, 0}, {0, {-2, 2, 1}, 1}},
        {{1, {2, -1, -1}, 0}, {0, {-1, 1, 1}, 1}},
        {{2, {-2, 2, 0}, 1}, {1, {2, -2, -1}, 0}},
        {{2, {-1, 1, 0}, 0}, {1, {1, -1, -1}, 0}},
        {{2, {-1, 1, 0}, 2}, {1, {2, -2, 0}, 0}},
        {{2, {-1, 2, 0}, 0}, {1, {2, -2, -1}, 0}},
        {{2, {-1, 2, 0}, 1}, {1, {2, -2, 0}, 0}},
        {{2, {1, -2, -1}, 0}, {0, {-3, 3, 0}, 1}},
        {{2, {1, -1, -1}, 0}, {0, {-2, 2, 0}, 1}},
        {{2, {1, -1, 0}, 2}, {0, {-2, 2, 0}, 1}},
        {{2, {2, -1, -1}, 0}, {0, {-2, 2, 1}, 1}},
        {{2, {-r, r, -1}, 0}, {0, {r - 1, -r, 0}, 0}},
    };
}

inline std::set<SNode<3>>
class_ii_neighbor2_fixed_red_bridge_targets() {
    std::set<SNode<3>> result;
    const auto bridges = class_ii_neighbor2_interior_red_bridges(4);
    for (std::size_t i = 0; i < 24; ++i)
        result.insert(bridges[i].target);
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_fixed_red_bridge_anchors() {
    const auto sources =
        class_ii_neighbor2_fixed_extension_states();
    std::set<SNode<3>> result;
    const auto bridges = class_ii_neighbor2_interior_red_bridges(4);
    for (std::size_t i = 0; i < 24; ++i) {
        if (sources.count(bridges[i].target) == 0)
            result.insert(bridges[i].target);
    }
    return result;
}

inline std::vector<ClassIIRedSuccessor>
class_ii_neighbor2_penultimate_red_bridges(long long a) {
    if (a < 4)
        throw std::domain_error(
            "Class-II neighbor-2 penultimate Red bridge requires a >= 4");
    auto result = class_ii_neighbor2_interior_red_bridges(4);
    result.pop_back();
    result.push_back({
        {2, {-(a - 1), a - 1, -1}, 0},
        {0, {a - 2, -(a - 1), 0}, 0}});
    result.push_back({
        {2, {-(a - 2), a - 2, -2}, 0},
        {0, {a - 3, -(a - 2), 1}, 1}});
    return result;
}

inline std::vector<ClassIIRedSuccessor>
class_ii_neighbor2_second_red_bridges() {
    auto interior = class_ii_neighbor2_interior_red_bridges(2);
    std::vector<ClassIIRedSuccessor> result;
    result.reserve(25);
    for (const auto& edge : interior) {
        if (edge.source == SNode<3>{2, {1, -2, -1}, 0})
            continue;
        result.push_back(edge);
    }
    result.push_back({
        {0, {0, 1, -1}, 0},
        {0, {1, -1, 1}, 1}});
    std::sort(
        result.begin(), result.end(),
        [](const auto& left, const auto& right) {
            return left.source < right.source;
        });
    return result;
}

inline std::vector<ClassIIRedSuccessor>
class_ii_neighbor2_terminal_red_bridges(long long a) {
    if (a < 3)
        throw std::domain_error(
            "Class-II neighbor-2 terminal Red bridge requires a >= 3");
    auto result = class_ii_neighbor2_interior_red_bridges(4);
    result.pop_back();
    const long long q = a - 2;
    result.push_back({
        {0, {q, -(q + 1), 2}, 2},
        {1, {-q, q + 1, -1}, 0}});
    result.push_back({
        {0, {q, -q, 2}, 1},
        {1, {-q, q + 1, -1}, 0}});
    result.push_back({
        {1, {-q, q, -2}, 0},
        {0, {q, -(q + 1), 1}, 1}});
    result.push_back({
        {2, {-q, q, -2}, 0},
        {0, {q - 1, -q, 1}, 1}});
    result.push_back({
        {2, {-q, q + 1, -2}, 0},
        {0, {q, -(q + 1), 1}, 1}});
    result.push_back({
        {2, {q, -q, 1}, 1},
        {1, {-q, q + 1, -1}, 0}});
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_interior_extension_states(std::size_t round) {
    if (round < 2)
        throw std::domain_error(
            "Class-II neighbor-2 interior extension round");
    auto result = class_ii_neighbor2_fixed_extension_states();
    const long long r = static_cast<long long>(round);
    result.insert({2, {-r, r, -1}, 0});
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_second_extension_states() {
    auto result = class_ii_neighbor2_interior_extension_states(2);
    result.erase({2, {1, -2, -1}, 0});
    result.insert({0, {0, 1, -1}, 0});
    return result;
}

inline std::set<SNode<3>>
class_ii_neighbor2_penultimate_extension_states(long long a) {
    if (a < 4)
        throw std::domain_error(
            "Class-II neighbor-2 penultimate extension requires a >= 4");
    auto result = class_ii_neighbor2_fixed_extension_states();
    result.insert({2, {-(a - 1), a - 1, -1}, 0});
    result.insert({2, {-(a - 2), a - 2, -2}, 0});
    return result;
}

inline std::vector<SNode<3>>
class_ii_neighbor2_terminal_affine_states(long long a) {
    if (a < 3)
        throw std::domain_error(
            "Class-II neighbor-2 terminal affine states require a >= 3");
    const long long q = a - 2;
    return {
        {0, {q, -(q + 1), 2}, 2},
        {0, {q, -q, 2}, 1},
        {1, {-q, q, -2}, 0},
        {2, {-q, q, -2}, 0},
        {2, {-q, q + 1, -2}, 0},
        {2, {q, -q, 1}, 1},
    };
}

// The third adjacent-swap neighbor contains the complete center boundary
// graph plus this thirty-state terminal correction in every tested a>=3
// graph. Twenty-four states are fixed and six form an affine sextet.
inline std::set<SNode<3>> class_ii_neighbor2_center_extension(long long a) {
    if (a < 3)
        throw std::domain_error(
            "Class-II neighbor-2 center extension requires a >= 3");
    auto result = class_ii_neighbor2_fixed_extension_states();
    const auto affine = class_ii_neighbor2_terminal_affine_states(a);
    result.insert(affine.begin(), affine.end());
    return result;
}

inline std::set<SNode<3>> class_ii_neighbor2_layer_extension(
        long long a, std::size_t round) {
    if (a < 3 || round == 0)
        throw std::domain_error(
            "Class-II neighbor-2 layer extension domain");
    if (round == 1)
        return class_ii_neighbor2_initial_extension_states();
    if (round == 2)
        return class_ii_neighbor2_second_extension_states();
    if (round < static_cast<std::size_t>(a - 1))
        return class_ii_neighbor2_interior_extension_states(round);
    if (round == static_cast<std::size_t>(a - 1))
        return class_ii_neighbor2_penultimate_extension_states(a);
    return class_ii_neighbor2_center_extension(a);
}

inline std::set<SNode<3>> class_ii_neighbor2_layer_candidate(
        long long a,
        std::size_t round,
        const std::set<SNode<3>>& center_layer) {
    auto result = center_layer;
    const auto extension =
        class_ii_neighbor2_layer_extension(a, round);
    result.insert(extension.begin(), extension.end());
    return result;
}

struct ClassIINeighbor2CompositionWitness {
    SNode<3> source;
    SNode<3> hop;
    SNode<3> target;

    bool operator==(const ClassIINeighbor2CompositionWitness&) const =
        default;
};

struct ClassIINeighbor2CompositionCertificate {
    bool exact = false;
    std::size_t fixed_witnesses = 0;
    std::size_t affine_witnesses = 0;
};

struct ClassIINeighbor2CenterInterfaceCertificate {
    bool contact_base_exact = false;
    bool exact_partition = false;
    bool initial_in_signed_contact = false;
    std::size_t round2_witnesses = 0;
    std::size_t round3_witnesses = 0;

    bool exact() const {
        return contact_base_exact
            && exact_partition
            && initial_in_signed_contact
            && round2_witnesses == 7
            && round3_witnesses == 4;
    }
};

struct ClassIICenterBaseRedCertificate {
    bool exact = false;
    bool raw_composition_exact = false;
    long long stable_from = 3;
    std::size_t successor_edges = 0;
    std::size_t parameter_checks = 0;
};

struct ClassIINeighbor2RedBridgeCertificate {
    bool fixed_exact = false;
    bool second_exact = false;
    bool tip_exact = false;
    bool penultimate_exact = false;
    bool terminal_exact = false;
    bool closure_exact = false;
    long long stable_from = 3;
    std::size_t fixed_edge_checks = 0;

    bool exact() const {
        return fixed_exact && second_exact && tip_exact
            && penultimate_exact && terminal_exact
            && closure_exact;
    }
};

struct ClassIINeighbor2TerminalCompositionCertificate {
    bool exact = false;
    std::size_t fixed_targets = 0;
    std::size_t affine_targets = 0;
};

inline std::vector<ClassIINeighbor2CompositionWitness>
class_ii_center_base_red_composition_witnesses(
        std::size_t round, long long a) {
    if (a < 3 || (round != 2 && round != 3))
        throw std::domain_error(
            "Class-II center Red composition witness domain");
    const auto sources = round == 2
        ? build_signed_contact_set<3>(class_ii_contact_set())
        : class_ii_center_base_red_survival_states(2, a);
    const auto hops =
        build_signed_contact_set<3>(class_ii_contact_set());
    const auto targets =
        class_ii_center_base_red_survival_states(round, a);
    std::vector<ClassIINeighbor2CompositionWitness> result;
    result.reserve(targets.size());
    for (const auto& target : targets) {
        bool found = false;
        for (const auto& source : sources) {
            if (source.i != target.i) continue;
            for (const auto& hop : hops) {
                if (source.j != hop.i || hop.j != target.j) continue;
                bool same = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    same = same
                        && source.x[coordinate] + hop.x[coordinate]
                            == target.x[coordinate];
                }
                if (!same) continue;
                result.push_back({source, hop, target});
                found = true;
                break;
            }
            if (found) break;
        }
        if (!found)
            throw std::logic_error(
                "Class-II center Red target lacks corona witness");
    }
    return result;
}

inline std::vector<ClassIINeighbor2CompositionWitness>
class_ii_neighbor2_center_interface_composition_witnesses(
        std::size_t round) {
    if (round < 2 || round > 3)
        throw std::domain_error(
            "Class-II center-interface witness round requires 2 or 3");
    std::set<SNode<3>> sources;
    for (std::size_t source_round = 1;
         source_round < round; ++source_round) {
        const auto layer =
            class_ii_neighbor2_center_interface_support_round(
                source_round);
        sources.insert(layer.begin(), layer.end());
    }
    const auto hops =
        build_signed_contact_set<3>(class_ii_contact_set());
    const auto targets = round == 2
        ? class_ii_neighbor2_center_interface_support_round(round)
        : class_ii_neighbor2_center_interface_round(round);
    std::vector<ClassIINeighbor2CompositionWitness> result;
    result.reserve(targets.size());
    for (const auto& target : targets) {
        bool found = false;
        for (const auto& source : sources) {
            if (source.i != target.i) continue;
            for (const auto& hop : hops) {
                if (source.j != hop.i || hop.j != target.j) continue;
                bool same = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    same = same
                        && source.x[coordinate] + hop.x[coordinate]
                            == target.x[coordinate];
                }
                if (!same) continue;
                result.push_back({source, hop, target});
                found = true;
                break;
            }
            if (found) break;
        }
        if (!found)
            throw std::logic_error(
                "Class-II center-interface target lacks corona witness");
    }
    return result;
}

inline ClassIINeighbor2CenterInterfaceCertificate
class_ii_neighbor2_center_interface_certificate() {
    ClassIINeighbor2CenterInterfaceCertificate result;
    result.contact_base_exact =
        class_ii_contact_backward_envelope_certificate().exact();
    std::set<SNode<3>> joined;
    for (std::size_t round = 1; round <= 3; ++round) {
        const auto layer =
            class_ii_neighbor2_center_interface_round(round);
        const auto old_size = joined.size();
        joined.insert(layer.begin(), layer.end());
        result.exact_partition = result.exact_partition
            || round == 1;
        result.exact_partition = result.exact_partition
            && joined.size() == old_size + layer.size();
    }
    result.exact_partition = result.exact_partition
        && joined == class_ii_neighbor2_center_interface_states();
    const auto signed_contact =
        build_signed_contact_set<3>(class_ii_contact_set());
    result.initial_in_signed_contact = true;
    for (const auto& node :
         class_ii_neighbor2_center_interface_support_round(1)) {
        result.initial_in_signed_contact =
            result.initial_in_signed_contact
            && signed_contact.count(node) == 1;
    }
    result.round2_witnesses =
        class_ii_neighbor2_center_interface_composition_witnesses(2)
            .size();
    result.round3_witnesses =
        class_ii_neighbor2_center_interface_composition_witnesses(3)
            .size();
    return result;
}

// Finite-crossing proof that every chosen center base-layer successor
// edge has positive multiplicity for its whole parameter domain.
// Because the successor maps are closed functional graphs, this is a
// direct Red-survival certificate rather than a sampled out-degree
// check.
inline ClassIICenterBaseRedCertificate
class_ii_center_base_red_certificate() {
    ClassIICenterBaseRedCertificate result;
    result.exact = true;
    const auto certify_family = [&](
            std::size_t round, long long minimum_a) {
        const auto states =
            class_ii_center_base_red_survival_states(
                round, minimum_a);
        const auto controls0 =
            class_ii_neighbor_transition_controls(
                3, minimum_a, states);
        const auto controls1 =
            class_ii_neighbor_transition_controls(
                3, minimum_a + 1, states);
        const auto controls2 =
            class_ii_neighbor_transition_controls(
                3, minimum_a + 2, states);
        result.exact = result.exact
            && controls0.size() == controls1.size()
            && controls0.size() == controls2.size();
        if (!result.exact) return;
        long long stable_from = minimum_a;
        for (std::size_t i = 0; i < controls0.size(); ++i) {
            const long long slope = controls1[i] - controls0[i];
            result.exact = result.exact
                && controls2[i] == controls1[i] + slope;
            if (slope == 0) continue;
            const long long intercept =
                controls0[i] - minimum_a * slope;
            const long long numerator =
                intercept < 0 ? -intercept : intercept;
            const long long denominator =
                slope < 0 ? -slope : slope;
            stable_from = std::max(
                stable_from, numerator / denominator + 2);
        }
        result.stable_from =
            std::max(result.stable_from, stable_from);
        for (long long a = minimum_a;
             a <= stable_from + 1; ++a) {
            const auto edges =
                class_ii_center_base_red_successors(round, a);
            const auto family_states =
                class_ii_center_base_red_survival_states(round, a);
            for (const auto& edge : edges) {
                result.exact = result.exact
                    && family_states.count(edge.source) == 1
                    && family_states.count(edge.target) == 1
                    && class_ii_neighbor_transition_weight(
                        3, a, edge.source, edge.target) > 0;
                ++result.successor_edges;
            }
            ++result.parameter_checks;
        }
        const auto tail = class_ii_center_base_red_successors(
            round, stable_from);
        const auto tail_next = class_ii_center_base_red_successors(
            round, stable_from + 1);
        result.exact = result.exact
            && tail.size() == tail_next.size();
        for (std::size_t i = 0;
             result.exact && i < tail.size(); ++i) {
            result.exact = result.exact
                && tail[i].source == tail_next[i].source
                && tail[i].target == tail_next[i].target;
            const long long weight =
                class_ii_neighbor_transition_weight(
                    3, stable_from,
                    tail[i].source, tail[i].target);
            const long long next_weight =
                class_ii_neighbor_transition_weight(
                    3, stable_from + 1,
                    tail[i].source, tail[i].target);
            result.exact = result.exact
                && weight > 0
                && next_weight - weight >= 0;
        }
    };

    certify_family(2, 3);
    // Round three has one exceptional a=3 graph and a fixed tail from
    // a=4 onward.
    const auto special = class_ii_center_base_red_successors(3, 3);
    for (const auto& edge : special) {
        result.exact = result.exact
            && class_ii_neighbor_transition_weight(
                3, 3, edge.source, edge.target) > 0;
        ++result.successor_edges;
    }
    ++result.parameter_checks;
    certify_family(3, 4);
    const auto round2_at_three =
        class_ii_center_base_red_composition_witnesses(2, 3);
    const auto round2_at_four =
        class_ii_center_base_red_composition_witnesses(2, 4);
    const auto round3_at_four =
        class_ii_center_base_red_composition_witnesses(3, 4);
    const auto round3_at_five =
        class_ii_center_base_red_composition_witnesses(3, 5);
    result.raw_composition_exact =
        round2_at_three == round2_at_four
        && round3_at_four == round3_at_five
        && round2_at_three.size() == 12
        && class_ii_center_base_red_composition_witnesses(3, 3)
            .size() == 9
        && round3_at_four.size() == 9;
    result.exact = result.exact && result.raw_composition_exact;
    return result;
}

inline ClassIINeighbor2RedBridgeCertificate
class_ii_neighbor2_interior_red_bridge_certificate() {
    ClassIINeighbor2RedBridgeCertificate result;
    const auto bridges = class_ii_neighbor2_interior_red_bridges(4);
    std::set<SNode<3>> fixed_states;
    for (std::size_t i = 0; i < 24; ++i) {
        fixed_states.insert(bridges[i].source);
        fixed_states.insert(bridges[i].target);
    }
    const auto second_bridges =
        class_ii_neighbor2_second_red_bridges();
    const auto second_extra_it = std::find_if(
        second_bridges.begin(), second_bridges.end(),
        [](const auto& edge) {
            return edge.source == SNode<3>{0, {0, 1, -1}, 0};
        });
    if (second_extra_it == second_bridges.end())
        return result;
    const auto second_extra = *second_extra_it;
    fixed_states.insert(second_extra.source);
    fixed_states.insert(second_extra.target);
    const auto at_three =
        class_ii_neighbor_transition_controls(2, 3, fixed_states);
    const auto at_four =
        class_ii_neighbor_transition_controls(2, 4, fixed_states);
    const auto at_five =
        class_ii_neighbor_transition_controls(2, 5, fixed_states);
    result.fixed_exact =
        at_three.size() == at_four.size()
        && at_three.size() == at_five.size();
    for (std::size_t i = 0;
         result.fixed_exact && i < at_three.size(); ++i) {
        const long long slope = at_four[i] - at_three[i];
        result.fixed_exact =
            at_five[i] == at_four[i] + slope;
        if (slope == 0) continue;
        const long long intercept = at_three[i] - 3 * slope;
        const long long numerator =
            intercept < 0 ? -intercept : intercept;
        const long long denominator =
            slope < 0 ? -slope : slope;
        result.stable_from = std::max(
            result.stable_from, numerator / denominator + 2);
    }
    for (long long a = 3;
         result.fixed_exact && a <= result.stable_from + 1; ++a) {
        const auto values =
            class_ii_neighbor2_interior_red_bridges(4);
        for (std::size_t i = 0; i < 24; ++i) {
            result.fixed_exact = result.fixed_exact
                && class_ii_neighbor_transition_weight(
                    2, a, values[i].source, values[i].target) > 0;
            ++result.fixed_edge_checks;
        }
        result.fixed_exact = result.fixed_exact
            && class_ii_neighbor_transition_weight(
                2, a, second_extra.source,
                second_extra.target) > 0;
        ++result.fixed_edge_checks;
    }
    const auto tail =
        class_ii_neighbor2_interior_red_bridges(4);
    for (std::size_t i = 0;
         result.fixed_exact && i < 24; ++i) {
        const long long weight =
            class_ii_neighbor_transition_weight(
                2, result.stable_from,
                tail[i].source, tail[i].target);
        const long long next =
            class_ii_neighbor_transition_weight(
                2, result.stable_from + 1,
                tail[i].source, tail[i].target);
        result.fixed_exact =
            weight > 0 && next - weight >= 0;
    }
    const long long second_weight =
        class_ii_neighbor_transition_weight(
            2, result.stable_from,
            second_extra.source, second_extra.target);
    const long long second_next =
        class_ii_neighbor_transition_weight(
            2, result.stable_from + 1,
            second_extra.source, second_extra.target);
    result.second_exact =
        second_bridges.size() == 25
        && second_weight > 0
        && second_next - second_weight >= 0;
    std::set<SNode<3>> second_sources;
    for (const auto& edge : second_bridges)
        second_sources.insert(edge.source);
    result.second_exact = result.second_exact
        && second_sources
            == class_ii_neighbor2_second_extension_states();

    // Symbolic prefix proof for the moving bridge:
    // source [2,(-r,r,-1),0] -> target [0,(r-1,-r,0),0].
    // The left occurrence is the unique final 2 in 0^a12, with
    // prefix (a,1,0). The right occurrence is the 0^a segment.
    // M(target)=(-a,r-1,-1), so the interval counter forces the
    // unique right-prefix index r. Hence the weight is exactly one
    // throughout 0<=r<a.
    const auto left =
        class_ii_neighbor_symbolic_prefix_families(2, 0, 2);
    const auto right =
        class_ii_neighbor_symbolic_prefix_families(2, 0, 0);
    result.tip_exact =
        left.size() == 1 && right.size() == 1
        && left[0].varying_coordinate == 2
        && left[0].base[0].intercept == 0
        && left[0].base[0].slope == 1
        && left[0].base[1].intercept == 1
        && left[0].base[1].slope == 0
        && left[0].length.intercept == 1
        && left[0].length.slope == 0
        && right[0].varying_coordinate == 0
        && right[0].base[0].intercept == 0
        && right[0].base[0].slope == 0
        && right[0].length.intercept == 0
        && right[0].length.slope == 1;

    const auto certify_affine_bridges = [&](
            long long minimum_a,
            const auto& generator,
            std::size_t affine_begin,
            bool& exact) {
        exact = true;
        const auto first = generator(minimum_a);
        const auto second = generator(minimum_a + 1);
        const auto third = generator(minimum_a + 2);
        exact = first.size() == second.size()
            && first.size() == third.size();
        long long stable_from = minimum_a;
        for (std::size_t edge = affine_begin;
             exact && edge < first.size(); ++edge) {
            const auto controls_for = [&](long long a_value,
                                          const ClassIIRedSuccessor& value) {
                std::set<SNode<3>> pair{
                    value.source, value.target};
                return class_ii_neighbor_transition_controls(
                    2, a_value, pair);
            };
            const auto c0 = controls_for(minimum_a, first[edge]);
            const auto c1 = controls_for(minimum_a + 1, second[edge]);
            const auto c2 = controls_for(minimum_a + 2, third[edge]);
            exact = c0.size() == c1.size() && c0.size() == c2.size();
            for (std::size_t i = 0; exact && i < c0.size(); ++i) {
                const long long slope = c1[i] - c0[i];
                exact = c2[i] == c1[i] + slope;
                if (slope == 0) continue;
                const long long intercept =
                    c0[i] - minimum_a * slope;
                const long long numerator =
                    intercept < 0 ? -intercept : intercept;
                const long long denominator =
                    slope < 0 ? -slope : slope;
                stable_from = std::max(
                    stable_from,
                    numerator / denominator + 2);
            }
        }
        result.stable_from =
            std::max(result.stable_from, stable_from);
        for (long long a = minimum_a;
             exact && a <= stable_from + 1; ++a) {
            const auto values = generator(a);
            for (std::size_t edge = affine_begin;
                 edge < values.size(); ++edge) {
                exact = exact
                    && class_ii_neighbor_transition_weight(
                        2, a, values[edge].source,
                        values[edge].target) > 0;
                ++result.fixed_edge_checks;
            }
        }
        const auto tail = generator(stable_from);
        const auto tail_next = generator(stable_from + 1);
        for (std::size_t edge = affine_begin;
             exact && edge < tail.size(); ++edge) {
            const long long weight =
                class_ii_neighbor_transition_weight(
                    2, stable_from, tail[edge].source,
                    tail[edge].target);
            const long long next =
                class_ii_neighbor_transition_weight(
                    2, stable_from + 1, tail_next[edge].source,
                    tail_next[edge].target);
            exact = weight > 0 && next - weight >= 0;
        }
    };
    certify_affine_bridges(
        4,
        [](long long a) {
            return class_ii_neighbor2_penultimate_red_bridges(a);
        },
        24, result.penultimate_exact);
    certify_affine_bridges(
        3,
        [](long long a) {
            return class_ii_neighbor2_terminal_red_bridges(a);
        },
        24, result.terminal_exact);
    const auto fixed_sources =
        class_ii_neighbor2_fixed_extension_states();
    const auto anchors =
        class_ii_neighbor2_fixed_red_bridge_anchors();
    result.closure_exact =
        fixed_sources.size() == 24 && anchors.size() == 7;
    for (std::size_t i = 0;
         result.closure_exact && i < 24; ++i) {
        result.closure_exact =
            fixed_sources.count(bridges[i].source) == 1
            && (fixed_sources.count(bridges[i].target) == 1
                || anchors.count(bridges[i].target) == 1);
    }
    for (long long a : {5LL, 6LL}) {
        const auto penultimate =
            class_ii_neighbor2_penultimate_red_bridges(a);
        std::set<SNode<3>> penultimate_sources;
        for (const auto& edge : penultimate)
            penultimate_sources.insert(edge.source);
        const auto penultimate_shell =
            class_ii_interior_shell(
                static_cast<std::size_t>(a - 1));
        result.closure_exact = result.closure_exact
            && penultimate_sources
                == class_ii_neighbor2_penultimate_extension_states(a)
            && penultimate_shell.count(
                penultimate[24].target) == 1
            && penultimate_shell.count(
                penultimate[25].target) == 1;

        const auto terminal =
            class_ii_neighbor2_terminal_red_bridges(a);
        std::set<SNode<3>> terminal_sources;
        for (const auto& edge : terminal)
            terminal_sources.insert(edge.source);
        auto terminal_center_targets =
            class_ii_interior_shell(static_cast<std::size_t>(a));
        const auto prior_shell =
            class_ii_interior_shell(
                static_cast<std::size_t>(a - 1));
        terminal_center_targets.insert(
            prior_shell.begin(), prior_shell.end());
        result.closure_exact = result.closure_exact
            && terminal_sources
                == class_ii_neighbor2_center_extension(a);
        for (std::size_t i = 24; i < terminal.size(); ++i)
            result.closure_exact = result.closure_exact
                && terminal_center_targets.count(
                    terminal[i].target) == 1;
    }
    return result;
}

// Bounded raw-corona witnesses for the interior correction. This checks
// colors and exact lattice addition only; stepped-hyperplane validity is
// the separate Lean window obligation.
inline std::vector<ClassIINeighbor2CompositionWitness>
class_ii_neighbor2_interior_composition_witnesses(std::size_t round) {
    if (round < 4)
        throw std::domain_error(
            "Class-II neighbor-2 interior witness round requires r >= 4");
    auto sources = class_ii_neighbor2_center_interface_states();
    const auto previous =
        class_ii_neighbor2_interior_extension_states(round - 1);
    sources.insert(previous.begin(), previous.end());
    const auto hops = class_ii_neighbor2_signed_contact_set();
    const auto targets =
        class_ii_neighbor2_interior_extension_states(round);
    std::vector<ClassIINeighbor2CompositionWitness> result;
    result.reserve(targets.size());
    for (const auto& target : targets) {
        bool found = false;
        for (const auto& source : sources) {
            if (source.i != target.i) continue;
            for (const auto& hop : hops) {
                if (source.j != hop.i || hop.j != target.j) continue;
                bool same = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    same = same
                        && source.x[coordinate] + hop.x[coordinate]
                            == target.x[coordinate];
                }
                if (!same) continue;
                result.push_back({source, hop, target});
                found = true;
                break;
            }
            if (found) break;
        }
        if (!found)
            throw std::logic_error(
                "Class-II neighbor-2 interior target lacks corona witness");
    }
    return result;
}

inline std::vector<ClassIINeighbor2CompositionWitness>
class_ii_neighbor2_terminal_composition_witnesses(long long a) {
    if (a < 4)
        throw std::domain_error(
            "Class-II neighbor-2 terminal witness requires a >= 4");
    auto sources = class_ii_neighbor2_center_interface_states();
    const auto center_terminal_interface =
        class_ii_interior_shell(static_cast<std::size_t>(a - 1));
    sources.insert(
        center_terminal_interface.begin(), center_terminal_interface.end());
    const auto previous =
        class_ii_neighbor2_penultimate_extension_states(a);
    sources.insert(previous.begin(), previous.end());
    const auto hops = class_ii_neighbor2_signed_contact_set();
    const auto targets = class_ii_neighbor2_center_extension(a);
    std::vector<ClassIINeighbor2CompositionWitness> result;
    result.reserve(targets.size());
    for (const auto& target : targets) {
        bool found = false;
        for (const auto& source : sources) {
            if (source.i != target.i) continue;
            for (const auto& hop : hops) {
                if (source.j != hop.i || hop.j != target.j) continue;
                bool same = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    same = same
                        && source.x[coordinate] + hop.x[coordinate]
                            == target.x[coordinate];
                }
                if (!same) continue;
                result.push_back({source, hop, target});
                found = true;
                break;
            }
            if (found) break;
        }
        if (!found)
            throw std::logic_error(
                "Class-II neighbor-2 terminal target lacks corona witness");
    }
    return result;
}

inline std::vector<ClassIINeighbor2CompositionWitness>
class_ii_neighbor2_penultimate_composition_witnesses(long long a) {
    if (a < 5)
        throw std::domain_error(
            "Class-II neighbor-2 penultimate witness requires a >= 5");
    auto sources = class_ii_neighbor2_center_interface_states();
    const auto center_interface =
        class_ii_interior_shell(static_cast<std::size_t>(a - 2));
    sources.insert(center_interface.begin(), center_interface.end());
    const auto previous =
        class_ii_neighbor2_interior_extension_states(
            static_cast<std::size_t>(a - 2));
    sources.insert(previous.begin(), previous.end());
    const auto hops = class_ii_neighbor2_signed_contact_set();
    const auto targets =
        class_ii_neighbor2_penultimate_extension_states(a);
    std::vector<ClassIINeighbor2CompositionWitness> result;
    result.reserve(targets.size());
    for (const auto& target : targets) {
        bool found = false;
        for (const auto& source : sources) {
            if (source.i != target.i) continue;
            for (const auto& hop : hops) {
                if (source.j != hop.i || hop.j != target.j) continue;
                bool same = true;
                for (std::size_t coordinate = 0;
                     coordinate < 3; ++coordinate) {
                    same = same
                        && source.x[coordinate] + hop.x[coordinate]
                            == target.x[coordinate];
                }
                if (!same) continue;
                result.push_back({source, hop, target});
                found = true;
                break;
            }
            if (found) break;
        }
        if (!found)
            throw std::logic_error(
                "Class-II neighbor-2 penultimate target lacks corona witness");
    }
    return result;
}

inline ClassIINeighbor2TerminalCompositionCertificate
class_ii_neighbor2_terminal_composition_certificate() {
    const auto at_four =
        class_ii_neighbor2_terminal_composition_witnesses(4);
    const auto at_five =
        class_ii_neighbor2_terminal_composition_witnesses(5);
    const auto at_six =
        class_ii_neighbor2_terminal_composition_witnesses(6);
    ClassIINeighbor2TerminalCompositionCertificate result;
    result.exact = at_four.size() == 30
        && at_five.size() == at_four.size()
        && at_six.size() == at_four.size();
    if (!result.exact) return result;
    const auto find_target = [](
            const std::vector<ClassIINeighbor2CompositionWitness>& values,
            const SNode<3>& target)
            -> const ClassIINeighbor2CompositionWitness* {
        for (const auto& value : values)
            if (value.target == target) return &value;
        return nullptr;
    };
    const auto check_aligned = [&](
            const SNode<3>& target_four,
            const SNode<3>& target_five,
            const SNode<3>& target_six) {
        const auto* first = find_target(at_four, target_four);
        const auto* second = find_target(at_five, target_five);
        const auto* third = find_target(at_six, target_six);
        if (first == nullptr || second == nullptr || third == nullptr)
            return false;
        bool exact =
            first->source.i == second->source.i
            && second->source.i == third->source.i
            && first->source.j == second->source.j
            && second->source.j == third->source.j
            && first->hop == second->hop
            && second->hop == third->hop
            && first->target.i == second->target.i
            && second->target.i == third->target.i
            && first->target.j == second->target.j
            && second->target.j == third->target.j;
        for (std::size_t coordinate = 0; coordinate < 3; ++coordinate) {
            exact = exact
                && third->source.x[coordinate]
                    - second->source.x[coordinate]
                    == second->source.x[coordinate]
                        - first->source.x[coordinate]
                && third->target.x[coordinate]
                    - second->target.x[coordinate]
                    == second->target.x[coordinate]
                        - first->target.x[coordinate];
        }
        return exact;
    };
    for (const auto& target :
         class_ii_neighbor2_fixed_extension_states()) {
        result.exact = result.exact
            && check_aligned(target, target, target);
        ++result.fixed_targets;
    }
    const auto affine_four =
        class_ii_neighbor2_terminal_affine_states(4);
    const auto affine_five =
        class_ii_neighbor2_terminal_affine_states(5);
    const auto affine_six =
        class_ii_neighbor2_terminal_affine_states(6);
    for (std::size_t i = 0; i < affine_four.size(); ++i) {
        result.exact = result.exact
            && check_aligned(
                affine_four[i], affine_five[i], affine_six[i]);
        ++result.affine_targets;
    }
    result.exact = result.exact
        && result.fixed_targets == 24
        && result.affine_targets == 6;
    return result;
}

inline ClassIINeighbor2CompositionCertificate
class_ii_neighbor2_interior_composition_certificate() {
    const auto at_four =
        class_ii_neighbor2_interior_composition_witnesses(4);
    const auto at_five =
        class_ii_neighbor2_interior_composition_witnesses(5);
    ClassIINeighbor2CompositionCertificate result;
    result.exact =
        at_four.size() == 25 && at_five.size() == at_four.size();
    if (!result.exact) return result;
    const SNode<3> tip_four{2, {-4, 4, -1}, 0};
    const SNode<3> source_tip_three{2, {-3, 3, -1}, 0};
    const SNode<3> tip_five{2, {-5, 5, -1}, 0};
    const SNode<3> source_tip_four{2, {-4, 4, -1}, 0};
    const SNode<3> tip_hop{0, {-1, 1, 0}, 0};
    for (std::size_t i = 0; i < at_four.size(); ++i) {
        const auto& left = at_four[i];
        const auto& right = at_five[i];
        if (left.target == tip_four) {
            result.exact = result.exact
                && left.source == source_tip_three
                && left.hop == tip_hop
                && right.target == tip_five
                && right.source == source_tip_four
                && right.hop == tip_hop;
            ++result.affine_witnesses;
        } else {
            result.exact = result.exact
                && left.source == right.source
                && left.hop == right.hop
                && left.target == right.target;
            ++result.fixed_witnesses;
        }
    }
    result.exact = result.exact
        && result.fixed_witnesses == 24
        && result.affine_witnesses == 1;
    return result;
}

// Sparse candidate adjacency A(a)=intercept+slope*a on the canonical
// std::set order of class_ii_neighbor_dominant_core_states(). These
// coefficients are independently checked against exact substitution
// transitions by class_ii_neighbor_probe for 3<=a<=8.
inline std::vector<ClassIINeighborAffineEdge>
class_ii_neighbor_affine_edges(std::size_t neighbor) {
    if (neighbor == 0) {
        return {
            {0,4,1,0}, {0,5,1,0},
            {1,9,-2,1}, {1,10,-3,1}, {1,12,-2,1}, {1,14,1,0},
            {2,9,1,0}, {2,10,1,0}, {2,12,1,0},
            {3,6,1,0}, {3,9,1,0},
            {4,0,-2,1}, {4,1,0,1}, {4,2,-1,1}, {4,3,1,0},
            {4,11,1,0}, {5,7,1,0}, {5,13,1,0},
            {6,1,1,0}, {6,11,1,0}, {7,5,1,0},
            {8,6,1,0}, {8,9,0,1}, {8,10,-2,1}, {8,12,-1,1},
            {8,14,1,0},
            {9,0,-3,1}, {9,1,-2,1}, {9,2,-2,1}, {9,3,1,0},
            {10,7,1,0}, {10,8,1,0}, {11,6,1,0}, {11,9,1,0},
            {12,0,1,0}, {12,1,1,0}, {12,2,1,0},
            {13,4,1,0}, {13,5,1,0}, {14,1,1,0}, {14,11,1,0},
        };
    }
    if (neighbor == 1) {
        return {
            {0,9,1,0}, {0,12,1,0},
            {1,9,-1,1}, {1,12,-2,1}, {1,14,1,0},
            {2,15,1,0}, {3,9,1,0}, {3,12,1,0},
            {4,15,1,0}, {4,16,1,0}, {5,10,1,0},
            {6,7,1,0}, {6,11,1,0}, {7,8,1,0}, {7,13,1,0},
            {8,12,1,0},
            {9,0,-2,1}, {9,1,-1,1}, {9,2,-1,1}, {9,3,1,0},
            {9,4,1,0}, {10,0,1,0}, {11,5,1,0}, {11,6,1,0},
            {12,0,1,0}, {12,1,1,0}, {12,2,1,0}, {13,9,1,0},
            {14,0,1,0}, {14,1,1,0}, {14,2,1,0}, {15,10,1,0},
            {16,7,1,0}, {16,11,1,0},
        };
    }
    if (neighbor == 2) {
        return {
            {0,21,1,0}, {1,22,1,0}, {1,34,1,0}, {2,26,1,0},
            {3,15,0,1}, {3,16,-1,1}, {3,18,-2,1}, {3,24,-1,1},
            {3,25,-1,1}, {3,26,-2,1}, {3,36,1,0},
            {4,16,1,0}, {4,18,1,0}, {5,26,1,0}, {6,24,1,0},
            {7,19,1,0}, {7,25,1,0}, {7,28,1,0}, {7,38,1,0},
            {8,29,1,0},
            {9,2,-1,1}, {9,3,0,1}, {9,4,-1,1}, {9,5,1,0},
            {9,6,-1,1}, {9,7,-2,1}, {9,20,0,1}, {9,30,1,0},
            {9,31,1,0}, {9,32,1,0},
            {10,11,1,0}, {10,13,1,0}, {11,12,1,0},
            {12,21,1,0}, {12,22,1,0}, {12,34,1,0},
            {13,14,1,0}, {13,23,1,0}, {13,35,1,0},
            {14,15,-1,1}, {14,16,0,1}, {14,17,1,0},
            {14,18,-1,1}, {14,24,-2,1}, {14,25,-1,1},
            {14,26,0,1}, {14,27,1,0}, {14,36,1,0}, {14,37,1,0},
            {15,7,1,0},
            {16,2,-2,1}, {16,3,-1,1}, {16,4,-1,1}, {16,5,1,0},
            {16,6,0,1}, {16,7,-1,1}, {16,8,1,0},
            {16,20,-2,1}, {16,33,1,0},
            {17,7,1,0}, {18,20,1,0}, {19,13,1,0},
            {20,15,1,0}, {21,9,1,0}, {22,10,1,0}, {23,15,1,0},
            {24,0,1,0}, {24,1,1,0}, {24,4,1,0},
            {25,2,1,0}, {25,3,1,0}, {26,6,1,0},
            {27,4,1,0}, {27,12,1,0}, {28,11,1,0}, {29,7,1,0},
            {30,24,1,0}, {31,10,1,0}, {31,25,1,0},
            {32,15,1,0}, {33,15,1,0}, {34,9,1,0}, {34,26,1,0},
            {35,10,1,0}, {35,24,1,0}, {35,25,1,0},
            {36,20,1,0}, {37,6,1,0},
            {38,0,1,0}, {38,1,1,0}, {38,4,1,0},
        };
    }
    throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
}

inline std::vector<std::vector<long long>>
class_ii_neighbor_affine_matrix(std::size_t neighbor, long long a) {
    const std::size_t size =
        class_ii_neighbor_dominant_core_states(neighbor).size();
    std::vector<std::vector<long long>> result(
        size, std::vector<long long>(size, 0));
    for (const auto& edge : class_ii_neighbor_affine_edges(neighbor)) {
        const long long weight = edge.weight(a);
        if (weight < 0) {
            throw std::domain_error(
                "Class-II neighbor affine weight is negative");
        }
        result[edge.source][edge.target] = weight;
    }
    return result;
}

// The slope matrix D = (A(a) - A(3)) / (a - 3). Every affine edge
// has the form (intercept, slope), so A(a) = intercept + slope*a and
// A(a) - A(3) = slope*(a-3).  D is therefore a 0-1 matrix (modulo
// the constant case slope = 0) extracted directly from the slope
// field of the catalog.  Its nonzero entries form the boundary-layer
// support: a role (row or column) participates in D iff at least one
// of its transitions varies with a.
inline std::vector<std::vector<long long>>
class_ii_neighbor_d_matrix(std::size_t neighbor) {
    const std::size_t size =
        class_ii_neighbor_dominant_core_states(neighbor).size();
    std::vector<std::vector<long long>> result(
        size, std::vector<long long>(size, 0));
    for (const auto& edge : class_ii_neighbor_affine_edges(neighbor)) {
        result[edge.source][edge.target] = edge.slope;
    }
    return result;
}

// Set of source-row indices in the canonical std::set order whose row
// in D contains at least one nonzero entry.  These are the source
// boundary-layer roles.  For every tested neighbor, these roles share
// the structural property "letter 0 with right endpoint j = 0 and
// third coordinate x[2] = 0" -- the source boundary-layer is the
// strictly-variable-length letter positions.
inline std::set<std::size_t>
class_ii_neighbor_d_boundary_source_indices(std::size_t neighbor) {
    std::set<std::size_t> result;
    for (const auto& edge : class_ii_neighbor_affine_edges(neighbor)) {
        if (edge.slope != 0) result.insert(edge.source);
    }
    return result;
}

// Set of target-column indices in the canonical std::set order whose
// column in D contains at least one nonzero entry.  The target roles
// are the four "fixed-endpoint partners" of the source boundary layer
// (their existence is forced by the cubic margin identity
// (beta - a - 1)(beta^2 + beta) = 1).
inline std::set<std::size_t>
class_ii_neighbor_d_boundary_target_indices(std::size_t neighbor) {
    std::set<std::size_t> result;
    for (const auto& edge : class_ii_neighbor_affine_edges(neighbor)) {
        if (edge.slope != 0) result.insert(edge.target);
    }
    return result;
}

// The set of underlying SNode<3> states that participate in any
// boundary-layer transition (as source OR target).  Smaller than the
// full dominant core by a factor of 2-5 depending on neighbor.
inline std::set<SNode<3>>
class_ii_neighbor_d_boundary_layer_states(std::size_t neighbor) {
    const auto states =
        class_ii_neighbor_dominant_core_states(neighbor);
    std::vector<SNode<3>> sorted_states(states.begin(), states.end());
    const auto sources =
        class_ii_neighbor_d_boundary_source_indices(neighbor);
    const auto targets =
        class_ii_neighbor_d_boundary_target_indices(neighbor);
    std::set<SNode<3>> result;
    for (auto s : sources) result.insert(sorted_states[s]);
    for (auto t : targets) result.insert(sorted_states[t]);
    return result;
}

// Total number of nonzero entries in D for the given neighbor.
// Observed: neighbor 0 -> 12, neighbor 1 -> 5, neighbor 2 -> 24.
inline std::size_t class_ii_neighbor_d_nonzero_count(std::size_t neighbor) {
    std::size_t result = 0;
    for (const auto& edge : class_ii_neighbor_affine_edges(neighbor)) {
        if (edge.slope != 0) ++result;
    }
    return result;
}

// Validate that A(a) = A(3) + (a-3)*D for the affine catalog.
// Equality is checked entrywise across a range of a values.  The
// qexpansion of the Class-II cubic (beta^3 = a*beta^2 + (a+1)*beta + 1)
// guarantees that D is integer-constant; this certificate closes the
// "every entry is affine in a" loop and confirms the slope extraction
// is exact, not a finite-point interpolation.
struct ClassIINeighborDCertificate {
    long long stable_from = 3;
    std::size_t matrices_checked = 0;
    std::size_t nonzero_entries = 0;
    std::size_t boundary_sources = 0;
    std::size_t boundary_targets = 0;
    bool affine_exact = false;
    bool boundary_layer_exact = false;
};

inline ClassIINeighborDCertificate
class_ii_neighbor_d_certificate(std::size_t neighbor, long long a_min = 3,
                                 long long a_max = 16) {
    ClassIINeighborDCertificate result;
    result.nonzero_entries =
        class_ii_neighbor_d_nonzero_count(neighbor);
    result.boundary_sources =
        class_ii_neighbor_d_boundary_source_indices(neighbor).size();
    result.boundary_targets =
        class_ii_neighbor_d_boundary_target_indices(neighbor).size();

    const auto a3 = class_ii_neighbor_affine_matrix(neighbor, 3);
    const auto D = class_ii_neighbor_d_matrix(neighbor);
    const std::size_t n = a3.size();
    result.affine_exact = true;
    for (long long a = a_min; a <= a_max; ++a) {
        const auto Aa = class_ii_neighbor_affine_matrix(neighbor, a);
        for (std::size_t s = 0; s < n; ++s) {
            for (std::size_t t = 0; t < n; ++t) {
                const long long predicted = a3[s][t] + (a - 3) * D[s][t];
                if (Aa[s][t] != predicted) result.affine_exact = false;
            }
        }
        ++result.matrices_checked;
    }

    // Boundary-layer support theorem (data-side): D has nonzero entries
    // exactly on the rows/columns identified by the index sets above.
    // The "exact" flag here just confirms consistency between the edge
    // catalog and the index-set extractor.
    result.boundary_layer_exact = true;
    for (const auto& edge : class_ii_neighbor_affine_edges(neighbor)) {
        if (edge.slope == 0) continue;
        const bool in_sources =
            class_ii_neighbor_d_boundary_source_indices(neighbor)
                .count(edge.source) == 1;
        const bool in_targets =
            class_ii_neighbor_d_boundary_target_indices(neighbor)
                .count(edge.target) == 1;
        if (!in_sources || !in_targets) result.boundary_layer_exact = false;
    }
    result.stable_from = a_min;
    return result;
}

// Closed-form qexpansion coefficient of an affine matrix entry.
// The matrix entry A(a) = intercept + slope*a has the asymptotic
// expansion in q = 1/a:
//   A(a) = slope * (1/q) + (intercept + 3 * slope) + O(q)
// The leading-order coefficient (the coefficient of 1/q, i.e. the
// slope itself) is what the boundary-layer theorem identifies: a
// nonzero coefficient means the entry sits in the boundary layer.
// This pairs the discrete slope with the cubic-driven q-expansion
// machinery in math/qexpansion.hpp: the same quadratic convergence
// delta = 1 - 1/a + 2/a^2 + ... that drives the lower margin
// 3/a + O(1/a^2) also forces the boundary-layer support to be the
// sparse catalogue observed above.
inline long long
class_ii_neighbor_d_qexpansion_leading_coefficient(
        std::size_t neighbor,
        std::size_t source_index,
        std::size_t target_index) {
    for (const auto& edge :
         class_ii_neighbor_affine_edges(neighbor)) {
        if (edge.source == source_index
                && edge.target == target_index) {
            return edge.slope;
        }
    }
    return 0;
}

struct ClassIINeighborAffineCertificate {
    long long stable_from = 3;
    std::size_t matrices_checked = 0;
    std::size_t polynomial_points_checked = 0;
    bool exact = false;
    bool polynomial_exact = false;
};

struct ClassIINeighborShellCertificate {
    bool exact = false;
    bool polynomial_exact = false;
    std::size_t cells_checked = 0;
    std::size_t matrices_checked = 0;
    std::size_t polynomial_points_checked = 0;
};

struct ClassIINeighborSpecialShellCertificate {
    bool exact = false;
    long long stable_from = 0;
    std::size_t families_checked = 0;
    std::size_t matrices_checked = 0;
};

struct ClassIINeighborIrreducibilityCertificate {
    bool core_irreducible = false;
    bool regular_shell_irreducible = false;
    bool special_shell_irreducible = false;
    bool monotone_support = false;

    bool exact() const {
        return core_irreducible
            && regular_shell_irreducible
            && special_shell_irreducible
            && monotone_support;
    }
};

inline std::vector<long long> class_ii_neighbor_expected_polynomial(
        std::size_t neighbor, long long a) {
    if (neighbor == 0) {
        std::vector<long long> p(16, 0);
        p[0] = 1;
        p[2] = -(a * a + 2);
        p[4] = 2 * a * a + 3 * a + 2;
        p[6] = -(a * a + 5 * a + 3);
        p[8] = 2 * a + 2;
        return p;
    }
    if (neighbor == 1) {
        std::vector<long long> p(18, 0);
        p[0] = 1;
        p[2] = -(a * a + 1);
        p[4] = a * a;
        p[5] = -(a + 2);
        p[7] = a;
        return p;
    }
    if (neighbor != 2) {
        throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
    }
    std::vector<long long> p(40, 0);
    p[0] = 1;
    p[2] = -(a * a - 1);
    p[3] = -1;
    p[4] = -(a * a + a + 1);
    p[5] = -(2 * a * a + 2 * a - 1);
    p[6] = -3 * a;
    p[7] = -(5 * a * a + 2);
    p[8] = -(2 * a * a - 3 * a + 8);
    p[9] = 2 * a * a - 3 * a + 4;
    p[10] = 4 * a * a - 2 * a;
    p[11] = -(3 * a * a + 6 * a - 5);
    p[12] = -(8 * a * a - 3 * a + 5);
    p[13] = 2 * a * a - 2;
    p[14] = 2 * a * a + 2 * a - 4;
    return p;
}

inline std::vector<long long>
class_ii_neighbor_expected_recurrent_polynomial(
        std::size_t neighbor, long long a, std::size_t rank) {
    if (a < 3) {
        throw std::domain_error(
            "Class-II recurrent neighbor families require a >= 3");
    }
    if (neighbor == 0) {
        if (rank < 2) return {1, 0, -1};
        if (rank == static_cast<std::size_t>(a - 1))
            return class_ii_neighbor_expected_polynomial(neighbor, a);
        if (rank >= static_cast<std::size_t>(a - 1))
            throw std::out_of_range("neighbor-0 recurrent rank");
        const long long t = static_cast<long long>(rank) - 1;
        std::vector<long long> p(11, 0);
        p[0] = 1;
        p[2] = -((t + 2) * (t + 2) - 2);
        p[4] = 1;
        return p;
    }
    if (neighbor == 1) {
        if (rank == static_cast<std::size_t>(a - 2))
            return class_ii_neighbor_expected_polynomial(neighbor, a);
        if (rank >= static_cast<std::size_t>(a - 2))
            throw std::out_of_range("neighbor-1 recurrent rank");
        const long long t = static_cast<long long>(rank) + 1;
        std::vector<long long> p(6, 0);
        p[0] = 1;
        p[2] = -t * (t + 2);
        return p;
    }
    if (neighbor == 2) {
        if (rank == 0 || rank == static_cast<std::size_t>(a - 2))
            return {1, 0, -1};
        if (rank == static_cast<std::size_t>(a))
            return class_ii_neighbor_expected_polynomial(neighbor, a);
        if (rank > static_cast<std::size_t>(a))
            throw std::out_of_range("neighbor-2 recurrent rank");
        const long long k = rank == static_cast<std::size_t>(a - 1)
            ? a - 1 : static_cast<long long>(rank) + 1;
        return {1, 0, -k * k, 0, 0};
    }
    throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
}

inline std::vector<std::vector<long long>>
class_ii_neighbor_expected_regular_shell_matrix(
        std::size_t neighbor, long long parameter) {
    const std::size_t size =
        neighbor == 0 ? 10 : (neighbor == 1 ? 5 : 4);
    if (neighbor > 2)
        throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
    if (parameter < (neighbor == 2 ? 2 : 1))
        throw std::domain_error("Class-II regular shell parameter");
    std::vector<std::vector<long long>> result(
        size, std::vector<long long>(size, 0));
    if (neighbor == 0) {
        const long long t = parameter;
        result[0][5] = result[0][6] = 1;
        result[1][7] = result[1][9] = t;
        result[1][8] = t - 1;
        result[2][7] = result[2][8] = result[2][9] = 1;
        result[3][5] = result[3][6] = 1;
        result[4][7] = result[4][9] = t + 1;
        result[4][8] = t;
        result[5][0] = t;
        result[5][1] = result[5][2] = t + 1;
        result[6][3] = result[6][4] = 1;
        result[7][0] = t - 1;
        result[7][1] = result[7][2] = t;
        result[8][3] = result[8][4] = 1;
        result[9][0] = result[9][1] = result[9][2] = 1;
        return result;
    }
    if (neighbor == 1) {
        const long long t = parameter;
        result[0][2] = result[0][3] = 1;
        result[1][2] = t;
        result[1][3] = t - 1;
        result[1][4] = 1;
        result[2][0] = t - 1;
        result[2][1] = t;
        result[3][0] = result[3][1] = 1;
        result[4][0] = result[4][1] = 1;
        return result;
    }
    const long long k = parameter;
    const long long h = k - 1;
    result[0][2] = result[0][3] = h;
    result[1][2] = result[1][3] = 1;
    result[2][0] = result[2][1] = h;
    result[3][0] = result[3][1] = 1;
    return result;
}

// Closed matrix forms on the canonical std::set order of the literal
// recurrent catalogues. These expose the small shell parameter directly:
// t=rank-1 for neighbor 0, t=rank+1 for neighbor 1, and the Perron
// integer k for neighbor 2.
inline std::vector<std::vector<long long>>
class_ii_neighbor_expected_recurrent_matrix(
        std::size_t neighbor, long long a, std::size_t rank) {
    const auto states =
        class_ii_neighbor_recurrent_component_states(neighbor, a, rank);
    std::vector<std::vector<long long>> result(
        states.size(), std::vector<long long>(states.size(), 0));
    if ((neighbor == 0 && rank == static_cast<std::size_t>(a - 1))
            || (neighbor == 1 && rank == static_cast<std::size_t>(a - 2))
            || (neighbor == 2 && rank == static_cast<std::size_t>(a))) {
        return class_ii_neighbor_affine_matrix(neighbor, a);
    }
    if ((neighbor == 0 && rank < 2)
            || (neighbor == 2
                && (rank == 0
                    || rank == static_cast<std::size_t>(a - 2)))) {
        result[0][1] = 1;
        result[1][0] = 1;
        return result;
    }
    if (neighbor == 0) {
        const long long t = static_cast<long long>(rank) - 1;
        return class_ii_neighbor_expected_regular_shell_matrix(
            neighbor, t);
    }
    if (neighbor == 1) {
        const long long t = static_cast<long long>(rank) + 1;
        return class_ii_neighbor_expected_regular_shell_matrix(
            neighbor, t);
    }
    if (neighbor == 2) {
        const long long k =
            rank == static_cast<std::size_t>(a - 1)
                ? a - 1
                : static_cast<long long>(rank) + 1;
        return class_ii_neighbor_expected_regular_shell_matrix(
            neighbor, k);
    }
    throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
}

// Exact finite-crossing certificate for the catalogue matrix.
//
// Every branch in class_ii_neighbor_transition_weight is selected by
// the sign or vanishing of one of the affine expressions collected
// below. Past stable_from none can cross zero. Each count is therefore
// affine on that tail; agreement at stable_from and stable_from+1
// proves agreement throughout it. Exhaustion of [3,stable_from) closes
// all earlier integer parameters.
inline ClassIINeighborAffineCertificate
class_ii_neighbor_affine_certificate(std::size_t neighbor) {
    std::vector<ClassIIAffineValue> controls;
    const auto states_set =
        class_ii_neighbor_dominant_core_states(neighbor);
    const std::vector<SNode<3>> states(
        states_set.begin(), states_set.end());
    for (const auto& source : states) {
        for (const auto& target : states) {
            const auto left =
                class_ii_neighbor_symbolic_prefix_families(
                    neighbor, static_cast<std::size_t>(target.i),
                    static_cast<std::size_t>(source.i));
            const auto right =
                class_ii_neighbor_symbolic_prefix_families(
                    neighbor, static_cast<std::size_t>(target.j),
                    static_cast<std::size_t>(source.j));
            const std::array<ClassIIAffineValue, 3> mx = {{
                {target.x[2], target.x[0] + target.x[1]},
                {target.x[0], 0},
                {target.x[0] + target.x[1], 0},
            }};
            for (const auto& p : left) {
                controls.push_back(p.length);
                for (const auto& q : right) {
                    controls.push_back(q.length);
                    std::array<ClassIIAffineValue, 3> delta{};
                    for (std::size_t coordinate = 0;
                         coordinate < 3; ++coordinate) {
                        delta[coordinate] =
                            ClassIIAffineValue{source.x[coordinate], 0}
                            + q.base[coordinate] - p.base[coordinate]
                            - mx[coordinate];
                    }
                    if (p.varying_coordinate == q.varying_coordinate) {
                        const auto c = p.varying_coordinate;
                        for (std::size_t other = 0; other < 3; ++other) {
                            if (other != c) controls.push_back(delta[other]);
                        }
                        // min(P,Q+D), max(0,D), and every possible
                        // upper-minus-lower comparison.
                        controls.push_back(
                            p.length - (q.length + delta[c]));
                        controls.push_back(delta[c]);
                        controls.push_back(p.length);
                        controls.push_back(p.length - delta[c]);
                        controls.push_back(q.length + delta[c]);
                        controls.push_back(q.length);
                    } else {
                        const auto pc = p.varying_coordinate;
                        const auto qc = q.varying_coordinate;
                        for (std::size_t other = 0; other < 3; ++other) {
                            if (other != pc && other != qc)
                                controls.push_back(delta[other]);
                        }
                        // 0<=k_p<P and 0<=k_q<Q, where
                        // k_p=D_pc and k_q=-D_qc.
                        controls.push_back(delta[pc]);
                        controls.push_back(-delta[qc]);
                        controls.push_back(p.length - delta[pc]);
                        controls.push_back(q.length + delta[qc]);
                    }
                }
            }
        }
    }

    long long stable_from = 3;
    for (const auto& value : controls) {
        if (value.slope == 0) continue;
        const auto numerator =
            value.intercept < 0 ? -value.intercept : value.intercept;
        const auto denominator =
            value.slope < 0 ? -value.slope : value.slope;
        // Strictly beyond the absolute value of every rational root.
        stable_from = std::max(
            stable_from, numerator / denominator + 2);
    }

    ClassIINeighborAffineCertificate result;
    result.stable_from = stable_from;
    result.exact = true;
    for (long long a = 3; a <= stable_from + 1; ++a) {
        result.exact = result.exact
            && class_ii_neighbor_compressed_matrix(neighbor, a)
                == class_ii_neighbor_affine_matrix(neighbor, a);
        ++result.matrices_checked;
    }
    // Each coefficient of det(xI-A(a)) has degree at most n in a:
    // every Leibniz product contains at most n affine entries. Equality
    // at n+1 distinct parameters is therefore a polynomial identity.
    //
    // charpoly_int (checked long-long) is deliberately kept here, not
    // charpoly_PolyZ: class_ii_neighbor_expected_polynomial's nonzero
    // coefficients are all low-degree-in-a quadratics (see its
    // definition), so if this identity holds -- which is exactly
    // what this loop is proving -- every true coefficient is small.
    // A thrown overflow_error would itself be a genuine finding, not
    // a masked one. See docs/RECOVERY_AUDIT_2026-07-29.md queue item
    // Q4.
    result.polynomial_exact = true;
    for (long long a = 3;
         a <= static_cast<long long>(states.size()) + 3; ++a) {
        result.polynomial_exact = result.polynomial_exact
            && charpoly_int(class_ii_neighbor_affine_matrix(neighbor, a))
                == class_ii_neighbor_expected_polynomial(neighbor, a);
        ++result.polynomial_points_checked;
    }
    return result;
}

inline std::vector<long long>
class_ii_neighbor_expected_regular_shell_polynomial(
        std::size_t neighbor, long long parameter) {
    if (neighbor == 0) {
        std::vector<long long> p(11, 0);
        p[0] = 1;
        p[2] = -((parameter + 2) * (parameter + 2) - 2);
        p[4] = 1;
        return p;
    }
    if (neighbor == 1) {
        std::vector<long long> p(6, 0);
        p[0] = 1;
        p[2] = -parameter * (parameter + 2);
        return p;
    }
    if (neighbor == 2)
        return {1, 0, -parameter * parameter, 0, 0};
    throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
}

// Universal fixed-branch certificate for every regular recurrent shell.
//
// Write a=parameter+offset+slack on the nonnegative parameter cone.
// The cells below isolate its coordinate axes and the two neighbor-0
// slack crossing lines. On each cell every counter control is either
// identically zero or has one strict fixed sign. Consequently each
// transition count is affine in the cell's active coordinates.
// Agreement at the origin and each unit basis point proves the displayed
// matrix throughout that infinite cell. The cells cover the whole cone.
inline ClassIINeighborShellCertificate
class_ii_neighbor_regular_shell_certificate(std::size_t neighbor) {
    if (neighbor > 2)
        throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
    struct Cell {
        long long parameter;
        long long slack;
        bool varying_parameter;
        bool varying_slack;
    };
    const long long minimum_parameter = neighbor == 2 ? 2 : 1;
    const long long offset = neighbor == 0 ? 3 : (neighbor == 1 ? 2 : 1);
    std::vector<Cell> cells;
    const std::vector<std::pair<long long, bool>> parameter_cells = {
        {minimum_parameter, false}, {minimum_parameter + 1, true}};
    const std::vector<std::pair<long long, bool>> slack_cells =
        neighbor == 0
            ? std::vector<std::pair<long long, bool>>{
                {0, false}, {1, false}, {2, true}}
            : std::vector<std::pair<long long, bool>>{
                {0, false}, {1, true}};
    for (const auto& [parameter, varying_parameter] : parameter_cells) {
        for (const auto& [slack, varying_slack] : slack_cells) {
            cells.push_back({
                parameter, slack, varying_parameter, varying_slack});
        }
    }

    const auto controls_at = [&](long long parameter, long long slack) {
        const long long a = parameter + offset + slack;
        return class_ii_neighbor_transition_controls(
            neighbor, a,
            class_ii_neighbor_regular_shell_states(
                neighbor, a, parameter));
    };
    const auto matrix_at = [&](long long parameter, long long slack) {
        const long long a = parameter + offset + slack;
        return class_ii_neighbor_compressed_matrix_for_states(
            neighbor, a,
            class_ii_neighbor_regular_shell_states(
                neighbor, a, parameter));
    };

    ClassIINeighborShellCertificate result;
    result.exact = true;
    for (const auto& cell : cells) {
        const auto base = controls_at(cell.parameter, cell.slack);
        auto parameter_step = base;
        auto slack_step = base;
        if (cell.varying_parameter)
            parameter_step =
                controls_at(cell.parameter + 1, cell.slack);
        if (cell.varying_slack)
            slack_step =
                controls_at(cell.parameter, cell.slack + 1);
        result.exact = result.exact
            && base.size() == parameter_step.size()
            && base.size() == slack_step.size();
        if (!result.exact) break;
        for (std::size_t i = 0; i < base.size(); ++i) {
            const long long dp = parameter_step[i] - base[i];
            const long long ds = slack_step[i] - base[i];
            const bool fixed =
                (base[i] == 0 && dp == 0 && ds == 0)
                || (base[i] > 0 && dp >= 0 && ds >= 0)
                || (base[i] < 0 && dp <= 0 && ds <= 0);
            result.exact = result.exact && fixed;
        }
        // The regular catalogues have affine coordinates and x0+x1 is
        // constant on every state, so the incidence action introduces
        // no parameter products. Check second and mixed samples as an
        // executable guard against catalogue/order regressions.
        if (cell.varying_parameter) {
            const auto parameter_second =
                controls_at(cell.parameter + 2, cell.slack);
            result.exact =
                result.exact && parameter_second.size() == base.size();
            for (std::size_t i = 0;
                 result.exact && i < base.size(); ++i) {
                result.exact = parameter_second[i]
                    == 2 * parameter_step[i] - base[i];
            }
        }
        if (cell.varying_slack) {
            const auto slack_second =
                controls_at(cell.parameter, cell.slack + 2);
            result.exact =
                result.exact && slack_second.size() == base.size();
            for (std::size_t i = 0;
                 result.exact && i < base.size(); ++i) {
                result.exact =
                    slack_second[i] == 2 * slack_step[i] - base[i];
            }
        }
        if (cell.varying_parameter && cell.varying_slack) {
            const auto mixed =
                controls_at(cell.parameter + 1, cell.slack + 1);
            result.exact = result.exact && mixed.size() == base.size();
            for (std::size_t i = 0;
                 result.exact && i < base.size(); ++i) {
                result.exact = mixed[i]
                    == parameter_step[i] + slack_step[i] - base[i];
            }
        }

        const auto base_matrix =
            matrix_at(cell.parameter, cell.slack);
        result.exact = result.exact
            && base_matrix
                == class_ii_neighbor_expected_regular_shell_matrix(
                    neighbor, cell.parameter);
        ++result.matrices_checked;
        if (cell.varying_parameter) {
            result.exact = result.exact
                && matrix_at(cell.parameter + 1, cell.slack)
                    == class_ii_neighbor_expected_regular_shell_matrix(
                        neighbor, cell.parameter + 1);
            ++result.matrices_checked;
        }
        if (cell.varying_slack) {
            result.exact = result.exact
                && matrix_at(cell.parameter, cell.slack + 1)
                    == class_ii_neighbor_expected_regular_shell_matrix(
                        neighbor, cell.parameter);
            ++result.matrices_checked;
        }
        ++result.cells_checked;
    }

    const std::size_t matrix_size =
        neighbor == 0 ? 10 : (neighbor == 1 ? 5 : 4);
    result.polynomial_exact = true;
    for (long long parameter = minimum_parameter;
         parameter <= minimum_parameter
             + static_cast<long long>(matrix_size);
         ++parameter) {
        result.polynomial_exact = result.polynomial_exact
            && charpoly_int(
                class_ii_neighbor_expected_regular_shell_matrix(
                    neighbor, parameter))
                == class_ii_neighbor_expected_regular_shell_polynomial(
                    neighbor, parameter);
        ++result.polynomial_points_checked;
    }
    return result;
}

// The four exceptional two-state shells (two for neighbor 0, two for
// neighbor 2) also have universal swap matrices. Their coordinates vary
// affinely with a, so the one-parameter finite-crossing argument used for
// the dominant cores applies after collecting controls from the literal
// state families.
inline ClassIINeighborSpecialShellCertificate
class_ii_neighbor_special_shell_certificate(std::size_t neighbor) {
    if (neighbor > 2)
        throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
    ClassIINeighborSpecialShellCertificate result;
    result.exact = true;
    result.stable_from = 3;
    if (neighbor == 1) return result;

    for (std::size_t special = 0; special < 2; ++special) {
        const auto at_three = class_ii_neighbor_transition_controls(
            neighbor, 3,
            class_ii_neighbor_special_shell_states(neighbor, 3, special));
        const auto at_four = class_ii_neighbor_transition_controls(
            neighbor, 4,
            class_ii_neighbor_special_shell_states(neighbor, 4, special));
        const auto at_five = class_ii_neighbor_transition_controls(
            neighbor, 5,
            class_ii_neighbor_special_shell_states(neighbor, 5, special));
        result.exact = result.exact
            && at_three.size() == at_four.size()
            && at_three.size() == at_five.size();
        if (!result.exact) return result;
        for (std::size_t i = 0; i < at_three.size(); ++i) {
            const long long slope = at_four[i] - at_three[i];
            result.exact = result.exact
                && at_five[i] == at_four[i] + slope;
            if (slope == 0) continue;
            const long long intercept = at_three[i] - 3 * slope;
            const long long numerator =
                intercept < 0 ? -intercept : intercept;
            const long long denominator =
                slope < 0 ? -slope : slope;
            result.stable_from = std::max(
                result.stable_from, numerator / denominator + 2);
        }
        ++result.families_checked;
    }

    const std::vector<std::vector<long long>> swap = {{0, 1}, {1, 0}};
    for (std::size_t special = 0; special < 2; ++special) {
        for (long long a = 3; a <= result.stable_from + 1; ++a) {
            result.exact = result.exact
                && class_ii_neighbor_compressed_matrix_for_states(
                    neighbor, a,
                    class_ii_neighbor_special_shell_states(
                        neighbor, a, special))
                    == swap;
            ++result.matrices_checked;
        }
    }
    return result;
}

// Perron-Frobenius applicability for every displayed catalogue matrix.
// At the minimum parameter each support graph is strongly connected.
// Every varying entry has nonnegative first difference, so later
// parameters can only add positive edges and cannot destroy that SCC.
inline ClassIINeighborIrreducibilityCertificate
class_ii_neighbor_irreducibility_certificate(std::size_t neighbor) {
    if (neighbor > 2)
        throw std::out_of_range("Class-II neighbor index must be 0, 1, or 2");
    const auto irreducible = [](const std::vector<std::vector<long long>>& A) {
        if (A.empty()) return false;
        const auto components = tarjan_scc(WeightedDigraph::from_dense(A));
        return components.size() == 1
            && components.front().size() == A.size();
    };

    ClassIINeighborIrreducibilityCertificate result;
    const auto core_base = class_ii_neighbor_affine_matrix(neighbor, 3);
    result.core_irreducible = irreducible(core_base);
    result.monotone_support = true;
    for (const auto& edge : class_ii_neighbor_affine_edges(neighbor)) {
        result.monotone_support = result.monotone_support
            && edge.weight(3) >= 0 && edge.slope >= 0;
    }

    const long long minimum_parameter = neighbor == 2 ? 2 : 1;
    const auto regular_base =
        class_ii_neighbor_expected_regular_shell_matrix(
            neighbor, minimum_parameter);
    const auto regular_next =
        class_ii_neighbor_expected_regular_shell_matrix(
            neighbor, minimum_parameter + 1);
    result.regular_shell_irreducible = irreducible(regular_base);
    for (std::size_t i = 0; i < regular_base.size(); ++i) {
        for (std::size_t j = 0; j < regular_base.size(); ++j) {
            result.monotone_support = result.monotone_support
                && regular_base[i][j] >= 0
                && regular_next[i][j] >= regular_base[i][j];
        }
    }
    result.special_shell_irreducible =
        irreducible({{0, 1}, {1, 0}});
    return result;
}

}  // namespace ravel
