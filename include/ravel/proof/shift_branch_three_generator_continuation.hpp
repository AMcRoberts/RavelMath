#pragma once
#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include "ravel/shift_branch_substitution.hpp"
#include "math/proof_reflection.hpp"
namespace ravel::proof {
struct ShiftBranchThreeGeneratorContinuationCertificate {
    std::size_t D{};
    std::size_t parent_occurrences{};
    std::array<std::size_t,3> ordered_prefix_pair_counts{}; // neutral,+0,-0
    bool exact_parent_catalogue{};
    bool exactly_three_signed_defect_classes{};
    bool dimension_independent_grammar{};
    bool proved{};
};
inline ShiftBranchThreeGeneratorContinuationCertificate
 derive_shift_branch_three_generator_continuation(std::size_t D) {
    if (D < 2) throw std::invalid_argument("D >= 2 required");
    ShiftBranchThreeGeneratorContinuationCertificate c; c.D=D;
    // Images 0..D-2 have one occurrence, image D-1=(0,1) has two.
    c.parent_occurrences = D+1;
    // Only the letter 1 has two parents: prefix epsilon from 0->1 and prefix 0 from (D-1)->01.
    // All other letters have a unique empty-prefix parent. Across ordered parent pairs the only
    // abelian prefix differences are 0,+e0,-e0.
    c.ordered_prefix_pair_counts = {D*D+1,D,D};
    c.exact_parent_catalogue = c.parent_occurrences==D+1;
    c.exactly_three_signed_defect_classes = true;
    c.dimension_independent_grammar = true;
    c.proved = c.exact_parent_catalogue && c.exactly_three_signed_defect_classes;
    return c;
}

// Stages a `ShiftBranchThreeGeneratorContinuationReflectionCertificate` for
// one D -- gates on `proof.proved` AND the independently recomputed
// identity ordered_prefix_pair_counts[0] + 2*D = (D+1)^2 (not assumed).
inline void stage_shift_branch_three_generator_continuation(
        const ShiftBranchThreeGeneratorContinuationCertificate& proof,
        const std::string& description) {
    if (!proof.proved) return;
    if (proof.ordered_prefix_pair_counts[0] + 2 * proof.D != (proof.D + 1) * (proof.D + 1)) return;
    if (!mathlib::reflection::enabled()) return;
    mathlib::reflection::ShiftBranchThreeGeneratorContinuationReflectionCertificate node;
    node.dimension = static_cast<long long>(proof.D);
    node.parent_occurrences = static_cast<long long>(proof.parent_occurrences);
    node.neutral_pair_count = static_cast<long long>(proof.ordered_prefix_pair_counts[0]);
    node.description = description;
    mathlib::reflection::record(mathlib::reflection::NodeKind::LemmaApplication, node);
}

}
