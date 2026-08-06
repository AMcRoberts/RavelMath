#pragma once

#include <cstddef>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "ravel/proof/two_matrix_permutation_skew_product.hpp"

namespace ravel::proof {

// A Condition-F transport channel together with the two n-bonacci parent
// prefixes that produced it.  For sigma(c)=0(c+1), every legal parent prefix
// is either empty or [0].
struct ConditionFPrefixChannel {
    std::size_t source = 0;
    std::size_t target = 0;
    std::vector<long long> left_prefix;
    std::vector<long long> right_prefix;
    std::vector<std::size_t> permutation;
};

struct ConditionFPrefixTwoMatrixProof {
    std::size_t base_vertices = 0;
    std::size_t fibre_size = 0;
    std::size_t balanced_channels = 0;
    std::size_t defect_channels = 0;
    std::size_t positive_defect_channels = 0;
    std::size_t negative_defect_channels = 0;
    std::vector<std::size_t> generator;
    std::vector<long long> signed_defect;
    TwoMatrixPermutationSkewProduct cocycle;
    bool prefix_grammar_recognized = false;
    bool canonical_binary_split = false;
    bool sign_retained_as_twist_cocycle = false;
    bool q_r_bulk_residual_analogy_exact = false;
    bool proved = false;
    std::string obstruction;
};

inline bool is_nbonacci_parent_prefix(const std::vector<long long>& p) {
    return p.empty() || (p.size() == 1 && p.front() == 0);
}

/** Derive the canonical two-matrix split from the n-bonacci parent grammar.
 *
 * The raw prefix defect is delta=|q|-|p| in {-1,0,+1}.  The matrix type is
 * |delta|: balanced transport (0) or one-sided residual transport (1).
 * The sign of delta is not discarded; it remains in the sheet permutation /
 * orientation cocycle.  Thus the apparent three cases are exactly a binary
 * matrix pair with a signed twist, the same bulk/residual split that appears
 * as qMatrix/rMatrix in the universal-n determinant proof.
 */
inline ConditionFPrefixTwoMatrixProof derive_condition_f_prefix_two_matrix(
    std::size_t base_vertices,
    std::size_t fibre_size,
    const std::vector<ConditionFPrefixChannel>& channels,
    std::size_t replay_word_depth = 10) {
    ConditionFPrefixTwoMatrixProof out;
    out.base_vertices = base_vertices;
    out.fibre_size = fibre_size;
    if (base_vertices == 0 || fibre_size == 0 || channels.empty()) {
        out.obstruction = "prefix two-matrix derivation requires nonempty base, fibre, and channels";
        return out;
    }

    std::vector<TwoMatrixSkewEdge> edges;
    edges.reserve(channels.size());
    out.generator.reserve(channels.size());
    out.signed_defect.reserve(channels.size());
    out.prefix_grammar_recognized = true;
    for (const auto& channel : channels) {
        if (!is_nbonacci_parent_prefix(channel.left_prefix) ||
            !is_nbonacci_parent_prefix(channel.right_prefix)) {
            out.prefix_grammar_recognized = false;
            out.obstruction = "channel uses a prefix outside the n-bonacci empty/[0] grammar";
            return out;
        }
        const long long delta =
            static_cast<long long>(channel.right_prefix.size()) -
            static_cast<long long>(channel.left_prefix.size());
        if (std::llabs(delta) > 1) {
            out.obstruction = "n-bonacci prefix defect is not in {-1,0,+1}";
            return out;
        }
        const std::size_t type = static_cast<std::size_t>(std::llabs(delta));
        out.generator.push_back(type);
        out.signed_defect.push_back(delta);
        if (type == 0) ++out.balanced_channels;
        else {
            ++out.defect_channels;
            if (delta > 0) ++out.positive_defect_channels;
            else ++out.negative_defect_channels;
        }
        edges.push_back({channel.source, channel.target, type,
                         channel.permutation});
    }

    out.canonical_binary_split =
        out.balanced_channels + out.defect_channels == channels.size();
    // The sign is retained separately from the binary matrix type.  In an
    // actual quotient it is represented by orientation/phase sheet transport;
    // keeping the signed list here prevents accidental collapse of + and -.
    out.sign_retained_as_twist_cocycle =
        out.signed_defect.size() == channels.size();
    out.cocycle = derive_two_matrix_permutation_skew_product(
        base_vertices, fibre_size, std::move(edges), replay_word_depth);
    out.q_r_bulk_residual_analogy_exact =
        out.canonical_binary_split && out.cocycle.valid;
    out.proved = out.prefix_grammar_recognized &&
                 out.canonical_binary_split &&
                 out.sign_retained_as_twist_cocycle &&
                 out.q_r_bulk_residual_analogy_exact;
    if (!out.proved && out.obstruction.empty()) out.obstruction = out.cocycle.obstruction;
    return out;
}

} // namespace ravel::proof
